#pragma once

#include "CoreMinimal.h"

class ISocketSubsystem;
class FSocket;

namespace Xiao
{
	class FNetworkTrace
	{
	public:
		FNetworkTrace();
		virtual ~FNetworkTrace();

		bool Init();
		bool Connect(const FString& InIp, const int32 InPort);
		void Disconnect();
		bool SendTrace(const uint32& InPos, TArray<uint8>& OutResponse);

		bool IsConnected() const;

		void RequestStopInitiator(const FString& InIP, const int32 InPort);

	private:
		bool Validation();

	private:
		bool bIsConnected = false;
		bool bSendFailed = false;
		bool bReceiveFailed = false;

		FGuid ClientGuid;
		FGuid ServerGuid;

		uint32 MessageId = 0;

		ISocketSubsystem* SocketSubsystem = nullptr;
		FSocket* Socket = nullptr;
	};
}