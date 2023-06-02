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

#ifndef GC_APPLICATION_CORE_GENERIC_PLATFORM_WINDOW_H
#define GC_APPLICATION_CORE_GENERIC_PLATFORM_WINDOW_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCPlatformWindow GCWindow;
	typedef struct GCEvent GCEvent;

	typedef void(*GCWindowEventCallbackFunction)(GCWindow* const, GCEvent* const);

	typedef struct GCWindowProperties
	{
		const char* Title;
		uint32_t Width, Height;
		GCWindowEventCallbackFunction EventCallback;
	} GCWindowProperties;

	GCWindow* GCWindow_Create(const GCWindowProperties* const Properties);
	void GCWindow_ProcessEvents(GCWindow* const Window);
	void GCWindow_GetWindowSize(const GCWindow* const Window, uint32_t* const Width, uint32_t* const Height);
	void GCWindow_Destroy(GCWindow* Window);

#ifdef __cplusplus
}
#endif

#endif