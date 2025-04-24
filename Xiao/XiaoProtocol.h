/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Protobuf/xiao_msg.pb.h"
#include "XiaoLog.h"

namespace XiaoProtocol
{
	const FIPv4Endpoint SInvalidEndpoint(FIPv4Address::LanBroadcast, 0);

	// 默认失活时间
	constexpr float SDefaultInactiveTimeoutSeconds = 10.0f; // TODO 后面需要改成 3s
	constexpr float SHelperInactiveTimeoutSesconds = 5.0f;
	constexpr double SBackThreadSleepTime = 1.0f;
	constexpr float SWaitForPendingTimeoutSeconds = 0.05f;

	// 远程任务失活时间
	constexpr float SRemoteTaskInactiveTimeout = 300.0f;

	// TCP 数据缓冲区默认大小
	constexpr int32 SDefaultBufferSize = 64 * 1024;
	inline int32 SSendBufferSize = SDefaultBufferSize;
	inline int32 SReceiveBufferSize = SDefaultBufferSize;

	// 特别的Protobuf
	inline FXiaoMsg STryConnectMsg;
	inline FXiaoMsg SDisconnectMsg;
	inline FXiaoMsg SKeepaliveMsg;

	class FXiaoTaskBase
	{
	public:
		FXiaoTaskBase(const FXiaoMsg& InMsg, const FIPv4Endpoint& InRecipient)
			: Msg(InMsg)
			, Recipient(InRecipient)
		{}
		
		virtual ~FXiaoTaskBase()
		{}

		/** Calculates a hash that should be the same for equivalent Tasks, even if their TaskID is different */
		virtual bool RunTask() { return false; }
		virtual uint32 GetEquivalenceHash() const
		{
			return HashCombine(GetTypeHash(Msg.type()), GetTypeHash(Recipient));
		}
	
		FXiaoMsg Msg;
		FIPv4Endpoint Recipient;
	};
	
	// 数据解析器，用于解析检测来自Tcp原始的数据中存在的数据包
	struct FRawBufferParser
	{
		// 开始标识符
		inline static const TArray<uint8> SStartFlag = { '\r', '\n', 'X', 'I', 'A', 'O', 'M', 'S', 'G', 'B', 'E', 'G', 'I', 'N', '\r', '\n'};
		static const int32 SStartFlagLen = 16;
		// 完整数据包的长度
		static constexpr int32 SBufferFlagLen = sizeof(int32);
		// 结束标识符
		inline static const TArray<uint8> SEndFlag = { '\r', '\n', 'X', 'I', 'A', 'O', 'M', 'S', 'G', 'E', 'N', 'D', '\r', '\n'};
		static const int32 SEndFlagLen = 14;

