#ifndef GC_CORE_MEMORY_ALLOCATOR_H
#define GC_CORE_MEMORY_ALLOCATOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	void* GCMemory_Allocate(const size_t Size);
	void* GCMemory_AllocateZero(const size_t Size);
	void* GCMemory_Reallocate(void* Data, const size_t NewSize);
	void GCMemory_Free(void* Data);

#ifdef __cplusplus
}
#endif

#endif