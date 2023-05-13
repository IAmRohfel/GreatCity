#include "ApplicationCore/Event/ApplicationEvent.h"
#include "ApplicationCore/Event/Event.h"

GCWindowClosedEvent GCWindowClosedEvent_Create(void)
{
	GCWindowClosedEvent Event;
	Event.EventType = GCEventType_WindowClosed;

	return Event;
}

GCWindowResizedEvent GCWindowResizedEvent_Create(const uint32_t Width, const uint32_t Height)
{
	GCWindowResizedEvent Event;
	Event.EventType = GCEventType_WindowResized;
	Event.Width = Width;
	Event.Height = Height;

	return Event;
}
