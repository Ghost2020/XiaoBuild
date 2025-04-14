/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:08 PM
 */

#pragma once

#include "Service/GenericService.h"
#include "Platform/Windows/WindowsService.h"

/*
 * windows服务
 */
class FWindowsCoordiService final : public FGenericService
{
public:
	explicit FWindowsCoordiService(const FServiceCommandLineOptions& InOptions, const FServiceDesc& InServiceDesc);
	virtual ~FWindowsCoordiService() override;

	// ~ Begin Override FGenericService
	virtual bool OnInstall() override;
	virtual bool OnRegister() override;
	virtual bool OnStart() override;
	virtual bool OnEnable() override;
	virtual bool OnDisable() override;
	virtual bool OnQuery() override;
	virtual bool OnTick(const float Internal) override;
	virtual bool OnPause() override;
	virtual bool OnContinue() override;
	virtual bool OnStop() override;
	virtual bool OnDelete() override;
	// ~ End Override FGenericService

private:
	static void WINAPI ServiceCtrlHandler(const DWORD InDwCtrl);

public:
	static void MainProc(DWORD DwArgc, LPWSTR* LpszArgv);
};

typedef FWindowsCoordiService FXiaoService;
