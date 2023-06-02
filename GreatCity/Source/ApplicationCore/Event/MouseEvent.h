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