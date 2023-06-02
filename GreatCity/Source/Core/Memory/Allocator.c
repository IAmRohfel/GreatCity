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

#include "Core/Memory/Allocator.h"

#include <stdlib.h>
#include <stdint.h>

void* GCMemory_Allocate(const size_t Size)
{
	return malloc(Size);
}

void* GCMemory_AllocateZero(const size_t Size)
{
	return calloc(1, Size);
}

void* GCMemory_Reallocate(void* Data, const size_t NewSize)
{
	return realloc(Data, NewSize);
}

void GCMemory_Free(void* Data)
{
	free(Data);
}