/**
  * @author cxx2020@outlook.com
  * @date 5:57 PM
 */
#include "SManageBuildGroups.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "SSimpleButton.h"
#include "../Widgets/SConstrainBox.h"
#include "XiaoShare.h"
#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "SManageBuildGroups"

static const FName SGroupNameId = TEXT("Name");
static const FName SAssignedNumId = TEXT("AssignedAgents");
static const FText SRepeatGroup = LOCTEXT("BuildGruopAlreadyExists_Text", "同名的群组已经存在!");

static TMap<SGroupItemRow*, TWeakPtr<FBuildGroup>> View2Logic;
static TWeakPtr<SGroupItemRow> LastSelected = nullptr;

static SGroupItemRow* FindRowByGroup(const FBuildGroup* InGroup)
{
	for(const auto& Iter : View2Logic)
	{
		if(auto Widget = Iter.Value.Pin(); Widget.IsValid() && InGroup == Widget.Get())
		{
			return Iter.Key;
		}
	}
	return nullptr;
}

void SGroupItemRow::Construct(const FArguments& InArgs, TSharedPtr<FBuildGroup> InRowData, const TSharedRef<STableViewBase>& InOwnerTable)
{
	View2Logic.Add(MakeTuple(this, InRowData));
	WeakRow = InRowData;
	SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTable);
}

TSharedRef<SWidget> SGroupItemRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if(ColumnName == SGroupNameId)
	{
		return SAssignNew(NameBox, SEditableTextBox)
		.Text(FText::FromString(WeakRow.Pin()->Name))
		.OnVerifyTextChanged_Lambda([&, this] (const FText& InText, FText& OutText)
		{
			int SameIndex = 0;
			for(const auto& Iter : View2Logic)
			{
				if(InText.ToString() == Iter.Value.Pin()->Name)
				{
					SameIndex++;		
				}
			}
			if (SameIndex == 2)
			{
				SetHintText(SRepeatGroup);
				return false; 
			}

			SetHintText(FText::GetEmpty());
			for (const auto& Iter : View2Logic)
			{
				if (Iter.Key == this)
				{
					Iter.Value.Pin()->Name = InText.ToString();
					return true;
				}
			}
			return false;
		})
		.OnTextCommitted_Lambda([this] (const FText& InText, ETextCommit::Type InType)
		{
			SetHintText(FText::GetEmpty());
			const FString GroupStr = InText.ToString();
			if (GroupStr.IsEmpty())
			{
				return;
			}
			for (const auto& Group : GGroupArray)
			{
				if (Group.IsValid())
				{
					if (GroupStr == *Group)
					{
						return;
					}
				}
			}
			GGroupArray.Add(MakeShared<FString>(GroupStr));
		});
	}
	if(ColumnName == SAssignedNumId)
	{
		return SNew(SOverlay)
		+SOverlay::Slot().HAlign(HAlign_Left)
		[
			SAssignNew(NumBlock, STextBlock)
			.Text(FText::FromString(FString::FromInt(WeakRow.Pin()->AssignedNum)))
		]
		+SOverlay::Slot().HAlign(HAlign_Right)
		[
			SAssignNew(DeleteButton, SButton).Visibility(EVisibility::Hidden)
			.OnPressed_Lambda([] ()
			{
				
			})
			.Content()
			[
				SNew(SImage).Image(FXiaoStyle::Get().GetBrush(TEXT("delete")))
			]
		];
	}
	return SNullWidget::NullWidget;
}

void SGroupItemRow::SetHintText(const FText& InError) const
{
	NameBox->SetError(InError);
}

void SGroupItemRow::SetSelected(const bool bSelected)
{
	if(LastSelected.IsValid() && LastSelected.Pin().Get() != this)
	{
		LastSelected.Pin()->SetSelected(false);
	}
	if (DeleteButton.IsValid())
	{
		DeleteButton->SetVisibility(bSelected ? EVisibility::Visible : EVisibility::Hidden);
	}
	LastSelected = SharedThis(this);
}