		// 解析其中存在的数据包， 一段完整的数据应该 是  [1.开始标识符 + 2.数据包长度 + 3.数据包 + 4.结束标识符]
		// 解析一次数据包，可能会解析出多个完整数据包，但最后一个数据包可能是不完整的，需要等待下一次数据到来，然后拼接成一个完整的数据包
		void Parse(const TArray<uint8>& InRawData)
		{
			// 每次解析都会清空一下数据，默认已经拿走数据了
			CompleteBuffers.Empty();

			const int32 DataLen = InRawData.Num();
			for (int Index = 0; Index < DataLen; Index++)
			{
				const uint8 Byte = InRawData[Index];

				// 开始标志检测
				if (ParseState == PS_StartFlag)
				{
					if (Byte == SStartFlag[BeginFlagCheckPos])
					{
						BeginFlagCheckPos++;
						if (BeginFlagCheckPos == SStartFlagLen)
						{
							// 检查标志重置
							XIAO_LOG(VeryVerbose, TEXT("<===========================[%d/%d]-[Start]-===========================>"), Index, DataLen);
							BeginFlagCheckPos = 0;
							ParseState = PS_BufferSize;
						}
						continue;
					}
					// 如果不匹配，需要重新检测
					BeginFlagCheckPos = 0;
					continue;
				}

				// 数据包大小检测
				if (ParseState == PS_BufferSize)
				{
					BufferSizeBuffer.Add(Byte);
					if (BufferSizeBuffer.Num() == SBufferFlagLen)
					{
						FMemory::Memcpy(&BufferSize, BufferSizeBuffer.GetData(), SBufferFlagLen);
						if (BufferSize >= INT32_MAX)
						{
							XIAO_LOG(Error, TEXT("xxxxxxxxxxxxxxxxxxxxxData package size is too large, please check the data integrityxxxxxxxxxxxxxxxxxxxxx"));
							ParseState = PS_StartFlag;
							ProtoBuffer.Empty();
							continue;
						}
						BufferSizeBuffer.Empty();
						ParseState = PS_Buffer;
						XIAO_LOG(VeryVerbose, TEXT("===========================[%d/%d]-[Size]:[%d]==========================="), BufferSize, Index, DataLen);
					}
					continue;
				}

				// 数据包填充阶段
				if (ParseState == PS_Buffer)
				{
					// 计算还需要添加多少字节才能满足BufferSize
					const int32 BytesNeeded = BufferSize - ProtoBuffer.Num();
					// 计算InRawData中剩余的字节
					const int32 BytesRemaining = DataLen - Index;
					// 计算实际可以添加的字节
					const int32 BytesToAdd = FMath::Min(BytesNeeded, BytesRemaining);
					// 如果有足够的字节可以添加
					if (BytesToAdd > 0)
					{
						// 使用内存拷贝的方式直接复制数据
						ProtoBuffer.Append(&InRawData[Index], BytesToAdd);
						// 更新索引
						Index += BytesToAdd - 1;
					}

					// ProtoBuffer.Add(Byte);

					// 是否已经满足一个完整的数据包
					if (ProtoBuffer.Num() == BufferSize)
					{
						XIAO_LOG(VeryVerbose, TEXT("===========================[%d/%d]-[Protobuf]-==========================="), Index, DataLen);
						CompleteBuffers.Add(ProtoBuffer);
						ProtoBuffer.Empty();
						BufferSize = 0;
						ParseState = PS_EndFlag;
					}
					continue;
				}

				// 结束标志检测
				if (ParseState == PS_EndFlag)
				{
					if (Byte == SEndFlag[EndFlagCheckPos])
					{
						EndFlagCheckPos++;
						if (EndFlagCheckPos == SEndFlagLen)
						{
							// 检查标志重置
							EndFlagCheckPos = 0;
							ParseState = PS_StartFlag;
							XIAO_LOG(VeryVerbose, TEXT("<===========================[%d/%d]-[End]-==========================>"), Index, DataLen);
						}
					}
					else
					{
						// 如果不匹配，需要重新检测
						XIAO_LOG(Error, TEXT("<xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx[%d/%d]-[UnEnd]:[%d]-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx>"), Index, DataLen, EndFlagCheckPos);
						EndFlagCheckPos = 0;
						ParseState = PS_StartFlag;
					}
				}
			}
		}

		// 获取完之后还需要清理一下，相当于拿走了这些数据，避免数据一直累积
		const TArray<TArray<uint8>>& GetBuffers() const
		{
			return CompleteBuffers;
		}

	private:
		// 解析的阶段
		enum EParseState
		{
			PS_StartFlag,	// 开始标识符检测
			PS_BufferSize,	// 数据包大小检测
			PS_Buffer,		// 数据包检测
			PS_EndFlag		// 结束标识符检测
		};

		// 当前解析的阶段
		EParseState ParseState = PS_StartFlag;
		// 开始标识符检测的位置
		int32 BeginFlagCheckPos = 0;
		// 结束标识符检测的位置
		int32 EndFlagCheckPos = 0;
		// 正在组装的Buffer的大小
		int32 BufferSize = 0;
		// Buffer大小的Buffer
		TArray<uint8> BufferSizeBuffer;
		// 正在解析拼接的Buffer缓存
		TArray<uint8> ProtoBuffer;
		// 完整的Buffer数据
		TArray<TArray<uint8>> CompleteBuffers;
	};


