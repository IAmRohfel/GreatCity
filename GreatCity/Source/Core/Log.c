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

#define _CRT_SECURE_NO_WARNINGS
#include "Core/Log.h"
#include "Core/Memory/Allocator.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void GCLog_LogToConsole(const GCLogLevel Level, const char* const Format, ...)
{
    FILE* OutputHandle = NULL;

    if (Level < GCLogLevel_Error)
    {
        OutputHandle = stdout;
    }
    else
    {
        OutputHandle = stderr;
    }

    GCLog_ChangeConsoleColor(Level);

    va_list Arguments;
    va_start(Arguments, Format);

    char* FormatNewLine = (char*)GCMemory_Allocate((strlen(Format) + 2) * sizeof(char));
    strcpy(FormatNewLine, Format);
    strcat(FormatNewLine, "\n");

    vfprintf(OutputHandle, FormatNewLine, Arguments);

    GCMemory_Free(FormatNewLine);

    va_end(Arguments);
}
