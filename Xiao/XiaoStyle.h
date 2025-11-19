// Copyright Xiao Studio, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Styling/StyleColors.h"
#include "Brushes/SlateImageBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateBoxBrush.h"
#include "Brushes/SlateBorderBrush.h"
#include "Dialog/SMessageDialog.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "XiaoStyle"

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define IMAGE_BRUSH_SVG( RelativePath, ... ) FSlateVectorImageBrush( RootToContentDir(RelativePath, TEXT(".svg") ), __VA_ARGS__)
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

#define FIR_PADDING \
	AutoHeight() \
	.Padding(5.0f, 10.0f, 10.0f, 2.0f)

#define SEC_PADDING \
	AutoHeight() \
	.Padding(30.0f, 10.0f, 10.0f, 2.0f)

#define THR_PADDING \
    AutoHeight() \
    .Padding(30.0f, 10.0f, 10.0f, 2.0f)

#define L_PADDING(LeftPad) \
    Padding(LeftPad, 0.0f, 0.0f, 0.0f)

#define H_SEPATATOR \
	SNew(SSeparator) \
	.Thickness(3.0f) \
	.Orientation(Orient_Horizontal)

#define V_WIDGET(WDIGET, VALIGN, HALIGN) \
	SNew(SVerticalBox) \
	+ SVerticalBox::Slot().VAlign(VALIGN).HAlign(HALIGN) \
	[ \
		WDIGET \
	] \

#define V_CENTER_WIGET(WDIGET) \
	V_WIDGET(WDIGET, EVerticalAlignment::VAlign_Center, EHorizontalAlignment::HAlign_Center)

#define V_FILL_WIGET(WDIGET) \
	V_WIDGET(WDIGET, EVerticalAlignment::VAlign_Fill, EHorizontalAlignment::HAlign_Fill)


#define V_ADD_CHECKBOX(PREDIATE, DISPLAY) \
	+ SVerticalBox::Slot().SEC_PADDING \
	[ \
		SNew(SCheckBox) \
		.IsChecked_Lambda([]() { \
			return PREDIATE ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; \
		}) \
		.OnCheckStateChanged_Lambda([](const ECheckBoxState InState) { \
			PREDIATE = InState == ECheckBoxState::Checked ? true : false; \
		}) \
		.Content() \
		[ \
			SNew(STextBlock) \
			.Text(DISPLAY) \
		] \
	] \

#define V_ADD_SPINBOX(PREDIATE, MIN, MAX, DISPLAY, TOOLTIP_TEXT) \
	+ SVerticalBox::Slot().SEC_PADDING \
	[ \
		SNew(SHorizontalBox) \
		+ SHorizontalBox::Slot() \
		[ \
			SNew(STextBlock) \
			.Text(DISPLAY) \
			.ToolTipText(TOOLTIP_TEXT) \
		] \
		+ SHorizontalBox::Slot() \
		[ \
			SNew(SSpinBox<uint32>).MinValue(MIN).MaxValue(MAX) \
			.OnValueChanged_Lambda([](const uint32 InValue) { \
				PREDIATE = InValue; \
			}) \
			.Value_Lambda([](){ \
				return PREDIATE; \
			}) \
		] \
	] \
	

