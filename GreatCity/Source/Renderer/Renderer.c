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

#include "Renderer/Renderer.h"
#include "Renderer/RendererModel.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/RendererSwapChain.h"
#include "Renderer/RendererCommandList.h"
#include "Renderer/RendererVertexBuffer.h"
#include "Renderer/RendererIndexBuffer.h"
#include "Renderer/RendererUniformBuffer.h"
#include "Renderer/RendererTexture2D.h"
#include "Renderer/RendererShader.h"
#include "Renderer/RendererGraphicsPipeline.h"
#include "Renderer/RendererFramebuffer.h"
#include "Renderer/RendererEnums.h"
#include "Core/Memory/Allocator.h"
#include "ApplicationCore/GenericPlatform/Window.h"
#include "ApplicationCore/Application.h"
#include "Scene/Entity.h"
#include "Scene/Camera/WorldCamera.h"
#include "ImGui/ImGuiManager.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <stddef.h>
#include <stdint.h>

typedef struct GCRendererDrawData
{
	const GCRendererVertexBuffer* VertexBuffer;
	uint32_t VertexCount;

	const GCRendererIndexBuffer* IndexBuffer;
	uint32_t IndexCount;
} GCRendererDrawData;

typedef struct GCRenderer
{
	GCRendererDevice* Device;
	GCRendererSwapChain* SwapChain;
	GCRendererCommandList* CommandList;
	GCRendererUniformBuffer* UniformBuffer;
	GCRendererShader* BasicShader;
	GCRendererGraphicsPipeline* GraphicsPipeline;
	GCRendererFramebuffer* Framebuffer;

	const GCWorldCamera* WorldCamera;

	uint32_t MaximumDrawDataCount;
	GCRendererDrawData* DrawData;
	uint32_t DrawDataCount;
} GCRenderer;

typedef struct GCRendererUniformBufferData
{
	GCMatrix4x4 ViewProjectionMatrix;
} GCRendererUniformBufferData;

static void GCRenderer_ResizeSwapChain(void);

static GCRenderer* Renderer = NULL;

