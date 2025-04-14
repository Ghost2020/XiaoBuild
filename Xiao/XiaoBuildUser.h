/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#include "XiaoShare.h"
#include "Serialization/JsonSerializerMacros.h"

using namespace XiaoUserParam;

struct FUserDesc : FJsonSerializable
{
	explicit FUserDesc(const uint16 InId, const FString& InUsername, const FString& InNickname,
		const EUserRole InRole, const FDateTime& InLastLogin, const EUserStatus InStatus,
		const FString& InLoginMachine=TEXT(""))
		: ID(InId)
		, Username(InUsername)
		, Nickname(InNickname)
		, Role(InRole)
		, LastLogin(InLastLogin)
		, Status(InStatus)
		, LoginMachine(InLoginMachine)
	{}

	explicit FUserDesc(){}
	
	uint16 ID = 0;
	FString Username = TEXT("user");
	FString Password = TEXT("A123456");
	FString Email = TEXT(""); //可选
	FString Nickname = TEXT("Niyan");
	int Role = EUserRole::Role_Viewer;
	FDateTime LastLogin = FDateTime::MinValue(); // 精确到秒
	int Status = EUserStatus::Inactive;
	FString LoginMachine = TEXT(""); // 上次登录的机器
	bool bModifyPassword = false;
	bool bModifyLast = false;

	BEGIN_JSON_SERIALIZER
		JSON_SERIALIZE("username", Username);
		JSON_SERIALIZE("password", Password);
		JSON_SERIALIZE("role", Role);
		JSON_SERIALIZE("status", Status);
		JSON_SERIALIZE("nickname", Nickname);
		JSON_SERIALIZE("machine_id", LoginMachine);
		JSON_SERIALIZE("last_active", LastLogin);
		JSON_SERIALIZE("modify_password", bModifyPassword);
		JSON_SERIALIZE("modify_last", bModifyLast);
	END_JSON_SERIALIZER
};

