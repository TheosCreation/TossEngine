/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_COMMON_H
#define _SCE_PFX_COMMON_H

// Include common headers
#ifdef _WIN32
	#include <windows.h>
	#include <stdio.h>
	#include <tchar.h>
	#pragma warning(disable: 4201) // nonstandard extension used : nameless struct/union
#else
	#include <stdio.h>
	#include <stdint.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "pfx_vectormath_include.h"

#ifdef _WIN32
#if defined(SCE_PFX_LIB_EXPORTS)
#define SCE_PFX_API
#elif defined(SCE_PFX_DLL_EXPORTS)
#define SCE_PFX_API __declspec(dllexport)
#else
#define SCE_PFX_API __declspec(dllimport)
#endif
#else
#define SCE_PFX_API
#endif

// Optional
#define SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
//#define SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS

#ifndef SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS
	#undef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
#endif

#define SCE_PFX_ENABLE_CONTACT_FILTER_METHOD_AND
#define SCE_PFX_ENABLE_IGNORE_GROUP_AS_EXCEPTION

namespace sce {
namespace pfxv4 {
// Basic Types
#if defined(_WIN32)
	typedef char               PfxInt8;
	typedef unsigned char      PfxUInt8;
	typedef short              PfxInt16;
	typedef unsigned short     PfxUInt16;
	typedef int                PfxInt32;
	typedef unsigned int       PfxUInt32;
	typedef long long          PfxInt64;
	typedef unsigned long long PfxUInt64;
#else
	typedef int8_t             PfxInt8;
	typedef uint8_t            PfxUInt8;
	typedef int16_t            PfxInt16;
	typedef uint16_t           PfxUInt16;
	typedef int32_t            PfxInt32;
	typedef uint32_t           PfxUInt32;
	typedef int64_t            PfxInt64;
	typedef uint64_t           PfxUInt64;
#endif

typedef bool                        PfxBool;
typedef float                       PfxFloat;

} //namespace pfxv4
} //namespace sce

// Debug Print
#ifdef _WIN32
static void pfxOutputDebugString(const char *str, ...)
{
    char strDebug[1024]={0};
    va_list argList;
    va_start(argList, str);
    vsprintf_s(strDebug,str,argList);
	OutputDebugStringA(strDebug);
    va_end(argList);
}
#endif

// printf
//#if defined(_DEBUG) || defined(SCE_PFX_VERIFY_BUILD)
	#if defined(_WIN32)
		#define SCE_PFX_PRINTF pfxOutputDebugString
	#else
		#define SCE_PFX_PRINTF(...) printf(__VA_ARGS__)
	#endif
//#else
//	#ifdef _WIN32
//		#define SCE_PFX_PRINTF
//	#else
//		#define SCE_PFX_PRINTF(...)
//	#endif
//#endif

// sprintf
#if defined(_WIN32)
	#define SCE_PFX_SNPRINTF(B, S, ...) _snprintf_s((char*)B,(S),(S)-2, __VA_ARGS__)
#else
	#define SCE_PFX_SNPRINTF(...) snprintf(__VA_ARGS__)
#endif

// Hint
#if defined(_WIN32)
	#define SCE_PFX_UNLIKELY(a) (a)
	#define SCE_PFX_LIKELY(a) (a)
#else
	#define SCE_PFX_UNLIKELY(a)		__builtin_expect((a),0)
	#define SCE_PFX_LIKELY(a)		__builtin_expect((a),1)
#endif

// Inline
#ifdef _DEBUG
    #define SCE_PFX_FORCE_INLINE inline
#else
#if defined(_MSC_VER)
	#define SCE_PFX_FORCE_INLINE __forceinline
#elif defined(__SNC__) || defined(__GNUC__)
	#define SCE_PFX_FORCE_INLINE inline __attribute__((always_inline))
#endif
#endif

// Assert
#if defined(_MSC_VER)
	#define SCE_PFX_HALT() __debugbreak()
#elif defined(__ORBIS__) || defined(__PROSPERO__)
	#define SCE_PFX_HALT() __debugbreak()
#else
	#define SCE_PFX_HALT() abort()
#endif

#if defined(_DEBUG) || defined(SCE_PFX_VERIFY_BUILD)
	#define SCE_PFX_ASSERT(test) {if(!(test)){SCE_PFX_PRINTF("Assert " __FILE__ ":%d ("#test")\n", __LINE__);SCE_PFX_HALT();}}
	#define SCE_PFX_ASSERT_MSG(test,msg) {if(!(test)){SCE_PFX_PRINTF("Assert " msg " " __FILE__ ":%d ("#test")\n",__LINE__);SCE_PFX_HALT();}}
