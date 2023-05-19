#ifndef GC_RENDERER_RENDERER_SWAP_CHAIN_H
#define GC_RENDERER_RENDERER_SWAP_CHAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererSwapChain GCRendererSwapChain;
	typedef struct GCRendererDevice GCRendererDevice;

	GCRendererSwapChain* GCRendererSwapChain_Create(const GCRendererDevice* const Device);
	void GCRendererSwapChain_Recreate(GCRendererSwapChain* const SwapChain);
	void GCRendererSwapChain_Destroy(GCRendererSwapChain* SwapChain);

#ifdef __cplusplus
}
#endif

#endif