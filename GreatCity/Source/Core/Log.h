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

#ifndef GC_CORE_LOG_H
#define GC_CORE_LOG_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum GCLogLevel
    {
        GCLogLevel_Trace,
        GCLogLevel_Information,
        GCLogLevel_Warning,
        GCLogLevel_Error,
        GCLogLevel_Fatal
    } GCLogLevel;

    void GCLog_LogToConsole(const GCLogLevel Level, const char* const Format, ...);
    void GCLog_ChangeConsoleColor(const GCLogLevel Level);

#define GC_LOG_TRACE(...) GCLog_LogToConsole(GCLogLevel_Trace, __VA_ARGS__)
#define GC_LOG_INFORMATION(...) GCLog_LogToConsole(GCLogLevel_Information, __VA_ARGS__)
#define GC_LOG_WARNING(...) GCLog_LogToConsole(GCLogLevel_Warning, __VA_ARGS__)
#define GC_LOG_ERROR(...) GCLog_LogToConsole(GCLogLevel_Error, __VA_ARGS__)
#define GC_LOG_FATAL(...) GCLog_LogToConsole(GCLogLevel_Fatal, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif