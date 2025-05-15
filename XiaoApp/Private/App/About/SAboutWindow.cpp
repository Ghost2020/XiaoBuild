/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SAboutWindow.h"
#include "Runtime/Launch/Resources/Version.h"
#include "SlateOptMacros.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Widgets/Layout/SBox.h"
#include "XiaoShare.h"
#include "XiaoLog.h"
#include "../Slate/Const/WeChatQR.h"

#define LOCTEXT_NAMESPACE "SAboutWindow"

static constexpr int32 GAboutWindow_Width = 400;
static constexpr int32 GAboutWindow_Height = 200;

static FText GetEngineSupportInformation()
{
#if PLATFORM_WINDOWS
	return LOCTEXT("CurrentSupport_Text", "\t\t\t\t\tUE4.26;UE4.27\nUE5.0;UE5.1;UE5.2;UE5.3;UE5.4;UE5.5;UE5.6");
#else
	return LOCTEXT("OnlySupportSource_Text", "暂时只支持源码版本");
#endif
}

static FText GetUbaSupportVersion()
{
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
	return FText::FromString(TEXT("33"));
#else
	return FText::FromString(TEXT("33"));
#endif
}

static FString GetArchitecture()
{
#if PLATFORM_CPU_ARM_FAMILY
	return TEXT("arm64");
#else
	return TEXT("x64");
#endif
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAboutWindow::Construct(const FArguments& Args)
{
	XIAO_LOG(Log, TEXT("SAboutWindow::Construct::Begin"));
	GLog->Flush();

	QRBrush = LoadFromBuffer(SWechatQR, TEXT("ChenXiaoXi"));

	SWindow::Construct(SWindow::FArguments()
		.Title(LOCTEXT("WindowTitle", "关于XiaoBuild"))
		.ClientSize(FVector2D(GAboutWindow_Width * 2, GAboutWindow_Height * 2))
		[
			SNew(SHorizontalBox)
#pragma region Left
			+SHorizontalBox::Slot().FillWidth(0.4f)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot().VAlign(VAlign_Fill)
				[
					SNew(SImage).Image(QRBrush.Get())
				]

				+SVerticalBox::Slot().VAlign(VAlign_Bottom).HAlign(HAlign_Center).AutoHeight()
				[
					SNew(STextBlock).Text_Raw(this, &SAboutWindow::GetVersionText)
				]

				+SVerticalBox::Slot().VAlign(VAlign_Bottom).HAlign(HAlign_Center).Padding(0.0f, 0.0f, 0.0f, 10.0f).AutoHeight()
				[
					SNew(STextBlock).Text(LOCTEXT("CopyRight_Text", "Copyright © 2025-至今 XiaoBuild Software Ltd."))
				]
			]
#pragma endregion
#pragma region Right
			+SHorizontalBox::Slot().FillWidth(0.6f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(5.0f, 5.0f)
				[
					SNew(STextBlock).Text(LOCTEXT("UbaSupportInfo_Text", "Uba当前版本"))
				]
				+ SVerticalBox::Slot().Padding(5.0f, 5.0f).HAlign(HAlign_Center).VAlign(VAlign_Center).AutoHeight()
				[
					SNew(SMultiLineEditableText).Text_Static(&GetUbaSupportVersion).AutoWrapText(true)
				]
				+SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(5.0f, 5.0f)
				[
					SNew(STextBlock).Text(LOCTEXT("EngineSupportInfo_Text", "引擎支持版本"))
				]
				+SVerticalBox::Slot().Padding(5.0f, 5.0f).HAlign(HAlign_Center).VAlign(VAlign_Center).AutoHeight()
				[
					SNew(SMultiLineEditableText).Text_Static(&GetEngineSupportInformation).AutoWrapText(true)	
				]
				+SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(5.0f, 5.0f)
				[
					SNew(STextBlock).Text(LOCTEXT("SystemInfo_Text", "当前系统信息"))
				]
				+SVerticalBox::Slot().FillHeight(0.6f).Padding(5.0f, 5.0f)
				[
					SNew(SBorder)
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot().VAlign(VAlign_Top).HAlign(HAlign_Center).AutoHeight()
						[
							SNew(STextBlock).Text_Lambda([] ()
							{
								FString OSVersionLabel, OSSubVersionLabel; 
								FPlatformMisc::GetOSVersions(OSVersionLabel, OSSubVersionLabel);
								return FText::FromString(TEXT("OS::") + OSVersionLabel + TEXT(" ") + OSSubVersionLabel);
							})
						]
						+SVerticalBox::Slot().VAlign(VAlign_Top).HAlign(HAlign_Center).Padding(0.0f, 30.0f, 0.0f, 0.0f).AutoHeight()
						[
							SNew(STextBlock).Text_Lambda([]()
							{
								const FString CpuVendor = FPlatformMisc::GetCPUVendor();
								const FString CPUBrand = FPlatformMisc::GetCPUBrand();
								return FText::FromString(TEXT("Cpu::") + CpuVendor + TEXT(" ") + CPUBrand);
							})
						]
						+ SVerticalBox::Slot().VAlign(VAlign_Top).HAlign(HAlign_Center).AutoHeight().Padding(0.0f, 30.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Arch::%s"), *GetArchitecture())))
						]
						+SVerticalBox::Slot().VAlign(VAlign_Top).HAlign(HAlign_Center).AutoHeight().Padding(0.0f, 30.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock).Text_Lambda([]()
							{
								const FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
								return FText::FromString(FString::Printf(TEXT("RAM::%llu MB "), MemoryStats.TotalPhysical / 1024 / 1024));
							})
						]
					]
				]
				+SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Bottom).Padding(5.0f, 10.0f)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().HAlign(HAlign_Fill)
					[
						SNullWidget::NullWidget
					]
					+SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Right)
					[
						SNew(SButton).Text(LOCTEXT("OkButton_Text", "    OK    "))
						.OnPressed_Lambda([this] ()
						{
							OnExit(SharedThis(this));
						})
					]
				]
			]
#pragma endregion
		]
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.FocusWhenFirstShown(true)
		.SizingRule(ESizingRule::FixedSize)
	);

	OnWindowClosed.BindRaw(this, &SAboutWindow::OnExit);

	XIAO_LOG(Log, TEXT("SAboutWindow::Construct::Finish"));
	GLog->Flush();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FText SAboutWindow::GetVersionText() const
{
	return FText::FromString(GetBuildVersion());
}

void SAboutWindow::OnExit(const TSharedRef<SWindow>& InWindow) const
{
	RequestEngineExit(TEXT("BuildAbout Closed"));
}

#undef LOCTEXT_NAMESPACE
