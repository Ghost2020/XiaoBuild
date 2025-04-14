// /**
//   * @author cxx2020@outlook.com
//   * @date 10:18 PM
//  */

#pragma once

template <typename T, int32 Size = 64>
struct TFixedCircularBuffer
{
	T Buffer[Size];
	int32 BufferStartOffset = 0;
	int32 BufferLength = 0;

	void Reset()
	{
		BufferStartOffset = 0;
		BufferLength = 0;
	}

	void AddValue(const T Value)
	{
		Buffer[BufferStartOffset] = Value;
		BufferStartOffset++;
		if (BufferStartOffset == Size)
		{
			BufferStartOffset = 0;
		}
		if (BufferLength < Size)
		{
			BufferLength++;
		}
	}

	const T GetValue(const int32 RecentIndex) const
	{
		return Buffer[(BufferStartOffset + Size - RecentIndex - 1) % Size];
	}

	const T ComputeAverage() const
	{
		if (BufferLength == 0)
		{
			return 0;
		}
		T Total = 0;
		for (int32 RecentIndex = 0; RecentIndex < BufferLength; ++RecentIndex)
		{
			Total += Buffer[(BufferStartOffset + Size - RecentIndex - 1) % Size];
		}
		return Total / BufferLength;
	}
};