void GCRenderer_Initialize(const GCWorldCamera* const WorldCamera)
{
	Renderer = (GCRenderer*)GCMemory_Allocate(sizeof(GCRenderer));
	Renderer->Device = GCRendererDevice_Create();

	GCRendererSwapChainDescription SwapChainDescription = { 0 };
	SwapChainDescription.Device = Renderer->Device;
	Renderer->SwapChain = GCRendererSwapChain_Create(&SwapChainDescription);

	GCRendererCommandListDescription CommandListDescription = { 0 };
	CommandListDescription.Device = Renderer->Device;
	CommandListDescription.SwapChain = Renderer->SwapChain;
	Renderer->CommandList = GCRendererCommandList_Create(&CommandListDescription);

	GCRendererUniformBufferDescription UniformBufferDescription = { 0 };
	UniformBufferDescription.Device = Renderer->Device;
	UniformBufferDescription.CommandList = Renderer->CommandList;
	UniformBufferDescription.DataSize = sizeof(GCRendererUniformBufferData);
	Renderer->UniformBuffer = GCRendererUniformBuffer_Create(&UniformBufferDescription);
	
	GCRendererShaderDescription ShaderDescription = { 0 };
	ShaderDescription.Device = Renderer->Device;
	ShaderDescription.VertexShaderPath = "Assets/Shaders/Basic/Basic.vertex.glsl";
	ShaderDescription.FragmentShaderPath = "Assets/Shaders/Basic/Basic.fragment.glsl";
	Renderer->BasicShader = GCRendererShader_Create(&ShaderDescription);

	GCRendererGraphicsPipelineAttachment GraphicsPipelineAttachments[3] = { 0 };
	GraphicsPipelineAttachments[0].Type = GCRendererAttachmentType_Color;
	GraphicsPipelineAttachments[0].Format = GCRendererAttachmentFormat_SRGB;
	GraphicsPipelineAttachments[0].SampleCount = GCRendererAttachmentSampleCount_2;
	GraphicsPipelineAttachments[1].Type = GCRendererAttachmentType_Color;
	GraphicsPipelineAttachments[1].Format = GCRendererAttachmentFormat_Integer;
	GraphicsPipelineAttachments[1].SampleCount = GCRendererAttachmentSampleCount_2;
	GraphicsPipelineAttachments[2].Type = GCRendererAttachmentType_DepthStencil;
	GraphicsPipelineAttachments[2].Format = GCRendererAttachmentFormat_D32;
	GraphicsPipelineAttachments[2].SampleCount = GCRendererAttachmentSampleCount_2;

	GCRendererGraphicsPipelineVertexInputBinding GraphicsPipelineVertexInputBindings[1] = { 0 };
	GraphicsPipelineVertexInputBindings[0].Binding = 0;
	GraphicsPipelineVertexInputBindings[0].Stride = sizeof(GCRendererVertex);

	GCRendererGraphicsPipelineVertexInputAttribute GraphicsPipelineVertexInputAttributes[4] = { 0 };
	GraphicsPipelineVertexInputAttributes[0].Location = 0;
	GraphicsPipelineVertexInputAttributes[0].Format = GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector3;
	GraphicsPipelineVertexInputAttributes[0].Offset = offsetof(GCRendererVertex, Position);

	GraphicsPipelineVertexInputAttributes[1].Location = 1;
	GraphicsPipelineVertexInputAttributes[1].Format = GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector4;
	GraphicsPipelineVertexInputAttributes[1].Offset = offsetof(GCRendererVertex, Color);

	GraphicsPipelineVertexInputAttributes[2].Location = 2;
	GraphicsPipelineVertexInputAttributes[2].Format = GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector2;
	GraphicsPipelineVertexInputAttributes[2].Offset = offsetof(GCRendererVertex, TextureCoordinate);

	GraphicsPipelineVertexInputAttributes[3].Location = 3;
	GraphicsPipelineVertexInputAttributes[3].Format = GCRendererGraphicsPipelineVertexInputAttributeFormat_Integer;
	GraphicsPipelineVertexInputAttributes[3].Offset = offsetof(GCRendererVertex, EntityID);

	GCRendererGraphicsPipelineVertexInput GraphicsPipelineVertexInput = { 0 };
	GraphicsPipelineVertexInput.Bindings = GraphicsPipelineVertexInputBindings;
	GraphicsPipelineVertexInput.BindingCount = sizeof(GraphicsPipelineVertexInputBindings) / sizeof(GCRendererGraphicsPipelineVertexInputBinding);
	GraphicsPipelineVertexInput.Attributes = GraphicsPipelineVertexInputAttributes;
	GraphicsPipelineVertexInput.AttributeCount = sizeof(GraphicsPipelineVertexInputAttributes) / sizeof(GCRendererGraphicsPipelineVertexInputAttribute);

	GCRendererGraphicsPipelineDescription GraphicsPipelineDescription = { 0 };
	GraphicsPipelineDescription.Device = Renderer->Device;
	GraphicsPipelineDescription.SwapChain = Renderer->SwapChain;
	GraphicsPipelineDescription.CommandList = Renderer->CommandList;
	GraphicsPipelineDescription.Attachments = GraphicsPipelineAttachments;
	GraphicsPipelineDescription.AttachmentCount = 3;
	GraphicsPipelineDescription.VertexInput = &GraphicsPipelineVertexInput;
	GraphicsPipelineDescription.SampleCount = GCRendererAttachmentSampleCount_2;
	GraphicsPipelineDescription.UniformBuffer = Renderer->UniformBuffer;
	GraphicsPipelineDescription.Texture2Ds = NULL;
	GraphicsPipelineDescription.Texture2DCount = 0;
	GraphicsPipelineDescription.Shader = Renderer->BasicShader;
	Renderer->GraphicsPipeline = GCRendererGraphicsPipeline_Create(&GraphicsPipelineDescription);

	GCRendererFramebufferAttachment FramebufferAttachments[3] = { 0 };
	FramebufferAttachments[0].Type = GCRendererAttachmentType_Color;
	FramebufferAttachments[0].Flags = GCRendererFramebufferAttachmentFlags_Sampled;
	FramebufferAttachments[0].Format = GCRendererAttachmentFormat_SRGB;
	FramebufferAttachments[0].SampleCount = GCRendererAttachmentSampleCount_2;

	FramebufferAttachments[1].Type = GCRendererAttachmentType_Color;
	FramebufferAttachments[1].Flags = GCRendererFramebufferAttachmentFlags_None;
	FramebufferAttachments[1].Format = GCRendererAttachmentFormat_Integer;
	FramebufferAttachments[1].SampleCount = GCRendererAttachmentSampleCount_2;

	FramebufferAttachments[2].Type = GCRendererAttachmentType_DepthStencil;
	FramebufferAttachments[2].Flags = GCRendererFramebufferAttachmentFlags_None;
	FramebufferAttachments[2].Format = GCRendererAttachmentFormat_D32;
	FramebufferAttachments[2].SampleCount = GCRendererAttachmentSampleCount_2;

	uint32_t WindowWidth = 0, WindowHeight = 0;
	GCWindow_GetWindowSize(GCApplication_GetWindow(), &WindowWidth, &WindowHeight);

	GCRendererFramebufferDescription FramebufferDescription = { 0 };
	FramebufferDescription.Device = Renderer->Device;
	FramebufferDescription.SwapChain = Renderer->SwapChain;
	FramebufferDescription.GraphicsPipeline = Renderer->GraphicsPipeline;
	FramebufferDescription.Width = WindowWidth;
	FramebufferDescription.Height = WindowHeight;
	FramebufferDescription.Attachments = FramebufferAttachments;
	FramebufferDescription.AttachmentCount = 3;
	Renderer->Framebuffer = GCRendererFramebuffer_Create(&FramebufferDescription);
	Renderer->WorldCamera = WorldCamera;
	Renderer->MaximumDrawDataCount = 100;
	Renderer->DrawData = (GCRendererDrawData*)GCMemory_Allocate(Renderer->MaximumDrawDataCount * sizeof(GCRendererDrawData));
	Renderer->DrawDataCount = 0;

	GCRendererCommandList_SetSwapChainResizeCallback(Renderer->CommandList, GCRenderer_ResizeSwapChain);
}

