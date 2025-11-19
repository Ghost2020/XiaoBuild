/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -12:18 AM
 */
#pragma once

#include "CoreMinimal.h"
#include <string>

#define LOCTEXT_NAMESPACE "XiaoShareField"

static const FString SXiaoBuild(TEXT("XiaoBuild"));

static const FString SSepetator(TEXT(";"));
static const FString SPATH(TEXT("PATH"));
static const FString SGraph(TEXT("graph"));
static const FString SBuilds(TEXT("builds"));
static const FString SSummary(TEXT("summary"));
static const FString SMessages(TEXT("messages"));
static const FString STickStat(TEXT("tick_stat"));
static const FString SActiveWorker(TEXT("active_worker"));

static const FString SXiaoHome(TEXT("XIAO_HOME"));

static const FString SIP(TEXT("ip"));
static const FString SDisplayName(TEXT("display_name"));
static const FString SCore(TEXT("core"));
static const FString SAction(TEXT("actions"));
static const FString SId(TEXT("id"));
static const FString SProjectName(TEXT("project"));
static const FString SName(TEXT("name"));
static const FString SStartTime(TEXT("start"));
static const FString SEndTime(TEXT("end"));
static const FString SDuration(TEXT("duration"));
static const FString SWorkingFlag(TEXT("flag"));
static const FString SWorkingDir(TEXT("working_dir"));
static const FString SCmd(TEXT("cmd"));
static const FString SCmdId(TEXT("cmd_id"));
static const FString SExitCode(TEXT("exit_code"));
static const FString SStatusCode(TEXT("status"));
static const FString SMessage(TEXT("message"));
static const FString SDependentActions(TEXT("dependens"));
static const FString SQuotedActions(TEXT("quoted"));

static const FString SAccumulateTime(TEXT("accu_time"));
static const FString SSend(TEXT("send"));
static const FString SReceive(TEXT("recv"));
static const FString SPing(TEXT("ping"));
static const FString SMemAvail(TEXT("memAvail"));
static const FString SCpuUsage(TEXT("cpu_usage"));
static const FString SConnectionCount(TEXT("connectCount"));
static const FString SKernelTime(TEXT("kernel_time"));
static const FString SUserTime(TEXT("user_time"));
static const FString SDDCTime(TEXT("DPC_time"));
static const FString SInterruptTime(TEXT("interrupt_time"));
static const FString SInterruptCount(TEXT("interrupt_count"));
static const FString SPhysicalTotal(TEXT("physical_total"));
static const FString SPhysicalAvailable(TEXT("physical_available"));
static const FString SVirtualTotal(TEXT("virtual_total"));
static const FString SVirtualLimit(TEXT("virtual_limit"));
static const FString SSoftPageFaults(TEXT("soft_page_faults"));
static const FString SHardPageFaults(TEXT("hard_page_faults"));
static const FString STotalPageFaults(TEXT("total_page_faults"));
static const FString SAssignedCpus(TEXT("assigned_cpus"));
static const FString SUtilizedCpus(TEXT("utilized_cpus"));
static const FString SAssignedPower(TEXT("assigned_power"));
static const FString SUtilizedPower(TEXT("utilized_power"));
static const FString SReadyTasks(TEXT("ready_tasks"));
static const FString SActiveTasks(TEXT("active_tasks"));
static const FString SNeededHelpers(TEXT("needed_helpers"));
static const FString SFilesSynchronized(TEXT("files_synchronized"));
static const FString SDirsSynchronized(TEXT("dirs_synchronized"));
static const FString SKeysSynchronized(TEXT("keys_synchronized"));
static const FString SDirsScanned(TEXT("dirs_scanned"));
static const FString SBytesToDownload(TEXT("bytes_to_download"));
static const FString SFilesToDownload(TEXT("files_to_download"));
static const FString SBytesToWrite(TEXT("bytes_to_write"));

static const FString SBuildID(TEXT("build_id"));
static const FString SBuildStatus(TEXT("build_status"));
static const FString SProjectFolder(TEXT("proj_path"));
static const FString SProjectType(TEXT("proj_type"));
static const FString SBuildVersion(TEXT("build_version"));
static const FString SUser(TEXT("user"));
static const FString SComputer(TEXT("computer"));
static const FString SLogLevel(TEXT("log_level"));
static const FString SErrorNum(TEXT("err_num"));
static const FString SWarningNum(TEXT("war_num"));
static const FString SSystemErrorNum(TEXT("sys_err_num"));
static const FString SSystemWarningNum(TEXT("sys_war_num"));

