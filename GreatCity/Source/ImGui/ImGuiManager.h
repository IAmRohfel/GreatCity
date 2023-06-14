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

#ifndef GC_IMGUI_IMGUI_MANAGER_H
#define GC_IMGUI_IMGUI_MANAGER_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCEvent GCEvent;

	void GCImGuiManager_Initialize(void);
	void GCImGuiManager_BeginFrame(void);
	void GCImGuiManager_RenderUI(void);
	void GCImGuiManager_EndFrame(void);
	void GCImGuiManager_Render(void);
	void GCImGuiManager_OnEvent(GCEvent* const Event);
	void GCImGuiManager_Update(void);
	void GCImGuiManager_Terminate(void);

#ifdef __cplusplus
}
#endif

#endif