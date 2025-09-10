/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_UTIL_COMMON_H
#define _SCE_PFX_UTIL_COMMON_H

#include <string.h>
#include "../base_level/base/pfx_common.h"

namespace sce {
namespace pfxv4 {

/// @name Utility common

/// Commmon API

/// @brief Memory allocator function
/// @details Function definition for allocating memory with specified alignment.
/// This function is called when utility functions request memory allocation.
/// @param align Alignment bytes
/// @param size Size of memory to be allocated
/// @return Pointer to the allocated memory
typedef void* (*pfxUtilAllocFunc)(size_t align, size_t size);

/// @brief Memory reallocate function
/// @details Function definition for reallocating memory with specified alignment.
/// This function is called when utility functions request memory reallocation.
/// @param ptr Pointer to the allocated memory
/// @param align Alignment bytes
/// @param size Size of memory to be reallocated
/// @return Pointer to the reallocated memory
typedef void* (*pfxUtilReallocFunc)(void* ptr, size_t align, size_t size);

/// @brief Memory deallocate function
/// @details Function definition for deallocating memory.
/// This function is called when utility functions request memory deallocation.
/// @param ptr Pointer to the memory to be deallocated
typedef void  (*pfxUtilDeallocateFunc)(void* ptr);

extern SCE_PFX_API pfxUtilAllocFunc			palloc_func;
extern SCE_PFX_API pfxUtilReallocFunc		prealloc_func;
extern SCE_PFX_API pfxUtilDeallocateFunc	pfree_func;

/// @brief Specify memory allocator used by utility function group
/// @details Sets the memory allocator called by a utility function to the one designated by the user.
/// If the allocator is not specified with this function, the standard functions memalign(), reallocalign() and free() are used for the utility.
/// Specified allocator functions can be used by following macros.
/// @arg SCE_PFX_UTIL_ALLOC(align,size)
/// @arg SCE_PFX_UTIL_REALLOC(ptr,align,size)
/// @arg SCE_PFX_UTIL_FREE(ptr)
/// @param funcAlloc Pointer to the func that allocate memory with a proper boundary
/// @param funcRealloc Pointer to the func that reallocate memory with a proper boundary
/// @param funcFree  Pointer to the func that free memory
void SCE_PFX_API pfxSetUtilityAllocator(pfxUtilAllocFunc funcAlloc, pfxUtilReallocFunc funcRealloc, pfxUtilDeallocateFunc funcFree);

} //namespace pfxv4
} //namespace sce

#define SCE_PFX_UTIL_ALLOC(align,size) 			sce::pfxv4::palloc_func(align,size)
#define SCE_PFX_UTIL_REALLOC(ptr,align,size) 	sce::pfxv4::prealloc_func(ptr,align,size)
#define SCE_PFX_UTIL_FREE(ptr) 					if(ptr){sce::pfxv4::pfree_func(ptr);ptr=NULL;}

#endif // _SCE_PFX_UTIL_COMMON_H
