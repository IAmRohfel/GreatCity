#include "Scene/Camera/WorldCamera.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Matrix4x4.h"
#include "Math/Quaternion.h"
#include "Math/Utilities.h"
#include "Core/Memory/Allocator.h"
#include "ApplicationCore/GenericPlatform/Window.h"
#include "ApplicationCore/GenericPlatform/Input.h"
#include "ApplicationCore/GenericPlatform/KeyCode.h"
#include "ApplicationCore/GenericPlatform/MouseButtonCode.h"
#include "ApplicationCore/Event/Event.h"
#include "ApplicationCore/Event/MouseEvent.h"
#include "ApplicationCore/Application.h"

#include <math.h>
#include <stdint.h>

typedef struct GCWorldCamera
{
	GCMatrix4x4 ViewMatrix, ProjectionMatrix;

	GCVector2 InitialMousePosition;
	GCVector3 Position, FocalPoint;

	float FoV, AspectRatio, Distance, Near, Far;
	float Pitch, Yaw;

	float Width, Height;
} GCWorldCamera;

static void GCWorldCamera_UpdateView(GCWorldCamera* const WorldCamera);
static void GCWorldCamera_UpdateProjection(GCWorldCamera* const WorldCamera);

static bool GCWorldCamera_OnMouseScrolled(GCEvent* const Event, void* CustomData);

static void GCWorldCamera_Pan(GCWorldCamera* const WorldCamera, const GCVector2 MouseDelta);
static void GCWorldCamera_Rotate(GCWorldCamera* const WorldCamera, const GCVector2 MouseDelta);
static void GCWorldCamera_Zoom(GCWorldCamera* const WorldCamera, const float MouseDelta);

static void GCWorldCamera_GetPanSpeed(const GCWorldCamera* const WorldCamera, float* const XSpeed, float* const YSpeed);
static float GCWorldCamera_GetRotationSpeed(const GCWorldCamera* const WorldCamera);
static float GCWorldCamera_GetZoomSpeed(const GCWorldCamera* const WorldCamera);

static GCVector3 GCWorldCamera_CalculatePosition(GCWorldCamera* const WorldCamera);
static GCQuaternion GCWorldCamera_GetOrientation(GCWorldCamera* const WorldCamera);
static GCVector3 GCWorldCamera_GetUpDirection(GCWorldCamera* const WorldCamera);
static GCVector3 GCWorldCamera_GetRightDirection(GCWorldCamera* const WorldCamera);
static GCVector3 GCWorldCamera_GetForwardDirection(GCWorldCamera* const WorldCamera);

GCWorldCamera* GCWorldCamera_Create(const float FoV, const float AspectRatio, const float Near, const float Far)
{
	GCWorldCamera* WorldCamera = (GCWorldCamera*)GCMemory_Allocate(sizeof(GCWorldCamera));

	uint32_t WindowWidth = 0, WindowHeight = 0;
	GCWindow_GetWindowSize(GCApplication_GetWindow(), &WindowWidth, &WindowHeight);

	WorldCamera->ViewMatrix = GCMatrix4x4_CreateIdentity();
	WorldCamera->ProjectionMatrix = GCMatrix4x4_CreatePerspective(GCMathUtilities_DegreesToRadians(FoV), AspectRatio, Near, Far);
	WorldCamera->InitialMousePosition = GCVector2_CreateZero();
	WorldCamera->Position = GCVector3_CreateZero();
	WorldCamera->FocalPoint = GCVector3_CreateZero();
	WorldCamera->FoV = 45.0f;
	WorldCamera->AspectRatio = (float)WindowWidth / (float)WindowHeight;
	WorldCamera->Distance = 10.0f;
	WorldCamera->Near = 0.1f;
	WorldCamera->Far = 1000.0f;
	WorldCamera->Pitch = 0.0f;
	WorldCamera->Yaw = 0.0f;
	WorldCamera->Width = (float)WindowWidth;
	WorldCamera->Height = (float)WindowHeight;

	GCWorldCamera_UpdateView(WorldCamera);

	return WorldCamera;
}

