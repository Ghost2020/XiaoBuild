/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SLogsView.h"
#include "../Widgets/SAgentExceptionRow.h"
#include "XiaoLog.h"
#include "XiaoShareRedis.h"
#include "exception.pb.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Optional.h"

#define LOCTEXT_NAMESPACE "SLogsView"

using namespace XiaoRedis;

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLogsView::Construct(const FArguments& InArgs)
{
	XIAO_LOG(Log, TEXT("SLogsView::Construct::Begin"));
	GLog->Flush();

	OnQueueNotification = InArgs._OnQueueNotification;

	ChildSlot
		[
			SNew(SVerticalBox)
#pragma region TableBody
				+ SVerticalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill).Padding(25.0f, 10.0f, 50.0f, 5.0f)
				[
					SAssignNew(ExceptionListView, SListView<TSharedPtr<FException>>)
					.ListItemsSource(&ExceptionArray)
					.Orientation(Orient_Vertical)
					.SelectionMode(ESelectionMode::Type::Multi)
					.EnableAnimatedScrolling(true)
					// .ItemHeight(50.0f)
					.AllowOverscroll(EAllowOverscroll::Yes)
					.OnGenerateRow_Raw(this, &SLogsView::OnGenerateRow)
					.OnContextMenuOpening_Raw(this, &SLogsView::OnContextMenuOpening)
					.HeaderRow(
						SNew(SHeaderRow)
						+ SHeaderRow::Column(GExceptTableColumnIDTimestamp)
						.FixedWidth(200.0f)
						.HAlignHeader(EHorizontalAlignment::HAlign_Center)
						.VAlignHeader(VAlign_Center)
						.DefaultLabel(LOCTEXT("TimeStamp_Text", "触发时间"))
						.InitialSortMode(EColumnSortMode::Type::Ascending)
						.OnSort_Raw(this, &SLogsView::OnSortTable)
						.SortMode_Raw(this, &SLogsView::GetSortModeForColumn, GExceptTableColumnIDTimestamp)

						+ SHeaderRow::Column(GExceptTableColumnIDIP)
						.FixedWidth(150.0f)
						.HAlignHeader(EHorizontalAlignment::HAlign_Center)
						.VAlignHeader(VAlign_Center)
						.DefaultLabel(LOCTEXT("IP_Text", "触发IP"))
						.InitialSortMode(EColumnSortMode::Type::Ascending)
						.OnSort_Raw(this, &SLogsView::OnSortTable)
						.SortMode_Raw(this, &SLogsView::GetSortModeForColumn, GExceptTableColumnIDIP)

						+ SHeaderRow::Column(GExceptTableColumnIDContent)
						.FillWidth(.5f)
						.HAlignHeader(EHorizontalAlignment::HAlign_Center)
						.VAlignHeader(VAlign_Center)
						.DefaultLabel(LOCTEXT("Content_Text", "异常描述"))
						.InitialSortMode(EColumnSortMode::Type::Ascending)
					)
				]
#pragma endregion
#pragma region Foot
				+ SVerticalBox::Slot().VAlign(VAlign_Bottom).AutoHeight().Padding(50.0f, 10.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().HAlign(HAlign_Fill)
					[
						SNullWidget::NullWidget
					]
					+ SHorizontalBox::Slot().HAlign(HAlign_Right).Padding(5.0f, 10.0f).AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() {
							return FText::FromString(FString::FromInt(ExceptionArray.Num()) + LOCTEXT("TotalException_Text", " 个异常").ToString());
						})
					]
				]
