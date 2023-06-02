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

#ifndef GC_CORE_FILE_SYSTEM_H
#define GC_CORE_FILE_SYSTEM_H

#include <stdbool.h>

#ifdef GC_PLATFORM_WINDOWS
	#include <Windows.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCFileSystemFileTime
	{
#ifdef GC_PLATFORM_WINDOWS
		WORD Year;
		WORD Month;
		WORD DayOfWeek;
		WORD Day;
		WORD Hour;
		WORD Minute;
		WORD Second;
		WORD Milliseconds;
#endif
	} GCFileSystemFileTime;

	typedef struct GCFileSystemFileAttributes
	{
		GCFileSystemFileTime LastWriteTime;
	} GCFileSystemFileAttributes;

	bool GCFileSystem_Exists(const char* const Path);
	char* GCFileSystem_GetFileName(const char* const Path);
	void GCFileSystem_CreateDirectories(const char* const Path);

	GCFileSystemFileAttributes GCFileSystem_GetFileAttributes(const char* const Path);

	bool GCFileSystemFileTime_IsNewer(const GCFileSystemFileTime FileTime1, const GCFileSystemFileTime FileTime2);
	bool GCFileSystemFileTime_IsEqual(const GCFileSystemFileTime FileTime1, const GCFileSystemFileTime FileTime2);
	bool GCFileSystemFileTime_IsOlder(const GCFileSystemFileTime FileTime1, const GCFileSystemFileTime FileTime2);

#ifdef __cplusplus
}
#endif

#endif