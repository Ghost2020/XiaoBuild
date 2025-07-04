/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#include "Misc/FileHelper.h"
#include "Async/Async.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "XiaoShareField.h"
#include "XiaoLog.h"
#ifdef SOCKETS_API
#include "SocketSubsystem.h"
#include "Sockets.h"
#endif

#include "XiaoInterprocess.h"
#include <iostream>

#if PLATFORM_LINUX
#include <netinet/tcp.h>
#include "Misc/ScopeExit.h"
#endif

#if PLATFORM_WINDOWS
#ifndef FALSE
#define FALSE 0
#endif
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <ipifcons.h>
#include <Mstcpip.h>
#pragma comment (lib, "Netapi32.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib") // For GetAdaptersInfo

#ifdef SetPort
#undef SetPort
#pragma push_macro("SetPort")
#endif
#else
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define TIMEVAL timeval
#define SOCKET_ERROR -1
#define SOCKET int
#define INVALID_SOCKET -1
#define SD_BOTH SHUT_RDWR
#define WSAHOST_NOT_FOUND 0
#define WSAENOTCONN ENOTCONN
#define WSAEADDRINUSE EADDRINUSE
#define closesocket(a) close(a)
#define addrinfoW addrinfo
#define GetAddrInfoW getaddrinfo
#define FreeAddrInfoW freeaddrinfo
#define WSAGetLastError() errno
#define strcpy_s(a, b, c) strcpy(a, c)
#define WSAPOLLFD pollfd
#define WSAPoll poll
#endif


#ifndef XIAO_LOG
#define XIAO_LOG(Verbosity, FMT, ...)
#endif


namespace XiaoNetwork
{
	static const uint16 SProbeRange = 20;

	// 默认使用监听端口
	inline uint16 SCoordiServicePort = 37000;
	inline uint16 SLicenseServicePort = 37020;
	inline uint16 SCacheServicePort = 37040;
	inline uint16 SUIServicePort = 37060;
	inline uint16 SIPerfServicePort = 37080;
	inline uint16 SAgentServicePort = 1345;
	inline uint16 SSchedulerServerPort = 1346;
	inline uint16 SHelpListenPort = SAgentServicePort;

	// 服务地址端口
	inline uint16 SListenPort = 0;
	inline FString SLocalAgentListen = FString::Printf(TEXT("http://localhost:%d"), SAgentServicePort);
	inline FString SLicenseServiceListen = FString::Printf(TEXT("http://localhost:%d"), SLicenseServicePort);
	inline FString SCoordiServiceListen = FString::Printf(TEXT("http://localhost:%d"), SCoordiServicePort);
	inline FString SCacheServiceListen = FString::Printf(TEXT("http://localhost:%d"), SCacheServicePort);
	inline FString SPerfServiceListen = FString::Printf(TEXT("http://localhost:%d"), SIPerfServicePort);

	static bool ReadConfig(TSharedPtr<FJsonObject>& ConfigObject)
	{
		XIAO_LOG(Log, TEXT("ReadConfig Begin!"));

		const FString ConfigPath = FPaths::Combine(FPlatformProcess::GetCurrentWorkingDirectory(), TEXT("../../Config/config.json"));
		if (!FPaths::FileExists(ConfigPath))
		{
			XIAO_LOG(Error, TEXT("Config file not found::%s!"), *ConfigPath);
			return false;
		}

		FString Content;
		if (!FFileHelper::LoadFileToString(Content, *ConfigPath))
		{
			XIAO_LOG(Error, TEXT("LoadFileToString Failed::%s!"), *ConfigPath);
			return false;
		}

		TSharedRef<FJsonStringReader> JsonReader = FJsonStringReader::Create(FString(Content));
		if (!FJsonSerializer::Deserialize(JsonReader.Get(), ConfigObject))
		{
			XIAO_LOG(Error, TEXT("Deserialize Failed!"));
			return false;
		}

		const TSharedPtr<FJsonObject>* ServiceObj = nullptr;
		if (!ConfigObject->TryGetObjectField(XiaoConfig::SService, ServiceObj))
		{
			XIAO_LOG(Error, TEXT("Get \"service\" Field Failed!"));
			return false;
		}

		const TSharedPtr<FJsonObject>* PortObj = nullptr;
		if (!(*ServiceObj)->TryGetObjectField(TEXT("port"), PortObj))
		{
			XIAO_LOG(Error, TEXT("Get \"port\" Field Failed!"));
			return false;
		}

		XIAO_LOG(Log, TEXT("ReadConfig Finish!"));
		return true;
	}