const FVector2D Icon600x340(600.f, 340.f);
const FVector2D Icon128x88(128.0f, 88.0f);
const FVector2D Icon600x88(600.f, 88.f);
const FVector2D Icon240x240(240.f, 240.f);
const FVector2D Icon128x128(128.f, 128.f);
const FVector2D Icon64x64(64.f, 64.f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon30x30(30.0f, 30.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon20x16(20.0f, 16.0f);
const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon12x12(12.0f, 12.0f);
const FVector2D Icon8x8(8.f, 8.f);


static const FColor XiaoRed = FColor(237, 28, 36, 255);
static const FColor XiaoGreen = FColor(73, 222, 172, 255);
static const FColor XiaoGrey = FColor(195, 195, 195, 255);

inline FTextBlockStyle NormalText;

inline FTextBlockStyle XiaoH1TextStyle;
inline FTextBlockStyle XiaoH2TextStyle;
inline FTextBlockStyle XiaoH3TextStyle;

inline FButtonStyle XiaoButton;


class FXiaoStyle final : public FSlateStyleSet
{
public:
	FXiaoStyle()
		: FSlateStyleSet(TEXT("Xiao"))
	{
		Construct();
		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

	~FXiaoStyle()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}

	static void ReloadTextures()
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}

	static FXiaoStyle& Get()
	{
		static FXiaoStyle Inst;
		return Inst;
	}

	static bool DoModel(const FText InMessage = LOCTEXT("DoModel_Text", "确定改变吗？"), const bool InbWarning = false)
	{
		static bool bSure;
		bSure = false;
		const TSharedRef<SMessageDialog> Dialog = SNew(SMessageDialog)
			.Title(LOCTEXT("ConfirmTitle", "警告"))
			.Message(InMessage)
			.UseScrollBox(false)
			.AutoCloseOnButtonPress(true)
			.Buttons(
				{
					SMessageDialog::FButton(LOCTEXT("SureButton", "确定"))
					.SetOnClicked(FSimpleDelegate::CreateLambda([&]()
					{
						if (!InbWarning)
						{
							bSure = true;
						}
					})),
					SMessageDialog::FButton(LOCTEXT("CancelButton", "取消")).SetPrimary(true).SetFocus()
				});
		Dialog->ShowModal();
		return bSure;
	}

	virtual const FName& GetStyleSetName() const override
	{
		static FName StyleName(TEXT("Xiao"));
		return StyleName;
	}

private:	
	void Construct()
	{
		// SetParentStyleName("CoreStyle");
		// FAppStyle::SetAppStyleSet(*this);
		SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

		FString ResourceFolder = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EngineDir(), TEXT("Source/Programs/XiaoBuild/Xiao/Resources")));
		if (!FPaths::DirectoryExists(ResourceFolder))
		{
			ResourceFolder = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EngineDir(), TEXT("Content/Slate/XiaoBuild")));
		}
		if (FPaths::DirectoryExists(ResourceFolder))
		{
			SetContentRoot(ResourceFolder);

			Set("NoBrush", new FSlateNoResource());
			Set("NoBorder", new FSlateNoResource());
			Set("NoBorder.Normal", new FSlateNoResource());
			Set("NoBorder.Hovered", new FSlateNoResource());
			Set("NoBorder.Pressed", new FSlateNoResource());

			Set("Localization", new IMAGE_BRUSH("Icons/Localization", Icon30x30));

			Set("AppIcon", new IMAGE_BRUSH("Icons/Windows/AppIcon", Icon40x40));

			Set("Welcome", new IMAGE_BRUSH("Icons/Installer/Welcome", Icon600x340));
			Set("LongLogo", new IMAGE_BRUSH("Icons/Installer/LongLogo", Icon600x88));
			
			Set("btn_close_normal", new IMAGE_BRUSH("Icons/btn_close_normal", Icon30x30));
			Set("btn_close_hover", new IMAGE_BRUSH("Icons/btn_close_hover", Icon30x30));

			Set("WhiteBrush", new FSlateColorBrush(FLinearColor::White));
			Set("BigLogo", new IMAGE_BRUSH("Icons/Xiao236", Icon128x128));
			Set("Filter", new IMAGE_BRUSH_SVG("Icons/Filter", Icon20x20));
			Set("IsValid", new IMAGE_BRUSH_SVG("Icons/IsValid", Icon20x20));
			Set("Preferences", new IMAGE_BRUSH_SVG("Icons/Preferences", Icon40x40));
			Set("serach", new IMAGE_BRUSH_SVG("Icons/serach", Icon20x20));
			Set("world", new IMAGE_BRUSH_SVG("Icons/world", Icon20x20));
			Set("Developer", new IMAGE_BRUSH_SVG("Icons/Developer", Icon40x40));
			Set("visible", new IMAGE_BRUSH("Icons/visible", Icon40x40));
			Set("invisible", new IMAGE_BRUSH("Icons/invisible", Icon40x40));
			Set("clear", new IMAGE_BRUSH("Icons/clear", Icon40x40));
			Set("lock", new IMAGE_BRUSH("Icons/lock", Icon40x40));
			Set("delete", new IMAGE_BRUSH("Icons/delete", Icon20x20));

			Set("agents", new IMAGE_BRUSH_SVG("Icons/nav_agent_list", Icon40x40));
			Set("agents_selected", new IMAGE_BRUSH_SVG("Icons/nav_agent_list_selected", Icon40x40));
			Set("users", new IMAGE_BRUSH_SVG("Icons/nav_users", Icon40x40));
			Set("users_selected", new IMAGE_BRUSH_SVG("Icons/nav_users_selected", Icon40x40));
			Set("agent_logs", new IMAGE_BRUSH_SVG("Icons/MessageLog", Icon40x40));
			Set("settings", new IMAGE_BRUSH_SVG("Icons/nav_settings", Icon40x40));
			Set("settings_selected", new IMAGE_BRUSH_SVG("Icons/nav_settings", Icon40x40));
			Set("license", new IMAGE_BRUSH_SVG("Icons/nav_license", Icon40x40));
			Set("license_selected", new IMAGE_BRUSH_SVG("Icons/nav_license_selected", Icon40x40));
			

			Set("profile", new IMAGE_BRUSH_SVG("Icons/nav_user_info_profile", Icon40x40));
			Set("profile_seleted", new IMAGE_BRUSH_SVG("Icons/nav_user_info_profile_selected", Icon40x40));
			Set("help", new IMAGE_BRUSH_SVG("Icons/nav_help", Icon40x40));
			Set("help_selected", new IMAGE_BRUSH_SVG("Icons/nav_help_selected", Icon40x40));
			Set("reload", new IMAGE_BRUSH("Icons/icon_compile_40x", Icon40x40));

			Set("notification", new IMAGE_BRUSH("Icons/Notifications", Icon30x30));
			Set("arrow_left", new IMAGE_BRUSH("Icons/arrow_left", Icon30x30));

			Set("white_point", new IMAGE_BRUSH("Icons/status/WhiteDot", Icon30x30));
			Set("red_point", new IMAGE_BRUSH("Icons/status/RedDot", Icon30x30));
			Set("green_point", new IMAGE_BRUSH("Icons/status/GreenDot", Icon30x30));
			Set("yellow_point", new IMAGE_BRUSH("Icons/status/YellowDot", Icon30x30));

			Set("Icons.Reset", new IMAGE_BRUSH_SVG("Icons/Reset", Icon30x30));
			Set("Icons.Add", new IMAGE_BRUSH("Icons/Add", Icon30x30));
			Set("Icons.AddGroup", new IMAGE_BRUSH("Icons/AddGroup", Icon30x30));

			Set("Icons.StatusReady", new IMAGE_BRUSH("Icons/status/BlueDot", Icon12x12));
			Set("Icons.StatusInitiating", new IMAGE_BRUSH("Icons/status/YellowDot", Icon12x12));
			Set("Icons.StatusOffline", new IMAGE_BRUSH("Icons/status/GreyDot", Icon12x12));
			Set("Icons.StatusHelping", new IMAGE_BRUSH("Icons/status/GreenDot", Icon12x12));
			Set("Icons.StatusUpdating", new IMAGE_BRUSH("Icons/status/UpdateBox", Icon12x12));
			Set("Icons.StatusStopped", new IMAGE_BRUSH("Icons/status/RedDot", Icon12x12));
			Set("Icons.StatusUndefined", new IMAGE_BRUSH("Icons/status/YellowDot", Icon12x12));

			Set("empty_search", new IMAGE_BRUSH_SVG("Icons/empty_search", Icon64x64));

			Set("active", new IMAGE_BRUSH("Icons/icon_ActiveTool_Accept_40x", Icon30x30));
			Set("in_active", new IMAGE_BRUSH("Icons/icon_ActiveTool_Cancel_40x", Icon30x30));
			Set("build.ok", new IMAGE_BRUSH("Icons/ok", Icon30x30));
			Set("build.error", new IMAGE_BRUSH("Icons/error", Icon30x30));
			Set("build.warning", new IMAGE_BRUSH("Icons/warning", Icon30x30));

			Set("build_groups", new IMAGE_BRUSH_SVG("Icons/build-groups", Icon30x30));
			Set("manage_columns", new IMAGE_BRUSH_SVG("Icons/manage_columns", Icon30x30));
			Set("add_cloud", new IMAGE_BRUSH("Icons/cloud", Icon30x30));

			Set("Icons.LicenseLocked", new IMAGE_BRUSH_SVG("Icons/license-locked", Icon128x128));
			Set("Icons.LicenseExclamation", new IMAGE_BRUSH_SVG("Icons/license-exclamation", Icon128x128));
			Set("Icons.Refresh", new IMAGE_BRUSH("Icons/status/Refresh", Icon16x16));

			Set("Icons.copy", new IMAGE_BRUSH("Icons/copy", Icon16x16));
			Set("Icons.update", new IMAGE_BRUSH("Icons/update", Icon16x16));
			Set("Icons.pin", new IMAGE_BRUSH("Icons/pin", Icon16x16));
			Set("Icons.switch", new IMAGE_BRUSH("Icons/switch", Icon16x16));

			Set("Icons.switch-on", new IMAGE_BRUSH("Icons/switch-on", Icon16x16));
			Set("Icons.switch-off", new IMAGE_BRUSH("Icons/switch-off", Icon16x16));

			Set("Icons.link", new IMAGE_BRUSH("Icons/link", Icon16x16));
			Set("Icons.disconnect", new IMAGE_BRUSH("Icons/disconnect", Icon16x16));

			Set("Tray.CheckUpdate", new IMAGE_BRUSH("Icons/Tray/update", Icon16x16));

			Set("Icons.Trace", new IMAGE_BRUSH_SVG("Icons/BuildMonitor/Profile", Icon30x30));
			Set("Icons.build", new IMAGE_BRUSH_SVG("Icons/BuildMonitor/build", Icon30x30));
			Set("Icons.graph", new IMAGE_BRUSH("Icons/BuildMonitor/graph", Icon30x30));
			Set("Icons.output", new IMAGE_BRUSH("Icons/BuildMonitor/output", Icon30x30));
			Set("Icons.projects", new IMAGE_BRUSH("Icons/BuildMonitor/projects", Icon30x30));
			Set("Icons.summary", new IMAGE_BRUSH("Icons/BuildMonitor/summary", Icon30x30));
			Set("Icons.message", new IMAGE_BRUSH("Icons/BuildMonitor/warning", Icon30x30));
			Set("Icons.history", new IMAGE_BRUSH_SVG("Icons/BuildMonitor/history", Icon30x30));
			Set("Icons.Prosesor", new IMAGE_BRUSH_SVG("Icons/BuildMonitor/ProfileDataVisualizer", Icon30x30));
			Set("Icons.Details", new IMAGE_BRUSH_SVG("Icons/BuildMonitor/Details", Icon30x30));
			Set("Icons.Compact", new IMAGE_BRUSH("Icons/BuildMonitor/Compact", Icon30x30));
			Set("TreeTable.RowBackground", new IMAGE_BRUSH("Icons/BuildMonitor/White", Icon16x16));
			Set("Monitor.Realtime", new BOX_BRUSH("Icons/BuildMonitor/RealtimeBorder", FMargin(18.0f / 64.0f)));

			Set("Icons.frames", new IMAGE_BRUSH("Icons/BuildMonitor/GraphViewStack_48x", Icon16x16));
			Set("Icons.timing", new IMAGE_BRUSH_SVG("Icons/BuildMonitor/timing", Icon16x16));
			Set("Icons.network", new IMAGE_BRUSH_SVG("Icons/BuildMonitor/network", Icon30x30));
			Set("Icons.download", new IMAGE_BRUSH("Icons/download", Icon30x30));

			Set("Tray.Github", new IMAGE_BRUSH("Icons/Tray/github-mark-white", Icon16x16));
			Set("Tray.Monitor", new IMAGE_BRUSH("Icons/Tray/monitor", Icon16x16));
			Set("Tray.History", new IMAGE_BRUSH("Icons/Tray/history", Icon16x16));
			Set("Tray.BeginBuild", new IMAGE_BRUSH("Icons/Tray/start", Icon16x16));
			Set("Tray.StopBuild", new IMAGE_BRUSH("Icons/Tray/stop", Icon16x16));
			Set("Tray.Log", new IMAGE_BRUSH_SVG("Icons/Tray/Log", Icon16x16));
			Set("Tray.Website", new IMAGE_BRUSH("Icons/Tray/website", Icon16x16));
			Set("Tray.OnlineHelp", new IMAGE_BRUSH("Icons/Tray/help", Icon16x16));
			Set("Tray.Support", new IMAGE_BRUSH("Icons/Tray/support", Icon16x16));
			Set("Tray.Community", new IMAGE_BRUSH("Icons/Tray/community", Icon16x16));
			Set("Tray.CheckUpdate", new IMAGE_BRUSH("Icons/Tray/update", Icon16x16));
			Set("Tray.About", new IMAGE_BRUSH("Icons/Tray/about", Icon16x16));
			Set("Tray.NetworkTest", new IMAGE_BRUSH("Icons/Tray/network", Icon16x16));
			Set("Tray.CacheClean", new IMAGE_BRUSH("Icons/Tray/clean", Icon16x16));
			Set("Tray.AgentSettings", new IMAGE_BRUSH("Icons/Tray/settings", Icon16x16));
			Set("Tray.CoordiManager", new IMAGE_BRUSH("Icons/Tray/coordinator", Icon16x16));
			Set("Tray.EnableAsHelper", new IMAGE_BRUSH("Icons/Tray/helper", Icon16x16));
			Set("Tray.DisableAsHelper", new IMAGE_BRUSH("Icons/Tray/leave", Icon16x16));
			Set("Tray.Exit", new IMAGE_BRUSH("Icons/Tray/exit", Icon16x16));

			Set("Port.Query", new IMAGE_BRUSH("Icons/status/Query", Icon16x16));
			Set("Port.OK", new IMAGE_BRUSH("Icons/status/OK", Icon16x16));
			Set("Port.NG", new IMAGE_BRUSH("Icons/disconnect", Icon16x16));


			Set("Installer/Grid", new IMAGE_BRUSH("Icons/Installer/Grid", Icon128x128));
			Set("Installer/Agent", new IMAGE_BRUSH("Icons/Installer/Agent", Icon128x128));
			Set("Installer/SyncUpdate", new IMAGE_BRUSH("Icons/Installer/SyncUpdate", Icon128x128));

			Set("Coordi.Time", new IMAGE_BRUSH_SVG("Icons/CoordiManager/time_dark", Icon16x16));
			Set("Coordi.Measure", new IMAGE_BRUSH_SVG("Icons/CoordiManager/measure_dark", Icon20x16));
			Set("Coordi.Memory", new IMAGE_BRUSH_SVG("Icons/CoordiManager/memory_dark", Icon16x16));
			Set("Coordi.NetworkUpDown", new IMAGE_BRUSH("Icons/CoordiManager/in_out", Icon16x16));
			Set("Coordi.Client", new IMAGE_BRUSH_SVG("Icons/CoordiManager/user_dark", Icon16x16));
			Set("Coordi.Quit", new IMAGE_BRUSH("Icons/CoordiManager/quit", Icon16x16));

			Set("LeftArrow", new IMAGE_BRUSH("Icons/Arrow-Left", Icon16x16));
			Set("RightArrow", new IMAGE_BRUSH("Icons/Arrow-Right", Icon16x16));

			Set("Install.Congradulations", new IMAGE_BRUSH("Icons/Windows/AppIcon", Icon40x40));

			{
				const FTableRowStyle ProjectBrowserTableRowStyle = FTableRowStyle()
					.SetEvenRowBackgroundBrush(FSlateNoResource())
					.SetEvenRowBackgroundHoveredBrush(FSlateNoResource())
					.SetOddRowBackgroundBrush(FSlateNoResource())
					.SetOddRowBackgroundHoveredBrush(FSlateNoResource())
					.SetSelectorFocusedBrush(FSlateNoResource())
					.SetActiveBrush(FSlateNoResource())
					.SetActiveHoveredBrush(FSlateNoResource())
					.SetInactiveBrush(FSlateNoResource())
					.SetInactiveHoveredBrush(FSlateNoResource())
					.SetActiveHighlightedBrush(FSlateNoResource())
					.SetInactiveHighlightedBrush(FSlateNoResource())
					.SetTextColor(FStyleColors::Foreground)
					.SetSelectedTextColor(FStyleColors::ForegroundInverted);

				Set("ProjectBrowser.TableRow", ProjectBrowserTableRowStyle);
				
				Set("ProjectBrowser.ProjectTile.Font", DEFAULT_FONT("Regular", 9));
				Set("ProjectBrowser.ProjectTile.ThumbnailAreaBackground", new FSlateRoundedBoxBrush(COLOR("#474747FF"), FVector4(4.0f,4.0f,0.0f,0.0f)));
				Set("ProjectBrowser.ProjectTile.NameAreaBackground", new FSlateRoundedBoxBrush(EStyleColor::Header, FVector4(0.0f, 0.0f, 4.0f, 4.0f)));
				Set("ProjectBrowser.ProjectTile.NameAreaHoverBackground", new FSlateRoundedBoxBrush(FStyleColors::Hover, FVector4(0.0f, 0.0f, 4.0f, 4.0f)));
				Set("ProjectBrowser.ProjectTile.NameAreaSelectedBackground", new FSlateRoundedBoxBrush(FStyleColors::Primary, FVector4(0.0f, 0.0f, 4.0f, 4.0f)));
				Set("ProjectBrowser.ProjectTile.NameAreaSelectedHoverBackground", new FSlateRoundedBoxBrush(FStyleColors::PrimaryHover, FVector4(0.0f, 0.0f, 4.0f, 4.0f)));
				Set("ProjectBrowser.ProjectTile.DropShadow", new BOX_BRUSH("Icons/Installer/drop-shadow", FMargin(4.0f / 64.0f)));
				
				FLinearColor TransparentPrimary = FStyleColors::Primary.GetSpecifiedColor();
				TransparentPrimary.A = 0.0;
				Set("ProjectBrowser.ProjectTile.SelectedBorder", new FSlateRoundedBoxBrush(TransparentPrimary, 4.0f, FStyleColors::Primary, 1.0f));

				FLinearColor TransparentPrimaryHover = FStyleColors::PrimaryHover.GetSpecifiedColor();
				TransparentPrimaryHover.A = 0.0;
				Set("ProjectBrowser.ProjectTile.SelectedHoverBorder", new FSlateRoundedBoxBrush(TransparentPrimaryHover, 4.0f, FStyleColors::PrimaryHover, 1.0f));

				FLinearColor TransparentHover = FStyleColors::Hover.GetSpecifiedColor();
				TransparentHover.A = 0.0;
				Set("ProjectBrowser.ProjectTile.HoverBorder", new FSlateRoundedBoxBrush(TransparentHover, 4.0f, FStyleColors::Hover, 1.0f));
			}

			// Border
			Set("ToolPanel.GroupBorder", new BOX_BRUSH("Common/GroupBorder", FMargin(4.0f / 16.0f)));

			Set("RoundedWarning", new FSlateRoundedBoxBrush(FStyleColors::Transparent, 4.0f, FStyleColors::Warning, 1.0f));
			Set("RoundedError", new FSlateRoundedBoxBrush(FStyleColors::Transparent, 4.0f, FStyleColors::Error, 1.0f));
			Set("Icons.WarningWithColor", new IMAGE_BRUSH_SVG("Common/alert-triangle", Icon16x16, FStyleColors::Warning));


			// 字体
			{
				Set("NormalText", NormalText);

				XiaoH1TextStyle = FTextBlockStyle::GetDefault();
				XiaoH1TextStyle.SetColorAndOpacity(FColor::White);
				XiaoH1TextStyle.SetFontSize(25.0f);

				XiaoH2TextStyle = FTextBlockStyle::GetDefault();
				XiaoH2TextStyle.SetColorAndOpacity(FColor::White);
				XiaoH2TextStyle.SetFontSize(20.0f);

				XiaoH3TextStyle = FTextBlockStyle::GetDefault();
				XiaoH3TextStyle.SetColorAndOpacity(FColor::White);
				XiaoH3TextStyle.SetFontSize(15.0f);

				FSlateFontInfo FontInfo;
				Set("XiaoFont", FontInfo);
			}

			// 风格
			{
				const FSlateColor SelectionColor = FLinearColor(0.728f, 0.364f, 0.003f);
				const FSlateColor SelectionColor_Inactive = FLinearColor(0.25f, 0.25f, 0.25f);
				const FSlateColor SelectorColor = FLinearColor(0.701f, 0.225f, 0.003f);
				const FSlateColor DefaultForeground = FLinearColor(0.72f, 0.72f, 0.72f, 1.f);
				const FSlateColor InvertedForeground = FLinearColor(0, 0, 0);

				FTableRowStyle NormalTableRowStyle = FTableRowStyle()
					.SetEvenRowBackgroundBrush(FSlateNoResource())
					.SetEvenRowBackgroundHoveredBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, FLinearColor(1.0f, 1.0f, 1.0f, 0.1f)))
					.SetOddRowBackgroundBrush(FSlateNoResource())
					.SetOddRowBackgroundHoveredBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, FLinearColor(1.0f, 1.0f, 1.0f, 0.1f)))
					.SetSelectorFocusedBrush(BORDER_BRUSH("Common/Selector", FMargin(4.f / 16.f), SelectorColor))
					.SetActiveBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, SelectionColor))
					.SetActiveHoveredBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, SelectionColor))
					.SetInactiveBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, SelectionColor_Inactive))
					.SetInactiveHoveredBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, SelectionColor_Inactive))
					.SetTextColor(DefaultForeground)
					.SetSelectedTextColor(InvertedForeground)
					.SetDropIndicator_Above(BOX_BRUSH("Common/DropZoneIndicator_Above", FMargin(10.0f / 16.0f, 10.0f / 16.0f, 0, 0), SelectionColor))
					.SetDropIndicator_Onto(BOX_BRUSH("Common/DropZoneIndicator_Onto", FMargin(4.0f / 16.0f), SelectionColor))
					.SetDropIndicator_Below(BOX_BRUSH("Common/DropZoneIndicator_Below", FMargin(10.0f / 16.0f, 0, 0, 10.0f / 16.0f), SelectionColor));

				Set("TableView.DarkRow", FTableRowStyle(NormalTableRowStyle)
					.SetEvenRowBackgroundBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, FLinearColor(1.0f, 1.0f, 1.0f, 0.1f)))
					.SetEvenRowBackgroundHoveredBrush(IMAGE_BRUSH("PropertyView/DetailCategoryMiddle_Hovered", FVector2D(16, 16)))
					.SetOddRowBackgroundBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, FLinearColor(1.0f, 1.0f, 1.0f, 0.1f)))
					.SetOddRowBackgroundHoveredBrush(IMAGE_BRUSH("PropertyView/DetailCategoryMiddle_Hovered", FVector2D(16, 16)))
					.SetSelectorFocusedBrush(BORDER_BRUSH("Common/Selector", FMargin(4.f / 16.f), SelectorColor))
					.SetActiveBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, SelectionColor))
					.SetActiveHoveredBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, SelectionColor))
					.SetInactiveBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, SelectionColor_Inactive))
					.SetInactiveHoveredBrush(IMAGE_BRUSH("Common/Selection", Icon8x8, SelectionColor_Inactive))
					.SetTextColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.1f))
				);
			}

			{
				struct ButtonColor
				{
				public:
					FName Name;
					FLinearColor Normal;
					FLinearColor Hovered;
					FLinearColor Pressed;

					ButtonColor(const FName& InName, const FLinearColor& Color) : Name(InName)
					{
						Normal = Color * 0.8f;
						Normal.A = Color.A;
						Hovered = Color * 1.0f;
						Hovered.A = Color.A;
						Pressed = Color * 0.6f;
						Pressed.A = Color.A;
					}
				};

				Set("FlatButton.DefaultTextStyle", FTextBlockStyle(NormalText)
					.SetFont(DEFAULT_FONT("Bold", 10))
					.SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f))
					.SetHighlightColor(FLinearColor(1.0f, 1.0f, 1.0f))
					.SetShadowOffset(FVector2D(1, 1))
					.SetShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.9f)));

				TArray< ButtonColor > FlatButtons;
				FlatButtons.Add(ButtonColor("FlatButton.Primary", FLinearColor(0.02899, 0.19752, 0.48195)));
				FlatButtons.Add(ButtonColor("FlatButton.Success", FLinearColor(0.10616, 0.48777, 0.10616)));
				FlatButtons.Add(ButtonColor("FlatButton.Info", FLinearColor(0.10363, 0.53564, 0.7372)));
				FlatButtons.Add(ButtonColor("FlatButton.Warning", FLinearColor(0.87514, 0.42591, 0.07383)));
				FlatButtons.Add(ButtonColor("FlatButton.Danger", FLinearColor(0.70117, 0.08464, 0.07593)));

				for (const ButtonColor& Entry : FlatButtons)
				{
					Set(Entry.Name, FButtonStyle(XiaoButton)
						.SetNormal(BOX_BRUSH("Icons/Common/FlatButton", 2.0f / 8.0f, Entry.Normal))
						.SetHovered(BOX_BRUSH("Icons/Common/FlatButton", 2.0f / 8.0f, Entry.Hovered))
						.SetPressed(BOX_BRUSH("Icons/Common/FlatButton", 2.0f / 8.0f, Entry.Pressed))
					);
				}
			}
		}
	}
};

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef IMAGE_BRUSH_SVG
#undef IMAGE_PLUGIN_BRUSH_SVG
#undef DEFAULT_FONT
#undef LOCTEXT_NAMESPACE