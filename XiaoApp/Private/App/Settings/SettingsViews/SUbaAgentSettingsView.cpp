/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#include "SUbaAgentSettingsView.h"

#include "ShareDefine.h"
#include "SlateOptMacros.h"
#include "HAL/PlatformMisc.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SSpinBox.h"
#include "../../Slate/Widgets/SWarningBox.h"

#include "XiaoAgent.h"
#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "SUbaAgentSettingsView"

static const FText SWarningTitle = LOCTEXT("WarningTitle_Text", "警告");

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SUbaAgentSettingsView::Construct(const FArguments& InArgs)
{	
	HardwareDesc = QueryComputerConfiguration();
	
	ChildSlot
	[
		SNew(SBorder)
		.Padding(5.0)
		[
			SNew(SScrollBox).Orientation(Orient_Vertical)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				
				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SWarningBox)
					.Message(LOCTEXT("AgentDir_Tool", "如需修改UbaAgent缓存数据的存储目录，需要将XiaoAgentService服务停止"))
					.Visibility_Lambda([this]() {
						return IsAppRunning(XiaoAppName::SUbaAgent) ? EVisibility::Visible : EVisibility::Collapsed;
					})
				]
				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock)
							.Text(LOCTEXT("StoreFolder_Text", "存储目录"))
							.ToolTipText(LOCTEXT("StoreFolder_ToolTipText", "Agent缓存数据存放位置"))
					]
					+ SHorizontalBox::Slot().L_PADDING(10)
					[
						SAssignNew(SaveDirFolderText, SEditableTextBox)
							.IsEnabled_Raw(this, &SUbaAgentSettingsView::GetCanChangeDir)
							.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type InType) {
							const auto& Str = InText.ToString();
							if (!FPaths::DirectoryExists(Str))
							{
								SaveDirFolderText->SetError(TEXT("目录不存在"));
								SModifiedAgentSettings.UbaAgent.Dir = TEXT("");
							}
							else
							{
								SaveDirFolderText->SetError(TEXT(""));
								ChangeDir(Str);
							}
						})
						.Text_Lambda([this]() {
							return FText::FromString(SModifiedAgentSettings.UbaAgent.Dir);
						})
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SButton)
							.Text(LOCTEXT("Explorer_Text", "..."))
							.IsEnabled_Raw(this, &SUbaAgentSettingsView::GetCanChangeDir)
							.OnPressed_Lambda([this]() {
							if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
							{
								if (FString OutFolder; DesktopPlatform->OpenDirectoryDialog(
									FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
									LOCTEXT("StoreFolder_FileDesc", "打开存储文件夹...").ToString(),
									TEXT(""),
									OutFolder
								))
								{
									ChangeDir(OutFolder);
									SaveDirFolderText->SetText(FText::FromString(OutFolder));
								}
							}
						})
					]
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("ConfigPath_Text", "配置文件")).ToolTipText(LOCTEXT("ConfigPath_ToolTipText", "Config file that contains options for various systems"))
					]

					+ SHorizontalBox::Slot().L_PADDING(10)
					[
						SAssignNew(ConifgPathText, SEditableTextBox)
						.OnTextCommitted_Lambda([](const FText& InText, ETextCommit::Type InType){
							if (!FPaths::FileExists(InText.ToString()))
							{
								FMessageDialog::Open(EAppMsgType::Type::Ok, LOCTEXT("ConfigPathWaning_Text", "目录无效!"), SWarningTitle);
							}
						})
						.Text_Lambda([this](){
							return FText::FromString(SModifiedAgentSettings.UbaAgent.Config);
						})
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SButton)
						.Text(LOCTEXT("ConfigPathExplorer_Text", "..."))
						.OnPressed_Lambda([this](){
							if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
							{
								if (TArray<FString> OutFiles; DesktopPlatform->OpenFileDialog(
									FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
									LOCTEXT("ConfigPath_FileDesc", "打开配置文件...").ToString(),
									TEXT(""),
									SModifiedAgentSettings.UbaAgent.Config,
									TEXT("*.json"),
									0,
									OutFiles
								))
								{
									if (OutFiles.Num() > 0)
									{
										ConifgPathText->SetText(FText::FromString(OutFiles[0]));
										SModifiedAgentSettings.UbaAgent.Config = OutFiles[0];
									}
								}
							}
						})
					]
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(STextBlock).Text(LOCTEXT("MaxCpu_Text", "可协助的最大进程数"))
					]

					+ SHorizontalBox::Slot()
					[
						SAssignNew(MaxCpuSpinBox, SSpinBox<uint32>).MinValue(0).MaxValue(FPlatformMisc::NumberOfCoresIncludingHyperthreads())
						.OnValueChanged_Lambda([](const uint32 InValue)
							{
								SModifiedAgentSettings.UbaAgent.MaxCpu = InValue;
								SModifiedAgentSettings.UbaAgent.MaxWorkers = InValue;
							})
						.Value_Lambda([]()
							{
								return SModifiedAgentSettings.UbaAgent.MaxCpu;
							}
						)
					]
				]
				
				V_ADD_SPINBOX(SModifiedAgentSettings.UbaAgent.Mulcpu, 1, 16, LOCTEXT("Mulcpu_Text", "该值与cpu数量相乘，得出最大cpu。默认为1"), FText::GetEmpty())
				V_ADD_SPINBOX(SModifiedAgentSettings.UbaAgent.MaxCon, 1, FPlatformMisc::NumberOfCoresIncludingHyperthreads(), LOCTEXT("MaxCon_Text", "代理可以接受的最大连接数"), FText::GetEmpty())
				V_ADD_SPINBOX(SModifiedAgentSettings.UbaAgent.Capacity, 1, 1024 * 1024, LOCTEXT("Capacity_Text", "本地存储能力"), FText::GetEmpty())
				V_ADD_SPINBOX(SModifiedAgentSettings.UbaAgent.SendSize, (256 * 1024), (1024 * 1024), LOCTEXT("SendSize_Text", "从客户端发送到服务器的消息的最大大小"), FText::GetEmpty())
				V_ADD_SPINBOX(SModifiedAgentSettings.UbaAgent.MemWait, 50, 100, LOCTEXT("MemWait_Text", "Mem Wait"), LOCTEXT("MemWait_ToolTipText", "The amount of memory needed to spawn a process. Set this to 100 to disable. Defaults to 95%"))
				V_ADD_SPINBOX(SModifiedAgentSettings.UbaAgent.MemKill, 70, 100, LOCTEXT("MemKill_Text", "Mem Kill"), LOCTEXT("MemKill_ToolTipText", "The amount of memory needed before processes starts to be killed. Set this to 100 to disable. Defaults to 95%"))

				V_ADD_SPINBOX(SModifiedAgentSettings.UbaAgent.Stats, 1, 65535, LOCTEXT("Stats_Text", "Stats"), LOCTEXT("Stats_ToolTipText", "Print stats for each process if higher than threshold"))

				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bVerbose, LOCTEXT("Verbose_Text", "将调试信息打印到控制台"))
				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bLog, LOCTEXT("Log_Text", "将所有绕过信息的进程记录到文件中（仅适用于调试版本）"))
				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bNoCustoMalloc, LOCTEXT("NoCustoMalloc_Text", "禁用进程的自定义分配器。如果你看到奇怪的碰撞，可以进行测试"))
				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bStoreRaw, LOCTEXT("bStoreRaw_Text", "禁用存储压缩。这将使用更多的存储空间，并可能提高性能"))
				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bSendRaw, LOCTEXT("bSendRaw_Text", "禁用发送压缩。这将使用更多的带宽，但占用更少的cpu"))
				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bNoPoll, LOCTEXT("bNoPoll_Text", "No Poll"))
				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bNoStore, LOCTEXT("bNoStore_Text", "不使用存储来存储文件（二进制文件等少数例外）"))
				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bResetStore, LOCTEXT("bResetStore_Text", "删除所有cas"))
				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bQuiet, LOCTEXT("bQuiet_Text", "不输出任何登录控制台"))
				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bBinaryVersion, LOCTEXT("bBinaryVersion_Text", "将使用二进制文件作为版本。这将导致每次主机端二进制文件更改时都会进行更新"))
				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bSummary, LOCTEXT("SummaryCheckBox_Text", "在会话结束时打印摘要"))
				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bSentry, LOCTEXT("bSentryCheckBox_Text", "启用哨兵"))
				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bNoProxy, LOCTEXT("NoProxyCheckBox_Text", "不允许此代理作为其他代理的存储代理"))
				V_ADD_CHECKBOX(SModifiedAgentSettings.UbaAgent.bKillRandom, LOCTEXT("KillRandom_Text", "杀死随机进程并退出会话"))

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("Crypto_Text", "Crypto")).ToolTipText(LOCTEXT("Crypto_ToolTipText", "32 character (16 bytes) crypto key used for secure network transfer"))
					]

					+ SHorizontalBox::Slot().L_PADDING(10)
					[
						SAssignNew(CryptoText, SEditableTextBox)
						.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type InType){
							const FString CryptStr = InText.ToString();
							if (CryptStr.Len() == 32)
							{
								SModifiedAgentSettings.UbaAgent.Crypto = CryptStr;
							}
							else
							{
								SModifiedAgentSettings.UbaAgent.Crypto = TEXT("");
								CryptoText->SetError(TEXT("不满足32位！"));
							}
						})
						.Text_Lambda([this]()
						{
							return FText::FromString(SModifiedAgentSettings.UbaAgent.Crypto);
						})
					]
				]
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FString SUbaAgentSettingsView::QueryComputerConfiguration()
{
	const bool b64Bit = FPlatformMisc::Is64bitOperatingSystem();
	LogicThreadNum = GetLogicProcessorNum();
	const auto ProcessDesc = FPlatformMisc::GetProcessorGroupDesc();
	const uint32 ProcessorNum =  ProcessDesc.NumProcessorGroups;
	const FString CpuBrand = FPlatformMisc::GetCPUBrand();
	const FString CpuChipset = FPlatformMisc::GetCPUChipset();
	const FString CpuVendor = FPlatformMisc::GetCPUVendor();
	const int32 NumberOfWorker = FPlatformMisc::NumberOfWorkerThreadsToSpawn();

	return FString::Format(TEXT("{0} CPU,{1} Logic Core, {2}Hyperthreading"), {ProcessorNum, LogicThreadNum, FPlatformProcess::SupportsMultithreading() ? TEXT("") : TEXT("No ")});
}

bool SUbaAgentSettingsView::GetCanChangeDir() const
{
	return !IsAppRunning(XiaoAppName::SUbaAgent);
}

void SUbaAgentSettingsView::ChangeDir(const FString& InNewDir)
{
	FString& Dir = SModifiedAgentSettings.UbaAgent.Dir;
	if(!Dir.IsEmpty() && FPaths::DirectoryExists(Dir))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.DeleteDirectoryRecursively(*Dir);
	}
	Dir = InNewDir;
}

#undef LOCTEXT_NAMESPACE
