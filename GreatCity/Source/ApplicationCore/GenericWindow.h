#ifndef GC_APPLICATION_CORE_GENERIC_WINDOW_H
#define GC_APPLICATION_CORE_GENERIC_WINDOW_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCPlatformWindow GCGenericWindow;

	typedef struct GCGenericWindowProperties
	{
		const char* Title;
		uint32_t Width, Height;
	} GCGenericWindowProperties;

	GCGenericWindow* GCGenericWindow_Create(const GCGenericWindowProperties* const Properties);
	void GCGenericWindow_ProcessEvents(GCGenericWindow* const Window);
	void GCGenericWindow_Destroy(GCGenericWindow* Window);

#ifdef __cplusplus
}
#endif

#endif