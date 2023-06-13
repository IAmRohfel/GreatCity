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
#include "ApplicationCore/GenericPlatform/Input.h"
#include "ApplicationCore/Event/Event.h"
#include "ApplicationCore/Application.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererCommandList.h"
#include "Renderer/RendererFramebuffer.h"
#include "Scene/Camera/WorldCamera.h"
#include "Math/Vector2.h"
#include "Core/Log.h"

#include <cstdint>

#include <imgui.h>

extern "C" void GCImGuiManager_InitializePlatform(void);
extern "C" void GCImGuiManager_TerminatePlatform(void);
extern "C" void GCImGuiManager_InitializeRenderer(void);
extern "C" void GCImGuiManager_BeginFramePlatform(void);
extern "C" void GCImGuiManager_BeginFrameRenderer(void);
extern "C" void* GCImGuiManager_GetTexturePlatform(void);
extern "C" void GCImGuiManager_RenderDrawData(void);
extern "C" void GCImGuiManager_TerminateRenderer(void);

static void GCImGuiManager_SetDarkTheme(void);
static void GCImGuiManager_ResizeAttachment(void);

static GCVector2 ViewportSize{};
static GCVector2 ViewportBounds[2]{};

extern "C" void GCImGuiManager_Initialize(void)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& IO = ImGui::GetIO();
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	IO.Fonts->AddFontFromFileTTF("Assets/Fonts/OpenSans/OpenSans-Regular.ttf", 18.0f);

	GCImGuiManager_SetDarkTheme();

	GCImGuiManager_InitializePlatform();
	GCImGuiManager_InitializeRenderer();

	GCRendererCommandList_SetAttachmentResizeCallback(GCRenderer_GetCommandList(), GCImGuiManager_ResizeAttachment);
}

extern "C" void GCImGuiManager_BeginFrame(void)
{
	GCImGuiManager_BeginFrameRenderer();
	GCImGuiManager_BeginFramePlatform();

	ImGui::NewFrame();

	uint32_t FramebufferWidth = 0, FramebufferHeight = 0;
	GCRendererFramebuffer_GetSize(GCRenderer_GetFramebuffer(), &FramebufferWidth, &FramebufferHeight);

	if (ViewportSize.X > 0.0f && ViewportSize.Y > 0.0f && (ViewportSize.X != FramebufferWidth || ViewportSize.Y != FramebufferHeight))
	{
		GCRendererCommandList_ShouldAttachmentResize(GCRenderer_GetCommandList(), true);
	}

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
		ImGuiID DockspaceID = ImGui::GetID("GreatCityDockspace");
		ImGui::DockSpace(DockspaceID, ImVec2{ 0.0f, 0.0f }, DockspaceFlags);
	}

	Style.WindowMinSize.x = MinimumWindowSizeX;

	ImGui::End();
}

extern "C" void GCImGuiManager_EndFrame(void)
{
	const bool IsAltPressed = GCInput_IsKeyPressed(GCKeyCode_LeftAlt);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
	ImGui::Begin("Viewport", nullptr, IsAltPressed ? ImGuiWindowFlags_NoMove : ImGuiWindowFlags_None);

	const ImVec2 ViewportMinimumRegion = ImGui::GetWindowContentRegionMin();
	const ImVec2 ViewportMaximumRegion = ImGui::GetWindowContentRegionMax();
	const ImVec2 ViewportOffset = ImGui::GetWindowPos();

	ViewportBounds[0] = { ViewportMinimumRegion.x + ViewportOffset.x, ViewportMinimumRegion.y + ViewportOffset.y };
	ViewportBounds[1] = { ViewportMaximumRegion.x + ViewportOffset.x, ViewportMaximumRegion.y + ViewportOffset.y };
	
	ImGui::Image(static_cast<ImTextureID>(GCImGuiManager_GetTexturePlatform()), ImVec2{ ViewportSize.X, ViewportSize.Y });

	ImGui::End();
	ImGui::PopStyleVar();
}

extern "C" void GCImGuiManager_Render(void)
{
	ImGui::Render();
	GCImGuiManager_RenderDrawData();
}

