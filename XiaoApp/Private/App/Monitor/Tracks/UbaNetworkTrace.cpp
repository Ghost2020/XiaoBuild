#include "UbaNetworkTrace.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "XiaoLog.h"
#include "Runtime/Launch/Resources/Version.h"

namespace Xiao
{
	static constexpr uint8_t SServerId = 2;
	static constexpr uint32 SVersionNumber = 1339;
	// 2 byte id, 3 bytes size
	static constexpr uint32 SHeaderSize = 5;
	static constexpr uint32 SValidSize = 17;
	static constexpr uint32 SReceiveMaxSize = 128 * 1024 * 1024;
	static constexpr uint8 SSessionMessageType_GetTraceInformation =
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
		18
#else
		18
#endif
		;
	static constexpr uint8 SSessionMessageType_Exit = 63;

	FNetworkTrace::FNetworkTrace()
	{
		ClientGuid = FGuid::NewGuid();
		ServerGuid.Invalidate();
	}

	FNetworkTrace::~FNetworkTrace()
	{
		Disconnect();
		if (SocketSubsystem)
		{
			if (Socket)
			{
				SocketSubsystem->DestroySocket(Socket);
			}
		}
	}

	bool FNetworkTrace::Init()
	{
		if (!SocketSubsystem)
		{
			SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
			if (!SocketSubsystem)
			{
				XIAO_LOG(Error, TEXT("Failed to get SocketSubsystem!"));
				return false;
			}
		}

		if (!Socket)
		{
			Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("UbaTcpClient"));
			if (!Socket)
			{
				XIAO_LOG(Error, TEXT("Failed to CreateSocket!"));
				return false;
			}
		}

