/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -8:15 PM
 */
#include "SNetworkView.h"

#include <ranges>

#include "Misc/MessageDialog.h"
#include "../Widgets/SNetworkListRow.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "XiaoStyle.h"
#include "XiaoLog.h"
#include "XiaoShareNetwork.h"
#include "agent.pb.h"
#include "ShareDefine.h"

#define LOCTEXT_NAMESPACE "SNetworkView"

using namespace XiaoRedis;

static FString LocalIpV4Address = XiaoNetwork::GetLANV4();

SNetworkView::SNetworkView()
{
}

SNetworkView::~SNetworkView()
{
	TryDisconnectRedis();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SNetworkView::Construct(const FArguments& InArgs)
{
	XIAO_LOG(Log, TEXT("SNetworkView::Construct::Begin"));
	GLog->Flush();

	FTextBlockStyle NoResultStyle = XiaoH2TextStyle;
	NoResultStyle.SetColorAndOpacity(XiaoRed);

	ChildSlot
	[
		SNew(SVerticalBox)
#pragma region Table
		+ SVerticalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill).Padding(10.0f, 10.0f, 10.0f, 5.0f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			[
				SAssignNew(NetworkListView, SListView<TSharedPtr<FNetworkConnectivity>>)
				.ListItemsSource(&NetworkArray)
				.Orientation(Orient_Vertical)
				.SelectionMode(ESelectionMode::Type::Multi)
				.EnableAnimatedScrolling(true)
				// .ItemHeight(50.0f)
				.AllowOverscroll(EAllowOverscroll::Yes)
				.OnGenerateRow_Raw(this, &SNetworkView::OnGenerateRow)
				.OnContextMenuOpening_Raw(this, &SNetworkView::OnContextMenu)
				.HeaderRow
				(
					SNew(SHeaderRow)

					+ SHeaderRow::Column(GNetworkColumnIDUpdate)
					.FixedWidth(60.0f)
					.HAlignHeader(EHorizontalAlignment::HAlign_Center)
					.VAlignHeader(VAlign_Center)
					.DefaultLabel(LOCTEXT("Update_Text", "更新测试"))
					.DefaultTooltip(LOCTEXT("Update_Tooltip", "点击按钮进行网络性能测试"))
					.InitialSortMode(EColumnSortMode::Type::Ascending)

					+SHeaderRow::Column(GNetworkColumnIDIcon)
					.FixedWidth(60.0f)
					.HAlignHeader(EHorizontalAlignment::HAlign_Center)
					.VAlignHeader(VAlign_Center)
					.DefaultLabel(LOCTEXT("Icon_Text", "连接状态"))
					.DefaultTooltip(LOCTEXT("Icon_Tooltip", "绿色代表连接是成功的"))
					.InitialSortMode(EColumnSortMode::Type::Ascending)
					.OnSort_Raw(this, &SNetworkView::OnSortTable)
					.SortMode_Raw(this, &SNetworkView::GetSortModeForColumn, GNetworkColumnIDIcon)
					
					+SHeaderRow::Column(GNetworkColumnIDName)
					.FillWidth(.18f)
					.HAlignHeader(EHorizontalAlignment::HAlign_Center)
					.VAlignHeader(VAlign_Center)
					.DefaultLabel(LOCTEXT("AgentName_Text", "代理名称"))
					.DefaultTooltip(LOCTEXT("AgentName_Tooltip", "运行代理的计算机网络名称"))
					.InitialSortMode(EColumnSortMode::Type::Ascending)
					.OnSort_Raw(this, &SNetworkView::OnSortTable)
					.SortMode_Raw(this, &SNetworkView::GetSortModeForColumn, GNetworkColumnIDName)
		
					+SHeaderRow::Column(GNetworkColumnIDStatus)
					.FillWidth(.18f)
					.HAlignHeader(EHorizontalAlignment::HAlign_Center)
					.VAlignHeader(VAlign_Center)
					.DefaultLabel(LOCTEXT("Status_Text", "状态"))
					.DefaultTooltip(LOCTEXT("Status_Tooltip", "代理当前的可用性以及网络适配器的速度"))
					.InitialSortMode(EColumnSortMode::Type::Ascending)
					.OnSort_Raw(this, &SNetworkView::OnSortTable)
					.SortMode_Raw(this, &SNetworkView::GetSortModeForColumn, GNetworkColumnIDStatus)
					
					+SHeaderRow::Column(GNetworkColumnIDPerformance)
					.FillWidth(.18f)
					.HAlignHeader(EHorizontalAlignment::HAlign_Center)
					.VAlignHeader(VAlign_Center)
           		    .DefaultLabel(LOCTEXT("Performance_Text", "网络性能"))
           		    .DefaultTooltip(LOCTEXT("Performance_Tooltip", "对代理的整体网络性能的评估,百分比表示测试中利用了多少网络连接的能力"))
           		    .InitialSortMode(EColumnSortMode::Type::Ascending)
					.OnSort_Raw(this, &SNetworkView::OnSortTable)
					.SortMode_Raw(this, &SNetworkView::GetSortModeForColumn, GNetworkColumnIDPerformance)
		
           		    +SHeaderRow::Column(GNetworkColumnIDReceivePerfor)
					.FillWidth(.18f)
					.HAlignHeader(EHorizontalAlignment::HAlign_Center)
					.VAlignHeader(VAlign_Center)
					.DefaultLabel(LOCTEXT("RecievePerfor_Text", "接收性能"))
					.DefaultTooltip(LOCTEXT("RecievePerfor_Tooltip", "代理接收数据时的速度"))
					.InitialSortMode(EColumnSortMode::Type::Ascending)
					.OnSort_Raw(this, &SNetworkView::OnSortTable)
					.SortMode_Raw(this, &SNetworkView::GetSortModeForColumn, GNetworkColumnIDReceivePerfor)
		
					+SHeaderRow::Column(GNetworkColumnIDSendPerfor)
					.FillWidth(.18f)
					.HAlignHeader(EHorizontalAlignment::HAlign_Center)
					.VAlignHeader(VAlign_Center)
					.DefaultLabel(LOCTEXT("SendPerfor_Text", "发送性能"))
					.DefaultTooltip(LOCTEXT("SendPerfor_Tooltip", "代理发送数据时的速度"))
					.InitialSortMode(EColumnSortMode::Type::Ascending)
					.OnSort_Raw(this, &SNetworkView::OnSortTable)
					.SortMode_Raw(this, &SNetworkView::GetSortModeForColumn, GNetworkColumnIDSendPerfor)
		
					+SHeaderRow::Column(GNetworkColumnIDRoundTrip)
					.FillWidth(.18f)
					.HAlignHeader(EHorizontalAlignment::HAlign_Center)
					.VAlignHeader(VAlign_Center)
					.DefaultLabel(LOCTEXT("RoundTripTime_Text", "回环时间"))
					.DefaultTooltip(LOCTEXT("RoundTripTime_Tooltip", "整个测试往返所用的平均时间(发送+接受)。还显示了所有思想测试的结果"))
					.InitialSortMode(EColumnSortMode::Type::Ascending)
					.OnSort_Raw(this, &SNetworkView::OnSortTable)
					.SortMode_Raw(this, &SNetworkView::GetSortModeForColumn, GNetworkColumnIDRoundTrip)
					
					+SHeaderRow::Column(GNetworkColumnIDIPAddress)
					.FixedWidth(130.0f)
					.HAlignHeader(EHorizontalAlignment::HAlign_Center)
					.VAlignHeader(VAlign_Center)
					.DefaultLabel(LOCTEXT("IPAddress_Text", "IP地址"))
					.DefaultTooltip(LOCTEXT("IPAddress_Tooltip", "机器的IP地址和端口"))
				)
			]
			+ SOverlay::Slot()
			[
				SNew(SHorizontalBox).Visibility_Lambda([this]()
				{
					return this->NetworkArray.Num() > 0 ? EVisibility::Collapsed : EVisibility::Visible;
				})
				+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(50.0f, 20.0f, 50.0f, 20.0f)
					[
						SNew(SImage).Image(FXiaoStyle::Get().GetBrush("empty_search"))
					]
					+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(50.0f, 20.0f, 50.0f, 20.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("NoAgentFound_Text", "当前没有查询到已订阅的代理机器"))
						.ToolTipText(LOCTEXT("NoAgentFound_ToolTip", "确认是否连接到协调器服务，相关的代理设置是否正确！"))
						.TextStyle(&NoResultStyle)
					]
				]
			]
		]	
