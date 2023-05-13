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