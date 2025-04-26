/**
  * @author cxx2020@outlook.com
  * @date 5:57 PM
 */

#include "SAddUserDialog.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "SlateOptMacros.h"
#include "XiaoStyle.h"
#include "XiaoShare.h"


#define LOCTEXT_NAMESPACE "SAddUserDialog"

static const FText UserError = LOCTEXT("UsernameError_Text", "账户不能包含特殊字符");
static const FText Less8 = LOCTEXT("Less8Cha_Text", "密码不少于8位字符,且至少有一位大写字符");
static const FText NotEqual = LOCTEXT("NotEqual_Text", "密码不一致");

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAddUserDialog::Construct(const FArguments& InArgs)
{
	OnAddUser = InArgs._OnAddUser;
	SWindow::Construct(SWindow::FArguments()
	.bDragAnywhere(false)
	.HasCloseButton(true)
	.SupportsMaximize(false)
	.SupportsMinimize(false)
	.SizingRule(ESizingRule::FixedSize)
	.AutoCenter(EAutoCenter::PreferredWorkArea)
	.Title(LOCTEXT("ForgetUsername_Title", "添加用户"))
	.ClientSize(FVector2D(400, 200))
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Fill)
		[
			SNew(SGridPanel)
			.FillColumn(2, 1.0f)
			.FillRow(3, 1.0f)
			+ SGridPanel::Slot(0, 0).HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			[
				SAssignNew(Username, SEditableTextBox)
				.HintText(LOCTEXT("Username_Text", "用户名"))
				.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type)
				{
					bUser = false;
					const FString User = InText.ToString();
					if (StringContainSpecialCase(User))
					{
						Username->SetError(UserError);
						return;
					}

					Username->SetError(TEXT(""));
					UserDesc.Username = User;
					bUser = true;
				})
			]
			+ SGridPanel::Slot(1, 0).HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			[
				SAssignNew(RoleComboBox, SComboBox<TSharedPtr<FText>>)
				.OptionsSource(&GRoleArray)
				.InitiallySelectedItem(GRoleArray[0])
				.OnGenerateWidget_Lambda([](const TSharedPtr<FText>& InText)
				{
					return SNew(STextBlock).Text(*InText);
				})
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FText>& InText, ESelectInfo::Type InType)
				{
					this->RoleComboBox->SetSelectedItem(InText);
					UserDesc.Role = GRoleArray.Find(InText);
				})
				.Content()
				[
					SNew(STextBlock).Text_Lambda([this]()
					{
						return *this->RoleComboBox->GetSelectedItem();
					})
				]
			]

			+ SGridPanel::Slot(0, 1).HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			[
				SAssignNew(Password, SEditableTextBox)
				.HintText(LOCTEXT("Password_Text", "密码"))
				.IsPassword(true)
				.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type)
				{
					bPassword = false;
					const FString Pass = Password->GetText().ToString();
					if (Pass.Len() < 8 || !StringContainUpperCase(Pass))
					{
						Password->SetError(Less8);
						return;
					}
					const FString RePass = RePassword->GetText().ToString();
					if (Pass != RePass)
					{
						RePassword->SetError(NotEqual);
						return;
					}

					Password->SetError(TEXT(""));
					UserDesc.Password = Pass;
					bPassword = true;
				})
			]
			+ SGridPanel::Slot(1, 1).HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			[
				SAssignNew(RePassword, SEditableTextBox)
				.HintText(LOCTEXT("PasswordRepeat_Text", "确定密码"))
				.IsPassword(true)
				.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type)
				{
					bPassword = false;
					const FString RePass = InText.ToString();
					if (RePass.Len() < 8 || !StringContainUpperCase(RePass))
					{
						RePassword->SetError(Less8);
						return;
					}

					const FString Pass = Password->GetText().ToString();
					if (Pass != RePass)
					{
						RePassword->SetError(NotEqual);
						return;
					}

					RePassword->SetError(TEXT(""));
					bPassword = true;
				})
			]
		]
		+SVerticalBox::Slot().VAlign(VAlign_Bottom)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().HAlign(HAlign_Fill)
			[
				SNullWidget::NullWidget
			]
			+SHorizontalBox::Slot().HAlign(HAlign_Right).AutoWidth()
			[
				SNew(SButton).ButtonColorAndOpacity(FStyleColors::AccentGreen)
				.Text(LOCTEXT("AddUser_Text", "添加"))
				.OnPressed_Lambda([this]()
				{
					UserDesc.Username = Username->GetText().ToString();
					UserDesc.Password = Password->GetText().ToString();
					UserDesc.LastLogin = FDateTime::Now();
					UserDesc.LoginMachine = GetUniqueDeviceID();
					(void)OnAddUser.ExecuteIfBound(UserDesc);
				})
				.IsEnabled_Raw(this, &SAddUserDialog::OnCanAdd)
			]
		]
	]);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

bool SAddUserDialog::OnCanAdd() const
{
	return bUser && bPassword;
}

#undef LOCTEXT_NAMESPACE