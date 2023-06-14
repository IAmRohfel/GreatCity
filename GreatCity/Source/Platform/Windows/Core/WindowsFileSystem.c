/*
    Copyright (C) 2023  Rohfel Adyaraka Christianugrah Puspoasmoro

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Core/Container/String.h"
#include "Core/FileSystem.h"
#include "Core/Memory/Allocator.h"

#include <stdbool.h>
#include <string.h>

#include <Shlwapi.h>
#include <Windows.h>

bool GCFileSystem_Exists(const char *const Path)
{
    wchar_t *PathUTF16 = GCString_UTF8ToUTF16(Path);
    const DWORD Attribute = GetFileAttributesW(PathUTF16);
    GCMemory_Free(PathUTF16);

    return Attribute != INVALID_FILE_ATTRIBUTES;
}

char *GCFileSystem_GetFileName(const char *const Path)
{
    wchar_t *PathUTF16 = GCString_UTF8ToUTF16(Path);
    LPWSTR FileNameUTF16 = PathFindFileNameW(PathUTF16);

    char *FileNameUTF8 = GCString_UTF16ToUTF8(FileNameUTF16);

    GCMemory_Free(PathUTF16);

    return FileNameUTF8;
}

void GCFileSystem_CreateDirectories(const char *const Path)
{
    char *ThePath = (char *)GCMemory_AllocateZero((strlen(Path) + 1) * sizeof(char));

    for (uint32_t Counter = 0; Counter < strlen(Path) + 1; Counter++)
    {
        ThePath[Counter] = Path[Counter];

        if (ThePath[Counter] == '/' || ThePath[Counter] == '\\')
        {
            wchar_t *ThePathUTF16 = GCString_UTF8ToUTF16(ThePath);
            CreateDirectoryW(ThePathUTF16, NULL);
            GCMemory_Free(ThePathUTF16);
        }
    }

    GCMemory_Free(ThePath);
}

GCFileSystemFileAttributes GCFileSystem_GetFileAttributes(const char *const Path)
{
    wchar_t *PathUTF16 = GCString_UTF8ToUTF16(Path);
    WIN32_FILE_ATTRIBUTE_DATA FileAttributeData = {0};
    GetFileAttributesExW(PathUTF16, GetFileExInfoStandard, &FileAttributeData);
    GCMemory_Free(PathUTF16);

    GCFileSystemFileAttributes FileAttributes = {0};

    SYSTEMTIME LastWriteSystemTime = {0};
    FileTimeToSystemTime(&FileAttributeData.ftLastWriteTime, &LastWriteSystemTime);

    FileAttributes.LastWriteTime = *(GCFileSystemFileTime *)&LastWriteSystemTime;

    return FileAttributes;
}

bool GCFileSystemFileTime_IsNewer(const GCFileSystemFileTime FileTime1, const GCFileSystemFileTime FileTime2)
{
    FILETIME TheFileTime1 = {0};
    SystemTimeToFileTime((const SYSTEMTIME *)&FileTime1, &TheFileTime1);
    FILETIME TheFileTime2 = {0};
    SystemTimeToFileTime((const SYSTEMTIME *)&FileTime2, &TheFileTime2);

    return CompareFileTime(&TheFileTime1, &TheFileTime2) == 1;
}

bool GCFileSystemFileTime_IsEqual(const GCFileSystemFileTime FileTime1, const GCFileSystemFileTime FileTime2)
{
    FILETIME TheFileTime1 = {0};
    SystemTimeToFileTime((const SYSTEMTIME *)&FileTime1, &TheFileTime1);
    FILETIME TheFileTime2 = {0};
    SystemTimeToFileTime((const SYSTEMTIME *)&FileTime2, &TheFileTime2);

    return CompareFileTime(&TheFileTime1, &TheFileTime2) == 0;
}

bool GCFileSystemFileTime_IsOlder(const GCFileSystemFileTime FileTime1, const GCFileSystemFileTime FileTime2)
{
    FILETIME TheFileTime1 = {0};
    SystemTimeToFileTime((const SYSTEMTIME *)&FileTime1, &TheFileTime1);
    FILETIME TheFileTime2 = {0};
    SystemTimeToFileTime((const SYSTEMTIME *)&FileTime2, &TheFileTime2);

    return CompareFileTime(&TheFileTime1, &TheFileTime2) == -1;
}