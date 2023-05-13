#ifndef GC_APPLICATION_CORE_EVENT_EVENT_H
#define GC_APPLICATION_CORE_EVENT_EVENT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum GCEventType
	{
		GCEventType_WindowClosed,
		GCEventType_WindowResized,
		GCEventType_KeyPressed,
		GCEventType_KeyReleased,
		GCEventType_MouseButtonPressed,
		GCEventType_MouseButtonReleased,
		GCEventType_MouseMoved,
		GCEventType_MouseScrolled
	} GCEventType;

	typedef struct GCEvent
	{
		GCEventType EventType;
		const void* EventDetail;
		bool IsHandled;
	} GCEvent;

	typedef bool(*GCEventCallbackFunction)(GCEvent* const, void*);

	GCEvent GCEvent_Create(const GCEventType EventType, const void* const EventDetail);
	void GCEvent_Dispatch(const GCEventType EventType, GCEvent* const Event, const GCEventCallbackFunction Callback, void* CustomData);

#ifdef __cplusplus
}
#endif

#endif