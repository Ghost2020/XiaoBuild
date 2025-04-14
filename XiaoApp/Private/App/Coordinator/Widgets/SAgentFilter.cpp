/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SAgentFilter.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Input/SComboBox.h"
#include "STwowaySlider.h"
#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "SAgentFilter"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAgentFilter::Construct(const FArguments& InArgs)
{
	LicenseAdditionalSource.Add(MakeShared<FString>(TEXT("所有的")));
	LicenseAdditionalSource.Add(MakeShared<FString>(TEXT("固定的")));
	LicenseAdditionalSource.Add(MakeShared<FString>(TEXT("浮动的")));
	
	ChildSlot
	[
		SNew(SVerticalBox)
#pragma region Reset
		+ SVerticalBox::Slot().VAlign(VAlign_Bottom).AutoHeight()
		[
			SNew(SButton).Text(LOCTEXT("ClearFilter_Text", "恢复所有"))
			.OnPressed_Lambda([]()
			{
				
			})
		]
#pragma endregion 
#pragma region Main
		+SVerticalBox::Slot()
		[
			SNew(SScrollBox).Orientation(Orient_Vertical)
			+ SScrollBox::Slot()
			[
				SNew(SBorder).BorderImage(FAppStyle::GetBrush("Menu.Background"))
				[
					SNew(SVerticalBox)
#pragma region Status
					+ SVerticalBox::Slot().Padding(FMargin(5)).AutoHeight()
					[
						SNew(SBorder).BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							SNew(SExpandableArea)
							.MinWidth(250)
							.AreaTitleFont(FXiaoStyle::Get().GetFontStyle("FilterHeader.Font"))
							.AreaTitle(LOCTEXT("StatusFilter_Text", "状态"))
							.ToolTipText(LOCTEXT("StatusFilter_ToolTip", "状态..."))
							.InitiallyCollapsed(false)
							.BodyContent()
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(20.0f, 3.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left)
									[
										SNew(STextBlock).Text(LOCTEXT("StatusAll_Text", "所有"))
									]
								]
		
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("StatusReady_Text", "已待命"))
									]
								]
		
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("StatusInitiating_Text", "启动中"))
									]
								]
		
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("StatusOffline_Text", "已掉线"))
									]
								]
		
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("StatusHelping_Text", "协助中"))
									]
								]
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("StatusUpdating_Text", "更新中"))
									]
								]
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("StatusStopped_Text", "已停止"))
									]
								]
							]

						]
					]
#pragma endregion 
#pragma region CPULoad
					+ SVerticalBox::Slot().Padding(FMargin(5)).AutoHeight()
					[
						SNew(SBorder).BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							SNew(SExpandableArea)
							.AreaTitleFont(FXiaoStyle::Get().GetFontStyle("FilterHeader.Font"))
							.AreaTitle(LOCTEXT("CPULoadFilter_Text", "CPU负载"))
							.ToolTipText(LOCTEXT("CPULoadFilter_ToolTip", "CPU负载..."))
							.InitiallyCollapsed(false)
							.BodyContent()
							[
								SNew(SBorder).BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                        		[					
                        			SNew(STwowaySlider)
                        			.InitialString(TEXT("显示0%-100%"))
                        			.ShowUnit(TEXT("%"))
									.MinValue(0.0).MaxValue(100.0).InitialMinValue(0.0).InitialMaxValue(100.0)
									.OnValueChanged_Lambda([] (float MinValue, float MaxValue)
									{
										
									})
								]
							]
						]
					]
#pragma endregion CPULoad
#pragma region Core
					+ SVerticalBox::Slot().Padding(FMargin(5)).AutoHeight()
					[
						SNew(SBorder).BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							SNew(SExpandableArea)
							.AreaTitleFont(FXiaoStyle::Get().GetFontStyle("FilterHeader.Font"))
							.AreaTitle(LOCTEXT("AgentCoreFilter_Text", "代理核心"))
							.ToolTipText(LOCTEXT("AgentCoreFilter_ToolTip", "代理核心..."))
							.InitiallyCollapsed(false)
							.BodyContent()
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(20.0f, 3.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left)
									[
										SNew(STextBlock).Text(LOCTEXT("LicenseAll_Text", "所有"))
									]
								]
							]
						]
					]
#pragma endregion Core
#pragma region License
					+ SVerticalBox::Slot().Padding(FMargin(5)).AutoHeight()
					[
						SNew(SBorder).BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                        [
							SNew(SExpandableArea)
							.AreaTitleFont(FXiaoStyle::Get().GetFontStyle("FilterHeader.Font"))
							.AreaTitle(LOCTEXT("LicenseTypeFilter_Text", "许可类型"))
							.ToolTipText(LOCTEXT("LicenseTypeFilter_ToolTip", "许可类型..."))
							.InitiallyCollapsed(false)
							.BodyContent()
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(20.0f, 3.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left)
									[
										SNew(STextBlock).Text(LOCTEXT("LicenseAll_Text", "所有"))
									]
								]
			
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("LicenseOnlyInitator_Text", "仅仅发起者"))
									]
								]
			
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("LicenseOnlyHeler_Text", "仅仅协助者"))
									]
								]
			
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("LicenseBoth_Text", "发起者+协助者"))
									]
								]

								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("LicenseNoLicense_Text", "没有许可"))
									]
								]

								+SVerticalBox::Slot().AutoHeight()
								[
									SNew(SVerticalBox)
									+SVerticalBox::Slot().Padding(20.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("AddtionalOptions_Text", "其他选项"))
									]
									+SVerticalBox::Slot().Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(SComboBox<TSharedPtr<FString>>)
										.OptionsSource(&LicenseAdditionalSource)
										.InitiallySelectedItem(LicenseAdditionalSource[0])
										.OnGenerateWidget_Lambda([] (const TSharedPtr<FString> InItem)
										{
											return SNew(STextBlock).Text(FText::FromString(*InItem));
										})
										.Content()
										[
											SAssignNew(LicenseAdditionalText, STextBlock).Text(FText::FromString(*LicenseAdditionalSource[0]))
										]
										.OnSelectionChanged_Lambda([&] (const TSharedPtr<FString>& InItem, ESelectInfo::Type InType)
										{
											this->LicenseAdditionalText->SetText(FText::FromString(*InItem));
										})
									]
								]
							]
						]
					]
