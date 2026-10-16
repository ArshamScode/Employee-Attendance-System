<div align="center">

# 员工考勤管理系统

**基于 C++、Qt、OpenCV 和 SQLite 的 Windows 桌面应用**

[English](README.md) · [فارسی](README.fa.md) · [العربية](README.ar.md) · [简体中文](README.zh-CN.md)

</div>

## 项目简介

员工考勤管理系统是一款 Windows 桌面应用，用于管理员工资料、查看考勤事件、配置工作时间规则并导出考勤数据。应用包含基于 OpenCV 人脸检测与图像比较的摄像头工作流。本项目用于软件开发作品集展示，并非经过认证的人力资源、工时记录、安全或生物识别产品。

## 功能特性

- **Attendance 页面：** 显示考勤状态并提供基于摄像头的考勤流程。
- **员工管理：** 创建、查看、更新和删除员工记录，包括个人照片。
- **考勤记录：** 查看和搜索记录，并将表格数据导出为 CSV。
- **工作设置：** 配置工作时间规则和加班计算的最小阈值。
- **本地数据库：** 通过 Qt SQL 使用 SQLite 存储数据。
- **人脸处理流程：** 使用 Haar Cascade 检测人脸，并进行基于图像的比较/训练；识别准确率尚未经过独立评估。
- **Windows 安装程序定义：** 仓库包含 Inno Setup 脚本；安装程序所需的运行文件与源代码分开维护。

## 界面截图

| Attendance | Employees |
|---|---|
| ![考勤页面](assets/screenshots/attendance.png) | ![员工页面](assets/screenshots/employees.png) |

| Records | Settings |
|---|---|
| ![记录页面](assets/screenshots/records.png) | ![设置页面](assets/screenshots/settings.png) |

## 技术栈

- **语言：** C++
- **桌面 UI：** Qt Widgets；项目配置目标为 Qt 5.15.2、MSVC 2019 和 64 位环境。
- **计算机视觉：** OpenCV 4.5.5
- **数据库：** 通过 Qt SQL 使用 SQLite
- **构建环境：** Visual Studio 2019、`v142` 工具集及 Qt/MSBuild 集成
- **目标平台：** Windows x64

Visual Studio 项目文件包含与本地开发环境相关的依赖路径，在其他计算机上可能需要调整。本次仓库整理未验证其能否在干净系统上成功构建。

## 演示登录凭据

以下凭据以硬编码形式存在于应用源代码中，仅用于体验演示版本。这不是安全的身份验证机制。由于仓库是公开的，任何访问者都能读取这些值。请勿使用这些凭据保护真实员工信息或生物识别数据。

| 区域 | 用户名 | 密码 |
|---|---|---|
| Employees | `AS_admin` | `AS_P@ssword-Emp` |
| Records | `AS_admin` | `AS_P@ssword-Rec` |
| Settings | `AS_admin` | `AS_P@ssword-Sett` |

## 从源代码构建

1. 安装 Visual Studio 2019、使用 C++ 的桌面开发工作负载和 Windows SDK。
2. 安装项目所需的 Qt 配置 `5.15.2_msvc2019_64` 及其 Visual Studio 集成工具。
3. 安装 x64 版 OpenCV 4.5.5，并根据本机环境调整 `src/AttendanceSystem/AttendanceSystem.vcxproj` 中的 include 和 library 路径。
4. 在 Visual Studio 中打开 `src/AttendanceSystem/AttendanceSystem.vcxproj`，选择 **Release | x64** 并构建项目。
5. 确保应用能够访问所需的 Qt platform、SQL 和图像格式插件、OpenCV DLL 以及 Haar Cascade 模型。
6. 发布前在干净的 Windows x64 系统上测试构建结果。

仓库保留了原始 Visual Studio 源码结构，并排除了构建产物、本机用户设置和运行时员工数据。

## 安装程序

Inno Setup 定义文件位于 [`installer/InstallerScript.iss`](installer/InstallerScript.iss)。默认安装目录位于当前用户的桌面文件夹下，桌面快捷方式为可选项且默认不勾选。安装程序所需的运行文件未包含在源代码仓库中；本次整理也未编译或测试该脚本。Windows 和 OneDrive 策略可能影响文件写入权限。

脚本会在安装目录下创建 `data/` 和 `data/faces/`，以匹配应用的相对数据库路径 `data/attendance.db`。最终安装包需要测试数据库与图像写入、Qt 插件加载、摄像头访问、卸载以及快捷方式行为。

## 隐私、限制与许可

- 本项目用于作品集展示，不是经过认证的考勤、人力资源、安全或生物识别产品。
- 人脸匹配可能出现误匹配或漏检。重要决策应由人工复核，并提供适当的非生物识别替代方式。
- 仅使用虚构数据或已获得明确授权的数据，并遵守适用的隐私、劳动和生物识别数据法律。
- 本仓库不能证明本地数据库已加密或具备适用于生产环境的访问控制。
- Qt、OpenCV、SQLite、Qt 插件及其他依赖分别具有许可证和声明要求，详见 [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md)。
- 根目录 [`LICENSE`](LICENSE) 是项目专用权利声明，并非 OSI 认可的开源许可证。公开仓库本身不授予复用或再分发原始项目代码的权利。
- 使用 Inno Setup 构建并分发安装程序前，应确认所用版本适用的许可证条款。

## 仓库结构

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

## 项目状态

**作品集项目。** 仓库包含 Visual Studio 源代码及项目配置。界面截图文件和安装程序所需运行文件与源代码包分开存放。安装程序尚未在干净系统上完成编译与验证。

## 作者

**Arsham Sarmast**