	static void ToggleProxyServer(const bool bInOnOrOff)
	{
#if PLATFORM_WINDOWS
		HKEY hKey;
		LPCWSTR regPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";

		if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
			DWORD proxyEnable = bInOnOrOff ? 1 : 0;
			if (RegSetValueEx(hKey, L"ProxyEnable", 0, REG_DWORD, (const BYTE*)&proxyEnable, sizeof(proxyEnable)) == ERROR_SUCCESS) 
			{
				XIAO_LOG(Log, TEXT("proxy changed::%u"), proxyEnable);
			}
			else 
			{
				XIAO_LOG(Error, TEXT("Can\'t set ProxyEnable key。"));
			}

			// 清空 ProxyServer 值
			if (!bInOnOrOff)
			{
				if (RegSetValueEx(hKey, L"ProxyServer", 0, REG_SZ, (const BYTE*)L"", sizeof(L"")) == ERROR_SUCCESS)
				{
					XIAO_LOG(Log, TEXT("Proxy server has changged。"));
				}
				else
				{
					XIAO_LOG(Error, TEXT("Can\'t set ProxyServer。"));
				}
			}

			RegCloseKey(hKey);
		}
		else 
		{
			XIAO_LOG(Error, TEXT("无法打开注册表键。"));
		}
#endif
	}

	static bool IsReachability(const std::string& InIp, const uint16 InPort, std::string& OutError)
	{
		try
		{
#ifdef XIAO_USE_BOOST
			boost::asio::io_context ioContext;
			boost::asio::ip::tcp::resolver resolver(ioContext);
			boost::asio::ip::tcp::socket socket(ioContext);

			auto endpoints = resolver.resolve(InIp, std::to_string(InPort));
			boost::asio::connect(socket, endpoints);
			return true;
#endif
			return false;
		}
		catch (const std::exception& E)
		{
			OutError = E.what();
			return false;
		}
	}

	static bool IsPortAvailable(const uint16& InProbePort)
	{
#ifdef SOCKETS_API
		if (ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM))
		{
			if (TSharedPtr<FSocket> TestSocket = MakeShareable(SocketSubsystem->CreateSocket(NAME_Stream, TEXT("ProbeSocket"), false)))
			{
				TSharedRef<FInternetAddr> LocalHostAddr = SocketSubsystem->CreateInternetAddr(FNetworkProtocolTypes::IPv4);
				LocalHostAddr->SetAnyAddress();
				LocalHostAddr->SetPort(InProbePort);
				if (TestSocket->Bind(*LocalHostAddr))
				{
					return true;
				}
			}
			XIAO_LOG(Error, TEXT("Can\'t bind socket with port \"%u\",LAST SOCKET ERROR::%s"), InProbePort, SocketSubsystem->GetSocketError());
		}
		else
		{
			XIAO_LOG(Error, TEXT("Can\'t get socket subsystem"));
		}
		return false;
#elif PLATFORM_WINDOWS
		WSADATA wsaData;
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0) 
		{
			XIAO_LOG(Error, TEXT("WSAStartup failed:%d"), result);
			return false;
		}

		// 创建套接字
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET) 
		{
			XIAO_LOG(Error, TEXT("Error at socket():%d"), WSAGetLastError());
			WSACleanup();
			return false;
		}

		// 设置端口复用选项，允许立即重用地址
		int optval = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));

		// 绑定地址
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(InProbePort);

		result = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
		if (result == SOCKET_ERROR) 
		{
			if (WSAGetLastError() == WSAEADDRINUSE) 
			{
				closesocket(sock);
				WSACleanup();
				return false; // 端口被占用
			}
		}

		// 清理资源
		closesocket(sock);
		WSACleanup();
		return true; // 端口未被占用
#else
		const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			XIAO_LOG(Error, TEXT("Socket creation failed: %hs"), strerror(errno));
			return false;
		}

		const int opt = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			XIAO_LOG(Warning, TEXT("setsockopt failed: %hs"), strerror(errno));
		}

		struct sockaddr_in addr;
		std::memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(InProbePort);

		const int bindResult = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));

		close(sockfd);

		if (bindResult == 0)
		{
			return true;
		}
		
		if (errno == EADDRINUSE)
		{
			XIAO_LOG(Info, TEXT("Port %d is already in use."), InProbePort);
		}
		else
		{
			XIAO_LOG(Error, TEXT("Bind failed: %hs"), strerror(errno));
		}
		return false;
