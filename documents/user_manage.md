# 用户管理 #

Coordinator 安装的一部分包括创建一个 Grid Admin 用户，该用户具有访问 Coordinator Monitor 用户名和密码的权限。每次访问 Coordinator Manager 时，您都需要登录。如果您在用户界面中处于非活动状态超过 45 分钟，则需要再次登录。

您可以创建和编辑其他用户。每个用户都分配了一个角色，该角色决定了他们在 Coordinator Manager 中的权限等级。

> ## 用户角色 ##
* **Grid Admin**：具有查看和编辑 Coordinator Manager 中所有项目的全部权限。创建的第一个用户将被自动分配 Grid Admin 的角色。

* **Group Manager**：具有编辑 Agent List 中的 Agents 以及查看（但不编辑）Settings 和 License 区域的权限。

* **Viewer**： 具有查看 Agent List 和 License Details 的权限，但不能编辑 Coordinator Manager 中的任何内容。


---
> ## 创建一个新的用户 ##

1. 在 **User Management** 区域中，点击 **Add User**。
![](/documents/resource/user_management.png)

2. 填写 **Role** 和 **Password**，以及有助于管理用户的任何可选详情，然后点击 **Add**。
![](/documents/resource/add_new_user.png)

您还可以使用我们的 **User Management API** 创建新用户。


---
> ## 编辑现有用户 ##

**Grid Admin** 用户可通过点击单个用户旁的编辑图标并修改详情来编辑任何用户的详细信息。
![](/documents/resource/edit_user.png)

**Grid Admin** 用户还可通过使用 Role 下拉列表直接在用户网格中更改用户的角色。
![](/documents/resource/change_role.png)

此外，所有用户均可查看和编辑自己的用户详情和密码。有关详细内容，请见下文。


---
> ## 忘记用户名或密码 ##

非管理员用户

如非管理员用户忘记了其用户名或密码，其管理员可以使用以下程序来设置新密码：

1. 转到 Coordinator Manager > User Management 区域。

2. 选择一个用户，然后单击 Change Password。
![](/documents/resource/change_user_password.png)



管理员用户

如果您的 Grid Admin 用户被锁定，或者忘记密码，请使用以下命令创建新的管理员用户。这必须在 Coordinator 机器上以管理员身份在命令行中完成：