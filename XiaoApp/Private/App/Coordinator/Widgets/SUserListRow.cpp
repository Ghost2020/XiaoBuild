/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SUserListRow.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SComboBox.h"
#include "SPrimaryButton.h"
#include "Dialog/SMessageDialog.h"
#include "../SCoordinatorWindow.h"

#include "XiaoShare.h"
#include "XiaoStyle.h"
#include "XiaoAppBase.h"
#include "ShareDefine.h"


#define LOCTEXT_NAMESPACE "SUserListRow"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SUserListRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	check(InArgs._UserDesc.IsValid());
	UserDesc = InArgs._UserDesc;
	OnUserChange = InArgs._OnUserChange;
	OnUserDelete = InArgs._OnUserDelete;
	
	SMultiColumnTableRow::Construct(FSuperRowType::FArguments().Padding(2.0f), InOwnerTableView);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SUserListRow::GenerateWidgetForColumn(const FName& InColumnName)
{
	if(UserDesc.IsValid())
	{
		if(const auto User = UserDesc.Pin())
		{
			if(InColumnName == GUserTableColumnIDID)
			{
				return V_CENTER_WIGET(SNew(STextBlock).Text(FText::FromString(FString::FromInt(User->Id))).Justification(ETextJustify::Type::Center));
			}
			if(InColumnName == GUserTableColumnIDUsername)
			{
				return V_CENTER_WIGET(SNew(STextBlock).Text(FText::FromString(User->Username)).Justification(ETextJustify::Type::Center));
			}
			if(InColumnName == GUserTableColumnIDRole)
			{
				return SAssignNew(RoleComboBox, SComboBox<TSharedPtr<FText>>)
				.OptionsSource(&GRoleArray)
				.IsEnabled_Lambda([]() { return GCurrentUser.Role != 2; })
				.InitiallySelectedItem(GRoleArray[User->Role])
				.OnComboBoxOpening_Lambda([]() 
				{
					GCanUpdate = false;
				})
				.OnGenerateWidget_Lambda([](const TSharedPtr<FText> InText)
				{
					return SNew(STextBlock).Text(*InText);
				})
				.OnSelectionChanged_Lambda([this] (const TSharedPtr<FText> InText, ESelectInfo::Type)
				{
					GCanUpdate = true;
					this->UserDesc.Pin()->Role = static_cast<EUserRole>(GRoleArray.Find(InText));
					OnUserChange.ExecuteIfBound(this->UserDesc);
				})
				.Content()
				[
					SNew(STextBlock).Text_Lambda([this]()
					{
						return *GRoleArray[this->UserDesc.Pin()->Role];
					})
				];
			}
			if(InColumnName == GUserTableColumnIDLastLogin)
			{
				return V_CENTER_WIGET(SNew(STextBlock).Text(FText::FromString(User->LastLogin.ToString())).Justification(ETextJustify::Type::Center));
			}
			if(InColumnName == GUserTableColumnIDStatus)
			{
				return SNew(SHorizontalBox)
				+SHorizontalBox::Slot().AutoWidth()
				[
					SAssignNew(StatusImage, SImage)
					.Image_Lambda([this]() 
					{
						const FName ImageKey = (this->UserDesc.Pin()->Status == 0) ? TEXT("active") : TEXT("in_active");
						return FXiaoStyle::Get().GetBrush(ImageKey);
					})
				]
				+SHorizontalBox::Slot().AutoWidth()
				[
					SAssignNew(StatusComboBox, SComboBox<TSharedPtr<FText>>)
					.OptionsSource(&GStatusArray)
					.IsEnabled_Lambda([]() { return GCurrentUser.Role != 2; })
					.InitiallySelectedItem(GStatusArray[User->Status])
					.OnComboBoxOpening_Lambda([]() 
					{
						GCanUpdate = false;
					})
					.OnGenerateWidget_Lambda([](const TSharedPtr<FText> InText)
					{
						return SNew(STextBlock).Text(*InText);
					})
					.OnSelectionChanged_Lambda([this](const TSharedPtr<FText> InText, ESelectInfo::Type)
					{
						GCanUpdate = true;
						this->UserDesc.Pin()->Status = GStatusArray.Find(InText);
						OnUserChange.ExecuteIfBound(this->UserDesc);
					})
					.Content()
					[
						SNew(STextBlock).Text_Lambda([this]()
						{
							return *GStatusArray[this->UserDesc.Pin()->Status];
						})
					]
				];
			}
			if (InColumnName == GUserTableColumnIDAttempts)
			{
				return V_CENTER_WIGET(SNew(STextBlock).Text(FText::FromString(FString::FromInt(User->LoginAttempts))).Justification(ETextJustify::Type::Center));
			}
			if (InColumnName == GUserTableColumnIDCreateBy)
			{
				return V_CENTER_WIGET(SNew(STextBlock).Text(FText::FromString(FString::FromInt(User->CreateByUserId))).Justification(ETextJustify::Type::Center));
			}
			if (InColumnName == GUserTableColumnIDDelete)
			{
				if (UserDesc.IsValid() && UserDesc.Pin()->Id != 0)
				{
					return SAssignNew(DeleteButton, SButton)
					.IsEnabled_Lambda([]() { return GCurrentUser.Role != 2; })
					.ButtonColorAndOpacity_Lambda([this]()
					{
						return bHoverd ? FColor::Red : FColor::Transparent;
					})
					.OnHovered_Lambda([this]()
					{
						bHoverd = true;
					})
					.OnUnhovered_Lambda([this]()
					{
						bHoverd = false;
					})
					.OnClicked_Raw(this, &SUserListRow::OnDeleteUser)
					.Content()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SImage).Image(FXiaoStyle::Get().GetBrush(TEXT("delete")))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(LOCTEXT("Delete_Text", "删除"))
						]
					];
				}
			}
		}
	}
	return SNullWidget::NullWidget;
}

FReply SUserListRow::OnDeleteUser() const
{
	if (auto Window = StaticCastSharedPtr<SCoordinatorWindow>(FXiaoAppBase::GApp->GetMainWindow().Pin()))
	{
		Window->SetLockedState(true);
		FXiaoAppBase::GApp->AddNextTickTask(FSimpleDelegate::CreateLambda([this]()
		{
			const TSharedRef<SMessageDialog> Dialog = SNew(SMessageDialog)
			.Title(LOCTEXT("ConfirmTitle", "确定"))
			.Icon(FAppStyle::Get().GetBrush("Icons.WarningWithColor.Large"))
			.Message(LOCTEXT("ConfirmModity_Message", "确定删除当前选中的用户?"))
			.UseScrollBox(false)
			.AutoCloseOnButtonPress(true)
			.Buttons(
			{
				SMessageDialog::FButton(LOCTEXT("ConfirmButton", "确定"))
				.SetOnClicked(FSimpleDelegate::CreateLambda([this]()
				{
					(void)OnUserDelete.ExecuteIfBound(UserDesc);
				})),
				SMessageDialog::FButton(LOCTEXT("CancelButton", "取消")).SetPrimary(true).SetFocus()
			});
			
			Dialog->GetOnWindowClosedEvent().AddLambda([](const TSharedRef<SWindow>&)
			{
				if (auto Window = StaticCastSharedPtr<SCoordinatorWindow>(FXiaoAppBase::GApp->GetMainWindow().Pin()))
				{
					Window->SetLockedState(false);
				}
			});
			Dialog->ShowModal();
		}));
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE