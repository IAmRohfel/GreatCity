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
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include "Scene/Camera/WorldCamera.h"
#include "Core/Log.h"

#include <stddef.h>

typedef struct GCRenderer
{
	GCRendererModel* BasicTerrainModel;

	GCRendererDevice* Device;
	GCRendererSwapChain* SwapChain;
	GCRendererCommandList* CommandList;
	GCRendererVertexBuffer* VertexBuffer;
	GCRendererIndexBuffer* IndexBuffer;
	GCRendererUniformBuffer* UniformBuffer;
	GCRendererTexture2D* TerrainTexture;
	GCRendererShader* BasicShader;
	GCRendererGraphicsPipeline* GraphicsPipeline;
	GCRendererFramebuffer* Framebuffer;

	const GCWorldCamera* WorldCamera;
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

	const char* ModelPaths[1] =
	{
		"Assets/Models/Terrains/BasicTerrain.obj"
	};

	const char* MaterialPaths[1] =
	{
		"Assets/Models/Terrains"
	};

	Renderer->BasicTerrainModel = GCRendererModel_CreateFromFile(ModelPaths, MaterialPaths, 1);

	Renderer->Device = GCRendererDevice_Create();
	Renderer->SwapChain = GCRendererSwapChain_Create(Renderer->Device);
	Renderer->CommandList = GCRendererCommandList_Create(Renderer->Device);
	Renderer->VertexBuffer = GCRendererVertexBuffer_Create(Renderer->Device, Renderer->CommandList, Renderer->BasicTerrainModel->Vertices, Renderer->BasicTerrainModel->VertexCount * sizeof(GCRendererVertex));
	Renderer->IndexBuffer = GCRendererIndexBuffer_Create(Renderer->Device, Renderer->CommandList, Renderer->BasicTerrainModel->Indices, Renderer->BasicTerrainModel->IndexCount * sizeof(uint32_t));
	Renderer->UniformBuffer = GCRendererUniformBuffer_Create(Renderer->Device, Renderer->CommandList, sizeof(GCRendererUniformBufferData));
	Renderer->TerrainTexture = GCRendererTexture2D_Create(Renderer->Device, Renderer->CommandList, "Assets/Textures/Terrains/BasicTerrain.png");

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

	const GCRendererTexture2D* Texture2Ds[1] = { Renderer->TerrainTexture };
	Renderer->GraphicsPipeline = GCRendererGraphicsPipeline_Create(Renderer->Device, Renderer->SwapChain, Renderer->CommandList, &GraphicsPipelineVertexInput, Renderer->UniformBuffer, Texture2Ds, 1, Renderer->BasicShader);
	Renderer->Framebuffer = GCRendererFramebuffer_Create(Renderer->Device, Renderer->SwapChain, Renderer->GraphicsPipeline);

	GCRendererCommandList_SetResizeCallback(Renderer->CommandList, GCRenderer_ResizeSwapChain);

	Renderer->WorldCamera = WorldCamera;
}

void GCRenderer_Begin(void)
{
}

void GCRenderer_Present(void)
{
	GCRendererCommandList_SubmitAndPresent(Renderer->CommandList, Renderer->SwapChain, GCRenderer_RecordCommands);
}

void GCRenderer_End(void)
{
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
	GCRendererTexture2D_Destroy(Renderer->TerrainTexture);
	GCRendererUniformBuffer_Destroy(Renderer->UniformBuffer);
	GCRendererIndexBuffer_Destroy(Renderer->IndexBuffer);
	GCRendererVertexBuffer_Destroy(Renderer->VertexBuffer);
	GCRendererCommandList_Destroy(Renderer->CommandList);
	GCRendererSwapChain_Destroy(Renderer->SwapChain);
	GCRendererDevice_Destroy(Renderer->Device);

	GCRendererModel_Destroy(Renderer->BasicTerrainModel);

	GCMemory_Free(Renderer);
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

	GCRendererCommandList_BindVertexBuffer(Renderer->CommandList, Renderer->VertexBuffer);
	GCRendererCommandList_BindIndexBuffer(Renderer->CommandList, Renderer->IndexBuffer);
	GCRendererCommandList_BindGraphicsPipeline(Renderer->CommandList, Renderer->GraphicsPipeline);
	GCRendererCommandList_SetViewport(Renderer->CommandList, Renderer->SwapChain);

	GCRendererCommandList_DrawIndexed(Renderer->CommandList, Renderer->BasicTerrainModel->IndexCount, 0);

	GCRendererCommandList_EndRenderPass(Renderer->CommandList);
	GCRendererCommandList_EndRecord(Renderer->CommandList);
}