/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SLoginWindow.h"
#include "SlateOptMacros.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Notifications/SErrorHint.h"
#include "Dialogs/SForgetDialog.h"
#include "XiaoShareField.h"
#include "XiaoInterprocess.h"
#include "XiaoShareNetwork.h"
#include "XiaoStyle.h"
#include "XiaoAgent.h"
#include "XiaoShareRedis.h"
#include "CoordiManagerApp.h"
#include "ShareDefine.h"


#define LOCTEXT_NAMESPACE "LoginWindow"


using namespace XiaoRedis;
using namespace boost::interprocess;


static const FText SUserOrPassNotCorrect = LOCTEXT("UserOrPassNotCorrect_Text", "账号或密码不正确");


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLoginWindow::Construct(const FArguments& InArgs)
{
	XIAO_LOG(Log, TEXT("SLoginWindow::Construct::Begin"));
	GLog->Flush();

	Auth.LoginMachine = GetUniqueDeviceID();

	FTextBlockStyle BigBlockStyle = FTextBlockStyle::GetDefault();
	BigBlockStyle.SetFontSize(40.0f);
	BigBlockStyle.SetColorAndOpacity(FColor::White);
	
	FTextBlockStyle MiddleBlockStyle = BigBlockStyle;
	MiddleBlockStyle.SetFontSize(20.0f);
	MiddleBlockStyle.SetColorAndOpacity(FLinearColor(FVector4f(0, 250, 0, 0.8)));

	FTextBlockStyle InputBlockStyle = BigBlockStyle;
	InputBlockStyle.SetFontSize(15.0f);
	
	FEditableTextBoxStyle NormalBlockStyle = FEditableTextBoxStyle::GetDefault();
	NormalBlockStyle.SetTextStyle(InputBlockStyle);

	SWindow::Construct(SWindow::FArguments()
	.Title(LOCTEXT("LoginWindowTitle", "登录"))
	.ClientSize(FVector2D(215 * 2, 300 * 2))
	.SupportsMinimize(false).SupportsMaximize(false).HasCloseButton(true).CreateTitleBar(false)
	.SizingRule(ESizingRule::FixedSize)
	.AutoCenter(EAutoCenter::PrimaryWorkArea)
	.bDragAnywhere(false)
	[
		SNew(SVerticalBox)
#pragma region CloseButton
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().Padding(5.0f).HAlign(HAlign_Right)
			[
				SNew(SButton).ButtonColorAndOpacity(FColor::Transparent)
				.ContentPadding(4.0f)
				.Content()
				[
					SAssignNew(CloseImage, SImage).Image(FXiaoStyle::Get().GetBrush("btn_close_normal"))
					.DesiredSizeOverride(FVector2D(20.0f))
				]
				.OnHovered_Lambda([this]()
				{
					this->CloseImage->SetImage(FXiaoStyle::Get().GetBrush("btn_close_hover"));
				})
				.OnUnhovered_Lambda([this]()
				{
					this->CloseImage->SetImage(FXiaoStyle::Get().GetBrush("btn_close_normal"));
				})
				.OnPressed_Lambda([this] ()
				{
					RequestEngineExit(TEXT("XiaoCoordiManager LoginWindow"));
				})
			]
		]
#pragma endregion
	+ SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(ErrorText, SErrorText)
			.Visibility(EVisibility::Collapsed)
			.ToolTipText(LOCTEXT("CantConnectToolTip_Text", "查看调度器所运行的是否正常,或者使用\"代理设置工具重新设置调度器IP\""))
		]
#pragma region Logo
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0, 10.0, 0.0, 10.0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(HAlign_Center)
			[
				SNew(SImage).Image(FXiaoStyle::Get().GetBrush(TEXT("BigLogo")))
			]
		]
#pragma endregion
#pragma region Welcome
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0, 10.0, 0.0, 10.0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(HAlign_Center)
			[
				SNew(STextBlock).Text(LOCTEXT("Welcome_Text", "欢迎"))
				.TextStyle(&BigBlockStyle)
			]
		]
