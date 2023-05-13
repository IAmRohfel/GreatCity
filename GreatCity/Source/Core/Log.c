#define _CRT_SECURE_NO_WARNINGS
#include "Core/Log.h"
#include "Core/Memory/Allocator.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

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
