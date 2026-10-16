#include "databasemanager.h"
#include <QCoreApplication>

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
    , m_dbPath(QCoreApplication::applicationDirPath() + "/data/attendance.db")
{
    QDir().mkpath(QCoreApplication::applicationDirPath() + "/data");
    QDir().mkpath(QCoreApplication::applicationDirPath() + "/data/faces");
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::initializeDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        logError("Open Database", m_db.lastError());
        return false;
    }

    QSqlQuery query(m_db);
    if (!query.exec("PRAGMA foreign_keys = ON;")) {
        logError("Enable Foreign Keys", query.lastError());
    }

    return createTables();
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_db);

    QString employeesSql = R"(
        CREATE TABLE IF NOT EXISTS employees (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            position TEXT,
            department TEXT,
            phone TEXT,
            photo_path TEXT,
            photo_data BLOB,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    )";

    if (!query.exec(employeesSql)) {
        logError("Create Employees Table", query.lastError());
        return false;
    }

    QString attendanceSql = R"(
        CREATE TABLE IF NOT EXISTS attendance (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            employee_id INTEGER NOT NULL,
            date DATE NOT NULL,
            entry_time TIME,
            exit_time TIME,
            status TEXT DEFAULT 'Present',
            entry_count INTEGER DEFAULT 0,
            exit_count INTEGER DEFAULT 0,
            FOREIGN KEY (employee_id) REFERENCES employees(id) ON DELETE CASCADE
        );
    )";

    if (!query.exec(attendanceSql)) {
        logError("Create Attendance Table", query.lastError());
        return false;
    }

    QString settingsSql = R"(
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL
        );
    )";

    if (!query.exec(settingsSql)) {
        logError("Create Settings Table", query.lastError());
        return false;
    }

    query.exec("CREATE INDEX IF NOT EXISTS idx_attendance_employee_date ON attendance(employee_id, date);");
    query.exec("CREATE INDEX IF NOT EXISTS idx_attendance_date ON attendance(date);");

    return true;
}

int DatabaseManager::addEmployee(const Employee& employee)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO employees (name, position, department, phone, photo_path, photo_data)
        VALUES (:name, :position, :department, :phone, :photo_path, :photo_data)
    )");

    query.bindValue(":name", employee.name);
    query.bindValue(":position", employee.position);
    query.bindValue(":department", employee.department);
    query.bindValue(":phone", employee.phone);
    query.bindValue(":photo_path", employee.photoPath);
    query.bindValue(":photo_data", employee.photoData);

    if (!query.exec()) {
        logError("Add Employee", query.lastError());
        return -1;
    }

    return query.lastInsertId().toInt();
}

bool DatabaseManager::updateEmployee(const Employee& employee)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE employees 
        SET name = :name, position = :position, department = :department,
            phone = :phone, photo_path = :photo_path, photo_data = :photo_data
        WHERE id = :id
    )");

    query.bindValue(":id", employee.id);
    query.bindValue(":name", employee.name);
    query.bindValue(":position", employee.position);
    query.bindValue(":department", employee.department);
    query.bindValue(":phone", employee.phone);
    query.bindValue(":photo_path", employee.photoPath);
    query.bindValue(":photo_data", employee.photoData);

    if (!query.exec()) {
        logError("Update Employee", query.lastError());
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool DatabaseManager::deleteEmployee(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM employees WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        logError("Delete Employee", query.lastError());
        return false;
    }

    return query.numRowsAffected() > 0;
}

Employee DatabaseManager::getEmployee(int id)
{
    Employee employee;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM employees WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        employee.id = query.value("id").toInt();
        employee.name = query.value("name").toString();
        employee.position = query.value("position").toString();
        employee.department = query.value("department").toString();
        employee.phone = query.value("phone").toString();
        employee.photoPath = query.value("photo_path").toString();
        employee.photoData = query.value("photo_data").toByteArray();
        employee.createdAt = query.value("created_at").toDateTime();
    }

    return employee;
}

Employee DatabaseManager::getEmployeeByName(const QString& name)
{
    Employee employee;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM employees WHERE name = :name");
    query.bindValue(":name", name);

    if (query.exec() && query.next()) {
        employee.id = query.value("id").toInt();
        employee.name = query.value("name").toString();
        employee.position = query.value("position").toString();
        employee.department = query.value("department").toString();
        employee.phone = query.value("phone").toString();
        employee.photoPath = query.value("photo_path").toString();
        employee.photoData = query.value("photo_data").toByteArray();
        employee.createdAt = query.value("created_at").toDateTime();
    }

    return employee;
}