void GCRenderer_BeginScene(void)
{
	Renderer->DrawDataCount = 0;

	GCRendererCommandList_BeginRecord(Renderer->CommandList);

	const float ClearColorTexture[4] = { 0.729f, 0.901f, 0.992f, 1.0f };
	GCRendererCommandList_BeginAttachmentRenderPass(Renderer->CommandList, Renderer->GraphicsPipeline, Renderer->Framebuffer, ClearColorTexture);

	GCRendererUniformBufferData UniformBufferData = { 0 };
	UniformBufferData.ViewProjectionMatrix = GCWorldCamera_GetViewProjectionMatrix(Renderer->WorldCamera);

	GCRendererCommandList_UpdateUniformBuffer(Renderer->CommandList, Renderer->UniformBuffer, &UniformBufferData, sizeof(GCRendererUniformBufferData));
	GCRendererCommandList_BindGraphicsPipeline(Renderer->CommandList, Renderer->GraphicsPipeline);
	GCRendererCommandList_SetViewport(Renderer->CommandList, Renderer->Framebuffer);
}

void GCRenderer_RenderEntity(const GCEntity Entity)
{
	const GCMeshComponent* const MeshComponent = GCEntity_GetMeshComponent(Entity);
	const GCRendererModel* const Model = MeshComponent->Model;

	if (Renderer->DrawDataCount >= Renderer->MaximumDrawDataCount)
	{
		Renderer->MaximumDrawDataCount += Renderer->MaximumDrawDataCount;
		Renderer->DrawData = (GCRendererDrawData*)GCMemory_Reallocate(Renderer->DrawData, Renderer->MaximumDrawDataCount * sizeof(GCRendererDrawData));
	}

	Renderer->DrawData[Renderer->DrawDataCount].VertexBuffer = Model->VertexBuffer;
	Renderer->DrawData[Renderer->DrawDataCount].VertexCount = Model->VertexCount;
	Renderer->DrawData[Renderer->DrawDataCount].IndexBuffer = Model->IndexBuffer;
	Renderer->DrawData[Renderer->DrawDataCount].IndexCount = Model->IndexCount;

	Renderer->DrawDataCount++;
}

