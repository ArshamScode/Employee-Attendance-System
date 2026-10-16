#pragma once
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QImage>
#include <QBuffer>

struct Employee {
    int id = -1;
    QString name;
    QString position;
    QString department;
    QString phone;
    QString photoPath;
    QByteArray photoData;
    QDateTime createdAt;
};

struct AttendanceRecord {
    int id = -1;
    int employeeId;
    QString employeeName;
    QDate date;
    QTime entryTime;
    QTime exitTime;
    QString status;
    int entryCount = 0;
    int exitCount = 0;
};

struct WorkSettings {
    QTime workStartTime = QTime(8, 0);
    QTime workEndTime = QTime(17, 0);
    int graceMinutes = 15;
    int overtimeThresholdMinutes = 30;
};

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager();

    bool initializeDatabase();

    int addEmployee(const Employee& employee);
    bool updateEmployee(const Employee& employee);
    bool deleteEmployee(int id);
    Employee getEmployee(int id);
    Employee getEmployeeByName(const QString& name);
    QList<Employee> getAllEmployees();

    int addAttendanceRecord(const AttendanceRecord& record);
    bool updateAttendanceExitTime(int recordId, const QTime& exitTime);
    bool updateAttendanceStatus(int recordId, const QString& status);
    AttendanceRecord getTodayAttendance(int employeeId);
    QList<AttendanceRecord> getAttendanceByDate(const QDate& date);
    QList<AttendanceRecord> getAttendanceByMonth(int year, int month);
    QList<AttendanceRecord> getAttendanceByEmployee(int employeeId, int year, int month);

    bool saveWorkSettings(const WorkSettings& settings);
    WorkSettings loadWorkSettings();

    QList<AttendanceRecord> searchAttendance(const QString& query, int year, int month);
    bool updateEntryTime(int recordId, const QTime& time);

    QMap<int, qint64> calculateMonthlyOvertime(int year, int month);
    QMap<int, int> calculateMonthlyAbsences(int year, int month);
    QMap<int, int> calculateMonthlyLeaves(int year, int month);
    QMap<int, int> calculateMonthlyForgotExits(int year, int month);

private:
    QSqlDatabase m_db;
    QString m_dbPath;

    bool createTables();
    void logError(const QString& operation, const QSqlError& error);
};

#endif
