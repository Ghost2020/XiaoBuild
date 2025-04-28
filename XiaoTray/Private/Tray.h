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
	void OnUpdateLocalization(const FString& InLocalization);
	static void OnDeleteFolderFiles(const FString& InFolder);
	static void OnCleanTempFolder();

private:
	FBuildProgress ProgressStatus;
	bool bAgentServiceState = false;
	float TotalTime = 5.f;

	TUniquePtr<shared_memory_object> ProgressShm;
	TUniquePtr<mapped_region> ProgressRegion;

	bool bBuilding = false;
	bool bEnableAsHelper = false;
	bool bCanSyncUpdate = false;
	FString Localization = TEXT("zh-CN");

	QAction* SyncAction;
	QAction* DetectAction;
	QAction* DocumentAction ;
	QAction* AboutAction;
	QAction* LogAction;
	QMenu* ClearMenu;
	QAction* ClearTempAction;
	QAction* ClearAgentAction;
	QAction* ClearSchedulerAction;
	QAction* HistoryAction;
	QAction* NetworkTestAction;
	QAction* CoordiManagerAction;
	QAction* AgentSettingsAction;
	QAction* ExitAction;
};