#pragma endregion
#pragma region Foot
		+ SVerticalBox::Slot().AutoHeight()
		.VAlign(VAlign_Bottom)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().HAlign(HAlign_Left)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock).Text_Lambda([this]() {
							return FText::FromString(FString::FromInt(NetworkArray.Num()));
							})
					]
					+SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("NetworkFoot_Text", " 个代理"))
					]
				]
	
				+SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock).Text_Lambda([this]() {
							return FText::FromString("," + FString::FromInt(NetworkListView->GetNumItemsSelected()));
							})
					]
					+SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("SelectedNetwork_Text", " 个选中"))
					]
				]
			]
		]
#pragma endregion
#pragma region ToolZTip
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(0.0f)
		[
			SNew(SBorder)
			.BorderImage(FXiaoStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.05f, 0.1f, 0.2f, 1.0f))
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Margin(FMargin(4.0f, 1.0f, 4.0f, 1.0f))
				.ColorAndOpacity(FLinearColor(1.0f, 0.75f, 0.5f, 1.0f))
				.Text_Lambda([this]()
				{
					return (SRedisStatus != ERedisStatus::Redis_Ok) ?
						LOCTEXT("NotConnectToCoordinator_Text", "-- 当前环境没有连接到协调器! --") :
						LOCTEXT("NotAvaluableAgent_Text", "-- 当前没有可用的协助者 --");
				})
				.ToolTipText_Lambda([this]() 
				{
					return (NetworkArray.Num() == 0) ?
						LOCTEXT("NotConnectToCoordinator_ToolTip", "打开代理设置面板，进行网络设置") :
						LOCTEXT("NotAvaluableAgent_ToolTip", "如果有多余的机器可以使用请加入到联合编译当中");
				})
				.Visibility_Lambda([this]()
				{
					return (SRedisStatus != ERedisStatus::Redis_Ok || NetworkArray.Num() == 0) ? EVisibility::Visible : EVisibility::Collapsed;
				})
			]
		]
