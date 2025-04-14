/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SBackupListRow.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "SPrimaryButton.h"
#include "Dialog/SMessageDialog.h"

#include "XiaoShare.h"
#include "XiaoStyle.h"
#include "XiaoAppBase.h"
#include "ShareDefine.h"


#define LOCTEXT_NAMESPACE "SBackupListRow"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SBackupRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	check(InArgs._BackupDesc.IsValid());
	BackupDesc = InArgs._BackupDesc;
	OnRoleChange = InArgs._OnRoleChange;
	OnBackupChange = InArgs._OnBackupChange;
	OnBackupDelete = InArgs._OnBackupDelete;
	
	SMultiColumnTableRow::Construct(FSuperRowType::FArguments().Padding(2.0f), InOwnerTableView);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SBackupRow::GenerateWidgetForColumn(const FName& InColumnName)
{
	if(BackupDesc.IsValid())
	{
		if(InColumnName == GBackupTableColumnIDIp)
		{
			return V_CENTER_WIGET(SNew(STextBlock)
				.Text_Lambda([this]() {
					return BackupDesc.IsValid() ? FText::FromString(FString::Printf(TEXT("%s:%u"), *BackupDesc.Pin()->Host, BackupDesc.Pin()->Port)) : FText::GetEmpty();
					})
				.Justification(ETextJustify::Type::Center));
		}
		if(InColumnName == GBackupTableColumnIDRole)
		{
			return V_FILL_WIGET(SAssignNew(RoleComboBox, SComboBox<TSharedPtr<FText>>)
			.IsEnabled_Lambda([this]() 
			{ 
				return this->BackupDesc.IsValid() ? !this->BackupDesc.Pin()->Role : false;
			})
			.OptionsSource(&GMasterSlaveArray)
			.InitiallySelectedItem(GMasterSlaveArray[BackupDesc.Pin()->Role])
			.OnGenerateWidget_Lambda([](const TSharedPtr<FText> InText)
			{
				return SNew(STextBlock).Text(*InText);
			})
			.OnSelectionChanged_Lambda([this] (const TSharedPtr<FText> InText, ESelectInfo::Type)
			{
				if (FXiaoStyle::DoModel(LOCTEXT("ChangeRole_Text", "确定要改变主从服务器的角色吗?")))
				{
					if (this->BackupDesc.IsValid())
					{
						this->BackupDesc.Pin()->Role = GMasterSlaveArray.Find(InText) == 0 ? true : false;
						OnRoleChange.ExecuteIfBound(this->BackupDesc);
					}
				}
				else
				{
					const int SelectIndex = (*GMasterSlaveArray[0]).ToString() == GMaster.ToString() ? 0 : 1;
					this->RoleComboBox->SetSelectedItem(GMasterSlaveArray[SelectIndex]);
				}
			})
			.Content()
			[
				SNew(STextBlock).Text_Lambda([this]()
				{
					int Index = 1;
					if (this->BackupDesc.IsValid())
					{
						Index = this->BackupDesc.Pin()->Role ? 0 : 1;
					}
					return *GMasterSlaveArray[Index];
				})
			]);
		}
		if(InColumnName == GBackupTableColumnIDPriority)
		{
			return SAssignNew(PriorityBox, SSpinBox<uint8>)
				.Justification(ETextJustify::Type::Center)
				.IsEnabled_Lambda([this]() 
				{
					if (BackupDesc.IsValid())
					{
						return BackupDesc.Pin()->Role ? false : true;
					}
					return false;
				})
				.MinValue(1)
				.MaxValue(255)
				.OnValueChanged_Lambda([this](const uint8 InValue)
					{
						if (this->BackupDesc.IsValid())
						{
							this->BackupDesc.Pin()->Priority = InValue;
							OnBackupChange.ExecuteIfBound(this->BackupDesc);
						}
					})
				.Value_Lambda([this]()
					{
						return this->BackupDesc.IsValid() ? this->BackupDesc.Pin()->Priority : 1;
					}
				);
		}
		if(InColumnName == GBackupTableColumnIDStatus)
		{
			return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(HAlign_Fill)
			[
				SNullWidget::NullWidget
			]
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center)
			[
				SAssignNew(StatusImage, SImage)
				.Image_Lambda([this]()
				{
					FName ImageKey = TEXT("in_active");
					if (this->BackupDesc.IsValid())
					{
						ImageKey = (this->BackupDesc.Pin()->Status) ? TEXT("active") : TEXT("in_active");
					}
					return FXiaoStyle::Get().GetBrush(ImageKey);
				})
			]
			+ SHorizontalBox::Slot().HAlign(HAlign_Fill)
			[
				SNullWidget::NullWidget
			];
		}
		if(InColumnName == GBackupTableColumnIDDelete)
		{
			if (BackupDesc.IsValid())
			{
				return SNew(SButton)
				.IsEnabled_Lambda([this]() 
				{
					if (BackupDesc.IsValid())
					{
						return BackupDesc.Pin()->Role ? false : true;
					}
					return false;
				})
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
				.OnClicked_Lambda([this]() 
				{
					if (FXiaoStyle::DoModel(LOCTEXT("DeleteBackup_Text", "确定删除备用服务吗?")))
					{
						OnBackupDelete.ExecuteIfBound(BackupDesc);
					}
					return FReply::Handled();
				})
				.Content()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().HAlign(HAlign_Fill)
					[
						SNullWidget::NullWidget
					]
					+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center)
					[
						SNew(SImage).Image(FXiaoStyle::Get().GetBrush(TEXT("delete")))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).HAlign(HAlign_Center)
					[
						SNew(STextBlock).Text(LOCTEXT("Delete_Text", "删除"))
					]
					+ SHorizontalBox::Slot().HAlign(HAlign_Fill)
					[
						SNullWidget::NullWidget
					]
				];
			}
		}
	}
	return SNullWidget::NullWidget;
}

#undef LOCTEXT_NAMESPACE