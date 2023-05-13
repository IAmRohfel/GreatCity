#include "ApplicationCore/Event/Event.h"

#include <stdbool.h>

GCEvent GCEvent_Create(const GCEventType EventType, const void* const EventDetail)
{
	GCEvent Event;
	Event.EventType = EventType;
	Event.EventDetail = EventDetail;
	Event.IsHandled = false;

	return Event;
}

void GCEvent_Dispatch(const GCEventType EventType, GCEvent* const Event, const GCEventCallbackFunction Callback, void* CustomData)
{
	if (EventType == Event->EventType)
	{
		if (Callback && !Event->IsHandled)
		{
			Event->IsHandled = Callback(Event, CustomData);
		}
	}
}