#pragma endregion
	];

	XIAO_LOG(Log, TEXT("SNetworkView::Construct::Finish"));
	GLog->Flush();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SNetworkView::UpdateNetwork(const bool bTest)
{
	if (!IsConnected())
	{
		XIAO_LOG(Error, TEXT("Can\'t connect Redis server::%s:%u!"), UTF8_TO_TCHAR(GMasterConnection.host.c_str()), GMasterConnection.port);
		LoadAgentSettings(SOriginalAgentSettings);
		const auto& NetworkCoordi = SOriginalAgentSettings.NetworkCoordinate;
		GMasterConnection.host = TCHAR_TO_UTF8(*NetworkCoordi.IP);
		GMasterConnection.port = NetworkCoordi.Port;
		GMasterConnection.keep_alive = true;
		if (!XiaoRedis::TryConnectRedis())
		{
			return;
		}
	}

	try
	{
		bool NeedRebuild = false;
		std::unordered_map<std::string, std::string> AgentStats;
		SRedisClient->hgetall(Hash::SAgentStats, std::inserter(AgentStats, AgentStats.begin()));
		TSet<FString> IPSet;
		for (const auto& Iter : AgentStats)
		{
			const std::string UniqueId = Iter.first;

			if (UniqueId.empty())
			{
				XIAO_LOG(Error, TEXT("UpdateNetwork::AgentId is empty!"));
				continue;
			}

			FAgentProto Proto;
			if (!Proto.ParseFromString(Iter.second))
			{
				XIAO_LOG(Error, TEXT("UpdateNetwork::ParseFromString failed!"));
				continue;
			}

			const FString& AgentIp = UTF8_TO_TCHAR(Proto.routerip().c_str());
			if (AgentIp.IsEmpty() || AgentIp == LocalIpV4Address)
			{
				XIAO_LOG(Warning, TEXT("UpdateNetwork::AgentIp is empty or is local!"));
				continue;
			}

			XIAO_LOG(Log, TEXT("UPdateNetwork::%s"), *AgentIp);

			IPSet.Add(AgentIp);
			bool NewAdd = false;
			if (!Ip2Desc.Contains(AgentIp))
			{
				NeedRebuild = true;
				auto Item = MakeShared<FNetworkConnectivity>();
				Item->Name = UTF8_TO_TCHAR(Proto.loginuser().c_str());
				Item->RemoteConnection = AgentIp;
				NetworkArray.Add(Item);
				Ip2Desc.Add(MakeTuple(AgentIp, Item));
				NewAdd = true;
			}

			if (bTest || NewAdd)
			{
				Ip2Desc[AgentIp]->OnNetworkTest();
			}
		}

		TSet<FString> NeedDelete;
		for (const auto& Iter : Ip2Desc)
		{
			if (!IPSet.Contains(Iter.Key))
			{
				NeedDelete.Add(Iter.Key);
				NeedRebuild = true;
			}
		}
		for (const FString& Ip : NeedDelete)
		{
			NetworkArray.Remove(Ip2Desc[Ip]);
			Ip2Desc.Remove(Ip);
		}

		if (NeedRebuild)
		{
			NetworkListView->RequestListRefresh();
		}
	}
	CATCH_REDIS_EXCEPTRION();
}