static const FString SMiddlePath =
#if PLATFORM_CPU_ARM_FAMILY
	TEXT("UBAC/arm64/");
#else
	TEXT("UBAC/x64/");
#endif

namespace XiaoConfig
{
	static const FString SService(TEXT("service"));
	static const FString SListenPort(TEXT("listen_port"));
	static const FString SLogPath(TEXT("log_path"));
	static const FString SLogLevel(TEXT("log_level"));
	static const FString SLicenseService(TEXT("license_service"));

	static const FString SSSL(TEXT("ssl"));
	static const FString SCertPath(TEXT("cert_path"));
	static const FString SKeyPath(TEXT("key_path"));
}

namespace XiaoAppName
{
	static const FString SInstallConsole(TEXT("XiaoInstallConsole"));
	static const FString SBuildConsole(TEXT("XiaoBuildConsole"));
	static const FString SBuildWait(TEXT("XiaoBuildWait"));
	static const FString SBuildSubmit(TEXT("XiaoBuildSubmit"));
	static const FString SBuildApp(TEXT("XiaoApp"));
	static const FString SBuildInstall(TEXT("XiaoBuildInstall"));
	static const FString SBuildMonitor(TEXT("XiaoBuildMonitor"));
	static const FString SBuildSystem(TEXT("XiaoBuildSystem"));
	static const FString SBuildAbout(TEXT("XiaoBuildAbout"));
	static const FString SBuildAgentSettings(TEXT("XiaoAgentSettings"));
	static const FString SBuildAgentService(TEXT("XiaoAgentService"));
	static const FString SBuildCoordiManager(TEXT("XiaoCoordiManager"));
	static const FString SBuildCoordiService(TEXT("XiaoCoordiService"));
	static const FString SBuildLicenseService(TEXT("XiaoLicenseService"));
	static const FString SBuildCacheService(TEXT("XiaoCacheService"));
	static const FString SBuildManagerService(TEXT("XiaoManageService"));
	static const FString SBuildNgLogService(TEXT("XiaoNgLogService"));
	static const FString SBuildPerfService(TEXT("XiaoPerfService"));
	static const FString SBuildTray(TEXT("XiaoTray"));
	static const FString SXiaoCompressor(TEXT("XiaoCompressor"));
	static const FString SXiaoDocument(TEXT("XiaoBuildDocument"));
	static const FString SCacheServer(TEXT("CacheServer"));
	static const FString SIperfServer(TEXT("iperf3"));
	static const FString SUbaAgent(TEXT("UbaAgent"));
	static const FString SUbaCacheService(TEXT("UbaCacheService"));
	static const FString SXiaoScheduler(TEXT("XiaoScheduler"));

	static const FString SVisualStudio(TEXT("devenv"));
	static const FString SRider(TEXT("rider64"));

	static const TSet<FString> SXiaoAppSet = {
		SInstallConsole,
		// SBuildConsole,
		// SBuildWait,
		// SBuildSubmit,
		SBuildApp,
		SBuildTray,
		SUbaAgent,
		SUbaCacheService,
		SXiaoScheduler,
		// SBuildSystem,
		SBuildAgentService,
		// SBuildLicenseService,
		SBuildCoordiService,
		SCacheServer,
		SIperfServer
		// SBuildManagerService
	};

	namespace AppControl
	{
		static const FString SInstall(TEXT("install"));
		static const FString SQuery(TEXT("query"));
		static const FString SEnable(TEXT("enable"));
		static const FString SDisable(TEXT("disable"));
		static const FString SStop(TEXT("stop"));
		static const FString SDelete(TEXT("delete"));
	}
}

namespace XiaoEncryptKey
{
	static const FString SXBMon(TEXT("5LxtxSRttIotKIJ-CyeBG6DhSwEeS4RrZENUo3bfv0E="));
	static const FString SStats(TEXT("f1Hkj7Xe7-d9epPTiaZ+BVEZTj2nvo9Y+SOoZ/wVKzc="));
	// TODO 需要重新生成
	static const FString SAgentSettings(TEXT("f1Hkj7Xe7-d9epPTiaZ+BVEZTj2nvo9Y+SOoZ/wVKzc="));
	// TODO 需要重新生成
	static const FString SAuth(TEXT("f1Hkj7Xe7-d9epPTiaZ+BVEZTj2nvo9Y+SOoZ/wVKzc="));
	// TODO 需要重新生成
	static const FString SCompress(TEXT("f1Hkj7Xe7-d9epPTiaZ+BVEZTj2nvo9Y+SOoZ/wVKzc="));
	// TODO 需要重新生成
	static const FString SLicense(TEXT("f1Hkj7Xe7-d9epPTiaZ+BVEZTj2nvo9Y+SOoZ/wVKzc="));
}

