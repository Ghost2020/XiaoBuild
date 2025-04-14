#pragma once

#include "Serialization/JsonSerializerMacros.h"
#include "Misc/FileHelper.h"
#include "Misc/SecureHash.h"
#include "HAL/PlatformFileManager.h"
#include "Protobuf/files_buffer.pb.h"
#include "XiaoShare.h"

THIRD_PARTY_INCLUDES_START
#if USE_LZ4
#include "Compression/lz4.h"
#else
#include "zstd.h"
#endif
THIRD_PARTY_INCLUDES_END

#include "XiaoLog.h"


namespace XiaoCompress
{
	static int TryDecompress(TArray<uint8>& InCompressedBuffer, TArray<uint8>& OutDecompressBuffer)
	{
#if USE_LZ4
		return LZ4_decompress_safe(reinterpret_cast<char*>(InCompressedBuffer.GetData()), reinterpret_cast<char*>(OutDecompressBuffer.GetData()), InCompressedBuffer.Num(), OutDecompressBuffer.Num());
#else
		return ZSTD_decompress(OutDecompressBuffer.GetData(), OutDecompressBuffer.Num(), InCompressedBuffer.GetData(), InCompressedBuffer.Num());
#endif
	}

	static bool Decompress(TArray<uint8>& InCompressedBuffer, TArray<uint8>& OutDecompressBuffer)
	{
		const int BufferSize = InCompressedBuffer.Num();
		int Multiplier = 5;
		int Size = 0;
		for (int Index = 0; Index < 5; ++Index)
		{
			OutDecompressBuffer.SetNumZeroed(BufferSize * Multiplier);
			Size = XiaoCompress::TryDecompress(InCompressedBuffer, OutDecompressBuffer);
			if (Size > 0)
			{
				break;
			}
			XIAO_LOG(Error, TEXT("TryDecompress Failed::Multiplier::%d"), Multiplier);
			Multiplier *= 5;
		}
		if (Size <= 0)
		{
			XIAO_LOG(Error, TEXT("DecompressFile Failed!"));
			return false;
		}
		OutDecompressBuffer.SetNum(Size);
		return true;
	}

	static bool Decompress(const FString& InCompressedContent, TArray<uint8>& OutDecompressBuffer)
	{
		TArray<uint8> CompressedBuffer;
		CompressedBuffer.SetNum(InCompressedContent.Len());
		if (StringToBytes(InCompressedContent, CompressedBuffer.GetData(), CompressedBuffer.Num()) > 0)
		{
			return Decompress(CompressedBuffer, OutDecompressBuffer);
		}
		return false;
	}

	static bool Compress(TArray<uint8>& InRawBuffer, TArray<uint8>& OutCompressedBuffer, const int InCompressLevel = 5)
	{
		const int BufferLength = InRawBuffer.Num();
		int Multiplier = 5;
		for (int Index = 0; Index < 5; ++Index)
		{
			const uint32 BufferSize = BufferLength * Multiplier;
			OutCompressedBuffer.SetNumZeroed(BufferSize);
#if USE_LZ4
			const int Size = LZ4_compress_fast(reinterpret_cast<char*>(InRawBuffer.GetData()), reinterpret_cast<char*>(OutCompressedBuffer.GetData()), BufferLength, BufferLength, InCompressLevel);
#else
			const int Size = ZSTD_compress(OutCompressedBuffer.GetData(), OutCompressedBuffer.Num(), InRawBuffer.GetData(), BufferLength, InCompressLevel);
#endif
			if (Size >= 0)
			{
				OutCompressedBuffer.SetNum(Size);
				return true;
			}

#if !USE_LZ4
			XIAO_LOG(Error, TEXT("ZSTD Compress Error::%s\n Buffer Size::%d"), UTF8_TO_TCHAR(ZSTD_getErrorName(Size)), BufferSize);
#endif
			Multiplier *= 5;
		}
		return false;
	}

