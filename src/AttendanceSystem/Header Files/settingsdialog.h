#pragma once
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTimeEdit>
#include <QSpinBox>
#include <QPushButton>
#include "databasemanager.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(DatabaseManager* dbManager, QWidget* parent = nullptr);
    ~SettingsDialog();

private slots:
    void saveSettings();
    void resetToDefaults();

private:
    DatabaseManager* m_dbManager;

    QTimeEdit* m_startTimeEdit;
    QTimeEdit* m_endTimeEdit;
    QSpinBox* m_graceMinutesSpin;
    QSpinBox* m_overtimeThresholdSpin;

    void setupUI();
    void loadSettings();
};

#endif