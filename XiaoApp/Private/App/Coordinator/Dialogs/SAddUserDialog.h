/**
  * @author cxx2020@outlook.com
  * @date 5:57 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
#include "Database/Users.h"

using namespace XiaoDB;


class SEditableTextBox;
template< typename OptionType >
class SComboBox;

class SAddUserDialog final : public SWindow
{
public:
	DECLARE_DELEGATE_OneParam(FOnAddNewUser, FUserDetail&);
	
	SLATE_BEGIN_ARGS(SAddUserDialog){}
		SLATE_EVENT(FOnAddNewUser, OnAddUser)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

protected:
	bool OnCanAdd() const;

private:
	FOnAddNewUser OnAddUser;
	TSharedPtr<SEditableTextBox> Username = nullptr;
	TSharedPtr<SComboBox<TSharedPtr<FText>>> RoleComboBox = nullptr;
	TSharedPtr<SEditableTextBox> Password = nullptr;
	TSharedPtr<SEditableTextBox> RePassword = nullptr;
	FUserDetail UserDesc{};
	bool bUser = false;
	bool bPassword = false;
};