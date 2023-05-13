#include "ApplicationCore/Event/MouseEvent.h"
#include "ApplicationCore/Event/Event.h"

GCMouseButtonPressedEvent GCMouseButtonPressedEvent_Create(const GCMouseButtonCode MouseButtonCode)
{
	GCMouseButtonPressedEvent Event;
	Event.EventType = GCEventType_MouseButtonPressed;
	Event.MouseButtonCode = MouseButtonCode;

	return Event;
}

GCMouseButtonReleasedEvent GCMouseButtonReleasedEvent_Create(const GCMouseButtonCode MouseButtonCode)
{
	GCMouseButtonReleasedEvent Event;
	Event.EventType = GCEventType_MouseButtonReleased;
	Event.MouseButtonCode = MouseButtonCode;

	return Event;
}

GCMouseMovedEvent GCMouseMovedEvent_Create(const float X, const float Y)
{
	GCMouseMovedEvent Event;
	Event.EventType = GCEventType_MouseMoved;
	Event.X = X;
	Event.Y = Y;

	return Event;
}

GCMouseScrolledEvent GCMouseScrolledEvent_Create(const float XOffset, const float YOffset)
{
	GCMouseScrolledEvent Event;
	Event.EventType = GCEventType_MouseScrolled;
	Event.XOffset = XOffset;
	Event.YOffset = YOffset;

	return Event;
}
