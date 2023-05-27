#include "Renderer/Renderer.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/RendererSwapChain.h"
#include "Renderer/RendererCommandList.h"
#include "Renderer/RendererVertexBuffer.h"
#include "Renderer/RendererIndexBuffer.h"
#include "Renderer/RendererUniformBuffer.h"
#include "Renderer/RendererShader.h"
#include "Renderer/RendererGraphicsPipeline.h"
#include "Renderer/RendererFramebuffer.h"
#include "Core/Memory/Allocator.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include "Scene/Camera/WorldCamera.h"
#include "Core/Log.h"

typedef struct GCRenderer
{
	GCRendererDevice* Device;
	GCRendererSwapChain* SwapChain;
	GCRendererCommandList* CommandList;
	GCRendererVertexBuffer* VertexBuffer;
	GCRendererIndexBuffer* IndexBuffer;
	GCRendererUniformBuffer* UniformBuffer;
	GCRendererShader* BasicShader;
	GCRendererGraphicsPipeline* GraphicsPipeline;
	GCRendererFramebuffer* Framebuffer;

	const GCWorldCamera* WorldCamera;
} GCRenderer;

typedef struct GCRendererVertex
{
	GCVector3 Position;
	GCVector4 Color;
} GCRendererVertex;

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

	GCRendererVertex Vertices[4];
	Vertices[0].Position = GCVector3_Create(-0.5f, -0.5f, 0.0f);
	Vertices[0].Color = GCVector4_Create(1.0f, 0.0f, 0.0f, 1.0f);
	Vertices[1].Position = GCVector3_Create(0.5f, -0.5f, 0.0f);
	Vertices[1].Color = GCVector4_Create(1.0f, 0.0f, 0.0f, 1.0f);
	Vertices[2].Position = GCVector3_Create(0.5f, 0.5f, 0.0f);
	Vertices[2].Color = GCVector4_Create(1.0f, 0.0f, 0.0f, 1.0f);
	Vertices[3].Position = GCVector3_Create(-0.5f, 0.5f, 0.0f);
	Vertices[3].Color = GCVector4_Create(1.0f, 0.0f, 0.0f, 1.0f);

	Renderer->VertexBuffer = GCRendererVertexBuffer_Create(Renderer->Device, Renderer->CommandList, Vertices, sizeof(Vertices));

	const uint32_t Indices[6] = { 0, 1, 2, 2, 3, 0 };

	Renderer->IndexBuffer = GCRendererIndexBuffer_Create(Renderer->Device, Renderer->CommandList, Indices, sizeof(Indices));
	Renderer->UniformBuffer = GCRendererUniformBuffer_Create(Renderer->Device, Renderer->CommandList, sizeof(GCRendererUniformBufferData));

	Renderer->BasicShader = GCRendererShader_Create(Renderer->Device, "Assets/Shader/Basic/Basic.vertex.glsl", "Assets/Shader/Basic/Basic.fragment.glsl");

	GCRendererGraphicsPipelineVertexInput GraphicsPipelineVertexInput = { 0 };
	GraphicsPipelineVertexInput.Stride = sizeof(GCRendererVertex);

	GCRendererGraphicsPipelineVertexInputAttribute GraphicsPipelineVertexInputAttributes[2] = { 0 };
	GraphicsPipelineVertexInputAttributes[0].Location = 0;
	GraphicsPipelineVertexInputAttributes[0].Format = GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector3;
	GraphicsPipelineVertexInputAttributes[0].Offset = 0;

	GraphicsPipelineVertexInputAttributes[1].Location = 1;
	GraphicsPipelineVertexInputAttributes[1].Format = GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector4;
	GraphicsPipelineVertexInputAttributes[1].Offset = 12;

	GraphicsPipelineVertexInput.Attributes = GraphicsPipelineVertexInputAttributes;
	GraphicsPipelineVertexInput.AttributeCount = 2;

	Renderer->GraphicsPipeline = GCRendererGraphicsPipeline_Create(Renderer->Device, Renderer->SwapChain, &GraphicsPipelineVertexInput, Renderer->UniformBuffer, Renderer->BasicShader);
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
	GCRendererUniformBuffer_Destroy(Renderer->UniformBuffer);
	GCRendererIndexBuffer_Destroy(Renderer->IndexBuffer);
	GCRendererVertexBuffer_Destroy(Renderer->VertexBuffer);
	GCRendererCommandList_Destroy(Renderer->CommandList);
	GCRendererSwapChain_Destroy(Renderer->SwapChain);
	GCRendererDevice_Destroy(Renderer->Device);

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
	GCRendererCommandList_BindUniformBuffer(Renderer->CommandList, Renderer->GraphicsPipeline, Renderer->UniformBuffer);
	GCRendererCommandList_BindGraphicsPipeline(Renderer->CommandList, Renderer->GraphicsPipeline);
	GCRendererCommandList_SetViewport(Renderer->CommandList, Renderer->SwapChain);

	GCRendererCommandList_DrawIndexed(Renderer->CommandList, 6, 0);

	GCRendererCommandList_EndRenderPass(Renderer->CommandList);
	GCRendererCommandList_EndRecord(Renderer->CommandList);
}