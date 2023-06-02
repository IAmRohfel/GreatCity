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