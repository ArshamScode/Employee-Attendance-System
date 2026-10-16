**Language:**
English | [فارسی](README.fa.md) | [العربية](README.ar.md) | [简体中文](README.zh-CN.md)

# Installer Documentation

This directory contains the Inno Setup script and supporting documentation used to build the Windows installer for the **Employee Attendance System**.

## Contents

* `InstallerScript.iss` — Inno Setup configuration script.
* `License.txt` — License terms displayed during installation.
* `BeforeInstall.txt` — Information displayed before installation.
* `AfterInstall.txt` — Instructions displayed after installation.

## Requirements

* Windows
* Inno Setup compatible with the script
* A working release build of `AttendanceSystem.exe`
* All required runtime libraries, Qt plugins, OpenCV dependencies, and model files

## Building the Installer

1. Build the application in Visual Studio using the appropriate Release configuration.
2. Prepare the application executable and all required runtime dependencies.
3. Open `InstallerScript.iss` in Inno Setup.
4. Verify that all source paths in the script point to the correct files on your computer.
5. Confirm that the application icon, license file, installation information files, Qt plugins, OpenCV library, and model files are available at their configured locations.
6. Compile the script in Inno Setup.
7. Test the generated installer on a separate Windows environment before distributing it.

## Important Notes

* The installer script may require path adjustments when used on another computer.
* Do not include personal employee information, real face photographs, or private attendance records in a public repository or installer.
* The installer must include all dependencies required by the application.
* Review the applicable licenses for Inno Setup, Qt, OpenCV, and all redistributed third-party files before distributing the installer.
* The final installer executable and temporary build files should generally be published through GitHub Releases rather than committed to this directory.

## Disclaimer

The installer documentation describes the intended packaging process. Successful compilation and installation depend on the actual project configuration, available dependencies, and correctly configured file paths.
