# 系统要求 #

> ## 硬件 ##

| **安装类型**   | **CPU** | **RAM**  | **存储空间**   | **网络** |
| ------------  | --- | ---- | --------- | -------------- |
| 调度器         | 4核 | 4GB  | 10GB(SSD) | 静态IP(1Gbps)  |
| 发起机器       | 4核 | 4GB  | 30GB(SSD) | 1Gbps          |
| 协助机器       | 4核 | 4GB  | 10GB(SSD) | 1Gbps          |
| 构建缓存主机   | 8核 | 16GB | 50GB(SSD) | 10Gbps         |

---

> ## 端口 ##
* 代理机器或者调度器机器的防火墙和网络设备（路由器）上必须打开以下端口（详情如下）。所有端口号均可在安装过程中和 Coordinator Manager 进行修改。

| **端口号** | **机器的必要条件** | **备注** |
| ------------ | ---------- | -----------|
|  8000        | 协调器      |
|  37000       | 代理        | 启用已安装的代理和协调器之间的通信
|  37020       | 代理        | 每个内核需要一个端口，每个代理都有一个Helper内核许可证
|  38000       | 许可证服务端口 | 这是与许可证服务进行通信的必要条件。仅用于 Coordinator 机器内部 Incredibuild 组件之间的通信。
| SSH          | 22         |

---
> ## 网络 ##

* **网络带宽:** 带宽包括上传和下载速度。最低 100 Mbps，建议 1 Gbps 或以上，对于 Build Cache 端点，明显更高（见上表）。

* **位置:** 机器需要在同一个物理局域网（LAN）内，或者通过虚拟个人网络（VPN）连接（要求与 LAN 相同）。

* **调度器:** Coordinator 需要有一个静态 IP 或者 DNS 识别的主机名。

* **许可证激活:** 需要 Coordinator 机器之间的通信
    和 https://lvep.xiaobuild.com:443（我们的许可证验证服务）。
    * 代理管理：如果使用代理来管理本地机器的出站通信，则应定义一个名为 HTTPS_PROXY 的 windows 环境变量，并将代理的 URL 作为 Coordinator 机器上的值。这允许我们通过代理路由流量。代理需要有一个静态 IP 或者 DNS 识别的主机名。


---
> ## 杀毒软件 ##
在 Initiator Agent 机器的杀毒软件上，应将如下位置排除：
* XiaoBuild 安装文件夹。

* 保存将要执行的源代码的文件夹。

* 对于 Visual Studio – 当 XiaoBuild 与 Visual Studio 使用时，建议将 Visual Studio 安装文件夹排除在外。如果未将整个文件夹排除在外，则必须执行如下 Visual Studio 可执行程序：“devenv.exe”、“devenv.com”和“MSBuild.exe”。


---
> ## 存储空间 ##
* Initiator 机器上，源文件和输出目录必须使用 SATA 或者 NVMe 固态存储。

* 另一个影响所需存储空间的因素是 IncrediBuild 在 Build History 数据库中保存的构建的数量。您保存的构建数量越大，则需要的储存空间越大。您可以在 Max 构建中设置此值，以便在 Agent Settings 对话框中保存至 DB 参数。


---
> ## 操作系统 ##
* 支持的运行系统：

    * Windows: 8.1, 10, 11

    * Windows 服务器：2012 R2、2016、2019、2022

    * 使用本地 Build Cache 的 Coordinator、Build Cache 端点和 Initiator 需要 64 位。

* 权限： 安装需要 Power 用户权限或更高的权限。