#pragma endregion
		];

	XIAO_LOG(Log, TEXT("SLogsView::Construct::Finish"));
	GLog->Flush();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLogsView::OnUpdate(const bool bInRebuild) const
{
	if (!IsConnected())
	{
		return;
	}

	try
	{
		bool bNeedRebuild = false;
		std::unordered_map<std::string, std::string> Exceptions;
		std::set<std::string> KeySet;
		SRedisClient->hgetall(Hash::SExceptionDesc, std::inserter(Exceptions, Exceptions.begin()));
		int Index = 0;
		for (const auto& Iter : Exceptions)
		{
			const std::string Key = Iter.first;
			const FString KeyStr = UTF8_TO_TCHAR(Key.c_str());

			if (KeyStr.IsEmpty() || Iter.second.empty())
			{
				continue;
			}

			++Index;

			KeySet.insert(Key);
			const bool bAreayExist = Key2Exception.contains(Key);
			if (!bAreayExist)
			{
				auto Desc = MakeShared<FException>();
				if (Desc->ParsePartialFromString(Iter.second))
				{
					Key2Exception.insert(std::make_tuple(Key, Desc));
					ExceptionArray.Add(Desc);
					bNeedRebuild = true;
				}
			}

			if (bAreayExist)
			{
				auto Row = StaticCastSharedPtr<SAgentExceptionRow>(ExceptionListView->WidgetFromItem(Key2Exception[Key]));
				if (Row.IsValid())
				{
					Row->Invalidate(EInvalidateWidgetReason::Paint);
				}
			}
		}

		// 清理已经删除的数据
		{
			std::set<std::string> NeedDeleteIds;
			for (const auto& Iter : Key2Exception)
			{
				if (!KeySet.contains(Iter.first))
				{
					NeedDeleteIds.insert(Iter.first);
					bNeedRebuild = true;
				}
			}
			for (const std::string& Key : NeedDeleteIds)
			{
				ExceptionArray.Remove(Key2Exception[Key]);
				Key2Exception.erase(Key);
			}
		}

		if (bNeedRebuild)
		{
			ExceptionListView->RequestListRefresh();
		}
	}
	CATCH_REDIS_EXCEPTRION()
}

TSharedRef<ITableRow> SLogsView::OnGenerateRow(const TSharedPtr<FException> InDesc, const TSharedRef<STableViewBase>& InTableView) const
{
	return SNew(SAgentExceptionRow, InTableView).ExceptionDesc(InDesc);
}

TSharedPtr<SWidget> SLogsView::OnContextMenuOpening()
{
	const auto Items = this->ExceptionListView->GetSelectedItems();
	if (Items.Num() > 0)
	{
		FMenuBuilder MenuBuilder(true, nullptr);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteException_Text", "删除异常"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([this]() {
					const auto _Items = this->ExceptionListView->GetSelectedItems();
					IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

					try
					{
						for (int i = _Items.Num()-1; i >= 0; --i)
						{
							const std::string Key = _Items[i]->buildid();
							SRedisClient->hdel(Hash::SExceptionDesc, Key);

							ExceptionArray.Remove(_Items[i]);
							Key2Exception.erase(Key);
						}
					}
					CATCH_REDIS_EXCEPTRION();

					ExceptionListView->RequestListRefresh();
				})
			)
		);

		MenuBuilder.SetSearchable(false);

		return MenuBuilder.MakeWidget();
	}
	return SNullWidget::NullWidget;
}

void SLogsView::OnSortTable(const EColumnSortPriority::Type InPriority, const FName& InName, EColumnSortMode::Type InMode)
{
	static const TArray<FName> ColumnIds =
	{
		GExceptTableColumnIDTimestamp,
		GExceptTableColumnIDIP
	};

	ColumnIdToSort = InName;
	ActiveSortMode = InMode;

	TArray<FName> ColumnIdsBySortOrder = { InName };
	for (const FName& Id : ColumnIds)
	{
		if (Id != InName)
		{
			ColumnIdsBySortOrder.Add(Id);
		}
	}

	ExceptionArray.Sort([ColumnIdsBySortOrder, InMode](const TSharedPtr<FException>& Left, const TSharedPtr<FException>& Right)
	{
		int32 CompareResult = 0;
		for (const FName& ColumnId : ColumnIdsBySortOrder)
		{
			if (ColumnId == GExceptTableColumnIDTimestamp)
			{
				const FString LTimestamp = UTF8_TO_TCHAR(Left->timestamp().c_str());
				const FString RTimestamp = UTF8_TO_TCHAR(Right->timestamp().c_str());
				FDateTime LDateTime, RDateTime;
				FDateTime::Parse(LTimestamp, LDateTime);
				FDateTime::Parse(RTimestamp, RDateTime);
				CompareResult = (LDateTime == RDateTime) ? 0 : (LDateTime < RDateTime ? -1 : 1);
			}
			else if (ColumnId == GExceptTableColumnIDIP)
			{
				CompareResult = Left->exechost().compare(Right->exechost());
			}

			if (CompareResult != 0)
			{
				return InMode == EColumnSortMode::Ascending ? CompareResult < 0 : CompareResult > 0;
			}
		}
		return InMode == EColumnSortMode::Ascending ? true : false;
	});

	ExceptionListView->RequestListRefresh();
}

EColumnSortMode::Type SLogsView::GetSortModeForColumn(const FName InColumnId) const
{
	return (InColumnId == ColumnIdToSort) ? ActiveSortMode : EColumnSortMode::None;
}

#undef LOCTEXT_NAMESPACE