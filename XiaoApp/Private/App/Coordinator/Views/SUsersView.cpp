/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SUsersView.h"
#include "SlateOptMacros.h"
#include "SPrimaryButton.h"
#include "SSimpleButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SSearchBox.h"
#include "../Widgets/SConstrainBox.h"
#include "../Widgets/SUserListRow.h"
#include "../Dialogs/SAddUserDialog.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/MessageDialog.h"
// #include "XiaoHttpRequest.h"
#include "XiaoLog.h"
#include "XiaoShare.h"
#include "XiaoShareField.h"
#include "XiaoShareNetwork.h"
#include "ShareDefine.h"

#include "Database/Users.h"

#define LOCTEXT_NAMESPACE "SUsersView"

// using namespace XiaoHttp;
using namespace XiaoUserParam;

static FText SCantConnect = LOCTEXT("ConnectFailed_Text", "无法连接");
static FText SUpdateSuccess = LOCTEXT("UpdateSuccess_Text", "更新完成");
static FText SUpdateFailed = LOCTEXT("UpdateFailed_Text", "更新失败");

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SUsersView::Construct(const FArguments& InArgs)
{
	XIAO_LOG(Log, TEXT("SUsersView::Construct::Begin"));
	GLog->Flush();

	OnQueueNotification = InArgs._OnQueueNotification;
	
	ChildSlot
	[
		SNew(SVerticalBox)
#pragma region Top
		+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(50.0f, 10.0f)
		[
			SNew(SHorizontalBox).Visibility(GCurrentUser.Role != 2 ? EVisibility::Visible : EVisibility::Collapsed)

			+SHorizontalBox::Slot().HAlign(HAlign_Right).Padding(10.0f)
			[
				SNew(SPrimaryButton).Text(LOCTEXT("AddUser_Text", "添加用户"))
				.Visibility(GCurrentUser.Role != 2 ? EVisibility::Visible : EVisibility::Collapsed)
				.OnClicked_Lambda([this]()
				{
					if (!AddUserWindow.IsValid())
					{
						const TSharedRef<SAddUserDialog> Window = SNew(SAddUserDialog)
						.OnAddUser_Raw(this, &SUsersView::OnAddUser);
						AddUserWindow = FSlateApplication::Get().AddWindow(Window, true);
					}
					else 
					{
						AddUserWindow.Pin()->BringToFront();
					}
					
					return FReply::Handled();
				})
			]
		]
#pragma endregion 
#pragma region TableBody
		+ SVerticalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill).Padding(25.0f, 10.0f, 50.0f, 5.0f)
		[
			SAssignNew(UserListView, SListView<TSharedPtr<FUserDesc>>)
			.ListItemsSource(&UserArray)
			.Orientation(Orient_Vertical)
			.SelectionMode(ESelectionMode::Type::Single)
			.EnableAnimatedScrolling(true)
			// .ItemHeight(50.0f)
			.AllowOverscroll(EAllowOverscroll::Yes)
			.OnGenerateRow_Raw(this, &SUsersView::OnGenerateRow)
			.HeaderRow(
				SNew(SHeaderRow)
				+SHeaderRow::Column(GUserTableColumnIDID)
				.FixedWidth(50.0f)
				.HAlignHeader(EHorizontalAlignment::HAlign_Center)
				.VAlignHeader(VAlign_Center)
				.DefaultLabel(LOCTEXT("ID_Text", "ID"))
				.DefaultTooltip(LOCTEXT("ID_Tooltip", "ID"))
				.InitialSortMode(EColumnSortMode::Type::Ascending)
				.OnSort_Raw(this, &SUsersView::OnSortTable)
				.SortMode_Raw(this, &SUsersView::GetSortModeForColumn, GUserTableColumnIDID)

				+SHeaderRow::Column(GUserTableColumnIDUsername)
				.FillWidth(.18f)
				.HAlignHeader(EHorizontalAlignment::HAlign_Center)
				.VAlignHeader(VAlign_Center)
				.DefaultLabel(LOCTEXT("Username_Text", "用户名"))
				.DefaultTooltip(LOCTEXT("Username_Tooltip", "用户名"))
				.InitialSortMode(EColumnSortMode::Type::Ascending)
				.OnSort_Raw(this, &SUsersView::OnSortTable)
				.SortMode_Raw(this, &SUsersView::GetSortModeForColumn, GUserTableColumnIDUsername)

                +SHeaderRow::Column(GUserTableColumnIDRole)
				.FillWidth(.18f)
				.HAlignHeader(EHorizontalAlignment::HAlign_Center)
				.VAlignHeader(VAlign_Center)
				.DefaultLabel(LOCTEXT("Role_Text", "职能"))
				.DefaultTooltip(LOCTEXT("Role_Tooltip", "职能"))
				.InitialSortMode(EColumnSortMode::Type::Ascending)
				.OnSort_Raw(this, &SUsersView::OnSortTable)
				.SortMode_Raw(this, &SUsersView::GetSortModeForColumn, GUserTableColumnIDRole)

				+SHeaderRow::Column(GUserTableColumnIDLastLogin)
				.FillWidth(.18f)
				.HAlignHeader(EHorizontalAlignment::HAlign_Center)
				.VAlignHeader(VAlign_Center)
				.DefaultLabel(LOCTEXT("LastLogin_Text", "上次登录时间"))
				.DefaultTooltip(LOCTEXT("LastLogin_Tooltip", "上次登录时间"))
				.InitialSortMode(EColumnSortMode::Type::Ascending)
				.OnSort_Raw(this, &SUsersView::OnSortTable)
				.SortMode_Raw(this, &SUsersView::GetSortModeForColumn, GUserTableColumnIDLastLogin)

				+SHeaderRow::Column(GUserTableColumnIDStatus)
				.FillWidth(.18f)
				.HAlignHeader(EHorizontalAlignment::HAlign_Center)
				.VAlignHeader(VAlign_Center)
				.DefaultLabel(LOCTEXT("Status_Text", "状态"))
				.DefaultTooltip(LOCTEXT("Status_Tooltip", "状态"))
				.InitialSortMode(EColumnSortMode::Type::Ascending)
				.OnSort_Raw(this, &SUsersView::OnSortTable)
				.SortMode_Raw(this, &SUsersView::GetSortModeForColumn, GUserTableColumnIDStatus)

				+ SHeaderRow::Column(GUserTableColumnIDAttempts)
				.FillWidth(.18f)
				.HAlignHeader(EHorizontalAlignment::HAlign_Center)
				.VAlignHeader(VAlign_Center)
				.DefaultLabel(LOCTEXT("Attemp_Text", "尝试登录"))
				.DefaultTooltip(LOCTEXT("StatusAttemp_Tooltip", "尝试登录次数"))
				.InitialSortMode(EColumnSortMode::Type::Ascending)
				.OnSort_Raw(this, &SUsersView::OnSortTable)
				.SortMode_Raw(this, &SUsersView::GetSortModeForColumn, GUserTableColumnIDAttempts)

				+ SHeaderRow::Column(GUserTableColumnIDCreateBy)
				.FillWidth(.18f)
				.HAlignHeader(EHorizontalAlignment::HAlign_Center)
				.VAlignHeader(VAlign_Center)
				.DefaultLabel(LOCTEXT("Create_Text", "创建者"))
				.DefaultTooltip(LOCTEXT("Create_Tooltip", "账户创建者"))
				.InitialSortMode(EColumnSortMode::Type::Ascending)
				.OnSort_Raw(this, &SUsersView::OnSortTable)
				.SortMode_Raw(this, &SUsersView::GetSortModeForColumn, GUserTableColumnIDCreateBy)

				+ SHeaderRow::Column(GUserTableColumnIDDelete)
				.Visibility((GCurrentUser.Role != 2) ? EVisibility::Visible : EVisibility::Collapsed)
				.FixedWidth(75.0f)
				.HAlignHeader(EHorizontalAlignment::HAlign_Center)
				.VAlignHeader(VAlign_Center)
				.DefaultLabel(LOCTEXT("Delete_Text", "删除"))
				.DefaultTooltip(LOCTEXT("Delete_Tooltip", "删除当前账户"))
			)
		]
#pragma endregion
#pragma region Foot
		+ SVerticalBox::Slot().VAlign(VAlign_Bottom).AutoHeight().Padding(50.0f, 10.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(HAlign_Fill)
			[
				SNullWidget::NullWidget
			]
			+SHorizontalBox::Slot().HAlign(HAlign_Right).AutoWidth().VAlign(VAlign_Center)
			[
				SAssignNew(UserNumText, STextBlock).Text_Raw(this, &SUsersView::GetUsername)
				.Justification(ETextJustify::Type::Center)
			]
			+SHorizontalBox::Slot().HAlign(HAlign_Right).Padding(5.0f, 10.0f).AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(LOCTEXT("TotalUser_Text", "位用户"))
			]
		]
