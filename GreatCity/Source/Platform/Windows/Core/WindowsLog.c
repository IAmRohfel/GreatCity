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
    case GCLogLevel_Trace: {
        ConsoleColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

        break;
    }
    case GCLogLevel_Information: {
        ConsoleColor = FOREGROUND_GREEN;

        break;
    }
    case GCLogLevel_Warning: {
        ConsoleColor = FOREGROUND_RED | FOREGROUND_GREEN;

        break;
    }
    case GCLogLevel_Error: {
        ConsoleColor = FOREGROUND_RED;

        break;
    }
    case GCLogLevel_Fatal: {
        ConsoleColor = BACKGROUND_RED;

        break;
    }
    }

    SetConsoleTextAttribute(OutputHandle, ConsoleColor);
}