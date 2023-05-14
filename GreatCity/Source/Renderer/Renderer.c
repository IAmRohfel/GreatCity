#include "Renderer/Renderer.h"
#include "Renderer/RendererDevice.h"
#include "Core/Memory/Allocator.h"

typedef struct GCRenderer
{
	GCRendererDevice* Device;
} GCRenderer;

static GCRenderer* Renderer = NULL;

void GCRenderer_Initialize(void)
{
	Renderer = (GCRenderer*)GCMemory_Allocate(sizeof(GCRenderer));
	Renderer->Device = GCRendererDevice_Create();
}

void GCRenderer_Present(void)
{
}

void GCRenderer_Terminate(void)
{
	GCRendererDevice_Destroy(Renderer->Device);

	GCMemory_Free(Renderer);
}
