/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SDocumentWindow.h"

#include "Misc/FileHelper.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "XiaoShare.h"


#define LOCTEXT_NAMESPACE "SDocumentWindow"

static constexpr int32 GDocumentWindow_Width = 1000;
static constexpr int32 GDocumentWindow_Height = 700;


namespace
{
	const FString SOutlinerField(TEXT("outliner"));
	const FString STextField(TEXT("text"));
	const FString STypeField(TEXT("type"));
	const FString SColorField(TEXT("color"));
	const FString SImageField(TEXT("image"));
	const FString SPaddingField(TEXT("padding"));
	const FString SHAlignField(TEXT("halign"));
	const FString SVAlignField(TEXT("valign"));
	const FString STextStyleField(TEXT("text_style"));
	const FString SContentField(TEXT("content"));
	const FString SHeaderField(TEXT("header"));
	const FString SSlotsField(TEXT("slots"));
	const FString SChildField(TEXT("child"));
}

namespace SWidgetType
{
	const FString SNullWidgetType(TEXT("SNullWidget"));
	const FString STextBlockType(TEXT("STextBlock"));
	const FString SMultilineTextBlockType(TEXT("SMultilineText"));
	const FString SMultilineTextBoxType(TEXT("SMultiLineEditableTextBox"));
	const FString SRichTextBlockType(TEXT("SRichTextBlock"));
	const FString SImageType(TEXT("SImage"));
	const FString SSeparatorType(TEXT("Separator"));
	const FString SVerticalSlotField(TEXT("vertical_slot"));
	const FString SHorizontalSlotField(TEXT("horizontal_slot"));
}

TSharedRef<SWidget> ConstructSlot(const TSharedPtr<FJsonObject>& InSlotObj);

#define SET_SLOT_ATTRIBUTE(Box, SlotObject) \
	Box->AddSlot().Padding(45.0f, 5.0f, 5.0f, 5.0f) \
	[ \
		ConstructSlot(SlotObject) \
	]; \
	auto& Slot = Box->GetSlot(Box->NumSlots() - 1); \
	FString PaddingStr; \
	FMargin Margin(45.0f, 5.0f, 5.0f, 5.0f); \
	if (SlotObject->TryGetStringField(SPaddingField, PaddingStr)) \
	{ \
		FVector4 Padding; \
		Padding.InitFromString(PaddingStr); \
		Margin = FMargin(Padding.X, Padding.Y, Padding.Z, Padding.W); \
	} \
	Slot.SetPadding(Margin); \
	int32 HAlign = 2; \
	if (SlotObject->TryGetNumberField(SHAlignField, HAlign)) \
	{ \
		Slot.SetHorizontalAlignment(static_cast<EHorizontalAlignment>(HAlign)); \
	} \
	int32 VAlign = 2; \
	if (SlotObject->TryGetNumberField(SVAlignField, VAlign)) \
	{ \
		Slot.SetVerticalAlignment(static_cast<EVerticalAlignment>(VAlign)); \
	} \
	

void ConstructSlots(const TSharedPtr<SBoxPanel>& BoxPanel, const TArray<TSharedPtr<FJsonValue>>* InSlotArray)
{
	if (InSlotArray)
	{
		for (const auto& SlotValue : *InSlotArray)
		{
			if (SlotValue.IsValid())
			{
				const auto SlotObject = SlotValue->AsObject();

				// Content
				if (const TSharedPtr<SVerticalBox> VerticalBox = StaticCastSharedPtr<SVerticalBox>(BoxPanel))
				{
					SET_SLOT_ATTRIBUTE(VerticalBox, SlotObject);
					VerticalBox->GetSlot(VerticalBox->NumSlots() - 1).SetAutoHeight();
				}
				else if (const TSharedPtr<SHorizontalBox> HorizontalBox = StaticCastSharedPtr<SHorizontalBox>(BoxPanel))
				{
					SET_SLOT_ATTRIBUTE(HorizontalBox, SlotObject);
					HorizontalBox->GetSlot(VerticalBox->NumSlots() - 1).SetAutoWidth();
				}
			}
		}
	}
}

