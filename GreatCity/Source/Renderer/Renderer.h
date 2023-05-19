#ifndef GC_RENDERER_RENDERER_H
#define GC_RENDERER_RENDERER_H

#ifdef __cplusplus
extern "C"
{
#endif

	void GCRenderer_Initialize(void);
	void GCRenderer_Resize(void);
	void GCRenderer_Present(void);
	void GCRenderer_Terminate(void);

#ifdef __cplusplus
}
#endif

#endif