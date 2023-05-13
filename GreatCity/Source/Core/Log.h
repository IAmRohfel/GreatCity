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