#include "ApplicationCore/Event/KeyEvent.h"
#include "ApplicationCore/Event/Event.h"

GCKeyPressedEvent GCKeyPressedEvent_Create(const GCKeyCode KeyCode)
{
	GCKeyPressedEvent Event;
	Event.EventType = GCEventType_KeyPressed;
	Event.KeyCode = KeyCode;

	return Event;
}

GCKeyReleasedEvent GCKeyReleasedEvent_Create(const GCKeyCode KeyCode)
{
	GCKeyReleasedEvent Event;
	Event.EventType = GCEventType_KeyReleased;
	Event.KeyCode = KeyCode;

	return Event;
}
