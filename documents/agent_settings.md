# 代理设置 #

> 常规设置
![](/documents/resource/agent_general.jpg)

## 代理服务 ##
使用 代理服务 中的 Start、Stop 或 Restart 按钮，以启动、停止或重新启动 XiaoBuild 的 代理服务（“AgentService.exe”）。此部分还会显示 XiaoBuild 代理服务 的当前状态（“正在运行”、“正在停止”、“已停止”或“正在启动”）。

## 构建历史 ##
构建历史 设置与任务栏图标“构建历史...”命令结合使用。/command

## 将构建保留在历史记录中的天数 ##
设置 XiaoBuild 保存以往构建的天数，用于在 构建历史 窗口中进行显示。

## 清除构建历史 ##
删除 构建历史 存储中所有已保存构建。


> 偏好设置
![](/documents/resource/agent_preference.jpg)

## 系统启动 ##
确定 Agent 在机器启动时是否已启用。以下选项可用：

Enable Agent–Agent 始终以“Enabled”状态启动。
Disable Agent–Agent 始终以“Disabled”状态启动。
Keep last Agent state–Agent 以上次系统关闭时所处的有效状态启动。

## 启用 Agent 计划 ##
允许在预定义的时间内自动启用和禁用 Agent。举例而言，对于在标准工作时间之外启用 Agent，此功能很有用。

## 版本更新 ##
**执行远端启动的版本更新之前，要求用户进行确认。**

## Windows 防火墙 ##
**不要显示与 Windows 防火墙相关的消息。**
选择此选项，以防止 XiaoBuild 建议在此计算机的 MS Windows 防火墙中打开 XiaoBuild 端口。

## CPU利用率 ##
![](/documents/resource/agent_settings_helper_cpu0.png)

CPU 利用率 - CPU 作为 Initiator 时

选择此选项，您即可覆盖 XiaoBuild 在启动构建时检测并报告给 Coordinator 的逻辑处理器数量。

在某些情况下（特别是在具有多核和/或超线程处理器的机器中），XiaoBuild 可能会列出多种可能的硬件配置选择。请选择最能恰当描述您机器硬件的一种。

Utilize x logical core(s)– 在下拉列表中选择“用户定义”时，XiaoBuild 在该机器上所使用的自定义核心数量可由此设置进行显式设定。

* 在注册许可证中，须将“X cores”扩展包分配至 Agent，以便其在构建中使用其他处理器或核心。

* 通过将“Utilize x logical cores”设置从较高值降低到“1”，从 Agent 处重新分配“Multiple CPU(s)/Core(s)”或“Agent + X cores”扩展包。如需恢复使用多个处理器，则您需要使用 Coordinator 应用程序将扩展包重新分配至 Agent。
CPU 利用率 - CPU 作为 Helper 时
您可以限制机器上有助于来自其他 Initiator 的构建的内核数。

此设置可能与 Coordinator Monitor > Agent List > 许可配置 中的类似设置冲突。Coordinator Manager 中的设置主要由网格管理者用来管理许可证。Helper 机器所有者可以通过 Agent 设置限制其使用情况。这两个设置的较低值将决定特定 Agent 可以使用的最大内核数。

在某些情况下，当您的 Agent 用作 Helper 时，您有可能想要使用不同的核心数量。要实现此需求，只需在选择框中定义想要的核心数量即可。