#pragma endregion License
#pragma region HelperCache
					+ SVerticalBox::Slot().Padding(FMargin(5)).AutoHeight()
					[
						SNew(SBorder).BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                    	[
							SNew(SExpandableArea)
							.AreaTitleFont(FXiaoStyle::Get().GetFontStyle("FilterHeader.Font"))
							.AreaTitle(LOCTEXT("HelperCacheSizeFilter_Text", "缓存大小"))
							.ToolTipText(LOCTEXT("HelperCacheSizeFilter_ToolTip", "缓存大小..."))
							.InitiallyCollapsed(false)
							.BodyContent()
							[
								SNew(STwowaySlider)
								.InitialString(TEXT("显示0MB-20480MB"))
								.ShowUnit(TEXT("MB"))
								.MinValue(0.0).MaxValue(20480.0).InitialMinValue(0.0).InitialMaxValue(20480.0)
								.OnValueChanged_Lambda([] (float MinValue, float MaxValue)
								{
										
								})
							]
						]
					]
#pragma endregion HelperCache
#pragma region HelperEnabled
					+ SVerticalBox::Slot().Padding(FMargin(5)).AutoHeight()
					[
						SNew(SBorder).BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							SNew(SExpandableArea)
							.AreaTitleFont(FXiaoStyle::Get().GetFontStyle("FilterHeader.Font"))
							.AreaTitle(LOCTEXT("HelperEnabledFilter_Text", "开启协助"))
							.ToolTipText(LOCTEXT("HelperEnabledFilter_ToolTip", "开启协助..."))
							.InitiallyCollapsed(false)
							.BodyContent()
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(20.0f, 3.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left)
									[
										SNew(STextBlock).Text(LOCTEXT("EnabledAsHelper_Text", "开启作为协助者"))
									]
								]
				
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(20.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("DisbledAsHeler_Text", "关闭作为协助者"))
									]
								]
							]
						]
					]
#pragma endregion HelperEnabled
#pragma region BuildGroup
					+ SVerticalBox::Slot().Padding(FMargin(5)).AutoHeight()
					[
						SNew(SBorder).BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							SNew(SExpandableArea)
							.AreaTitleFont(FXiaoStyle::Get().GetFontStyle("FilterHeader.Font"))
							.AreaTitle(LOCTEXT("BuildGroupFilter_Text", "构建分组"))
							.ToolTipText(LOCTEXT("LogroupFilter_ToolTip", "构建分组..."))
							.InitiallyCollapsed(false)
							.BodyContent()
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(20.0f, 3.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+SHorizontalBox::Slot().HAlign(HAlign_Left)
									[
										SNew(STextBlock).Text(LOCTEXT("BuildGroupAll_Text", "所有"))
									]
								]
							]
						]
					]
#pragma endregion BuildGroup
#pragma region HelperPermissitons
					+ SVerticalBox::Slot().Padding(FMargin(5)).AutoHeight()
					[
						SNew(SBorder).BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							SNew(SExpandableArea)
							.AreaTitleFont(FXiaoStyle::Get().GetFontStyle("FilterHeader.Font"))
							.AreaTitle(LOCTEXT("AgentHelperPermissitonsFilter_Text", "协助权限"))
							.ToolTipText(LOCTEXT("AgentHelperPermissitonsFilter_ToolTip", "协助权限..."))
							.InitiallyCollapsed(false)
							.BodyContent()
							[
								
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(20.0f, 3.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("AllowdToE/DAsHelper_Text", "允许(Enable/Diabled)作为协助者"))
									]
								]
	
								+ SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(20.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("NotAllowdSetEnable/Disable_Text", "不允许设置为Enable/Disable"))
									]
								]
							]
						]
					]
#pragma endregion HelperPermissitons
#pragma region LogLevel
					+ SVerticalBox::Slot().Padding(FMargin(5)).AutoHeight()
					[
						SNew(SBorder).BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							SNew(SExpandableArea)
							.AreaTitleFont(FXiaoStyle::Get().GetFontStyle("FilterHeader.Font"))
							.AreaTitle(LOCTEXT("LogLevelFilter_Text", "日志等级"))
							.ToolTipText(LOCTEXT("LogLevelFilter_ToolTip", "日志等级..."))
							.InitiallyCollapsed(false)
							.BodyContent()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(20.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("LogLevelAll_Text", "所有"))
									]
								]
	
								+ SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("MinimalLogLevel_Text", "1-最小"))
									]
								]
								+ SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("BasicLogLevel_Text", "2-基础"))
									]
								]
								+ SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("IntermidiateLogLevel_Text", "3-中等"))
									]
								]
								+ SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("ExtendedLogLevel_Text", "4-扩展"))
									]
								]
								+ SVerticalBox::Slot()
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(40.0f, 5.0f, 5.0f, 5.0f).AutoWidth()
									[
										SNew(SCheckBox)
									]
									+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(0.0f, 5.0f, 5.0f, 5.0f)
									[
										SNew(STextBlock).Text(LOCTEXT("DetailedLogLevel_Text", "5-详细"))
									]
								]
							]
						]
					]
#pragma endregion LogLevel
				]
			]
		]
#pragma endregion
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