TSharedRef<ITableRow> SNetworkView::OnGenerateRow(const TSharedPtr<FNetworkConnectivity> InDesc, const TSharedRef<STableViewBase>& InTableView) const
{
	return SNew(SNetworkListRow, InTableView).NetworkDesc(InDesc);
}

TSharedPtr<SWidget> SNetworkView::OnContextMenu() const
{
	const auto Items = this->NetworkListView->GetSelectedItems();
	FMenuBuilder MenuBuilder(true, nullptr);
	if (Items.Num() > 0)
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("NetworkTest_Text", "网络通讯测试"),
			LOCTEXT("NetworkTest_Tooltip", "重新启动一个程序来显示所选记录"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([Items, this]()
				{
					for (const auto& Iter : Items)
					{
						if (Iter.IsValid())
						{
							Iter->OnNetworkTest();
						}
					}
				})
			)
		);

		MenuBuilder.SetSearchable(false);
	}

	return MenuBuilder.MakeWidget();
}

void SNetworkView::OnSortTable(const EColumnSortPriority::Type InPriority, const FName& InName, EColumnSortMode::Type InMode)
{
	static const TArray<FName> ColumnIds =
	{
		GNetworkColumnIDName,
		GNetworkColumnIDStatus,
		GNetworkColumnIDPerformance,
		GNetworkColumnIDReceivePerfor,
		GNetworkColumnIDSendPerfor,
		GNetworkColumnIDRoundTrip,
		GNetworkColumnIDIPAddress
	};

	ColumnIdToSort = InName;
	ActiveSortMode = InMode;

	TArray<FName> ColumnIdsBySortOrder = {InName};
	for (const FName& Id : ColumnIds)
	{
		if (Id != InName)
		{
			ColumnIdsBySortOrder.Add(Id);
		}
	}

	NetworkArray.Sort([ColumnIdsBySortOrder, InMode](const TSharedPtr<FNetworkConnectivity>& Left, const TSharedPtr<FNetworkConnectivity>& Right)
	{
		int32 CompareResult = 0;
		for (const FName& ColumnId : ColumnIdsBySortOrder)
		{
			if (ColumnId == GNetworkColumnIDStatus)
			{
				CompareResult = (Left->Status == Right->Status) ? 0 : (Left->Status < Right->Status ? -1 : 1);
			}
			else if (ColumnId == GNetworkColumnIDName)
			{
				CompareResult = Left->Name.Compare(Right->Name);
			}
			
			else if(ColumnId == GNetworkColumnIDPerformance)
			{
				CompareResult = (Left->Performance == Right->Performance) ? 0 : (Left->Performance < Right->Performance ? -1 : 1);
			}
			else if(ColumnId == GNetworkColumnIDReceivePerfor)
			{
				CompareResult = (Left->ReceivePerfor == Right->ReceivePerfor) ? 0 : (Left->ReceivePerfor < Right->ReceivePerfor ? -1 : 1);
			}
			else if(ColumnId == GNetworkColumnIDSendPerfor)
			{
				CompareResult = (Left->SendPerfor == Right->SendPerfor) ? 0 : (Left->SendPerfor < Right->SendPerfor ? -1 : 1);
			}
			else if(ColumnId == GNetworkColumnIDRoundTrip)
			{
				CompareResult = (Left->RoundTripTime == Right->RoundTripTime) ? 0 : (Left->RoundTripTime < Right->RoundTripTime ? -1 : 1);
			}
			
			if (CompareResult != 0)
			{
				return (InMode == EColumnSortMode::Ascending) ? (CompareResult < 0) : (CompareResult > 0);
			}
		}
		return InMode == EColumnSortMode::Ascending ? true : false;
	});

	NetworkListView->RequestListRefresh();
}

EColumnSortMode::Type SNetworkView::GetSortModeForColumn(const FName InColumnId) const
{
	return (InColumnId == ColumnIdToSort) ? ActiveSortMode : EColumnSortMode::None;
}

#undef LOCTEXT_NAMESPACE