#pragma endregion
	];

	XIAO_LOG(Log, TEXT("SUsersView::Construct::Finish"));
	GLog->Flush();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<ITableRow> SUsersView::OnGenerateRow(const TSharedPtr<FUserDesc> InDesc, const TSharedRef<STableViewBase>& InTableView) const
{
	return SNew(SUserListRow, InTableView)
		.UserDesc(InDesc)
		.OnUserChange_Raw(this, &SUsersView::OnRoleChange)
		.OnUserDelete_Raw(this, &SUsersView::OnDeleteUser);
}

void SUsersView::OnSortTable(const EColumnSortPriority::Type InPriority, const FName& InName, const EColumnSortMode::Type InMode)
{
	XIAO_LOG(Log, TEXT("Priority::%d Name::%s Mode::%d"), InPriority, *InName.ToString(), InMode);
	static const TArray<FName> ColumnIds =
	{
		GUserTableColumnIDID,
		GUserTableColumnIDUsername,
		GUserTableColumnIDRole,
		GUserTableColumnIDLastLogin,
		GUserTableColumnIDStatus,
		GUserTableColumnIDAttempts,
		GUserTableColumnIDCreateBy
	};

	ColumnIdToSort = InName;
	ActiveSortMode = InMode;

	TArray<FName> ColumnIdsBySortOrder = {InName};
	for (const FName& Id : ColumnIds)
	{
		if (Id != InName)
		{
			ColumnIdsBySortOrder.Add(Id);
		}
	}

	UserArray.Sort([ColumnIdsBySortOrder, InMode](const TSharedPtr<FUserDesc>& Left, const TSharedPtr<FUserDesc>& Right)
	{
		int32 CompareResult = 0;
		for (const FName& ColumnId : ColumnIdsBySortOrder)
		{
			if (ColumnId == GUserTableColumnIDID)
			{
				CompareResult = (Left->Id == Right->Id) ? 0 : (Left->Id < Right->Id ? -1 : 1);
			}
			else if(ColumnId == GUserTableColumnIDUsername)
			{
				CompareResult = Left->Username.Compare(Right->Username);
			}
			else if(ColumnId == GUserTableColumnIDRole)
			{
				CompareResult = (Left->Role == Right->Role) ? 0 : (Left->Role < Right->Role ? -1 : 1);
			}
			else if(ColumnId == GUserTableColumnIDLastLogin)
			{
				CompareResult = (Left->LastLogin == Right->LastLogin) ? 0 : (Left->LastLogin < Right->LastLogin ? -1 : 1);
			}
			else if(ColumnId == GUserTableColumnIDStatus)
			{
				CompareResult = (Left->Status == Right->Status) ? 0 : (Left->Status < Right->Status ? -1 : 1);
			}
			else if (ColumnId == GUserTableColumnIDAttempts)
			{
				CompareResult = (Left->LoginAttempts == Right->LoginAttempts) ? 0 : (Left->LoginAttempts < Right->LoginAttempts ? -1 : 1);
			}
			else if (ColumnId == GUserTableColumnIDCreateBy)
			{
				CompareResult = (Left->CreateByUserId == Right->CreateByUserId) ? 0 : (Left->CreateByUserId < Right->CreateByUserId ? -1 : 1);
			}

			if (CompareResult != 0)
			{
				return InMode == EColumnSortMode::Ascending ? CompareResult < 0 : CompareResult > 0;
			}
		}
		return InMode == EColumnSortMode::Ascending ? true : false;
	});

	UserListView->RequestListRefresh();
}

