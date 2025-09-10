/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_ATOMIC_H
#define _SCE_PFX_ATOMIC_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"

#if defined(__ORBIS__) || defined(__PROSPERO__) // [SMS_CHANGE] Prospero support
	#include <sce_atomic.h>
	#include <kernel.h>
#endif

namespace sce{
namespace pfxv4{

#if defined(__ORBIS__) || defined(__PROSPERO__) // [SMS_CHANGE] Prospero support

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicAdd(volatile PfxInt32* ptr, PfxInt32 value)
{
	return sceAtomicAdd32(ptr,value);
}

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicExchange(volatile PfxInt32* ptr, PfxInt32 swap)
{
	return sceAtomicExchange32(ptr,swap);
}

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicIncrement(volatile PfxInt32* ptr)
{
	return sceAtomicIncrement32(ptr);
}

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicDecrement(volatile PfxInt32* ptr)
{
	return sceAtomicDecrement32(ptr);
}

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicCompareAndSwap(volatile PfxInt32* ptr, PfxInt32 compare, PfxInt32 swap)
{
	return sceAtomicCompareAndSwap32(ptr, compare, swap);
}

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicOr(volatile PfxInt32* ptr, PfxInt32 mask)
{
	return sceAtomicOr32(ptr,mask);
}

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicAnd(volatile PfxInt32* ptr, PfxInt32 mask)
{
	return sceAtomicAnd32(ptr,mask);
}

#elif defined(_WIN32)

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicAdd(volatile PfxInt32* ptr, PfxInt32 value)
{
	return InterlockedExchangeAdd((volatile LONG*)ptr, value);
}

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicExchange(volatile PfxInt32* ptr, PfxInt32 swap)
{
	return InterlockedExchange((volatile LONG*)ptr,swap);
}

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicIncrement(volatile PfxInt32* ptr)
{
	return InterlockedExchangeAdd((volatile LONG*)ptr,1);
}

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicDecrement(volatile PfxInt32* ptr)
{
	return InterlockedExchangeAdd((volatile LONG*)ptr,-1);
}

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicCompareAndSwap(volatile PfxInt32* ptr, PfxInt32 compare, PfxInt32 swap)
{
	return InterlockedCompareExchange((volatile LONG*)ptr, swap, compare);
}

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicOr(volatile PfxInt32* ptr, PfxInt32 mask)
{
	return 0;// not supported
}

static SCE_PFX_FORCE_INLINE
PfxInt32 pfxAtomicAnd(volatile PfxInt32* ptr, PfxInt32 mask)
{
	return 0;// not supported
}

#endif

static SCE_PFX_FORCE_INLINE
void pfxYieldThread()
{
#if defined(__ORBIS__) || defined(__PROSPERO__)
	scePthreadYield();
#elif defined(_WIN32)
	SwitchToThread();
#endif
}


} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_ATOMIC_H
