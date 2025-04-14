/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -12:18 AM
 */
#pragma once

#include "CoreMinimal.h"

namespace Xiao
{
	struct FStringKey
	{
		constexpr FStringKey() : a(0), b(0) {}
		uint64 a;
		uint64 b;
		bool operator==(const FStringKey& o) const { return a == o.a && b == o.b; }
		bool operator!=(const FStringKey& o) const { return a != o.a || b != o.b; }
		bool operator<(const FStringKey& o) const { if (a != o.a) return a < o.a; return b < o.b; }
	};
	constexpr FStringKey StringKeyZero;

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

	constexpr FCasKey CasKeyZero;
}