namespace XiaoFileExtension
{
	static const FString SMonitorEx(TEXT("uba"));
}

namespace XiaoMessageQueueParam
{
	// 最大队列长度参数
	static const std::string SMaxLength("x-max-length");
	// 最大消息内容长度
	static const std::string SMaxLengthBytes("max-length-bytes");
	// 消息的长度
	static const std::string SMessageBodyLength("body_length");
	static const std::string SBuildStatus("build_status");
	static const std::string SBuildProgress("build_progress");
	static const std::string SIncrement("increment");
}

namespace XiaoUserParam
{
	static const std::string SUserTable("user");
	static const std::string SUsers("users");
	static const std::string SUsername("username");
	static const std::string SPassword("password");
	static const std::string SRole("role");
	static const std::string SStatus("status");
	static const std::string SNickname("nickname");
	static const std::string SLastLoginMachineId("machine_id");
	static const std::string SLastActive("last_active");
}

namespace XiaoRestUrl
{
	static const FString SUrlRoot("/");
	static const FString SUrlAuth("/auth");
	static const FString SUrlUser("/user");
	static const FString SUrlAgent("/agent");
	static const FString SUrlAgentSettings("/agent/settings");
}

namespace XiaoAgentParam
{
	static const std::string SKEYID("id");
	static const std::string SUniqueIPKey = SKEYID + (" TEXT PRIMARY KEY UNIQUE NOT NULL,");
	
	static const std::string SAgentID("agent_id");

	static const FString SEnableUBAC(TEXT("enable_ubac"));

	static const FString SLocalization(TEXT("localization"));

	static const std::string SAgentGeneral("agent_general");
	static const std::string SIpAddress("ip");
	static const std::string SCpuInfor("cpu_info");
	static const std::string SMacAddress("mac_address");
	static const std::string SAgentGeneralKeepBuildNum("keep_build_num");
	static const std::string SAgentGeneralLevel("level");
	static const std::string SAgentGeneralGeneralDetails("generate_details");
	static const std::string SAgentGeneralGeneralDetailsLocal("generate_details_local");

	static const std::string SAgentPreference("agent_preference");
	static const std::string SAgentPreferenceState("state");
	static const std::string SAgentPreferenceEnableAgentAchedualing("enable_agent_schedule");
	static const std::string SAgentPreferenceFromTime("from_time");
	static const std::string SAgentPreferenceToTime("to_time");
	static const std::string SAgentPreferenceDontShowWindowsFireMsg("dont_show_windows_firewall");

	static const std::string SAgentCpu("agent_cpu");
	static const std::string SAgentCpuDescribe("desc");
	static const std::string SAgentCpuUtilizeCore("utilize_cores");
	static const std::string SAgentCpuEnableHelper("enable_helper");
	static const std::string SAgentCpuHelperCore("helper_core");

	static const std::string SNetworkGeneral("network_general");
	static const std::string SNetworkGeneralAutoSelect("auto_port");
	static const std::string SNetworkGeneralPrimaryPort("primary_port");
	static const std::string SNetworkGeneralSecondPort("second_port");

	static const std::string SNetworkCoordi("network_coordi");
	static const std::string SNetworkCoordiIp("ip");
	static const std::string SNetworkCoordiPort("port");
	static const std::string SNetworkCoordiBackupPort("backup_port");
	static const std::string SNetworkCoordiDesc("desc");
	static const std::string SNetworkCoordiGroup("build_group");

	static const std::string SInitiatorGeneral("initiator_general");
	static const std::string SInitiatorGeneralRestartRemote("restart_remote");
	static const std::string SInitiatorGeneralAvoidTask("avoid_task");
	static const std::string SInitiatorGeneralEnableStandone("enable_standalone");
	static const std::string SInitiatorGeneralLitmitCore("limit_core_num");
	static const std::string SInitiatorGeneralLimitMaxCoreNum("limit_max_num");

