/**
  * @author cxx2020@outlook.com
  * @date 5:57 PM
 */

#include "SForgetDialog.h"
#include "SlateOptMacros.h"
#include "SSimpleButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Text/STextBlock.h"
#include "XiaoStyle.h"
#include "XiaoShare.h"

#define LOCTEXT_NAMESPACE "SForgetDialog"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SForgetDialog::Construct(const FArguments& InArgs)
{
	SWindow::Construct(SWindow::FArguments()
	.bDragAnywhere(false)
	.CreateTitleBar(false)
	.HasCloseButton(false)
	.SupportsMaximize(false)
	.SupportsMaximize(false)
	.SizingRule(ESizingRule::FixedSize)
	.AutoCenter(EAutoCenter::PreferredWorkArea)
	.Title(LOCTEXT("AddNewUser_Title", "添加用户"))
	.ClientSize(FVector2D(400, 200))
	[
		SNew(SVerticalBox)

		+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(50.0f, 30.0f)
		[
			SNew(STextBlock).TextStyle(&XiaoH2TextStyle).Text(LOCTEXT("ForgetUsername_Text", "忘记账户或密码"))
		]

		+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(50.0f, 10.0f)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().HAlign(HAlign_Center)
			[
				SNew(STextBlock).Text(LOCTEXT("Solve_Text", "为了修改你的账户/密码请参考我们的文档"))
			]
			+SHorizontalBox::Slot().HAlign(HAlign_Center).AutoWidth().Padding(5.0f, 0.0f)
			[
				SNew(SHyperlink).Text(LOCTEXT("Here_Text", "点此"))
					.OnNavigate_Lambda([]() {
						GetHelp(GLocalization == TEXT("zh-CN") ? TEXT("修改密码") : TEXT(""));
					})
			]
		]
		
		+SVerticalBox::Slot().Padding(30.0f).AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SButton).Text(LOCTEXT("Got_Text", "了解"))
			.OnPressed_Lambda([this]()
			{
				this->RequestDestroyWindow();
			})
		]
	]);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE