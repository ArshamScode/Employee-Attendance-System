<div align="center">

# Employee Attendance System

**A Windows desktop employee attendance management application built with C++, Qt, OpenCV, and SQLite.**

[English](README.md) · [فارسی](README.fa.md) · [العربية](README.ar.md) · [简体中文](README.zh-CN.md)

</div>

## Overview

Employee Attendance System is a Windows desktop application for managing employee records, reviewing attendance events, configuring work-hour rules, and exporting attendance data. It includes a camera workflow based on OpenCV face detection and image comparison. The project is presented as a software-development portfolio project, not as a certified HR, timekeeping, security, or biometric-identification product.

## Features

- **Attendance dashboard:** attendance status and a camera-based attendance workflow.
- **Employee management:** create, view, update, and remove employee records, including profile photos.
- **Attendance records:** review and search records and export table data to CSV.
- **Work settings:** configure working-hour rules and the minimum overtime threshold.
- **Local database:** SQLite storage accessed through Qt SQL.
- **Face-processing workflow:** Haar Cascade face detection and image-based comparison/training. Recognition accuracy has not been independently evaluated.
- **Windows installer definition:** an Inno Setup script is included; the distributable payload is maintained separately from the source files.

## Screenshots

| Attendance | Employees |
|---|---|
| ![Attendance screen](assets/screenshots/attendance.png) | ![Employees screen](assets/screenshots/employees.png) |

| Records | Settings |
|---|---|
| ![Records screen](assets/screenshots/records.png) | ![Settings screen](assets/screenshots/settings.png) |

## Technology stack

- **Language:** C++
- **Desktop UI:** Qt Widgets; project configuration targets Qt 5.15.2 with MSVC 2019, 64-bit.
- **Computer vision:** OpenCV 4.5.5
- **Database:** SQLite through Qt SQL
- **Build environment:** Visual Studio 2019, `v142` toolset, and Qt/MSBuild integration
- **Target platform:** Windows x64

The Visual Studio project contains environment-specific dependency paths that may require adjustment on another machine. A clean-machine build has not been verified as part of this repository publication.

## Demo credentials

The following credentials are hard-coded in the application source and are listed for navigation of the demonstration build only. They do not provide secure authentication. Because this repository is public, anyone can read these values. Do not use them to protect real employee information or biometric data.

| Area | Username | Password |
|---|---|---|
| Employees | `AS_admin` | `AS_P@ssword-Emp` |
| Records | `AS_admin` | `AS_P@ssword-Rec` |
| Settings | `AS_admin` | `AS_P@ssword-Sett` |

## Build from source

1. Install Visual Studio 2019 with the Desktop development with C++ workload and Windows SDK.
2. Install the Qt configuration expected by the project: `5.15.2_msvc2019_64`, including its Visual Studio integration.
3. Install OpenCV 4.5.5 for x64 and adjust the include and library paths in `src/AttendanceSystem/AttendanceSystem.vcxproj` for the local environment.
4. Open `src/AttendanceSystem/AttendanceSystem.vcxproj` in Visual Studio, select **Release | x64**, and build the project.
5. Make the required Qt platform, SQL, and image-format plugins, OpenCV DLL, and Haar Cascade model available to the application.
6. Test the resulting build on a clean Windows x64 system before distribution.

The original Visual Studio source layout is preserved. Generated build products, machine-specific user settings, and runtime employee data are excluded.

## Installer

The Inno Setup definition is located at [`installer/InstallerScript.iss`](installer/InstallerScript.iss). It uses a default installation directory under the current user's Desktop and provides an optional, unchecked Desktop shortcut. The installer payload is not included in this source repository, and the script has not been compiled or tested as part of this publication. Desktop and OneDrive policies may affect write access.

The script creates `data/` and `data/faces/` under the installation directory to match the application's relative database path, `data/attendance.db`. The final package must be tested to verify database and image writes, Qt plugin loading, camera access, uninstallation, and shortcut behavior.

## Privacy, limitations, and licensing

- This is a portfolio/demo project, not a certified HR, timekeeping, security, or biometric-identification product.
- Face matching may produce false matches or missed detections. Consequential decisions require human review and a suitable non-biometric alternative.
- Use only synthetic or explicitly authorized data and comply with applicable privacy, employment, and biometric-data laws.
- This repository does not establish that the local database is encrypted or protected by production-grade access controls.
- Qt, OpenCV, SQLite, Qt plugins, and other dependencies have separate licensing and notice requirements. See [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).
- The root [`LICENSE`](LICENSE) is a project-specific rights notice, not an OSI-approved open-source license. Public visibility does not grant permission to reuse or redistribute the original project code.
- The applicable Inno Setup license terms must be considered before distributing an installer built with it.

## Repository structure

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
├── assets/screenshots/
├── src/AttendanceSystem/
└── installer/
```

## Project status

**Portfolio project.** The Visual Studio source files and project configuration are included. The installer payload and screenshot image files are separate from this source package. The installer has not been compiled or validated on a clean system.

## Author

**Arsham Sarmast**

