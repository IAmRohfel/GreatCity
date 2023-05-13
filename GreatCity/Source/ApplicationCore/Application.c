#include "ApplicationCore/Application.h"
#include "ApplicationCore/GenericWindow.h"
#include "Core/Memory/Allocator.h"

#include <stdbool.h>

typedef struct GCApplication
{
	GCGenericWindow* Window;
} GCApplication;

static GCApplication* Application = NULL;

void GCApplication_Create(void)
{
	Application = (GCApplication*)GCMemory_Allocate(sizeof(GCApplication));

	GCGenericWindowProperties WindowProperties;
	WindowProperties.Title = "Great City";
	WindowProperties.Width = 1280;
	WindowProperties.Height = 720;

	Application->Window = GCGenericWindow_Create(&WindowProperties);
}

void GCApplication_Run(void)
{
	while (true)
	{
		GCGenericWindow_ProcessEvents(Application->Window);
	}
}

void GCApplication_Destroy(void)
{
	GCGenericWindow_Destroy(Application->Window);
}