EColumnSortMode::Type SUsersView::GetSortModeForColumn(const FName InColumnId) const
{
	return (InColumnId == ColumnIdToSort) ? ActiveSortMode : EColumnSortMode::None;
}

FText SUsersView::GetUsername() const
{
	return FText::FromString(FString::FromInt(UserArray.Num()));
}

void SUsersView::OnUpdate(const bool bInRebuild) const
{
	static double LastTime = 0.0f;
	if (!bInRebuild && (FPlatformTime::Seconds() - LastTime < 600.0f))
	{
		return;
	}
	LastTime = FPlatformTime::Seconds();

	bool NeedRebuild = bInRebuild;

	if (XiaoRedis::IsConnected())
	{
		std::unordered_map<std::string, std::string> UserDetails;
		XiaoRedis::SRedisClient->hgetall(XiaoRedis::Hash::SUserDetail, std::inserter(UserDetails, UserDetails.begin()));

		TSet<FString> UserSet;
		for (const auto& Iter : UserDetails)
		{
			const FString Username = UTF8_TO_TCHAR(Iter.first.c_str());
			const FString Val = UTF8_TO_TCHAR(Iter.second.c_str());
			FString DecrypedStr;
			if (DecryptString(Val, XiaoEncryptKey::SAuth, DecrypedStr))
			{
				FUserDesc UserDesc;
				if (UserDesc.FromJson(DecrypedStr))
				{
					UserSet.Add(Username);
					if (!User2Desc.Contains(Username))
					{
						auto Item = MakeShared<FUserDesc>(UserDesc);
						User2Desc.Add({ Username, Item });
						UserArray.Add(Item);
						NeedRebuild = true;
					}
					else
					{
						auto& Temp = User2Desc[Username];
						if (Temp.IsValid())
						{
							*Temp.Pin() = UserDesc;
						}
					}
				}
			}
		}
		 
		TSet<FString> NeedDelete;
		for (const auto& User : UserArray)
		{
			if (!UserSet.Contains(User->Username))
			{
				NeedDelete.Add(User->Username);
				NeedRebuild = true;
			}
		}
		for (const FString& User : NeedDelete)
		{
			if (User2Desc.Contains(User))
			{
				UserArray.Remove(User2Desc[User].Pin());
				User2Desc.Remove(User);
			}
		}

		if (NeedRebuild && UserListView.IsValid())
		{
			UserListView->RequestListRefresh();
		}
	}
}

