#pragma once
#ifndef ATTENDANCETABLE_H
#define ATTENDANCETABLE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDateEdit>
#include "databasemanager.h"

class AttendanceTable : public QWidget
{
    Q_OBJECT

public:
    explicit AttendanceTable(DatabaseManager* dbManager, QWidget* parent = nullptr);
    ~AttendanceTable();

    void refreshTable();
    void refreshForDate(const QDate& date);
    void refreshForMonth(int year, int month);
    void markForgotExits();

public slots:
    void searchAttendance();
    void exportToCSV();

signals:
    void requestMarkAbsent();

private:
    DatabaseManager* m_dbManager;
    QTableWidget* m_table;
    QLineEdit* m_searchEdit;
    QComboBox* m_searchField;
    QDateEdit* m_dateEdit;
    QComboBox* m_viewMode;
    QLabel* m_summaryLabel;
    QPushButton* m_searchBtn;
    QPushButton* m_exportBtn;
    QPushButton* m_refreshBtn;

    void setupUI();
    void populateTable(const QList<AttendanceRecord>& records);
    void updateSummary();
    void setupTableHeaders();

    QString getStatusDisplay(const QString& status) const;
    QColor getStatusColor(const QString& status) const;
};

#endif