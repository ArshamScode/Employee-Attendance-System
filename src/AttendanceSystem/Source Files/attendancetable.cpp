#include "attendancetable.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QGroupBox>

AttendanceTable::AttendanceTable(DatabaseManager* dbManager, QWidget* parent)
    : QWidget(parent)
    , m_dbManager(dbManager)
{
    setupUI();
    refreshForDate(QDate::currentDate());
}

AttendanceTable::~AttendanceTable()
{
}

void AttendanceTable::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QGroupBox* filterGroup = new QGroupBox("Filters");
    QHBoxLayout* filterLayout = new QHBoxLayout(filterGroup);

    filterLayout->addWidget(new QLabel("View:"));
    m_viewMode = new QComboBox();
    m_viewMode->addItems({ "Daily", "Monthly" });
    m_viewMode->setCurrentIndex(0);
    connect(m_viewMode, &QComboBox::currentTextChanged, [this](const QString& mode) {
        if (mode == "Daily") {
            refreshForDate(m_dateEdit->date());
        }
        else {
            refreshForMonth(m_dateEdit->date().year(), m_dateEdit->date().month());
        }
        });
    filterLayout->addWidget(m_viewMode);

    filterLayout->addWidget(new QLabel("Date:"));
    m_dateEdit = new QDateEdit(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    connect(m_dateEdit, &QDateEdit::dateChanged, [this](const QDate& date) {
        if (m_viewMode->currentText() == "Daily") {
            refreshForDate(date);
        }
        else {
            refreshForMonth(date.year(), date.month());
        }
        });
    filterLayout->addWidget(m_dateEdit);

    filterLayout->addStretch();

    filterLayout->addWidget(new QLabel("Search:"));
    m_searchField = new QComboBox();
    m_searchField->addItems({ "All Fields", "Name", "Position", "Department", "Phone" });
    filterLayout->addWidget(m_searchField);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Enter search term...");
    m_searchEdit->setMinimumWidth(200);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &AttendanceTable::searchAttendance);
    filterLayout->addWidget(m_searchEdit);

    m_searchBtn = new QPushButton("Search");
    connect(m_searchBtn, &QPushButton::clicked, this, &AttendanceTable::searchAttendance);
    filterLayout->addWidget(m_searchBtn);

    mainLayout->addWidget(filterGroup);

    m_table = new QTableWidget();
    m_table->setAlternatingRowColors(true);
    m_table->setSortingEnabled(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setDefaultSectionSize(25);

    setupTableHeaders();
    mainLayout->addWidget(m_table);

    QHBoxLayout* bottomLayout = new QHBoxLayout();

    m_summaryLabel = new QLabel();
    m_summaryLabel->setStyleSheet("font-size: 13px; padding: 10px; background-color: #e3f2fd; border-radius: 5px;");
    bottomLayout->addWidget(m_summaryLabel, 1);

    m_refreshBtn = new QPushButton("Refresh");
    connect(m_refreshBtn, &QPushButton::clicked, this, &AttendanceTable::refreshTable);
    bottomLayout->addWidget(m_refreshBtn);

    m_exportBtn = new QPushButton("Export CSV");
    m_exportBtn->setStyleSheet("background-color: #4CAF50; color: white; padding: 8px;");
    connect(m_exportBtn, &QPushButton::clicked, this, &AttendanceTable::exportToCSV);
    bottomLayout->addWidget(m_exportBtn);

    mainLayout->addLayout(bottomLayout);
}

void AttendanceTable::setupTableHeaders()
{
    QStringList headers = {
        "ID", "Employee Name", "Date", "Entry Time", "Exit Time",
        "Status", "Entry Count", "Exit Count", "Work Hours", "Overtime"
    };
    m_table->setColumnCount(headers.size());
    m_table->setHorizontalHeaderLabels(headers);

    m_table->setColumnWidth(0, 50);
    m_table->setColumnWidth(1, 150);
    m_table->setColumnWidth(2, 100);
    m_table->setColumnWidth(3, 100);
    m_table->setColumnWidth(4, 100);
    m_table->setColumnWidth(5, 120);
    m_table->setColumnWidth(6, 80);
    m_table->setColumnWidth(7, 80);
    m_table->setColumnWidth(8, 100);
    m_table->setColumnWidth(9, 100);
}