TSharedRef<SWidget> ConstructContent(const TSharedPtr<FRightItemDesc>& Content)
{
	if (Content.IsValid())
	{
		// Border
		const TSharedPtr<SBorder> Border = SNew(SBorder).HAlign(HAlign_Fill).VAlign(VAlign_Fill).Padding(10.0f);
		const TSharedPtr<SVerticalBox> VerticalBox = SNew(SVerticalBox);
		Border->SetContent(VerticalBox.ToSharedRef());

		if (Content.IsValid())
		{
			const auto ContentJson = Content->ContentObject.Pin();

			// Header
			FString Header;
			if (ContentJson->TryGetStringField(SHeaderField, Header))
			{
				VerticalBox->AddSlot().AutoHeight().Padding(45.0f, 5.0f, 5.0f, 5.0f)
				[
					SNew(STextBlock).Text(FText::FromString(Header)).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
				];

				VerticalBox->AddSlot().AutoHeight().Padding(8.0f, 4.0f, 8.0f, 4.0f)
				[
					SNew(SSeparator)
					.Orientation(Orient_Horizontal)
					.Thickness(1.0f)
				];
			}

			// Slots
			const TArray<TSharedPtr<FJsonValue>>* SlotArray = nullptr;
			if (ContentJson->TryGetArrayField(SSlotsField, SlotArray))
			{
				ConstructSlots(VerticalBox, SlotArray);
			}

			return Border.ToSharedRef();
		}
	}
	return SNullWidget::NullWidget;
}

TSharedRef<SWidget> ConstructSlot(const TSharedPtr<FJsonObject>& InSlotObj)
{
	TSharedPtr<SWidget> WidgetPtr = nullptr;

	FString TextStr;
	InSlotObj->TryGetStringField(STextField, TextStr);
	const FText Text = FText::FromString(TextStr);

	FString WidgetType;
	InSlotObj->TryGetStringField(STypeField, WidgetType);
	if (WidgetType == SWidgetType::STextBlockType)
	{
		WidgetPtr = SNew(STextBlock).Text(Text);
	}
	else if(WidgetType == SWidgetType::SMultilineTextBlockType)
	{
		WidgetPtr = SNew(SMultiLineEditableText).Text(Text).AutoWrapText(true).SelectAllTextWhenFocused(true).IsReadOnly(true);
	}
	else if(WidgetType == SWidgetType::SMultilineTextBoxType)
	{
		WidgetPtr = SNew(SMultiLineEditableTextBox).Text(Text).AutoWrapText(true).SelectAllTextOnCommit(true).IsReadOnly(true);
	}
	else if (WidgetType == SWidgetType::SRichTextBlockType)
	{
		WidgetPtr = SNew(SRichTextBlock).Text(Text);
	}
	else if (WidgetType == SWidgetType::SImageType)
	{
		FString Image;
		if (!InSlotObj->TryGetStringField(SImageField, Image))
		{
			UE_LOG(LogTemp, Warning, TEXT("Image field not found::%s"), *Image);
		}
		WidgetPtr = SNew(SImage).Image(FCoreStyle::Get().GetBrush(FName(Image)));
	}
	else if(WidgetType == SWidgetType::SSeparatorType)
	{
		WidgetPtr = SNew(SSeparator).Thickness(1.0f);
	}
	else if (WidgetType == SWidgetType::SVerticalSlotField || WidgetType == SWidgetType::SHorizontalSlotField)
	{
		const TArray<TSharedPtr<FJsonValue>>* SlotArray = nullptr;
		TSharedPtr<SBoxPanel> Box = nullptr;
		if (InSlotObj->TryGetArrayField(SSlotsField, SlotArray))
		{
			if (SlotArray)
			{
				if (WidgetType == SWidgetType::SVerticalSlotField)
				{
					const TSharedPtr<SVerticalBox> VerticalBox = SNew(SVerticalBox);
					Box = VerticalBox;
				}
				else if (WidgetType == SWidgetType::SHorizontalSlotField)
				{
					const TSharedPtr<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);
					Box = HorizontalBox;
				}

				ConstructSlots(Box, SlotArray);
			}
		}
		WidgetPtr = Box;
	}
	else if (WidgetType == SWidgetType::SNullWidgetType)
	{
		return SNullWidget::NullWidget;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Support this type::%s"), *WidgetType);
		return SNullWidget::NullWidget;
	}

	return WidgetPtr.ToSharedRef();
}


FLeftItemDesc::FLeftItemDesc(TArray<TSharedPtr<FRightItemDesc>>& ContentArray, const TSharedPtr<FJsonObject>& InItemObject, const uint32 InLevel)
	: Level(InLevel)
{
    ConstructItem(ContentArray, InItemObject);
}


