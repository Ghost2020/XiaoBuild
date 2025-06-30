/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SSetupOptions.h"
#include "SlateOptMacros.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "SInstallView.h"
#include "SReadyToUpdate.h"
#include "SUninstallView.h"
#include "ShareWidget.h"
#include "XiaoShare.h"
#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "SSetupOptions"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSetupOptions::Construct(const FArguments& InArgs)
{
	SelectionContainer = SNew(SVerticalBox);

	bool bInstallAready = false;
#if PLATFORM_WINDOWS || PLATFORM_UNIX
	const FString XiaoHome = FPlatformMisc::GetEnvironmentVariable(*SXiaoHome);
	bInstallAready = !XiaoHome.IsEmpty() && FPaths::DirectoryExists(XiaoHome);
#elif PLATFORM_MAC
	bInstallAready = FPaths::DirectoryExists(TEXT("/Applications/XiaoApp.app/Contents/UE/Engine"));
#endif

#pragma region GuiInstall
	SelectionContainer->AddSlot().AutoHeight().SEC_PADDING
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().HAlign(HAlign_Left).AutoWidth()
				[
					SAssignNew(InstallCheck, SCheckBox)
					.IsChecked(ECheckBoxState::Checked)
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
					{
						if (InState == ECheckBoxState::Checked)
						{
							CreateSilentCheck->SetIsChecked(ECheckBoxState::Unchecked);
							UninstallCheck->SetIsChecked(ECheckBoxState::Unchecked);

							GInstallSettings.SetupUpType = EInstallType::IT_Install;
							bCanNext = true;
						}
						else
						{
							InstallCheck->SetIsChecked(true);
						}
					})
				]
				+SHorizontalBox::Slot().HAlign(HAlign_Left).AutoWidth()
				[
					SNew(STextBlock).Text(LOCTEXT("InstallXiaoBuild_Text", "安装Xiaobuild"))
				]
			]
			+SVerticalBox::Slot().Padding(20.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("InstallXiaoBuildAgent_Text", "安装Xiaobuild代理，构建管理器，协调器或者备用协调器"))
			]
		]
	];
#pragma endregion
	
#pragma region CommandInstall
	SelectionContainer->AddSlot().AutoHeight().SEC_PADDING
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().HAlign(HAlign_Left).AutoWidth()
				[
					SAssignNew(CreateSilentCheck, SCheckBox)
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState) 
					{
						if (InState == ECheckBoxState::Checked)
						{
							InstallCheck->SetIsChecked(ECheckBoxState::Unchecked);
							UninstallCheck->SetIsChecked(ECheckBoxState::Unchecked);

							GInstallSettings.SetupUpType = 1;
							bCanNext = false;
						}
						else 
						{
							CreateSilentCheck->SetIsChecked(true);
						}
					})
				]
				+SHorizontalBox::Slot().HAlign(HAlign_Left).AutoWidth()
				[
					SNew(STextBlock).Text(LOCTEXT("CreateSilentInstallation_Text", "使用静默安装文件"))
				]
			]
			+SVerticalBox::Slot().Padding(20.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("CreateAFile_Text", "创建一个文件以此允许你采用命令行的方式来安装构建代理"))
			]
		]
	];
#pragma endregion
	
#pragma region UnInstall
	SelectionContainer->AddSlot().AutoHeight().SEC_PADDING
	[
		SNew(SVerticalBox)
		.Visibility_Lambda([bInstallAready]()
		{
			return bInstallAready ? EVisibility::Visible : EVisibility::Collapsed;
		})
		+SVerticalBox::Slot()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().HAlign(HAlign_Left).AutoWidth()
				[
					SAssignNew(UninstallCheck, SCheckBox)
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
					{
						if (InState == ECheckBoxState::Checked)
						{
							InstallCheck->SetIsChecked(ECheckBoxState::Unchecked);
							CreateSilentCheck->SetIsChecked(ECheckBoxState::Unchecked);
							GInstallSettings.SetupUpType = EInstallType::IT_Unistall;
							bCanNext = true;
						}
						else 
						{
							UninstallCheck->SetIsChecked(true);
						}
					})
				]
				+SHorizontalBox::Slot().HAlign(HAlign_Left).AutoWidth()
				[
					SNew(STextBlock).Text(LOCTEXT("UninstallXiaoBuild_Text", "卸载XiaoBuild"))
				]
			]
			+SVerticalBox::Slot().Padding(20.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("UninstallAllFileAndSettings_Text", "移除当前机器上所有文件和设置"))
			]
		]
	];
#pragma endregion
	
	ChildSlot
	[
		SNew(SVerticalBox)
		TOP_WIDGET
#pragma region 
		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().FIR_PADDING
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SetupOption_Text", "选项设置"))
			]
			+SVerticalBox::Slot().SEC_PADDING
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SelectOption_Text", "选择下述设置选项"))
			]
		]
		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
		]
#pragma endregion 

#pragma region Selection
		+SVerticalBox::Slot().AutoHeight()
		[
			SelectionContainer.ToSharedRef()
		]
	];
#pragma endregion

	InstallCheck->SetIsChecked(ECheckBoxState::Checked);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SSetupOptions::GetNext()
{
	if(GInstallSettings.SetupUpType == 0)
	{
		if(!InstallView.IsValid())
		{
			InstallView = SNew(SInstallView);
		}
		return InstallView;
	}
	if(GInstallSettings.SetupUpType == 1)
	{
		if(!UpdateView.IsValid())
		{
			// TODO 是否不对
			UpdateView = SNew(SReadyToUpdate);
		}
		return UpdateView;
	}

	if(GInstallSettings.SetupUpType == 2)
	{
		if(!UninstallView.IsValid())
		{
			UninstallView = SNew(SUninstallView);
		}
		return UninstallView;
	}

	return nullptr;
}

bool SSetupOptions::OnCanNext()
{
	return bCanNext;
}

bool SSetupOptions::OnCanBack()
{
	return true;
}

#undef LOCTEXT_NAMESPACE
