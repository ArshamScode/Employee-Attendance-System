#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDateTime>
#include <QGroupBox>
#include <QSplitter>
#include <QApplication>
#include <opencv2/opencv.hpp>
#include "smartrecognition.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_dbManager(new DatabaseManager(this))
    , m_cameraWidget(new CameraWidget(this))
    , m_employeeManagement(new EmployeeManagement(m_dbManager, this))
    , m_attendanceTable(new AttendanceTable(m_dbManager, this))
    , m_tabWidget(new QTabWidget(this))
    , m_statusBarLabel(new QLabel(this))
    , m_clockLabel(new QLabel(this))
    , m_clockTimer(new QTimer(this))
    , m_endOfDayTimer(new QTimer(this))
{
    if (!m_dbManager->initializeDatabase()) {
        QMessageBox::critical(this, "Error", "Failed to initialize database!");
        exit(1);
    }

    initializeDefaultSettings();
    setupUI();
    setupMenuBar();
    setupStatusBar();

    connect(m_cameraWidget, &CameraWidget::faceRecognized, this, &MainWindow::handleFaceRecognized);
    connect(m_employeeManagement, &EmployeeManagement::employeesChanged, this, &MainWindow::onEmployeesChanged);

    connect(m_clockTimer, &QTimer::timeout, this, &MainWindow::updateClock);
    m_clockTimer->start(1000);
    updateClock();

    connect(m_endOfDayTimer, &QTimer::timeout, this, &MainWindow::markEndOfDayAbsences);
    m_endOfDayTimer->start(60000);

    m_cameraWidget->startCamera();
    m_attendanceTable->markForgotExits();

    setWindowTitle("Employee Attendance System");
    setMinimumSize(1200, 800);
    showMaximized();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    m_tabWidget->addTab(createAttendanceTab(), "Attendance");
    m_tabWidget->addTab(m_employeeManagement, "Employees");
    m_tabWidget->addTab(m_attendanceTable, "Records");

    m_tabWidget->setDocumentMode(true);
    mainLayout->addWidget(m_tabWidget);
    setCentralWidget(centralWidget);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
}

QWidget* MainWindow::createAttendanceTab()
{
    QWidget* tab = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(tab);

    QGroupBox* cameraGroup = new QGroupBox("Face Recognition Camera");
    QVBoxLayout* cameraLayout = new QVBoxLayout(cameraGroup);
    cameraLayout->addWidget(m_cameraWidget);

    QHBoxLayout* cameraControls = new QHBoxLayout();

    QPushButton* startBtn = new QPushButton("Start Camera");
    connect(startBtn, &QPushButton::clicked, [this]() { m_cameraWidget->startCamera(); });
    cameraControls->addWidget(startBtn);

    QPushButton* stopBtn = new QPushButton("Stop Camera");
    connect(stopBtn, &QPushButton::clicked, [this]() { m_cameraWidget->stopCamera(); });
    cameraControls->addWidget(stopBtn);

    QPushButton* trainBtn = new QPushButton("Train Model");
    trainBtn->setStyleSheet("background-color: #9C27B0; color: white; font-weight: bold;");
    connect(trainBtn, &QPushButton::clicked, this, &MainWindow::trainFaceModel);
    cameraControls->addWidget(trainBtn);

    cameraLayout->addLayout(cameraControls);
    layout->addWidget(cameraGroup);

    QGroupBox* statusGroup = new QGroupBox("Today's Status");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);

    QLabel* infoLabel = new QLabel(
        "<h3>Instructions:</h3>"
        "<ul>"
        "<li>1st scan: <b style='color:green;'>ENTRY</b></li>"
        "<li>2nd scan: <b style='color:orange;'>EXIT</b></li>"
        "<li>3rd scan: <b style='color:green;'>ENTRY</b></li>"
        "<li>4th scan: <b style='color:orange;'>EXIT</b></li>"
        "</ul>"
        "<p><b>Note:</b> If employee doesn't exit, marked as <b style='color:red;'>ABSENT</b></p>"
    );
    infoLabel->setWordWrap(true);
    statusLayout->addWidget(infoLabel);

    QLabel* activityTitle = new QLabel("<b>Recent Activity:</b>");
    statusLayout->addWidget(activityTitle);

    QListWidget* activityList = new QListWidget();
    activityList->setMaximumHeight(300);
    connect(this, &MainWindow::logMessage, [activityList](const QString& msg) {
        if (!msg.isEmpty()) {
            activityList->insertItem(0, QTime::currentTime().toString("HH:mm:ss") + " - " + msg);
            if (activityList->count() > 50) delete activityList->takeItem(activityList->count() - 1);
        }
        });
    statusLayout->addWidget(activityList);

    layout->addWidget(statusGroup, 1);
    return tab;
}

