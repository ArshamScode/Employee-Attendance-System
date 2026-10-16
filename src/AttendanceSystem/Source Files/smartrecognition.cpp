#include "smartrecognition.h"
#include <QDebug>
#include <QFile>
#include <QDataStream>

SmartRecognition::SmartRecognition(QObject* parent) : QObject(parent) {}
SmartRecognition::~SmartRecognition() {}

bool SmartRecognition::initialize(const QString& cascadePath) {
    QMutexLocker locker(&m_mutex);
    if (!m_faceCascade.load(cascadePath.toStdString())) {
        qWarning() << "Cascade failed to load, relying on AI fallback.";
    }
    m_isInitialized = true;
    return true;
}

bool SmartRecognition::detectFaceCascade(const cv::Mat& gray, std::vector<cv::Rect>& faces) {
    if (m_faceCascade.empty()) return false;
    m_faceCascade.detectMultiScale(gray, faces, 1.05, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(m_minFaceSize, m_minFaceSize));
    if (faces.empty()) {
        m_faceCascade.detectMultiScale(gray, faces, 1.05, 1, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));
    }
    return !faces.empty();
}

cv::Rect SmartRecognition::fallbackSkinDetection(const cv::Mat& frame) {
    cv::Mat ycrb;
    cv::cvtColor(frame, ycrb, cv::COLOR_BGR2YCrCb);

    cv::Mat skinMask;
    cv::inRange(ycrb, cv::Scalar(0, 133, 77), cv::Scalar(255, 173, 127), skinMask);

    cv::morphologyEx(skinMask, skinMask, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
    cv::morphologyEx(skinMask, skinMask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(11, 11)));

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(skinMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) return cv::Rect();

    int maxArea = 0;
    cv::Rect largestRect;
    for (const auto& cnt : contours) {
        cv::Rect r = cv::boundingRect(cnt);
        if (r.width < 50 || r.height < 60) continue;

        if (r.area() > maxArea) {
            maxArea = r.area();
            largestRect = r;
        }
    }
    return largestRect;
}

cv::Mat SmartRecognition::extractFaceSafely(const cv::Mat& frame) {
    if (frame.empty()) return cv::Mat();

    cv::Mat gray;
    if (frame.channels() == 3) cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    else gray = frame.clone();

    cv::Rect faceRect;
    std::vector<cv::Rect> faces;

    if (!detectFaceCascade(gray, faces)) {
        faceRect = fallbackSkinDetection(frame);
        if (faceRect.area() == 0) return cv::Mat();
    }
    else {
        int maxArea = 0;
        for (const auto& f : faces) if (f.area() > maxArea) { maxArea = f.area(); faceRect = f; }
    }

    int x = std::max(0, faceRect.x);
    int y = std::max(0, faceRect.y);
    int w = std::min(faceRect.width, frame.cols - x);
    int h = std::min(faceRect.height, frame.rows - y);
    if (w <= 0 || h <= 0) return cv::Mat();

    cv::Mat faceROI = frame(cv::Rect(x, y, w, h));
    return preprocessFace(faceROI);
}

cv::Mat SmartRecognition::preprocessFace(const cv::Mat& face) {
    cv::Mat gray;
    if (face.channels() == 3) cv::cvtColor(face, gray, cv::COLOR_BGR2GRAY);
    else gray = face.clone();

    cv::equalizeHist(gray, gray);
    cv::Mat resized;
    cv::resize(gray, resized, cv::Size(150, 150));
    return resized;
}

bool SmartRecognition::trainModel(const QMap<int, std::vector<cv::Mat>>& facesData) {
    QMutexLocker locker(&m_mutex);
    if (facesData.isEmpty()) return false;

    m_profiles.clear();
    for (auto it = facesData.begin(); it != facesData.end(); ++it) {
        cv::Mat avgFace;
        int count = 0;
        for (const auto& face : it.value()) {
            if (!face.empty()) {
                if (avgFace.empty()) {
                    avgFace = face.clone();
                }
                else {
                    cv::addWeighted(avgFace, 0.5, face, 0.5, 0, avgFace);
                }
                count++;
            }
        }
        if (count > 0) {
            ProfileData data;
            data.histGray = avgFace;
            m_profiles[it.key()] = data;
        }
    }

    m_isTrained = !m_profiles.isEmpty();
    return m_isTrained;
}