		XIAO_LOG(Display, TEXT("UbaClient Inited."));
		return true;
	}

	bool FNetworkTrace::Connect(const FString& InIp, const int32 InPort)
	{
		TSharedPtr<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
		if (!Addr.IsValid())
		{
			XIAO_LOG(Error, TEXT("Failed to CreateInternetAddr!"));
			return false;
		}

		bool bIsValid = false;
		Addr->SetIp(*InIp, bIsValid);
		Addr->SetPort(InPort);
		if (!Socket->Connect(*Addr))
		{
			XIAO_LOG(Error, TEXT("Failed to Connect \"%s:%u\"!"), *InIp, InPort);
			return false;
		}

		if (!Validation())
		{
			XIAO_LOG(Error, TEXT("Validation::failed!"));
			return false;
		}

		bIsConnected = true;
		return true;
	}

	void FNetworkTrace::Disconnect()
	{
		bIsConnected = false;
		if (Socket)
		{
			Socket->Close();
			if (Socket)
			{
				SocketSubsystem->DestroySocket(Socket);
			}
			Socket = nullptr;
		}
	}

	bool FNetworkTrace::SendTrace(const uint32& InPos, TArray<uint8>& OutResponse)
	{
		TArray<uint8> Buffer;
		Buffer.SetNum(10); //HeaderLength(6) + TracePos(4)
		uint8* Data = Buffer.GetData();
		// ServiceID
		Data[0] = (Xiao::SServerId << 6) | SSessionMessageType_GetTraceInformation;
		// MessageID
		Data[1] = ++MessageId >> 8;
		// DataLength
		const uint32 Length = (4 | MessageId << 24);
		FMemory::Memcpy(Data + 2, &Length, 4);
		// Pos
		FMemory::Memcpy(Data + 6, &InPos, 4);
		int32 BytesSent = 0;
		if (!Socket->Send(Buffer.GetData(), Buffer.Num(), BytesSent))
		{
			static int Index = 0;
			if (++Index > 8)
			{
				bSendFailed = true;
			}
			XIAO_LOG(Error, TEXT("SendTrace failed!"));
			return false;
		}

		if (!Socket->Wait(ESocketWaitConditions::Type::WaitForRead, FTimespan::FromSeconds(5.0f)))
		{
			XIAO_LOG(Error, TEXT("Socket wait failed!"));
			return false;
		}

		TArray<uint8> ReceiveBuffer;
		ReceiveBuffer.SetNumZeroed(SReceiveMaxSize);
		int32 BytesRead = 0;
		if (!Socket->Recv(ReceiveBuffer.GetData(), SReceiveMaxSize, BytesRead, ESocketReceiveFlags::Type::None))
		{
			bReceiveFailed = true;	
			XIAO_LOG(Error, TEXT("ReceiveTrace failed!"));
			return false;
		}
		const uint32 DataLength = BytesRead - SHeaderSize;
		OutResponse.SetNum(DataLength);
		FMemory::Memcpy(OutResponse.GetData(), ReceiveBuffer.GetData() + SHeaderSize, DataLength);
		return true;
	}

	bool FNetworkTrace::IsConnected() const
	{
		return bIsConnected && !bSendFailed && !bReceiveFailed;
	}

	bool FNetworkTrace::Validation()
	{
		for (int TryCount = 1; TryCount <= 3; ++TryCount)
		{
			XIAO_LOG(Log, TEXT("Try validation count::%d!"), TryCount);

			int32 ByteSend = 0;
			TArray<uint8> SendBuffer;
			SendBuffer.SetNum(20);
			FMemory::Memcpy(SendBuffer.GetData(), (void*)&SVersionNumber, 4);
			FMemory::Memcpy(SendBuffer.GetData() + 4, (void*)&ClientGuid, sizeof(FGuid));
			if (!Socket->Send(SendBuffer.GetData(), 20, ByteSend))
			{
				XIAO_LOG(Error, TEXT("Failed to send client guid::%s!"), *ClientGuid.ToString());
				continue;
			}

			TArray<uint8> Buffer;
			Buffer.SetNumZeroed(Xiao::SValidSize);
			for (int TryRecv = 0; TryRecv < 3; ++TryRecv)
			{
				int32 BytesRead = 0;
				Socket->Wait(ESocketWaitConditions::Type::WaitForRead, FTimespan::FromSeconds(1.0f));
				if (Socket->Recv(Buffer.GetData(), Xiao::SValidSize, BytesRead, ESocketReceiveFlags::Type::WaitAll))
				{
					if (BytesRead < Xiao::SValidSize)
					{
						XIAO_LOG(Error, TEXT("Receive Buffer not equal %d!"), BytesRead);
						continue;
					}
					if (BytesRead > Xiao::SValidSize)
					{
						if (Buffer[0] != uint8(0))
						{
							XIAO_LOG(Error, TEXT("Receive Response Message Size::%d!"), BytesRead);
							XIAO_LOG(Error, TEXT("Receive Initial Response::%d!"), Buffer[0]);
							continue;
						}
					}
					XIAO_LOG(Display, TEXT("Receive accept response."));

					FMemory::Memcpy(&ServerGuid, Buffer.GetData() + 1, sizeof(FGuid));
					int32 NewSize = 0;
					if (!Socket->SetReceiveBufferSize(Xiao::SReceiveMaxSize, NewSize))
					{
						XIAO_LOG(Warning, TEXT("Failed to SetReceiveBufferSize NewSize::%d!"), NewSize);
					}
					return true;
				}
			}
		}

		return false;
	}

	void FNetworkTrace::RequestStopInitiator(const FString& InIP, const int32 InPort)
	{
		Disconnect();
		if (Init())
		{
			bool bConnected = false;
			for (int32 TestPort = InPort; TestPort < InPort+3; ++TestPort)
			{
				if (Connect(InIP, TestPort))
				{
					bConnected = true;
					break;
				}
			}

			if (!bConnected)
			{
				XIAO_LOG(Warning, TEXT("Try 3 times,always failed!"));
				return;
			}

			TArray<uint8> Buffer;
			Buffer.SetNum(10); 
			uint8* Data = Buffer.GetData();
			// ServiceID
			Data[0] = (Xiao::SServerId << 6) | SSessionMessageType_Exit;
			// MessageID
			Data[1] = ++MessageId >> 8;
			// DataLength
			constexpr uint32 DataLength = (4 + 0 | 0 << 24);
			FMemory::Memcpy(Data + 2, &DataLength, 4);
			int32 BytesSent = 0;
			if (!Socket->Send(Buffer.GetData(), Buffer.Num(), BytesSent))
			{
				XIAO_LOG(Error, TEXT("Request host %s:%d stop initiator failed!"), *InIP, InPort);
			}
		}
	}
}