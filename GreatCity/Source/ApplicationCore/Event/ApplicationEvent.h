#ifndef GC_APPLICATION_CORE_EVENT_APPLICATION_EVENT_H
#define GC_APPLICATION_CORE_EVENT_APPLICATION_EVENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum GCEventType GCEventType;

	typedef struct GCWindowClosedEvent
	{
		GCEventType EventType;
	} GCWindowClosedEvent;

	typedef struct GCWindowResizedEvent
	{
		GCEventType EventType;
		uint32_t Width, Height;
	} GCWindowResizedEvent;

	GCWindowClosedEvent GCWindowClosedEvent_Create(void);
	GCWindowResizedEvent GCWindowResizedEvent_Create(const uint32_t Width, const uint32_t Height);

#ifdef __cplusplus
}
#endif

#endif