#else
	#define SCE_PFX_ASSERT(test)
	#define SCE_PFX_ASSERT_MSG(test,msg)
#endif

#define SCE_PFX_ALWAYS_ASSERT(test) {if(!(test)){SCE_PFX_PRINTF("Assert " __FILE__ ":%d ("#test")\n", __LINE__);SCE_PFX_HALT();}}
#define SCE_PFX_ALWAYS_ASSERT_MSG(test,msg) {if(!(test)){SCE_PFX_PRINTF("Assert:" msg " " __FILE__ ":%d ("#test")\n",__LINE__);SCE_PFX_HALT();}}

// Verify float values
#if defined(_DEBUG) || defined(SCE_PFX_VERIFY_BUILD)
#define SCE_PFX_VALIDATE_FLOAT(f) {SCE_PFX_ASSERT(!isnan((float)f) && !isinf((float)f))}
	#define SCE_PFX_VALIDATE_VECTOR3(v) {SCE_PFX_VALIDATE_FLOAT(v[0]);SCE_PFX_VALIDATE_FLOAT(v[1]);SCE_PFX_VALIDATE_FLOAT(v[2])}
	#define SCE_PFX_VALIDATE_QUAT(q) {SCE_PFX_VALIDATE_FLOAT(q[0]);SCE_PFX_VALIDATE_FLOAT(q[1]);SCE_PFX_VALIDATE_FLOAT(q[2]);SCE_PFX_VALIDATE_FLOAT(q[3])}
	#define SCE_PFX_VALIDATE_MATRIX3(m) {SCE_PFX_VALIDATE_VECTOR3(m.getCol0());SCE_PFX_VALIDATE_VECTOR3(m.getCol1());SCE_PFX_VALIDATE_VECTOR3(m.getCol2());}
#else
	#define SCE_PFX_VALIDATE_FLOAT(f) 
	#define SCE_PFX_VALIDATE_VECTOR3(v) 
	#define SCE_PFX_VALIDATE_QUAT(q)
	#define SCE_PFX_VALIDATE_MATRIX3(m)
#endif

// Aligned 
#if defined(_MSC_VER)
	#define SCE_PFX_ALIGNED(alignment)   __declspec(align(alignment))
#elif defined(__SNC__) || defined(__GNUC__)
	#define SCE_PFX_ALIGNED(alignment)   __attribute__((__aligned__((alignment))))
#endif

// Etc
#define SCE_PFX_MIN(a,b) (((a)<(b))?(a):(b))
#define SCE_PFX_MAX(a,b) (((a)>(b))?(a):(b))
#define SCE_PFX_CLAMP(v,a,b) SCE_PFX_MAX(a,SCE_PFX_MIN(v,b))
#define SCE_PFX_SWAP(type, x, y) do {type t; t=x; x=y; y=t; } while (0)
#define SCE_PFX_SQR(a) ((a)*(a))

#define SCE_PFX_ALIGN16(count,size)   ((((((count) * (size)) + 15ul) & (~15ul)) + (size)-1) / (size))
#define SCE_PFX_ALIGN128(count,size)  ((((((count) * (size)) + 127ul) & (~127ul)) + (size)-1) / (size))

#define SCE_PFX_AVAILABLE_BYTES_ALIGN16(ptr,bytes) (bytes-((uintptr_t)(ptr)&0x0ful))
#define SCE_PFX_AVAILABLE_BYTES_ALIGN128(ptr,bytes) (bytes-((uintptr_t)(ptr)&0x7ful))

#define SCE_PFX_BYTES_ALIGN4(bytes) (((bytes)+3ul)&(~3ul))
#define SCE_PFX_BYTES_ALIGN8(bytes) (((bytes)+7ul)&(~7ul))
#define SCE_PFX_BYTES_ALIGN16(bytes) (((bytes)+15ul)&(~15ul))
#define SCE_PFX_BYTES_ALIGN128(bytes) (((bytes)+127ul)&(~127ul))
#define SCE_PFX_BYTES_ALIGN256(bytes) (((bytes)+255ul)&(~255ul))
#define SCE_PFX_BYTES_ALIGN_X(bytes, x) (((bytes)+(uintptr_t)(x-1))&(~(uintptr_t)(x-1)))

