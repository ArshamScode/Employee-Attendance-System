#include "camerawidget.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDateTime>
#include <QApplication>
#include <QDebug>
#include "smartrecognition.h"

CameraWidget::CameraWidget(QWidget* parent)
    : QWidget(parent)
    , m_faceRecognizer(new SmartRecognition(this))
    , m_captureTimer(new QTimer(this))
    , m_statusLabel(new QLabel(this))
    , m_cameraLabel(new QLabel(this))
    , m_colorTimer(new QTimer(this))
    , m_disappearTimer(new QTimer(this))
{
    QString cascadePath = QApplication::applicationDirPath() + "/models/haarcascade_frontalface_default.xml";
    if (!m_faceRecognizer->initialize(cascadePath)) {
        cascadePath = "models/haarcascade_frontalface_default.xml";
        m_faceRecognizer->initialize(cascadePath);
    }

    QString modelPath = QApplication::applicationDirPath() + "/data/facemodel.yml";
    m_faceRecognizer->loadModel(modelPath);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_cameraLabel->setAlignment(Qt::AlignCenter);
    m_cameraLabel->setMinimumSize(640, 480);
    m_cameraLabel->setStyleSheet("background-color: black; border: 3px solid black;");
    layout->addWidget(m_cameraLabel);

    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setMinimumHeight(30);
    m_statusLabel->setStyleSheet("font-size: 14px; font-weight: bold; padding: 5px; background-color: #333; color: white;");
    layout->addWidget(m_statusLabel);

    connect(m_captureTimer, &QTimer::timeout, this, &CameraWidget::processFrame);
    m_captureTimer->setInterval(150);

    connect(m_colorTimer, &QTimer::timeout, this, [this]() {
        m_frameColor = Qt::black;
        m_cameraLabel->setStyleSheet("border: 3px solid black;");
        m_colorTimer->stop();
        });

    m_disappearTimer->setSingleShot(true);
    connect(m_disappearTimer, &QTimer::timeout, this, &CameraWidget::checkFaceDisappeared);

    setMinimumSize(660, 530);
}

CameraWidget::~CameraWidget() { stopCamera(); }

bool CameraWidget::trainModel(const QMap<int, std::vector<cv::Mat>>& facesData)
{
    return m_faceRecognizer->trainModel(facesData);
}

bool CameraWidget::startCamera()
{
    if (m_isRunning) return true;
    for (int i = 0; i < 3; ++i) {
        m_cap.open(i);
        if (m_cap.isOpened()) {
            m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
            m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
            m_captureTimer->start();
            m_isRunning = true;
            m_statusLabel->setText("Camera active - Waiting for face...");
            return true;
        }
    }
    emit cameraError("Could not open camera");
    return false;
}

void CameraWidget::stopCamera()
{
    m_captureTimer->stop();
    if (m_cap.isOpened()) m_cap.release();
    m_isRunning = false;
    m_currentFrame = QImage();
    update();
}

bool CameraWidget::isRunning() const { return m_isRunning; }

QImage CameraWidget::captureImage()
{
    if (!m_cap.isOpened()) return QImage();
    cv::Mat frame;
    m_cap >> frame;
    return frame.empty() ? QImage() : m_faceRecognizer->matToQImage(frame);
}

void CameraWidget::setMode(QString mode)
{
    m_mode = mode;
    m_statusLabel->setText(mode == "register" ? "Registration Mode" : "Attendance Mode");
}

void CameraWidget::processFrame()
{
    if (!m_cap.isOpened() || !m_captureEnabled) return;

    cv::Mat frame;
    m_cap >> frame;
    if (frame.empty()) return;

    if (m_mode == "attendance") {
        cv::Rect faceRect;
        double confidence = 0;
        int employeeId = -1;

        try {
            employeeId = m_faceRecognizer->recognizeFace(frame, faceRect, confidence);
        }
        catch (...) {
            employeeId = -1;
        }

        QImage displayImage = m_faceRecognizer->matToQImage(frame);
        if (!displayImage.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(displayImage).scaled(m_cameraLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation);

            if (employeeId >= 0 || faceRect.area() > 0) {
                QPainter painter(&pixmap);

                double scaleX = (double)pixmap.width() / frame.cols;
                double scaleY = (double)pixmap.height() / frame.rows;
                QRect drawRect(faceRect.x * scaleX, faceRect.y * scaleY, faceRect.width * scaleX, faceRect.height * scaleY);

                if (employeeId >= 0) {
                    painter.setPen(QPen(Qt::green, 3));
                }
                else {
                    painter.setPen(QPen(Qt::red, 3));
                }
                painter.drawRect(drawRect);
            }
            m_cameraLabel->setPixmap(pixmap);
        }

        if (employeeId >= 0 && !m_faceInFrame) {
            m_faceInFrame = true;
            m_disappearTimer->stop();
            emit faceRecognized(employeeId, confidence);
        }
        else if (employeeId >= 0 && m_faceInFrame) {
            m_disappearTimer->stop();
        }
        else if (employeeId < 0 && m_faceInFrame) {
            if (!m_disappearTimer->isActive()) {
                m_disappearTimer->start(2000);
            }
        }
        return;
    }

    m_cameraLabel->setPixmap(QPixmap::fromImage(m_faceRecognizer->matToQImage(frame)).scaled(m_cameraLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
}

void CameraWidget::handleRecognitionResult(int employeeId, double confidence) { Q_UNUSED(employeeId); Q_UNUSED(confidence); }
void CameraWidget::drawFaceBox(QPainter& painter, const QRect& rect, const QColor& color) { painter.setPen(QPen(color, 3)); painter.drawRect(rect); }
void CameraWidget::paintEvent(QPaintEvent* event) { Q_UNUSED(event) }

void CameraWidget::checkFaceDisappeared()
{
    m_faceInFrame = false;
}