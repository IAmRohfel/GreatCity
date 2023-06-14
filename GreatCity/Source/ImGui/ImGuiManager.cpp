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
#include "ApplicationCore/GenericPlatform/KeyCode.h"
#include "ApplicationCore/Event/Event.h"
#include "ApplicationCore/Event/KeyEvent.h"
#include "ApplicationCore/Event/MouseEvent.h"
#include "ApplicationCore/Application.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererCommandList.h"
#include "Renderer/RendererTexture2D.h"
#include "Renderer/RendererFramebuffer.h"
#include "Renderer/RendererModel.h"
#include "World/Camera/WorldCamera.h"
#include "World/World.h"
#include "World/Entity.h"
#include "World/Components.h"
#include "Math/Vector2.h"

#include <array>
#include <vector>
#include <string>
#include <cstdint>

#include <imgui.h>
#include <ImGuizmo.h>

extern "C" void GCImGuiManager_InitializePlatform(void);
extern "C" void GCImGuiManager_TerminatePlatform(void);
extern "C" void GCImGuiManager_InitializeRenderer(const GCRendererTexture2D* const* const Texture2Ds, void** const Texture2DData, const uint32_t Texture2DCount);
extern "C" void GCImGuiManager_BeginFramePlatform(void);
extern "C" void GCImGuiManager_BeginFrameRenderer(void);
extern "C" void* GCImGuiManager_GetTexturePlatform(void);
extern "C" void GCImGuiManager_RenderDrawData(void);
extern "C" void GCImGuiManager_TerminateRenderer(void);

struct GCImGuiManagerState
{
public:
	std::vector<void*> Texture2Ds{};
	std::vector<GCEntity> Entities{};

	GCVector2 ViewportSize{};
	GCVector2 ViewportBounds[2]{};
	GCEntity HoveredEntity{};
	GCEntity SelectedEntity{};

	bool IsViewportHovered{};
	bool IsViewportFocused{};

	int32_t GizmoType{ -1 };
};

static void GCImGuiManager_SetDarkTheme(void);
static void GCImGuiManager_ResizeAttachment(void);
static bool GCImGuiManager_OnKeyPressed(GCEvent* const Event, void* CustomData);
static bool GCImGuiManager_OnMouseButtonPressed(GCEvent* const Event, void* CustomData);

static GCImGuiManagerState* ImGuiManagerState{};

extern "C" void GCImGuiManager_Initialize(void)
{
	ImGuiManagerState = new GCImGuiManagerState{};

	const GCRendererDevice* const Device = GCRenderer_GetDevice();
	const GCRendererCommandList* const CommandList = GCRenderer_GetCommandList();

	GCRendererTexture2DDescription SmallOfficeTextureDescription = { 0 };
	SmallOfficeTextureDescription.Device = Device;
	SmallOfficeTextureDescription.CommandList = CommandList;
	SmallOfficeTextureDescription.TexturePath = "Assets/Textures/Buildings/Offices/SmallOffice.png";
	GCRendererTexture2D* SmallOfficeTexture = GCRendererTexture2D_Create(&SmallOfficeTextureDescription);

	std::array<GCRendererTexture2D*, 1> Texture2Ds{};
	Texture2Ds[0] = SmallOfficeTexture;

	ImGuiManagerState->Texture2Ds.resize(Texture2Ds.size());
	GCRenderer_SetTexture2Ds(Texture2Ds.data(), static_cast<uint32_t>(Texture2Ds.size()));

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& IO = ImGui::GetIO();
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	IO.Fonts->AddFontFromFileTTF("Assets/Fonts/OpenSans/OpenSans-Regular.ttf", 18.0f);

	GCImGuiManager_SetDarkTheme();

	GCImGuiManager_InitializePlatform();
	GCImGuiManager_InitializeRenderer(Texture2Ds.data(), ImGuiManagerState->Texture2Ds.data(), static_cast<uint32_t>(Texture2Ds.size()));

	GCRendererCommandList_SetAttachmentResizeCallback(GCRenderer_GetCommandList(), GCImGuiManager_ResizeAttachment);

	ImGuizmo::Style& ImGuizmoStyle = ImGuizmo::GetStyle();

	ImGuizmoStyle.TranslationLineThickness = 5.0f;
	ImGuizmoStyle.ScaleLineThickness = 5.0f;
	ImGuizmoStyle.RotationLineThickness = 5.0f;
	ImGuizmoStyle.RotationOuterLineThickness = 5.0f;
	ImGuizmoStyle.TranslationLineArrowSize = 5.0f;
}

