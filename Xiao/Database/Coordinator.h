#pragma once

#include "BaseSerializable.h"

namespace XiaoDB
{
	struct FAgent final : FTimestamp
	{
		FString AgentId;	// PrimaryKey
		FString MachineId;
		FString AgentName;
		FString VersionNumber;
		FString BuildNumber;
		uint32 InitiatorAllocationTypeId;
		uint32 HelperAllocationTypeId;
		uint32 CiInitiatorAllocationTypeId;
		bool IsRegisteredInitiator = false;
		uint32 NumberOfHelperCores;
		bool IsBuildCacheAllocated;
		uint32 LicenseStatus;
		bool IsDisableAsHelper = false;
		bool IsOnline = false;
		bool IsDeleted = false;
		FString XbSettings;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(AgentId);
			JSON_MCI_VALUE(MachineId);
			JSON_MCI_VALUE(AgentName);
			JSON_MCI_VALUE(VersionNumber);
			JSON_MCI_VALUE(BuildNumber);
			JSON_MCI_VALUE(InitiatorAllocationTypeId);
			JSON_MCI_VALUE(HelperAllocationTypeId);
			JSON_MCI_VALUE(CiInitiatorAllocationTypeId);
			JSON_MCI_VALUE(IsRegisteredInitiator);
			JSON_MCI_VALUE(NumberOfHelperCores);
			JSON_MCI_VALUE(IsBuildCacheAllocated);
			JSON_MCI_VALUE(LicenseStatus);
			JSON_MCI_VALUE(IsDisableAsHelper);
			JSON_MCI_VALUE(IsOnline);
			JSON_MCI_VALUE(IsDeleted);
			JSON_MCI_VALUE(XbSettings);
			FTimestamp::Serialize(Serializer, bFlatObject);
		END_JSON_SERIALIZER
	};


	struct FAgentBuildGroup final : FTimestamp
	{
		uint32 AgentId;		// PrimaryKey
		uint32 CoordinatorBuildGroupId;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(AgentId);
			JSON_MCI_VALUE(CoordinatorBuildGroupId);
			FTimestamp::Serialize(Serializer, bFlatObject);
		END_JSON_SERIALIZER
	};


	struct FAllocationType final : FJsonSerializable
	{
		uint32 TypeId;		// PrimaryKey
		FString TypeName;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(TypeId);
			JSON_MCI_VALUE(TypeName);
		END_JSON_SERIALIZER
	};


	struct FBackCoordInfo final : FTimestamp
	{
		FString MachineId;		// PrimaryKey
		bool IsSelected;
		FString VersionNumber;
		FString BuildNumber;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(MachineId);
			JSON_MCI_VALUE(IsSelected);
			JSON_MCI_VALUE(VersionNumber);
			JSON_MCI_VALUE(BuildNumber);
			FTimestamp::Serialize(Serializer, bFlatObject);
		END_JSON_SERIALIZER
	};

	
	struct FCloudState final : FJsonSerializable
	{
		// PrimaryKey
		uint32 StateId;			// 0		1		2		3		4		 5			  6
		FString StateName;		// Disabled Enabled Paused Creating Deleting Deactivating InProcess

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(StateId);
			JSON_MCI_VALUE(StateName);
		END_JSON_SERIALIZER
	};


	struct FCoordinator final : FTimestamp
	{
		FString Id;			// PrimaryKey
		FString LicenseId;
		FString MachineId;
		uint32 LicenseTypeId;
		uint32 CloudStateId;
		FString VersionNumber;
		FString BuildNumber;
		FString XbSettings;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(Id);
			JSON_MCI_VALUE(LicenseId);
			JSON_MCI_VALUE(MachineId);
			JSON_MCI_VALUE(LicenseTypeId);
			JSON_MCI_VALUE(CloudStateId);
			JSON_MCI_VALUE(VersionNumber);
			JSON_MCI_VALUE(BuildNumber);
			JSON_MCI_VALUE(XbSettings);
			FTimestamp::Serialize(Serializer, bFlatObject);
		END_JSON_SERIALIZER
	};


