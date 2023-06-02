/*
	Copyright (C) 2023  Rohfel Adyaraka Christianugrah Puspoasmoro

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
