/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */
#pragma once

#include "CoreMinimal.h"
#include "XiaoAppBase.h"

class FCoordiManagerApp final : public FXiaoAppBase
{
public:
	explicit FCoordiManagerApp(FAppParam& InParam);

	virtual bool InitApp() override;
	virtual void ShutApp() override;

	static bool CheckCertificate();

	void ShowMainWindow();
	void ShowLoginWindow();
	void ShowLicenseLockedWindow();

	double LastLoginTime = 0.0f;
private:
	void InitGlobalData() const;

private:
	mutable TWeakPtr<class SCoordinatorWindow> MainWindow = nullptr;
	mutable TWeakPtr<SWindow> LoginWindow = nullptr;
};
