#ifndef GC_RENDERER_RENDERER_TEXTURE_2D_H
#define GC_RENDERER_RENDERER_TEXTURE_2D_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererTexture2D GCRendererTexture2D;
	typedef struct GCRendererDevice GCRendererDevice;
	typedef struct GCRendererCommandList GCRendererCommandList;

	GCRendererTexture2D* GCRendererTexture2D_Create(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const char* const TexturePath);
	void GCRendererTexture2D_Destroy(GCRendererTexture2D* Texture2D);

#ifdef __cplusplus
}
#endif

#endif