void MainWindow::setupMenuBar()
{
    QMenuBar* menuBar = this->menuBar();

    QMenu* fileMenu = menuBar->addMenu("File");
    QAction* settingsAction = fileMenu->addAction("Settings");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);
    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    QMenu* toolsMenu = menuBar->addMenu("Tools");
    QAction* trainAction = toolsMenu->addAction("Train Face Model");
    connect(trainAction, &QAction::triggered, this, &MainWindow::trainFaceModel);

    QAction* markAbsentAction = toolsMenu->addAction("Mark Forgot Exits");
    connect(markAbsentAction, &QAction::triggered, [this]() {
        m_attendanceTable->markForgotExits();
        m_attendanceTable->refreshTable();
        QMessageBox::information(this, "Done", "Forgot exits marked.");
        });

    QMenu* helpMenu = menuBar->addMenu("Help");
    QAction* aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::setupStatusBar()
{
    statusBar()->addWidget(m_statusBarLabel, 1);
    statusBar()->addPermanentWidget(m_clockLabel);
    m_statusBarLabel->setText("Ready");
}

void MainWindow::updateClock()
{
    m_clockLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss dddd"));
}

void MainWindow::handleFaceRecognized(int employeeId, double confidence)
{
    if (employeeId <= 0) {
        m_cameraWidget->setStyleSheet("border: 3px solid red;");
        QTimer::singleShot(5000, m_cameraWidget, [this]() { m_cameraWidget->setStyleSheet(""); });
        emit logMessage("Unknown face detected!");
        m_statusBarLabel->setText("Unknown face detected");
        m_statusBarLabel->setStyleSheet("color: red; font-weight: bold;");
        QTimer::singleShot(5000, this, [this]() { m_statusBarLabel->setStyleSheet(""); m_statusBarLabel->setText("Ready"); });
        return;
    }

    Employee emp = m_dbManager->getEmployee(employeeId);
    if (emp.id <= 0) {
        m_cameraWidget->setStyleSheet("border: 3px solid red;");
        QTimer::singleShot(5000, m_cameraWidget, [this]() { m_cameraWidget->setStyleSheet(""); });
        emit logMessage("Employee not found in database!");
        return;
    }

    int result = processAttendance(employeeId);
    double conf = 100.0 - confidence;

    if (result == 1) {
        m_cameraWidget->setStyleSheet("border: 5px solid green; background-color: rgba(0, 255, 0, 0.1);");
        QTimer::singleShot(5000, m_cameraWidget, [this]() { m_cameraWidget->setStyleSheet(""); });
        QString msg = QString("ENTRY: %1 (Conf: %2%)").arg(emp.name).arg(conf, 0, 'f', 1);
        emit logMessage(msg);
        m_statusBarLabel->setText(msg);
        m_statusBarLabel->setStyleSheet("color: green; font-weight: bold; background-color: #e8f5e9; padding: 5px;");
    }
    else if (result == 2) {
        m_cameraWidget->setStyleSheet("border: 5px solid yellow; background-color: rgba(255, 255, 0, 0.1);");
        QTimer::singleShot(5000, m_cameraWidget, [this]() { m_cameraWidget->setStyleSheet(""); });
        QString msg = QString("EXIT: %1 (Conf: %2%)").arg(emp.name).arg(conf, 0, 'f', 1);
        emit logMessage(msg);
        m_statusBarLabel->setText(msg);
        m_statusBarLabel->setStyleSheet("color: orange; font-weight: bold; background-color: #fff8e1; padding: 5px;");
    }
    else {
        m_cameraWidget->setStyleSheet("border: 5px solid red;");
        QTimer::singleShot(5000, m_cameraWidget, [this]() { m_cameraWidget->setStyleSheet(""); });
        emit logMessage("Error processing attendance for " + emp.name);
    }

    QTimer::singleShot(5000, this, [this]() { m_statusBarLabel->setStyleSheet(""); m_statusBarLabel->setText("Ready"); });
    if (m_tabWidget->currentIndex() == 2) m_attendanceTable->refreshTable();
}

int MainWindow::processAttendance(int employeeId)
{
    QTime now = QTime::currentTime();
    if (!ensureTodayAttendanceRecord(employeeId)) return -1;

    AttendanceRecord record = m_dbManager->getTodayAttendance(employeeId);

    if (record.entryCount == record.exitCount) {
        if (record.entryCount == 0) m_dbManager->updateAttendanceStatus(record.id, "Present");
        m_dbManager->updateEntryTime(record.id, now);
        return 1;
    }
    else {
        m_dbManager->updateAttendanceExitTime(record.id, now);
        return 2;
    }
}

bool MainWindow::ensureTodayAttendanceRecord(int employeeId)
{
    AttendanceRecord existing = m_dbManager->getTodayAttendance(employeeId);
    if (existing.id > 0) return true;

    Employee emp = m_dbManager->getEmployee(employeeId);
    if (emp.id <= 0) return false;

    AttendanceRecord newRecord;
    newRecord.employeeId = employeeId;
    newRecord.employeeName = emp.name;
    newRecord.date = QDate::currentDate();
    newRecord.status = "Present";
    newRecord.entryCount = 0;
    newRecord.exitCount = 0;

    return m_dbManager->addAttendanceRecord(newRecord) > 0;
}