	static bool Compress(TArray<uint8>& InRawBuffer, FString& InCompressedContent, const int InCompressLevel = 5)
	{
		TArray<uint8> Buffer;
		if (Compress(InRawBuffer, Buffer))
		{
			InCompressedContent = BytesToString(Buffer.GetData(), Buffer.Num());
			return true;
		}
		return false;
	}

	static bool Compress(const FString& InContent, TArray<uint8>& OutCompressedBuffer, const int InCompressLevel = 5)
	{
		TArray<uint8> RawBuffer;
		const int BufferLength = InContent.Len();
		RawBuffer.SetNumZeroed(BufferLength);
		// TODO 下面可能考虑memcpy 有问题
		const int CopySize = StringToBytes(InContent, RawBuffer.GetData(), BufferLength);
		RawBuffer.SetNum(CopySize);

		OutCompressedBuffer.SetNumZeroed(CopySize);
		return Compress(RawBuffer, OutCompressedBuffer, InCompressLevel);
	}

	static bool Compress(const FString& InContent, FString& InCompressedContent, const int InCompressLevel = 5)
	{
		TArray<uint8> Buffer;
		if (Compress(InContent, Buffer, InCompressLevel))
		{
			InCompressedContent = BytesToString(Buffer.GetData(), Buffer.Num());
			return true;
		}
		return false;
	}
}

struct FFilesProxy
{
private:
    FDateTime Start;

	struct FPathPairBlob
	{
		FString Path;
		TArray<uint8> Blob;
	};

public:
    FString ComRefPath;
    FString DepRefPath;
    uint32 BufferState = 0; // 0::空的状态   1::压缩    2::解压
	FFilesBuffer Buffer;


	void Reset()
	{
		ComRefPath = TEXT("");
		DepRefPath = TEXT("");
		BufferState = 0;
		Buffer.Clear();
	}

