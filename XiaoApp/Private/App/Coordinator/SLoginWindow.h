/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
// #include "Interfaces/IHttpRequest.h"
#include "Database/Users.h"


class SEditableTextBox;
class SErrorText;
class SButton;
class SImage;


using namespace XiaoDB;


class SLoginWindow final : public SWindow
{
public:
	SLATE_BEGIN_ARGS(SLoginWindow)
	{}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

protected:
	void OnLogin();
	bool CheckUsername(const FString& InUsername) const;
	bool CheckPassword(const FString& InPassword) const;

private:
	bool bEnbaleProxy = true;
	mutable bool bShowProxy = false;
	mutable FAuthUser Auth;
	TSharedPtr<SImage> CloseImage = nullptr;
	TSharedPtr<SErrorText> ErrorText = nullptr;
	TSharedPtr<SEditableTextBox> UsernameTextBox = nullptr;
	TSharedPtr<SEditableTextBox> PasswordTextBox = nullptr;
	TSharedPtr<SErrorText> UsernameText = nullptr;
	TSharedPtr<SErrorText> PasswordText = nullptr;
	TSharedPtr<SButton> UserButton = nullptr;
	TSharedPtr<SImage> UserImage = nullptr;
	TSharedPtr<SButton> CodeButton = nullptr;
	TSharedPtr<SImage> CodeImage = nullptr;
	TSharedPtr<SButton> LoginButton = nullptr;
	bool bShowCode = false;
};
