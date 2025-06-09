#pragma once

#include "UbaCommon.h"
#include "Misc/StringBuilder.h"
#include "Runtime/Launch/Resources/Version.h"
#include "oodle2.h"
#include <string>

namespace Xiao
{
	#define UBA_ASSERT_WRITE(size) ensureMsgf(m_pos + size <= m_end, TEXT("BinaryWriter overflow. Written: %llu, Capacity: %llu, Trying to write: %llu"), uint64(m_pos - m_begin), uint64(m_end - m_begin), uint64(size))
	#define UBA_ASSERT_READ(size) ensureMsgf(m_pos + size <= m_end, TEXT("BinaryReader overflow. Read: %llu, Size: %llu, Trying to read: %llu"), uint64(m_pos - m_begin), uint64(m_end - m_begin), uint64(size))
	
	#if PLATFORM_WINDOWS
		constexpr bool IsWindows = true;
	#endif

	uint8* g_messageMappingMem = nullptr;

	constexpr uint32 CommunicationMemSize = 
#if PLATFORM_WINDOWS
		64 * 1024 
#else
		64 * 1024 * 2
#endif
		;

	struct FBinaryReader
	{
	public:
		FBinaryReader()
		{
			m_begin = g_messageMappingMem;
			m_pos = m_begin;
			m_end = m_begin + CommunicationMemSize - /*sizeof(Event)*/128 * 3;
		}

		FORCEINLINE FBinaryReader(const uint8* data, const uint64 offset = 0, const uint64 size = InvalidValue)
		{ 
			m_begin = data; 
			m_pos = data + offset; 
			m_end = data + size; 
		}

		FORCEINLINE void ReadBytes(void* data, const uint64 size)
		{
			UBA_ASSERT_WRITE(size);
			FPlatformMemory::Memcpy(data, m_pos, size);
			m_pos += size;
		}
		FORCEINLINE uint8 ReadByte()
		{
			UBA_ASSERT_READ(sizeof(uint8));
			return *m_pos++;
		}
		FORCEINLINE uint16 ReadU16()
		{
			UBA_ASSERT_READ(sizeof(uint16));
			const uint16 value = *(uint16*)m_pos;
			m_pos += sizeof(uint16);
			return value;
		}
		FORCEINLINE uint32 ReadU32()
		{
			UBA_ASSERT_READ(sizeof(uint32));
			const uint32 value = *(uint32*)m_pos;
			m_pos += sizeof(uint32);
			return value;
		}
		FORCEINLINE uint64 ReadU64()
		{
			UBA_ASSERT_READ(sizeof(uint64));
			const uint64 value = *(uint64*)m_pos;
			m_pos += sizeof(uint64);
			return value;
		}

		template<typename CharType>
		FORCEINLINE bool TryReadString(TStringBuilderBase<CharType>& out)
		{
			uint64 charLen;
			if (!TryRead7BitEncoded(charLen))
				return false;	
			TCHAR* it = out.GetData() + out.Len();
			uint64 left = charLen;
			while (left--)
			{
				if (m_pos >= m_end)
					return false;
				uint8 a = *m_pos++;
				if (a <= 127)
				{
					*it++ = a;
					continue;
				}
				if (m_pos >= m_end)
					return false;
				uint8 b = *m_pos++;
				if (a >= 192 && a <= 223)
				{
					*it++ = (a - 192) * 64 + (b - 128);
					continue;
				}
				if (m_pos >= m_end)
					return false;
				uint8 c = *m_pos++;
				if (a >= 224 && a <= 239)
				{
					*it++ = (a - 224) * 4096 + (b - 128) * 64 + (c - 128);
					continue;
				}
				return false;
			}
			*it = 0;
			const auto NewCapacity = uint32(it - out.GetData());
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
			out.Reserve(NewCapacity);
#else
			const auto AdditinalCapacity = NewCapacity - out.Len();
			if (AdditinalCapacity > 0)
			{
				out.AddUninitialized(AdditinalCapacity);
			}
#endif
			return true;
		}
		FORCEINLINE uint64 ReadString(TCHAR* str, const uint64 strCapacity)
		{
			const uint64 charLen = Read7BitEncoded();
			checkf(charLen < strCapacity - 1, TEXT("Strlen: %llu, Capacity: %llu"), charLen, strCapacity); (void)strCapacity;
			FString String;
			return InternalReadString(str, charLen, String);
		}
		template<typename CharType> 
		FORCEINLINE void ReadString(TStringBuilderBase<CharType>& out)
		{
			ReadString(out.GetData() + out.Len(), (out.GetAllocatedSize() - out.Len()));
		}
		FORCEINLINE FString ReadString()
		{
			const uint64 len = Read7BitEncoded();
			FString Temp, String;
			Temp.Reserve(len);
			InternalReadString(GetData(Temp), len, String);
			return String;
		}
		FORCEINLINE FString ReadLongString()
		{
			uint8 s = ReadByte();
			if (!s)
				return ReadString();

			const uint64 stringLength = Read7BitEncoded();
			const uint64 uncompressedSize = stringLength * s;
			const uint64 compressedSize = Read7BitEncoded();

			const uint8* data = GetPositionData();
			Skip(compressedSize);

			if (s == sizeof(TCHAR))
			{
				FString str;
				str.Reserve(stringLength);
				OO_SINTa decompLen = OodleLZ_Decompress(data, (OO_SINTa)compressedSize, str.GetCharArray().GetData(), (OO_SINTa)uncompressedSize);
				check(decompLen == (OO_SINTa)uncompressedSize); (void)decompLen;
				return str;
			}
			else
			{
				check(s == 1);
				std::string temp;
				temp.resize(stringLength);
				OO_SINTa decompLen = OodleLZ_Decompress(data, (OO_SINTa)compressedSize, temp.data(), (OO_SINTa)uncompressedSize);
				check(decompLen == (OO_SINTa)uncompressedSize); (void)decompLen;
				return UTF8_TO_TCHAR(temp.data());
			}
		}
		FORCEINLINE void SkipString()
		{
			const uint64 len = Read7BitEncoded();
			Skip(len);
		}
		FORCEINLINE FStringKey ReadStringKey()
		{
			UBA_ASSERT_READ(sizeof(FStringKey));
			FStringKey k;
			k.a = *(uint64*)m_pos;
			k.b = ((uint64*)m_pos)[1];
			m_pos += sizeof(uint64) * 2;
			return k;
		}
		FORCEINLINE Guid ReadGuid()
		{
			UBA_ASSERT_READ(sizeof(Guid));
			uint64 g[2];
			g[0] = *(uint64*)m_pos;
			g[1] = ((uint64*)m_pos)[1];
			m_pos += sizeof(uint64) * 2;
			return *(Guid*)g;
		}
		FORCEINLINE FCasKey ReadCasKey()
		{
			UBA_ASSERT_READ(sizeof(FCasKey));
			FCasKey k;
			k.a = *(uint64*)m_pos;
			k.b = ((uint64*)m_pos)[1];
			k.c = ((uint32*)m_pos)[4];
			m_pos += sizeof(uint64) * 2 + sizeof(uint32);
			return k;
		}
		FORCEINLINE bool ReadBool() { return ReadByte() != 0; }
		FORCEINLINE uint64 Read7BitEncoded()
		{
			uint64 result = 0;
			uint64 byteIndex = 0;
			bool hasMoreBytes;
			do
			{
				UBA_ASSERT_READ(1);
				uint8 value = *m_pos++;
				hasMoreBytes = value & 0x80;
				result |= uint64(value & 0x7f) << (byteIndex * 7);
				++byteIndex;
			} while (hasMoreBytes);
			return result;
		}
		FORCEINLINE bool TryRead7BitEncoded(uint64& outValue)
		{
			const uint8* pos = m_pos;
			uint64 result = 0;
			uint64 byteIndex = 0;
			bool hasMoreBytes;
			do
			{
				if (m_pos >= m_end)
					return false;
				uint8 value = *pos++;
				hasMoreBytes = value & 0x80;
				result |= uint64(value & 0x7f) << (byteIndex * 7);
				++byteIndex;
			} while (hasMoreBytes);
			outValue = result;
			m_pos = pos;
			return true;
		}
		template<typename CharType> 
		FORCEINLINE CharType ReadUtf8Char()
		{
			UBA_ASSERT_READ(1);
			const uint8 a = *m_pos++;
			if (a <= 127)
				return CharType(a);

			UBA_ASSERT_READ(1);
			const uint8 b = *m_pos++;
			if (a >= 192 && a <= 223)
				return CharType((a - 192) * 64 + (b - 128));

			UBA_ASSERT_READ(1);
			const uint8 c = *m_pos++;
			if (a >= 224 && a <= 239)
				return CharType((a - 224) * 4096 + (b - 128) * 64 + (c - 128));

			if (a >= 240 && a <= 253)
			{
				check(false); // Wide chars cannot exceed 16 bits
				return CharType(~0);
			}

			check(false); // Wide chars cannot exceed 16 bits
			return CharType(~0);
		}
		FORCEINLINE uint32 PeekU32()
		{
			return *(uint32*)m_pos;
		}
		FORCEINLINE uint64 PeekU64()
		{
			return *(uint64*)m_pos;
		}
		FORCEINLINE void Skip(const uint64 size) { m_pos += size; }
		FORCEINLINE uint64 GetPosition() const { return uint64(m_pos - m_begin); }
		FORCEINLINE uint64 GetLeft() const { return uint64(m_end - m_pos); }
		FORCEINLINE void SetPosition(const uint64 pos) { m_pos = m_begin + pos; }
		FORCEINLINE void SetSize(const uint64 size) { m_end = m_begin + size; }
		FORCEINLINE const uint8* GetPositionData() { return m_pos; }

	private:
		FORCEINLINE uint64 InternalReadString(TCHAR* str, const uint64 charLen, FString& OutString)
		{
			if (str == nullptr)
			{
				return 0;
			}

			TCHAR* it = str;
			uint64 left = charLen;
			while (left--)
			{
				UBA_ASSERT_READ(1);
				uint8 a = *m_pos++;
				if (a <= 127)
				{
					*it++ = a;
					continue;
				}
				UBA_ASSERT_READ(1);
				uint8 b = *m_pos++;
				if (a >= 192 && a <= 223)
				{
					*it++ = (a - 192) * 64 + (b - 128);
					continue;
				}
				UBA_ASSERT_READ(1);
				uint8 c = *m_pos++;
				if (a >= 224 && a <= 239)
				{
					*it++ = (a - 224) * 4096 + (b - 128) * 64 + (c - 128);
					continue;
				}
				if (a >= 240 && a <= 253)
				{
					check(false); // Wide chars cannot exceed 16 bits
					*it++ = TCHAR(~0);
					continue;
				}
				check(false); // Wide chars cannot exceed 16 bits
				*it++ = TCHAR(~0);
			}
			*it = 0;
			OutString = FString(str);
			return uint64(it - str);
		}

	protected:
		const uint8* m_begin;
		const uint8* m_pos;
		const uint8* m_end;
	};
}

