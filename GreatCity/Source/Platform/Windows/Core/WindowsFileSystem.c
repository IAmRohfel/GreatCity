#include "Core/FileSystem.h"
#include "Core/Memory/Allocator.h"
#include "Core/Container/String.h"

#include <string.h>
#include <stdbool.h>

#include <Windows.h>
#include <Shlwapi.h>

bool GCFileSystem_Exists(const char* const Path)
{
	wchar_t* PathUTF16 = GCString_UTF8ToUTF16(Path);
	const DWORD Attribute = GetFileAttributesW(PathUTF16);
	GCMemory_Free(PathUTF16);

	return Attribute != INVALID_FILE_ATTRIBUTES;
}

char* GCFileSystem_GetFileName(const char* const Path)
{
	wchar_t* PathUTF16 = GCString_UTF8ToUTF16(Path);
	LPWSTR FileNameUTF16 = PathFindFileNameW(PathUTF16);

	char* FileNameUTF8 = GCString_UTF16ToUTF8(FileNameUTF16);

	GCMemory_Free(PathUTF16);

	return FileNameUTF8;
}

void GCFileSystem_CreateDirectories(const char* const Path)
{
	char* ThePath = (char*)GCMemory_AllocateZero((strlen(Path) + 1) * sizeof(char));

	for (uint32_t Counter = 0; Counter < strlen(Path) + 1; Counter++)
	{
		ThePath[Counter] = Path[Counter];

		if (ThePath[Counter] == '/' || ThePath[Counter] == '\\')
		{
			wchar_t* ThePathUTF16 = GCString_UTF8ToUTF16(ThePath);
			CreateDirectoryW(ThePathUTF16, NULL);
			GCMemory_Free(ThePathUTF16);
		}
	}

	GCMemory_Free(ThePath);
}

GCFileSystemFileAttributes GCFileSystem_GetFileAttributes(const char* const Path)
{
	wchar_t* PathUTF16 = GCString_UTF8ToUTF16(Path);
	WIN32_FILE_ATTRIBUTE_DATA FileAttributeData = { 0 };
	GetFileAttributesExW(PathUTF16, GetFileExInfoStandard, &FileAttributeData);
	GCMemory_Free(PathUTF16);

	GCFileSystemFileAttributes FileAttributes = { 0 };

	SYSTEMTIME LastWriteSystemTime = { 0 };
	FileTimeToSystemTime(&FileAttributeData.ftLastWriteTime, &LastWriteSystemTime);

	FileAttributes.LastWriteTime = *(GCFileSystemFileTime*)&LastWriteSystemTime;

	return FileAttributes;
}

bool GCFileSystemFileTime_IsNewer(const GCFileSystemFileTime FileTime1, const GCFileSystemFileTime FileTime2)
{
	FILETIME TheFileTime1 = { 0 };
	SystemTimeToFileTime((const SYSTEMTIME*)&FileTime1, &TheFileTime1);
	FILETIME TheFileTime2 = { 0 };
	SystemTimeToFileTime((const SYSTEMTIME*)&FileTime2, &TheFileTime2);

	return CompareFileTime(&TheFileTime1, &TheFileTime2) == 1;
}

bool GCFileSystemFileTime_IsEqual(const GCFileSystemFileTime FileTime1, const GCFileSystemFileTime FileTime2)
{
	FILETIME TheFileTime1 = { 0 };
	SystemTimeToFileTime((const SYSTEMTIME*)&FileTime1, &TheFileTime1);
	FILETIME TheFileTime2 = { 0 };
	SystemTimeToFileTime((const SYSTEMTIME*)&FileTime2, &TheFileTime2);

	return CompareFileTime(&TheFileTime1, &TheFileTime2) == 0;
}

bool GCFileSystemFileTime_IsOlder(const GCFileSystemFileTime FileTime1, const GCFileSystemFileTime FileTime2)
{
	FILETIME TheFileTime1 = { 0 };
	SystemTimeToFileTime((const SYSTEMTIME*)&FileTime1, &TheFileTime1);
	FILETIME TheFileTime2 = { 0 };
	SystemTimeToFileTime((const SYSTEMTIME*)&FileTime2, &TheFileTime2);

	return CompareFileTime(&TheFileTime1, &TheFileTime2) == -1;
}