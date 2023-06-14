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
