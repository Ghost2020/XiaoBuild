/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -12:18 AM
 */
#pragma once

#include "CoreMinimal.h"
#include "HttpPath.h"
#include "HttpServerRequest.h"
#include "SQLiteDatabaseConnection.h"
#include "XiaoShareField.h"

class FSQLiteDatabaseConnection;

#define FREE_RESULT_SET(ResultSet) \
	if(ResultSet) \
	{ \
		delete ResultSet; \
		ResultSet = nullptr; \
	}\
	

DECLARE_DELEGATE_RetVal_TwoParams(bool, FRequestHandlerDelegate, const FHttpServerRequest&, const FHttpResultCallback&);


class FBaseRouter
{
public:
	FBaseRouter(FString InDescription, FHttpPath InPath, EHttpServerRequestVerbs InVerbs, FRequestHandlerDelegate InHandler)
		: Description(MoveTemp(InDescription))
		, Path(MoveTemp(InPath))
		, Verb(InVerbs)
		, Handler(MoveTemp(InHandler))
	{}

	friend uint32 GetTypeHash(const FBaseRouter& Route) { return HashCombine(GetTypeHash(Route.Path), GetTypeHash(Route.Verb)); }
	friend bool operator==(const FBaseRouter& Lhs, const FBaseRouter& Rhs) { return Lhs.Path == Rhs.Path && Lhs.Verb == Rhs.Verb; }

	FORCEINLINE static TUniquePtr<FHttpServerResponse> CreateHttpResponse(const EHttpServerResponseCodes InResponseCode = EHttpServerResponseCodes::BadRequest, const FString& InBody = TEXT(""))
	{
		TUniquePtr<FHttpServerResponse> Response = MakeUnique<FHttpServerResponse>();
		Response->Code = InResponseCode;
		Response->HttpVersion = HttpVersion::EHttpServerHttpVersion::HTTP_VERSION_1_1;
		Response->Body = StringToBytes(InBody);
		return Response;
	}

	FORCEINLINE static FString GetBoolString(const bool InFlag)
	{
		return InFlag ? TEXT("true") : TEXT("false");
	
	}

	FORCEINLINE static FString EmbraceByQuotes(const FString& InString)
	{
		return TEXT("\"") + InString + TEXT("\"");
	}

	FORCEINLINE static bool IntToBool(const int32& InInt)
	{
		return InInt == 1;
	}

	FORCEINLINE static bool GetRealRouterPath(const FString& InRouterPath, FString& RelativePath)
	{
		if (InRouterPath.StartsWith("/"))
		{
			RelativePath = InRouterPath;
			return true;
		}

		FHttpPath Path(InRouterPath);

		static TArray<FString> May = {
			FString::Printf(TEXT("http://127.0.0.1:%d"), SServerPort),
			FString::Printf(TEXT("http://localhost:%d"), SServerPort),
			FString::Printf(TEXT("http://%s:%d"), *SIpv4Address, SServerPort)
		};

		RelativePath = InRouterPath;
		for (const auto& Begin : May)
		{
			Path.MakeRelative(Begin);
			if (Path.GetPath() != RelativePath)
			{
				RelativePath = Path.GetPath();
				return true;
			}
		}

		return false;
	}

	FString Description;
	FHttpPath Path;
	EHttpServerRequestVerbs Verb;
	FRequestHandlerDelegate Handler;

	static TWeakPtr<FSQLiteDatabaseConnection> InDataBaseConnection;
	static FString SIpv4Address;
	// 服务器监控端口起始
	static uint16 SServerPort;
	static const FHttpPath SRootPath;
};

TWeakPtr<FSQLiteDatabaseConnection> FBaseRouter::InDataBaseConnection = nullptr;
FString FBaseRouter::SIpv4Address;
uint16 FBaseRouter::SServerPort = 37000;
const FHttpPath FBaseRouter::SRootPath{TEXT("/")};