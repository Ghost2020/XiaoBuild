#include "XiaoRedis.h"
#include "HAL/UnrealMemory.h"
#include <iostream>

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
		
		Options.connect_timeout = nullptr;
		Options.command_timeout = nullptr;

		Context = redisConnectWithOptions(&Options);
		if (Context == nullptr || Context->err)
		{
			throw ClosedError("redisConnectWithOptions failed");
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
		if (!HandleReply(reply))
		{
			return "Failed to execute command: PING";
		}

		if (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "PONG") == 0) 
		{
			return "PONG";
		}
		else 
		{
			throw ReplyError("Unexpected reply type or content");
			return "";
		}
		freeReplyObject(reply);
	}

	int64_t Redis::exists(const std::string& InKey)
	{
		int64_t Num = -1;
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "EXISTS %s", InKey.c_str()));
		if (!HandleReply(reply))
		{
			return Num;
		}

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Num = reply->integer;
		}
		else 
		{
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
		return Num;
	}

	bool Redis::set(const std::string& InKey, const std::string& InVal, const std::chrono::milliseconds& ttl)
	{
		bool bRtn = false;
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "SET %s %s", InKey.c_str(), InVal.c_str()));
		if (!HandleReply(reply))
		{
			return bRtn;
		}

		if (reply->type == REDIS_REPLY_STATUS) 
		{
			if (strcmp(reply->str, "OK") == 0) 
			{
				bRtn = true;
			}
			else 
			{
				std::cerr << "Unexpected reply: " << reply->str << std::endl;
			}
		}
		else 
		{
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
		return bRtn;
	}

	std::optional<std::string> Redis::get(const std::string& InKey)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "GET %s", InKey.c_str()));
		if (!HandleReply(reply))
		{
			return std::optional<std::string>();
		}
		
		switch (reply->type) 
		{
		case REDIS_REPLY_STRING:
		{
			std::string TempStr;
			TempStr.resize(reply->len);
			FMemory::Memcpy(&TempStr[0], reply->str, reply->len);
			const auto Val = std::optional<std::string>(TempStr);
			freeReplyObject(reply);
			return Val;
		}
		case REDIS_REPLY_NIL:
			std::cout << "Key: " << InKey << " does not exist." << std::endl;
			break;
		default:
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
			break;
		}

		freeReplyObject(reply);
		return std::optional<std::string>();
	}

	int64_t Redis::llen(const std::string& InKey)
	{
		int64_t Len = -1;
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "LLEN %s", InKey.c_str()));
		if (!HandleReply(reply))
		{
			return Len;
		}

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Len = reply->integer;
		}
		else 
		{
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
		return Len;
	}

	int64_t Redis::lpush(const std::string& InKey, const std::string& InVal)
	{
		int64_t Len = -1;
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "LPUSH %s %s", InKey.c_str(), InVal.c_str()));
		if (!HandleReply(reply))
		{
			return Len;
		}

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Len = reply->integer;
		}
		else 
		{
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
		return Len;
	}

	void Redis::lrange(const std::string& InKey, const int64_t InStart, const int64_t InStop, std::insert_iterator<std::vector<std::string>> Output)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "LRANGE %s %ld %ld", InKey.c_str(), InStart, InStop));
		if (!HandleReply(reply))
		{
			return;
		}

		if (reply->type == REDIS_REPLY_ARRAY)
		{
			for (unsigned int i = 0; i < reply->elements; ++i) 
			{
				std::string element;
				element.resize(reply->element[i]->len);
				FMemory::Memcpy(&element[0], reply->element[i]->str, reply->element[i]->len);
				*Output++ = element;
			}
		}
		else 
		{
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
	}

	void Redis::ltrim(const std::string& InKey, const int64_t InStart, const int64_t InStop)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "LTRIM %s %ld %ld", InKey.c_str(), InStart, InStop));
		if (!HandleReply(reply))
		{
			return;
		}

		if (reply->type == REDIS_REPLY_STATUS) 
		{
			if (strcmp(reply->str, "OK") != 0) 
			{
				std::cerr << "Unexpected reply: " << reply->str << std::endl;
			}
		}
		else 
		{
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
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
		if (!HandleReply(reply))
		{
			return Num;
		}

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Num = reply->integer;
		}
		else 
		{
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		// 清理资源
		freeReplyObject(reply);
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
		if (!HandleReply(reply))
		{
			return added;
		}

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			added = reply->integer;
		}
		else 
		{
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
		return added;
	}

	bool Redis::hexists(const std::string& InKey, const std::string& InField)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HEXISTS %s %s", InKey.c_str(), InField.c_str()));
		if (!HandleReply(reply))
		{
			return false;
		}

		bool exists = false;
		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			exists = (reply->integer == 1);
		}
		else 
		{
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
		return exists;
	}

	int64_t Redis::hlen(const std::string& InKey)
	{
		int64_t Len = -1;
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HLEN %s", InKey.c_str()));
		if (!HandleReply(reply))
		{
			return Len;
		}

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Len = reply->integer;
		}
		else 
		{
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
		return Len;
	}

	int64_t Redis::hset(const std::string& InKey, const std::string& InField, const std::string& InVal)
	{
		int64_t Val = -1;
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HSET %s %s %s", InKey.c_str(), InField.c_str(), InVal.c_str()));
		if (!HandleReply(reply))
		{
			return Val;
		}

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Val = reply->integer;
		}
		else 
		{
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
		return Val;
	}

	int64_t Redis::hset(const std::string& InKey, const std::pair<std::string, std::string>& InItem)
	{
		int64_t Val = -1;
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HSET %s %s %s", InKey.c_str(), InItem.first.c_str(), InItem.second.c_str()));
		if (!HandleReply(reply))
		{
			return Val;
		}

		if (reply->type == REDIS_REPLY_INTEGER)
		{
			Val = reply->integer;
		}
		else
		{
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
		return Val;
	}

	std::optional<std::string> Redis::hget(const std::string& InKey, const std::string& InField)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HGET %s %s", InKey.c_str(), InField.c_str()));
		if (!HandleReply(reply))
		{
			return std::optional<std::string>();
		}

		switch (reply->type) 
		{
		case REDIS_REPLY_STRING:
		{
			std::string field;
			field.resize(reply->len);
			FMemory::Memcpy(&field[0], reply->str, reply->len);
			const auto Val = std::optional<std::string>(field);
			freeReplyObject(reply);
			return Val;
		}
		case REDIS_REPLY_NIL:
			std::cout << "Field: " << InField << " does not exist in hash: " << InKey << std::endl;
			break;
		default:
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
			break;
		}

		freeReplyObject(reply);
		return std::optional<std::string>();
	}

	int64_t Redis::hdel(const std::string& InKey, const std::string& InField)
	{
		int64_t Val = -1;
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HDEL %s %s", InKey.c_str(), InField.c_str()));
		if (!HandleReply(reply))
		{
			return Val;
		}

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Val = reply->integer;
		}
		else 
		{
			std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
		return Val;
	}

	void Redis::hgetall(const std::string& InKey, std::insert_iterator<std::unordered_map<std::string, std::string>> OutMap)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HGETALL %s", InKey.c_str()));
		if (!HandleReply(reply))
		{
			return;
		}

		if (reply->type == REDIS_REPLY_ARRAY) 
		{
			if (reply->elements % 2 != 0) 
			{
				std::cerr << "Unexpected number of elements in reply" << std::endl;
				freeReplyObject(reply);
				return;
			}
			for (unsigned int i = 0; i < reply->elements; i += 2) 
			{
				std::string field;
				field.resize(reply->element[i]->len);
				FMemory::Memcpy(&field[0], reply->element[i]->str, reply->element[i]->len);

				std::string value;
				value.resize(reply->element[i + 1]->len);
				FMemory::Memcpy(&value[0], reply->element[i + 1]->str, reply->element[i + 1]->len);
				*OutMap++ = std::make_pair(field, value);
			}
		}
		else 
		{
			std::cerr << "Unexpected reply type or empty hash: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
	}

	void Redis::hkeys(const std::string& InKey, std::insert_iterator<std::vector<std::string>> Output)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "HKEYS %s", InKey.c_str()));
		if (!HandleReply(reply))
		{
			return;
		}

		if (reply->type == REDIS_REPLY_ARRAY) 
		{
			if (reply->elements > 0) 
			{
				for (unsigned int i = 0; i < reply->elements; ++i) 
				{
					std::string field;
					field.resize(reply->element[i]->len);
					FMemory::Memcpy(&field[0], reply->element[i]->str, reply->element[i]->len);
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
			std::cerr << "Unexpected reply type or empty hash: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
	}

	int64_t Redis::publish(const std::string& InKey, const std::string& InVal)
	{
		int64_t Result = -1;
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "PUBLISH %s %s", InKey.c_str(), InVal.c_str()));
		if (!HandleReply(reply))
		{
			return Result;
		}

		if (reply->type == REDIS_REPLY_INTEGER) 
		{
			Result = reply->integer;
		}
		else 
		{
			throw ReplyError("Unexpected reply type");
			// std::cerr << "Unexpected reply type: " << reply->type << std::endl;
		}

		freeReplyObject(reply);
		return Result;
	}

	redisReply* Redis::command(const std::string& InCommand, const std::string& InArg)
	{
		return static_cast<redisReply*>(redisCommand(Context, "%s %s", InCommand.c_str(), InArg.c_str()));
	}

	void Redis::command(const std::string& InCommand, const std::string& InArg1, const std::string& InArg2)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(Context, "%s %s", InCommand.c_str(), InArg1.c_str(), InArg2.c_str()));
		(void)HandleReply(reply);
		freeReplyObject(reply);
	}

	bool Redis::HandleReply(redisReply* InReply)
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