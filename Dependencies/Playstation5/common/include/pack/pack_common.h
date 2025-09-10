/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2023 Sony Interactive Entertainment Inc.
* 
*/

#ifndef PACK_FILE_COMMON_INCLUDE_GUARD
#define PACK_FILE_COMMON_INCLUDE_GUARD

#if __clang__
#define RESTRICT_PTR restrict
#else
#define RESTRICT_PTR 
#endif

#ifndef ENABLE_ASSERTS
#ifndef NDEBUG
#define ENABLE_ASSERTS 1
#else
#define ENABLE_ASSERTS 0
#endif
#endif

#ifndef USE_STD_ASSERT
#define USE_STD_ASSERT 0
#endif

#include <stdio.h> // printf

#if _MSC_VER
#define WIN32_LEAN_AND_MEAN 
#define NOMINMAX

#include <Windows.h>
#include <malloc.h>
#include <crtdbg.h>
#endif

#if USE_STD_ASSERT
#include <cassert>
#endif

#if (defined(__PROSPERO__) || defined(__ORBIS__))
#include <scebase_common.h>
#if !USE_STD_ASSERT
#include <libdbg.h>
#endif
#else
#ifndef SCE_OK
#define SCE_OK 0
#endif
#endif

#if ENABLE_ASSERTS

# if USE_STD_ASSERT
#  define PACK_ASSERT(x) assert(x)
#  define PACK_VERIFY(x) assert(x)
# else
    inline void pack_assert(bool test, const char* file_name, int line, const char* expression_name)
    {
        (void)file_name;
        (void)line;
        (void)expression_name;
        if (test) return;
#  if _MSC_VER
        if (_CrtDbgReport(_CRT_ASSERT, file_name, line, "PackFormat", "%s", expression_name))
            DebugBreak();
#  elif defined(__PROSPERO__)
        SCE_BREAK();
#  elif defined(__ORBIS__)
        SCE_BREAK();
#  else
#   error "Unknown platform"
#  endif
    }

#  define PACK_ASSERT(x) pack_assert(x, __FILE__, __LINE__, #x)
#  define PACK_VERIFY(x) pack_assert(x, __FILE__, __LINE__, #x)
# endif // USE_STD_ASSERT

# define DEBUG_ONLY(x) x

#else  // -

# if _MSC_VER
#  define PACK_ASSERT(x) (__assume(x))
# else
#  if __has_builtin(__builtin_assume)
#   define PACK_ASSERT(x) (__builtin_assume(x))
#  else
#   define PACK_ASSERT(x)
#  endif
# endif
# define PACK_VERIFY(x) (void)(x)
# define DEBUG_ONLY(x)

#endif // ENABLE_ASSERTS

inline void report_error(const char* message)
{
    fprintf(stderr, "Error: %s", message);
}

inline void report_warning(const char* message)
{
    fprintf(stderr, "Warning: %s", message);
}

#include "array.h"
#include "error_codes.h"
#include "obj_serialization.h"

#endif // PACK_FILE_COMMON_INCLUDE_GUARD