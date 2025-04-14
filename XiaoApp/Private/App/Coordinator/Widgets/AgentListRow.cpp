/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#include "AgentListRow.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Misc/Attribute.h"
#include "Styling/SlateColor.h"
#include "SSimpleButton.h"
#include "XiaoShare.h"
#include "XiaoStyle.h"
#include "ShareDefine.h"
#include "XiaoShareRedis.h"


#define LOCTEXT_NAMESPACE "SAgentListRow"

#define TEXT_BLOCK(PREDICATE) \
	V_CENTER_WIGET( \
		SNew(SEditableText) \
		.Justification(ETextJustify::Type::Center) \
		.IsReadOnly(true) \
		.Text_Lambda([this]() { \
			return this->AgentProto.IsValid() ? FText::FromString(UTF8_TO_TCHAR(AgentProto.Pin()->PREDICATE().c_str())) : FText::GetEmpty();  \
		}) \
	);

#define NUMEIRC_TEXT_BLOCK(PREDICATE) \
	V_CENTER_WIGET( \
		SNew(SEditableText) \
		.Justification(ETextJustify::Type::Center) \
		.IsReadOnly(true) \
		.Text_Lambda([this]() { \
			return FText::FromString(FString::FromInt(this->AgentProto.IsValid() ? AgentProto.Pin()->PREDICATE() : 0)); \
		}) \
	);


#define NUMEIRC_LIMIT_TEXT_BLOCK(GET_MAX_TER, PREDICATE, GETTER) \
	V_CENTER_WIGET( \
	SNew(SHorizontalBox) \
	+ SHorizontalBox::Slot().HAlign(HAlign_Center).AutoWidth() \
	[ \
		SNew(SNumericEntryBox<uint32>) \
		.IsEnabled_Raw(this, &SAgentListRow::IsEnableEdit) \
		.AllowSpin(true) \
		.MinValue(0) \
		.MaxValue_Lambda([this] () { return GET_MAX_TER; }) \
		.Value_Lambda([this]() { return this->AgentProto.IsValid() ? this->AgentProto.Pin()->PREDICATE() : 0; }) \
		.OnValueCommitted_Lambda([this](uint16 InValue, ETextCommit::Type InType) { \
			if (InType != ETextCommit::Type::OnCleared && this->AgentProto.IsValid()) \
			{ \
				if (InValue != this->AgentProto.Pin()->PREDICATE()) \
				{ \
					this->AgentProto.Pin()->set_##PREDICATE(InValue); \
					(void)OnAgentChanged.ExecuteIfBound(AgentProto); \
				} \
			} \
		}) \
	] \
	+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).AutoWidth() \
	[ \
		SNew(STextBlock) \
		.Text_Lambda([this]() { \
			return FText::FromString(FString::Printf(TEXT("/ %u"), GETTER())); \
		})\
		.Justification(ETextJustify::Type::Center) \
	]);


#define PERCENT_BOX(AVAL, MIN, FORMAT) \
	V_FILL_WIGET(SNew(SBorder) \
	.BorderBackgroundColor_Lambda([this]() { \
		if (AgentProto.IsValid()) \
		{ \
			const float Percent = AgentProto.Pin()->AVAL() / GOriginalSystemSettings.MIN(); \
			if (Percent < 1.0f) \
			{ \
				const float Alpha = (1.0f - Percent) * 0.5f + 0.5f; \
				return FLinearColor(1.f, 0, 0, Alpha); \
			} \
		} \
		return FLinearColor::Transparent; \
	}) \
	[ \
		V_CENTER_WIGET(SNew(STextBlock) \
			.Text_Lambda([this]() { \
				FText NetText = FText::GetEmpty(); \
				if (this->AgentProto.IsValid()) \
				{ \
					NetText = FText::FromString(FString::Printf(FORMAT, static_cast<int>(AgentProto.Pin()->AVAL()))); \
				} \
				return NetText; \
			}) \
			.Justification(ETextJustify::Type::Center) \
		) \
	]);


#define COMBO_BOX(OPTION_SOURCE, PREDICATE) \
	V_FILL_WIGET( \
		SNew(SComboBox<TSharedPtr<FText>>) \
		.OnGenerateWidget_Lambda([](const TSharedPtr<FText> InText) \
		{ \
			return SNew(STextBlock).Text(*InText); \
		}) \
		.OnSelectionChanged_Lambda([this](const TSharedPtr<FText> InText, ESelectInfo::Type) { \
			GCanUpdate = true; \
			if (this->AgentProto.IsValid()) \
			{ \
				const int32 Val = OPTION_SOURCE.Find(InText); \
				if (Val != this->AgentProto.Pin()->PREDICATE()) \
				{ \
					this->AgentProto.Pin()->set_##PREDICATE(Val); \
					(void)OnAgentChanged.ExecuteIfBound(AgentProto); \
				} \
			} \
		}).Content() \
		[ \
			SNew(STextBlock).Text_Lambda([this]() { return this->AgentProto.IsValid() ? *OPTION_SOURCE[this->AgentProto.Pin()->PREDICATE()] : FText::GetEmpty(); }) \
		] \
		.IsEnabled_Raw(this, &SAgentListRow::IsEnableEdit) \
		.InitiallySelectedItem(OPTION_SOURCE[this->AgentProto.IsValid() ? this->AgentProto.Pin()->PREDICATE() : 0]) \
		.OnComboBoxOpening_Lambda([]() { GCanUpdate = false; }) \
		.OptionsSource(&OPTION_SOURCE));


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAgentListRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	check(InArgs._AgentProto.IsValid());
	AgentProto = InArgs._AgentProto;
	OnAgentChanged = InArgs._OnAgentChanged;
	
	SMultiColumnTableRow::Construct(
		FSuperRowType::FArguments().Cursor_Lambda([this]() {
			return AgentProto.IsValid() ? (AgentProto.Pin()->status() == 1 ? EMouseCursor::Type::Crosshairs : EMouseCursor::Hand) : EMouseCursor::Default;
		})
		.Padding(2.0f).ShowWires(true)/*.Style(FAppStyle::Get(), "PropertyTable.TableRow")*/, 
		InOwnerTableView
	);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SAgentListRow::GenerateWidgetForColumn(const FName& InColumnName)
{
	if(InColumnName == S_ColumnIdIndex)
	{
		return NUMEIRC_TEXT_BLOCK(index);
	}
	if(InColumnName == S_ColumnIdInfor)
	{
		return V_FILL_WIGET(
			SNew(SBorder)
			.BorderBackgroundColor_Lambda([this]()
			{
				FText Text, ToolTip;
				FLinearColor Color;
				GetStatusDesc(Text, ToolTip, Color);
				return Color;
			})
			[
				V_WIDGET(
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5.0f).HAlign(HAlign_Left).VAlign(VAlign_Center).MaxWidth(50.0f).MinWidth(50.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { 
						std::string Desc;
						if (this->AgentProto.IsValid())
						{
							auto Proto = this->AgentProto.Pin();
							Desc = Proto->desc();
							if (Desc.empty())
							{
								Desc = Proto->loginuser();
							}
						}
						return FText::FromString(UTF8_TO_TCHAR(Desc.c_str()));
					})
					.Justification(ETextJustify::Left)
					.MinDesiredWidth(50.0f)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				]
				+SHorizontalBox::Slot().AutoWidth().Padding(5.0f).HAlign(HAlign_Left)
				[
					SNew(SBorder)
					.Visibility_Lambda([this]() { return this->AgentProto.IsValid() ? this->AgentProto.Pin()->benableinitator() ? EVisibility::Visible : EVisibility::Collapsed : EVisibility::Collapsed; })
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SImage).Image_Lambda([this]() { return GetImage(AgentProto.IsValid() ? AgentProto.Pin()->bfixedinitator() : false); })
						]
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Initiator_Text", "发起者"))
							.ToolTipText_Lambda([this]() {
								if (AgentProto.IsValid())
								{
									return AgentProto.Pin()->bfixedinitator() ? LOCTEXT("FixedInitiator_ToolTpText", "尽可能的长时间占有协助者的计算资源，不论是否现在是否需要那么多协助") : LOCTEXT("NotFixedInitiator_ToolTpText", "按需进行获取，当不需要那么多计算资源，自动断开不必要的协助者的链接");
								}
								return FText::GetEmpty();
							})
						]
					]
				]
				+SHorizontalBox::Slot().AutoWidth().Padding(5.0f).HAlign(HAlign_Left)
				[
					SNew(SBorder).Visibility_Lambda([this](){ return this->AgentProto.IsValid() ? this->AgentProto.Pin()->benablehelper() ? EVisibility::Visible : EVisibility::Collapsed : EVisibility::Collapsed; })
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SImage).Image_Lambda([this]() { return GetImage(AgentProto.IsValid() ? AgentProto.Pin()->bfixedhelper() : false); })
						]
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Helper_Text", "协助者"))
							.ToolTipText_Lambda([this]() {
								if (AgentProto.IsValid())
								{
									return AgentProto.Pin()->bfixedhelper() ? LOCTEXT("FixedHelper_ToolTpText", "固定协助者TODO") : LOCTEXT("NotFixedHelper_ToolTpText", "流动式协助者TODO");
								}
								return FText::GetEmpty();
							})
						]
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Fill)
				[
					SNullWidget::NullWidget
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(5.0f).HAlign(HAlign_Right).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() {
						if (this->AgentProto.IsValid())
						{
							auto Proto = this->AgentProto.Pin();
							const EAgentStatus Status = static_cast<EAgentStatus>(Proto->status());
							if (Status == EAgentStatus::Status_Initiating || Status == EAgentStatus::Status_Helping)
							{
								return FText::FromString(UTF8_TO_TCHAR(Proto->message().c_str()));
							}
							FText Text, _T;
							FLinearColor _C;
							GetStatusDesc(Text, _T, _C);
							return Text;
						}
						return FText::GetEmpty();
					})
					.ToolTipText_Lambda([this]() {
						FText _T, ToolTip;
						FLinearColor _C;
						GetStatusDesc(_T, ToolTip, _C);
						return ToolTip;
					})
					.MinDesiredWidth(200.0f)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				]
				, VAlign_Center, HAlign_Left)
			]
		);
	}
	if(InColumnName == S_ColumnIdAvaCpu)
	{
		return V_FILL_WIGET(SNew(SBorder)
			.BorderBackgroundColor_Lambda([this]()
			{
				if (AgentProto.IsValid())
				{
					const float AvaCpu = AgentProto.Pin()->cpuava();
					const float Percent = AvaCpu / GOriginalSystemSettings.cpuavailableminimal();
					if (Percent < 1.0f)
					{
						const float Alpha = (1.0f - Percent) * 0.5f + 0.5f;
						return FLinearColor(1.f, 0, 0, Alpha);
					}
				}
				return FLinearColor::Transparent;
			})
			[
				V_CENTER_WIGET(SNew(STextBlock)
					.Text_Lambda([this]() { return this->AgentProto.IsValid() ? FText::FromString(FString::Printf(TEXT("%d%%"), static_cast<int>(100-AgentProto.Pin()->cpuava()))) : FText::GetEmpty(); })
					.Justification(ETextJustify::Type::Center)
				)
			]);
	}
	if(InColumnName == S_ColumnIdFreeDiskSpace)
	{
		return V_FILL_WIGET(SNew(SBorder)
			.BorderBackgroundColor_Lambda([this]()
			{
				if (AgentProto.IsValid())
				{
					const auto AgentPtr = AgentProto.Pin();
					const float AvaHardSpace = AgentPtr->totalhardspace() - AgentPtr->usehardspace();
					const float Percent = AvaHardSpace / GOriginalSystemSettings.harddiskminimal();
					if (Percent < 1.0f)
					{
						const float Alpha = (1.0f - Percent) * 0.5f + 0.5f;
						return FLinearColor(1.f, 0, 0, Alpha);
					}
				}
				return FLinearColor::Transparent;
			})
			[
				V_CENTER_WIGET(SNew(STextBlock)
					.Text_Lambda([this]()
					{
						FText DiskText = FText::GetEmpty();
						if (this->AgentProto.IsValid())
						{
							const auto AgentPtr = AgentProto.Pin();
							DiskText = FText::FromString(FString::Printf(TEXT("%.1f/%.1f GB"), AgentPtr->totalhardspace() - AgentPtr->usehardspace(), AgentPtr->totalhardspace()));
						}
						return DiskText;
					})
					.Justification(ETextJustify::Type::Center)
				)
			]
		);
	}
	if(InColumnName == S_ColumnIdAvailableMemory)
	{
		return V_FILL_WIGET(SNew(SBorder)
			.BorderBackgroundColor_Lambda([this]()
				{
					if (AgentProto.IsValid())
					{
						const float AvaMemory = AgentProto.Pin()->totalmemory() - AgentProto.Pin()->usememory();
						const float Percent = AvaMemory / GOriginalSystemSettings.physicalmemory();
						if (Percent < 1.0f)
						{
							const float Alpha = (1.0f - Percent) * 0.5f + 0.5f;
							return FLinearColor(1.f, 0, 0, Alpha);
						}
					}
					return FLinearColor::Transparent;
				})
			[
				V_CENTER_WIGET(SNew(STextBlock)
					.Text_Lambda([this]() {
						FText MemoryText = FText::GetEmpty();
						if (this->AgentProto.IsValid())
						{
							const float AvaMemory = AgentProto.Pin()->totalmemory() - AgentProto.Pin()->usememory();
							MemoryText = FText::FromString(FString::Printf(TEXT("%.1f/%.1f GB"), AvaMemory, AgentProto.Pin()->totalmemory()));
						}
						return MemoryText;
						})
					.Justification(ETextJustify::Type::Center)
				)
			]);
	}
	if(InColumnName == S_ColumnIdAvaNet)
	{
		return PERCENT_BOX(avalnet, networkavamin, TEXT("%d%%"));
	}
	if(InColumnName == S_ColumnIdAvaGpu)
	{
		return PERCENT_BOX(avagpu, gpuavamin, TEXT("%d%%"));
	}
	if(InColumnName == S_ColumnIdDesc)
	{
		return V_CENTER_WIGET(
			SNew(SEditableText)
			.IsEnabled_Raw(this, &SAgentListRow::IsEnableEdit)
			.HintText(LOCTEXT("Desc_Text", "请输入代理描述"))
			.Text_Lambda([this]() {
				const FString Desc = this->AgentProto.IsValid() ? FString(UTF8_TO_TCHAR(this->AgentProto.Pin()->desc().c_str())) : FString(TEXT(""));
				return FText::FromString(Desc);
			})
			.OnTextChanged_Lambda([](const FText& InText) { GCanUpdate = false; })
			.OnTextCommitted_Lambda([this] (const FText& InText, ETextCommit::Type)
			{
				GCanUpdate = true;
				if (this->AgentProto.IsValid())
				{
					const std::string Desc = TCHAR_TO_UTF8(*InText.ToString());
					if (Desc != this->AgentProto.Pin()->desc())
					{
						this->AgentProto.Pin()->set_desc(Desc);
						(void)OnAgentChanged.ExecuteIfBound(AgentProto);
					}
				}
			}));
	}
	if(InColumnName == S_ColumnIdGroup)
	{
		const FString Group = this->AgentProto.IsValid() ? UTF8_TO_TCHAR(this->AgentProto.Pin()->group().c_str()) : SDefaultStr;
		int Index = -1;
		bool bFind = false;
		for (const auto& I : GGroupArray)
		{
			++Index;
			if (*I == Group)
			{
				bFind = true;
				break;
			}
		}
		if (!bFind)
		{
			Index = 0;
		}
		return V_FILL_WIGET(SNew(SComboBox<TSharedPtr<FString>>)
		.IsEnabled_Raw(this, &SAgentListRow::IsEnableEdit)
		.OptionsSource(&GGroupArray)
		.InitiallySelectedItem(GGroupArray[Index])
		.OnComboBoxOpening_Lambda([]() { GCanUpdate = false; })
		.OnGenerateWidget_Lambda([] (const TSharedPtr<FString> InText)
		{
			return SNew(STextBlock).Text(InText.IsValid() ? FText::FromString(*InText) : FText::GetEmpty());
		})
		.OnSelectionChanged_Lambda([this] (const TSharedPtr<FString> InText, ESelectInfo::Type)
		{
			if (InText.IsValid())
			{
				GCanUpdate = true;
				if (this->AgentProto.IsValid())
				{
					const std::string Group = TCHAR_TO_UTF8(**InText);
					if (Group != AgentProto.Pin()->group())
					{
						AgentProto.Pin()->set_group(Group);
						(void)OnAgentChanged.ExecuteIfBound(AgentProto);
					}
				}
			}
		})
		.Content()
		[
			SNew(STextBlock)
			.Text_Lambda([this]() 
			{
				FString Group = TEXT("Default");
				if (AgentProto.IsValid())
				{
					FString Str = UTF8_TO_TCHAR(AgentProto.Pin()->group().c_str());
					if (!Str.IsEmpty())
					{
						Group = Str;
					}
				}
				return FText::FromString(Group);
			})
		]
		);
	}
	if(InColumnName == S_ColumnIdLastConnected)
	{
		return TEXT_BLOCK(lastcon);
	}
	if(InColumnName == S_ColumnIdUsedHelpersCache)
	{
		return V_CENTER_WIGET(SNew(STextBlock)
			.Text_Lambda([this]() {
				FText CacheText = FText::GetEmpty();
				if (this->AgentProto.IsValid())
				{
					const float AvaCache = AgentProto.Pin()->totalhelpcache() - AgentProto.Pin()->usehelpcache();
					CacheText = FText::FromString(FString::Printf(TEXT("%.1f/%.1f MB"), AvaCache, AgentProto.Pin()->totalhelpcache()));
				}
				return CacheText;
			})
			.Justification(ETextJustify::Type::Center));
	}
	if(InColumnName == S_ColumnIdRegisteredHelperCores)
	{
		return NUMEIRC_LIMIT_TEXT_BLOCK((this->AgentProto.IsValid() ? this->AgentProto.Pin()->logiccore() : 1024), helpercore, GetLogicCoreNum);
	}
	if(InColumnName == S_ColumnIdMaxConnectAgentNum)
	{
		return NUMEIRC_LIMIT_TEXT_BLOCK(GOriginalSystemSettings.maxconnum(), maxcon, GOriginalSystemSettings.maxconnum);
	}
	if(InColumnName == S_ColumnIdMaxProcessors)
	{
		return NUMEIRC_LIMIT_TEXT_BLOCK(GOriginalSystemSettings.maxcorenum(), maxcpu, GOriginalSystemSettings.maxcorenum);
	}
	if(InColumnName == S_ColumnIdMaxLocalProcessors)
	{
		return NUMEIRC_LIMIT_TEXT_BLOCK(FPlatformMisc::NumberOfCoresIncludingHyperthreads(), localmaxcpu, GetLogicCoreNum);
	}
	if(InColumnName == S_ColumnIdBuildCache)
	{
		return V_CENTER_WIGET(SNew(STextBlock)
			.Text_Lambda([this]() { return this->AgentProto.IsValid() ? (this->AgentProto.Pin()->bbuildcache() ? LOCTEXT("Yes_Text", "是") : LOCTEXT("No_Text", "否")) : FText::GetEmpty(); })
			.Justification(ETextJustify::Type::Center));
	}
	if(InColumnName == S_ColumnIdBuildPriority)
	{
		return COMBO_BOX(GPriorityArray, buildpriority);
	}
	if(InColumnName == S_ColumnIdLogLevel)
	{
		return COMBO_BOX(GLevelArray, loglevel);
	}
	if(InColumnName == S_ColumnIdAssignmentPriority)
	{
		return COMBO_BOX(GPriorityArray, allocationpriority);
	}
	if(InColumnName == S_ColumnIdCpuInfo)
	{
		return TEXT_BLOCK(cpuinfo);
	}
	if(InColumnName == S_ColumnIdCpuArch)
	{
		return TEXT_BLOCK(cpuarch);
	}
	if(InColumnName == S_ColumnIdLoggedOnUser)
	{
		return TEXT_BLOCK(loginuser);
	}
	if(InColumnName == S_ColumnIdLogicCores)
	{
		return NUMEIRC_TEXT_BLOCK(logiccore);
	}
	if(InColumnName == S_ColumnIdMacAddress)
	{
		return TEXT_BLOCK(macaddress);
	}
	if(InColumnName == S_ColumnIdNetwork)
	{
		return V_CENTER_WIGET(SNew(STextBlock)
			.Text_Lambda([this]() {
				return this->AgentProto.IsValid() ? FText::FromString(FString::Printf(TEXT("%.2f Mbits/sec"), AgentProto.Pin()->networkspeed())) : FText::GetEmpty();
			})
			.Justification(ETextJustify::Type::Center));
	}
	if(InColumnName == S_ColumnIdOSSystem)
	{
		return TEXT_BLOCK(opsystem);
	}
	if(InColumnName == S_ColumnIdPhysicalCores)
	{
		return NUMEIRC_TEXT_BLOCK(physicalcore);
	}
	if(InColumnName == S_ColumnIdRoutingIP)
	{
		return V_FILL_WIGET(SNew(SBorder)
			.BorderBackgroundColor_Lambda([this]()
			{
				if (AgentProto.IsValid())
				{
					const FString RouterIP = UTF8_TO_TCHAR(AgentProto.Pin()->routerip().c_str());
					return IsValidIp(RouterIP) ? FLinearColor::Transparent : FLinearColor::Red;
				}
				return FLinearColor::Red;
			})
			[
				V_CENTER_WIGET(SNew(SEditableText)
				.Text_Lambda([this]() {
					return this->AgentProto.IsValid() ? FText::FromString(UTF8_TO_TCHAR(AgentProto.Pin()->routerip().c_str())) : FText::GetEmpty();
				})
				.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type) {
					const FString IpStr = InText.ToString();
					if (AgentProto.IsValid() && IsValidIp(IpStr))
					{
						const std::string Ip = TCHAR_TO_UTF8(*IpStr);
						if (Ip != AgentProto.Pin()->routerip())
						{
							AgentProto.Pin()->set_routerip(Ip);
							(void)OnAgentChanged.ExecuteIfBound(AgentProto);
						}
					}
				})
				.Justification(ETextJustify::Type::Center)
				)
			]);
	}
	if(InColumnName == S_ColumeIdListenPort)
	{
		return NUMEIRC_TEXT_BLOCK(helperport);
	}
	if (InColumnName == S_ColumnIdPortMappedAddress)
	{
		return V_CENTER_WIGET(SNew(SEditableText)
			.Text_Lambda([this]() {
				return this->AgentProto.IsValid() ? FText::FromString(UTF8_TO_TCHAR(AgentProto.Pin()->portmappedaddress().c_str())) : FText::GetEmpty();
				})
			.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type) {
				const FString AddressStr = InText.ToString();
				if (AgentProto.IsValid() && IsValidAddress(AddressStr))
				{
					const std::string Address = TCHAR_TO_UTF8(*AddressStr);
					if (Address != AgentProto.Pin()->portmappedaddress())
					{
						AgentProto.Pin()->set_portmappedaddress(Address);
						(void)OnAgentChanged.ExecuteIfBound(AgentProto);
					}
				}
				})
			.Justification(ETextJustify::Type::Center)
		);
	}
	if(InColumnName == S_ColumnIdUpDownTime)
	{
		return TEXT_BLOCK(updowntime);
	}
	if(InColumnName == S_ColumeIdResetState)
	{
		return V_CENTER_WIGET(SNew(SSimpleButton)
			.IsEnabled_Lambda([]() { return GCurrentUser.Role != 2; })
			.Visibility_Lambda([this]() {
			const bool bShow = this->AgentProto.IsValid() ? (this->AgentProto.Pin()->status() != 0) : false;
			return bShow ? EVisibility::Visible : EVisibility::Collapsed;
				})
			.OnClicked_Lambda([this]() {
				if (this->AgentProto.IsValid())
				{
					this->AgentProto.Pin()->set_status(0);
					static std::string SEmptyMsg = "";
					this->AgentProto.Pin()->set_message(SEmptyMsg);
					(void)OnAgentChanged.ExecuteIfBound(AgentProto);
				}
				return FReply::Handled();
			}).Icon(FXiaoStyle::Get().GetBrush("Icons.Reset"))
		);
	}

	return SNullWidget::NullWidget;
}