void SUsersView::OnAddUser(FUserDetail& InUserDesc)
{
	bool bConflict = false;
	uint32 Id = 0;
	for(const auto& User : UserArray)
	{
		if(User->Id >= InUserDesc.Id)
		{
			bConflict = true;
			Id = User->Id + 1;
		}
	}
	if(bConflict)
	{
		InUserDesc.Id = Id;
	}

	FString EncryptContent;
	if (EncryptString(InUserDesc.ToJson(), XiaoEncryptKey::SAuth, EncryptContent))
	{
		uint8 Status = 0;
		FText Text;
		if (XiaoRedis::IsConnected())
		{
			try
			{
				const std::string Key = TCHAR_TO_UTF8(*InUserDesc.Username);
				const std::string Val = TCHAR_TO_UTF8(*EncryptContent);
				XiaoRedis::SRedisClient->hset(XiaoRedis::Hash::SUserDetail, Key, Val);
				Text = LOCTEXT("AddUserSuccess_Text", "添加用户成功");
				OnUpdate(true);
				if (AddUserWindow.IsValid())
				{
					AddUserWindow.Pin()->DestroyWindowImmediately();
					AddUserWindow = nullptr;
				}
			}
			CATCH_REDIS_EXCEPTRION();
		}
		else
		{
			Status = -1;
			Text = SCantConnect;
		}
		(void)OnQueueNotification.ExecuteIfBound(Status, Text);
	}
}

void SUsersView::OnDeleteUser(const TWeakPtr<FUserDesc>& InUser) const
{
	int8 Status = 0;
	FText Text;
	if (XiaoRedis::IsConnected())
	{
		try
		{
			const std::string Key = TCHAR_TO_UTF8(*InUser.Pin()->Username);
			XiaoRedis::SRedisClient->hdel(XiaoRedis::Hash::SUserDetail, Key);
			Text = LOCTEXT("DeleteUser_Text", "删除用户成功");
			OnUpdate(true);
		}
		CATCH_REDIS_EXCEPTRION();
	}
	else
	{
		Status = -1;
		Text = SCantConnect;
	}

	(void)OnQueueNotification.ExecuteIfBound(Status, Text);
}

void SUsersView::OnRoleChange(const TWeakPtr<FUserDesc>& InUser) const
{
	FString EncryptContent;
	if (!EncryptString(InUser.Pin()->ToJson(), XiaoEncryptKey::SAuth, EncryptContent))
	{
		XIAO_LOG(Error, TEXT("EncryptString Failed!"));
		return;
	}
	
	uint8 Status = 0;
	FText Text;
	if (XiaoRedis::IsConnected())
	{
		const std::string Key = TCHAR_TO_UTF8(*InUser.Pin()->Username);
		const std::string Val = TCHAR_TO_UTF8(*EncryptContent);
		XiaoRedis::SRedisClient->hset(XiaoRedis::Hash::SUserDetail, Key, Val);
		Text = SUpdateSuccess;
	}
	else
	{
		Status = -1;
		Text = SCantConnect;
	}
	(void)OnQueueNotification.ExecuteIfBound(Status, Text);
}

#undef LOCTEXT_NAMESPACE
