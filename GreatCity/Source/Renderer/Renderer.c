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
#include "Core/Memory/Allocator.h"
#include "Scene/Entity.h"
#include "Scene/Camera/WorldCamera.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <stddef.h>

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
static void GCRenderer_RecordCommands(const GCRendererCommandListRecordData* const RecordData);

static GCRenderer* Renderer = NULL;

void GCRenderer_Initialize(const GCWorldCamera* const WorldCamera)
{
	Renderer = (GCRenderer*)GCMemory_Allocate(sizeof(GCRenderer));
	Renderer->Device = GCRendererDevice_Create();
	Renderer->SwapChain = GCRendererSwapChain_Create(Renderer->Device);
	Renderer->CommandList = GCRendererCommandList_Create(Renderer->Device);
	Renderer->UniformBuffer = GCRendererUniformBuffer_Create(Renderer->Device, Renderer->CommandList, sizeof(GCRendererUniformBufferData));
	Renderer->BasicShader = GCRendererShader_Create(Renderer->Device, "Assets/Shaders/Basic/Basic.vertex.glsl", "Assets/Shaders/Basic/Basic.fragment.glsl");

	GCRendererGraphicsPipelineVertexInput GraphicsPipelineVertexInput = { 0 };
	GraphicsPipelineVertexInput.Stride = sizeof(GCRendererVertex);

	GCRendererGraphicsPipelineVertexInputAttribute GraphicsPipelineVertexInputAttributes[3] = { 0 };
	GraphicsPipelineVertexInputAttributes[0].Location = 0;
	GraphicsPipelineVertexInputAttributes[0].Format = GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector3;
	GraphicsPipelineVertexInputAttributes[0].Offset = offsetof(GCRendererVertex, Position);

	GraphicsPipelineVertexInputAttributes[1].Location = 1;
	GraphicsPipelineVertexInputAttributes[1].Format = GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector4;
	GraphicsPipelineVertexInputAttributes[1].Offset = offsetof(GCRendererVertex, Color);

	GraphicsPipelineVertexInputAttributes[2].Location = 2;
	GraphicsPipelineVertexInputAttributes[2].Format = GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector2;
	GraphicsPipelineVertexInputAttributes[2].Offset = offsetof(GCRendererVertex, TextureCoordinate);

	GraphicsPipelineVertexInput.Attributes = GraphicsPipelineVertexInputAttributes;
	GraphicsPipelineVertexInput.AttributeCount = sizeof(GraphicsPipelineVertexInputAttributes) / sizeof(GCRendererGraphicsPipelineVertexInputAttribute);

	Renderer->GraphicsPipeline = GCRendererGraphicsPipeline_Create(Renderer->Device, Renderer->SwapChain, Renderer->CommandList, &GraphicsPipelineVertexInput, Renderer->UniformBuffer, NULL, 0, Renderer->BasicShader);
	Renderer->Framebuffer = GCRendererFramebuffer_Create(Renderer->Device, Renderer->SwapChain, Renderer->GraphicsPipeline);

	GCRendererCommandList_SetResizeCallback(Renderer->CommandList, GCRenderer_ResizeSwapChain);

	Renderer->WorldCamera = WorldCamera;

	Renderer->MaximumDrawDataCount = 100;
	Renderer->DrawData = (GCRendererDrawData*)GCMemory_Allocate(Renderer->MaximumDrawDataCount * sizeof(GCRendererDrawData));
	Renderer->DrawDataCount = 0;
}

void GCRenderer_Begin(void)
{
	Renderer->DrawDataCount = 0;
}

void GCRenderer_RenderEntity(const GCEntity Entity)
{
	const GCMeshComponent* const MeshComponent = GCEntity_GetMeshComponent(Entity);
	const GCRendererModel* const Model = MeshComponent->Model;

	Renderer->DrawData[Renderer->DrawDataCount].VertexBuffer = Model->VertexBuffer;
	Renderer->DrawData[Renderer->DrawDataCount].VertexCount = Model->VertexCount;
	Renderer->DrawData[Renderer->DrawDataCount].IndexBuffer = Model->IndexBuffer;
	Renderer->DrawData[Renderer->DrawDataCount].IndexCount = Model->IndexCount;

	Renderer->DrawDataCount++;
}

void GCRenderer_End(void)
{
}

void GCRenderer_Present(void)
{
	GCRendererCommandList_SubmitAndPresent(Renderer->CommandList, Renderer->SwapChain, GCRenderer_RecordCommands);
}

void GCRenderer_Resize(void)
{
	if (Renderer)
	{
		GCRendererCommandList_SetResize(Renderer->CommandList, true);
	}
}

void GCRenderer_Terminate(void)
{
	GCRendererDevice_WaitIdle(Renderer->Device);

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

const GCRendererDevice* const GCRenderer_GetDevice(void)
{
	return Renderer->Device;
}

const GCRendererCommandList* const GCRenderer_GetCommandList(void)
{
	return Renderer->CommandList;
}

void GCRenderer_ResizeSwapChain(void)
{
	GCRendererDevice_WaitIdle(Renderer->Device);

	GCRendererSwapChain_Recreate(Renderer->SwapChain);
	GCRendererFramebuffer_Recreate(Renderer->Framebuffer);
}

void GCRenderer_RecordCommands(const GCRendererCommandListRecordData* const RecordData)
{
	GCRendererCommandList_BeginRecord(Renderer->CommandList);

	const float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GCRendererCommandList_BeginRenderPass(Renderer->CommandList, Renderer->SwapChain, Renderer->GraphicsPipeline, Renderer->Framebuffer, RecordData, ClearColor);

	GCRendererUniformBufferData UniformBufferData = { 0 };
	UniformBufferData.ViewProjectionMatrix = GCWorldCamera_GetViewProjectionMatrix(Renderer->WorldCamera);

	GCRendererCommandList_UpdateUniformBuffer(Renderer->CommandList, Renderer->UniformBuffer, RecordData, &UniformBufferData, sizeof(GCRendererUniformBufferData));
	GCRendererCommandList_BindGraphicsPipeline(Renderer->CommandList, Renderer->GraphicsPipeline);
	GCRendererCommandList_SetViewport(Renderer->CommandList, Renderer->SwapChain);

	for (uint32_t Counter = 0; Counter < Renderer->DrawDataCount; Counter++)
	{
		GCRendererCommandList_BindVertexBuffer(Renderer->CommandList, Renderer->DrawData[Counter].VertexBuffer);
		GCRendererCommandList_BindIndexBuffer(Renderer->CommandList, Renderer->DrawData[Counter].IndexBuffer);
		GCRendererCommandList_DrawIndexed(Renderer->CommandList, Renderer->DrawData[Counter].IndexCount, 0);
	}

	GCRendererCommandList_EndRenderPass(Renderer->CommandList);
	GCRendererCommandList_EndRecord(Renderer->CommandList);
}