int SmartRecognition::recognizeFace(const cv::Mat& frame, cv::Rect& faceRect, double& confidence) {
    QMutexLocker locker(&m_mutex);
    if (!m_isTrained) return -1;
    if (frame.empty()) return -1;

    cv::Mat smallFrame;
    cv::resize(frame, smallFrame, cv::Size(320, 240));

    cv::Mat gray;
    cv::cvtColor(smallFrame, gray, cv::COLOR_BGR2GRAY);

    std::vector<cv::Rect> faces;
    if (!detectFaceCascade(gray, faces)) {
        return -1;
    }

    int maxArea = 0;
    cv::Rect bestFace;
    for (const auto& f : faces) {
        if (f.area() > maxArea) { maxArea = f.area(); bestFace = f; }
    }

    if (bestFace.area() == 0) return -1;

    double scaleX = (double)frame.cols / smallFrame.cols;
    double scaleY = (double)frame.rows / smallFrame.rows;
    faceRect = cv::Rect(bestFace.x * scaleX, bestFace.y * scaleY, bestFace.width * scaleX, bestFace.height * scaleY);

    if (faceRect.x < 0) faceRect.x = 0;
    if (faceRect.y < 0) faceRect.y = 0;
    if (faceRect.x + faceRect.width > frame.cols) faceRect.width = frame.cols - faceRect.x;
    if (faceRect.y + faceRect.height > frame.rows) faceRect.height = frame.rows - faceRect.y;

    cv::Mat faceROI = frame(faceRect);
    cv::Mat processedFace = preprocessFace(faceROI);

    double bestMatch = 0.0;
    int bestLabel = -1;

    for (auto it = m_profiles.begin(); it != m_profiles.end(); ++it) {
        if (!it.value().histGray.empty() && it.value().histGray.size() == processedFace.size()) {
            cv::Mat result;
            cv::matchTemplate(processedFace, it.value().histGray, result, cv::TM_CCOEFF_NORMED);
            double match = result.at<float>(0, 0);

            if (match > bestMatch) {
                bestMatch = match;
                bestLabel = it.key();
            }
        }
    }

    confidence = (1.0 - bestMatch) * 100;
    return (bestMatch >= 0.60) ? bestLabel : -1;
}

QImage SmartRecognition::matToQImage(const cv::Mat& mat) {
    switch (mat.type()) {
    case CV_8UC4: { QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB32); return image.copy(); }
    case CV_8UC3: { cv::Mat rgb; cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB); QImage image(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888); return image.copy(); }
    case CV_8UC1: { QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8); return image.copy(); }
    default: return QImage();
    }
}

cv::Mat SmartRecognition::qImageToMat(const QImage& image) {
    switch (image.format()) {
    case QImage::Format_RGB32: { cv::Mat mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine()); return mat.clone(); }
    case QImage::Format_RGB888: { cv::Mat mat(image.height(), image.width(), CV_8UC3, const_cast<uchar*>(image.bits()), image.bytesPerLine()); cv::Mat bgr; cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR); return bgr; }
    case QImage::Format_Grayscale8: { cv::Mat mat(image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.bits()), image.bytesPerLine()); return mat.clone(); }
    default: { QImage converted = image.convertToFormat(QImage::Format_RGB888); return qImageToMat(converted); }
    }
}

bool SmartRecognition::saveModel(const QString& path) {
    QMutexLocker locker(&m_mutex);
    if (!m_isTrained) return false;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return false;
    QDataStream out(&file);
    out << m_profiles.size();
    for (auto it = m_profiles.begin(); it != m_profiles.end(); ++it) {
        out << it.key() << it.value().histGray.rows << it.value().histGray.cols << it.value().histGray.type();
        out.writeRawData(reinterpret_cast<const char*>(it.value().histGray.data), static_cast<int>(it.value().histGray.total() * it.value().histGray.elemSize()));
    }
    return true;
}

bool SmartRecognition::loadModel(const QString& path) {
    QMutexLocker locker(&m_mutex);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QDataStream in(&file);
    int size; in >> size;
    m_profiles.clear();
    for (int i = 0; i < size; ++i) {
        int id, rows, cols, type;
        in >> id >> rows >> cols >> type;
        cv::Mat mat(rows, cols, type);
        in.readRawData(reinterpret_cast<char*>(mat.data), static_cast<int>(mat.total() * mat.elemSize()));
        m_profiles[id].histGray = mat;
    }
    if (m_profiles.size() > 0) { m_isTrained = true; return true; }
    return false;
}