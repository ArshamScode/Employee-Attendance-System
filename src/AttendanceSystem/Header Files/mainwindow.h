#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QLabel>
#include <QTimer>
#include <QStatusBar>
#include "databasemanager.h"
#include "camerawidget.h"
#include "employeemanagement.h"
#include "attendancetable.h"
#include "settingsdialog.h"
#include "smartrecognition.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void handleFaceRecognized(int employeeId, double confidence);
    void onEmployeesChanged();
    void trainFaceModel();
    void showSettings();
    void onTabChanged(int index);
    void updateClock();
    void markEndOfDayAbsences();
    void showAbout();

signals:
    void logMessage(const QString& msg);

private:
    DatabaseManager* m_dbManager;
    CameraWidget* m_cameraWidget;
    EmployeeManagement* m_employeeManagement;
    AttendanceTable* m_attendanceTable;

    QTabWidget* m_tabWidget;
    QLabel* m_statusBarLabel;
    QLabel* m_clockLabel;
    QTimer* m_clockTimer;
    QTimer* m_endOfDayTimer;

    int m_lastTabIndex = 0;
    bool authenticate(const QString& expectedUser, const QString& expectedPass);
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void initializeDefaultSettings();
    QWidget* createAttendanceTab();
    int processAttendance(int employeeId);
    bool ensureTodayAttendanceRecord(int employeeId);
};

#endif