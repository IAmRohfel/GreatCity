#ifndef GC_APPLICATION_CORE_GENERIC_PLATFORM_WINDOW_H
#define GC_APPLICATION_CORE_GENERIC_PLATFORM_WINDOW_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCPlatformWindow GCWindow;
	typedef struct GCEvent GCEvent;

	typedef void(*GCWindowEventCallbackFunction)(GCWindow* const, GCEvent* const);

	typedef struct GCWindowProperties
	{
		const char* Title;
		uint32_t Width, Height;
		GCWindowEventCallbackFunction EventCallback;
	} GCWindowProperties;

	GCWindow* GCWindow_Create(const GCWindowProperties* const Properties);
	void GCWindow_ProcessEvents(GCWindow* const Window);
	void GCWindow_Destroy(GCWindow* Window);

#ifdef __cplusplus
}
#endif

#endif