void AttendanceTable::populateTable(const QList<AttendanceRecord>& records)
{
    m_table->setRowCount(records.size());
    WorkSettings settings = m_dbManager->loadWorkSettings();

    for (int i = 0; i < records.size(); ++i) {
        const AttendanceRecord& record = records[i];
        int row = i;

        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(record.id)));

        m_table->setItem(row, 1, new QTableWidgetItem(record.employeeName));

        m_table->setItem(row, 2, new QTableWidgetItem(record.date.toString(Qt::ISODate)));

        m_table->setItem(row, 3, new QTableWidgetItem(
            record.entryTime.isValid() ? record.entryTime.toString("HH:mm:ss") : "-"));

        m_table->setItem(row, 4, new QTableWidgetItem(
            record.exitTime.isValid() ? record.exitTime.toString("HH:mm:ss") : "-"));

        QTableWidgetItem* statusItem = new QTableWidgetItem(getStatusDisplay(record.status));
        statusItem->setForeground(getStatusColor(record.status));
        statusItem->setFont(QFont("", -1, QFont::Bold));
        m_table->setItem(row, 5, statusItem);

        m_table->setItem(row, 6, new QTableWidgetItem(QString::number(record.entryCount)));

        m_table->setItem(row, 7, new QTableWidgetItem(QString::number(record.exitCount)));

        QString workHours = "-";
        if (record.entryTime.isValid() && record.exitTime.isValid()) {
            qint64 seconds = record.entryTime.secsTo(record.exitTime);
            int hours = seconds / 3600;
            int minutes = (seconds % 3600) / 60;
            workHours = QString("%1h %2m").arg(hours).arg(minutes);
        }
        m_table->setItem(row, 8, new QTableWidgetItem(workHours));

        QString overtime = "-";
        if (record.entryTime.isValid() && record.exitTime.isValid() && record.status == "Present") {
            qint64 workedSeconds = record.entryTime.secsTo(record.exitTime);
            qint64 requiredSeconds = settings.workStartTime.secsTo(settings.workEndTime);

            if (workedSeconds > requiredSeconds) {
                qint64 overtimeSeconds = workedSeconds - requiredSeconds;
                int hours = overtimeSeconds / 3600;
                int minutes = (overtimeSeconds % 3600) / 60;
                overtime = QString("+%1h %2m").arg(hours).arg(minutes);

                QTableWidgetItem* overtimeItem = new QTableWidgetItem(overtime);
                overtimeItem->setForeground(QColor(255, 152, 0));
                m_table->setItem(row, 9, overtimeItem);
                continue;
            }
        }
        m_table->setItem(row, 9, new QTableWidgetItem(overtime));

        QColor rowColor = getStatusColor(record.status);
        rowColor.setAlpha(30);
        for (int col = 0; col < m_table->columnCount(); ++col) {
            if (m_table->item(row, col)) {
                m_table->item(row, col)->setBackground(rowColor);
            }
        }
    }

    updateSummary();
}

void AttendanceTable::updateSummary()
{
    int year = m_dateEdit->date().year();
    int month = m_dateEdit->date().month();

    QMap<int, qint64> overtimeMap = m_dbManager->calculateMonthlyOvertime(year, month);
    QMap<int, int> absenceMap = m_dbManager->calculateMonthlyAbsences(year, month);
    QMap<int, int> leaveMap = m_dbManager->calculateMonthlyLeaves(year, month);
    QMap<int, int> forgotExitMap = m_dbManager->calculateMonthlyForgotExits(year, month);

    qint64 totalOvertimeMinutes = 0;
    for (auto it = overtimeMap.begin(); it != overtimeMap.end(); ++it) {
        totalOvertimeMinutes += it.value();
    }

    int totalAbsences = 0;
    for (auto it = absenceMap.begin(); it != absenceMap.end(); ++it) {
        totalAbsences += it.value();
    }

    int totalLeaves = 0;
    for (auto it = leaveMap.begin(); it != leaveMap.end(); ++it) {
        totalLeaves += it.value();
    }

    int totalForgotExits = 0;
    for (auto it = forgotExitMap.begin(); it != forgotExitMap.end(); ++it) {
        totalForgotExits += it.value();
    }

    QString summary = QString(
        "<b>Monthly Summary (%1/%2):</b> "
        "Total Overtime: <span style='color:#ff9800;'>%3h %4m</span> | "
        "Absences: <span style='color:#f44336;'>%5</span> | "
        "Leaves: <span style='color:#2196f3;'>%6</span> | "
        "Forgot Exit: <span style='color:#ff5722;'>%7</span>"
    ).arg(year)
        .arg(month, 2, 10, QChar('0'))
        .arg(totalOvertimeMinutes / 60)
        .arg(totalOvertimeMinutes % 60)
        .arg(totalAbsences)
        .arg(totalLeaves)
        .arg(totalForgotExits);

    m_summaryLabel->setText(summary);
}

