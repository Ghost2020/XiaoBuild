#pragma once

#include "Users.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "XiaoLog.h"
#include "XiaoShare.h"
#include "XiaoShareField.h"
#include "Protobuf/system_settings.pb.h"

static const FString SLicenseDatabaseFile = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPlatformProcess::ApplicationSettingsDir(), TEXT("XiaoBuild/License.xdb")));

namespace XiaoDB
{
	struct FLicenseDatabase final : FJsonSerializable
	{
		TArray<FUserDetail> Users;
		TArray<FString> UsersArray;
		FSystemSettings SystemSettings;
		FString SettingsString;

		BEGIN_JSON_SERIALIZER
			if (Serializer.IsSaving())
			{
				UsersArray.Reset();
				for (const auto& User : Users)
				{
					UsersArray.Add(User.ToJson());
				}
				JSON_SERIALIZE_ARRAY("users", UsersArray);
				SettingsString.Empty();
				std::string Protobuf;
				if (SystemSettings.SerializeToString(&Protobuf))
				{
					SettingsString = UTF8_TO_TCHAR(Protobuf.c_str());
					JSON_SERIALIZE("settings", SettingsString);
				}
			}
			else if(Serializer.IsLoading())
			{
				JSON_SERIALIZE_ARRAY("users", UsersArray);
				Users.Reset();
				for (const auto& Content : UsersArray)
				{
					FUserDetail Detail;
					if (Detail.FromJson(Content))
					{
						Users.Add(Detail);
					}
				}

				JSON_SERIALIZE("settings", SettingsString);
				SystemSettings.ParseFromString(TCHAR_TO_UTF8(*SettingsString));
			}
		END_JSON_SERIALIZER

		void Update()
		{
			XIAO_LOG(Log, TEXT("Update License database begin"));
			Save();
			Upload();
			XIAO_LOG(Log, TEXT("Update License database finish"));
		}

		bool Load()
		{
			XIAO_LOG(Log, TEXT("Load License database Begin!"));

			if(!FPaths::FileExists(SLicenseDatabaseFile))
			{
				XIAO_LOG(Error, TEXT("FileExists Failed::%s"), *SLicenseDatabaseFile);
				return false;
			}

			FString DatabaseContent;
			if(!FFileHelper::LoadFileToString(DatabaseContent, *SLicenseDatabaseFile))
			{
				XIAO_LOG(Error, TEXT("LoadFileToString Failed::%s"), *SLicenseDatabaseFile);
				return false;
			}

			FString DecryptContent;
			if(!DecryptString(DatabaseContent, XiaoEncryptKey::SLicense, DecryptContent))
			{
				XIAO_LOG(Error, TEXT("DecryptString Failed!"));
				return false;
			}

			if (!this->FromJson(DecryptContent))
			{
				XIAO_LOG(Error, TEXT("FromJson Failed!"));
				return false;
			}

			XIAO_LOG(Log, TEXT("Load License database Finish!"));
			return true;
		}
		
		bool Save()
		{
			XIAO_LOG(Log, TEXT("Save license database begin!"));
			if(!FPaths::FileExists(SLicenseDatabaseFile))
			{
				const FString Folder = FPaths::GetPath(SLicenseDatabaseFile);
				if(!FPaths::DirectoryExists(Folder))
				{
					IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
					PlatformFile.CreateDirectoryTree(*Folder);
				}
			}

			FString EncryptContent;
			if(!EncryptString(ToJson(), XiaoEncryptKey::SLicense, EncryptContent))
			{
				XIAO_LOG(Error, TEXT("License database EncryptString Failed!"));
				return false;
			}

			const FString DataFolder = FPaths::GetPath(SLicenseDatabaseFile);
			if(!FPaths::DirectoryExists(DataFolder))
			{
				IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
				PlatformFile.CreateDirectoryTree(*DataFolder);
			}

			if(!FFileHelper::SaveStringToFile(EncryptContent, *SLicenseDatabaseFile))
			{
				XIAO_LOG(Error, TEXT("SaveStringToFile Failed::%s!"), *SLicenseDatabaseFile);
				return false;
			}

			XIAO_LOG(Log, TEXT("Save license database Finish!"));
			return true;
		}

		void Upload()
		{
			// TODO 上传文件
		}
	};
}