extern "C" void GCImGuiManager_Update(void)
{
	ImVec2 MousePosition{ ImGui::GetMousePos() };
	MousePosition.x -= ViewportBounds[0].X;
	MousePosition.y -= ViewportBounds[0].Y;

	ViewportSize = GCVector2_Subtract(ViewportBounds[1], ViewportBounds[0]);

	const std::int32_t MouseX = static_cast<std::int32_t>(MousePosition.x);
	const std::int32_t MouseY = static_cast<std::int32_t>(MousePosition.y);

	uint32_t FramebufferWidth = 0, FramebufferHeight = 0;
	GCRendererFramebuffer_GetSize(GCRenderer_GetFramebuffer(), &FramebufferWidth, &FramebufferHeight);

	if (ViewportSize.X == FramebufferWidth && ViewportSize.Y == FramebufferHeight)
	{
		if (MouseX >= 0 && MouseY >= 0 && MouseX < static_cast<std::int32_t>(ViewportSize.X) && MouseY < static_cast<std::int32_t>(ViewportSize.Y))
		{
			const int32_t Pixel = GCRendererFramebuffer_GetPixel(GCRenderer_GetFramebuffer(), GCRenderer_GetCommandList(), 1, MouseX, MouseY);

			(void)Pixel;
		}
	}
}

extern "C" void GCImGuiManager_Terminate(void)
{
	GCImGuiManager_TerminateRenderer();
	GCImGuiManager_TerminatePlatform();

	ImGui::DestroyContext();
}

void GCImGuiManager_SetDarkTheme(void)
{
	ImGuiStyle& Style = ImGui::GetStyle();

	Style.Colors[ImGuiCol_DockingPreview] = ImVec4{ 0.054f, 0.647f, 0.913f, 1.0f };

	Style.Colors[ImGuiCol_Header] = ImVec4{ 0.121f, 0.160f, 0.215f, 1.0f };
	Style.Colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.215f, 0.254f, 0.317f, 1.0f };
	Style.Colors[ImGuiCol_HeaderActive] = ImVec4{ 0.066f, 0.094f, 0.152f, 1.0f };

	Style.Colors[ImGuiCol_Button] = ImVec4{ 0.215f, 0.254f, 0.317f, 1.0f };
	Style.Colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.294f, 0.333f, 0.388f, 1.0f };
	Style.Colors[ImGuiCol_ButtonActive] = ImVec4{ 0.121f, 0.160f, 0.215f, 1.0f };

	Style.Colors[ImGuiCol_FrameBg] = ImVec4{ 0.121f, 0.160f, 0.215f, 1.0f };
	Style.Colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.215f, 0.254f, 0.317f, 1.0f };
	Style.Colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.066f, 0.094f, 0.152f, 1.0f };

	Style.Colors[ImGuiCol_Tab] = ImVec4{ 0.215f, 0.254f, 0.317f, 1.0f };
	Style.Colors[ImGuiCol_TabHovered] = ImVec4{ 0.294f, 0.333f, 0.388f, 1.0f };
	Style.Colors[ImGuiCol_TabActive] = ImVec4{ 0.121f, 0.160f, 0.215f, 1.0f };
	Style.Colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.121f, 0.160f, 0.215f, 1.0f };
	Style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.066f, 0.094f, 0.152f, 1.0f };

	Style.Colors[ImGuiCol_TitleBg] = ImVec4{ 0.121f, 0.160f, 0.215f, 1.0f };
	Style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.121f, 0.160f, 0.215f, 1.0f };
	Style.Colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.121f, 0.160f, 0.215f, 1.0f };
}

void GCImGuiManager_ResizeAttachment(void)
{
	GCRendererFramebuffer_RecreateAttachmentFramebuffer(GCRenderer_GetFramebuffer(), static_cast<uint32_t>(ViewportSize.X), static_cast<uint32_t>(ViewportSize.Y));
	GCWorldCamera_SetSize(GCApplication_GetWorldCamera(), static_cast<uint32_t>(ViewportSize.X), static_cast<uint32_t>(ViewportSize.Y));
}