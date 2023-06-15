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
#include "Core/Memory/Allocator.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <Windows.h>

wchar_t* GCString_UTF8ToUTF16(const char* const UTF8String)
{
    const int32_t UTF16StringLength =
        MultiByteToWideChar(CP_UTF8, 0, UTF8String, (int32_t)strlen(UTF8String) + 1, NULL, 0);

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
    const int32_t UTF8StringLength =
        WideCharToMultiByte(CP_UTF8, 0, UTF16String, (int32_t)wcslen(UTF16String) + 1, NULL, 0, NULL, NULL);

    if (UTF8StringLength > 0)
    {
        char* UTF8String = (char*)GCMemory_Allocate(UTF8StringLength * sizeof(char));
        WideCharToMultiByte(CP_UTF8, 0, UTF16String, (int32_t)wcslen(UTF16String) + 1, UTF8String, UTF8StringLength,
                            NULL, NULL);

        return UTF8String;
    }

    return NULL;
}
