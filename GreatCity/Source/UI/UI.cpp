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

#include "UI/UI.h"
#include "ApplicationCore/Application.h"
#include "ApplicationCore/Event/Event.h"
#include "ApplicationCore/Event/KeyEvent.h"
#include "ApplicationCore/Event/MouseEvent.h"
#include "ApplicationCore/GenericPlatform/Input.h"
#include "ApplicationCore/GenericPlatform/KeyCode.h"
#include "ApplicationCore/GenericPlatform/MouseButtonCode.h"
#include "Core/Memory/Allocator.h"
#include "ImGui/ImGuiManager.h"
#include "Math/Vector2.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererCommandList.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/RendererFramebuffer.h"
#include "Renderer/RendererMesh.h"
#include "Renderer/RendererModel.h"
#include "Renderer/RendererTexture2D.h"
#include "World/Camera/WorldCamera.h"
#include "World/Components.h"
#include "World/Entity.h"
#include "World/World.h"

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// clang-format off
#include <imgui.h>
#include <ImGuizmo.h>
// clang-format on

struct GCUIData
{
  public:
    std::vector<GCEntity> Entities{};
    std::unordered_map<GCRendererModel*, std::pair<GCRendererTexture2D*, void*>> UIEntityData{};

    GCVector2 ViewportSize{};
    GCVector2 ViewportBounds[2]{};
    GCEntity HoveredEntity{};
    GCEntity SelectedEntity{};

    bool IsViewportHovered{};
    bool IsViewportFocused{};

    int32_t GizmoType{-1};
};

static void GCUI_ResizeAttachment(void);
static bool GCUI_OnKeyPressed(GCEvent* const Event, void* CustomData);
static bool GCUI_OnMouseButtonPressed(GCEvent* const Event, void* CustomData);
static GCVector2 GCUI_GetMousePosition(void);

static GCUIData* UIData{};

void GCUI_Initialize(void)
{
    UIData = new GCUIData();

    const GCRendererDevice* const Device = GCRenderer_GetDevice();
    const GCRendererCommandList* const CommandList = GCRenderer_GetCommandList();

    GCRendererCommandList_SetAttachmentResizeCallback(GCRenderer_GetCommandList(), GCUI_ResizeAttachment);

    GCImGuiManager_Initialize();
    GCImGuiManager_InitializePlatform();
    GCImGuiManager_InitializeRenderer();

    const std::array<std::tuple<std::string, std::string, std::string>, 2> ModelLocations = {
        {{"Assets/Models/Buildings/Offices/SmallOffice.obj", "Assets/Models/Buildings/Offices",
          "Assets/Textures/Buildings/Offices/SmallOffice.png"},
         {"Assets/Models/Buildings/Offices/Office.obj", "Assets/Models/Buildings/Offices",
          "Assets/Textures/Buildings/Offices/Office.png"}}};

    for (const std::tuple<std::string, std::string, std::string>& ModelLocation : ModelLocations)
    {
        GCRendererTexture2DDescription TextureDescription = {0};
        TextureDescription.Device = Device;
        TextureDescription.CommandList = CommandList;
        TextureDescription.TexturePath = std::get<2>(ModelLocation).c_str();
        GCRendererTexture2D* Texture = GCRendererTexture2D_Create(&TextureDescription);

        UIData->UIEntityData[GCRendererModel_CreateFromFile(std::get<0>(ModelLocation).c_str(),
                                                            std::get<1>(ModelLocation).c_str())] =
            std::make_pair(Texture, GCImGuiManager_AddTexture(Texture));
    }
}

