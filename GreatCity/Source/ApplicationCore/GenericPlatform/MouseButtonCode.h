#ifndef GC_APPLICATION_CORE_GENERIC_PLATFORM_MOUSE_BUTTON_CODE_H
#define GC_APPLICATION_CORE_GENERIC_PLATFORM_MOUSE_BUTTON_CODE_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum GCMouseButtonCode
	{
#ifdef GC_PLATFORM_WINDOWS
		GCMouseButtonCode_Left = 0x01,
		GCMouseButtonCode_Right = 0x02,
		GCMouseButtonCode_Middle = 0x04
#endif
	} GCMouseButtonCode;

#ifdef __cplusplus
}
#endif

#endif