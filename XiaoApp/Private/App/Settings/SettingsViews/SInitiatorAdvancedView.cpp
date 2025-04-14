/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */


#include "SInitiatorAdvancedView.h"
#include "ShareDefine.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"

#include "XiaoAgent.h"
#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "InitiatorAdvanced"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SInitiatorAdvancedView::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.Padding(5.0)
		[
			SNew(SScrollBox).Orientation(Orient_Vertical)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().FIR_PADDING
				.VAlign(VAlign_Top)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Performance_Text", "性能"))
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock)
							.Text(LOCTEXT("StoreFolder_Text", "存储目录"))
							.ToolTipText(LOCTEXT("StoreFolder_ToolTipText", "The directory used to store data"))
					]

					+ SHorizontalBox::Slot().L_PADDING(10)
					[
						SAssignNew(SaveDirFolderText, SEditableTextBox)
						.IsEnabled_Raw(this, &SInitiatorAdvancedView::GetCanChangeDir)
						.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type InType){
							const auto& Str = InText.ToString();
							if (!FPaths::DirectoryExists(Str))
							{
								SaveDirFolderText->SetError(TEXT("目录不存在"));
								SModifiedAgentSettings.UbaScheduler.Dir = TEXT("");
							}
							else
							{
								SaveDirFolderText->SetError(TEXT(""));
								SModifiedAgentSettings.UbaScheduler.Dir = Str;
							}
						})
						.Text_Lambda([]()
						{
							return FText::FromString(SModifiedAgentSettings.UbaScheduler.Dir);
						})
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SButton)
						.Text(LOCTEXT("Explorer_Text", "..."))
						.IsEnabled_Raw(this, &SInitiatorAdvancedView::GetCanChangeDir)
						.OnPressed_Lambda([this](){
							if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
							{
								if (FString OutFolder; DesktopPlatform->OpenDirectoryDialog(
									FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
									LOCTEXT("StoreFolder_FileDesc", "打开存储文件夹...").ToString(),
									TEXT(""),
									OutFolder
								))
								{
									SModifiedAgentSettings.UbaScheduler.Dir = OutFolder;
									SaveDirFolderText->SetText(FText::FromString(OutFolder));
								}
							}
						})
					]
				]

				V_ADD_CHECKBOX(LogCheckBox, SModifiedAgentSettings.UbaScheduler.bLog, LOCTEXT("bLog_Text", "将所有进程detouring信息记录到文件中（仅适用于调试版本）"))
				V_ADD_CHECKBOX(StandaloneCheckBox, SModifiedAgentSettings.UbaScheduler.bStandalone, LOCTEXT("bStandalone_Text", "是否以单机模式运行，不适用代理进行联合编译"))
				V_ADD_CHECKBOX(QuietCheckBox, SModifiedAgentSettings.UbaScheduler.bQuiet, LOCTEXT("bQuiet_Text", "除了错误之外，不会在控制台中输出任何日志记录"))
				V_ADD_CHECKBOX(CheckCasCheckBox, SModifiedAgentSettings.UbaScheduler.bCheckCas, LOCTEXT("bCheckCas_Text", "检查所有 cas 条目是否正确"))
				V_ADD_CHECKBOX(DeletecasCheckBox, SModifiedAgentSettings.UbaScheduler.bDeletecas, LOCTEXT("bDeletecas_Text", "删除 cas db"))
				V_ADD_CHECKBOX(GetCasCheckBox, SModifiedAgentSettings.UbaScheduler.bGetCas, LOCTEXT("bGetCas_Text", "将打印应用程序的Hash"))
				V_ADD_CHECKBOX(SummaryCheckBox, SModifiedAgentSettings.UbaScheduler.bSummary, LOCTEXT("bSummary_Text", "在会话结束时打印摘要"))
				V_ADD_CHECKBOX(NoCustoMallocCheckBox, SModifiedAgentSettings.UbaScheduler.bNoCustoMalloc, LOCTEXT("bNoCustoMalloc_Text", "禁用进程的自定义分配器。如果您看到奇怪的崩溃，可以进行测试"))
				V_ADD_CHECKBOX(AllowMemoryMapsCheckBox, SModifiedAgentSettings.UbaScheduler.bAllowMemoryMaps, LOCTEXT("bAllowMemoryMaps_Text", "Allow Memory Maps"))
				V_ADD_CHECKBOX(EnableStdOutCheckBox, SModifiedAgentSettings.UbaScheduler.bEnableStdOut, LOCTEXT("bEnableStdOut_Text", "启用进程标准输出"))
				V_ADD_CHECKBOX(StoreRawCheckBox, SModifiedAgentSettings.UbaScheduler.bStoreRaw, LOCTEXT("bStoreRaw_Text", "禁用存储压缩。这将使用更多存储空间并可能提高性能"))
				V_ADD_CHECKBOX(VisualizerCheckBox, SModifiedAgentSettings.UbaScheduler.bVisualizer, LOCTEXT("bVisualizer_Text", "启动一个可视化进度的可视化工具"))
				
				V_ADD_SPINBOX(CapacitySpinBox, SModifiedAgentSettings.UbaScheduler.Capacity, 10, 1024 * 1024, LOCTEXT("Capacity_Text", "本地存储容量"), LOCTEXT("Capacity_ToolTipText", "..."))
				V_ADD_SPINBOX(MaxLocalCoreSpinBox, SModifiedAgentSettings.UbaScheduler.MaxLocalCore, 0, FPlatformMisc::NumberOfCoresIncludingHyperthreads(), LOCTEXT("MaxLocalCore_Text", "最大本地Workers数量由运行"), LOCTEXT("MaxLocalCore_ToolTip", "本地发起时，本地最多提供的运行的进程数量，此数值能够调节本机的运行负载"))
				V_ADD_SPINBOX(MaxConSpinBox, SModifiedAgentSettings.UbaScheduler.MaxCon, 0, 1024, LOCTEXT("MaxCon_Text", "最大可以连接协助的代理数量"), LOCTEXT("MaxCon_ToolTip", "本地发起时，可以向网络请求的最大协助机器数量"))
				V_ADD_SPINBOX(MaxCpuSpinBox, SModifiedAgentSettings.UbaScheduler.MaxCpu, 1, 65535, LOCTEXT("MaxCpuCore_Text", "可以调动的最大进程数"), LOCTEXT("MaxCpuCore_ToolTip", "本地发起时，最大能够调动的网络计算资源(逻辑核心数量)"))

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock)
							.Text(LOCTEXT("Crypto_Text", "Crypto"))
							.ToolTipText(LOCTEXT("Crypto_ToolTipText", "32 character (16 bytes) crypto key used for secure network transfer"))
					]

					+ SHorizontalBox::Slot().L_PADDING(10)
					[
						SAssignNew(CryptoText, SEditableTextBox)
						.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type InType)
							{
								const FString CryptStr = InText.ToString();
								if (CryptStr.Len() == 32)
								{
									SModifiedAgentSettings.UbaScheduler.Crypto = CryptStr;
								}
								else
								{
									SModifiedAgentSettings.UbaScheduler.Crypto = TEXT("");
									CryptoText->SetError(TEXT("不满足32位！"));
								}
							})
						.Text_Lambda([this]()
							{
								return FText::FromString(SModifiedAgentSettings.UbaScheduler.Crypto);
							})
					]
				]

				+ SVerticalBox::Slot().FIR_PADDING
				[
					SNew(STextBlock)
						.Text(LOCTEXT("BuildHistory_Text", "构建历史"))
				]
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

bool SInitiatorAdvancedView::GetCanChangeDir() const
{
	return !IsAppRunning(XiaoAppName::SXiaoScheduler);
}

#undef LOCTEXT_NAMESPACE
