/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#include "XiaoInstallConsole.h"
#include <iostream>
#include "XiaoShare.h"
#include "XiaoLog.h"
#include "XiaoInstall.h"
#include "Install.h"
#include "Version.h"

#include "RequiredProgramMainCPPInclude.h"

IMPLEMENT_APPLICATION(XiaoInstallConsole, XB_PRODUCT_NAME);

static void PrintHelp()
{
	const std::string HelpString =
"-----------------------------------------------------------------------\n"
+ std::string(XB_PRODUCT_NAME) + std::string(" ") + TCHAR_TO_UTF8(XB_VERSION_STRING) + std::string("\n")
+ std::string(XB_COPYRIGHT_STRING) + std::string("\n") +
"-----------------------------------------------------------------------\n"
"\n "
"Note:\n\n"
"   The Automated installer must be run from the CMD as Administrator\n\n"
"General Options:\n"
"\n"
" -install						This flag defines the action as installation.\n"
"							To uninstall,repair,or update see the section below.\n"
" -uninstall						To Uninstall Xiaobuild.\n"
" -update						To Upgrade Xiaobuild.\n"
" -repair						To repair Xiaobuild Installation.\n"
" -components						Defines what components you are installing.\n"
"							Can be agent, coordinator, or agent,\n"
"							coordinator to install both at one.\n"
" -coordinator						The IP address or hostname of the associated Coordinator\n"
"							Mandatory forthe installation of an Agent and Backup Coordinator\n"
"							If the Coordinator does NOT use the default port number(31104)\n"
"							you should add the port to the Coordinator name and\n"
"							put them both in quotes as follows:\"coordinator_name:port_no.\"\n"
"							For axample \"192.168.188.02:12345\"\n"
" -coordinator:backup					Use this flag to indicate that the coordinator you are \n"
"							installing is a backup coordinator.\n"
" -help							View usage information adn full list of flags.\n"
" -add_to_path={on|off}					Controls whether the XiaoBuild installation folder \n"
"							is added to the system search path.\n"
" -install_dir=\"{path}\"					Allows manual specification of the XiaoBuild\n"
"installation folder.\n"
"\n"
"                       Agent Options\n"
"\n"
" -agent:auto_select_ports				Controls whether setup should automaticaly select the\n"
"							first available TCP/IP ports for the Agent Service\n"
"							and Helper cores.If this option is on,\n"
"							any manual port specification will be overriden.\n"
" -agent:file_cache={number in MBs}			Allows manual specification,\n"
"							in MBs,of the maximum size of the Agent file cache folder.\n"
" -agent:group={group name}					Assign an Agent to Build Group during installation.\n"
"							If the name you specify does not exist,\n"
"										The Build Group will be created.\n"
" -agent:help_port={port no}				Allows manual specification of the Agent Helper port,\n"
"							Required for the communication between\n"
"							an Initiator and the Helper cores.\n"
"							This manual specification will take effect\n"
"							only if the/agent:auto_select_ports option is off.\n"
" -agent:install_addins={on|off}			controls whether the installation procedure should\n"
"							also install Xiaobuild Add-in/Extension in Visual Studio IDE.\n"
" -agent:open_firewall={on|off}					Controls whether setup should open Agent ports in the Windows Firewall.\n"
"												Limitation:This option should not be used together with -agent:auto_select_ports.In this case, Xiaobuild does not know which ports\n"
"												will be needed in advance, and you will be required to manually open all required ports on your Firewall.\n"
" -agent:service_port={port no.}			Allows manual specification of the Xiaobuild Agent\n"
"							Service port, required for the communication with the Coordinator.\n"
"							This manual specification will take effect\n"
"							only if the -agent:auto_select_ports option is OFF\n"
" -agent:agent_role={helper|initiator/both}		Indicates the type of the Agent license.\n"
" -agent:initiator_type={fixed|floating|ci_fixed|ci_floating}  Defines the license type of the Initiator.\n"
" -agent:helper_type={fixed|floating}			Defines the license type of the helper.\n"
" -agent:helper_cores={#|%}				Limites the number of cores that can be used when agent is helping workload distribution.Can be specified in real\n"
"												number of cores or percentage of total core count.\n"
" -agent:description={desc}						description the agent\n"
" -agent:ssl_key=\"{path to ,key file}\"			If you are using SLL, the path to the key you want to place on the Agent\n"
" -agent:ssl_cer=\"{path to .crt file}\"			If you are using SLL, the path to the certificate you want to place on the Agent\n"
" -agent:build_cache={on|off}				Assign a Build Cache license to the Initiator Agent.The Agent will be enabled as a Build Cache Endpoint with the default settings.\n"
" -agent:build_cache_service_port={port no.}    Define the value of the Build Cache Endpoint port.\n"
" -agent:db_dir=\"{path}\"						The database to be used for Build Cache. If nothing is specified, the default database will be automatically installed on each Initiator during installation.\n"
" -agent:cache_dir=\"{path}\"				The location of the folder that will contain the Build Cache files\n"
" -agent:max_file_cache_size={#}					The maximum size of the Build Cache folder.\n"
"\n"
"                       Coordinator Options\n"
"\n"
" -coordi:username={user name}				The username to access the Coordinator.Required for clean installation(not repair).\n"
"											The username must be bettwen 4-20 characters and cannot contain spaces or special characters.\n"
" -coordi:password={password}				The password to access the Coordinator,Rquired for clean installation(not repair)\n"
"											The password must be at least 8 characters and contain at least one upper characters.\n"
" -coordi:open_firewall={on|off}                Controls whether setup should open the Coordinator service ports in the Windows Firewall.\n"
" -coordi:database_dir=\"{path to database}\"   The directory for the Coordinator's databse\n"
" -coordi:service_port={port no.}					Define the value of the Agent Communications port.\n"
" -coordi:build_manager_port={port no.}				Define the value of the Xiaobuild Manager port.\n"  
" -coordi:perf_port={port no.}				Define the value of the Message Transport port.\n"
" -coordi:coordi_api_port={port no.}				Define the value of the Coordinator Service port.\n"
" -coordi:ssl_key=\"{path to .key.}\"		If you are using SSL, the path to the key you want to place on the Coordinator.\n"
" -coordi:ssl_cer=\"{path to .crt.}\"		If you are using SSL, the path to the certificate you want to place on the Coordinator.\n"
" -accpet_eula									When instaling or upgrading a Coordinator, this flag automaticaly accepts the End User\n"
" -install_setting={file path where the install_setting.json}\n"
" \n"
"	Examples\n"
"		Install Coordinator Only\n"
"		{path}/XiaoInstallConsole.exe -install -components=coordinator -coordi:username={user name} -coordi:password={password}\n"
"\n"
"		Install Agent and Coordinator\n"
"		{path}/XiaoInstallConsole.exe -install -components=coordinator,agent -coordinator={coord name}:{port no.}\n"
"									  [General Options] - coordi:username = {user name} - coordi : password = {password}\n"
"		Install Agent Only\n"
"		{path}/XiaoInstallConsole.exe -install -components=agent -agent:agent_role=helper -agent:helper_type=fixed -agent:helper_cores=4 -coordinator={coord name}:{port no.}\n"
;
    std::cout << HelpString << std::endl;
}

