# XiaoBuild

![BuildInsight](./documents/resource/BuildInsight.gif)

## Introduction

> This software is primarily developed based on UE5.5's UBA, designed for the Unreal Engine development process. It aims to enhance user efficiency and experience in source code compilation and material compilation as well as Lightmass swarm, providing the standard functionalities of IncrediBuild.

---

## Version Information

Current Version：v1.1.1
Release Date: ：2025-10-12

---

## Engine Support

| Engine Version     | Source Compilation | Material Compilation  | Lightmass Swarm
|--------------------|--------------------|-----------------------|----------------|
| 4_26    | ✔ [▶️](https://www.bilibili.com/video/BV1rSbKz5EqT/?share_source=copy_web&vd_source=13d934120a59d113eda5c8f73bff2c65)      |  ✔    |
| 4_27    | ✔ [▶️](https://www.bilibili.com/video/BV1N6bKzHEoX/?share_source=copy_web&vd_source=13d934120a59d113eda5c8f73bff2c65)       | ✔ [▶️](https://www.bilibili.com/video/BV1Beu4z7E9U/?share_source=copy_web&vd_source=13d934120a59d113eda5c8f73bff2c65) | 🕒 |
| 5_0     | ✔ [▶️](https://youtu.be/H8qjjH1VbRo?si=YAnu5DE9qh3JFa5M&t=73)      | ✔  | ✔ |
| 5_1     | ✔ [▶️](https://youtu.be/Id_Khmcypw0?si=yDGeibHtfjDIBHyH&t=191)     | ✔  | ✔ |
| 5_2     | ✔ [▶️](https://youtu.be/gRwpNMGWrb4?si=EIRa0z3PNzHSqsWE&t=167)     | ✔  | ✔ |
| 5_3     | ✔ [▶️](https://youtu.be/6dNMNT_D8Ts?si=JITDGahkYgP6MSVj&t=56)      | ✔  | ✔ |
| 5_4     | ✔ [▶️](https://youtu.be/WxD754CTsPE?si=-EcJWkgddUdlma5j&t=95)      | ✔  | ✔ |
| 5_5     | ✔ [▶️](https://youtu.be/QL_PEHftOOs?si=-QcaSyq6IM7EdPRT&t=152)     | ✔  | ✔ [▶️](https://www.bilibili.com/video/BV1pghCzWEEF/?share_source=copy_web&vd_source=13d934120a59d113eda5c8f73bff2c65) |
| 5_6     | ✔       | ✔       | ✔ |
| 5_7     | ✔       | ✔       | ✔ |

## Table of contents

- [XiaoBuild](#xiaobuild)
  - [Introduction](#introduction)
  - [Version Information](#version-information)
  - [Engine Support](#engine-support)
  - [Table of contents](#table-of-contents)
  - [Features](#features)
  - [System Requirements](#system-requirements)
  - [Installation Guide](#installation-guide)
  - [User Manual](#user-manual)
    - [1.Installing and Configuring the Tool](#1installing-and-configuring-the-tool)
    - [2.Tray Tool](#2tray-tool)
    - [3.Real-time Build Insight Tool](#3real-time-build-insight-tool)
    - [4.Build Coordinator Tool](#4build-coordinator-tool)
    - [5.Agent Settings Tool](#5agent-settings-tool)
  - [QA](#qa)
    - [The project gets stuck at 45% during material compilation, and the progress does not change for a long time?](#the-project-gets-stuck-at-45-during-material-compilation-and-the-progress-does-not-change-for-a-long-time)
    - [Sometimes the agent exceeds the maximum number of concurrent processes set by the system?](#sometimes-the-agent-exceeds-the-maximum-number-of-concurrent-processes-set-by-the-system)
    - [Backup Machine Setup](#backup-machine-setup)
  - [Changelog](#changelog)
    - [v1.0.9](#v109)
  - [License Agreement](#license-agreement)
  - [Contact Us](#contact-us)

---

## Features

Main functionalities of the software:

- Unified compilation of Unreal Engine source code and materials as well as Lightmass
- Does not require UE Engine installation for operation
- Real-time monitoring of source code and material compilation
- Configurable system and agent parameters to dynamically allocate and release computing resources, maximizing resource utilization
- Supports Unreal Engine versions from 4.26 to 5.6, with one-click installation for both source-built and launcher (binary) versions.
- User-friendly interface, requiring no specialized knowledge

Complete UE5_1 build process: 
https://www.bilibili.com/video/BV1dCr3YqExS/?spm_id_from=333.999.0.0&vd_source=e4b1cc7dae1637f7c09704300ea43634

---

## System Requirements

Description of the software's runtime environment and hardware requirements:

- **Operating System**：x64 Windows 10/11
  
- **Hardware Requirements**：

  - CPU: i5
  - Memory：4GB
  - Storage: Recommended 10GB+ SSD

- **System Architecture**：
  XiaoBuild consists of two machine types: Agent and Coordinator.

![architecture](./documents/resource/architecture.png)

---

## Changelog

Documenting software version updates:

### v1.1.1

- Basic functionality equivalent to IncrediBuild

---

## License Agreement

A brief description of the software license, or a link to the full license agreement:

> This software follows the MIT License. For details, please [refer to](.COPYRIGHT).

---

## Contact Us

Providing user support contact information:

- **Email**：cxx2020@outlook.com
- **Twitter**: Ghost202081
- **QQ**：794569465
- **QQ Group**：910420853
- **WeChat**：c794569465
![WeChat](./documents/resource/wechat_qr_code.jpg)