void GCWorldCamera_Update(GCWorldCamera* const WorldCamera)
{
	if (GCInput_IsKeyPressed(GCKeyCode_LeftAlt))
	{
		const GCVector2 CurrentMousePosition = GCInput_GetMousePosition();
		const GCVector2 DeltaMousePosition = GCVector2_MultiplyByScalar(GCVector2_Subtract(CurrentMousePosition, WorldCamera->InitialMousePosition), 0.003f);
		WorldCamera->InitialMousePosition = CurrentMousePosition;

		if (GCInput_IsMouseButtonPressed(GCMouseButtonCode_Left))
		{
			GCWorldCamera_Pan(WorldCamera, DeltaMousePosition);
		}
		else if (GCInput_IsMouseButtonPressed(GCMouseButtonCode_Right))
		{
			GCWorldCamera_Rotate(WorldCamera, DeltaMousePosition);
		}
	}

	GCWorldCamera_UpdateView(WorldCamera);
}

void GCWorldCemera_OnEvent(GCWorldCamera* const WorldCamera, GCEvent* const Event)
{
	GCEvent_Dispatch(GCEventType_MouseScrolled, Event, GCWorldCamera_OnMouseScrolled, (void*)WorldCamera);
}

void GCWorldCamera_SetSize(GCWorldCamera* const WorldCamera, const uint32_t Width, const uint32_t Height)
{
	WorldCamera->Width = (float)Width;
	WorldCamera->Height = (float)Height;

	GCWorldCamera_UpdateProjection(WorldCamera);
}

GCMatrix4x4 GCWorldCamera_GetViewProjectionMatrix(const GCWorldCamera* const WorldCamera)
{
	return GCMatrix4x4_Multiply(&WorldCamera->ProjectionMatrix, &WorldCamera->ViewMatrix);
}

void GCWorldCamera_UpdateView(GCWorldCamera* const WorldCamera)
{
	WorldCamera->Position = GCWorldCamera_CalculatePosition(WorldCamera);

	const GCQuaternion Orientation = GCWorldCamera_GetOrientation(WorldCamera);
	const GCMatrix4x4 OrientationAsRotationMatrix = GCQuaternion_ToRotationMatrix(Orientation);

	WorldCamera->ViewMatrix = GCMatrix4x4_CreateTranslation(WorldCamera->Position);
	WorldCamera->ViewMatrix = GCMatrix4x4_Multiply(&WorldCamera->ViewMatrix, &OrientationAsRotationMatrix);
	WorldCamera->ViewMatrix = GCMatrix4x4_Inverse(&WorldCamera->ViewMatrix);
}

void GCWorldCamera_UpdateProjection(GCWorldCamera* const WorldCamera)
{
	WorldCamera->AspectRatio = WorldCamera->Width / WorldCamera->Height;
	WorldCamera->ProjectionMatrix = GCMatrix4x4_CreatePerspective(GCMathUtilities_DegreesToRadians(WorldCamera->FoV), WorldCamera->AspectRatio, WorldCamera->Near, WorldCamera->Far);
}

bool GCWorldCamera_OnMouseScrolled(GCEvent* const Event, void* CustomData)
{
	const GCMouseScrolledEvent* const EventDetail = (const GCMouseScrolledEvent* const)Event->EventDetail;
	GCWorldCamera* const WorldCamera = (GCWorldCamera* const)CustomData;

	const float MouseDelta = EventDetail->YOffset * 0.1f;
	GCWorldCamera_Zoom(WorldCamera, MouseDelta);
	GCWorldCamera_UpdateView(WorldCamera);

	return false;
}

