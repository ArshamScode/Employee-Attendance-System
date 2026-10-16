#include "employeemanagement.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileDialog>
#include <QGroupBox>
#include <QLocale>
#include <opencv2/opencv.hpp>

EmployeeDialog::EmployeeDialog(QWidget* parent, int employeeId)
    : QDialog(parent)
    , m_employeeId(employeeId)
{
    setupUI();
    setWindowTitle(employeeId > 0 ? "Edit Employee" : "Add New Employee");
    setMinimumWidth(400);
}

EmployeeDialog::~EmployeeDialog()
{
}

void EmployeeDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QFormLayout* formLayout = new QFormLayout();

    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("Enter full name");
    m_nameEdit->setMaxLength(100);
    formLayout->addRow("Name*:", m_nameEdit);

    m_positionEdit = new QLineEdit();
    m_positionEdit->setPlaceholderText("Enter job position");
    m_positionEdit->setMaxLength(100);
    formLayout->addRow("Position:", m_positionEdit);

    m_departmentEdit = new QLineEdit();
    m_departmentEdit->setPlaceholderText("Enter department");
    m_departmentEdit->setMaxLength(100);
    formLayout->addRow("Department:", m_departmentEdit);

    m_phoneEdit = new QLineEdit();
    m_phoneEdit->setPlaceholderText("Enter phone number");
    m_phoneEdit->setMaxLength(20);
    formLayout->addRow("Phone:", m_phoneEdit);

    mainLayout->addLayout(formLayout);

    QGroupBox* photoGroup = new QGroupBox("Employee Photo");
    QVBoxLayout* photoLayout = new QVBoxLayout(photoGroup);

    m_photoLabel = new QLabel();
    m_photoLabel->setMinimumSize(200, 200);
    m_photoLabel->setMaximumSize(200, 200);
    m_photoLabel->setAlignment(Qt::AlignCenter);
    m_photoLabel->setStyleSheet("background-color: #f0f0f0; border: 2px dashed #999; border-radius: 5px;");
    m_photoLabel->setText("No photo captured");
    photoLayout->addWidget(m_photoLabel, 0, Qt::AlignCenter);

    m_captureBtn = new QPushButton("Capture Photo");
    m_captureBtn->setMinimumHeight(40);
    connect(m_captureBtn, &QPushButton::clicked, this, &EmployeeDialog::capturePhoto);
    photoLayout->addWidget(m_captureBtn);

    mainLayout->addWidget(photoGroup);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setMinimumWidth(100);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelBtn);

    QPushButton* saveBtn = new QPushButton("Save");
    saveBtn->setMinimumWidth(100);
    saveBtn->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");
    connect(saveBtn, &QPushButton::clicked, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Please enter the employee name.");
            return;
        }
        if (m_capturedPhoto.isNull()) {
            QMessageBox::warning(this, "Validation Error", "Please capture the employee photo.");
            return;
        }
        accept();
        });
    buttonLayout->addWidget(saveBtn);

    mainLayout->addLayout(buttonLayout);
}

void EmployeeDialog::capturePhoto()
{
    CaptureDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        m_capturedPhoto = dialog.getCapturedPhoto();
        if (!m_capturedPhoto.isNull()) {
            m_photoLabel->setPixmap(QPixmap::fromImage(m_capturedPhoto));
        }
    }
}

Employee EmployeeDialog::getEmployee() const
{
    Employee emp;
    emp.id = m_employeeId;
    emp.name = m_nameEdit->text().trimmed();
    emp.position = m_positionEdit->text().trimmed();
    emp.department = m_departmentEdit->text().trimmed();
    emp.phone = m_phoneEdit->text().trimmed();
    return emp;
}

QImage EmployeeDialog::getCapturedPhoto() const
{
    return m_capturedPhoto;
}

