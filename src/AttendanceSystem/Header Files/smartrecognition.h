#pragma once
#ifndef SMARTRECOGNITION_H
#define SMARTRECOGNITION_H

#include <QObject>
#include <QImage>
#include <QMap>
#include <QMutex>
#include <opencv2/opencv.hpp>

#pragma warning(disable: 4267)
class SmartRecognition : public QObject
{
    Q_OBJECT

public:
    explicit SmartRecognition(QObject* parent = nullptr);
    ~SmartRecognition();

    bool initialize(const QString& cascadePath);
    bool trainModel(const QMap<int, std::vector<cv::Mat>>& facesData);
    int recognizeFace(const cv::Mat& frame, cv::Rect& faceRect, double& confidence);

    cv::Mat extractFaceSafely(const cv::Mat& frame);

    QImage matToQImage(const cv::Mat& mat);
    cv::Mat qImageToMat(const QImage& image);

    bool saveModel(const QString& path);
    bool loadModel(const QString& path);

private:
    cv::CascadeClassifier m_faceCascade;

    struct ProfileData {
        cv::Mat histGray;
    };
    QMap<int, ProfileData> m_profiles;

    QMutex m_mutex;
    int m_minFaceSize = 60;
    double m_confidenceThreshold = 0.70;
    bool m_isInitialized = false;
    bool m_isTrained = false;

    bool detectFaceCascade(const cv::Mat& gray, std::vector<cv::Rect>& faces);
    cv::Rect fallbackSkinDetection(const cv::Mat& frame);
    cv::Mat preprocessFace(const cv::Mat& face);
};

#endif