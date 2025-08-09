/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SLoginView.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "XiaoStyle.h"
#include "ShareWidget.h"
#include "Widgets/Notifications/SErrorText.h"
#include "SInstallProgressView.h"

#define LOCTEXT_NAMESPACE "SLoginView"

static const FText SUserError = LOCTEXT("MinimalUsername_Text", "最少4个字符，不允许特殊字符和空格");

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLoginView::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(ErrorText, SErrorText)
		]
		
		TOP_WIDGET
			
		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().FIR_PADDING
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LoginInfor_Text", "登录信息"))
			]

			+SVerticalBox::Slot().SEC_PADDING
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EnterCredentials_Text", "输入许可用于登录调度器"))
			]
		]

		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
		]

		+SVerticalBox::Slot().THR_PADDING
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(LOCTEXT("Username_Text", "账户"))
			]
			+SHorizontalBox::Slot().MaxWidth(300.0f)
			[
				SAssignNew(UsernameTextBox, SEditableTextBox)
				.Text(FText::FromString(TEXT("admin")))
				.OnTextChanged_Raw(this, &SLoginView::OnUsernameChanged)
				.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type)
				{
					OnUsernameChanged(InText);
				})
			]
		]

		+SVerticalBox::Slot().THR_PADDING
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(LOCTEXT("Password_Text", "密码"))
			]
			+SHorizontalBox::Slot().MaxWidth(300.0f)
			[
				SAssignNew(PasswordTextBox, SEditableTextBox)
				.IsPassword(true)
				// .OnTextChanged_Raw(this, &SLoginView::OnPasswordChanged)
				.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type)
				{
					OnPasswordChanged(InText);
				})
			]
		]
		+ SVerticalBox::Slot().THR_PADDING
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(LOCTEXT("RePassword_Text", "再次"))
			]
			+ SHorizontalBox::Slot().MaxWidth(300.0f)
			[
				SAssignNew(RePasswordTextBox, SEditableTextBox)
				.IsPassword(true)
				// .OnTextChanged_Raw(this, &SLoginView::OnRePasswordChanged)
				.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type)
				{
					OnRePasswordChanged(InText);
				})
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SLoginView::GetNext()
{
	if(!ProgressView.IsValid())
	{
		ProgressView = SNew(SInstallProgressView);
	}
	return ProgressView;
}

bool SLoginView::OnCanNext()
{
	if(bIsValidUsername && bIsValidPassword && bIsSamePassword)
	{
		// TODO 最好能够提示用户 检查对应的程序是否有在运行
		return true;
	}
	return false;
}

void SLoginView::OnUsernameChanged(const FText& InText)
{
	const FString Username = InText.ToString();
	bIsValidUsername = Username.Len() >= 4 && !StringContainSpecialCase(Username);
	UsernameTextBox->SetError(bIsValidUsername ? FText::GetEmpty() : SUserError);
}

void SLoginView::OnPasswordChanged(const FText& InText)
{
	const FString Password = InText.ToString();
	bIsValidPassword = Password.Len() >= 8 && StringContainUpperCase(Password);
	FText _ErrorText;
	if (!bIsValidPassword)
	{
		_ErrorText = LOCTEXT("Minimal_Text", "最少8个字符，至少包含一个大写字符");
	}
	FSlateApplication& App = FSlateApplication::Get();
	if (App.GetModifierKeys().AreCapsLocked())
	{
		_ErrorText = LOCTEXT("CapsLocked_Text", "大写键开启中");
	}
	PasswordTextBox->SetError(_ErrorText);
}

void SLoginView::OnRePasswordChanged(const FText& InText)
{
	const FString RePassword = InText.ToString();
	bIsValidPassword = RePassword.Len() >= 8 && StringContainUpperCase(RePassword);
	if (bIsValidPassword)
	{
		const FString Password = PasswordTextBox->GetText().ToString();
		bIsSamePassword = Password == RePassword;
		GInstallSettings.Password = Password;
	}
					
	FText _ErrorText;
	if (!bIsSamePassword)
	{
		_ErrorText = LOCTEXT("NotSame_Text", "密码不一致");
	}
	RePasswordTextBox->SetError(_ErrorText);
}

#undef LOCTEXT_NAMESPACE