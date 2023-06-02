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
