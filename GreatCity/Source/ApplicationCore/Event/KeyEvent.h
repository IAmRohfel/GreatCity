#ifndef GC_APPLICATION_CORE_EVENT_KEY_EVENT_H
#define GC_APPLICATION_CORE_EVENT_KEY_EVENT_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum GCEventType GCEventType;
	typedef enum GCKeyCode GCKeyCode;

	typedef struct GCKeyEvent
	{
		GCEventType EventType;
		GCKeyCode KeyCode;
	} GCKeyPressedEvent, GCKeyReleasedEvent;

	GCKeyPressedEvent GCKeyPressedEvent_Create(const GCKeyCode KeyCode);
	GCKeyReleasedEvent GCKeyReleasedEvent_Create(const GCKeyCode KeyCode);

#ifdef __cplusplus
}
#endif

#endif