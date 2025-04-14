#pragma once

#include "BaseSerializable.h"

namespace XiaoDB
{
	struct FCloud final : FBaseSerializable 
	{
		FString CoordinatorId;
		FString CloudCoordinatorId;
		FString CloudToken;
		FString CloudApiToken;
		FString Secret;
		FString Status;					// Initialized
		FDateTime CreatedAt;
		FDateTime UpdatedAt;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(CoordinatorId);
			JSON_MCI_VALUE(CloudCoordinatorId);
			JSON_MCI_VALUE(CloudToken);
			JSON_MCI_VALUE(CloudApiToken);
			JSON_MCI_VALUE(Secret);
			JSON_MCI_VALUE(Status);
			JSON_MCI_VALUE(CreatedAt);
			JSON_MCI_VALUE(UpdatedAt);
		END_JSON_SERIALIZER
	};
}