SManageBuildGroups::~SManageBuildGroups()
{
	View2Logic.Empty();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SManageBuildGroups::Construct(const FArguments& InArgs)
{
	for(const auto& Group : GGroupArray)
	{
		BuildGroupsArray.Add(MakeShared<FBuildGroup>(*Group, 1));
	}
	
	SWindow::Construct(SWindow::FArguments()
	.HasCloseButton(true)
	.SupportsMaximize(false)
	.SupportsMinimize(false)
	.SizingRule(ESizingRule::FixedSize)
	.AutoCenter(EAutoCenter::PreferredWorkArea)
	.Title(LOCTEXT("ManageBuildGroups_Title", "管理构建群组"))
	.ClientSize(FVector2D(500, 400))
	[
		SNew(SVerticalBox)
#pragma region Header
		+SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Top)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SConstrainBox).MinWidth(200.0f).MaxWidth(200.0f)
				[
					SNew(SSearchBox).HintText(LOCTEXT("Search_Text", "搜索"))
				]
			]
			
			+SHorizontalBox::Slot().HAlign(HAlign_Right)
			[
				SNew(SSimpleButton).Text(LOCTEXT("AddBuildGroup_Text", "添加构建分组")).Icon(FXiaoStyle::Get().GetBrush("Icons.Add"))
				.OnClicked_Lambda([&]()
				{
					if(BuildGroupsArray.Num() > 1)
					{
						const FString NewGroupName = BuildGroupsArray.Last()->Name;
						if (NewGroupName.IsEmpty())
						{
							for(const auto& Iter : View2Logic)
							{
								if(NewGroupName == Iter.Value.Pin()->Name)
								{
									Iter.Key->SetHintText(LOCTEXT("CanBeEmpty", "不能为空"));
									GroupListView->RequestListRefresh();
									return FReply::Unhandled();
								}
							}
						}
					}
					BuildGroupsArray.Add(MakeShared<FBuildGroup>(TEXT(""), 0));
					GroupListView->RequestListRefresh();
					return FReply::Handled();
				})
			]
		]
#pragma endregion

#pragma region TableBody
		+SVerticalBox::Slot().VAlign(VAlign_Fill)
		[
			SNew(SScrollBox).Orientation(Orient_Vertical)
			+ SScrollBox::Slot()
			[
				SAssignNew(GroupListView, SListView<TSharedPtr<FBuildGroup>>)
				.ListItemsSource(&BuildGroupsArray)
				// .ItemHeight(20)
				.SelectionMode(ESelectionMode::Single)
				.ClearSelectionOnClick(false)
				.OnSelectionChanged_Lambda([] (const TSharedPtr<FBuildGroup>& InGroup, ESelectInfo::Type InType)
				{
					if(const auto Row = FindRowByGroup(InGroup.Get()))
					{
						Row->SetSelected(true);
					}
				})
				.HeaderRow
				(
				 	SNew(SHeaderRow)
				 	+SHeaderRow::Column(SGroupNameId)
				 	.DefaultLabel(LOCTEXT("HeaderName_Text", "名称"))
	
				 	+SHeaderRow::Column(SAssignedNumId)
					.DefaultLabel(LOCTEXT("AgentNum_Text", "对应代理数量"))
				)
				.OnGenerateRow_Raw(this, &SManageBuildGroups::OnGenerateRow)
			]
		]
#pragma endregion

#pragma region Foot
		/*+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(3.0f, 0.0f)
			[
				SNew(SSimpleButton)
				.Text(LOCTEXT("SFbxMaterialConflictWindow_Reset", "Reset To Fbx"))
				.Icon(FXiaoStyle::Get().GetBrush(TEXT("Filter")))
				.OnClicked_Lambda([]()
				{
					return FReply::Handled();
				})
			]
		]*/
#pragma endregion
	]);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<ITableRow> SManageBuildGroups::OnGenerateRow(TSharedPtr<FBuildGroup> BuildItem,
	const TSharedRef<STableViewBase>& OwnerTable) const
{
	auto Widget = SNew(SGroupItemRow, BuildItem, OwnerTable);
	Widget->SetSelected(true);
	return Widget;
}

#undef LOCTEXT_NAMESPACE