QList<Employee> DatabaseManager::getAllEmployees()
{
    QList<Employee> employees;
    QSqlQuery query(m_db);

    if (query.exec("SELECT * FROM employees ORDER BY name")) {
        while (query.next()) {
            Employee emp;
            emp.id = query.value("id").toInt();
            emp.name = query.value("name").toString();
            emp.position = query.value("position").toString();
            emp.department = query.value("department").toString();
            emp.phone = query.value("phone").toString();
            emp.photoPath = query.value("photo_path").toString();
            emp.photoData = query.value("photo_data").toByteArray();
            emp.createdAt = query.value("created_at").toDateTime();
            employees.append(emp);
        }
    }

    return employees;
}

int DatabaseManager::addAttendanceRecord(const AttendanceRecord& record)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO attendance (employee_id, date, entry_time, exit_time, status, entry_count, exit_count)
        VALUES (:employee_id, :date, :entry_time, :exit_time, :status, :entry_count, :exit_count)
    )");

    query.bindValue(":employee_id", record.employeeId);
    query.bindValue(":date", record.date.toString(Qt::ISODate));
    query.bindValue(":entry_time", record.entryTime.isValid() ? record.entryTime.toString(Qt::ISODate) : QVariant());
    query.bindValue(":exit_time", record.exitTime.isValid() ? record.exitTime.toString(Qt::ISODate) : QVariant());
    query.bindValue(":status", record.status);
    query.bindValue(":entry_count", record.entryCount);
    query.bindValue(":exit_count", record.exitCount);

    if (!query.exec()) {
        logError("Add Attendance Record", query.lastError());
        return -1;
    }

    return query.lastInsertId().toInt();
}

bool DatabaseManager::updateAttendanceExitTime(int recordId, const QTime& exitTime)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE attendance 
        SET exit_time = :exit_time, exit_count = exit_count + 1
        WHERE id = :id
    )");

    query.bindValue(":id", recordId);
    query.bindValue(":exit_time", exitTime.toString(Qt::ISODate));

    if (!query.exec()) {
        logError("Update Exit Time", query.lastError());
        return false;
    }

    return true;
}

bool DatabaseManager::updateAttendanceStatus(int recordId, const QString& status)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE attendance SET status = :status WHERE id = :id");
    query.bindValue(":id", recordId);
    query.bindValue(":status", status);

    if (!query.exec()) {
        logError("Update Attendance Status", query.lastError());
        return false;
    }

    return true;
}

AttendanceRecord DatabaseManager::getTodayAttendance(int employeeId)
{
    AttendanceRecord record;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT a.*, e.name as employee_name 
        FROM attendance a 
        JOIN employees e ON a.employee_id = e.id 
        WHERE a.employee_id = :employee_id AND a.date = :date
    )");

    query.bindValue(":employee_id", employeeId);
    query.bindValue(":date", QDate::currentDate().toString(Qt::ISODate));

    if (query.exec() && query.next()) {
        record.id = query.value("id").toInt();
        record.employeeId = query.value("employee_id").toInt();
        record.employeeName = query.value("employee_name").toString();
        record.date = QDate::fromString(query.value("date").toString(), Qt::ISODate);
        record.entryTime = query.value("entry_time").toTime();
        record.exitTime = query.value("exit_time").toTime();
        record.status = query.value("status").toString();
        record.entryCount = query.value("entry_count").toInt();
        record.exitCount = query.value("exit_count").toInt();
    }

    return record;
}

QList<AttendanceRecord> DatabaseManager::getAttendanceByDate(const QDate& date)
{
    QList<AttendanceRecord> records;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT a.*, e.name as employee_name 
        FROM attendance a 
        JOIN employees e ON a.employee_id = e.id 
        WHERE a.date = :date
        ORDER BY e.name
    )");

    query.bindValue(":date", date.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next()) {
            AttendanceRecord record;
            record.id = query.value("id").toInt();
            record.employeeId = query.value("employee_id").toInt();
            record.employeeName = query.value("employee_name").toString();
            record.date = QDate::fromString(query.value("date").toString(), Qt::ISODate);
            record.entryTime = query.value("entry_time").toTime();
            record.exitTime = query.value("exit_time").toTime();
            record.status = query.value("status").toString();
            record.entryCount = query.value("entry_count").toInt();
            record.exitCount = query.value("exit_count").toInt();
            records.append(record);
        }
    }

    return records;
}

