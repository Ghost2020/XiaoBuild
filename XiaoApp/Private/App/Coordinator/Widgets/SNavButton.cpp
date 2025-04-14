/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SNavButton.h"
#include "XiaoStyle.h"
#include "SlateOptMacros.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SNavButton::Construct(const FArguments& InArgs)
{
	NormalImage = FXiaoStyle::Get().GetBrush(InArgs._NormalImage);
	if (!InArgs._SeletedImage.IsNone())
	{
		SelectedImage = FXiaoStyle::Get().GetBrush(InArgs._SeletedImage);
	}
	ChildSlot
	[
		SNew(SButton).ButtonColorAndOpacity(FLinearColor::Transparent)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(HAlign_Left).Padding(25.0f, 5.0f, 10.0f, 3.0f).AutoWidth()
			[
				SAssignNew(Image, SImage)
				.Image(NormalImage)
			]
	
			+ SHorizontalBox::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center).AutoWidth()
			[
				SAssignNew(Text, STextBlock)
				.Text(InArgs._Text)
			]
		]
		.OnPressed(InArgs._OnPressed)
	];

	this->SetOnMouseEnter(FNoReplyPointerEventHandler::CreateLambda([this](const FGeometry& InGeometry, const FPointerEvent& InEvent) {
		this->SetCursor(EMouseCursor::Type::Hand);
	}));

	this->SetOnMouseLeave(FSimpleNoReplyPointerEventHandler::CreateLambda([this](const FPointerEvent& InEvent) {
		this->SetCursor(EMouseCursor::Type::Default);
	}));
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SNavButton::SetSelect(const bool bInSelect)
{
	const auto Color = bInSelect ? XiaoGreen : FLinearColor::White;
	if (SelectedImage)
	{
		Image->SetImage(bInSelect ? SelectedImage : NormalImage);
	}
	else
	{
		Image->SetColorAndOpacity(Color);
	}
	Text->SetColorAndOpacity(Color);
}
