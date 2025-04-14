#pragma once

#include "BaseSerializable.h"
#include "Misc/DateTime.h"

namespace XiaoDB
{
	struct FApiKeys final : FBaseSerializable
	{
		uint32 Id;
		FString Key;
		FString Status{TEXT("Active")};
		uint32 CreateByUserId;
		FDateTime CreatedAt, UpdatedAt, ExpirationDate, LastUsed;
		FString Description;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(Id);
			JSON_MCI_VALUE(Key);
			JSON_MCI_VALUE(Status);
			JSON_MCI_VALUE(CreateByUserId);
			JSON_DATETIME_VALUE(CreatedAt);
			JSON_DATETIME_VALUE(UpdatedAt);
			JSON_DATETIME_VALUE(ExpirationDate);
			JSON_DATETIME_VALUE(LastUsed);
			JSON_MCI_VALUE(Description);
		END_JSON_SERIALIZER
	};

	
	struct FAuthentications final : FBaseSerializable
	{
		uint32 UserId = 0;
		FString RefreshToken, AccessToken, CloudAccessToken;
		FDateTime CreatedAt, UpdatedAt;

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(UserId);
			JSON_MCI_VALUE(RefreshToken);
			JSON_MCI_VALUE(AccessToken);
			JSON_MCI_VALUE(CloudAccessToken);
			JSON_DATETIME_VALUE(CreatedAt);
			JSON_DATETIME_VALUE(UpdatedAt);
		END_JSON_SERIALIZER
	};


	struct FAuthUser : FJsonSerializable
	{
		explicit FAuthUser(){}
	
		FString Username = TEXT("user");
		FString Password;
		FString LastLogin;
		FString LoginMachine;
		bool bCompleVerifi = true;

		BEGIN_JSON_SERIALIZER
			JSON_SERIALIZE("username", Username);
			JSON_SERIALIZE("password", Password);
			JSON_SERIALIZE("last_active", LastLogin);
			JSON_SERIALIZE("machine_id", LoginMachine);
			JSON_SERIALIZE("complte_verifi", bCompleVerifi);
		END_JSON_SERIALIZER
	};


	struct FUserDesc : FBaseSerializable
	{
		uint32 Id = 0;
		FString Username;
		FString Email;
		FString FirstName, LastName;
		uint32 Status = 0;
		uint32 Role = 0;
		FDateTime LastLogin = FDateTime::UtcNow();
		uint32 CreateByUserId = 0;
		uint32 LoginAttempts = 0;
		FString LoginMachine;

		FORCEINLINE bool operator==(const FUserDesc& InAnother) const
		{
			return this->Id == InAnother.Id && this->Username == InAnother.Username;
		}

		FUserDesc& operator=(const FUserDesc& InAnother)
		{
			this->Id = InAnother.Id;
			this->Username = InAnother.Username;
			this->Email = InAnother.Email;
			this->FirstName = InAnother.FirstName;
			this->LastName = InAnother.LastName;
			this->Status = InAnother.Status;
			this->Role = InAnother.Role;
			this->LastLogin = InAnother.LastLogin;
			this->CreateByUserId = InAnother.CreateByUserId;
			this->LoginAttempts = InAnother.LoginAttempts;
			this->LoginMachine = InAnother.LoginMachine;
			return *this;
		}

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(Id);
			JSON_MCI_VALUE(Username);
			JSON_MCI_VALUE(Email);
			JSON_MCI_VALUE(FirstName);
			JSON_MCI_VALUE(LastName);
			JSON_MCI_VALUE(Status);
			JSON_MCI_VALUE(Role);
			JSON_DATETIME_VALUE(LastLogin);
			JSON_MCI_VALUE(CreateByUserId);
			JSON_MCI_VALUE(LoginAttempts);
			JSON_MCI_VALUE(LoginMachine);
		END_JSON_SERIALIZER
	};
	static const FString SCertificateBufferFile = FPaths::ConvertRelativePathToFull(FString::Printf(TEXT("%s/XiaoBuild/%s.data"), *FPaths::GetPath(FPlatformProcess::ApplicationSettingsDir()), TEXT("Misc")));
	

	struct FUsersDesc : FJsonSerializable
	{
	private:
		TArray<FString> _Array;
	public:
		TArray<FUserDesc> Users;
		
		BEGIN_JSON_SERIALIZER
			if(Serializer.IsSaving())
			{
				_Array.Empty();
				for(const auto& User : Users)
				{
					_Array.Add(User.ToJson());
				}
				JSON_SERIALIZE_ARRAY("users", _Array);
			}
			else if(Serializer.IsLoading())
			{
				JSON_SERIALIZE_ARRAY("users", _Array);
				Users.Empty();
				for(const auto& Content : _Array)
				{
					FUserDesc Desc;
					if(Desc.FromJson(Content))
					{
						Users.Add(Desc);
					}
				}
			}
		END_JSON_SERIALIZER
	};


	struct FUserDetail final : FUserDesc
	{
		FString Password;
		FDateTime CreateAt = FDateTime::UtcNow();
		FDateTime UpdateAt = FDateTime::UtcNow();

		FORCEINLINE bool operator==(const FUserDetail& InAnother) const
		{
			return this->Username == InAnother.Username && this->Password == InAnother.Password;
		}

		FORCEINLINE FUserDetail& operator=(const FUserDetail& InAnother)
		{
			FUserDesc::operator=(InAnother);
			if (InAnother.Password.Len() > 7)
			{
				this->Password = InAnother.Password;
			}
			this->CreateAt = InAnother.CreateAt;
			this->UpdateAt = InAnother.UpdateAt;
			return *this;
		}

		BEGIN_JSON_SERIALIZER
			JSON_MCI_VALUE(Password);
			JSON_DATETIME_VALUE(CreateAt);
			JSON_DATETIME_VALUE(UpdateAt);
			FUserDesc::Serialize(Serializer, true);
		END_JSON_SERIALIZER

		void GetUserDesc(FUserDesc& OutDesc) const
		{
			OutDesc.Id = this->Id;
			OutDesc.Username = this->Username;
			OutDesc.Email = this->Email;
			OutDesc.FirstName = this->FirstName;
			OutDesc.LastName = this->LastName;
			OutDesc.Status = this->Status;
			OutDesc.Role = this->Role;
			OutDesc.LastLogin = this->LastLogin;
			OutDesc.CreateByUserId = this->CreateByUserId;
			OutDesc.LoginAttempts = this->LoginAttempts;
			OutDesc.LoginMachine = this->LoginMachine;
		}
	};
}