void GCRenderer_EndScene(void)
{
	for (uint32_t Counter = 0; Counter < Renderer->DrawDataCount; Counter++)
	{
		GCRendererCommandList_BindVertexBuffer(Renderer->CommandList, Renderer->DrawData[Counter].VertexBuffer);
		GCRendererCommandList_BindIndexBuffer(Renderer->CommandList, Renderer->DrawData[Counter].IndexBuffer);
		GCRendererCommandList_DrawIndexed(Renderer->CommandList, Renderer->DrawData[Counter].IndexCount, 0);
	}

	GCRendererCommandList_EndAttachmentRenderPass(Renderer->CommandList, Renderer->Framebuffer);
}

void GCRenderer_BeginImGui(void)
{
	const float ClearColorSwapChain[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GCRendererCommandList_BeginSwapChainRenderPass(Renderer->CommandList, Renderer->GraphicsPipeline, Renderer->Framebuffer, ClearColorSwapChain);
}

void GCRenderer_EndImGui(void)
{
	GCRendererCommandList_EndSwapChainRenderPass(Renderer->CommandList);
}

void GCRenderer_Present(void)
{
	GCRendererCommandList_EndRecord(Renderer->CommandList);
	GCRendererCommandList_SubmitAndPresent(Renderer->CommandList);
}

void GCRenderer_Resize(void)
{
	if (Renderer)
	{
		GCRendererCommandList_ShouldSwapChainResize(Renderer->CommandList, true);
	}
}

void GCRenderer_Terminate(void)
{
	GCRendererFramebuffer_Destroy(Renderer->Framebuffer);
	GCRendererGraphicsPipeline_Destroy(Renderer->GraphicsPipeline);
	GCRendererShader_Destroy(Renderer->BasicShader);
	GCRendererUniformBuffer_Destroy(Renderer->UniformBuffer);
	GCRendererCommandList_Destroy(Renderer->CommandList);
	GCRendererSwapChain_Destroy(Renderer->SwapChain);
	GCRendererDevice_Destroy(Renderer->Device);

	GCMemory_Free(Renderer->DrawData);
	GCMemory_Free(Renderer);
}

GCRendererDevice* const GCRenderer_GetDevice(void)
{
	return Renderer->Device;
}

GCRendererSwapChain* const GCRenderer_GetSwapChain(void)
{
	return Renderer->SwapChain;
}

GCRendererCommandList* const GCRenderer_GetCommandList(void)
{
	return Renderer->CommandList;
}

GCRendererGraphicsPipeline* const GCRenderer_GetGraphicsPipeline(void)
{
	return Renderer->GraphicsPipeline;
}

GCRendererFramebuffer* const GCRenderer_GetFramebuffer(void)
{
	return Renderer->Framebuffer;
}

void GCRenderer_ResizeSwapChain(void)
{
	uint32_t Width = 0, Height = 0;
	GCWindow_GetWindowSize(GCApplication_GetWindow(), &Width, &Height);

	if (Width > 0 && Height > 0)
	{
		GCRendererSwapChain_Recreate(Renderer->SwapChain);
		GCRendererFramebuffer_RecreateSwapChainFramebuffer(Renderer->Framebuffer);
	}
}