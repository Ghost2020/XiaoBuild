/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SInstallView.h"
#include "SlateOptMacros.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Input/SButton.h"
#include "SGridInstallView.h"
#include "SAgentInstallView.h"
#include "SCustomInstallView.h"
#include "ShareWidget.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "SInstallView"

namespace Xiao
{
	constexpr float ThumbnailWidthSize = 128.0f, ThumbnailHeightSize = 88.0f, ThumbnailPadding = 5.f;
}

class SInstallTile final : public STableRow<TSharedPtr<FInstallDesc>>
{
public:
	SLATE_BEGIN_ARGS( SInstallTile ){}
		SLATE_ARGUMENT(TSharedPtr<FInstallDesc>, Item)
	SLATE_END_ARGS()

private:
	TWeakPtr<FInstallDesc> Item = nullptr;

public:
	/** Static build function */
	static TSharedRef<ITableRow> BuildTile(const TSharedPtr<FInstallDesc> Item, const TSharedRef<STableViewBase>& OwnerTable)
	{
		if (!ensure(Item.IsValid()))
		{
			return SNew(STableRow<TSharedPtr<FInstallDesc>>, OwnerTable);
		}

		return SNew(SInstallTile, OwnerTable).Item(Item);
	}

	/** Constructs this widget with InArgs */
	void Construct( const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable )
	{
		check(InArgs._Item.IsValid())
		Item = InArgs._Item;

		STableRow::Construct(
			STableRow::FArguments()
			.Style(FXiaoStyle::Get(), "ProjectBrowser.TableRow")
			.Padding(2.0f)
			.Content()
			[
				SNew(SBorder)
				.Padding(FMargin(0.0f, 0.0f, 5.0f, 5.0f))
				.BorderImage(FXiaoStyle::Get().GetBrush("ProjectBrowser.ProjectTile.DropShadow"))
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBorder)
						.Padding(FMargin(Xiao::ThumbnailPadding, 0))
						.VAlign(VAlign_Top)
						.Padding(FMargin(3.0f, 3.0f))
						.BorderImage_Lambda(
							[this]()
							{
								const bool bIsSelected = IsSelected();
								const bool bIsRowHovered = IsHovered();

								if (bIsSelected && bIsRowHovered)
								{
									static const FName SelectedHover("ProjectBrowser.ProjectTile.NameAreaSelectedHoverBackground");
									return FXiaoStyle::Get().GetBrush(SelectedHover);
								}
								if (bIsSelected)
								{
									static const FName Selected("ProjectBrowser.ProjectTile.NameAreaSelectedBackground");
									return FXiaoStyle::Get().GetBrush(Selected);
								}
								if (bIsRowHovered)
								{
									static const FName Hovered("ProjectBrowser.ProjectTile.NameAreaHoverBackground");
									return FXiaoStyle::Get().GetBrush(Hovered);
								}

								return FXiaoStyle::Get().GetBrush("ProjectBrowser.ProjectTile.NameAreaBackground");
							}
						)
						[
							SNew(STextBlock)
							.TextStyle(&XiaoH2TextStyle)
							.Text(InArgs._Item->Name)
							.ColorAndOpacity_Lambda
							(
								[this]()
								{
									if (IsSelected() || IsHovered())
									{
										return FStyleColors::White;
									}

									return FSlateColor::UseForeground();
								}
							)
						].HAlign(HAlign_Center).VAlign(VAlign_Center)
					]
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.Visibility(EVisibility::HitTestInvisible)
						.Image_Lambda
						(
							[this]()
							{
								const bool bIsSelected = IsSelected();
								const bool bIsRowHovered = IsHovered();

								if (bIsSelected && bIsRowHovered)
								{
									static const FName SelectedHover("ProjectBrowser.ProjectTile.SelectedHoverBorder");
									return FXiaoStyle::Get().GetBrush(SelectedHover);
								}
								if (bIsSelected)
								{
									static const FName Selected("ProjectBrowser.ProjectTile.SelectedBorder");
									return FXiaoStyle::Get().GetBrush(Selected);
								}
								if (bIsRowHovered)
								{
									static const FName Hovered("ProjectBrowser.ProjectTile.HoverBorder");
									return FXiaoStyle::Get().GetBrush(Hovered);
								}

								return FStyleDefaults::GetNoBrush();
							}
						)
					]
				]
			],
			OwnerTable
		);
	}

