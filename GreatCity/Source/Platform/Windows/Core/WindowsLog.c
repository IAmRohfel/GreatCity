#include "Core/Log.h"

#include <Windows.h>

void GCLog_ChangeConsoleColor(const GCLogLevel Level)
{
	HANDLE OutputHandle = NULL;
	WORD ConsoleColor = 0;

	if (Level < GCLogLevel_Error)
	{
		OutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	else
	{
		OutputHandle = GetStdHandle(STD_ERROR_HANDLE);
	}

	switch (Level)
	{
		case GCLogLevel_Trace:
		{
			ConsoleColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

			break;
		}
		case GCLogLevel_Information:
		{
			ConsoleColor = FOREGROUND_GREEN;

			break;
		}
		case GCLogLevel_Warning:
		{
			ConsoleColor = FOREGROUND_RED | FOREGROUND_GREEN;

			break;
		}
		case GCLogLevel_Error:
		{
			ConsoleColor = FOREGROUND_RED;

			break;
		}
		case GCLogLevel_Fatal:
		{
			ConsoleColor = BACKGROUND_RED;

			break;
		}
	}

	SetConsoleTextAttribute(OutputHandle, ConsoleColor);
}