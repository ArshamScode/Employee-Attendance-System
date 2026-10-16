#pragma once
#ifndef EMPLOYEEMANAGEMENT_H
#define EMPLOYEEMANAGEMENT_H

#include <QObject>
#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include "databasemanager.h"
#include <QTimer>
#include <opencv2/opencv.hpp>
#include "smartrecognition.h"
#include <QVBoxLayout>
#include <QPushButton>

class CaptureDialog : public QDialog {
    Q_OBJECT
public:
    CaptureDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Take Photo");
        setModal(true);
        resize(660, 540);

        QVBoxLayout* layout = new QVBoxLayout(this);
        m_label = new QLabel(this);
        m_label->setStyleSheet("background-color: black;");
        layout->addWidget(m_label);

        QPushButton* snapBtn = new QPushButton("Snap Photo", this);
        snapBtn->setStyleSheet("background-color: #4CAF50; color: white; font-size: 16px; padding: 10px;");
        connect(snapBtn, &QPushButton::clicked, this, &CaptureDialog::takeSnapshot);
        layout->addWidget(snapBtn);

        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &CaptureDialog::updateFrame);
        m_cap.open(0);
        m_timer->start(150);
        m_recognizer.initialize("models/haarcascade_frontalface_default.xml");
    }
    ~CaptureDialog() { m_cap.release(); }
    QImage getCapturedPhoto() { return m_capturedPhoto; }

private slots:
    void updateFrame() {
        cv::Mat frame; m_cap >> frame;
        if (frame.empty()) return;

        m_label->setPixmap(QPixmap::fromImage(m_recognizer.matToQImage(frame)).scaled(m_label->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
    }

    void takeSnapshot() {
        m_timer->stop();
        cv::Mat frame; m_cap >> frame;
        if (frame.empty()) { reject(); return; }

        cv::Mat face = m_recognizer.extractFaceSafely(frame);
        if (face.empty()) { reject(); return; }

        m_capturedPhoto = m_recognizer.matToQImage(face);
        accept();
    }

private:
    QLabel* m_label; QTimer* m_timer; cv::VideoCapture m_cap; QImage m_capturedPhoto; SmartRecognition m_recognizer;
};

class EmployeeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmployeeDialog(QWidget* parent = nullptr, int employeeId = -1);
    ~EmployeeDialog();

    Employee getEmployee() const;
    QImage getCapturedPhoto() const;
    void setEmployee(const Employee& employee);

public slots:
    void capturePhoto();

signals:
    void photoCaptured(const QImage& photo);

private:
    QLineEdit* m_nameEdit;
    QLineEdit* m_positionEdit;
    QLineEdit* m_departmentEdit;
    QLineEdit* m_phoneEdit;
    QLabel* m_photoLabel;
    QPushButton* m_captureBtn;

    QImage m_capturedPhoto;
    int m_employeeId;

    void setupUI();
};

class EmployeeManagement : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeeManagement(DatabaseManager* dbManager, QWidget* parent = nullptr);
    ~EmployeeManagement();

public slots:
    void addEmployee();
    void editEmployee();
    void deleteEmployee();
    void refreshList();

signals:
    void employeesChanged();
    void requestCameraCapture();

private:
    DatabaseManager* m_dbManager;
    QListWidget* m_employeeList;
    QLabel* m_detailsLabel;
    QPushButton* m_addBtn;
    QPushButton* m_editBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_refreshBtn;

    void setupUI();
    void showEmployeeDetails(const Employee& employee);
    bool saveEmployeeWithPhoto(const Employee& employee, const QImage& photo);
};

#endif