#ifndef GC_CORE_ASSERT_H
#define GC_CORE_ASSERT_H

#include <intrin.h>

#ifdef GC_ASSERT_ENABLED
	#ifndef GC_DEBUG_BREAK
		#ifdef GC_PLATFORM_WINDOWS
			#define GC_DEBUG_BREAK __debugbreak
		#else
			#define GC_DEBUG_BREAK
		#endif
	#endif

	#ifndef GC_ASSERT
		#define GC_ASSERT(Check) if(!(Check)) { GC_LOG_FATAL("Assertion '%s' failed at %s:%d", #Check, __FILE__, __LINE__); GC_DEBUG_BREAK(); }
	#endif

	#ifndef GC_ASSERT_WITH_MESSAGE
		#define GC_ASSERT_WITH_MESSAGE(Check, ...) if(!(Check)) { GC_LOG_FATAL("Assertion '%s' failed at %s:%d:", #Check, __FILE__, __LINE__); GC_LOG_FATAL(__VA_ARGS__); GC_DEBUG_BREAK(); }
	#endif
#endif

#endif