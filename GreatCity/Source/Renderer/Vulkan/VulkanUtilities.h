#ifndef GC_RENDERER_VULKAN_VULKAN_UTILITIES_H
#define GC_RENDERER_VULKAN_VULKAN_UTILITIES_H

#ifdef __cplusplus
extern "C"
{
#endif

	#ifndef GC_VULKAN_VALIDATE
		#define GC_VULKAN_VALIDATE(Function, ...) if((Function) != VK_SUCCESS) { GC_LOG_FATAL(__VA_ARGS__); GC_ASSERT(false); }
	#endif

#ifdef __cplusplus
}
#endif

#endif