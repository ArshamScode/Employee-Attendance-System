**语言：**
[English](README.md) | [فارسی](README.fa.md) | [العربية](README.ar.md) | 简体中文

# 安装程序文档

此目录包含用于为 **Employee Attendance System（员工考勤管理系统）** 创建 Windows 安装程序的 Inno Setup 脚本及相关文档。

## 目录内容

* `InstallerScript.iss` — Inno Setup 配置脚本。
* `License.txt` — 安装过程中显示的许可条款。
* `BeforeInstall.txt` — 安装前显示的信息。
* `AfterInstall.txt` — 安装完成后显示的说明。

## 所需环境

* Windows 操作系统
* 与该脚本兼容的 Inno Setup 版本
* 已构建完成的发布版程序 `AttendanceSystem.exe`
* 所有必需的运行时库、Qt 插件、OpenCV 依赖项及模型文件

## 构建安装程序

1. 在 Visual Studio 中使用适当的 Release 配置构建应用程序。
2. 准备应用程序可执行文件及所有必需的运行时依赖项。
3. 使用 Inno Setup 打开 `InstallerScript.iss` 文件。
4. 检查脚本中的所有源文件路径，确保它们指向当前计算机上的正确文件。
5. 确认程序图标、许可文件、安装说明文件、Qt 插件、OpenCV 库和模型文件均位于配置指定的路径中。
6. 在 Inno Setup 中编译脚本，生成安装程序。
7. 在发布之前，先在独立的 Windows 环境中测试生成的安装程序。

## 重要说明

* 在其他计算机上使用该脚本时，可能需要修改文件路径。
* 请勿将员工个人信息、真实人脸照片或私人考勤记录放入公开代码仓库或安装程序中。
* 安装程序必须包含应用程序运行所需的全部依赖项。
* 在分发安装程序之前，请检查 Inno Setup、Qt、OpenCV 以及所有随程序重新分发的第三方文件所适用的许可证。
* 建议通过 GitHub Releases 发布最终安装程序和临时构建文件，而不是直接将它们提交到此目录。

## 免责声明

本文档介绍了预期的应用程序打包流程。安装程序能否成功编译和安装，取决于实际的项目配置、可用依赖项以及文件路径是否正确设置。
