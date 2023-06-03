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

#include "ImGui/ImGuiManager.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererFramebuffer.h"

#include <imgui.h>

extern "C"
{
	extern void GCImGuiManager_InitializePlatform(void);
	extern void GCImGuiManager_TerminatePlatform(void);
	extern void GCImGuiManager_InitializeRenderer(void);
	extern void GCImGuiManager_BeginFramePlatform(void);
	extern void GCImGuiManager_BeginFrameRenderer(void);
	extern void* GCImGuiManager_GetTexturePlatform(void);
	extern void GCImGuiManager_RenderDrawData(void);
	extern void GCImGuiManager_TerminateRenderer(void);

	void GCImGuiManager_Initialize(void)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& IO = ImGui::GetIO();
		IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui::StyleColorsDark();

		GCImGuiManager_InitializePlatform();
		GCImGuiManager_InitializeRenderer();
	}

	void GCImGuiManager_BeginFrame(void)
	{
		GCImGuiManager_BeginFrameRenderer();
		GCImGuiManager_BeginFramePlatform();

		ImGui::NewFrame();

		const bool IsFullscreen = true;

		static bool IsDockspaceOpen = true;
		static ImGuiDockNodeFlags DockspaceFlags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		if (IsFullscreen)
		{
			ImGuiViewport* Viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(Viewport->Pos);
			ImGui::SetNextWindowSize(Viewport->Size);
			ImGui::SetNextWindowViewport(Viewport->ID);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

			WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			WindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		if (DockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
		{
			WindowFlags |= ImGuiWindowFlags_NoBackground;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
		ImGui::Begin("Dockspace", &IsDockspaceOpen, WindowFlags);
		ImGui::PopStyleVar();

		if (IsFullscreen)
		{
			ImGui::PopStyleVar(2);
		}

		ImGuiIO& IO = ImGui::GetIO();
		ImGuiStyle& Style = ImGui::GetStyle();
		
		const float MinimumWindowSizeX = Style.WindowMinSize.x;
		Style.WindowMinSize.x = 370.0f;

		if (IO.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID DockspaceID = ImGui::GetID("MyDockspace");
			ImGui::DockSpace(DockspaceID, ImVec2{ 0.0f, 0.0f }, DockspaceFlags);
		}

		Style.WindowMinSize.x = MinimumWindowSizeX;

		ImGui::End();
	}

	void GCImGuiManager_EndFrame(void)
	{
		GCRendererFramebuffer* const RendererFramebuffer = GCRenderer_GetFramebuffer();
		static ImVec2 ViewportSize = ImVec2{ 0.0f, 0.0f };

		ImGui::Begin("Viewport");

		const ImVec2 CurrentViewportSize = ImGui::GetContentRegionAvail();

		if (ViewportSize.x != CurrentViewportSize.x || ViewportSize.y != CurrentViewportSize.y)
		{
			ViewportSize = CurrentViewportSize;
			GCRendererFramebuffer_RecreateTexture(RendererFramebuffer, static_cast<uint32_t>(ViewportSize.x), static_cast<uint32_t>(ViewportSize.y));
		}

		ImGui::Image(static_cast<ImTextureID>(GCImGuiManager_GetTexturePlatform()), ViewportSize);

		ImGui::End();
	}

	void GCImGuiManager_Render(void)
	{
		ImGui::Render();
		GCImGuiManager_RenderDrawData();
	}

	void GCImGuiManager_Terminate(void)
	{
		GCImGuiManager_TerminateRenderer();
		GCImGuiManager_TerminatePlatform();

		ImGui::DestroyContext();
	}
}