#endif
	}

	static bool GetUsablePort(const uint16& InPortStart, uint16& OutFirstIdlePort)
	{
		for (uint16 Port = InPortStart; Port < (InPortStart + SProbeRange); ++Port)
		{
			if (IsPortAvailable(InPortStart))
			{
				OutFirstIdlePort = Port;
				return true;
			}
		}

		return false;
	}

	static TArray<FString> GetAllLAN4()
	{
		TArray<FString> IPAddresses;

#if PLATFORM_WINDOWS
		// Fallback code for some cloud setups where we can't use the dns to find out ip addresses. (note it always work by providing the adapter we want to listen on)
		IP_ADAPTER_INFO info[16];
		ULONG bufLen = sizeof(info);
		if (GetAdaptersInfo(info, &bufLen) != ERROR_SUCCESS)
		{
			TCHAR ErrorMessage[512];
			FWindowsPlatformMisc::GetSystemErrorMessage(ErrorMessage, 512, WSAGetLastError());
			XIAO_LOG(Error, TEXT("GetAdaptersInfo failed (%s)"), ErrorMessage);
			return IPAddresses;
		}
		for (IP_ADAPTER_INFO* it = info; it; it = it->Next)
		{
			if (it->Type != MIB_IF_TYPE_ETHERNET && it->Type != IF_TYPE_IEEE80211)
			{
				continue;
			}
			for (IP_ADDR_STRING* s = &it->IpAddressList; s; s = s->Next)
			{
				FString Ip;
				Ip.Appendf(TEXT("%hs"), s->IpAddress.String);
				if (Ip.Equals(L"0.0.0.0"))
					continue;
				IPAddresses.Add(Ip);
			}
		}
#else
		struct ifaddrs* ifaddr;
		if (getifaddrs(&ifaddr) == -1)
		{
			XIAO_LOG(Error, TEXT("getifaddrs failed"));
			return IPAddresses;
		}

		ON_SCOPE_EXIT
		{
			freeifaddrs(ifaddr);
		};

		for (struct ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next)
		{
			if (ifa->ifa_addr == nullptr)
				continue;

			int family = ifa->ifa_addr->sa_family;
			if (family != AF_INET)
				continue;

			char hbuf[NI_MAXHOST];
			int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST);
			if (s != 0)
				continue;
			const FString Ip(UTF8_TO_TCHAR(hbuf));
			if (Ip.StartsWith("169.254") || Ip.Equals("127.0.0.1"))
				continue;
			IPAddresses.Add(Ip);
		}
