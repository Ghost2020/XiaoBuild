# 网络设置 #

> 常规设置

通过 General 页面，可以设置 Agent 如何与网络中的其他 Incredibuild 组件交互：
![](/documents/resource/agent_settings_network.png)

## 通讯设置 ##

确定用于与其他 Incredibuild 组件通信的 TCP/IP 端口。

## 网络连通性设置 ##

点击该按钮，可打开 [网络连通性设置](http://www.google.com/) 窗口，可帮助查找网络性能较差的 Agent。


> 调度器

**调度器** 页面用于确认运行 Incredibuild 调度器 的计算机和设置 调度器 相关选项：
![](/documents/resource/agent_settings_network_coord.png)


## 调度器位置 ##

定义 调度器 机器的名称/IP 地址及其用于与 Agent 通信的 TCP/IP 端口。如果这是在 Setup 过程中设置的，那么这些值应保持不变，除非 调度器 服务已转移至另一台机器。Browse... 按钮可用于从网络邻居选择计算机。您可以使用 Test 按钮来查看是否能通过显示的设置与 调度器 成功建立连接。

## 选项 ##

**代理描述** 字段可用于提供一个将在 **Coordinator Monitor Agent** 列表中标识此计算机的文本字符串。**代理描述** 将显示在列表中 **Agent** 名称的旁边，这在包含多个 Agent 的环境中非常有用。如果未提供任何值，XiaoBuild 将使用为计算机的网络标识描述字段提供的文本（默认为空）。此字段仅支持英文字符。