static bool ParseCommandLine(const int32 ArgC, TCHAR* ArgV[])
{
	const FString CmdLine = FCommandLine::BuildFromArgV(nullptr, ArgC, ArgV, nullptr);
	if(CmdLine.IsEmpty())
	{
		PrintHelp();
		return false;
	}

	FCommandLine::Set(*CmdLine);

	if (!FParse::Value(*CmdLine, TEXT("-ppid="), SPPID))
	{
		XIAO_LOG(Warning, TEXT("pid not find"));
	}

	if (FParse::Param(*CmdLine, TEXT("install")) || FParse::Param(*CmdLine, TEXT("update")))
	{
		if (!TryCreateIPC())
		{
			return false;
		}
	}

	FString Out;
	FString InstallSettingPath;
	if (FParse::Value(*CmdLine, TEXT("-install_setting="), InstallSettingPath))
	{
		if(!FPaths::FileExists(InstallSettingPath))
		{
			XIAO_LOG(Error, TEXT("install_setting file not exist::%s"), *InstallSettingPath);
			return false;
		}

		FString SettingString;
		if (!FFileHelper::LoadFileToString(SettingString, *InstallSettingPath))
		{
			XIAO_LOG(Error, TEXT("LoadFileToString Failed::%s"), *InstallSettingPath);
			return false;
		}

		if (!GInstallSettings.FromJson(SettingString))
		{
			XIAO_LOG(Error, TEXT("FromJson Failed::%s"), *InstallSettingPath);
			return false;
		}

		UpdateMessage(0.13f, TEXT("Parse install config"));
		return true;
	}

	if(FParse::Param(*CmdLine, TEXT("help")))
	{
		return false;
	}
	if(FParse::Param(*CmdLine, TEXT("install")))
	{
		GInstallSettings.SetupUpType = IT_Install;
		// TODO 安装所在目录不能超过 MAX_PATH - 10
	}
	if(FParse::Param(*CmdLine, TEXT("uninstall")))
	{
		GInstallSettings.SetupUpType = IT_Unistall;
	}
	if(FParse::Param(*CmdLine, TEXT("update")))
	{
		GInstallSettings.SetupUpType = IT_Update;
	}
	/*if(FParse::Param(*CmdLine, TEXT("repair")))
	{
		GInstallSettings.SetupUpType = IT_Repair;
	}
	if (FParse::Param(*CmdLine, TEXT("coordinator:backup")))
	{
		GInstallSettings.SetupUpType = IT_Backup;
	}*/
	
	if (FParse::Value(*CmdLine, TEXT("components"), Out))
	{
		Out = Out.RightChop(1);
		if (Out == TEXT("agent"))
		{
			GInstallSettings.InstallType = CT_AgentVisualer;
		}
		else if (Out == TEXT("coordinator"))
		{
			GInstallSettings.InstallType = CT_AgentAndCoordi;
		}
		else if(Out == TEXT("all"))
		{
			GInstallSettings.InstallType = CT_None;
		}
		else 
		{
			XIAO_LOG(Error, TEXT("Not valid components=\"%s\""), *Out);
			return false;
		}
	}

	if ((GInstallSettings.InstallType | CT_Agent) == CT_Agent)
	{
		if (FParse::Param(*CmdLine, TEXT("agent:auto_select_ports")))
		{
			GAgentAutoSelectPort = true;
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:file_cache"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty() && Out.IsNumeric())
			{
				GInstallSettings.FileCache = FCString::Atoi(*Out);
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:group"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty())
			{
				GAgentBuildGroup = Out;
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:help_port"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty() && Out.IsNumeric())
			{
				GInstallSettings.HelpListenPort = FCString::Atoi(*Out);
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:install_addins"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty())
			{
				GAgentInstallAddsOn = (Out == TEXT("on") ? true : false);
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:open_firewall"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty())
			{
				GInstallSettings.bAutoOpenFirewall = (Out == TEXT("on") ? true : false);
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:service_port"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty() && Out.IsNumeric())
			{
				GInstallSettings.AgentListenPort = FCString::Atoi(*Out);
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:agent_role"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty())
			{
				GAgentType = (Out == TEXT("helper") ? AT_Helper : ((Out == TEXT("initiator")) ? AT_Initiator : AT_Both));
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:initiator_type"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty())
			{
				GInitiatorType = (Out == TEXT("fixed") ? InitiatorType_Fixed : ((Out == TEXT("floating")) ? InitiatorType_Floating : ((Out == TEXT("ci_fixed")) ? InitiatorType_CiFixed : InitiatorType_CiFloating)));
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:helper_type"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty())
			{
				GHelperType = (Out == TEXT("fixed") ? ET_Fixed : ET_Floating);
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:helper_cores"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty() && Out.IsNumeric())
			{
				GHelperCores = FCString::Atoi(*Out);
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:description"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty() && Out.IsNumeric())
			{
				GAgentDesc = Out;
			}
		}

		FString SSLKeyPath;;
		if (FParse::Value(*CmdLine, TEXT("-agent:ssl_key"), SSLKeyPath) && FPaths::FileExists(SSLKeyPath))
		{
			GInstallSettings.KeyFile = SSLKeyPath;	
		}

		FString SSLCerPath;
		if (FParse::Value(*CmdLine, TEXT("-agent:ssl_cer"), SSLCerPath) && FPaths::FileExists(SSLCerPath))
		{
			GInstallSettings.CertFile = SSLCerPath;
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:build_cache"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty())
			{
				GAgentBuildCache = Out == TEXT("on") ? true : false;
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:build_cache_service_port"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty() && Out.IsNumeric())
			{
				GInstallSettings.CacheServicePort = FCString::Atoi(*Out);
			}
		}

		FString DbDirPath;
		if (FParse::Value(*CmdLine, TEXT("-agent:db_dir="), SSLKeyPath) && FPaths::FileExists(DbDirPath))
		{
			GAgentDbDir = DbDirPath;
		}

		FString CacheDirPath;
		if (FParse::Value(*CmdLine, TEXT("-agent:cache_dir="), SSLKeyPath) && FPaths::FileExists(CacheDirPath))
		{
			GAgentCacheDir = CacheDirPath;
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("agent:max_file_cache_size"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty() && Out.IsNumeric())
			{
				GAgentMaxFileCacheSize = FCString::Atoi(*Out);
			}
		}
	}
	if ((GInstallSettings.InstallType | CT_Coordinator) == CT_Coordinator)
	{
		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("coordi:username"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty())
			{
				GInstallSettings.Username = Out;
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("coordi:password"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty())
			{
				GInstallSettings.Password = Out;
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("coordi:open_firewall"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty())
			{
				GInstallSettings.bAutoOpenFirewall = (Out == TEXT("on") ? true : false);
			}
		}

		FString InstallSettingPath;
		if (FParse::Value(*CmdLine, TEXT("-coordi:database_dir="), InstallSettingPath) && FPaths::DirectoryExists(InstallSettingPath))
		{
			GCoordiDatabaseDir = InstallSettingPath;
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("coordi:service_port"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty() && Out.IsNumeric())
			{
				GInstallSettings.CoordiListenPort = FCString::Atoi(*Out);
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("coordi:iperf_port"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty() && Out.IsNumeric())
			{
				GInstallSettings.PerfTransport = FCString::Atoi(*Out);
			}
		}

		Out = TEXT("");
		if (FParse::Value(*CmdLine, TEXT("coordi:coordi_api_port"), Out))
		{
			Out = Out.RightChop(1);
			if (!Out.IsEmpty() && Out.IsNumeric())
			{
				// TODO
				GInstallSettings.LicenseListenPort = FCString::Atoi(*Out);
			}
		}

		FString SSLKeyPath;
		if (FParse::Value(*CmdLine, TEXT("-coordi:ssl_key="), SSLKeyPath) && FPaths::FileExists(SSLKeyPath))
		{
			GInstallSettings.KeyFile = SSLKeyPath;
		}

		FString SSLCerPath;
		if (FParse::Value(*CmdLine, TEXT("-coordi:ssl_cer="), SSLCerPath) && FPaths::FileExists(SSLCerPath))
		{
			GInstallSettings.CertFile = SSLCerPath;
		}
	}

	return true;
}

static void BeforeExit()
{
	ReleaseAppMutex();
	FEngineLoop::AppExit();
}

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	GNeedFlush = true;
	XIAO_LOG(Log, TEXT("XiaoInstallConsole::Begin!"));

	atexit(BeforeExit);

	if (!IsRunAsAdmin())
	{
		XIAO_LOG(Error, TEXT("Not Run As Admin!"));
		return -1;
	}

	if (!CheckSingleton())
	{
		XIAO_LOG(Error, TEXT("Must run in singleton mode!"));
		return -1;
	}

	if (!ParseCommandLine(ArgC, ArgV))
	{
		PrintHelp();
		return 0;
	}

	if (GInstallSettings.SetupUpType != EInstallType::IT_Unistall)
	{
		FString Error;
		if (!CheckEnvRuning(Error, SPPID))
		{
			XIAO_LOG(Error, TEXT("%s"), *Error);
			return -1;
		}
	}
	
	bool Rtn = false;
	if (GInstallSettings.SetupUpType == IT_Install)
	{
		Rtn = OnInstall();
	}
	else if (GInstallSettings.SetupUpType == IT_Update)
	{
		Rtn = OnUpdate();
	}
	else if (GInstallSettings.SetupUpType == IT_Unistall)
	{
		Rtn = UnInstall();
	}

	XIAO_LOG(Log, TEXT("XiaoInstallConsole::Finish!"));

	return Rtn ? 0 : -1;
}