/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2022 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../include/physics_effects/util/pfx_util_common.h"
#include "../../include/physics_effects/base_level/base/pfx_common.h"

namespace sce {
namespace pfxv4 {

static void* stub_malloc(size_t align, size_t size)
{
#ifdef _WIN32
	return _aligned_malloc(size, align);
#else
	if (align <= 32) {
		return malloc(size);
	} else {
		return memalign(align, size);
	}
#endif
}

static void* stub_realloc(void* ptr, size_t align, size_t size)
{
#ifdef _WIN32
	return _aligned_realloc(ptr, size, align);
#else
	if (align <= 32) {
		return realloc(ptr, size);
	} else {
		return reallocalign(ptr, size, align);
	}
#endif
}

static void stub_free(void* ptr)
{
#ifdef _WIN32
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

pfxUtilAllocFunc		palloc_func		= stub_malloc;
pfxUtilReallocFunc		prealloc_func	= stub_realloc;
pfxUtilDeallocateFunc	pfree_func		= stub_free;

void pfxSetUtilityAllocator(pfxUtilAllocFunc func_alloc, pfxUtilReallocFunc func_realloc, pfxUtilDeallocateFunc func_free)
{
	SCE_PFX_ASSERT(func_alloc&&func_realloc&&func_free);
	palloc_func		= func_alloc;
	prealloc_func	= func_realloc;
	pfree_func		= func_free;
}

} //namespace pfxv4
} //namespace sce