	struct FXiaoMessage
	{
		FXiaoMsg Msg;
		FIPv4Endpoint Endpoint;

		FXiaoMessage(FXiaoMsg&& InMsg, const FIPv4Endpoint& InEndpoint)
			: Msg(MoveTemp(InMsg))
			, Endpoint(InEndpoint)
		{}

		FXiaoMessage(const FXiaoMsg& InProtobuf, const FIPv4Endpoint& InEndpoint)
			: Msg(InProtobuf)
			, Endpoint(InEndpoint)
		{}
	};


	static bool TryFindIdInBrokenProtobuf(const TArray<uint8>& InData, std::string& OutGuid)
	{
		FXiaoMsg Msg;
		if (Msg.ParsePartialFromArray(InData.GetData(), InData.Num()))
		{
			OutGuid = Msg.id();
			return true;
		}

		return false;
	}
	
	static bool CreateProtobuf(const FXiaoMsg& InMsg, TArray<uint8>& OutProtobuf)
	{
		const int32 BufferSize = InMsg.ByteSizeLong();
		int32 Offset = 0;
		OutProtobuf.SetNum(FRawBufferParser::SStartFlagLen + FRawBufferParser::SBufferFlagLen + BufferSize + FRawBufferParser::SEndFlagLen);
		// 填充开始标识符
		FMemory::Memcpy(OutProtobuf.GetData() + Offset, FRawBufferParser::SStartFlag.GetData(), FRawBufferParser::SStartFlagLen);
		Offset += FRawBufferParser::SStartFlagLen;
		// 填充Buffer大小
		FMemory::Memcpy(OutProtobuf.GetData() + Offset, &BufferSize, FRawBufferParser::SBufferFlagLen);
		Offset += FRawBufferParser::SBufferFlagLen;
		// 填充Buffer
		if (InMsg.SerializeToArray(OutProtobuf.GetData() + Offset, BufferSize))
		{
			Offset += BufferSize;
			FMemory::Memcpy(OutProtobuf.GetData() + Offset, FRawBufferParser::SEndFlag.GetData(), FRawBufferParser::SEndFlagLen);
			return true;
		}

		XIAO_LOG(Error, TEXT("CreateProtobuf failed"));
		return false;
	}

	static TArray<uint8> CreateAcceptedProtobuf(const std::string& InMessageID)
	{
		FXiaoMsg Msg;
		Msg.set_type(EXiaoMsgType::Xmt_Accepted);
		*Msg.mutable_id() = InMessageID.c_str();
		TArray<uint8> Protobuf;
		CreateProtobuf(Msg, Protobuf);
		return Protobuf;
	}
	
	static TArray<uint8> CreateDeclinedProtobuf(const std::string& InMessageID, const FString& InErrorMessage)
	{
		FXiaoMsg Msg;
		Msg.set_type(EXiaoMsgType::Xmt_Declined);
		*Msg.mutable_id() = InMessageID.c_str();
		*Msg.mutable_msg() = TCHAR_TO_UTF8(*InErrorMessage);
		TArray<uint8> Protobuf;
		CreateProtobuf(Msg, Protobuf);
		return Protobuf;
	}

	static void InitBasicProtobuf()
	{
		SKeepaliveMsg.set_type(EXiaoMsgType::Xmt_KeepAlive);
		STryConnectMsg.set_type(EXiaoMsgType::Xmt_TryConnect);
		SDisconnectMsg.set_type(EXiaoMsgType::Xmt_Disconnect);
	}

	static bool ParseMessageFromBuffer(const TArray<uint8>& InData, FXiaoMsg& OutMsg)
	{
		if (!OutMsg.ParseFromArray(InData.GetData(), InData.Num()))
		{
			XIAO_LOG(Error, TEXT("ParseFromArray Failed!"));
			if (!OutMsg.ParsePartialFromArray(InData.GetData(), InData.Num()))
			{
				static FString ParseError = FString::Printf(TEXT("Could not parse message"));
				XIAO_LOG(Error, TEXT("ParsePartialFromArray Failed!"));
				return false;
			}
		}

		return true;
	}
}