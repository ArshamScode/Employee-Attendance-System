#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QGroupBox>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>

SettingsDialog::SettingsDialog(DatabaseManager* dbManager, QWidget* parent)
    : QDialog(parent)
    , m_dbManager(dbManager)
{
    setupUI();
    loadSettings();
    setWindowTitle("Work Schedule Settings");
    setMinimumWidth(400);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QGroupBox* hoursGroup = new QGroupBox("Work Hours");
    QFormLayout* hoursLayout = new QFormLayout(hoursGroup);

    m_startTimeEdit = new QTimeEdit(QTime(8, 0));
    m_startTimeEdit->setDisplayFormat("HH:mm");
    hoursLayout->addRow("Work Start Time:", m_startTimeEdit);

    m_endTimeEdit = new QTimeEdit(QTime(17, 0));
    m_endTimeEdit->setDisplayFormat("HH:mm");
    hoursLayout->addRow("Work End Time:", m_endTimeEdit);

    mainLayout->addWidget(hoursGroup);

    QGroupBox* thresholdGroup = new QGroupBox("Thresholds");
    QFormLayout* thresholdLayout = new QFormLayout(thresholdGroup);

    m_graceMinutesSpin = new QSpinBox();
    m_graceMinutesSpin->setRange(0, 60);
    m_graceMinutesSpin->setSuffix(" minutes");
    m_graceMinutesSpin->setValue(15);
    thresholdLayout->addRow("Grace Period for Late Entry:", m_graceMinutesSpin);

    m_overtimeThresholdSpin = new QSpinBox();
    m_overtimeThresholdSpin->setRange(0, 120);
    m_overtimeThresholdSpin->setSuffix(" minutes");
    m_overtimeThresholdSpin->setValue(30);
    thresholdLayout->addRow("Minimum Overtime to Count:", m_overtimeThresholdSpin);

    mainLayout->addWidget(thresholdGroup);

    QLabel* infoLabel = new QLabel(
        "<i>Note: Employees who forget to exit will be marked as absent for that day as penalty.</i>"
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #666; padding: 10px;");
    mainLayout->addWidget(infoLabel);

    mainLayout->addStretch();

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton* resetBtn = new QPushButton("Reset to Defaults");
    connect(resetBtn, &QPushButton::clicked, this, &SettingsDialog::resetToDefaults);
    buttonLayout->addWidget(resetBtn);

    QPushButton* cancelBtn = new QPushButton("Cancel");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelBtn);

    QPushButton* saveBtn = new QPushButton("Save Settings");
    saveBtn->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; padding: 8px 20px;");
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::saveSettings);
    buttonLayout->addWidget(saveBtn);

    mainLayout->addLayout(buttonLayout);
}

void SettingsDialog::loadSettings()
{
    WorkSettings settings = m_dbManager->loadWorkSettings();
    m_startTimeEdit->setTime(settings.workStartTime);
    m_endTimeEdit->setTime(settings.workEndTime);
    m_graceMinutesSpin->setValue(settings.graceMinutes);
    m_overtimeThresholdSpin->setValue(settings.overtimeThresholdMinutes);
}

void SettingsDialog::saveSettings()
{
    QDialog authDialog(this);
    authDialog.setWindowTitle("Admin Authentication");
    authDialog.setModal(true);

    QVBoxLayout* layout = new QVBoxLayout(&authDialog);
    layout->addWidget(new QLabel("Enter Admin Credentials to Save:"));

    QLineEdit* userEdit = new QLineEdit();
    userEdit->setPlaceholderText("Username");
    layout->addWidget(userEdit);

    QLineEdit* passEdit = new QLineEdit();
    passEdit->setPlaceholderText("Password");
    passEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &authDialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &authDialog, &QDialog::reject);

    if (authDialog.exec() == QDialog::Accepted) {
        if (userEdit->text() != "AS_admin" || passEdit->text() != "AS_P@ssword-Sett") {
            QMessageBox::warning(this, "Access Denied", "Incorrect username or password.");
            return;
        }
    }
    else {
        return;
    }

    WorkSettings settings;
    settings.workStartTime = m_startTimeEdit->time();
    settings.workEndTime = m_endTimeEdit->time();
    settings.graceMinutes = m_graceMinutesSpin->value();
    settings.overtimeThresholdMinutes = m_overtimeThresholdSpin->value();

    if (m_dbManager->saveWorkSettings(settings)) {
        QMessageBox::information(this, "Success", "Settings saved successfully!");
        accept();
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to save settings.");
    }
}

void SettingsDialog::resetToDefaults()
{
    m_startTimeEdit->setTime(QTime(8, 0));
    m_endTimeEdit->setTime(QTime(17, 0));
    m_graceMinutesSpin->setValue(15);
    m_overtimeThresholdSpin->setValue(30);
}