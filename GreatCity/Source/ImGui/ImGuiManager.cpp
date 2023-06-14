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

// clang-format off
#include <imgui.h>
#include <ImGuizmo.h>
// clang-format on

extern "C" void GCImGuiManager_RenderDrawData(void);

static void GCImGuiManager_SetDarkTheme(void);

extern "C" void GCImGuiManager_Initialize(void)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &IO = ImGui::GetIO();
    IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    IO.Fonts->AddFontFromFileTTF("Assets/Fonts/OpenSans/OpenSans-Regular.ttf", 18.0f);

    GCImGuiManager_SetDarkTheme();

    ImGuizmo::Style &ImGuizmoStyle = ImGuizmo::GetStyle();

    ImGuizmoStyle.TranslationLineThickness = 5.0f;
    ImGuizmoStyle.ScaleLineThickness = 5.0f;
    ImGuizmoStyle.RotationLineThickness = 5.0f;
    ImGuizmoStyle.RotationOuterLineThickness = 5.0f;
    ImGuizmoStyle.TranslationLineArrowSize = 5.0f;
}

extern "C" void GCImGuiManager_Render(void)
{
    ImGui::Render();
    GCImGuiManager_RenderDrawData();
}

extern "C" void GCImGuiManager_Terminate(void)
{
    ImGui::DestroyContext();
}

void GCImGuiManager_SetDarkTheme(void)
{
    ImGuiStyle &Style = ImGui::GetStyle();

    Style.Colors[ImGuiCol_DockingPreview] = ImVec4{0.054f, 0.647f, 0.913f, 1.0f};

    Style.Colors[ImGuiCol_Header] = ImVec4{0.121f, 0.160f, 0.215f, 1.0f};
    Style.Colors[ImGuiCol_HeaderHovered] = ImVec4{0.215f, 0.254f, 0.317f, 1.0f};
    Style.Colors[ImGuiCol_HeaderActive] = ImVec4{0.066f, 0.094f, 0.152f, 1.0f};

    Style.Colors[ImGuiCol_Button] = ImVec4{0.215f, 0.254f, 0.317f, 1.0f};
    Style.Colors[ImGuiCol_ButtonHovered] = ImVec4{0.294f, 0.333f, 0.388f, 1.0f};
    Style.Colors[ImGuiCol_ButtonActive] = ImVec4{0.121f, 0.160f, 0.215f, 1.0f};

    Style.Colors[ImGuiCol_FrameBg] = ImVec4{0.121f, 0.160f, 0.215f, 1.0f};
    Style.Colors[ImGuiCol_FrameBgHovered] = ImVec4{0.215f, 0.254f, 0.317f, 1.0f};
    Style.Colors[ImGuiCol_FrameBgActive] = ImVec4{0.066f, 0.094f, 0.152f, 1.0f};

    Style.Colors[ImGuiCol_Tab] = ImVec4{0.215f, 0.254f, 0.317f, 1.0f};
    Style.Colors[ImGuiCol_TabHovered] = ImVec4{0.294f, 0.333f, 0.388f, 1.0f};
    Style.Colors[ImGuiCol_TabActive] = ImVec4{0.121f, 0.160f, 0.215f, 1.0f};
    Style.Colors[ImGuiCol_TabUnfocused] = ImVec4{0.121f, 0.160f, 0.215f, 1.0f};
    Style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.066f, 0.094f, 0.152f, 1.0f};

    Style.Colors[ImGuiCol_TitleBg] = ImVec4{0.121f, 0.160f, 0.215f, 1.0f};
    Style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.121f, 0.160f, 0.215f, 1.0f};
    Style.Colors[ImGuiCol_TitleBgActive] = ImVec4{0.121f, 0.160f, 0.215f, 1.0f};
}