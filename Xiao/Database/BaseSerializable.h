#pragma once

#include "Serialization/JsonSerializerMacros.h"

#define JSON_MCI_VALUE(var) JSON_SERIALIZE(#var, var)
#define JSON_DATETIME_VALUE(var) JSON_SERIALIZE_DATETIME_UNIX_TIMESTAMP(#var, var)

namespace XiaoDB
{
	struct FBaseSerializable : FJsonSerializable
	{
		bool bSame;
	};

	struct FTimestamp : FBaseSerializable
	{
		int64 StartTs;					// PrimaryKey
		int64 CurrentInd = 1;
		int64 EndTs = 4102358400000;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(StartTs);
			JSON_MCI_VALUE(CurrentInd);
			JSON_MCI_VALUE(EndTs);
		END_JSON_SERIALIZER
	};
}
