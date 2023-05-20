#include "Math/Utilities.h"

#define _USE_MATH_DEFINES
#include <math.h>

float GCMathUtilities_DegreesToRadians(const float Degrees)
{
	return Degrees * (float)M_PI / 180.0f;
}

float GCMathUtilities_RadiansToDegrees(const float Radians)
{
	return Radians * 180.0f / (float)M_PI;
}