void GCUI_Render(void)
{
    GCRenderer_BeginImGui();

    GCImGuiManager_BeginFrameRenderer();
    GCImGuiManager_BeginFramePlatform();

    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    uint32_t FramebufferWidth = 0, FramebufferHeight = 0;
    GCRendererFramebuffer_GetSize(GCRenderer_GetFramebuffer(), &FramebufferWidth, &FramebufferHeight);

    if (UIData->ViewportSize.X > 0.0f && UIData->ViewportSize.Y > 0.0f &&
        (UIData->ViewportSize.X != FramebufferWidth || UIData->ViewportSize.Y != FramebufferHeight))
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

        WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove;
        WindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    if (DockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
    {
        WindowFlags |= ImGuiWindowFlags_NoBackground;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});
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
        ImGui::DockSpace(DockspaceID, ImVec2{0.0f, 0.0f}, DockspaceFlags);
    }

    Style.WindowMinSize.x = MinimumWindowSizeX;

    ImGui::End();

    GCWorld* const World = GCApplication_GetWorld();

    bool OpenAddBuildingPopup = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("World"))
        {
            if (ImGui::MenuItem("Add Building"))
            {
                OpenAddBuildingPopup = true;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    const bool IsAltPressed = GCInput_IsKeyPressed(GCKeyCode_LeftAlt);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});
    ImGui::Begin("Viewport", nullptr, IsAltPressed ? ImGuiWindowFlags_NoMove : ImGuiWindowFlags_None);

    if (OpenAddBuildingPopup)
    {
        ImGui::OpenPopup("Add Building");
    }

    ImGui::SetNextWindowPos(ImVec2{UIData->ViewportSize.X / 2.0f, UIData->ViewportSize.Y / 2.0f}, ImGuiCond_Always,
                            ImVec2{0.5f, 0.5f});
    ImGui::SetNextWindowSize(ImVec2{UIData->ViewportSize.X / 1.25f, UIData->ViewportSize.Y / 1.25f}, ImGuiCond_Always);

    if (ImGui::BeginPopupModal("Add Building", nullptr, ImGuiWindowFlags_NoResize))
    {
        GCEntity Entity{};
        bool ClosePopup = false;

        for (const auto& UIEntityData : UIData->UIEntityData)
        {
            if (ImGui::ImageButton(UIEntityData.second.second, ImVec2{100.0f, 100.0f}))
            {
                static uint32_t ModelCount = 0;
                const std::string Name{"Model " + ModelCount};

                Entity = GCWorld_CreateEntity(World, Name.c_str());
                GCEntity_AddMeshComponent(Entity, UIEntityData.first);

                ModelCount++;
                ClosePopup = true;
            }
        }

        if (Entity != 0)
        {
            GCTransformComponent* const TransformComponent = GCEntity_GetTransformComponent(Entity);
            TransformComponent->Translation =
                GCVector3_Create(0.0f, GCEntity_GetTransformComponent(Entity)->Translation.Y - 2.5f, 0.0f);

            UIData->Entities.emplace_back(Entity);
        }

        if (ImGui::Button("Done"))
        {
            ClosePopup = true;
        }

        if (ClosePopup)
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    const ImVec2 ViewportMinimumRegion = ImGui::GetWindowContentRegionMin();
    const ImVec2 ViewportMaximumRegion = ImGui::GetWindowContentRegionMax();
    const ImVec2 ViewportOffset = ImGui::GetWindowPos();

    UIData->ViewportBounds[0] = {ViewportMinimumRegion.x + ViewportOffset.x,
                                 ViewportMinimumRegion.y + ViewportOffset.y};
    UIData->ViewportBounds[1] = {ViewportMaximumRegion.x + ViewportOffset.x,
                                 ViewportMaximumRegion.y + ViewportOffset.y};

    UIData->IsViewportHovered = ImGui::IsWindowHovered();
    UIData->IsViewportFocused = ImGui::IsWindowFocused();

    ImGui::Image(static_cast<ImTextureID>(GCImGuiManager_GetTexturePlatform()),
                 ImVec2{UIData->ViewportSize.X, UIData->ViewportSize.Y});

    if (UIData->SelectedEntity != 0 && UIData->SelectedEntity != GCWorld_GetTerrainEntity(GCApplication_GetWorld()) &&
        UIData->GizmoType != -1)
    {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(UIData->ViewportBounds[0].X, UIData->ViewportBounds[0].Y,
                          UIData->ViewportBounds[1].X - UIData->ViewportBounds[0].X,
                          UIData->ViewportBounds[1].Y - UIData->ViewportBounds[0].Y);

        const GCWorldCamera* const WorldCamera = GCWorld_GetCamera(World);
        const GCMatrix4x4* const WorldCameraViewMatrix = GCWorldCamera_GetViewMatrix(WorldCamera);
        const GCMatrix4x4* const OriginalWorldCameraProjectionMatrix = GCWorldCamera_GetProjectionMatrix(WorldCamera);

        GCMatrix4x4 WorldCameraProjectionMatrix = *OriginalWorldCameraProjectionMatrix;
        WorldCameraProjectionMatrix.Data[1][1] *= -1.0f;

        GCTransformComponent* const EntityTransformComponent = GCEntity_GetTransformComponent(UIData->SelectedEntity);
        GCMatrix4x4 EntityTransform = GCTransformComponent_GetTransform(EntityTransformComponent);

        std::array<float, 3> SnapValues{};

        if (UIData->GizmoType ==
            static_cast<int32_t>(ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Z))
        {
            SnapValues = {1.0f, 1.0f, 1.0f};
        }
        else
        {
            SnapValues = {90.0f, 90.0f, 90.0f};
        }

        ImGuizmo::Manipulate(&WorldCameraViewMatrix->Data[0][0], &WorldCameraProjectionMatrix.Data[0][0],
                             static_cast<ImGuizmo::OPERATION>(UIData->GizmoType), ImGuizmo::MODE::LOCAL,
                             &EntityTransform.Data[0][0], nullptr, SnapValues.data());

        if (ImGuizmo::IsUsing())
        {
            GCVector3 EntityTranslation{}, EntityRotation{}, EntityScale{};
            GCMatrix4x4_Decompose(&EntityTransform, &EntityTranslation, &EntityRotation, &EntityScale);

            const GCTransformComponent EntityTransformComponentCopy = *EntityTransformComponent;

            EntityTransformComponent->Translation = EntityTranslation;
            EntityTransformComponent->Rotation = EntityRotation;
            EntityTransformComponent->Scale = EntityScale;

            if (GCWorld_CheckCollision(World, UIData->SelectedEntity))
            {
                EntityTransformComponent->Translation = EntityTransformComponentCopy.Translation;
                EntityTransformComponent->Rotation = EntityTransformComponentCopy.Rotation;
                EntityTransformComponent->Scale = EntityTransformComponentCopy.Scale;
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();

    GCImGuiManager_Render();

    GCRenderer_EndImGui();
}

void GCUI_OnUpdate(void)
{
    if (UIData->IsViewportHovered)
    {
        GCWorldCamera_Update(GCWorld_GetCamera(GCApplication_GetWorld()));
    }

    UIData->ViewportSize = GCVector2_Subtract(UIData->ViewportBounds[1], UIData->ViewportBounds[0]);

    const GCVector2 MousePosition = GCUI_GetMousePosition();

    const int32_t MouseX = static_cast<int32_t>(MousePosition.X);
    const int32_t MouseY = static_cast<int32_t>(MousePosition.Y);

    uint32_t FramebufferWidth = 0, FramebufferHeight = 0;
    GCRendererFramebuffer_GetSize(GCRenderer_GetFramebuffer(), &FramebufferWidth, &FramebufferHeight);

    if (UIData->ViewportSize.X == FramebufferWidth && UIData->ViewportSize.Y == FramebufferHeight)
    {
        if (MouseX >= 0 && MouseY >= 0 && MouseX < static_cast<std::int32_t>(UIData->ViewportSize.X) &&
            MouseY < static_cast<std::int32_t>(UIData->ViewportSize.Y))
        {
            const int32_t EntityID = GCRendererFramebuffer_GetPixel(GCRenderer_GetFramebuffer(),
                                                                    GCRenderer_GetCommandList(), 1, MouseX, MouseY);

            UIData->HoveredEntity = EntityID == -1 ? GCEntity{} : GCEntity{static_cast<uint64_t>(EntityID)};
        }
    }
}

void GCUI_OnEvent(GCEvent* const Event)
{
    GCEvent_Dispatch(GCEventType_KeyPressed, Event, GCUI_OnKeyPressed, NULL);
    GCEvent_Dispatch(GCEventType_MouseButtonPressed, Event, GCUI_OnMouseButtonPressed, NULL);
}

void GCUI_Terminate(void)
{
    for (const auto& Iterator : UIData->UIEntityData)
    {
        GCRendererTexture2D_Destroy(Iterator.second.first);
        GCRendererModel_Destroy(Iterator.first);
    }

    GCImGuiManager_TerminateRenderer();
    GCImGuiManager_TerminatePlatform();
    GCImGuiManager_Terminate();

    delete UIData;
}

void GCUI_ResizeAttachment(void)
{
    GCRendererFramebuffer_RecreateAttachmentFramebuffer(GCRenderer_GetFramebuffer(),
                                                        static_cast<uint32_t>(UIData->ViewportSize.X),
                                                        static_cast<uint32_t>(UIData->ViewportSize.Y));
    GCWorldCamera_SetSize(GCWorld_GetCamera(GCApplication_GetWorld()), static_cast<uint32_t>(UIData->ViewportSize.X),
                          static_cast<uint32_t>(UIData->ViewportSize.Y));
}

bool GCUI_OnKeyPressed(GCEvent* const Event, void* CustomData)
{
    (void)CustomData;

    const GCKeyPressedEvent* const EventDetail = (const GCKeyPressedEvent* const)Event->EventDetail;

    switch (EventDetail->KeyCode)
    {
    case GCKeyCode_Escape: {
        if (UIData->GizmoType != -1)
        {
            UIData->GizmoType = -1;
        }

        break;
    }
    case GCKeyCode_P:
    case GCKeyCode_T: {
        UIData->GizmoType = static_cast<int32_t>(ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Z);

        break;
    }
    case GCKeyCode_R: {
        UIData->GizmoType = static_cast<int32_t>(ImGuizmo::OPERATION::ROTATE_Y);

        break;
    }
    }

    return false;
}

bool GCUI_OnMouseButtonPressed(GCEvent* const Event, void* CustomData)
{
    (void)CustomData;

    const GCMouseButtonPressedEvent* const EventDetail = (const GCMouseButtonPressedEvent* const)Event->EventDetail;

    if (EventDetail->MouseButtonCode == GCMouseButtonCode_Left)
    {
        if (UIData->IsViewportHovered && !ImGuizmo::IsOver() && !GCInput_IsKeyPressed(GCKeyCode_LeftAlt))
        {
            UIData->SelectedEntity = UIData->HoveredEntity;
        }
    }

    return false;
}

GCVector2 GCUI_GetMousePosition(void)
{
    ImVec2 MousePosition{ImGui::GetMousePos()};
    MousePosition.x -= UIData->ViewportBounds[0].X;
    MousePosition.y -= UIData->ViewportBounds[0].Y;

    return GCVector2_Create(MousePosition.x, MousePosition.y);
}