	static const std::string SInitiatorAdvance("initiator_advance");
	static const std::string SInitiatorAdvanceWriteOutput("write_output");
	static const std::string SInitiatorAdvanceEnableDirectory("enable_dir");
	static const std::string SInitiatorAdvanceUseThreshold("use_threshold");
	static const std::string SInitiatorAdvanceCompareSeconds("compare_seconds");
	static const std::string SInitiatorAdvanceDistriTaskOnly("distribute_task");
	static const std::string SInitiatorAdvanceTerminateHelperProcess("terminate_helper_pro");
	static const std::string SInitiatorAdvanceAfterSeconds("after_seconds");
	static const std::string SInitiatorAdvanceTerminateInactiveHelper("terminate_inactive_helper");

	static const std::string SInitiatorProcess("initiator_process");
	static const std::string SInitiatorProcessMain("main_initiator");
	static const std::string SInitiatorProcessLocalNonPro("local_non_distri_pri");
	static const std::string SInitiatorProcessLocalPri("local_distri_pri");

	static const std::string SHelperGeneral("helper_general");
	static const std::string SHelperGeneralProcessPririty("process_pririty");
	static const std::string SHelperGeneralFileCache ("file_cache");
	static const std::string SHelperGeneralFileCacheFolder("file_cache_folder");

	static const std::string SHelperCpuAva("helper_cpu_ava");
	static const std::string SHelperCpuAvaIgnoreProcessList("ignore_process_list");
	static const std::string SHelperCpuAvaDontPartipate("dont_participate_power");

	static const std::string SUbaAgent("uba_agent");
	static const std::string SUbaScheduler("uba_Scheduler");
}

namespace XiaoError
{
	static const FText SConnectToCoordinatorTitle = LOCTEXT("CanConnectWithServerTitle_Text", "Failed to connect to XiaoBuild Coordinator...");
	static const FText SConnectToCoordinatorMessage = LOCTEXT("CanConnectWithServerMessage_Text",
		"a.Please ensure that the machine, which is running the XiaoBuild Coordinator is up and online with valid network connection settings.\n\n \
b.Make sure that the same or IP of the Coordinator you provide is correct.\n\n \
c.Check that the Coordinator service \'XiaoCoordiService.exe\' is running on the machine that includes XiaoBuild Coordinator.\n\n \
d.Make sure that the ports that were defined on the machine runing XiaoBuild Coordinator are open.");
}

namespace XiaoUrl
{
	static const FString SXiaoBuildWeb(TEXT("https://github.com/Ghost2020/XiaoBuild"));
	static const FString SGiteeWeb(TEXT("https://gitee.com/Ghost2020/XiaoBuild"));
	static const FString SXiaoBuildManual = SXiaoBuildWeb + TEXT("/blob/main/README.md#");
	static const FString SXiaoBuildChineseMannual = SXiaoBuildWeb + TEXT("/blob/main/中文.md#");
	static const FString SXiaoRedisChannel(TEXT("https://discord.com/widget?id=1082847330258124860&theme=dark"));
}


static const FString SDefaultStr(TEXT("Default"));

static const FText SIdle = LOCTEXT("Idle_TEXT", "闲置的");
static const FText SBelowNormal = LOCTEXT("BelowNormal_TEXT", "低于正常优先级");
static const FText SNormal = LOCTEXT("Normal_TEXT", "正常的");
static const FText SAboveNormal = LOCTEXT("AboveNormal_TEXT", "高于正常的");
static const FText SHigh = LOCTEXT("High_TEXT", "高优先级");
static const FText SRealtime = LOCTEXT("Realtime_TEXT", "实时的");

static const FText SMinimal = LOCTEXT("Minimal_TEXT", "最小");
static const FText SBasic = LOCTEXT("Basic_TEXT", "基础");
static const FText SIntermediate = LOCTEXT("Intermediate_TEXT", "中级");
static const FText SExtended = LOCTEXT("Extended_TEXT", "扩展");
static const FText SDetails = LOCTEXT("Details_TEXT", "细节");

static const FText SAways = LOCTEXT("Aways_TEXT", "总是");
static const FText SContainsErrors = LOCTEXT("ContainsErrors_TEXT", "当包含错误时");
static const FText SContainsErrorsOrWarning = LOCTEXT("ContainsErrorsWarning_TEXT", "当包含错误或警告时");
static const FText SNever = LOCTEXT("Never_TEXT", "绝不");

static const FText SDefault = LOCTEXT("Default_TEXT", "默认");


#undef LOCTEXT_NAMESPACE