QList<AttendanceRecord> DatabaseManager::getAttendanceByMonth(int year, int month)
{
    QList<AttendanceRecord> records;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT a.*, e.name as employee_name 
        FROM attendance a 
        JOIN employees e ON a.employee_id = e.id 
        WHERE strftime('%Y', a.date) = :year AND strftime('%m', a.date) = :month
        ORDER BY a.date, e.name
    )");

    query.bindValue(":year", QString("%1").arg(year, 4, 10, QChar('0')));
    query.bindValue(":month", QString("%1").arg(month, 2, 10, QChar('0')));

    if (query.exec()) {
        while (query.next()) {
            AttendanceRecord record;
            record.id = query.value("id").toInt();
            record.employeeId = query.value("employee_id").toInt();
            record.employeeName = query.value("employee_name").toString();
            record.date = QDate::fromString(query.value("date").toString(), Qt::ISODate);
            record.entryTime = query.value("entry_time").toTime();
            record.exitTime = query.value("exit_time").toTime();
            record.status = query.value("status").toString();
            record.entryCount = query.value("entry_count").toInt();
            record.exitCount = query.value("exit_count").toInt();
            records.append(record);
        }
    }

    return records;
}

QList<AttendanceRecord> DatabaseManager::getAttendanceByEmployee(int employeeId, int year, int month)
{
    QList<AttendanceRecord> records;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT a.*, e.name as employee_name 
        FROM attendance a 
        JOIN employees e ON a.employee_id = e.id 
        WHERE a.employee_id = :employee_id 
        AND strftime('%Y', a.date) = :year AND strftime('%m', a.date) = :month
        ORDER BY a.date
    )");

    query.bindValue(":employee_id", employeeId);
    query.bindValue(":year", QString("%1").arg(year, 4, 10, QChar('0')));
    query.bindValue(":month", QString("%1").arg(month, 2, 10, QChar('0')));

    if (query.exec()) {
        while (query.next()) {
            AttendanceRecord record;
            record.id = query.value("id").toInt();
            record.employeeId = query.value("employee_id").toInt();
            record.employeeName = query.value("employee_name").toString();
            record.date = QDate::fromString(query.value("date").toString(), Qt::ISODate);
            record.entryTime = query.value("entry_time").toTime();
            record.exitTime = query.value("exit_time").toTime();
            record.status = query.value("status").toString();
            record.entryCount = query.value("entry_count").toInt();
            record.exitCount = query.value("exit_count").toInt();
            records.append(record);
        }
    }

    return records;
}

bool DatabaseManager::saveWorkSettings(const WorkSettings& settings)
{
    QSqlQuery query(m_db);

    QVariantMap values = {
        {"work_start_time", settings.workStartTime.toString(Qt::ISODate)},
        {"work_end_time", settings.workEndTime.toString(Qt::ISODate)},
        {"grace_minutes", settings.graceMinutes},
        {"overtime_threshold_minutes", settings.overtimeThresholdMinutes}
    };

    for (auto it = values.begin(); it != values.end(); ++it) {
        query.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (:key, :value)");
        query.bindValue(":key", it.key());
        query.bindValue(":value", it.value().toString());

        if (!query.exec()) {
            logError("Save Settings", query.lastError());
            return false;
        }
    }

    return true;
}

WorkSettings DatabaseManager::loadWorkSettings()
{
    WorkSettings settings;
    QSqlQuery query(m_db);

    if (query.exec("SELECT key, value FROM settings")) {
        while (query.next()) {
            QString key = query.value("key").toString();
            QString value = query.value("value").toString();

            if (key == "work_start_time") {
                settings.workStartTime = QTime::fromString(value, Qt::ISODate);
            }
            else if (key == "work_end_time") {
                settings.workEndTime = QTime::fromString(value, Qt::ISODate);
            }
            else if (key == "grace_minutes") {
                settings.graceMinutes = value.toInt();
            }
            else if (key == "overtime_threshold_minutes") {
                settings.overtimeThresholdMinutes = value.toInt();
            }
        }
    }

    return settings;
}

QList<AttendanceRecord> DatabaseManager::searchAttendance(const QString& searchQuery, int year, int month)
{
    QList<AttendanceRecord> records;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT a.*, e.name as employee_name, e.position, e.department
        FROM attendance a 
        JOIN employees e ON a.employee_id = e.id 
        WHERE (e.name LIKE :query OR e.position LIKE :query OR e.department LIKE :query OR e.phone LIKE :query)
        AND strftime('%Y', a.date) = :year AND strftime('%m', a.date) = :month
        ORDER BY a.date, e.name
    )");

    query.bindValue(":query", "%" + searchQuery + "%");
    query.bindValue(":year", QString("%1").arg(year, 4, 10, QChar('0')));
    query.bindValue(":month", QString("%1").arg(month, 2, 10, QChar('0')));

    if (query.exec()) {
        while (query.next()) {
            AttendanceRecord record;
            record.id = query.value("id").toInt();
            record.employeeId = query.value("employee_id").toInt();
            record.employeeName = query.value("employee_name").toString();
            record.date = QDate::fromString(query.value("date").toString(), Qt::ISODate);
            record.entryTime = query.value("entry_time").toTime();
            record.exitTime = query.value("exit_time").toTime();
            record.status = query.value("status").toString();
            record.entryCount = query.value("entry_count").toInt();
            record.exitCount = query.value("exit_count").toInt();
            records.append(record);
        }
    }

    return records;
}