	struct FCoordinatorBuildGroup final : FBaseSerializable
	{
		uint32 GroupId;		// PrimaryKey
		FString GroupName;
		bool IsDeleted = false;
		int64 CreatedTs;
		int64 DeletedTs = 4102358400000;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(GroupId);
			JSON_MCI_VALUE(GroupName);
			JSON_MCI_VALUE(IsDeleted);
			JSON_MCI_VALUE(CreatedTs);
			JSON_MCI_VALUE(DeletedTs);
		END_JSON_SERIALIZER
	};


	struct FFeature final : FBaseSerializable
	{
		uint32 Id;			// PrimaryKey
		uint32 TypeId;		// PrimaryKey
		FString Name;
		FString Description;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(Id);
			JSON_MCI_VALUE(TypeId);
			JSON_MCI_VALUE(Name);
			JSON_MCI_VALUE(Description);
		END_JSON_SERIALIZER
	};


	struct FFeatureType final : FBaseSerializable
	{
		uint32 TypeId;		// PrimaryKey
		FString TypeName;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(TypeId);
			JSON_MCI_VALUE(TypeName);
		END_JSON_SERIALIZER
	};


	struct FLicense final : FTimestamp
	{
		FString Id;			// PrimaryKey
		uint32 TypeId = 3;
		FString Tier;
		uint32 ExpirationDate;
		uint32 FixedHelperCores;
		uint32 FloatingHelperCores;
		uint32 FixedInitiators;
		uint32 FloatingInitiators;
		uint32 FixedCiInitiators;
		uint32 FloatingCiInitiators;
		uint32 BuildCache;
		bool IsLocked = true;
		bool IsActive = false;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(Id);
			JSON_MCI_VALUE(TypeId);
			JSON_MCI_VALUE(Tier);
			JSON_MCI_VALUE(ExpirationDate);
			JSON_MCI_VALUE(FixedHelperCores);
			JSON_MCI_VALUE(FloatingHelperCores);
			JSON_MCI_VALUE(FixedInitiators);
			JSON_MCI_VALUE(FloatingInitiators);
			JSON_MCI_VALUE(FixedCiInitiators);
			JSON_MCI_VALUE(FloatingCiInitiators);
			JSON_MCI_VALUE(BuildCache);
			JSON_MCI_VALUE(IsLocked);
			JSON_MCI_VALUE(IsActive);
			FTimestamp::Serialize(Serializer, bFlatObject);
		END_JSON_SERIALIZER
	};


	struct FLicenseFeature final : FTimestamp
	{
		uint32 Id;			// PrimaryKey
		FString FeatureId;
		FString TypeId;
		bool IsActive;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(Id);
			JSON_MCI_VALUE(FeatureId);
			JSON_MCI_VALUE(TypeId);
			JSON_MCI_VALUE(IsActive);
			FTimestamp::Serialize(Serializer, bFlatObject);
		END_JSON_SERIALIZER
	};


	struct FLicenseStatus final : FBaseSerializable
	{
		uint32 Id = 2;		// PrimaryKey	0		1		2			3			4
		FString Name;		//				Valid	Error	NoLicense	Duplicate	Restricted
		
		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(Id);
			JSON_MCI_VALUE(Name);
		END_JSON_SERIALIZER
	};


	struct FMachine final : FTimestamp
	{
		FString Id;
		FString Name;
		uint32 Cores;
		uint32 VirtualMemory;
		uint32 PhysicalMemory;
		uint32 DiskSize;
		uint32 TypeId;
		FString OsVersion;
		bool IsDeleted;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(Id);
			JSON_MCI_VALUE(Name);
			JSON_MCI_VALUE(Cores);
			JSON_MCI_VALUE(VirtualMemory);
			JSON_MCI_VALUE(PhysicalMemory);
			JSON_MCI_VALUE(DiskSize);
			JSON_MCI_VALUE(TypeId);
			JSON_MCI_VALUE(OsVersion);
			JSON_MCI_VALUE(IsDeleted);
			FTimestamp::Serialize(Serializer, bFlatObject);
		END_JSON_SERIALIZER
	};


	struct FMachineType final : FBaseSerializable
	{
		uint32 Id = 2;		// PrimaryKey	0		1		2			3			4
		FString Name;		//				
		
		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(Id);
			JSON_MCI_VALUE(Name);
		END_JSON_SERIALIZER
	};
}