extern "C" void GCImGuiManager_BeginFrame(void)
{
	GCImGuiManager_BeginFrameRenderer();
	GCImGuiManager_BeginFramePlatform();

	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	uint32_t FramebufferWidth = 0, FramebufferHeight = 0;
	GCRendererFramebuffer_GetSize(GCRenderer_GetFramebuffer(), &FramebufferWidth, &FramebufferHeight);

	if (ImGuiManagerState->ViewportSize.X > 0.0f && ImGuiManagerState->ViewportSize.Y > 0.0f && (ImGuiManagerState->ViewportSize.X != FramebufferWidth || ImGuiManagerState->ViewportSize.Y != FramebufferHeight))
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

extern "C" void GCImGuiManager_RenderUI(void)
{
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

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
	ImGui::Begin("Viewport", nullptr, IsAltPressed ? ImGuiWindowFlags_NoMove : ImGuiWindowFlags_None);

	if (OpenAddBuildingPopup)
	{
		ImGui::OpenPopup("Add Building");
	}

	if (ImGui::BeginPopupModal("Add Building"))
	{
		bool ClosePopup = false;

		if (ImGui::ImageButton(ImGuiManagerState->Texture2Ds[0], ImVec2{ 50, 50 }))
		{
			static uint32_t NameCount = 0;

			const std::string Name{ "Small Office" + NameCount };

			const GCEntity Entity = GCWorld_CreateEntity(World, Name.c_str());
			GCRendererModel* Model = GCRendererModel_CreateFromFile("Assets/Models/Buildings/Offices/SmallOffice.obj", "Assets/Models/Buildings/Offices");

			GCTransformComponent* const TransformComponent = GCEntity_GetTransformComponent(Entity);
			TransformComponent->Translation = GCVector3_Create(0.0f, GCEntity_GetTransformComponent(Entity)->Translation.Y - 2.5f, 0.0f);

			GCEntity_AddMeshComponent(Entity, Model);
			GCRendererModel_Destroy(Model);

			ImGuiManagerState->Entities.emplace_back(Entity);
			NameCount++;
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

	ImGuiManagerState->ViewportBounds[0] = { ViewportMinimumRegion.x + ViewportOffset.x, ViewportMinimumRegion.y + ViewportOffset.y };
	ImGuiManagerState->ViewportBounds[1] = { ViewportMaximumRegion.x + ViewportOffset.x, ViewportMaximumRegion.y + ViewportOffset.y };
	
	ImGuiManagerState->IsViewportHovered = ImGui::IsWindowHovered();
	ImGuiManagerState->IsViewportFocused = ImGui::IsWindowFocused();

	ImGui::Image(static_cast<ImTextureID>(GCImGuiManager_GetTexturePlatform()), ImVec2{ ImGuiManagerState->ViewportSize.X, ImGuiManagerState->ViewportSize.Y });

	if (ImGuiManagerState->SelectedEntity != 0 && ImGuiManagerState->SelectedEntity != GCWorld_GetTerrainEntity(GCApplication_GetWorld()) && ImGuiManagerState->GizmoType != -1)
	{
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGuiManagerState->ViewportBounds[0].X, ImGuiManagerState->ViewportBounds[0].Y, ImGuiManagerState->ViewportBounds[1].X - ImGuiManagerState->ViewportBounds[0].X, ImGuiManagerState->ViewportBounds[1].Y - ImGuiManagerState->ViewportBounds[0].Y);

		const GCWorldCamera* const WorldCamera = GCWorld_GetCamera(World);
		const GCMatrix4x4* const WorldCameraViewMatrix = GCWorldCamera_GetViewMatrix(WorldCamera);
		const GCMatrix4x4* const OriginalWorldCameraProjectionMatrix = GCWorldCamera_GetProjectionMatrix(WorldCamera);
		
		GCMatrix4x4 WorldCameraProjectionMatrix = *OriginalWorldCameraProjectionMatrix;
		WorldCameraProjectionMatrix.Data[1][1] *= -1.0f;

		GCTransformComponent* const EntityTransformComponent = GCEntity_GetTransformComponent(ImGuiManagerState->SelectedEntity);
		GCMatrix4x4 EntityTransform = GCTransformComponent_GetTransform(EntityTransformComponent);

		std::array<float, 3> SnapValues{};

		if (ImGuiManagerState->GizmoType == static_cast<int32_t>(ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Z))
		{
			SnapValues = { 1.0f, 1.0f, 1.0f };
		}
		else
		{
			SnapValues = { 90.0f, 90.0f, 90.0f };
		}

		ImGuizmo::Manipulate(&WorldCameraViewMatrix->Data[0][0], &WorldCameraProjectionMatrix.Data[0][0], static_cast<ImGuizmo::OPERATION>(ImGuiManagerState->GizmoType), ImGuizmo::MODE::LOCAL, &EntityTransform.Data[0][0], nullptr, SnapValues.data());

		if (ImGuizmo::IsUsing())
		{
			GCVector3 EntityTranslation{}, EntityRotation{}, EntityScale{};
			GCMatrix4x4_Decompose(&EntityTransform, &EntityTranslation, &EntityRotation, &EntityScale);

			GCVector3 DeltaRotation = GCVector3_Subtract(EntityRotation, EntityTransformComponent->Rotation);

			EntityTransformComponent->Translation = EntityTranslation;
			EntityTransformComponent->Rotation = GCVector3_Add(EntityTransformComponent->Rotation, DeltaRotation);
			EntityTransformComponent->Scale = EntityScale;
		}
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

extern "C" void GCImGuiManager_EndFrame(void)
{
}

extern "C" void GCImGuiManager_Render(void)
{
	ImGui::Render();
	GCImGuiManager_RenderDrawData();
}

void GCImGuiManager_OnEvent(GCEvent* const Event)
{
	GCEvent_Dispatch(GCEventType_KeyPressed, Event, GCImGuiManager_OnKeyPressed, NULL);
	GCEvent_Dispatch(GCEventType_MouseButtonPressed, Event, GCImGuiManager_OnMouseButtonPressed, NULL);
}

extern "C" void GCImGuiManager_Update(void)
{
	if (ImGuiManagerState->IsViewportHovered)
	{
		GCWorldCamera_Update(GCWorld_GetCamera(GCApplication_GetWorld()));
	}

	ImVec2 MousePosition{ ImGui::GetMousePos() };
	MousePosition.x -= ImGuiManagerState->ViewportBounds[0].X;
	MousePosition.y -= ImGuiManagerState->ViewportBounds[0].Y;

	ImGuiManagerState->ViewportSize = GCVector2_Subtract(ImGuiManagerState->ViewportBounds[1], ImGuiManagerState->ViewportBounds[0]);

	const std::int32_t MouseX = static_cast<std::int32_t>(MousePosition.x);
	const std::int32_t MouseY = static_cast<std::int32_t>(MousePosition.y);

	uint32_t FramebufferWidth = 0, FramebufferHeight = 0;
	GCRendererFramebuffer_GetSize(GCRenderer_GetFramebuffer(), &FramebufferWidth, &FramebufferHeight);

	if (ImGuiManagerState->ViewportSize.X == FramebufferWidth && ImGuiManagerState->ViewportSize.Y == FramebufferHeight)
	{
		if (MouseX >= 0 && MouseY >= 0 && MouseX < static_cast<std::int32_t>(ImGuiManagerState->ViewportSize.X) && MouseY < static_cast<std::int32_t>(ImGuiManagerState->ViewportSize.Y))
		{
			const int32_t EntityID = GCRendererFramebuffer_GetPixel(GCRenderer_GetFramebuffer(), GCRenderer_GetCommandList(), 1, MouseX, MouseY);

			ImGuiManagerState->HoveredEntity = EntityID == -1 ? GCEntity{} : GCEntity{ static_cast<uint64_t>(EntityID) };
		}
	}
}

extern "C" void GCImGuiManager_Terminate(void)
{
	GCImGuiManager_TerminateRenderer();
	GCImGuiManager_TerminatePlatform();

	ImGui::DestroyContext();

	delete ImGuiManagerState;
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
	GCRendererFramebuffer_RecreateAttachmentFramebuffer(GCRenderer_GetFramebuffer(), static_cast<uint32_t>(ImGuiManagerState->ViewportSize.X), static_cast<uint32_t>(ImGuiManagerState->ViewportSize.Y));
	GCWorldCamera_SetSize(GCWorld_GetCamera(GCApplication_GetWorld()), static_cast<uint32_t>(ImGuiManagerState->ViewportSize.X), static_cast<uint32_t>(ImGuiManagerState->ViewportSize.Y));
}

bool GCImGuiManager_OnKeyPressed(GCEvent* const Event, void* CustomData)
{
	(void)CustomData;

	const GCKeyPressedEvent* const EventDetail = (const GCKeyPressedEvent* const)Event->EventDetail;

	switch (EventDetail->KeyCode)
	{
		case GCKeyCode_Escape:
		{
			if (ImGuiManagerState->GizmoType != -1)
			{
				ImGuiManagerState->GizmoType = -1;
			}

			break;
		}
		case GCKeyCode_P:
		case GCKeyCode_T:
		{
			ImGuiManagerState->GizmoType = static_cast<int32_t>(ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Z);

			break;
		}
		case GCKeyCode_R:
		{
			ImGuiManagerState->GizmoType = static_cast<int32_t>(ImGuizmo::OPERATION::ROTATE_Y);

			break;
		}
	}

	return false;
}

bool GCImGuiManager_OnMouseButtonPressed(GCEvent* const Event, void* CustomData)
{
	(void)CustomData;

	const GCMouseButtonPressedEvent* const EventDetail = (const GCMouseButtonPressedEvent* const)Event->EventDetail;

	if (EventDetail->MouseButtonCode == GCMouseButtonCode_Left)
	{
		if (ImGuiManagerState->IsViewportHovered && !ImGuizmo::IsOver() && !GCInput_IsKeyPressed(GCKeyCode_LeftAlt))
		{
			ImGuiManagerState->SelectedEntity = ImGuiManagerState->HoveredEntity;
		}
	}

	return false;
}