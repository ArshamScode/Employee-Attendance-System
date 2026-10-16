# Repository structure

```text
.
├── README.md
├── README.fa.md
├── README.ar.md
├── README.zh-CN.md
├── LICENSE
├── .gitignore
├── .gitattributes
├── .editorconfig
├── THIRD_PARTY_NOTICES.md
├── SECURITY.md
├── CONTRIBUTING.md
├── PROJECT_STRUCTURE.md
├── assets/
│   └── screenshots/
├── src/
│   └── AttendanceSystem/
│       ├── AttendanceSystem.vcxproj
│       ├── AttendanceSystem.vcxproj.filters
│       ├── AttendanceSystem.rc
│       ├── AttendanceSystem.ico
│       ├── mainwindow.qrc
│       ├── mainwindow.ui
│       ├── Header Files/
│       ├── Source Files/
│       ├── Resource Files/
│       ├── models/
│       └── data/
└── installer/
    ├── InstallerScript.iss
    ├── AttendanceSystem.ico
    ├── License.txt
    ├── BeforeInstall.txt
    ├── AfterInstall.txt
    ├── files/
    └── output/
```

Visual Studio build output, machine-specific user settings, runtime databases, employee face data, and generated installer packages are excluded from the source repository.
