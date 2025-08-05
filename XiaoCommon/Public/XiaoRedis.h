/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <memory>
#include <iterator>
#include <exception>
#include <unordered_map>
#include "hiredis/hiredis.h"

namespace Xiao
{
	class Error : public std::exception 
	{
	public:
		explicit Error(const std::string& msg) : _msg(msg) {}

		Error(const Error&) = default;
		Error& operator=(const Error&) = default;

		Error(Error&&) = default;
		Error& operator=(Error&&) = default;

		virtual ~Error() override = default;

		virtual const char* what() const noexcept override 
		{
			return _msg.data();
		}

	private:
		std::string _msg;
	};

	class IoError : public Error 
	{
	public:
		explicit IoError(const std::string& msg) : Error(msg) {}

		IoError(const IoError&) = default;
		IoError& operator=(const IoError&) = default;

		IoError(IoError&&) = default;
		IoError& operator=(IoError&&) = default;

		virtual ~IoError() override = default;
	};

	class TimeoutError : public IoError 
	{
	public:
		explicit TimeoutError(const std::string& msg) : IoError(msg) {}

		TimeoutError(const TimeoutError&) = default;
		TimeoutError& operator=(const TimeoutError&) = default;

		TimeoutError(TimeoutError&&) = default;
		TimeoutError& operator=(TimeoutError&&) = default;

		virtual ~TimeoutError() override = default;
	};

	class ClosedError : public Error 
	{
	public:
		explicit ClosedError(const std::string& msg) : Error(msg) {}

		ClosedError(const ClosedError&) = default;
		ClosedError& operator=(const ClosedError&) = default;

		ClosedError(ClosedError&&) = default;
		ClosedError& operator=(ClosedError&&) = default;

		virtual ~ClosedError() override = default;
	};

	class ProtoError : public Error 
	{
	public:
		explicit ProtoError(const std::string& msg) : Error(msg) {}

		ProtoError(const ProtoError&) = default;
		ProtoError& operator=(const ProtoError&) = default;

		ProtoError(ProtoError&&) = default;
		ProtoError& operator=(ProtoError&&) = default;

		virtual ~ProtoError() override = default;
	};

	class OomError : public Error 
	{
	public:
		explicit OomError(const std::string& msg) : Error(msg) {}

		OomError(const OomError&) = default;
		OomError& operator=(const OomError&) = default;

		OomError(OomError&&) = default;
		OomError& operator=(OomError&&) = default;

		virtual ~OomError() override = default;
	};

	class ReplyError : public Error 
	{
	public:
		explicit ReplyError(const std::string& msg) : Error(msg) {}

		ReplyError(const ReplyError&) = default;
		ReplyError& operator=(const ReplyError&) = default;

		ReplyError(ReplyError&&) = default;
		ReplyError& operator=(ReplyError&&) = default;

		virtual ~ReplyError() override = default;
	};

	class WatchError : public Error 
	{
	public:
		explicit WatchError() : Error("Watched key has been modified") {}

		WatchError(const WatchError&) = default;
		WatchError& operator=(const WatchError&) = default;

		WatchError(WatchError&&) = default;
		WatchError& operator=(WatchError&&) = default;

		virtual ~WatchError() override = default;
	};

	enum class ConnectionType 
	{
		TCP = 0,
		UNIX
	};

	struct ConnectionOptions 
	{
		ConnectionOptions() = default;

		ConnectionOptions(const ConnectionOptions& InOther)
		{
			host = InOther.host;
			port = InOther.port;
			path = InOther.path;
			user = InOther.user;
			password = InOther.password;
			db = InOther.db;
			keep_alive = InOther.keep_alive;
			connect_timeout = InOther.connect_timeout;
			socket_timeout = InOther.socket_timeout;
			readonly = InOther.readonly;
		}

		ConnectionOptions& operator=(const ConnectionOptions&) = default;

		ConnectionOptions(ConnectionOptions&&) = default;
		ConnectionOptions& operator=(ConnectionOptions&&) = default;

		~ConnectionOptions() = default;

		ConnectionType type = ConnectionType::TCP;

		std::string host;

		int port = 6379;

		std::string path;

		std::string user = "default";

		std::string password;

		int db = 0;

		bool keep_alive = false;

		std::chrono::milliseconds connect_timeout{ 0 };

		std::chrono::milliseconds socket_timeout{ 0 };

		// tls::TlsOptions tls;

		bool readonly = false;
	};

	class Redis
	{
	public:
		Redis(const ConnectionOptions& InOptions);
		~Redis();

		std::string ping();

		int64_t exists(const std::string& InKey);

		bool set(const std::string& InKey, const std::string& InVal);
		std::optional<std::string> get(const std::string& InKey);

		int64_t llen(const std::string& InKey);
		int64_t lpush(const std::string& InKey, const std::string& InVal);
		void lrange(const std::string& InKey, const int64_t InStart, const int64_t InStop, std::insert_iterator<std::vector<std::string>> Output);
		void ltrim(const std::string& InKey, const int64_t InStart, const int64_t InStop);

		int64_t rpush(const std::string& InKey, std::vector<std::string>::iterator InFirst, std::vector<std::string>::iterator InLast);

		int64_t sadd(const std::string& InKey, const std::initializer_list<std::string>& InList);

		bool hexists(const std::string& InKey, const std::string& InField);
		int64_t hlen(const std::string& InKey);
		int64_t hset(const std::string& InKey, const std::string& InField, const std::string& InVal);
		int64_t hset(const std::string& InKey, const std::pair<std::string, std::string>& InItem);
		std::optional<std::string> hget(const std::string& InKey, const std::string& InField);
		int64_t hdel(const std::string& InKey, const std::string& InField);
		void hgetall(const std::string& InKey, std::insert_iterator<std::unordered_map<std::string, std::string>> OutMap);
		void hkeys(const std::string& InKey, std::insert_iterator<std::vector<std::string>> Output);

		int64_t publish(const std::string& InKey, const std::string& InVal);

		redisReply* command(const std::vector<std::string>& InArgs, const bool bDiscardReply = false);

	protected:
		bool CheckReply(redisReply* InReply);

	private:
		redisOptions Options;
		redisContext* Context;
	};
}