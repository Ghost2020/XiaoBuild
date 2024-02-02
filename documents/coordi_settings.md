# 调度器设置 #

调度器设置 选项卡可进行广泛的全局配置。

> 代理设置

配置适用于连接到该 调度器 的 Agent 的设置。
![](/documents/resource/agent_settings_agents.png)

## **Helper** 的任务要求 ##

可以定义**最小磁盘空间** ，**虚拟内存** ，**物理内存**和**可用 CPU** ，这样 Agent 才有资格被指派为 helper。

可以指定在计算可用 CPU 时应该被忽略的进程。这个选项是为了防止利用闲置 CPU 资源的进程影响这些计算。

* 对于已忽略的进程，也可以根据系统中各个不同的 Agent 进行单独配置。

* 使用此功能时，建议将 **编译任务进程优先级** 设置维持在一个较高水平上，以确保分配至 Agent 的任务能够获得足够的 CPU 时间。


## **代理 Helper** 的权限 ##

在 代理 Settings 中，Agents 可以启用或禁用其在通过其他 Initiators 启动的构建中提供帮助的功能。作为管理员，您可以使用 Allow to Enable/Disable as Helper 选项，在每个 代理 的 Agent List 选项中控制 Agents 是否具有此功能。此设置允许您定义分配给新 Agents 的 Allow to Enable/Disable as Helper 设置的默认设置。


## 许可设置 ##

* 你可以定义将分配给每个 Agent 机器的默认 Helper 核的数量 。

* 您可以在 Release Offline Agents 设置中，定义一个空闲的 Agent 自动失去浮动许可证所需要的时间。

* 您可以定义分配给新 Initiator 和 Helper Agent 的默认许可证。


## Helper 缓存清理 ##

Helper 缓存存储来自其他 agent 文件系统的文件，以便在构建分发时使用。这些文件不需要长期储存。默认情况下，缓存每天清理一次，但可以在这里修改时间表。


> 通用设置

![](/documents/resource/agent_settings_general.png)

修改端口和连接设置

* 在 **Network** 区域，可以修改用于连接到 Coordinator 的任何一个端口。

* 使用复选框，启用与 Coordinator 的所有通信的加密 。

* 在 Check connectivity 设置中定义每个 agent 的状态更新频率。Agent 的状态包括有关项目的信息，如他们的可用核心和 CPU。

系统设置

* 为了确保最大的性能，我们建议你将设置改为 防止服务运行时系统待机。

* 如果收到太多的防火墙信息，可以通过点击 不显示 Windows 防火墙相关消息 禁用它们。

* 我们感谢每一位客户通过分享他们的使用数据帮助我们改进产品和服务 。我们将始终保护您的隐私，绝不会将您的数据用于自我完善以外的任何其他方式。