void FLeftItemDesc::ConstructItem(TArray<TSharedPtr<FRightItemDesc>>& ContentArray, const TSharedPtr<FJsonObject>& InItemObject)
{
	if (InItemObject.IsValid())
	{
		FString TextStr;
		if (InItemObject->TryGetStringField(STextField, TextStr))
		{
			Text = FText::FromString(TextStr);
		}

		const TSharedPtr<FJsonObject>* ContentObject = nullptr;
		if(InItemObject->TryGetObjectField(SContentField, ContentObject))
		{
			if(ContentObject)
			{
				const auto Index = ContentArray.Add(MakeShared<FRightItemDesc>(*ContentObject));
				Content = ContentArray[Index];
			}
		}

		const TArray< TSharedPtr<FJsonValue> >* ChildrenArray = nullptr;
		if(InItemObject->TryGetArrayField(SChildField, ChildrenArray))
		{
			if(ChildrenArray)
			{
				for(const auto& ChildValue : *ChildrenArray)
				{
					if(ChildValue.IsValid())
					{
						auto ChildObject = ChildValue->AsObject();
						Children.Add(MakeShared<FLeftItemDesc>(ContentArray, ChildObject, Level+1));
					}
				}
			}
		}
	}
}

class SLeftItemRow final :  public SMultiColumnTableRow<TSharedPtr<FLeftItemDesc>>
{
	SLATE_BEGIN_ARGS(SLeftItemRow){}
		SLATE_ARGUMENT(TSharedPtr<FLeftItemDesc>, ItemPtr)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		check(InArgs._ItemPtr.IsValid());
		ItemPtr = InArgs._ItemPtr;
	
		SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTableView);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn( const FName& InColumnName ) override
	{
		if(ItemPtr.IsValid())
		{
			const auto Item = ItemPtr.Pin();
			const auto HorizontalBox = SNew(SHorizontalBox);
			const bool bLeafNode = Item->Children.Num() <= 0;
			if (!bLeafNode)
			{
				HorizontalBox->AddSlot().AutoWidth().Padding(Item->Level * 45.0f, 10.0f, 0.0f, 10.0f)
				[
					SNew(SExpanderArrow, SharedThis(this))
				];
			}

			HorizontalBox->AddSlot().VAlign(VAlign_Center).HAlign(HAlign_Fill)
			[
				SNew(STextBlock).Text(Item->Text)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", Item->Level == 1 ? 20 : 12))
			];

			auto& TextNode = HorizontalBox->GetSlot(HorizontalBox->NumSlots() - 1);
			if (bLeafNode)
			{
				TextNode.SetPadding(FMargin(Item->Level * 45.0f, 15.0f, 0.0f, 15.0f));
			}

			return HorizontalBox;
		}
		return SNullWidget::NullWidget;
	}

private:
	TWeakPtr<FLeftItemDesc> ItemPtr = nullptr;
};

class SContentList final : public STableRow<TSharedPtr<FRightItemDesc>>
{
public:
	SLATE_BEGIN_ARGS( SContentList ){}
	SLATE_ARGUMENT(TSharedPtr<FRightItemDesc>, Content)
SLATE_END_ARGS()

private:
	TWeakPtr<FRightItemDesc> Content = nullptr;

public:
	static TSharedRef<ITableRow> BuildList(const TSharedPtr<FRightItemDesc> InContent, const TSharedRef<STableViewBase>& OwnerTable)
	{
		if (!ensure(InContent.IsValid()))
		{
			return SNew(STableRow<TSharedPtr<FRightItemDesc>>, OwnerTable);
		}
		return SNew(SContentList, OwnerTable).Content(InContent);
	}

	BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
	void Construct( const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable )
	{
		check(InArgs._Content.IsValid())
		Content = InArgs._Content;

		STableRow::Construct(
			STableRow::FArguments()
			.Style(FAppStyle::Get(), "ProjectBrowser.TableRow")
			.Padding(2.0f)
			.Content()
			[
				ConstructContent(Content.Pin())
			],
			OwnerTable
		);
	}
	END_SLATE_FUNCTION_BUILD_OPTIMIZATION
};


