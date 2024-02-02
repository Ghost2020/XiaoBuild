# 构建群组 #

构建群组 允许将构建分布限制在一个选定的 Agent 组中。构建群组 包含 Initiator Agent 和 Helper Agent。默认情况下，每个代理都被分配到 Default 构建群组。可以在 Coordinator 的 Agent 列表中把 Agent 分配到不同的 构建群组。

您可以在 Coordinator Monitor > Agent List's 构建群组 栏中查看每个 agent 的 构建群组。
![](/documents/resource/build_group_in_agent_list.png)


---
> ## 创建或编辑 构建群组 ##

1. 在 Coordinator 的 **代理试图** 中，点击 **构建群组** 按钮。
![](/documents/resource/build_groups_manage_button.png)

2. 点击 **添加构建群组** 创建新的 构建群组。
![](/documents/resource/build_groups_add.jpg)

1. 在名称字段内点击并修改，以更改 构建群组 名称。
![](/documents/resource/build_groups_edit.jpg)


---
> ## 删除 构建群组 ##

如需删除现有 构建群组，点击 构建群组，然后使用待删除的 构建群组 旁的“回收站”按钮。
![](/documents/resource/build_groups_delete.jpg)



---
> ## 使用 构建群组 中的所有代理 ##

当您将 XiaoBuild 扩展到云时，您可以使 XiaoBuild 在您的云提供商中创建和使用 XiaoBuild Agent。为更好地管理这些资源，您可以定义 Build Group 中的 Initiator Agent 何时可以使用您的 Cloud Helper Agent。默认情况下，此设置允许您的所有 Initiator Agent 使用您的 Cloud Helper Agent。
![](/documents/resource/build_groups_enable_cloud.jpg)



---
> ## 将 Agent 分配至 构建群组 ##

1. 在 Coordinator 的 Agent List中，点击 Agent 旁的三点菜单，点击 Build Group，并选择 Build Group 名称。你可以使用搜索框来缩小结果范围
![](/documents/resource/build_groups_assign.png)

2. 要一次分配多个 agent，选择 agent 并使用 Actions 菜单选项。
![](/documents/resource/bulk_agent_select2.gif)



---
> ## 查看 构建群组 中的所有代理 ##

1. 在 Agent List 中，点击 Filters。
![](/documents/resource/filters_button.png)

2. 展开 Build Groups 部分，并选择你想显示的 Build Groups。
![](/documents/resource/build_groups_filter.png)

