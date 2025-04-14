/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"
#include "agent.pb.h"

static const FName S_ColumnIdIndex = TEXT("Index");
static const FName S_ColumnIdInfor = TEXT("Information");
static const FName S_ColumnIdDesc = TEXT("AgentDesc");
static const FName S_ColumnIdGroup = TEXT("Group");
static const FName S_ColumnIdAvaCpu = TEXT("AvaCpu");
static const FName S_ColumnIdAvaNet = TEXT("AvaNet");
static const FName S_ColumnIdAvaGpu = TEXT("AvaGPU");
static const FName S_ColumnIdLastConnected = TEXT("LastCon");
static const FName S_ColumnIdUsedHelpersCache = TEXT("UsedHelpersCache");
static const FName S_ColumnIdRegisteredHelperCores = TEXT("RegHelCores");
static const FName S_ColumnIdMaxConnectAgentNum = TEXT("MaxAgentCon");
static const FName S_ColumnIdMaxProcessors = TEXT("MaxPros");
static const FName S_ColumnIdMaxLocalProcessors = TEXT("MaxLocalPros");
static const FName S_ColumnIdBuildCache = TEXT("BuildCache");
static const FName S_ColumnIdBuildPriority = TEXT("BuildPri");
static const FName S_ColumnIdCpuInfo = TEXT("CpuInfo");
static const FName S_ColumnIdCpuArch = TEXT("CpuArch");
static const FName S_ColumnIdGpuInfo = TEXT("GpuInfo");
static const FName S_ColumnIdFreeDiskSpace = TEXT("FreeDS");
static const FName S_ColumnIdLogLevel = TEXT("LogLevel");
static const FName S_ColumnIdLoggedOnUser = TEXT("LoggedOnUser");
static const FName S_ColumnIdAssignmentPriority = TEXT("AssiPri");
static const FName S_ColumnIdAvailableMemory = TEXT("AvaMem");
static const FName S_ColumnIdAvailableVideoMemory = TEXT("AvaVideoMem");
static const FName S_ColumnIdLogicCores = TEXT("LogicCores");
static const FName S_ColumnIdMacAddress = TEXT("MacAddress");
static const FName S_ColumnIdNetwork = TEXT("Network");
static const FName S_ColumnIdOSSystem = TEXT("OSSystem");
static const FName S_ColumnIdPhysicalCores = TEXT("PhysicalCores");
static const FName S_ColumnIdRoutingIP = TEXT("RoutingIP");
static const FName S_ColumeIdListenPort = TEXT("ListenPort");
static const FName S_ColumnIdPortMappedAddress = TEXT("Port-mappedAddress");
static const FName S_ColumnIdUpDownTime = TEXT("UpDownTime");
static const FName S_ColumeIdResetState = TEXT("ResetState");

class SAgentListRow final : public SMultiColumnTableRow<TSharedPtr<FAgentProto>>
{
public:
	DECLARE_DELEGATE_OneParam(FOnAgentChanged, const TWeakPtr<FAgentProto>&);
	
	SLATE_BEGIN_ARGS(SAgentListRow){}
		SLATE_ARGUMENT(TSharedPtr<FAgentProto>, AgentProto)
		SLATE_EVENT(FOnAgentChanged, OnAgentChanged)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override;

	bool IsEnableEdit() const;

protected:
	const FSlateBrush* GetImage(const bool bFixed);
	uint32 GetLogicCoreNum() const;
	void GetStatusDesc(FText& OutText, FText& OutToolTip, FLinearColor& OutColor);
	static FORCEINLINE bool IsValidIp(const FString& InIp);
	static FORCEINLINE bool IsValidAddress(const FString& InAddress);

private:
	TWeakPtr<FAgentProto> AgentProto = nullptr;
	FOnAgentChanged OnAgentChanged;
	bool bEditing = false;
};