QMap<int, qint64> DatabaseManager::calculateMonthlyOvertime(int year, int month)
{
    QMap<int, qint64> overtimeMap;
    WorkSettings settings = loadWorkSettings();

    QList<Employee> employees = getAllEmployees();
    for (const auto& emp : employees) {
        QList<AttendanceRecord> records = getAttendanceByEmployee(emp.id, year, month);
        qint64 totalOvertimeMinutes = 0;

        for (const auto& record : records) {
            if (record.status == "Present" && record.exitTime.isValid() && record.entryTime.isValid()) {
                qint64 workedMinutes = record.entryTime.secsTo(record.exitTime) / 60;
                qint64 requiredMinutes = settings.workStartTime.secsTo(settings.workEndTime) / 60;

                if (workedMinutes > requiredMinutes + settings.overtimeThresholdMinutes) {
                    totalOvertimeMinutes += (workedMinutes - requiredMinutes);
                }
            }
        }

        if (totalOvertimeMinutes > 0) {
            overtimeMap[emp.id] = totalOvertimeMinutes;
        }
    }

    return overtimeMap;
}

QMap<int, int> DatabaseManager::calculateMonthlyAbsences(int year, int month)
{
    QMap<int, int> absenceMap;
    QDate startDate(year, month, 1);
    QDate endDate = startDate.addMonths(1).addDays(-1);
    int workingDays = 0;

    for (QDate date = startDate; date <= endDate; date = date.addDays(1)) {
        if (date.dayOfWeek() <= 5) {
            workingDays++;
        }
    }

    QList<Employee> employees = getAllEmployees();
    for (const auto& emp : employees) {
        QList<AttendanceRecord> records = getAttendanceByEmployee(emp.id, year, month);
        int absences = 0;

        for (const auto& record : records) {
            if (record.status == "Absent" || record.status == "Forgot_Exit") {
                absences++;
            }
        }

        QSet<int> recordedDates;
        for (const auto& record : records) {
            recordedDates.insert(record.date.day());
        }

        for (QDate date = startDate; date <= endDate; date = date.addDays(1)) {
            if (date.dayOfWeek() <= 5 && !recordedDates.contains(date.day())) {
                if (date <= QDate::currentDate()) {
                    absences++;
                }
            }
        }

        if (absences > 0) {
            absenceMap[emp.id] = absences;
        }
    }

    return absenceMap;
}

QMap<int, int> DatabaseManager::calculateMonthlyLeaves(int year, int month)
{
    QMap<int, int> leaveMap;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT employee_id, COUNT(*) as leave_count
        FROM attendance 
        WHERE status = 'Leave'
        AND strftime('%Y', date) = :year AND strftime('%m', date) = :month
        GROUP BY employee_id
    )");

    query.bindValue(":year", QString("%1").arg(year, 4, 10, QChar('0')));
    query.bindValue(":month", QString("%1").arg(month, 2, 10, QChar('0')));

    if (query.exec()) {
        while (query.next()) {
            leaveMap[query.value("employee_id").toInt()] = query.value("leave_count").toInt();
        }
    }

    return leaveMap;
}

QMap<int, int> DatabaseManager::calculateMonthlyForgotExits(int year, int month)
{
    QMap<int, int> forgotExitMap;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT employee_id, COUNT(*) as forgot_count
        FROM attendance 
        WHERE status = 'Forgot_Exit'
        AND strftime('%Y', date) = :year AND strftime('%m', date) = :month
        GROUP BY employee_id
    )");

    query.bindValue(":year", QString("%1").arg(year, 4, 10, QChar('0')));
    query.bindValue(":month", QString("%1").arg(month, 2, 10, QChar('0')));

    if (query.exec()) {
        while (query.next()) {
            forgotExitMap[query.value("employee_id").toInt()] = query.value("forgot_count").toInt();
        }
    }

    return forgotExitMap;
}

void DatabaseManager::logError(const QString& operation, const QSqlError& error)
{
    qCritical() << "Database Error in" << operation << ":"
        << error.text();
}

bool DatabaseManager::updateEntryTime(int recordId, const QTime& time)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE attendance SET entry_time = :time, entry_count = entry_count + 1 WHERE id = :id");
    query.bindValue(":id", recordId);
    query.bindValue(":time", time.toString(Qt::ISODate));

    if (!query.exec()) {
        logError("Update Entry Time", query.lastError());
        return false;
    }
    return true;
}