FRightItemDesc::FRightItemDesc(const TSharedPtr<FJsonObject>& InContentObject)
	: ContentObject(InContentObject)
{
	
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SDocumentWindow::Construct(const FArguments& Args)
{
	const TSharedRef<SScrollBar> ExternalScrollbar = SNew(SScrollBar);

	SWindow::Construct(SWindow::FArguments()
		.Title(LOCTEXT("WindowTitle", "文档"))
		.ClientSize(FVector2D(GDocumentWindow_Width, GDocumentWindow_Height))
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)
			+SSplitter::Slot().SizeRule(SSplitter::SizeToContent).Value(0.3f).MinSize(0.3f).Resizable(false)
			[
				SNew( SScrollBox ).Orientation(Orient_Vertical)
				+SScrollBox::Slot()
				[
					SAssignNew(TreeView, STreeView<TSharedPtr<FLeftItemDesc>>)
					// .ItemHeight(40.0f)
					.SelectionMode(ESelectionMode::Single)
					.TreeItemsSource(&TreeArray)
					.OnGenerateRow_Raw(this, &SDocumentWindow::OnGenerateRow_LeftTree)
					.OnGetChildren_Static(&SDocumentWindow::OnGetChildren_LeftTree)
					.OnMouseButtonDoubleClick_Raw(this, &SDocumentWindow::OnDoubleClicked)
					.HeaderRow
					(
						SNew(SHeaderRow).Visibility(EVisibility::Collapsed)
						+ SHeaderRow::Column("Item")
						.DefaultLabel(LOCTEXT("ItemHeader", "Item"))
						.FillWidth(1.0)
					)
				]
			]

			+SSplitter::Slot().SizeRule(SSplitter::FractionOfParent).Value(0.7f).MinSize(0.3f)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot().FillHeight(0.9f)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().VAlign(VAlign_Fill)
					[
						SAssignNew(ListView, SListView<TSharedPtr<FRightItemDesc>>)
						.ExternalScrollbar( ExternalScrollbar )
						.Orientation(Orient_Vertical)
						.EnableAnimatedScrolling(true)
						.AllowOverscroll(EAllowOverscroll::Yes)
						.ListItemsSource(&ListArray)
						.SelectionMode(ESelectionMode::Type::None)
						.OnGenerateRow_Static(&SContentList::BuildList)
					]
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						ExternalScrollbar
					]
				]
			]
		]
	);

	ConstructView();

	OnWindowClosed.BindLambda([](const TSharedRef<SWindow>&)
	{
		RequestEngineExit(TEXT("Document Window Request Closed"));
	});
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SDocumentWindow::ConstructView()
{
	TreeArray.Empty();
	ListArray.Empty();

	LoadJson();

	TreeView->RequestTreeRefresh();
	ListView->RequestListRefresh();
}

TSharedRef<ITableRow> SDocumentWindow::OnGenerateRow_LeftTree(const TSharedPtr<FLeftItemDesc> InItem, const TSharedRef<STableViewBase>& InTable) const
{
	return SNew(SLeftItemRow, InTable).ItemPtr(InItem);
}

void SDocumentWindow::OnGetChildren_LeftTree(const TSharedPtr<FLeftItemDesc> InParent, TArray<TSharedPtr<FLeftItemDesc>>& OutChildren)
{
	if(InParent.IsValid())
	{
		OutChildren = InParent->Children;
	}
}

void SDocumentWindow::OnDoubleClicked(const TSharedPtr<FLeftItemDesc> InItemNode) const
{
	if (InItemNode->Children.Num() > 0)
	{
		const bool bIsGroupExpanded = TreeView->IsItemExpanded(InItemNode);
		TreeView->SetItemExpansion(InItemNode, !bIsGroupExpanded);
	}
	else
	{
		if (InItemNode->Content.IsValid())
		{
			const auto Content = InItemNode->Content.Pin();
			ListView->RequestScrollIntoView(Content);
		}
	}
}

void SDocumentWindow::LoadJson()
{
	// 需要转换为config路径下的文档
	static const FString DocumentFile = TEXT("F:/Outline.markdown");
	FString Content;
	if (!FFileHelper::LoadFileToString(Content, *DocumentFile))
	{
		// XIAO_LOG(Error, TEXT("LoadFileToString Failed!"));
		return;
	}

	// TODO 还是要转换为加密的
	/*TArray<uint8> Buffer;
	if (!FFileHelper::LoadFileToArray(Buffer, *DocumentFile))
	{
		XIAO_LOG(Error, TEXT("LoadFileToArray Failed!"));
		return;
	}*/

	/*TArray<uint8> DecompressedBuffer;
	if (!XiaoCompress::Decompress(Buffer, DecompressedBuffer))
	{
		XIAO_LOG(Error, TEXT("Decompress Failed!"));
		return;
	}

	const FString Content = BytesToString(DecompressedBuffer.GetData(), DecompressedBuffer.Num());*/
	//if (!String2Json(Content, RootObject/*, XiaoEncryptKey::SStats*/))
	//{
	//	XIAO_LOG(Error, TEXT("String2Json Failed!"));
	//	return;
	//}
	RootObject = MakeShareable(new FJsonObject);
	const TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(Content);
	FJsonSerializer::Deserialize(Reader, RootObject);

	if (RootObject)
	{
		const TArray< TSharedPtr<FJsonValue> >* ChildrenArray = nullptr;
		if (RootObject->TryGetArrayField(SOutlinerField, ChildrenArray))
		{
			if (ChildrenArray)
			{
				for (const auto& ChildValue : *ChildrenArray)
				{
					if (ChildValue.IsValid())
					{
						auto ChildObject = ChildValue->AsObject();
						const auto Index = TreeArray.Add(MakeShareable(new FLeftItemDesc(ListArray, ChildObject, 1)));
						TreeView->SetItemExpansion(TreeArray[Index], true);
					}
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
