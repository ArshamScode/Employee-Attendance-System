#include <QApplication>
#include <QDir>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Employee Attendance System");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("MyCompany");

    app.setStyle("Fusion");

    app.setStyleSheet(R"(
        /* Main Window & Dialogs */
        QMainWindow, QDialog { background-color: #2b2b2b; }
        
        /* Group Boxes (The borders around sections) */
        QGroupBox {
            border: 1px solid #555555;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 15px;
            font-weight: bold;
            color: #ffffff;
            background-color: #333333;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
            color: #4FC3F7; /* Light blue title */
        }
        
        /* Input Boxes (Text boxes, Number boxes) */
        QLineEdit, QSpinBox, QTimeEdit, QDateEdit, QComboBox {
            background-color: #3a3a3a;
            color: #ffffff;
            border: 1px solid #555555;
            padding: 5px;
            border-radius: 3px;
        }
        QLineEdit:focus, QSpinBox:focus, QTimeEdit:focus {
            border: 1px solid #4FC3F7; /* Blue glow when clicked */
        }
        
        /* Drop-down lists (Fixes the white text on white background bug) */
        QComboBox QAbstractItemView {
            background-color: #3a3a3a;
            color: #ffffff;
            selection-background-color: #4FC3F7;
            selection-color: #000000;
        }
        QComboBox::drop-down {
            border: none;
            background-color: #444444;
        }
        
        /* Buttons */
        QPushButton {
            background-color: #0078D7; /* Windows Blue */
            color: white;
            border: none;
            padding: 5px 10px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #1a8ae8; }
        QPushButton:pressed { background-color: #005a9e; }
        
        /* Tables */
        QTableWidget {
            background-color: #2b2b2b;
            color: #ffffff;
            gridline-color: #444444;
            border: 1px solid #555555;
        }
        QTableWidget::item { padding: 5px; }
        QTableWidget::item:selected { background-color: #0078D7; }
        QHeaderView::section {
            background-color: #3a3a3a;
            color: #ffffff;
            padding: 6px;
            border: 1px solid #555555;
            font-weight: bold;
        }
        
        /* Tabs */
        QTabWidget::pane { border: 1px solid #555555; background-color: #2b2b2b; }
        QTabBar::tab {
            background-color: #3a3a3a;
            color: #cccccc;
            padding: 10px 20px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected { background-color: #2b2b2b; color: #4FC3F7; border-bottom: 2px solid #4FC3F7; }
        
        /* Lists (Like Employee List) */
        QListWidget {
            background-color: #2b2b2b;
            color: #ffffff;
            border: 1px solid #555555;
        }
        QListWidget::item:selected { background-color: #0078D7; }
        
        /* Status Bar */
        QStatusBar { background-color: #333333; color: #ffffff; }
        
        /* Menus */
        QMenuBar { background-color: #333333; color: #ffffff; }
        QMenuBar::item:selected { background-color: #0078D7; }
        QMenu { background-color: #333333; color: #ffffff; border: 1px solid #555555; }
        QMenu::item:selected { background-color: #0078D7; }
        
        /* Scrollbars */
        QScrollBar:vertical {
            background: #2b2b2b; width: 10px; margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #555555; min-height: 20px; border-radius: 5px;
        }
    )");

    QDir().mkpath(QApplication::applicationDirPath() + "/data");
    QDir().mkpath(QApplication::applicationDirPath() + "/data/faces");
    QDir().mkpath(QApplication::applicationDirPath() + "/models");

    QString cascadeDest = QApplication::applicationDirPath() + "/models/haarcascade_frontalface_default.xml";
    if (!QFile::exists(cascadeDest)) {
        QString cascadeSrc = QCoreApplication::applicationDirPath() + "/../data/haarcascades/haarcascade_frontalface_default.xml";
        if (QFile::exists(cascadeSrc)) {
            QFile::copy(cascadeSrc, cascadeDest);
        }
    }

    QFile styleFile(QApplication::applicationDirPath() + "/styles.qss");
    if (styleFile.open(QIODevice::ReadOnly)) {
        QString styleSheet = styleFile.readAll();
        app.setStyleSheet(styleSheet);
        styleFile.close();
    }

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}