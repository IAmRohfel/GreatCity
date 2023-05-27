#ifndef GC_APPLICATION_CORE_GENERIC_PLATFORM_INPUT_H
#define GC_APPLICATION_CORE_GENERIC_PLATFORM_INPUT_H

#include "ApplicationCore/GenericPlatform/KeyCode.h"
#include "ApplicationCore/GenericPlatform/MouseButtonCode.h"
#include "Math/Vector2.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

	bool GCInput_IsKeyPressed(const GCKeyCode KeyCode);
	bool GCInput_IsMouseButtonPressed(const GCMouseButtonCode MouseButtonCode);
	GCVector2 GCInput_GetMousePosition(void);

#ifdef __cplusplus
}
#endif

#endif