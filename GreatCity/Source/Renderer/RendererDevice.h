#ifndef GC_RENDERER_RENDERER_DEVICE_H
#define GC_RENDERER_RENDERER_DEVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererDevice GCRendererDevice;

	GCRendererDevice* GCRendererDevice_Create(void);
	void GCRendererDevice_WaitIdle(const GCRendererDevice* const Device);
	void GCRendererDevice_Destroy(GCRendererDevice* Device);

#ifdef __cplusplus
}
#endif

#endif