void GCWorldCamera_Pan(GCWorldCamera* const WorldCamera, const GCVector2 MouseDelta)
{
	float XSpeed = 0.0f, YSpeed = 0.0f;
	GCWorldCamera_GetPanSpeed(WorldCamera, &XSpeed, &YSpeed);

	const GCVector3 NegativeRightDirection = GCVector3_MultiplyByScalar(GCWorldCamera_GetRightDirection(WorldCamera), -1.0f);

	WorldCamera->FocalPoint = GCVector3_Add(WorldCamera->FocalPoint, GCVector3_MultiplyByScalar(GCVector3_MultiplyByScalar(GCVector3_MultiplyByScalar(NegativeRightDirection, MouseDelta.X), XSpeed), WorldCamera->Distance));
	WorldCamera->FocalPoint = GCVector3_Add(WorldCamera->FocalPoint, GCVector3_MultiplyByScalar(GCVector3_MultiplyByScalar(GCVector3_MultiplyByScalar(GCWorldCamera_GetUpDirection(WorldCamera), MouseDelta.Y), YSpeed), WorldCamera->Distance));
}

void GCWorldCamera_Rotate(GCWorldCamera* const WorldCamera, const GCVector2 MouseDelta)
{
	const float YawSign = GCWorldCamera_GetUpDirection(WorldCamera).Y < 0.0f ? 1.0f : -1.0f;

	WorldCamera->Pitch += -MouseDelta.Y * GCWorldCamera_GetRotationSpeed(WorldCamera);
	WorldCamera->Yaw += YawSign * MouseDelta.X * GCWorldCamera_GetRotationSpeed(WorldCamera);
}

void GCWorldCamera_Zoom(GCWorldCamera* const WorldCamera, const float MouseDelta)
{
	WorldCamera->Distance -= MouseDelta * GCWorldCamera_GetZoomSpeed(WorldCamera);

	if (WorldCamera->Distance < 1.0f)
	{
		WorldCamera->FocalPoint = GCVector3_Add(WorldCamera->FocalPoint, GCWorldCamera_GetForwardDirection(WorldCamera));
		WorldCamera->Distance = 1.0f;
	}
}

void GCWorldCamera_GetPanSpeed(const GCWorldCamera* const WorldCamera, float* const XSpeed, float* const YSpeed)
{
	const float X = fminf(WorldCamera->Width / 1000.0f, 5.0f);
	*XSpeed = 0.025f * (X * X) - (0.0f * X) + 0.0f;

	const float Y = fminf(WorldCamera->Height / 1000.0f, 5.0f);
	*YSpeed = 0.075f * (Y * Y) - (0.0f * Y) + 0.0f;
}

float GCWorldCamera_GetRotationSpeed(const GCWorldCamera* const WorldCamera)
{
	(void)WorldCamera;

	return 0.8f;
}

float GCWorldCamera_GetZoomSpeed(const GCWorldCamera* const WorldCamera)
{
	float Distance = WorldCamera->Distance * 0.2f;
	Distance = fmaxf(Distance, 0.0f);

	float Speed = Distance * Distance;
	Speed = fminf(Speed, 100.0f);

	return Speed;
}

GCVector3 GCWorldCamera_CalculatePosition(GCWorldCamera* const WorldCamera)
{
	return GCVector3_MultiplyByScalar(GCVector3_Subtract(WorldCamera->FocalPoint, GCWorldCamera_GetForwardDirection(WorldCamera)), WorldCamera->Distance);
}

GCQuaternion GCWorldCamera_GetOrientation(GCWorldCamera* const WorldCamera)
{
	return GCQuaternion_CreateFromEulerAngles(-WorldCamera->Pitch, -WorldCamera->Yaw, 0.0f);
}

GCVector3 GCWorldCamera_GetUpDirection(GCWorldCamera* const WorldCamera)
{
	return GCQuaternion_RotateVector(GCWorldCamera_GetOrientation(WorldCamera), GCVector3_Create(0.0f, -1.0f, 0.0f));
}

GCVector3 GCWorldCamera_GetRightDirection(GCWorldCamera* const WorldCamera)
{
	return GCQuaternion_RotateVector(GCWorldCamera_GetOrientation(WorldCamera), GCVector3_Create(1.0f, 0.0f, 0.0f));
}

GCVector3 GCWorldCamera_GetForwardDirection(GCWorldCamera* const WorldCamera)
{
	return GCQuaternion_RotateVector(GCWorldCamera_GetOrientation(WorldCamera), GCVector3_Create(0.0f, 0.0f, -1.0f));
}