	bool Compress(const FString& InTargetFile, const TArray<FString>& InFiles, TArray<uint8>& OutData, const FString InRelativeFolder, const int InCompressLevel = 5, const FString& InEncryptKey = TEXT(""))
	{
		XIAO_LOG(Verbose, TEXT("Compress:: file num::%d Begin!"), InFiles.Num());

		//auto& Platform = FPlatformFileManager::Get().GetPlatformFile();
		//TArray<FFileDesc> FileSizeArray;
		//for (const auto& File : InFiles)
		//{
		//	if (!FPaths::FileExists(File))
		//	{
		//		continue;
		//	}

		//	const auto FileHandle = Platform.OpenRead(*File);
		//	FileSizeArray.Add(FFileDesc(File, FileHandle->Size()));
		//}
		//// 先按大小进行排序
		//FileSizeArray.Sort([](const FFileDesc& L, const FFileDesc& R)
		//{
		//	return L.FileSize <= R.FileSize;
		//});

		//// 分组进行读取到buffer
		//const int32 ThreadNum = FPlatformMisc::NumberOfCores();
		//const int32 BatchNum = FMath::CeilToInt(double(FileSizeArray.Num()) / double(ThreadNum));
		//TArray<FPathPairBlob> Blobs;
		//for (int Index = 0; Index < FileSizeArray.Num(); ++Index)
		//{
		//	FPathPairBlob Blob;
		//	Blob.Path = FileSizeArray[Index].FilePath;
		//	Blobs.Add(Blob);
		//}Blobs.Shrink();

		//XIAO_LOG(Verbose, TEXT("Compress:: ready to start [%d] batch back worker:: Begin!"), BatchNum);
		//for (int BatchIndex = 0; BatchIndex < BatchNum; ++BatchIndex)
		//{
		//	TArray<TFuture<void>> BatchFuture;
		//	// 开启一个批次的线程运行
		//	for (int ThreadIndex = 0; ThreadIndex < ThreadNum; ThreadIndex++)
		//	{
		//		const int Index = BatchIndex*ThreadNum + ThreadIndex;
		//		auto& Blob = Blobs[Index];
		//		if (Index < Blobs.Num())
		//		{
		//			auto Future = AsyncThread([&Blob]()
		//			{
		//				if (!FFileHelper::LoadFileToArray(Blob.Blob, *Blob.Path))
		//				{
		//					XIAO_LOG(Error, TEXT("LoadFileToString Failed::%s"), *Blob.Path);
		//				}
		//			}, 0, TPri_Highest);

		//			BatchFuture.Add(MoveTemp(Future));
		//		}
		//	}

		//	// 等待这一批次完成
		//	for (const auto& Future : BatchFuture)
		//	{
		//		Future.Get();
		//	}
		//}

		for (const FString& File : InFiles)
		{
			if (!FPaths::FileExists(File))
			{
				XIAO_LOG(Error, TEXT("File Not Exist::%s"), *File);
				return false;
			}

			TArray<uint8> Content;
			if (!FFileHelper::LoadFileToArray(Content, *File))
			{
				XIAO_LOG(Error, TEXT("LoadFileToString Failed::%s"), *File);
				return false;
			}

			FString CompressedContent = TEXT("");
			if (Content.Num() > 0)
			{
				if (!XiaoCompress::Compress(Content, CompressedContent))
				{
					XIAO_LOG(Error, TEXT("Compress Failed::%s"), *File);
					return false;
				}
			}
			FString FilePath = File;
			if (!FPaths::MakePathRelativeTo(FilePath, *InRelativeFolder))
			{
				XIAO_LOG(Error, TEXT("MakePathRelativeTo Failed::%s"), *File);
				return false;
			}
			Buffer.add_files(TCHAR_TO_UTF8(*FilePath));
			Buffer.add_buffers(TCHAR_TO_UTF8(*CompressedContent));
		}

		if (!InEncryptKey.IsEmpty())
		{
			std::string Content;
			// TODO 可能会有问题
			if (!Buffer.SerializeToString(&Content))
			{
				XIAO_LOG(Error, TEXT("SerializeToString Failed!"));
				return false;
			}

			const FString TempContent = UTF8_TO_TCHAR(Content.c_str());
			FString EncryptContent;
			if (!EncryptString(TempContent, InEncryptKey, EncryptContent))
			{
				XIAO_LOG(Error, TEXT("EncryptString Failed!"));
				return false;
			}

			OutData.SetNum(EncryptContent.Len());
			if (StringToBytes(EncryptContent, OutData.GetData(), EncryptContent.Len()) <= 0)
			{
				XIAO_LOG(Error, TEXT("StringToBytes Failed!"));
				return false;
			}
		}
		else
		{
			const uint32 BufferSize = Buffer.ByteSizeLong();
			OutData.SetNum(BufferSize);
			if (!Buffer.SerializeToArray(OutData.GetData(), BufferSize))
			{
				XIAO_LOG(Error, TEXT("SerializeToArray Failed!"));
				return false;
			}
		}
		
		bool Flag = true;
		if (FPaths::ValidatePath(InTargetFile))
		{
			Flag = FFileHelper::SaveArrayToFile(OutData, *InTargetFile, &IFileManager::Get(), FILEWRITE_EvenIfReadOnly);
		}
		XIAO_LOG(Log, TEXT("CompressFiles Consuming time::%f seconds"), (FDateTime::UtcNow() - Start).GetTotalSeconds());
		return Flag;
	}

    bool Compress(const FString &InTargetFile, TArray<uint8>& OutData, const FString &InCompressFolder = TEXT(""), const int InCompressLevel = 5, const FString &InEncryptKey = TEXT(""))
    {
        Start = FDateTime::UtcNow();
		TArray<FString> Files;
        if (FPaths::DirectoryExists(InCompressFolder))
        {
            IFileManager &FileManager = IFileManager::Get();
            FileManager.FindFilesRecursive(Files, *InCompressFolder, TEXT("*"), true, false);
        }

		return Compress(InTargetFile, Files, OutData, InCompressFolder, InCompressLevel, InEncryptKey);
    }