void MainWindow::onEmployeesChanged()
{
    trainFaceModel();
    m_attendanceTable->refreshTable();
}

void MainWindow::trainFaceModel()
{
    QList<Employee> employees = m_dbManager->getAllEmployees();
    if (employees.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No employees to train.");
        return;
    }

    QString cascadePath = QApplication::applicationDirPath() + "/models/haarcascade_frontalface_default.xml";
    if (!QFile::exists(cascadePath)) {
        QMessageBox::critical(this, "Error", "Face detection file missing!\nPlease ensure 'haarcascade_frontalface_default.xml' is inside the 'models' folder next to the .exe file.");
        return;
    }

    QMap<int, std::vector<cv::Mat>> facesData;
    SmartRecognition tempRecognizer;
    tempRecognizer.initialize(cascadePath);

    int totalFaces = 0;
    int failedFaces = 0;

    for (const auto& emp : employees) {
        if (emp.photoData.isEmpty()) continue;

        QImage photo;
        if (!photo.loadFromData(emp.photoData)) continue;

        try {
            cv::Mat mat = tempRecognizer.qImageToMat(photo);
            if (mat.empty()) continue;

            cv::Mat processedFace = tempRecognizer.extractFaceSafely(mat);

            if (!processedFace.empty()) {
                facesData[emp.id].push_back(processedFace);
                totalFaces++;
            }
            else {
                failedFaces++;
            }
        }
        catch (const cv::Exception& e) {
            qWarning() << "OpenCV Error processing photo for" << emp.name << ":" << e.what();
            failedFaces++;
            continue;
        }
        catch (...) {
            failedFaces++;
            continue;
        }
    }

    if (facesData.isEmpty()) {
        QMessageBox::critical(this, "Training Failed",
            "No valid faces were detected in the employee photos.\n\n"
            "Please make sure:\n"
            "1. The photo is clear and well-lit.\n"
            "2. The face is looking directly at the camera.\n"
            "3. The face is centered and not cut off at the edges.");
        return;
    }

    if (m_cameraWidget->trainModel(facesData)) {
        QString modelPath = QApplication::applicationDirPath() + "/data/facemodel.yml";
        if (m_cameraWidget->m_faceRecognizer->saveModel(modelPath)) {
            QString msg = QString("Model trained successfully!\nFaces processed: %1").arg(totalFaces);
            if (failedFaces > 0) {
                msg += QString("\n\nWarning: %1 photo(s) failed to detect a face.").arg(failedFaces);
            }
            QMessageBox::information(this, "Success", msg);
        }
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to train model.");
    }
}

void MainWindow::showSettings()
{
    SettingsDialog dialog(m_dbManager, this);
    if (dialog.exec() == QDialog::Accepted) m_attendanceTable->refreshTable();
}

void MainWindow::markEndOfDayAbsences()
{
    if (QTime::currentTime().hour() == 23 && QTime::currentTime().minute() == 59) {
        m_attendanceTable->markForgotExits();
        m_attendanceTable->refreshTable();
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "About", "<h2>Employee Attendance System</h2><p>Version 1.0</p><p>Built with Qt and OpenCV</p>");
}

void MainWindow::initializeDefaultSettings()
{
    WorkSettings existing = m_dbManager->loadWorkSettings();
    if (existing.workStartTime.isNull()) m_dbManager->saveWorkSettings(WorkSettings());
}

void MainWindow::onTabChanged(int index)
{
    if (index == 1) {
        if (!authenticate("AS_admin", "AS_P@ssword-Emp")) {
            m_tabWidget->blockSignals(true);
            m_tabWidget->setCurrentIndex(m_lastTabIndex);
            m_tabWidget->blockSignals(false);
            return;
        }
    }
    else if (index == 2) {
        if (!authenticate("AS_admin", "AS_P@ssword-Rec")) {
            m_tabWidget->blockSignals(true);
            m_tabWidget->setCurrentIndex(m_lastTabIndex);
            m_tabWidget->blockSignals(false);
            return;
        }
    }

    m_lastTabIndex = index;
}

bool MainWindow::authenticate(const QString& expectedUser, const QString& expectedPass)
{
    QDialog dialog(this);
    dialog.setWindowTitle("Admin Authentication");
    dialog.setModal(true);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->addWidget(new QLabel("Enter Admin Credentials:"));

    QLineEdit* userEdit = new QLineEdit();
    userEdit->setPlaceholderText("Username");
    layout->addWidget(userEdit);

    QLineEdit* passEdit = new QLineEdit();
    passEdit->setPlaceholderText("Password");
    passEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (userEdit->text() == expectedUser && passEdit->text() == expectedPass) {
            return true;
        }
        else {
            QMessageBox::warning(this, "Access Denied", "Incorrect username or password.");
            return false;
        }
    }
    return false;
}