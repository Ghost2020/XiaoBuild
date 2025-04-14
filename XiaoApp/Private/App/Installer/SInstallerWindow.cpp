/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SInstallerWindow.h"

#include "HAL/PlatformApplicationMisc.h"
#include "SlateOptMacros.h"
#include "SPrimaryButton.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "XiaoStyle.h"
#include "XiaoShare.h"
#include "XiaoShareField.h"

#include "InstallerViews/SWelcomeView.h"
#include "InstallerViews/SSyncUpdateView.h"

#define LOCTEXT_NAMESPACE "SInstallerWindow"

static constexpr int32 GInstallerWindow_Width = 400;
static constexpr int32 GInstallerWindow_Height = 220;

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SInstallerWindow::Construct(const FArguments& Args)
{
	ConstructView();

	SWindow::Construct(SWindow::FArguments()
		.Title(!bSyncUpdate ? LOCTEXT("InstallTitle", "安装引导") : LOCTEXT("UpdateTitle", "同步更新"))
		.HasCloseButton(false)
		.ClientSize(FVector2D(GInstallerWindow_Width * 2, GInstallerWindow_Height * 2))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
#pragma region Laugauge
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Top).AutoHeight().HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().HAlign(HAlign_Right).AutoWidth()
					[
						SNew(SImage).Image(FXiaoStyle::Get().GetBrush(TEXT("Localization")))
					]
					+ SHorizontalBox::Slot().HAlign(HAlign_Right).AutoWidth()
					[
						SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&LauguageOptionList)
						.InitiallySelectedItem(LauguageOptionList[0])
						.HasDownArrow(true)
						.OnGenerateWidget_Lambda([](const TSharedPtr<FString>& InItem)
						{
							return SNew(STextBlock).Text(FText::FromString(*InItem));
						})
						[
							SAssignNew(LauguageText, STextBlock).Text(FText::FromString(*LauguageOptionList[0]))
						]
						.OnSelectionChanged_Lambda([this](const TSharedPtr<FString>& InItem, ESelectInfo::Type InType)
						{
							this->LauguageText->SetText(FText::FromString(*InItem));
							GInstallSettings.Localization = GLauguage2Loc[*InItem];
							FInternationalization::Get().SetCurrentCulture(GInstallSettings.Localization);
						})
					]
				]
#pragma endregion

#pragma region View
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Top).FillHeight(1.0f)
				[
					SAssignNew(Switcher, SWidgetSwitcher)
					+ SWidgetSwitcher::Slot()
					[
						CurrentView.Pin().ToSharedRef()
					]
				]
#pragma endregion

