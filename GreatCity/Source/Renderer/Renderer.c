#include "Renderer/Renderer.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/RendererSwapChain.h"
#include "Renderer/RendererShader.h"
#include "Renderer/RendererGraphicsPipeline.h"
#include "Renderer/RendererFramebuffer.h"
#include "Renderer/RendererCommandList.h"
#include "Core/Memory/Allocator.h"

typedef struct GCRenderer
{
	GCRendererDevice* Device;
	GCRendererSwapChain* SwapChain;
	GCRendererShader* BasicShader;
	GCRendererGraphicsPipeline* GraphicsPipeline;
	GCRendererFramebuffer* Framebuffer;
	GCRendererCommandList* CommandList;
} GCRenderer;

static void GCRenderer_ResizeSwapChain(void);
static void GCRenderer_RecordCommands(const GCRendererCommandListRecordData* const RecordData);

static GCRenderer* Renderer = NULL;

void GCRenderer_Initialize(void)
{
	Renderer = (GCRenderer*)GCMemory_Allocate(sizeof(GCRenderer));
	Renderer->Device = GCRendererDevice_Create();
	Renderer->SwapChain = GCRendererSwapChain_Create(Renderer->Device);
	Renderer->BasicShader = GCRendererShader_Create(Renderer->Device, "Assets/Shader/Basic/Basic.vertex.glsl", "Assets/Shader/Basic/Basic.fragment.glsl");
	Renderer->GraphicsPipeline = GCRendererGraphicsPipeline_Create(Renderer->Device, Renderer->SwapChain, Renderer->BasicShader);
	Renderer->Framebuffer = GCRendererFramebuffer_Create(Renderer->Device, Renderer->SwapChain, Renderer->GraphicsPipeline);
	Renderer->CommandList = GCRendererCommandList_Create(Renderer->Device, Renderer->SwapChain, Renderer->GraphicsPipeline, Renderer->Framebuffer);

	GCRendererCommandList_SetResizeCallback(Renderer->CommandList, GCRenderer_ResizeSwapChain);
}

void GCRenderer_Resize(void)
{
	if (Renderer)
	{
		GCRendererCommandList_SetResize(Renderer->CommandList, true);
	}
}

void GCRenderer_Present(void)
{
	GCRendererCommandList_SubmitAndPresent(Renderer->CommandList, GCRenderer_RecordCommands);
}

void GCRenderer_Terminate(void)
{
	GCRendererDevice_WaitIdle(Renderer->Device);

	GCRendererCommandList_Destroy(Renderer->CommandList);
	GCRendererFramebuffer_Destroy(Renderer->Framebuffer);
	GCRendererGraphicsPipeline_Destroy(Renderer->GraphicsPipeline);
	GCRendererShader_Destroy(Renderer->BasicShader);
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
	GCRendererCommandList_BeginRenderPass(Renderer->CommandList, RecordData, ClearColor);

	GCRendererCommandList_BindGraphicsPipeline(Renderer->CommandList);
	GCRendererCommandList_SetViewport(Renderer->CommandList);

	GCRendererCommandList_Draw(Renderer->CommandList, 3, 0);

	GCRendererCommandList_EndRenderPass(Renderer->CommandList);
	GCRendererCommandList_EndRecord(Renderer->CommandList);
}