    bool Decompress(const FString &InTargetFile, const FString &InDecompressDir, const FString &InDecryptKey = TEXT(""))
    {
        if (!FPaths::DirectoryExists(InDecompressDir))
        {
			XIAO_LOG(Error, TEXT("InDecompressDir is not exist::%s"), *InDecompressDir);
            return false;
        }

        Start = FDateTime::UtcNow();
		if(InDecryptKey.IsEmpty())
		{
			TArray<uint8> CompressedBuffer;
			if (!FFileHelper::LoadFileToArray(CompressedBuffer, *InTargetFile))
			{
				XIAO_LOG(Error, TEXT("LoadFileToArray Failed::%s"), *InTargetFile);
				return false;
			}
			if (!Buffer.ParseFromArray(CompressedBuffer.GetData(), CompressedBuffer.Num()))
			{
				XIAO_LOG(Error, TEXT("Parse From Array Failed::%s \n byte length::%d"), *InTargetFile , CompressedBuffer.Num());
				return false;
			}
		}
		else
		{
			FString EncryptContent;
			if (!FFileHelper::LoadFileToString(EncryptContent, *InTargetFile))
			{
				XIAO_LOG(Error, TEXT("LoadFileToString Failed::%s"), *InTargetFile);
				return false;
			}
			FString DecryptContent;
			if(!DecryptString(EncryptContent, InDecryptKey, DecryptContent))
			{
				XIAO_LOG(Error, TEXT("DecryptString Failed::%s"), *InTargetFile);
				return false;
			}

			const std::string Content = TCHAR_TO_UTF8(*DecryptContent);
			if (!Buffer.ParseFromString(Content))
			{
				XIAO_LOG(Error, TEXT("Parse From Array Failed::%s \n byte length::%d"), *InTargetFile , Content.size());
				return false;
			}
		}

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        for (int32 Index = 0; Index < Buffer.files_size(); ++Index)
        {
			const FString Filename = UTF8_TO_TCHAR(Buffer.files(Index).c_str());
            const FString CleanFilename = FPaths::GetCleanFilename(Filename);
            const FString TargetPath = FPaths::ConvertRelativePathToFull(InDecompressDir, Filename);
			const FString FileFolder = FPaths::GetPath(TargetPath);
			if (!FPaths::DirectoryExists(FileFolder))
			{
				PlatformFile.CreateDirectoryTree(*FileFolder);
			}
			const FString CompressedContent = UTF8_TO_TCHAR(Buffer.buffers(Index).c_str());
			TArray<uint8> DecompressBuffer;
			if (XiaoCompress::Decompress(CompressedContent, DecompressBuffer))
			{
				if (!FFileHelper::SaveArrayToFile(DecompressBuffer, *TargetPath, &IFileManager::Get(), FILEWRITE_EvenIfReadOnly))
				{
					XIAO_LOG(Error, TEXT("SaveStringToFile Failed::%s"), *TargetPath);
				}
			}
        }

		XIAO_LOG(Log, TEXT("Decompress Consuming time::%.5f seconds"), (FDateTime::UtcNow() - Start).GetTotalSeconds());
        return true;
    }
};

static bool GetSystemHash(FString& OutHash, std::string& OutBuffer)
{
	const FString XiaoHome = FPlatformMisc::GetEnvironmentVariable(TEXT("XIAO_HOME"));
	if (!FPaths::DirectoryExists(XiaoHome))
	{
		XIAO_LOG(Error, TEXT("SyncVersion failed::XIAO_HOME::[%s] not exist!"), *XiaoHome);
		return false;
	}

	FFilesProxy FileProxy;
	TArray<uint8> FileBuffer;
	if (!FileProxy.Compress(TEXT(""), FileBuffer, XiaoHome))
	{
		XIAO_LOG(Error, TEXT("SyncVersion failed::Compress target folder[%s] failed!"), *XiaoHome);
		return false;
	}

	OutHash = FMD5::HashBytes(FileBuffer.GetData(), FileBuffer.Num());
	OutBuffer.reserve(FileBuffer.Num());
	FMemory::Memcpy(OutBuffer.data(), FileBuffer.GetData(), FileBuffer.Num());
	return true;
}