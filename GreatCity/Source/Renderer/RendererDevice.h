#ifndef GC_RENDERER_RENDERER_DEVICE_H
#define GC_RENDERER_RENDERER_DEVICE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererDevice GCRendererDevice;

	typedef struct GCRendererDeviceCapabilities
	{
		bool IsAnisotropySupported;
		float MaximumAnisotropy;
	} GCRendererDeviceCapabilities;

	GCRendererDevice* GCRendererDevice_Create(void);
	void GCRendererDevice_WaitIdle(const GCRendererDevice* const Device);
	GCRendererDeviceCapabilities GCRendererDevice_GetDeviceCapabilities(const GCRendererDevice* const Device);
	void GCRendererDevice_Destroy(GCRendererDevice* Device);

#ifdef __cplusplus
}
#endif

#endif