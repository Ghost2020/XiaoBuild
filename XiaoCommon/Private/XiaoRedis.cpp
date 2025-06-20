#include "XiaoRedis.h"
#include "Misc/ScopeExit.h"
#include <iostream>
#include <format>
#if PLATFORM_WINDOWS
#include "WinSock2.h"
#endif

namespace Xiao
{
	Redis::Redis(const ConnectionOptions& InOptions)
		: Context(nullptr)
	{
		Options = { 0 };
		if (InOptions.type == Xiao::ConnectionType::TCP)
		{
			REDIS_OPTIONS_SET_TCP(&Options, InOptions.host.c_str(), InOptions.port);
		}
		else
		{
			REDIS_OPTIONS_SET_UNIX(&Options, "/tmp/redis.sock");
		}

		Options.options |= REDIS_OPT_PREFER_IPV4;
		
		static timeval connect_tv;
		if (InOptions.connect_timeout.count() > 0)
		{
			connect_tv.tv_sec = static_cast<long>(std::chrono::duration_cast<std::chrono::seconds>(InOptions.connect_timeout).count());
			Options.connect_timeout = &connect_tv;
		}

		static timeval command_tv;
		if (InOptions.socket_timeout.count() > 0)
		{
			command_tv.tv_sec = static_cast<long>(std::chrono::duration_cast<std::chrono::seconds>(InOptions.socket_timeout).count());
			Options.command_timeout = &command_tv;
		}

		Context = redisConnectWithOptions(&Options);
		if (Context == nullptr || Context->err)
		{
			throw ClosedError(std::format("redisConnectWithOptions failed with {}", Context->errstr));
			return;
		}

		if (InOptions.keep_alive)
		{
			if (redisEnableKeepAlive(Context) != REDIS_OK)
			{
				throw ClosedError(std::format("Failed to enable KeepAlive: {}", Context->errstr));
				return;
			}
		}

		if (!InOptions.user.empty() && !InOptions.password.empty())
		{
			redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "AUTH %s %s", InOptions.user.c_str(), InOptions.password.c_str()));

			if (!CheckReply(reply))
			{
				throw ClosedError("AUTH failed");
				return;
			}

			ON_SCOPE_EXIT{ freeReplyObject(reply); };