#pragma endregion
#pragma region Slogan
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0, 10.0, 0.0, 10.0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(HAlign_Center)
			[
				SNew(STextBlock).Text(LOCTEXT("Slogan_Text", "加速你的开发进程"))
				.TextStyle(&MiddleBlockStyle)
			]
		]
#pragma endregion
#pragma region UserName
		+ SVerticalBox::Slot().Padding(0.0, 10.0, 0.0, 5.0).AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(HAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().MaxHeight(45.0)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SAssignNew(UsernameTextBox, SEditableTextBox).MinDesiredWidth(250.0f)
						.Justification(ETextJustify::Center)
						.HintText(LOCTEXT("Username_Text", "账户"))
						.VirtualKeyboardType(Keyboard_AlphaNumeric)
						.OnTextCommitted_Lambda([this] (const FText& InText, ETextCommit::Type TextCommitType)
						{
							Auth.Username = InText.ToString();
							(void)CheckUsername(Auth.Username);
						})
						.OnKeyDownHandler_Lambda([this](const FGeometry&, const FKeyEvent& InKey)
						{
							if (InKey.GetKey() == EKeys::Tab)
							{
								FSlateApplication::Get().SetKeyboardFocus(PasswordTextBox, EFocusCause::OtherWidgetLostFocus);
								return FReply::Handled();
							}
							return FReply::Unhandled();
						})
					]
					+ SOverlay::Slot().VAlign(VAlign_Center).HAlign(HAlign_Left).Padding(4.0f)
					[
						SNew(SImage).DesiredSizeOverride(FVector2D(20.0f, 20.0f))
						.Image(FXiaoStyle::Get().GetBrush(TEXT("Developer")))
					]
					+ SOverlay::Slot().VAlign(VAlign_Center).HAlign(HAlign_Right).Padding(4.0f)
					[
						SAssignNew(UserButton, SButton).ButtonColorAndOpacity(FColor::Transparent)
						.Visibility(EVisibility::Hidden)
						.OnPressed_Lambda([this]()
						{
							UsernameTextBox->SetText(FText::GetEmpty());
						})
						[
							SAssignNew(UserImage, SImage)
							.DesiredSizeOverride(FVector2D(20.0f, 20.0f))
							.Image(FXiaoStyle::Get().GetBrush(TEXT("clear")))
						]
					]
				]
				+ SVerticalBox::Slot().MaxHeight(45.0)
				[
					SAssignNew(UsernameText, SErrorText)
				]
			]
		]
#pragma endregion
#pragma region Password
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0, 5.0, 0.0, 25.0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(HAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().MaxHeight(45.0)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SAssignNew(PasswordTextBox, SEditableTextBox).MinDesiredWidth(250.0f)
						.Justification(ETextJustify::Center)
						.VirtualKeyboardType(Keyboard_AlphaNumeric)
						.HintText(LOCTEXT("Password_Text", "密码"))
						.IsPassword(true)
						.OnTextCommitted_Lambda([this] (const FText& InText, ETextCommit::Type TextCommitType)
						{
							Auth.Password = InText.ToString();
							(void)this->CheckPassword(Auth.Password);
						})
						.OnKeyDownHandler_Lambda([this](const FGeometry&, const FKeyEvent& InKey)
						{
							if (InKey.GetKey() == EKeys::Tab)
							{
								FSlateApplication::Get().SetKeyboardFocus(LoginButton, EFocusCause::OtherWidgetLostFocus);
								return FReply::Handled();
							}
							return FReply::Unhandled();
						})
					]
					+ SOverlay::Slot().VAlign(VAlign_Center).HAlign(HAlign_Left).Padding(4.0f)
					[
						SNew(SImage).DesiredSizeOverride(FVector2D(20.0f, 20.0f))
						.Image(FXiaoStyle::Get().GetBrush(TEXT("lock")))
					]
					+ SOverlay::Slot().VAlign(VAlign_Center).HAlign(HAlign_Right).Padding(4.0f)
					[
						SAssignNew(CodeButton, SButton)
						.Visibility(EVisibility::Hidden).ButtonColorAndOpacity(FColor::Transparent).VAlign(VAlign_Center).HAlign(HAlign_Center)
						[
							SAssignNew(CodeImage, SImage)
							.DesiredSizeOverride(FVector2D(20.0f, 20.0f))
							.Image(FXiaoStyle::Get().GetBrush(TEXT("invisible")))
						]
						.OnPressed_Lambda([this]()
						{
							this->bShowCode = !this->bShowCode;
							PasswordTextBox->SetIsPassword(!this->bShowCode);
							this->CodeImage->SetImage(FXiaoStyle::Get().GetBrush(this->bShowCode ? TEXT("visible") : TEXT("invisible")));
						})
					]
				]
				+ SVerticalBox::Slot().MaxHeight(45.0)
				[
					SAssignNew(PasswordText, SErrorText)
				]
			]
		]
