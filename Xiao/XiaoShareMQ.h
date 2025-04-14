/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -12:18 AM
 */
#pragma once

#include <iostream>
#include <set>
#include "SimpleAmqpClient.h"

namespace XiaoMessageQueue
{
	namespace Queue
	{
		static const std::string SBuildSystemStamp("xiao-build-system-timestamp");
		static const std::string SBuildSystemStats("xiao-build-system-stats");
		static const std::string SBuildSystemProgress("xiao-build-system-progress");
		static const std::string SBuildAgentStats("xiao-build-agent-stats");
		static const std::string SBuildCoordiUI("xiao-build-coordi-ui");
	}
	namespace Exchange
	{
		static const std::string SBuildSystem("dxx_build");
	}
	namespace RoutingKey
	{
		static const std::string SBuildSystem("build.changes.timestamp");
		static const std::string SBuildMonitor("build.changes.stats");
		static const std::string SBuildProgress("build.changes.progress");
		static const std::string SAgentStats("build.agent.stats");
		static const std::string SCoordiUI("build.coordi.ui");
	}
	namespace Consumer
	{
		static const std::string SBuildSystem(Queue::SBuildSystemStamp);
		static const std::string SBuildMonitor(Queue::SBuildSystemStats);
		static const std::string SBuildTray(Queue::SBuildSystemProgress);
		static const std::string SCoordiService(Queue::SBuildAgentStats);
		static const std::string SCoordiUI(Queue::SBuildCoordiUI);
	}
	namespace AppId
	{
		static const std::string SBuildMonitor("build-monitor");
		static const std::string SBuildTray("build-tray");
	}
	namespace ContentType
	{
		static const std::string STextPlain("text/plain");
		static const std::string STextJson("text/json");
	}
	namespace DeltaTime
	{
		static const float SDeltaTime = 0.25f;
	}

	class MQ
	{
	public:
		static Channel::ptr_t OpenMQ(const int InPort = 5672, const std::string& InHost= "localhost")
		{
			try
			{
				Channel::OpenOpts Opts;
				Opts.host = InHost;
				Opts.vhost = "/";
				Opts.port = InPort;
				Opts.auth = Channel::OpenOpts::BasicAuth("xiaobuild", "Ghost");
				return Channel::Open(Opts);
			}
			catch (AmqpException& AmqpEx)
			{
				if (AmqpEx.is_soft_error())
				{
					std::cout << "Soft Error::" << AmqpEx.what() << " ErrorCode::" << AmqpEx.reply_code() << std::endl;
				}
				else
				{
					std::cout << "Hard Error::" << AmqpEx.what() << " ErrorCode::" << AmqpEx.reply_code() << std::endl;
				}
				return nullptr;
			}

			return nullptr;
		}

		static bool Declare(Channel::ptr_t Channel, const std::string& InQueueName, const std::string& InExchangeName, const std::string& InRoutingKey, const Table& Intable={})
		{
			if (Channel)
			{
				try
				{
					if (!Channel->CheckExchangeExists(InExchangeName))
					{
						Channel->DeclareExchange(InExchangeName/*, Channel::EXCHANGE_TYPE_DIRECT, false, true, false*/);
						ExchangeSet.insert(InExchangeName);
					}
					if (!Channel->CheckQueueExists(InQueueName))
					{
						QueueSet.insert(Channel->DeclareQueue(InQueueName, false, true, false, false, Intable));
					}
					Channel->BindQueue(InQueueName, InExchangeName, InRoutingKey);
					return true;
				}
				catch (AmqpException& AmqpEx)
				{
					if (AmqpEx.is_soft_error())
					{
						std::cout << "Soft Error::" << AmqpEx.what() << " ErrorCode::" << AmqpEx.reply_code() << std::endl;
					}
					else
					{
						std::cout << "Hard Error::" << AmqpEx.what() << " ErrorCode::" << AmqpEx.reply_code() << std::endl;
					}
					return false;
				}
			}
			return false;
		}

		static void SetMessagePropertys(BasicMessage* Message, 
			const std::string& InContentType, const std::string& InType,
			const std::string& InExpiration, const uint8_t InPriority,
			const std::string& InAppId, const std::string& InContentEncoding="gzip",
			const BasicMessage::delivery_mode_t InMode = BasicMessage::delivery_mode_t::dm_persistent,
			const std::string& InReplayTo="localhost", const std::string InUserId="xiaobuild")
		{
			if (Message)
			{
				Message->ContentType(InContentType);
				Message->Type(InType);
				Message->Expiration(InExpiration);
				Message->Priority(InPriority);
				Message->AppId(InAppId);
				Message->ContentEncoding(InContentEncoding);
				Message->DeliveryMode(InMode);
				Message->ReplyTo(InReplayTo);
				Message->UserId(InUserId);
			}
		}

		static void CloseMQ(Channel::ptr_t Channel)
		{
			if (Channel)
			{
				try
				{
					for(const auto& ExchangeName : ExchangeSet)
					{
						if (Channel->CheckExchangeExists(ExchangeName))
						{
							Channel->DeleteExchange(ExchangeName);
						}
					}
					ExchangeSet.clear();

					for(const auto& QueneName : QueueSet)
					{
						if (Channel->CheckQueueExists(QueneName))
						{
							Channel->DeleteQueue(QueneName);
						}
					}

					Channel->CloseChannel();
				}
				catch (AmqpException& AmqpEx)
				{
					if (AmqpEx.is_soft_error())
					{
						std::cerr << "Soft Error::" << AmqpEx.what() << " ErrorCode::" << AmqpEx.reply_code() << std::endl;
					}
					else
					{
						std::cerr << "Hard Error::" << AmqpEx.what() << " ErrorCode::" << AmqpEx.reply_code() << std::endl;
					}
				}
				catch (AmqpLibraryException& AmqpLibEx)
				{
					std::cerr << "AmqpLibraryException::ErrorCode::" << AmqpLibEx.ErrorCode() << " ErrorMessage::" << AmqpLibEx.what() << std::endl;
				}
			}
		}

	private:
		static std::string SQueueName;
		static std::set<std::string> ExchangeSet;
		static std::set<std::string> QueueSet;
	};

	std::string MQ::SQueueName;
	std::set<std::string> MQ::ExchangeSet;
	std::set<std::string> MQ::QueueSet;
};