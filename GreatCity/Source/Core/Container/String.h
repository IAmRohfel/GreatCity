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

#ifndef GC_CORE_CONTAINER_STRING_H
#define GC_CORE_CONTAINER_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    wchar_t *GCString_UTF8ToUTF16(const char *const UTF8String);
    char *GCString_UTF16ToUTF8(const wchar_t *const UTF16String);

#ifdef __cplusplus
}
#endif

#endif