private:
	const FSlateBrush* GetThumbnail() const
	{
		const TSharedPtr<FInstallDesc> ItemPtr = Item.Pin();
		if (ItemPtr.IsValid() && ItemPtr->Thumbnail)
		{
			return ItemPtr->Thumbnail;
		}
		return FAppStyle::GetBrush("UnrealDefaultThumbnail");
	}
	
};

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SInstallView::Construct(const FArguments& InArgs)
{
	ConstructWidget();
	
	ChildSlot
	[
		SNew(SVerticalBox)
		TOP_WIDGET

		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().FIR_PADDING
			[
				SNew(STextBlock).Text(LOCTEXT("InstallationOptions_Text", "安装选项"))
			]
			+SVerticalBox::Slot().SEC_PADDING
			[
				SNew(STextBlock).Text(LOCTEXT("SelectOneOf_Text", "选择下述选项中其一"))
			]
		]

		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
		]

		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SSplitter).Orientation(Orient_Horizontal)
			+SSplitter::Slot().Value(0.2f)
			[
				SAssignNew(TitleView, STileView<TSharedPtr<FInstallDesc>>)
				.ListItemsSource(&InstallArray)
				.SelectionMode(ESelectionMode::Single)
				.ClearSelectionOnClick(false)
				.ItemAlignment(EListItemAlignment::LeftAligned)
				.OnGenerateTile_Static(&SInstallTile::BuildTile)
				.ItemHeight(Xiao::ThumbnailHeightSize)
				.ItemWidth(Xiao::ThumbnailWidthSize)
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FInstallDesc> InDesc, ESelectInfo::Type InType)
				{
					if(InDesc.IsValid())
					{
						GInstallSettings.InstallType = InDesc->Type;
						int32 WidgetIndex = 0;
						if (GInstallSettings.InstallType == CT_AgentVisualer)
						{
							WidgetIndex = 0;
						}
						else if (GInstallSettings.InstallType == CT_AgentCoordiVisulizer)
						{
							WidgetIndex = 1;
						}
						else
						{
							WidgetIndex = 2;
						}
						this->Switcher->SetActiveWidgetIndex(WidgetIndex);
					}
				})
			]

			+SSplitter::Slot()
			[
				SAssignNew(Switcher, SWidgetSwitcher)
				+ SWidgetSwitcher::Slot()
				[
					AgentView.ToSharedRef()
				]
				+SWidgetSwitcher::Slot()
				[
					GridView.ToSharedRef()	
				]
				+SWidgetSwitcher::Slot()
				[
					CustomView.ToSharedRef()
				]
			]
		]
	];

	TitleView->SetItemSelection(InstallArray[0], true);
	this->Switcher->SetActiveWidgetIndex(0);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInstallView::ConstructWidget()
{
	InstallArray.Add(MakeShared<FInstallDesc>(EComponentTye::CT_AgentVisualer, LOCTEXT("Agent_Text", "代理"), FXiaoStyle::Get().GetBrush(TEXT("Installer/Agent"))));
	InstallArray.Add(MakeShared<FInstallDesc>(EComponentTye::CT_AgentCoordiVisulizer, LOCTEXT("Network_Text", "网格"), FXiaoStyle::Get().GetBrush(TEXT("Installer/Grid"))));
	InstallArray.Add(MakeShared<FInstallDesc>(EComponentTye::CT_Agent, LOCTEXT("Custom_Text", "自定义"), FXiaoStyle::Get().GetBrush(TEXT("Installer/Custom"))));
	
	if(!GridView.IsValid())
	{
		GridView = SNew(SGridInstallView);
	}
	if(!AgentView.IsValid())
	{
		AgentView = SNew(SAgentInstallView);
	}
	if(!CustomView.IsValid())
	{
		CustomView = SNew(SCustomInstallView);
	}
}

TWeakPtr<SWizardView> SInstallView::GetNext()
{
	if (GInstallSettings.InstallType == CT_AgentVisualer)
	{
		return AgentView->GetNext();
	}
	if(GInstallSettings.InstallType == CT_AgentCoordiVisulizer)
	{
		return GridView->GetNext();
	}
	
	return CustomView->GetNext();
}

bool SInstallView::OnCanNext()
{
	return true;
}

bool SInstallView::OnCanBack()
{
	return true;
}

#undef LOCTEXT_NAMESPACE