bool SAgentListRow::IsEnableEdit() const
{
	if (this->AgentProto.Pin())
	{
		const auto AgentStatus = this->AgentProto.Pin()->status();
		return GCurrentUser.Role != 2 && AgentStatus != 1 && AgentStatus != 2;
	}
	return false;
}

const FSlateBrush* SAgentListRow::GetImage(const bool bFixed)
{
	return FXiaoStyle::Get().GetBrush(bFixed ? TEXT("Icons.pin") : TEXT("Icons.switch"));
}

uint32 SAgentListRow::GetLogicCoreNum() const
{
	return this->AgentProto.IsValid() ? AgentProto.Pin()->logiccore() : 0;
}

void SAgentListRow::GetStatusDesc(FText& OutText, FText& OutToolTip, FLinearColor& OutColor)
{
	const int8 Status = this->AgentProto.IsValid() ? this->AgentProto.Pin()->status() : EAgentStatus::Status_Undefined;
	switch (static_cast<EAgentStatus>(Status))
	{
		case EAgentStatus::Status_Ready:
		{
			const FString RouterIp = UTF8_TO_TCHAR(this->AgentProto.Pin()->routerip().c_str());
			if (IsValidIp(RouterIp))
			{
				OutText = LOCTEXT("StatusReady_Text", "Ready");
				OutToolTip = LOCTEXT("StatusReady_ToolTip", "代理可以被用来运行构建");
				OutColor = FLinearColor::Transparent;
			}
			else
			{
				OutText = LOCTEXT("UnReached_Text", "Unreachable");
				OutToolTip = LOCTEXT("UnReached_ToolTip", "代理的ip地址无效，无法建立网络连接");
				OutColor = FLinearColor(1.f, 0, 0, 0.5f);
			}
			break;
		}
		case EAgentStatus::Status_Initiating:
		{
			OutText = LOCTEXT("StatusInitiating_Text", "Initiating");
			OutToolTip = LOCTEXT("StatusInitiating_ToolTip", "处于主动发起构建中,双击可查询实时构建进度");
			OutColor = FLinearColor(0, 1.f, 0, 0.5f);
			break;
		}
		case EAgentStatus::Status_Helping:
		{
			OutText = LOCTEXT("StatusHelping_Text", "Helping");
			OutToolTip = LOCTEXT("StatusHelping_ToolTip", "代理协助其他机器进行构建中");
			OutColor = FLinearColor(0, 0, 1.f, 0.5f);
			break;
		}
		case EAgentStatus::Status_UnCondi:
		{
			OutText = LOCTEXT("CondiNotMet_Text", "UnCondi");
			OutToolTip = LOCTEXT("CondiNotMet_ToolTip", "性能指标达不到系统要求");
			OutColor = FLinearColor(1.f, 1.f, 0, 0.5f);
			break;
		}
		case EAgentStatus::Status_Updating:
		{
			OutText = LOCTEXT("StatusUpdating_Text", "Updateting");
			OutToolTip = LOCTEXT("StatusUpdating_ToolTip", "代理正在安装最新的版本中.");
			OutColor = FLinearColor(0.95f, 0.61f, 0.07f, 0.5f);
			break;
		}
		case EAgentStatus::Status_Offline:
		{
			OutText = LOCTEXT("StatusOffline_Text", "Offline");
			OutToolTip = LOCTEXT("StatusOffline_ToolTip", "代理机器可能掉线，可能代理服务程序没有运行");
			OutColor = FLinearColor(1.f, 0, 0, 0.5f);
			break;
		}
		case EAgentStatus::Status_Stopped:
		{
			OutText = LOCTEXT("StatusStopped_Text", "Stopped");
			OutToolTip = LOCTEXT("StatusStopped_ToolTip", "A Agent form your pool when it has been paused.");
			OutColor = FLinearColor(1.f, 0, 0, 0.5f);
			break;
		}
		case EAgentStatus::Status_Undefined:
		default:
			OutText = LOCTEXT("StatusUndefined_Text", "Undefined");
			OutToolTip = LOCTEXT("StatusUndefined_ToolTip", "当前错误未定义，请查看日志.");
			OutColor = FLinearColor(1.f, 0, 0, 0.5f);
			break;
	}

	if (this->AgentProto.IsValid())
	{
		const auto Proto = this->AgentProto.Pin();
		if (!Proto->benableinitator() && !Proto->benablehelper())
		{
			OutText = LOCTEXT("FreeAgent_Text", "Unassigned");
			OutToolTip = LOCTEXT("FreeAgent_ToolTip", "当前未赋予当前机器任何角色,请设置代理角色功能");
			OutColor = FLinearColor::Yellow;
		}
	}
}

bool SAgentListRow::IsValidIp(const FString& InIp)
{
	FIPv4Address IpV4Address;
	// 为空则让对应的代理自己设置
	return FIPv4Address::Parse(InIp, IpV4Address) || InIp.IsEmpty();
}

bool SAgentListRow::IsValidAddress(const FString& InAddress)
{
	FString IpAddress, PortStr;
	if (InAddress.Split(TEXT(":"), &IpAddress, &PortStr))
	{
		const uint32 Port = FCString::Atoi(*PortStr);
		return (Port > 0 && Port < 65535) && IsValidIp(IpAddress);
	}
	return false;
}

#undef TEXT_BLOCK
#undef NUMEIRC_TEXT_BLOCK
#undef NUMEIRC_LIMIT_TEXT_BLOCK
#undef COMBO_BOX

#undef LOCTEXT_NAMESPACE