#pragma endregion
#pragma region LoginButton
		+ SVerticalBox::Slot().Padding(0.0, 10.0, 0.0, 10.0).MaxHeight(50.0f).AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(HAlign_Center).FillWidth(250.0f)
			[
				SAssignNew(LoginButton, SButton).Text(LOCTEXT("Login_Text", "登录"))
				.OnClicked_Lambda([this]() {
					OnLogin();
					return FReply::Handled();
				})
			]
		]
#pragma endregion
#pragma region Foot
		+ SVerticalBox::Slot().Padding(0.0, 5.0, 0.0, 10.0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(EHorizontalAlignment::HAlign_Fill)
			[
				SNullWidget::NullWidget
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().Padding(0.0, 10.0, 2.0, 10.0).AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SHyperlink).Text(LOCTEXT("ForgetPassword_Text", "忘记密码?"))
					.OnNavigate_Lambda([this]()
					{
						auto Window = SNew(SForgetDialog);
						FSlateApplication::Get().AddModalWindow(Window, SharedThis(this), false);
					})
				]

				+ SHorizontalBox::Slot().Padding(2.0, 10.0, 2.0, 10.0).AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(LOCTEXT("Seperator_Text", "●"))
				]

				+ SHorizontalBox::Slot().Padding(2.0, 10.0, 0.0, 10.0).AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SHyperlink).Text(LOCTEXT("ContactSupport_Text", "请求支持"))
					.OnNavigate_Lambda([]()
					{
						GetHelp(GLocalization == TEXT("zh-CN") ? TEXT("登录账户") : TEXT(""));
					})
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SButton).ButtonColorAndOpacity(FColor::Transparent)
					.ToolTipText(LOCTEXT("AgentSettings_ToolTip", "打开代理设置程序，进行网络相关设置"))
					.Content()
					[
						SNew(SImage).Image(FXiaoStyle::Get().GetBrush("Tray.AgentSettings"))
					]
					.OnClicked_Lambda([]() 
					{
						RunXiaoApp(XiaoAppName::SBuildApp, FString::Printf(TEXT("-app=%s"), *XiaoAppName::SBuildAgentSettings), true, true, true, true);
						return FReply::Handled();
					})
				]
			]
			+ SHorizontalBox::Slot().HAlign(EHorizontalAlignment::HAlign_Fill)
			[
				SNullWidget::NullWidget
			]
		]
