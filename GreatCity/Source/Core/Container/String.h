#ifndef GC_CORE_CONTAINER_STRING_H
#define GC_CORE_CONTAINER_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

	wchar_t* GCString_UTF8ToUTF16(const char* const UTF8String);
	char* GCString_UTF16ToUTF8(const wchar_t* const UTF16String);

#ifdef __cplusplus
}
#endif

#endif