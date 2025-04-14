/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#include "CoreMinimal.h"

static const uint32 SMaxPathLength = 1024;

namespace Xiao
{
	inline bool FixPath(const FString& InFileName, const FString& InWorkingDir, FString& OutPath)
	{
		const TCHAR* WorkingDir = *InWorkingDir;
		uint32 BufferCharLen = 0;
		const uint64 WorkingDirCharLen = InWorkingDir.Len();
		TCHAR* Buffer = const_cast<TCHAR*>(*OutPath);
		Buffer[1] = 0;
		ensureMsgf(WorkingDir == nullptr || !*WorkingDir || WorkingDir[WorkingDirCharLen-1] == *FPlatformMisc::GetDefaultPathSeparator(), TEXT("WorkingDir needs to end with path separator"));

#if PLATFORM_WINDOWS
		const TCHAR* Read = *InFileName;
		TCHAR* Write = Buffer;
		TCHAR LastLastChar = 0;
		TCHAR LastChar = 0;
		bool bHasDotBeforeSlash = false;
		bool bHasDotDot = false;
		//bool hasSlash = false;
		bool bSeenNonBackslash = false;
		if (InFileName[0] == '\"')
			++Read;

		if (const TCHAR* Tilde = FCString::Strchr(*InFileName, TEXT('~')))
		{
			TCHAR FullName[1024];
			// Since this might be a in memory file we can't use the actual full name, so let's find first slash after '~'
			const TCHAR* Backslash = FCString::Strchr(Tilde, TEXT('\\'));
			FCString::Strcpy(FullName, SIZEOF_ARRAY(FullName), *InFileName);
			if (Backslash)
				FullName[Backslash - *InFileName] = 0;
		
			const uint32 Len = ::GetLongPathNameW(FullName, FullName, SIZEOF_ARRAY(FullName));
			if (!Len)
			{
				return false;
			}
			if (Backslash)
				FCString::Strcpy(FullName + Len, SIZEOF_ARRAY(FullName) - Len, Backslash);
			Read = FullName;
		}
	
		while (TCHAR c = *Read++)
		{
			if (c == '/')
			{
				c = '\\';
			}
		
			else if (c == '\"')
			{
				*Write = 0;
				break;
			}
			else if (c == '.' && LastChar == '.')
			{
				bHasDotDot = true;
			}
		
			if (c == '\\')
			{
				if (LastChar == '?' && LastLastChar == '?') // We want to get rid of \\??\  .
				{
					Write = Buffer;
					continue;
				}
				//hasSlash = true;
				if (LastChar == '.')
				{
					bHasDotBeforeSlash = true;
				}
				if (LastChar == '\\' && bSeenNonBackslash)
				{
					continue;
				}
			}
			else
			{
				bSeenNonBackslash = true;
			}
			*Write++ = c;
			LastLastChar = LastChar;
			LastChar = c;
		}
	
		if (LastChar == '.' && LastLastChar == '\\') // Fix path <path>\.
			Write -= 2;
		if (LastChar == '\\')
			--Write;
	
		*Write = 0;
		uint64 CharLen = static_cast<uint64>(Write - Buffer + 1);
		bool bStartsWithDoubleBackslash = false;
		if (LastChar == '.' && LastLastChar == 0) // Sometimes path is '.'
		{
			ensureMsgf(WorkingDir && *WorkingDir, TEXT("Working dir is null or empty"));
			ensureMsgf(WorkingDirCharLen < SMaxPathLength, TEXT("%llu < %llu"), WorkingDirCharLen, SMaxPathLength);
			memcpy(Buffer, WorkingDir, WorkingDirCharLen*sizeof(TCHAR));
			Buffer[WorkingDirCharLen - 1] = 0;
			CharLen = WorkingDirCharLen;
		}
		else if (Buffer[0] == '\\' && Buffer[1] == '\\') // Network path, or pipe or something
		{
			bStartsWithDoubleBackslash = true;
		}
		else if (Buffer[1] != ':') // If not absolute, add current dir
		{
			TCHAR* CopyFrom = Buffer;
			if (CopyFrom[0] == '\\')
			{
				++CopyFrom;
				--CharLen;
			}
			
			ensureMsgf(WorkingDir && *WorkingDir, TEXT("No working dir provided but path is relative (%s)"), Buffer);
			
			TCHAR Temp2[1024];
			ensureMsgf(WorkingDirCharLen + CharLen < SIZEOF_ARRAY(Temp2), TEXT("%llu + %llu < %llu"), WorkingDirCharLen, CharLen, SIZEOF_ARRAY(Temp2));
			memcpy(Temp2, WorkingDir, WorkingDirCharLen*sizeof(TCHAR));
			memcpy(Temp2 + WorkingDirCharLen, CopyFrom, CharLen*sizeof(TCHAR));
			CharLen += WorkingDirCharLen;
			ensureMsgf(CharLen+1 <= SMaxPathLength, TEXT("%llu+1 <= %llu"), CharLen, SMaxPathLength);
			memcpy(Buffer, Temp2, (CharLen+1)*sizeof(TCHAR));
		}
		else if (LastChar == '.' && CharLen == 4) // X:.  .. this expands to X:\ unless working dir matches drive, then it becomes working dir
		{
			check(WorkingDir && *WorkingDir);
			if (FChar::ToLower(InFileName[0]) == FChar::ToLower(WorkingDir[0]))
			{
				memcpy(Buffer, WorkingDir, WorkingDirCharLen * sizeof(TCHAR));
				Buffer[WorkingDirCharLen - 1] = 0;
				CharLen = WorkingDirCharLen;
			}
			else
			{
				--CharLen; // Turn it to X:
			}
		}
	
		if (bHasDotDot || bHasDotBeforeSlash) // Clean up \..\ and such
		{
			Write = Buffer;
			if (bStartsWithDoubleBackslash)
				Write += 2;
			Read = Write;
		
			TCHAR* Folders[128];
			uint32 FolderCount = 0;
			TCHAR LastLastLastChar = 0;
			LastLastChar = 0;
			LastChar = 0;
		
			while (true)
			{
				TCHAR c = *Read;
				if (c == '\\' || c == 0)
				{
					if (LastChar == '.' && LastLastChar == '.' && LastLastLastChar == '\\')
					{
						if (FolderCount > 1)
							--FolderCount;
						Write = Folders[FolderCount - 1];
					}
					else if (LastChar == '.' && LastLastChar == '\\')
					{
						check(FolderCount > 0);
						if (FolderCount > 0)
							Write = Folders[FolderCount - 1];
					}
					else if (LastChar == '\\')
					{
						--Write;
					}
					else
						Folders[FolderCount++] = Write;
					if (c == 0)
						break;
				}
				LastLastLastChar = LastLastChar;
				LastLastChar = LastChar;
				LastChar = c;
				*Write = *Read;
				Read++;
				Write++;
			}
		
			check(Write);
			if (Write)
				*Write = 0;
		
			CharLen = static_cast<uint32>(Write - Buffer + 1);
		}
	
		if (Buffer[CharLen - 2] == '\\')
		{
			--CharLen;
			Buffer[CharLen-1] = 0;
		}
		else if (CharLen == 3) // If it is only <drive>:\ we re-add the last backslash
		{
			Buffer[2] = '\\';
			Buffer[3] = 0;
			++CharLen;
		}
		check(CharLen <= SMaxPathLength);
		BufferCharLen = static_cast<uint32>(CharLen - 1); // Remove terminator
		OutPath.Reserve(BufferCharLen);
		return true;
#else
		StringBuffer<MaxPath> tmp;
		if (InFileName[0] == '~')
		{
			const char* homeDir = getenv("HOME");
			tmp.Append(homeDir).EnsureEndsWithSlash().Append(InFileName + 1);
			InFileName = tmp.data;
		}
		u64 memPos = 0;
		if (InFileName[0] != '/')
		{
			UBA_ASSERTF(InWorkingDir && *InWorkingDir, "Need InWorkingDir to fix path %s", InFileName);
			memcpy(buffer, InWorkingDir, InWorkingDirCharLen);
			memPos = InWorkingDirCharLen;
		}
		else
		{
			while (InFileName[1] == '/')
			{
				++InFileName;
			}
		}
		u64 len = strlen(InFileName);
		memcpy(buffer + memPos, InFileName, len);
		memPos += len;
		buffer[memPos] = 0;
		//printf("PATH: %s\n", buffer);
		//fflush(stdout);

		{
			char* write = buffer;
			char* read = write;
			char* folders[128];
			u32 folderCount = 0;
			char lastLastLastChar = 0;
			char lastLastChar = 0;
			char lastChar = 0;
			while (true)
			{
				tchar c = *read;
				if (c == '/' || c == 0)
				{
					if (lastChar == '.' && lastLastChar == '.' && lastLastLastChar == '/')
					{
						if (folderCount > 1)
							--folderCount;
						write = folders[folderCount - 1];
					}
					else if (lastChar == '.' && lastLastChar == '/')
					{
						//folderCount -= 1;
						write = folders[folderCount - 1];
					}
					else if (lastChar == '/')
					{
						--write;
					}
					else
						folders[folderCount++] = write;
					if (c == 0)
						break;
				}
				lastLastLastChar = lastLastChar;
				lastLastChar = lastChar;
				lastChar = c;
				*write = *read;
				read++;
				write++;
			}
			*write = 0;
			memPos = u32(write - buffer);
		}
	
		if (memPos == 0) // If it is only drive '/' we re-add the last slash
			{
			buffer[0] = '/';
			buffer[1] = 0;
			memPos = 1;
			}
	
		buffer[memPos] = 0;
		if (outBufferCharLen)
			*outBufferCharLen = memPos;
	
		return true;
#endif
	}
}