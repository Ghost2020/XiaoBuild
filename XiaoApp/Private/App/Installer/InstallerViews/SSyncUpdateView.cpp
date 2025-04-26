/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SSyncUpdateView.h"
#include "ShareWidget.h"
#include "XiaoShareRedis.h"

#define LOCTEXT_NAMESPACE "SSyncUpdateView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSyncUpdateView::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Top)
		[
			SAssignNew(ErrorText, SErrorText)
		]

		TOP_WIDGET

		+ SVerticalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).AutoHeight()
		[
			SNew(SImage)
				.Image(FXiaoStyle::Get().GetBrush(TEXT("Installer/SyncUpdate")))
		]
	];

	CanSyncUpdate();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

bool SSyncUpdateView::OnCanBack()
{
	return false;
}

bool SSyncUpdateView::OnCanNext()
{
	return bCanUpdate;
}

FText SSyncUpdateView::GetNextTitle()
{
	return LOCTEXT("SyncUpdate_Text", "更新");
}

bool SSyncUpdateView::IsFinal()
{
	return false;
}

bool SSyncUpdateView::CanSyncUpdate()
{
	// 如果本地就有缓存服务器则跳过
	bCanUpdate = false;
	if (IsAppRunning(XiaoAppName::SCacheServer))
	{
		ErrorText->SetError(LOCTEXT("LocalMachineIsCacheServer_Text", "当前机器已经部署了缓存服务器，不支持缓存同步！"));
		return false;
	}

	const FString XiaoHome = FPlatformMisc::GetEnvironmentVariable(TEXT("XIAO_HOME"));
	if (!FPaths::DirectoryExists(XiaoHome))
	{
		ErrorText->SetError(LOCTEXT("XiaoHomeVavLost_Text", "环境变量\"XIAO_HOME\"不存在！"));
		XIAO_LOG(Error, TEXT("SyncVersion failed::XIAO_HOME::[%s] not exist!"), *XiaoHome);
		return false;
	}
	
	std::string Data;
	if (!GetCanSyncUpdate(Data))
	{
		ErrorText->SetError(LOCTEXT("NotAvalibaleUpdate_Text", "没有可供更新的数据"));
		return false;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE