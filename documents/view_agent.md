# 查看 Agent #

您可以在 Coordinator Manager 的 Agent 名单中，查看每个 Agent 的详细信息。
![](/documents/resource/agent_view.png)


---
> ## 许可证的使用 ##

查看目前已分配的许可证数量，等待分配（可用）的数量，或冷却的数量。每个固定 initiator 许可证每 24 小时只能分配一次。每次分配后，许可证被认为是冷却了 24 小时。

如果看不到 License Usage 区域，请点击 Show Status Bar。
![](/documents/resource/show_status_bar.png)


---
> ## 在线 Agent 活动 ##

获取在线 Agent 的高规格图片：

* 目前有多少台 Agent 机器在线

* 目前有多少个 Helper Core（包括云）和 Initiator Core 参与构建

* 目前有多少个构建正在使用 XiaoBuild 运行。
![](/documents/resource/online_agent_activity.png)


---
> ## 查看 Agent 细节和状态 ##

Agent List 显示了每个 Agent 的信息，如状态、可用 CPU 和描述。点击 Manage Columns 以在列表中添加或删除列。某些栏仅在启用 Cloud 时可用。
![](/documents/resource/agent_list.png)

您可以通过在搜索栏中键入字符串来筛选选项列表：
![](/documents/resource/manage_columns.png)


彩色圆圈表示每个 Agent 的状态：

![](/documents/resource/ready_status.png) **Ready**： Agent 可以运行一个构建。

![](/documents/resource/initiating_status.png) **Initiating**： Initiator Agent 目前正在运行一个构建。您会看到 "Running"一词和构建的名称。

![](/documents/resource/helping_status.png) **Helping**： Helper Agent 目前正在协助运行一个建设

![](/documents/resource/offline_status.png) **Offline**： Agent 没有连接到 Coordinator。

![](/documents/resource/stop_status.png) **停止**：暂停时从您池中退出的 Cloud Agent 。

![](/documents/resource/updating_status.png) **Updating**： Agent 正在安装最新的版本。

![](/documents/resource/error_status.png) **错误**：由于错误，Agent无法运行。这可能是版本不兼容错误或 Agent 名称重复错误。



---
> ## 筛选 Agent 名单 ##

你可以通过在搜索栏中输入，使用快速过滤下拉菜单，或使用过滤按钮来过滤 Agent 名单。搜索栏会扫描以下属性：Agent 名称、Build Group、IP、路由 IP、Mac 地址、当前构建标题和和登录用户
![](/documents/resource/agent_list_filters.png)

过滤按钮让你根据状态和可用的 CPU 等项目来过滤 Agent 列表。右边有黑色标签的过滤器显示目前正在过滤哪些项目。
