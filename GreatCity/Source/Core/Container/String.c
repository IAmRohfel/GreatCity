#include "Core/Container/String.h"
#include "Core/Memory/Allocator.h"

#include <string.h>
#include <stddef.h>
#include <stdint.h>

#include <Windows.h>

wchar_t* GCString_UTF8ToUTF16(const char* const UTF8String)
{
	const int32_t UTF16StringLength = MultiByteToWideChar(CP_UTF8, 0, UTF8String, (int32_t)strlen(UTF8String) + 1, NULL, 0);

	if (UTF16StringLength > 0)
	{
		wchar_t* UTF16String = (wchar_t*)GCMemory_Allocate(UTF16StringLength * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, UTF8String, (int32_t)strlen(UTF8String) + 1, UTF16String, UTF16StringLength);

		return UTF16String;
	}

	return NULL;
}

char* GCString_UTF16ToUTF8(const wchar_t* const UTF16String)
{
	const int32_t UTF8StringLength = WideCharToMultiByte(CP_UTF8, 0, UTF16String, (int32_t)wcslen(UTF16String) + 1, NULL, 0, NULL, NULL);

	if (UTF8StringLength > 0)
	{
		char* UTF8String = (char*)GCMemory_Allocate(UTF8StringLength * sizeof(char));
		WideCharToMultiByte(CP_UTF8, 0, UTF16String, (int32_t)wcslen(UTF16String) + 1, UTF8String, UTF8StringLength, NULL, NULL);
		
		return UTF8String;
	}

	return NULL;
}