void AttendanceTable::refreshTable()
{
    if (m_viewMode->currentText() == "Daily") {
        refreshForDate(m_dateEdit->date());
    }
    else {
        refreshForMonth(m_dateEdit->date().year(), m_dateEdit->date().month());
    }
}

void AttendanceTable::refreshForDate(const QDate& date)
{
    QList<AttendanceRecord> records = m_dbManager->getAttendanceByDate(date);

    QList<Employee> employees = m_dbManager->getAllEmployees();
    for (const auto& emp : employees) {
        bool found = false;
        for (const auto& record : records) {
            if (record.employeeId == emp.id) {
                found = true;
                break;
            }
        }

        if (!found && date.dayOfWeek() <= 5 && date <= QDate::currentDate()) {
            AttendanceRecord absentRecord;
            absentRecord.id = 0;
            absentRecord.employeeId = emp.id;
            absentRecord.employeeName = emp.name;
            absentRecord.date = date;
            absentRecord.status = "Absent";
            absentRecord.entryCount = 0;
            absentRecord.exitCount = 0;
            records.append(absentRecord);
        }
    }

    std::sort(records.begin(), records.end(),
        [](const AttendanceRecord& a, const AttendanceRecord& b) {
            return a.employeeName < b.employeeName;
        });

    populateTable(records);
}

void AttendanceTable::refreshForMonth(int year, int month)
{
    QList<AttendanceRecord> records = m_dbManager->getAttendanceByMonth(year, month);
    populateTable(records);
}

void AttendanceTable::markForgotExits()
{
    QDate yesterday = QDate::currentDate().addDays(-1);
    if (yesterday.dayOfWeek() > 5) {
        return;
    }

    QList<AttendanceRecord> records = m_dbManager->getAttendanceByDate(yesterday);
    for (const auto& record : records) {
        if (record.entryTime.isValid() && !record.exitTime.isValid() &&
            record.status != "Absent" && record.status != "Forgot_Exit") {
            m_dbManager->updateAttendanceStatus(record.id, "Forgot_Exit");
        }
    }
}

void AttendanceTable::searchAttendance()
{
    QString query = m_searchEdit->text().trimmed();
    if (query.isEmpty()) {
        refreshTable();
        return;
    }

    int year = m_dateEdit->date().year();
    int month = m_dateEdit->date().month();

    QList<AttendanceRecord> records = m_dbManager->searchAttendance(query, year, month);
    populateTable(records);
}

void AttendanceTable::exportToCSV()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Export to CSV",
        QString("attendance_%1_%2.csv").arg(m_dateEdit->date().year()).arg(m_dateEdit->date().month()),
        "CSV Files (*.csv)");

    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);

    out << "ID,Employee Name,Date,Entry Time,Exit Time,Status,Entry Count,Exit Count,Work Hours,Overtime\n";

    for (int i = 0; i < m_table->rowCount(); ++i) {
        QStringList row;
        for (int j = 0; j < m_table->columnCount(); ++j) {
            QTableWidgetItem* item = m_table->item(i, j);
            row << (item ? item->text().replace(",", ";") : "");
        }
        out << row.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "Success", "Data exported successfully!");
}

QString AttendanceTable::getStatusDisplay(const QString& status) const
{
    if (status == "Present") return "Present";
    if (status == "Absent") return "Absent";
    if (status == "Forgot_Exit") return "Forgot Exit (Absent)";
    if (status == "Leave") return "Leave";
    if (status == "Multiple_Entry") return "Multiple Entry";
    return status;
}

QColor AttendanceTable::getStatusColor(const QString& status) const
{
    if (status == "Present") return QColor(76, 175, 80);
    if (status == "Absent") return QColor(244, 67, 54);
    if (status == "Forgot_Exit") return QColor(255, 87, 34);
    if (status == "Leave") return QColor(33, 150, 243);
    if (status == "Multiple_Entry") return QColor(156, 39, 176);
    return QColor(0, 0, 0);
}