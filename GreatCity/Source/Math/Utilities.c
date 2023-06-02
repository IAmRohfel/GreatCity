/*
    Copyright (C) 2023  Rohfel Adyaraka Christianugrah Puspoasmoro

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
