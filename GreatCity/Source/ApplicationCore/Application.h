#ifndef GC_APPLICATION_CORE_APPLICATION_H
#define GC_APPLICATION_CORE_APPLICATION_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCPlatformWindow GCWindow;

	void GCApplication_Create(void);
	void GCApplication_Run(void);
	const GCWindow* const GCApplication_GetWindow(void);
	void GCApplication_Destroy(void);

#ifdef __cplusplus
}
#endif

#endif