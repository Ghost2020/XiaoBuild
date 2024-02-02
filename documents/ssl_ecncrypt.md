# SSL和加密 #

> 使用 SSL 证书保护对 XiaoBuild 组件的访问


安装 XiaoBuild 组件（调度器 或 Agents）时，建议您在安装向导中或在静默安装过程中添加自己的 SSL 证书。


> 注意：不支持使用 UNC（例如 \\remote_machine\certs\coordinator.crt）从远程计算机导入文件。

> 注意：如果您不上传自己的证书，XiaoBuild 将使用通用自签名证书来加密通信。但此证书不受信任，并且导致 Chrome 在您每次访问 Coordinator 用户界面时都会发出警告。尽管您每次都可以忽略此警告，但我们建议您使用自己的证书来提高安全性。

> 重要事项：如需在安装后添加或更改证书，请与 support@xiaobuild.com联系。

## 支持格式 ##

支持以下证书和密钥格式。每个证书都有自身专用的私钥。

| **类型**             | **支持格式** |
| -------------------- | ---------- |
| 证书类型              | X.509, PEM   |
| 证书扩展              | .pem, .crt   |
| 证书密钥编码类型       | PKCS#1      |
| 证书密钥扩展           | .key       |


## 调度器 证书验证 ##

每当在 Incredibuild 机器之间启动通信时，Credibuild 都会验证 SSL 证书。在验证 Coordinator 证书时，我们需要验证以下字段：

* 通用名称 (CN)：验证是否与主机 PC 的名称完全匹配。不可以使用通配符表达式。

* 证书颁发机构 (CA)：签署证书的 CA。

* 有效期：验证证书是否未过期。

* 证书吊销列表 (CRL)：如果您没有使用 OCSP，我们会验证该证书是否未在操作系统的 CRL 中列出。

在线证书状态协议 (OCSP)：如果您的证书指定了 OCSP，我们将使用该 OCSP 来验证证书是否未被吊销。为使 OCSP 验证生效，证书必须包括 OCSP 地址，并且 Incredibuild 必须能够与该位置通信（检查您的防火墙）。


## Agent 证书验证 ##

每当在 Incredibuild 机器之间启动通信时，Credibuild 都会验证 SSL 证书。在验证 Agent 证书时，我们需要验证以下字段：

* 通用名称 (CN)：验证是否与主机 PC 的名称匹配。您可以验证使用通用名称的通配符值创建证书，以便为多个 Agents 使用相同的证书（例如 *.example.local）。此外，您可以在安装后使用正则表达式进行验证。有关详细内容，请见下文。

* 证书颁发机构 (CA)：签署证书的 CA。

* 有效期：验证证书是否未过期。

* 证书吊销列表 (CRL)：验证该证书是否未在操作系统的 CRL 中列出。

## 使用正则表达式验证 Agent 名称 ##

您可以使用正则表达式来验证 Agents 的通用名称，而不是直接对照主机 PC 名称检查通用名称。

例如，如果您的所有机器名称都是“Agent123”形式，其中 123 是一个动态数字，那么您可以使用正则表达式来定义该模式。然后，名称与正则表达式匹配的任何 Agent 均将在证书验证过程中进行验证。

正则表达式必须少于 999 个字符。

示例：

验证任何以 rnd1.example.com 结尾的名称：

[a-zA-Z]*[.-]?(rnd\d{1})[.-]?[a-zA-Z]*[.-]?[a-zA-Z]*

验证任何以 rnd*.example.com. 结尾的名称。

这意味着 john.rnd12.example.com 和 dan.rnd3.example.com 将有效，而 george.sales.example.com 将无效。

[a-zA-Z]*[.-]?(rnd[0-9]*)[.-]?[a-zA-Z]*[.-]?[a-zA-Z]*

如需使用正则表达式进行 Agent 验证，请转到 Coordinator Monitor > Coordinator Settings > General > Network 区域，并添加有效的正则表达式。

![](/documents/ssl_regex.png)



> 加密通信

无论您是否上传了自己的证书，您都可以对 Incredibuild 内部组件之间的通信进行加密。默认情况下，通信不加密，因为 Incredibuild 机器通常位于同一环境中。如需对通信进行加密，请选中 Coordinator Monitor >Coordinator Settings > General > Network 区域中的 Encrypted communication 框，并指定安全端口来管理通信。

![](/documents/ssl_encrypt.png)


> 限制：与 Backup Coordinators 的所有通信均未加密。