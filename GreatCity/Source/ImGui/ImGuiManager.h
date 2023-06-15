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

    typedef struct GCRendererTexture2D GCRendererTexture2D;

    void GCImGuiManager_Initialize(void);
    void GCImGuiManager_Render(void);
    void GCImGuiManager_Terminate(void);

    void GCImGuiManager_InitializePlatform(void);
    void GCImGuiManager_TerminatePlatform(void);
    void GCImGuiManager_InitializeRenderer(void);
    void* GCImGuiManager_AddTexture(const GCRendererTexture2D* const Texture2D);
    void GCImGuiManager_BeginFramePlatform(void);
    void GCImGuiManager_BeginFrameRenderer(void);
    void* GCImGuiManager_GetTexturePlatform(void);
    void GCImGuiManager_TerminateRenderer(void);

#ifdef __cplusplus
}
#endif

#endif