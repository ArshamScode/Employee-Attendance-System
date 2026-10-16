#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include "smartrecognition.h"

class CameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraWidget(QWidget* parent = nullptr);
    ~CameraWidget();

    bool startCamera();
    void stopCamera();
    bool isRunning() const;
    QImage captureImage();
    void setMode(QString mode);
    bool trainModel(const QMap<int, std::vector<cv::Mat>>& facesData);
    SmartRecognition* m_faceRecognizer;

signals:
    void faceRecognized(int employeeId, double confidence);
    void faceCaptured(const QImage& image);
    void statusChanged(const QString& message, const QColor& color);
    void cameraError(const QString& error);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void processFrame();
    void handleRecognitionResult(int employeeId, double confidence);
    void checkFaceDisappeared();

private:
    cv::VideoCapture m_cap;
    QTimer* m_captureTimer;

    QLabel* m_statusLabel;
    QLabel* m_cameraLabel;

    QImage m_currentFrame;
    QImage m_lastCapturedFace;

    QString m_mode = "attendance";
    bool m_isRunning = false;
    bool m_captureEnabled = true;

    QTimer* m_colorTimer;
    QColor m_frameColor = Qt::black;

    bool m_faceInFrame = false;
    QTimer* m_disappearTimer;

    void drawFaceBox(QPainter& painter, const QRect& rect, const QColor& color);
};

#endif