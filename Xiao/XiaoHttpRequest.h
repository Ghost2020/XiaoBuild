/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#define LOCTEXT_NAMESPACE "XiaoHttp"

namespace XiaoHttp
{
	static FString SGetVerb(TEXT("GET"));
	static FString SDeleteVerb(TEXT("DELETE"));
	static FString SPutVerb(TEXT("PUT"));
	static FString SPatchVerb(TEXT("PATCH"));
	static FString SPostVerb(TEXT("POST"));
	static FString SOptionsVerb(TEXT("OPTIONS"));

	static FString SEncryptKey(TEXT("encrypt-length"));

	static FText SCantConnect = LOCTEXT("ConnectFailed_Text", "无法连接");
	static FText SUpdateSuccess = LOCTEXT("UpdateSuccess_Text", "更新完成");
	static FText SUpdateFailed = LOCTEXT("UpdateFailed_Text", "更新失败");

	void DoHttpRequest(
		const FString& InVerb,
		const FString& InUrl,
		const FHttpRequestCompleteDelegate& InCompleteDelegate,
		const FString& InContent = TEXT(""),
		bool InEncyprt = false,
		const float InTimeout = 1.0f)
	{
		const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		if (InEncyprt)
		{
			HttpRequest->SetHeader(SEncryptKey, FString::FromInt(InContent.Len()));
		}
		HttpRequest->SetVerb(InVerb);
		HttpRequest->SetURL(InUrl);
		if (InContent.Len() > 0)
		{
			HttpRequest->SetContentAsString(InContent);
		}
		HttpRequest->SetTimeout(InTimeout);
		HttpRequest->OnProcessRequestComplete() = InCompleteDelegate;
		HttpRequest->ProcessRequest();
	}
}

#undef LOCTEXT_NAMESPACE