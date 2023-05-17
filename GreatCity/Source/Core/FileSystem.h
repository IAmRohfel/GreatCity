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