void EmployeeDialog::setEmployee(const Employee& employee)
{
    m_nameEdit->setText(employee.name);
    m_positionEdit->setText(employee.position);
    m_departmentEdit->setText(employee.department);
    m_phoneEdit->setText(employee.phone);

    if (!employee.photoData.isEmpty()) {
        m_capturedPhoto.loadFromData(employee.photoData);
        m_photoLabel->setPixmap(QPixmap::fromImage(m_capturedPhoto.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }
}

EmployeeManagement::EmployeeManagement(DatabaseManager* dbManager, QWidget* parent)
    : QWidget(parent)
    , m_dbManager(dbManager)
{
    setupUI();
    refreshList();
}

EmployeeManagement::~EmployeeManagement()
{
}

void EmployeeManagement::setupUI()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    QVBoxLayout* listLayout = new QVBoxLayout();

    QLabel* listTitle = new QLabel("Employees");
    listTitle->setStyleSheet("font-size: 16px; font-weight: bold;");
    listLayout->addWidget(listTitle);

    m_employeeList = new QListWidget();
    m_employeeList->setMinimumWidth(250);
    connect(m_employeeList, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        int id = item->data(Qt::UserRole).toInt();
        Employee emp = m_dbManager->getEmployee(id);
        if (emp.id > 0) {
            showEmployeeDetails(emp);
        }
        });
    listLayout->addWidget(m_employeeList);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_addBtn = new QPushButton("Add");
    m_addBtn->setStyleSheet("background-color: #4CAF50; color: white; padding: 8px; font-weight: bold;");
    connect(m_addBtn, &QPushButton::clicked, this, &EmployeeManagement::addEmployee);
    buttonLayout->addWidget(m_addBtn);

    m_editBtn = new QPushButton("Edit");
    m_editBtn->setStyleSheet("background-color: #2196F3; color: white; padding: 8px; font-weight: bold;");
    connect(m_editBtn, &QPushButton::clicked, this, &EmployeeManagement::editEmployee);
    buttonLayout->addWidget(m_editBtn);

    m_deleteBtn = new QPushButton("Delete");
    m_deleteBtn->setStyleSheet("background-color: #f44336; color: white; padding: 8px; font-weight: bold;");
    connect(m_deleteBtn, &QPushButton::clicked, this, &EmployeeManagement::deleteEmployee);
    buttonLayout->addWidget(m_deleteBtn);

    m_refreshBtn = new QPushButton("Refresh");
    m_refreshBtn->setStyleSheet("padding: 8px;");
    connect(m_refreshBtn, &QPushButton::clicked, this, &EmployeeManagement::refreshList);
    buttonLayout->addWidget(m_refreshBtn);

    listLayout->addLayout(buttonLayout);
    mainLayout->addLayout(listLayout);

    QVBoxLayout* detailsLayout = new QVBoxLayout();

    QLabel* detailsTitle = new QLabel("Employee Details");
    detailsTitle->setStyleSheet("font-size: 16px; font-weight: bold;");
    detailsLayout->addWidget(detailsTitle);

    m_detailsLabel = new QLabel();
    m_detailsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_detailsLabel->setStyleSheet("padding: 20px; background-color: #f5f5f5; border-radius: 5px; font-size: 14px;");
    m_detailsLabel->setText("Select an employee to view details");
    detailsLayout->addWidget(m_detailsLabel);

    mainLayout->addLayout(detailsLayout, 1);
}

void EmployeeManagement::refreshList()
{
    m_employeeList->clear();

    QList<Employee> employees = m_dbManager->getAllEmployees();
    for (const auto& emp : employees) {
        QListWidgetItem* item = new QListWidgetItem(
            QString("%1 - %2").arg(emp.name).arg(emp.department)
        );
        item->setData(Qt::UserRole, emp.id);
        m_employeeList->addItem(item);
    }
}

void EmployeeManagement::addEmployee()
{
    EmployeeDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        Employee emp = dialog.getEmployee();
        QImage photo = dialog.getCapturedPhoto();

        QString photoPath = QString("data/faces/%1.jpg").arg(emp.name.replace(" ", "_"));
        photo.save(photoPath, "JPG", 90);

        QByteArray photoBytes;
        QBuffer buffer(&photoBytes);
        buffer.open(QIODevice::WriteOnly);
        photo.save(&buffer, "JPG", 90);

        emp.photoPath = photoPath;
        emp.photoData = photoBytes;

        if (m_dbManager->addEmployee(emp) > 0) {
            QMessageBox::information(this, "Success", "Employee added successfully!");
            refreshList();
            emit employeesChanged();
        }
        else {
            QMessageBox::critical(this, "Error", "Failed to add employee.");
        }
    }
}