#endif
		return IPAddresses;
	}

	static FString GetLANV4()
	{
		// A类地址 10.0.0.0 - 10.255.255.255
		// B类地址 172.16.0.0 - 172.31.255.255
		// C类地址 192.168.0.0 - 192.168.255.255
		FString IpV4 = TEXT("127.0.0.1");
		for (const FString& IP : GetAllLAN4())
		{
			if (IP.StartsWith(TEXT("192.")))
			{
				return IP;
			}
			else if (IP.StartsWith(TEXT("10.")) || IP.StartsWith(TEXT("172.")))
			{
				IpV4 = IP;
			}
		}

		return IpV4;
	}

	enum ENetworkPerformance : uint8
	{
		Performance_Excellent,
		Performance_Good,
		Performance_Normal,
		Performance_Bad,
	};

	static ENetworkPerformance GetPerformance(const float InSpeed)
	{
		if(InSpeed < 10.0f)
		{
			return Performance_Bad;
		}
		else if(InSpeed >= 10.0f && InSpeed < 30.0f)
		{
			return Performance_Normal;
		}
		else if(InSpeed >= 30.0f && InSpeed < 50.0f)
		{
			return Performance_Good;
		}
		else if(InSpeed > 50.0f)
		{
			return Performance_Excellent;
		}
		return Performance_Bad;
	}


	enum ETestConnectivity
	{
		Test_Unknown,
		Test_OK,
		Test_Progress,
		Test_NG,
		Test_Busy,
		Test_AgentUnReachable
	};

	// 连通性测试
	struct FNetworkConnectivity
	{
		static constexpr float SActive = 300.0f;
		bool bConnect = true;
		bool bWorking = false;
		FString Name = TEXT("Agent");
		ENetworkPerformance Status = ENetworkPerformance::Performance_Good;
		ETestConnectivity Connectivety = ETestConnectivity::Test_Unknown;
		uint64 LastUpdate = 0;
		uint64 LastActive = 0;
		uint32 NotActiveCount = 0;
		float LastActiveTestTime = 0.0f;
		float Performance = 0;
		float ReceivePerfor = 0.0f;			// MB/Sec
		float SendPerfor = 0.0f;			// MB/Sec
		uint16 RoundTripTime = 60;			// ms
		uint16 Trip1, Trip2, Trip3, Trip4;	// ms
		FString IPAddress = TEXT("localhost");
		uint16 Port = 37080;
		uint16 AgentPort = 1345;
		FString RemoteConnection;
	
		FString ErrorMsg;

		FProcHandle ProcHandle;
	
		void MarkError(const ETestConnectivity InError = ETestConnectivity::Test_NG, const FString& InErrorText=TEXT(""))
		{
			Connectivety = InError;
			ErrorMsg = InErrorText;
			bConnect = false;
			if (!InErrorText.IsEmpty())
			{
				XIAO_LOG(Warning, TEXT("Exceptions::%s"), *InErrorText);
			}
		}
	
		void OnNetworkTest()
		{
			bWorking = true;
			ErrorMsg = TEXT("");
			Connectivety = ETestConnectivity::Test_Progress;

			AsyncThread([this]() 
			{
				FString IPerfExePath = FPaths::Combine(FPaths::GetPath(FPlatformProcess::ExecutablePath()),
#if PLATFORM_WINDOWS
				TEXT("iperf3.exe")
#else
				TEXT("iperf3")
#endif
				);

				FPaths::MakeStandardFilename(IPerfExePath);
				const FString Params = FString::Printf(TEXT("-c %s -p %d"), *RemoteConnection, Port);
				uint32 ProcessId;
				void* WritePipeId = nullptr;
				void* ReadPipeId = nullptr;
				if (!FPlatformProcess::CreatePipe(ReadPipeId, WritePipeId))
				{
					XIAO_LOG(Warning, TEXT("OnNetworkTest::Createpipe::failed"));
					bWorking = false;
					return;
				}

				ON_SCOPE_EXIT
				{
					if (WritePipeId || ReadPipeId)
					{
						FPlatformProcess::ClosePipe(ReadPipeId, WritePipeId);
						XIAO_LOG(Verbose, TEXT("OnNetworkTest::ClosePipe!"));
					}
					bWorking = false;
				};
				ProcHandle = FPlatformProcess::CreateProc(*IPerfExePath, *Params, true, true, true, &ProcessId, 0, *FPaths::GetPath(IPerfExePath), WritePipeId, ReadPipeId);
				if (!ProcHandle.IsValid())
				{
					XIAO_LOG(Warning, TEXT("OnNetworkTest::CreateProc::failed"));
					return;
				}
			
				FPlatformProcess::WaitForProc(ProcHandle);
				if (ReadPipeId)
				{
					const FString Output = FPlatformProcess::ReadPipe(ReadPipeId);
					FString LeftStr;
					if(Output.Split(TEXT("iperf3: error - "), &LeftStr, &ErrorMsg))
					{
						static const FString BusyNetwork = TEXT("the server is busy running a test. try again later");
						if (!ErrorMsg.Contains(BusyNetwork))
						{
							MarkError(ETestConnectivity::Test_NG, FString::Printf(TEXT("IPerf::Params::%s->%s"), *Params, *ErrorMsg));
						}
						else
						{
							Connectivety = ETestConnectivity::Test_Busy;
							bConnect = true;
							XIAO_LOG(Warning, TEXT("IPerf::Params::%s->%s"), *Params, *ErrorMsg);
						}
					}
					else
					{
						FString Right;
						if (Output.Split(TEXT("- - - - - - - - - - - - - - - - - - - - - - - - -"), &LeftStr, &Right))
						{
							TArray<FString> Sections;
							Right.ParseIntoArray(Sections, TEXT("\n"));
							TArray<FString> SenderSection;
							Sections[1].ParseIntoArray(SenderSection, TEXT(" "));
							SendPerfor = FCString::Atof(*SenderSection[6]);
							TArray<FString> ReceiverSection;
							Sections[2].ParseIntoArray(ReceiverSection, TEXT(" "));
							ReceivePerfor = FCString::Atof(*ReceiverSection[6]);
							Performance = (SendPerfor + ReceivePerfor) / 2.0f;
							XIAO_LOG(Verbose, TEXT("IPerf::%s"), *Output);
							Connectivety = ETestConnectivity::Test_OK;
						}
					}
				}
			});
		}
	};
}

#if PLATFORM_WINDOWS
	#ifdef SetPort
	#pragma pop_macro("SetPort")
	#endif
#endif