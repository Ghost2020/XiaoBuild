# 代理操作 #

在 Coordinator Manager 的 **Agent List** 里，可以修改设置，如 Build Group 和日志级别。要修改单个 Agent，请使用三点式菜单，或右键单击任一 Agent。
![](/documents/resource/modify_single_agent.png)

* **Enabled as Helper** 暂时阻止该 Agent 的 workload 协助工作，请取消勾选。

* **Allow to Enable/Disable as Helper**： 允许该 Agent 禁用自己为 helper，暂时不协助 workload 的工作。

* **License Configuration**: 可以手动将许可证分配给 Agent。默认情况下，新的 Agent 若为可用状态，会收到一个 helper 可证。

* **Build Cache 配置**：您可以修改 Build Cache 端点的大小和位置。这些也可以在 Agent 本身的 Agent Settings 中修改。

* **Unsubscribe Agent**：从一个代理中删除所有许可证。

* **Agent 描述**：可用于识别 Agents 的自由文本。

* **Assignment Priority**： Coordinator 在分配任务时给予该 Helper Agent 的优先权。具有较高分配优先权的 Agent 将比具有较低优先权的 Agent 更容易被分配任务。

* **Build Priority**：分配至由此 Agent 启动的构建优先级。当资源有限时，具有较高优先级的 initiator 将比具有较低优先级的 agent 分配更多的 helper 。

* **Build Group**：您可以将 Agent 分配到现有的 Build Group 中，该 Build Group 将构建分布限制在指定的 Agent 集合中。详情请见Build Groups 。

* **Log Level**： 日志中包含了多少细节。除非 Incredibuild 支持团队另有要求，该值应设置为最小值，因为详细设置会导致文件非常大。

* **Resize Helper Cache**：helper 缓存让 Helper Agent 重新使用以前构建的数据。

* **Advanced Configuration**：这些选项允许你在一个或多个 agent 上远程改变各种设置。有关详细内容，请见下文。

* **Update Version**：  升级 Coordinator 后，每个 Agent 应自动升级到相同的版本。如果由于任何原因，这种情况没有发生，你可以使用这个选项来远程手动升级一个 Agent。

* **设置描述**：添加 Agent 的自由文本描述以供识别。

* **Clean Helper Cache**：  清理 Helper 缓存。


---
> ## 修改多个 Agent（批量操作）。 ##


你可以将任何行动应用于多个 agent。先过滤 agent 名单，可以加快批量操作的执行速度，如根据 agent 状态分配许可证或构建组。

将鼠标悬停在列表中的某一 Agent 上，查看左边的复选框。勾选相关 Agents，然后点击 Actions 按钮或右击。你所做的任何改变都将应用于所有选定的 Agent。

>提示： 你可以使用标题栏中复选框旁边的下拉菜单来选择特定类型的所有 Agent。

![](/documents/resource/bulk_agent_select.gif)



---
> ## 高级代理配置 ##

这些选项允许你在一个或多个 Agent 上远程改变各种设置。

* **允许并行链接**：允许并行运行多个链接步骤。

* **Avoid local**：在可能的情况下将任务分发给 Helper，即可避免在本地运行任务。

* **Coordinator Port**： 改变连接到 agent 所关联的 Coordinator 所需的端口。

* **Coordinator Server**：改变 agent 与之相关的 Coordinator。

* **Enable MsBuild**： 定义 Visual Studio 构建是否会运行 MSBuild.exe（已启用）或 devenv.exe（已禁用）。

* **Force CPU Count for Initiator**：  运行构建时将使用的 initiator 上的最大 CPU 核心数。这需要重新启动 Incredibuild 的 agent 服务器。

* **Helper Activity Report Timeout**： 如果在这个超时时间段后未能与 initiator 通信，则禁用 helper。默认情况下，超时时间为 40 秒。

* **Max concurrent PDB file instances**：可以并发构建的文件（属于同一项目）的最大数量。0 表示没有限制。

* **Max file sync threads**：用于 Initiator 和 Helper 之间单个文件传输的最大线程数。

* **Max parallel links**：可以并行运行的最大链接步数。需要启用 Allow parallel linking。

* **Only Fail on Local**： 启用后，远程失败的任务将在本地机器上再次重试。

* **Predictive Execution**：启用预执行。

* **Use 64 Bit Toolset**： 使用 Visual Studio 的本地 64 位工具集运行 Visual Studio 构建，而不是默认的 32 位工具集。