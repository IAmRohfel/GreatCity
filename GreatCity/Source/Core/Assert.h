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
#define GC_ASSERT(Check)                                                                                               \
    if (!(Check))                                                                                                      \
    {                                                                                                                  \
        GC_LOG_FATAL("Assertion '%s' failed at %s:%d", #Check, __FILE__, __LINE__);                                    \
        GC_DEBUG_BREAK();                                                                                              \
    }
#endif

#ifndef GC_ASSERT_WITH_MESSAGE
#define GC_ASSERT_WITH_MESSAGE(Check, ...)                                                                             \
    if (!(Check))                                                                                                      \
    {                                                                                                                  \
        GC_LOG_FATAL("Assertion '%s' failed at %s:%d:", #Check, __FILE__, __LINE__);                                   \
        GC_LOG_FATAL(__VA_ARGS__);                                                                                     \
        GC_DEBUG_BREAK();                                                                                              \
    }
#endif
#else
#ifndef GC_ASSERT
#define GC_ASSERT(Check)
#endif

#ifndef GC_ASSERT_WITH_MESSAGE
#define GC_ASSERT_WITH_MESSAGE(Check, ...)
#endif
#endif

#endif