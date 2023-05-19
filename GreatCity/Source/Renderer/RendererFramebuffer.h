#ifndef GC_RENDERER_RENDERER_FRAMEBUFFER_H
#define GC_RENDERER_RENDERER_FRAMEBUFFER_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererFramebuffer GCRendererFramebuffer;
	typedef struct GCRendererDevice GCRendererDevice;
	typedef struct GCRendererSwapChain GCRendererSwapChain;
	typedef struct GCRendererGraphicsPipeline GCRendererGraphicsPipeline;

	GCRendererFramebuffer* GCRendererFramebuffer_Create(const GCRendererDevice* const Device, const GCRendererSwapChain* const SwapChain, const GCRendererGraphicsPipeline* const GraphicsPipeline);
	void GCRendererFramebuffer_Recreate(GCRendererFramebuffer* const Framebuffer);
	void GCRendererFramebuffer_Destroy(GCRendererFramebuffer* Framebuffer);

#ifdef __cplusplus
}
#endif

#endif