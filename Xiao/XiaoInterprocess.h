/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -12:18 AM
 */
#pragma once

#include <string>

#ifdef XIAO_USE_BOOST
THIRD_PARTY_INCLUDES_START

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformAtomics.h"

#ifndef BOOST_USE_WINDOWS_H
#define BOOST_USE_WINDOWS_H
#endif

#pragma push_macro("check")
#undef check
#pragma push_macro("verify")
#undef verify

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif


#include <boost/asio.hpp>

#if PLATFORM_WINDOWS
#ifdef FALSE
#undef FALSE
#endif

#ifdef TRUE
#undef TRUE
#endif
#include <boost/detail/interlocked.hpp>
#endif

#if PLATFORM_MAC
#include <sys/shm.h>
#endif

#if PLATFORM_UNIX
#include <sys/mman.h>
#endif

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_sharable_mutex.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/exceptions.hpp>


#if PLATFORM_WINDOWS
#pragma pop_macro("check")
#pragma pop_macro("verify")

#include "Windows/HideWindowsPlatformAtomics.h"
#endif

THIRD_PARTY_INCLUDES_END
#endif


namespace XiaoIPC
{
#ifdef XIAO_USE_BOOST
	inline boost::interprocess::permissions Permissions;

	// 命令队列
	static const std::string SInputQueueName("Xiao-Input-Queue");
	static const std::string SOutputQueueName("Xiao-Ouput-Queue");
	static const uint32 SMaxQueueSize = 526;
	static const uint32 SCommandQueueBufferSize = 8192;

	// 错误日志队列
	static const std::string SNgLogQueueName("Xiao-Ng-Queue");
	static const uint32 SNgQueueSize = 64;
	static const uint32 SNgQueueBufferSize = 8192;

	// 任务同步锁
	static const std::string SCommandMutextName("Xiao-Command-Mutext");
	
	// 用于构建监视器显示进度
	static const std::string SMonitorStatsMemoryName("Xiao-Monitor-Stats");
	static const uint32 SMonitorStatsMemorySize = 1024 * 1024 * 100;
	// 用于同步信息
	static const std::string SMonitorStatsSyncMemoryName("Xiao-Monitor-Stats-Sync");
	static const uint32 SMonitorStatsSyncMemorySize = 1024;
	// 用于Tray更新进度
	static const std::string SMonitorProgressMemoryName("Xiao-tray-progress");
	static const uint32 SMonitorProgressMemorySize = 64;
	// AgentSettings
	static const std::string SAgentSettingsMemeoryName("Xiao-Agent-settings");
	static const uint32 SAgentSettingsMemorySize = 1024 * 1024;

	// 用于安装进度同步
	static const std::string SInstallProgressMemoryName("Xiao-Install-progress");
	static const uint32 SInstallProgressMemorySize = 1024 * 1024;


	// 时间
	static const float STickTime = 0.05f;
	static const float SDeltaTime = 5.0f;
#endif

}