void EmployeeManagement::editEmployee()
{
    QListWidgetItem* item = m_employeeList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Warning", "Please select an employee to edit.");
        return;
    }

    int id = item->data(Qt::UserRole).toInt();
    Employee emp = m_dbManager->getEmployee(id);

    if (emp.id <= 0) {
        QMessageBox::critical(this, "Error", "Employee not found.");
        return;
    }

    EmployeeDialog dialog(this, emp.id);
    dialog.setEmployee(emp);

    if (dialog.exec() == QDialog::Accepted) {
        Employee updatedEmp = dialog.getEmployee();
        QImage photo = dialog.getCapturedPhoto();

        QString photoPath = QString("data/faces/%1.jpg").arg(updatedEmp.name.replace(" ", "_"));
        photo.save(photoPath, "JPG", 90);

        QByteArray photoBytes;
        QBuffer buffer(&photoBytes);
        buffer.open(QIODevice::WriteOnly);
        photo.save(&buffer, "JPG", 90);

        updatedEmp.photoPath = photoPath;
        updatedEmp.photoData = photoBytes;
        updatedEmp.id = emp.id;

        if (m_dbManager->updateEmployee(updatedEmp)) {
            QMessageBox::information(this, "Success", "Employee updated successfully!");
            refreshList();
            emit employeesChanged();
        }
        else {
            QMessageBox::critical(this, "Error", "Failed to update employee.");
        }
    }
}

void EmployeeManagement::deleteEmployee()
{
    QListWidgetItem* item = m_employeeList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Warning", "Please select an employee to delete.");
        return;
    }

    int id = item->data(Qt::UserRole).toInt();
    Employee emp = m_dbManager->getEmployee(id);

    if (QMessageBox::question(this, "Confirm Delete",
        QString("Are you sure you want to delete %1?").arg(emp.name),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

        if (m_dbManager->deleteEmployee(id)) {
            if (!emp.photoPath.isEmpty()) {
                QFile::remove(emp.photoPath);
            }
            QMessageBox::information(this, "Success", "Employee deleted successfully!");
            refreshList();
            emit employeesChanged();
        }
        else {
            QMessageBox::critical(this, "Error", "Failed to delete employee.");
        }
    }
}

void EmployeeManagement::showEmployeeDetails(const Employee& emp)
{
    QString html = QString(
        "<h2 style='margin:0;'>%1</h2>"
        "<table style='margin-top:10px; font-size:14px;'>"
        "<tr><td><b>ID:</b></td><td>%2</td></tr>"
        "<tr><td><b>Position:</b></td><td>%3</td></tr>"
        "<tr><td><b>Department:</b></td><td>%4</td></tr>"
        "<tr><td><b>Phone:</b></td><td>%5</td></tr>"
        "<tr><td><b>Created:</b></td><td>%6</td></tr>"
        "</table>"
    ).arg(emp.name)
        .arg(emp.id)
        .arg(emp.position)
        .arg(emp.department)
        .arg(emp.phone)
        .arg(QLocale::system().toString(emp.createdAt, QLocale::ShortFormat));

    if (!emp.photoData.isEmpty()) {
        QString photoBase64 = QString(emp.photoData.toBase64());
        html = QString("<img src='data:image/jpeg;base64,%1' style='float:right; margin-left:15px; border-radius:5px; width:120px; height:120px; object-fit:cover;'/>").arg(photoBase64) + html;
    }

    m_detailsLabel->setText(html);
}

bool EmployeeManagement::saveEmployeeWithPhoto(const Employee& employee, const QImage& photo)
{
    Q_UNUSED(employee)
        Q_UNUSED(photo)
        return true;
}