#pragma region Foot
				+ SVerticalBox::Slot().AutoHeight().MaxHeight(45.0f)
				.VAlign(VAlign_Bottom)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().HAlign(HAlign_Left)
					[
						SAssignNew(HelpButton, SPrimaryButton)
						.Text(LOCTEXT("Help_Text", "帮助"))
						.OnClicked_Static(&SInstallerWindow::OnHelp)
					]

					+SHorizontalBox::Slot().HAlign(HAlign_Right)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot().AutoWidth()
						[
							SAssignNew(BackButton, SPrimaryButton)
							.Text(LOCTEXT("Back_Text", "< 回退"))
							.OnClicked_Lambda([this]()
							{
								if(CurrentView.IsValid())
								{
									CurrentView = CurrentView.Pin()->GetBack();
									if (auto ActiveWidget = StaticCastSharedPtr<SWizardView>(Switcher->GetActiveWidget()))
									{
										ActiveWidget->OnResetState();
										Switcher->RemoveSlot(ActiveWidget.ToSharedRef());
									}
									Switcher->AddSlot()[CurrentView.Pin().ToSharedRef()];
								}
								return FReply::Handled();
							})
							.IsEnabled_Lambda([this]()
							{
								if(this->CurrentView.IsValid())
								{
									return this->CurrentView.Pin()->OnCanBack();
								}
								return false;
							})
						]
						+SHorizontalBox::Slot()
						[
							SAssignNew(NextButton, SPrimaryButton)
							.Visibility_Lambda([this]() 
							{
								if (this->CurrentView.IsValid())
								{
									return this->CurrentView.Pin()->IsFinal() ? EVisibility::Collapsed : EVisibility::Visible;
								}
								return EVisibility::Collapsed;
							})
							.Text_Lambda([this]()
							{
								if(this->CurrentView.IsValid())
								{
									return this->CurrentView.Pin()->GetNextTitle();
								}
								return LOCTEXT("NextTitle_Text", "继续");
							})
							.OnClicked_Lambda([this]()
							{
								if(CurrentView.IsValid())
								{
									const TSharedPtr<SWizardView> LastView = CurrentView.Pin();
									CurrentView = LastView->GetNext();
									CurrentView.Pin()->BackView = LastView;
									Switcher->RemoveSlot(Switcher->GetActiveWidget().ToSharedRef());
									Switcher->AddSlot()[CurrentView.Pin().ToSharedRef()];
								}
								return FReply::Handled();
							})
							.IsEnabled_Lambda([this]()
							{
								if(this->CurrentView.IsValid())
								{
									return this->CurrentView.Pin()->OnCanNext();
								}
								return false;
							})
						]
						+SHorizontalBox::Slot()
						[
							SAssignNew(ExitButton, SPrimaryButton)
							.Text(LOCTEXT("Exit_Text", "退出"))
							.Visibility_Lambda([this]()
							{
								if (this->CurrentView.IsValid())
								{
									return this->CurrentView.Pin()->IsFinal() ? EVisibility::Collapsed : EVisibility::Visible;
								}
								return EVisibility::Collapsed;
							})
							.OnClicked_Lambda([this]()
							{
								if(this->CurrentView.IsValid())
								{
									this->CurrentView.Pin()->OnExit();
								}
								return FReply::Handled();
							})
							.IsEnabled_Lambda([this]()
							{
								if(this->CurrentView.IsValid())
								{
									return this->CurrentView.Pin()->OnCanExit();
								}
								return false;
							})
						]
						+ SHorizontalBox::Slot()
						[
							SAssignNew(FinishButton, SPrimaryButton)
							.Text(LOCTEXT("InstallFinish_Text", "完成"))
							.Visibility_Lambda([this]()
							{
								if (this->CurrentView.IsValid())
								{
									return this->CurrentView.Pin()->IsFinal() ? EVisibility::Visible : EVisibility::Collapsed;
								}
								return EVisibility::Collapsed;
							})
							.OnClicked_Lambda([this]()
							{
								if (this->CurrentView.IsValid())
								{
									this->CurrentView.Pin()->OnFinish();
								}
								return FReply::Handled();
							})
							.IsEnabled_Lambda([this]()
							{
								if (this->CurrentView.IsValid())
								{
									return this->CurrentView.Pin()->OnCanNext();
								}
								return false;
							})
						]
					]
				]
#pragma endregion
			]
			+ SOverlay::Slot()
			[
				SAssignNew(MaskBorder, SBorder)
				.BorderBackgroundColor(FColor(0.0f, 0.0f, 0.0f, 125.0f))
				.Visibility(EVisibility::Collapsed)
			]
		].SupportsMaximize(false).SizingRule(ESizingRule::FixedSize)
	);

	OnWindowClosed.BindRaw(this, &SInstallerWindow::OnExit);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInstallerWindow::SetLockedState(const bool InbLock) const
{
	MaskBorder->SetVisibility(InbLock ? EVisibility::Visible : EVisibility::Collapsed);
}

void SInstallerWindow::ConstructView()
{
	const FString Cmd = FCommandLine::Get();
	if (FParse::Param(*Cmd, TEXT("sync_update")))
	{
		bSyncUpdate = true;
		GInstallSettings.SetupUpType = EInstallType::IT_Update;
	}

	if (!bSyncUpdate)
	{
		if (!WelcomeView.IsValid())
		{
			WelcomeView = SNew(SWelcomeView);
		}
		CurrentView = WelcomeView;
	}
	else
	{
		if (!UpdateView.IsValid())
		{
			UpdateView = SNew(SSyncUpdateView);
		}
		CurrentView = UpdateView;
	}

	for (const auto& Iter : GLauguage2Loc)
	{
		LauguageOptionList.Add(MakeShared<FString>(Iter.Key));
	}
}

FReply SInstallerWindow::OnHelp()
{
	GetHelp(GLocalization == TEXT("zh-CN") ? TEXT("1.安装配置工具") : TEXT("1.Installing and Configuring the Tool"));
	return FReply::Unhandled();
}

void SInstallerWindow::OnExit(const TSharedRef<SWindow>& InWindow) const
{
	OnExit();
}

void SInstallerWindow::OnExit() const
{
	RequestEngineExit(TEXT("RequestExit"));
}

#undef LOCTEXT_NAMESPACE