#define SCE_PFX_PTR_ALIGN16(ptr) (((uintptr_t)(ptr)+15ull)&(~15ull))
#define SCE_PFX_PTR_ALIGN128(ptr) (((uintptr_t)(ptr)+127ull)&(~127ull))
#define SCE_PFX_PTR_ALIGN256(ptr) (((uintptr_t)(ptr)+255ull)&(~255ull))
#define SCE_PFX_PTR_ALIGN_X(ptr, x) (((uintptr_t)(ptr)+(uintptr_t)(x-1))&(~(uintptr_t)(x-1)))

#define SCE_PFX_PTR_IS_ALIGNED16(ptr) (((uintptr_t)(ptr)&0x0full)==0)
#define SCE_PFX_PTR_IS_ALIGNED128(ptr) (((uintptr_t)(ptr)&0x7full)==0)

#define SCE_PFX_GET_POINTER(offset,stride,id) ((uintptr_t)(offset)+(stride)*(id))

#define SCE_PFX_FLT_MAX 1e+38f
#define SCE_PFX_INT_MAX  2147483647
#define SCE_PFX_INT_MIN (-2147483647 -1)
#define SCE_PFX_PI 3.14159265358979f
#define SCE_PFX_DEG_TO_RAD 0.017453292519943f
#define SCE_PFX_MAX_RIGIDBODY 65536
#define SCE_PFX_RANGE_CHECK(val,minVal,maxVal) (((val)>=(minVal))&&((val)<=(maxVal)))

#define SCE_PFX_IS_RUNNING_ON_64BIT_ENV() ( ( sizeof(void*)==8 )? true : false )

#if defined(__SNC__) || defined(__GNUC__) || defined(__ORBIS__) || defined(__PROSPERO__)
	#define SCE_PFX_PADDING(count,bytes) PfxUInt8 padding##count[bytes];
	#define SCE_PFX_PADDING1	uint8_t : 8;
	#define SCE_PFX_PADDING2	uint16_t : 16;
	#define SCE_PFX_PADDING3	SCE_PFX_PADDING1;SCE_PFX_PADDING2
	#define SCE_PFX_PADDING4	uint32_t : 32;
	#define SCE_PFX_PADDING7	SCE_PFX_PADDING3;SCE_PFX_PADDING4
	#define SCE_PFX_PADDING8	uint64_t : 64;
	#define SCE_PFX_PADDING12	SCE_PFX_PADDING4;SCE_PFX_PADDING4;SCE_PFX_PADDING4
#else
	#define SCE_PFX_PADDING(count,bytes)
	#define SCE_PFX_PADDING1	
	#define SCE_PFX_PADDING2	
	#define SCE_PFX_PADDING3	
	#define SCE_PFX_PADDING4	
	#define SCE_PFX_PADDING7	
	#define SCE_PFX_PADDING8	
	#define SCE_PFX_PADDING12	
#endif

#define SCE_PFX_EXCLUDE_DOC(code) code

#if defined(__ORBIS__) || defined(__PROSPERO__)
	#define SCE_PFX_DEPRECATED(func_name) __attribute__((deprecated("This function will be deprecated. Please use '" #func_name "' instead.")))
#else
	#define SCE_PFX_DEPRECATED(func_name) __declspec(deprecated("This function will be deprecated. Please use '" #func_name "' instead."))
#endif

// Optimization
#if defined(_MSC_VER)
	#define SCE_PFX_PUSH_DISABLE_OPTIMIZATIONS __pragma(optimize("", off))
	#define SCE_PFX_POP_DISABLE_OPTIMIZATIONS __pragma(optimize("", on))
#elif defined(__SNC__) || defined(__GNUC__)
	#define SCE_PFX_PUSH_DISABLE_OPTIMIZATIONS _Pragma("clang optimize off")
	#define SCE_PFX_POP_DISABLE_OPTIMIZATIONS _Pragma("clang optimize on")
#endif

#ifdef _MSC_VER
inline unsigned int __builtin_ffs(unsigned int x) { unsigned long r; return _BitScanForward(&r, x) ? r + 1 : 0; }
#endif

template <class T> void pfxSwap(T &a, T &b) 
{
	T c = a;
	a = b;
	b = c;
}

#include "pfx_error_code.h"

#endif // _SCE_PFX_COMMON_H