			if (reply->type == REDIS_REPLY_ERROR) 
			{
				redisFree(Context);
				throw ClosedError(std::format("AUTH failed：{}", reply->str));
				return;
			}
		}
	}

	Redis::~Redis()
	{
		if (Context)
		{
			redisFree(Context);
		}
	}

	std::string Redis::ping()
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "PING"));
		if (!CheckReply(reply))
		{
			return "Failed to execute command: PING";
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "PONG") == 0) 
		{
			return "PONG";
		}
		else 
		{
			throw ReplyError("Unexpected reply type or content");
		}
		return "";
	}

	int64_t Redis::exists(const std::string& InKey)
	{
		int64_t Num = -1;
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "EXISTS %s", InKey.c_str()));
		if (!CheckReply(reply))
		{
			return Num;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Num = reply->integer;
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}

		return Num;
	}

	bool Redis::set(const std::string& InKey, const std::string& InVal)
	{
		bool bRtn = false;
		const char* argv[] = { "SET", InKey.c_str(), InVal.c_str() };
		size_t argvlen[] = { 3, InKey.size(), InVal.size() };
		redisReply* reply = static_cast<redisReply*>(redisCommandArgv(Context, 3, argv, argvlen));
		if (!CheckReply(reply))
		{
			return bRtn;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_STATUS) 
		{
			if (strcmp(reply->str, "OK") == 0) 
			{
				bRtn = true;
			}
			else 
			{
				throw ReplyError(std::format("Unexpected reply: {}", reply->str));
			}
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}

		return bRtn;
	}

	std::optional<std::string> Redis::get(const std::string& InKey)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "GET %s", InKey.c_str()));
		if (!CheckReply(reply))
		{
			return std::optional<std::string>();
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };
		
		switch (reply->type) 
		{
		case REDIS_REPLY_STRING:
		{
			std::string TempStr;
			TempStr.resize(reply->len);
			std::memcpy(&TempStr[0], reply->str, reply->len);
			return std::optional<std::string>(TempStr);
		}
		case REDIS_REPLY_NIL:
			std::cerr << "Key: " << InKey << " does not exist." << std::endl;
			break;
		default:
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}

		return std::optional<std::string>();
	}

	int64_t Redis::llen(const std::string& InKey)
	{
		int64_t Len = -1;
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "LLEN %s", InKey.c_str()));
		if (!CheckReply(reply))
		{
			return Len;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Len = reply->integer;
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}

		return Len;
	}

	int64_t Redis::lpush(const std::string& InKey, const std::string& InVal)
	{
		int64_t Len = -1;
		const char* argv[] = { "LPUSH", InKey.c_str(), InVal.c_str() };
		size_t argvlen[] = { 5, InKey.size(), InVal.size() };
		redisReply* reply = static_cast<redisReply*>(redisCommandArgv(Context, 3, argv, argvlen));
		if (!CheckReply(reply))
		{
			return Len;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Len = reply->integer;
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}

		return Len;
	}

	void Redis::lrange(const std::string& InKey, const int64_t InStart, const int64_t InStop, std::insert_iterator<std::vector<std::string>> Output)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "LRANGE %s %ld %ld", InKey.c_str(), InStart, InStop));
		if (!CheckReply(reply))
		{
			return;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_ARRAY)
		{
			for (unsigned int i = 0; i < reply->elements; ++i) 
			{
				std::string element;
				element.resize(reply->element[i]->len);
				std::memcpy(&element[0], reply->element[i]->str, reply->element[i]->len);
				*Output++ = element;
			}
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}
	}

	void Redis::ltrim(const std::string& InKey, const int64_t InStart, const int64_t InStop)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "LTRIM %s %ld %ld", InKey.c_str(), InStart, InStop));
		if (!CheckReply(reply))
		{
			return;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_STATUS) 
		{
			if (strcmp(reply->str, "OK") != 0) 
			{
				std::cerr << "Unexpected reply: " << reply->str << std::endl;
			}
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}
	}

	int64_t Redis::rpush(const std::string& InKey, std::vector<std::string>::iterator InFirst, std::vector<std::string>::iterator InLast)
	{
		int64_t Num = -1;
		std::string Command = "RPUSH " + InKey;
		auto Iter = InFirst;
		while(Iter != InLast)
		{
			Command += " " + *Iter;
			++Iter;
		}
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, Command.c_str()));
		if (!CheckReply(reply))
		{
			return Num;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Num = reply->integer;
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}

		return Num;
	}

	int64_t Redis::sadd(const std::string& InKey, const std::initializer_list<std::string>& InList)
	{
		int64_t added = -1;
		std::string Command = "SADD " + InKey;
		for (const auto& value : InList)
		{
			Command += " " + value;
		}
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, Command.c_str()));
		if (!CheckReply(reply))
		{
			return added;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			added = reply->integer;
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}

		return added;
	}

	bool Redis::hexists(const std::string& InKey, const std::string& InField)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HEXISTS %s %s", InKey.c_str(), InField.c_str()));
		if (!CheckReply(reply))
		{
			return false;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		bool exists = false;
		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			exists = (reply->integer == 1);
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}

		return exists;
	}

	int64_t Redis::hlen(const std::string& InKey)
	{
		int64_t Len = -1;
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HLEN %s", InKey.c_str()));
		if (!CheckReply(reply))
		{
			return Len;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Len = reply->integer;
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
			return Len;
		}

		return Len;
	}

	int64_t Redis::hset(const std::string& InKey, const std::string& InField, const std::string& InVal)
	{
		int64_t Val = -1;
		const char* argv[] = { "HSET", InKey.c_str(), InField.c_str(), InVal.c_str() };
		size_t argvlen[] = { 4, InKey.size(), InField.size(), InVal.size() };
		redisReply* reply = static_cast<redisReply*>(redisCommandArgv(Context, 4, argv, argvlen));
		if (!CheckReply(reply))
		{
			return Val;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Val = reply->integer;
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}

		return Val;
	}

	int64_t Redis::hset(const std::string& InKey, const std::pair<std::string, std::string>& InItem)
	{
		int64_t Val = -1;
		const char* argv[] = { "HSET", InKey.c_str(), InItem.first.c_str(), InItem.second.c_str() };
		size_t argvlen[] = { 4, InKey.size(), InItem.first.size(), InItem.second.size() };
		redisReply* reply = static_cast<redisReply*>(redisCommandArgv(Context, 4, argv, argvlen));
		if (!CheckReply(reply))
		{
			return Val;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_INTEGER)
		{
			Val = reply->integer;
		}
		else
		{
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}

		return Val;
	}

	std::optional<std::string> Redis::hget(const std::string& InKey, const std::string& InField)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HGET %s %s", InKey.c_str(), InField.c_str()));
		if (!CheckReply(reply))
		{
			return std::optional<std::string>();
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		switch (reply->type) 
		{
		case REDIS_REPLY_STRING:
		{
			std::string field;
			field.resize(reply->len);
			std::memcpy(&field[0], (void*)reply->str, reply->len);
			return std::optional<std::string>(field);
		}
		case REDIS_REPLY_NIL:
			std::cout << "Field: " << InField << " does not exist in hash: " << InKey << std::endl;
			break;
		default:
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}

		return std::optional<std::string>();
	}

	int64_t Redis::hdel(const std::string& InKey, const std::string& InField)
	{
		int64_t Val = -1;
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HDEL %s %s", InKey.c_str(), InField.c_str()));
		if (!CheckReply(reply))
		{
			return Val;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Val = reply->integer;
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type: {}", reply->type));
		}

		return Val;
	}

	void Redis::hgetall(const std::string& InKey, std::insert_iterator<std::unordered_map<std::string, std::string>> OutMap)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HGETALL %s", InKey.c_str()));
		if (!CheckReply(reply))
		{
			return;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_ARRAY) 
		{
			if (reply->elements % 2 != 0) 
			{
				throw ReplyError("Unexpected number of elements in reply");	
				return;
			}
			for (unsigned int i = 0; i < reply->elements; i += 2) 
			{
				std::string field;
				field.resize(reply->element[i]->len);
				std::memcpy(&field[0], reply->element[i]->str, reply->element[i]->len);

				std::string value;
				value.resize(reply->element[i + 1]->len);
				std::memcpy(&value[0], reply->element[i + 1]->str, reply->element[i + 1]->len);
				*OutMap++ = std::make_pair(field, value);
			}
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type or empty hash: {}", reply->type));
		}
	}

	void Redis::hkeys(const std::string& InKey, std::insert_iterator<std::vector<std::string>> Output)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HKEYS %s", InKey.c_str()));
		if (!CheckReply(reply))
		{
			return;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_ARRAY) 
		{
			if (reply->elements > 0) 
			{
				for (unsigned int i = 0; i < reply->elements; ++i) 
				{
					std::string field;
					field.resize(reply->element[i]->len);
					std::memcpy(&field[0], reply->element[i]->str, reply->element[i]->len);
					*Output++ = field;
				}
			}
			else 
			{
				std::cout << "No fields found in hash: " << InKey << std::endl;
			}
		}
		else 
		{
			throw ReplyError(std::format("Unexpected reply type or empty hash: {}", reply->type));
		}
	}

	int64_t Redis::publish(const std::string& InKey, const std::string& InVal)
	{
		int64_t Result = -1;
		const char* argv[] = { "PUBLISH", InKey.c_str(), InVal.c_str() };
		size_t argvlen[] = { 7, InKey.size(), InVal.size() };
		redisReply* reply = static_cast<redisReply*>(redisCommandArgv(Context, 3, argv, argvlen));
		if (!CheckReply(reply))
		{
			return Result;
		}
		ON_SCOPE_EXIT{ freeReplyObject(reply); };

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Result = reply->integer;
		}
		else 
		{
			throw ReplyError("Unexpected reply type");
		}

		return Result;
	}

	redisReply* Redis::command(const std::string& InCommand, const std::string& InArg)
	{
		const char* argv[] = { InCommand.c_str(), InArg.c_str() };
		size_t argvlen[] = { InCommand.size(), InArg.size()};
		return static_cast<redisReply*>(redisCommandArgv(Context, 2, argv, argvlen));
	}

	void Redis::command(const std::string& InCommand, const std::string& InArg1, const std::string& InArg2)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "%s %s", InCommand.c_str(), InArg1.c_str(), InArg2.c_str()));
		(void)CheckReply(reply);
		freeReplyObject(reply);
	}

	bool Redis::CheckReply(redisReply* InReply)
	{
		if (InReply == nullptr)
		{
			throw ClosedError("Unknown Error");
			return false;
		}

		if (InReply->type == REDIS_REPLY_ERROR || (Context && Context->err != 0))
		{
			if (Context)
			{
				const auto ErrNo = Context->err;
				const std::string ErrStr(Context->errstr);
				freeReplyObject(InReply);
				if (ErrNo == REDIS_ERR_IO)
				{
					throw IoError(ErrStr);
				}
				else if (ErrNo == REDIS_ERR_EOF)
				{
					throw ClosedError(ErrStr);
				}
				else if (ErrNo == REDIS_ERR_PROTOCOL)
				{
					throw ProtoError(ErrStr);
				}
				else if (ErrNo == REDIS_ERR_OOM)
				{
					throw OomError(ErrStr);
				}
				else if (ErrNo == REDIS_ERR_TIMEOUT)
				{
					throw TimeoutError(ErrStr);
				}
				else if (ErrNo == REDIS_ERR_OTHER)
				{
					throw Error(ErrStr);
				}
			}
			
			return false;
		}

		return true;
	}
}