#pragma endregion
	]);

	// TODO 输入系统
	// ITextInputMethodSystem* const TextInputMethodSystem = FSlateApplication::Get().GetTextInputMethodSystem();
	// if(TextInputMethodSystem)
	// {
	// 	// Activated input method
	// 	TextInputMethodSystem->ActivateContext();
	// }

	XIAO_LOG(Log, TEXT("SLoginWindow::Construct::Finish"));
	GLog->Flush();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLoginWindow::OnLogin()
{
	if(!CheckUsername(Auth.Username))
	{
		return ;
	}
	if(!CheckPassword(Auth.Password))
	{
		return;
	}

	ErrorText->SetError(FText::GetEmpty());

	Auth.LastLogin = FDateTime::Now().ToString();
	Auth.bCompleVerifi = true;
	
	try
	{
		if (IsConnected())
		{
			const std::string Username = TCHAR_TO_UTF8(*Auth.Username);
			if (!SRedisClient->hexists(Hash::SUserDetail, Username))
			{
				PasswordText->SetError(SUserOrPassNotCorrect);
				PasswordText->SetVisibility(EVisibility::Visible);
#if !PLATFORM_MAC
				return;
#endif
			}

			if (const auto Optional = SRedisClient->hget(Hash::SUserDetail, Username))
			{
				const FString EncrypedStr = UTF8_TO_TCHAR(Optional.value().c_str());
				FString DecrypedStr;
				if (DecryptString(EncrypedStr, XiaoEncryptKey::SAuth, DecrypedStr))
				{
					FUserDetail User;
					if (User.FromJson(DecrypedStr))
					{
						if (User.Password != Auth.Password)
						{
							PasswordText->SetError(SUserOrPassNotCorrect);
							PasswordText->SetVisibility(EVisibility::Visible);
							return;
						}

						GCurrentUser.FromJson(DecrypedStr);
						if (!SRedisClient->hexists(Hash::SLoginCache, GAgentUID))
						{
							FLoginCache LoginCache(GetUniqueDeviceID(), FPlatformTime::Seconds(), GCurrentUser.ToJson());
							const std::string Cache = TCHAR_TO_UTF8(*LoginCache.ToJson());
							SRedisClient->hset(Hash::SLoginCache, GAgentUID, Cache);
						}
						
						if (auto App = static_cast<FCoordiManagerApp*>(FXiaoAppBase::GApp))
						{
							RequestDestroyWindow();
							App->ShowMainWindow();
						}
					}
				}
				else
				{
					ErrorText->SetVisibility(EVisibility::Visible);
					ErrorText->SetError(LOCTEXT("CantDecypt_TEXT", "解码数据失败"));
				}
			}
			else
			{
				ErrorText->SetVisibility(EVisibility::Visible);
				ErrorText->SetError(LOCTEXT("CantFind_TEXT", "找不到对应的用户"));
			}
		}
		else
		{
			ErrorText->SetVisibility(EVisibility::Visible);
			ErrorText->SetError(LOCTEXT("CantConnect_Text", "无法连接到服务,是否开启了调度服务器"));
		}
	}
	CATCH_REDIS_EXCEPTRION();
}

bool SLoginWindow::CheckUsername(const FString& InUsername) const
{
	FText Error;
	if(InUsername.IsEmpty())
	{
		Error = LOCTEXT("UsernameRequire_TEXT", "需要账户");
	}
	if(InUsername.Len() < 4 || StringContainSpecialCase(InUsername))
	{
		Error = LOCTEXT("UsernameNotlid_TEXT", "至少有4个字符且不能包含特殊字符或空格");
	}
	this->UserButton->SetVisibility(InUsername.IsEmpty() ? EVisibility::Hidden : EVisibility::Visible);
	this->UsernameText->SetError(Error);
	this->UsernameText->SetVisibility(this->UsernameText->HasError() ? EVisibility::Visible : EVisibility::Hidden);
	return Error.IsEmpty();
}

bool SLoginWindow::CheckPassword(const FString& InPassword) const
{
	FText Error;
	if(InPassword.IsEmpty())
	{
		Error = LOCTEXT("PasswordRequire_TEXT", "需要密码");
	}
	if(InPassword.Len() < 8 || !StringContainUpperCase(InPassword))
	{
		Error = LOCTEXT("PasswordNotlid_TEXT", "至少有8个字符且至少包含一个大写字符  ");
	}

	this->CodeButton->SetVisibility(InPassword.IsEmpty() ? EVisibility::Hidden : EVisibility::Visible);
	this->PasswordText->SetError(Error);
	this->PasswordText->SetVisibility(this->PasswordText->HasError() ? EVisibility::Visible : EVisibility::Hidden);

	return Error.IsEmpty();
}


#undef LOCTEXT_NAMESPACE
