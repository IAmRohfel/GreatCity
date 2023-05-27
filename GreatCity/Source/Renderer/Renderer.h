#ifndef GC_RENDERER_RENDERER_H
#define GC_RENDERER_RENDERER_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCWorldCamera GCWorldCamera;

	void GCRenderer_Initialize(const GCWorldCamera* const WorldCamera);
	void GCRenderer_Begin(void);
	void GCRenderer_Present(void);
	void GCRenderer_End(void);
	void GCRenderer_Resize(void);
	void GCRenderer_Terminate(void);

#ifdef __cplusplus
}
#endif

#endif