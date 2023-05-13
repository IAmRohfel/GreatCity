#ifndef GC_APPLICATION_CORE_EVENT_MOUSE_EVENT_H
#define GC_APPLICATION_CORE_EVENT_MOUSE_EVENT_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum GCEventType GCEventType;
	typedef enum GCMouseButtonCode GCMouseButtonCode;

	typedef struct GCMouseButtonEvent
	{
		GCEventType EventType;
		GCMouseButtonCode MouseButtonCode;
	} GCMouseButtonPressedEvent, GCMouseButtonReleasedEvent;

	typedef struct GCMouseMovedEvent
	{
		GCEventType EventType;
		float X, Y;
	} GCMouseMovedEvent;

	typedef struct GCMouseScrolledEvent
	{
		GCEventType EventType;
		float XOffset, YOffset;
	} GCMouseScrolledEvent;

	GCMouseButtonPressedEvent GCMouseButtonPressedEvent_Create(const GCMouseButtonCode MouseButtonCode);
	GCMouseButtonReleasedEvent GCMouseButtonReleasedEvent_Create(const GCMouseButtonCode MouseButtonCode);
	GCMouseMovedEvent GCMouseMovedEvent_Create(const float X, const float Y);
	GCMouseScrolledEvent GCMouseScrolledEvent_Create(const float XOffset, const float YOffset);

#ifdef __cplusplus
}
#endif

#endif