/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#pragma once

#include "CoreMinimal.h"
#include "build_progress.pb.h"

namespace boost::interprocess
{
	class shared_memory_object;
	class mapped_region;
}
using namespace boost::interprocess;
class QSystemTrayIcon;
class QAction;
class QMenu;


class FXiaoTray
{
public:
	explicit FXiaoTray();

	bool InitApp();
	void RunApp();

protected:
	void OnChangeState(const int State, const float Progress, QSystemTrayIcon* InTrayIcon) const;
	void OnUpdate(QSystemTrayIcon* InTrayIcon);

private:
	void OnPullMessage(QSystemTrayIcon* InTrayIcon);
	bool OnCheckUpdate() const;
	void OnUpdateLocalization(const FString& InLocalizaiton);
	static void OnDeleteFolderFiles(const FString& InFolder);
	static void OnCleanTempFolder();

private:
	FBuildProgress ProgressStatus;
	bool bAgentServiceState = false;
	float TotalTime = 5.f;

	TUniquePtr<shared_memory_object> ProgressShm = nullptr;
	TUniquePtr<mapped_region> ProgressRegion = nullptr;

	bool bBuilding = false;
	bool bEnableAsHelper = false;
	bool bCanSyncUpdate = false;
	FString Localization = TEXT("zh-CN");

	QAction* SyncAction = nullptr;
	QAction* DetectAction = nullptr;;
	QAction* DocumentAction = nullptr;;
	QAction* AboutAction = nullptr;;
	QAction* LogAction = nullptr;;
	QMenu* ClearMenu = nullptr;;
	QAction* ClearTempAction = nullptr;;
	QAction* ClearAgentAction = nullptr;;
	QAction* ClearSchedulerAction = nullptr;;
	QAction* HistoryAction = nullptr;;
	QAction* NetworkTestAction = nullptr;;
	QAction* CoordiManagerAction = nullptr;;
	QAction* AgentSettingsAction = nullptr;;
	QAction* ExitAction = nullptr;;
};