#pragma once

#include "CoreMinimal.h"
#include <atomic>
#include <type_traits>

namespace Xiao
{
	constexpr uint64 InvalidValue = 0x7ffffffff;

	struct FStringKey
	{
		constexpr FStringKey() : a(0), b(0) {}
		uint64 a;
		uint64 b;
		bool operator==(const FStringKey& o) const { return a == o.a && b == o.b; }
		bool operator!=(const FStringKey& o) const { return a != o.a || b != o.b; }
		bool operator<(const FStringKey& o) const { if (a != o.a) return a < o.a; return b < o.b; }
	};

#pragma pack(push)
#pragma pack(4)
	struct FCasKey
	{
		constexpr FCasKey() : a(0), b(0), c(0) {}
		constexpr FCasKey(uint64 a_, uint64 b_, uint32 c_) : a(a_), b(b_), c(c_) {}
		uint64 a;
		uint64 b;
		uint32 c;
		bool operator==(const FCasKey& o) const { return a == o.a && b == o.b && c == o.c; }
		bool operator!=(const FCasKey& o) const { return a != o.a || b != o.b || c != o.c; }
		bool operator<(const FCasKey& o) const { if (a != o.a) return a < o.a; if (b != o.b) return b < o.b; return c < o.c; }
	};
#pragma pack(pop)

	enum ELogEntryType : uint8
	{
		LogEntryType_Error = 0,
		LogEntryType_Warning = 1,
		LogEntryType_Info = 2,
		LogEntryType_Detail = 3,
		LogEntryType_Debug = 4,
	};

	struct FProcessLogLine
	{
		FProcessLogLine()
		{}

		FProcessLogLine(const FString& inText, const ELogEntryType inType)
			: text(inText)
			, type(inType)
		{}

		FProcessLogLine& operator=(const FProcessLogLine& Other)
		{
			if (this != &Other)
			{
				text = Other.text;
				type = Other.type;
			}
			return *this;
		}

		FString text;
		ELogEntryType type = LogEntryType_Info;
	};

	constexpr FCasKey CasKeyZero;
	constexpr FCasKey CasKeyInvalid(~0ull, ~0ull, ~0u);

	struct Guid
	{
		uint32 data1 = 0; uint16 data2 = 0; uint16 data3 = 0; uint8 data4[8] = { 0 };
		bool operator==(const Guid& o) const { return *(uint64*)&data1 == *(uint64*)&o.data1 && *(uint64*)data4 == *(uint64*)o.data4; }

		FString GetString() const
		{
			return FString::Printf(TEXT("%ld:%u:%u:%u"), data1, data2, data3, data4);
		}
	};
}

template<>
struct std::hash<Xiao::FCasKey>
{
	size_t operator()(const Xiao::FCasKey& g) const
	{
		return g.a;
	}
};