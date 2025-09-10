/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CRITICAL_SECTION_H
#define _SCE_PFX_CRITICAL_SECTION_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"

#include "pfx_atomic.h"

namespace sce{
namespace pfxv4{

#if defined(_WIN32)

typedef CRITICAL_SECTION PfxCriticalSection;

static SCE_PFX_FORCE_INLINE
void pfxInitializeCriticalSection(PfxCriticalSection *criticalSection)
{
	InitializeCriticalSection((CRITICAL_SECTION*)criticalSection);
}

static SCE_PFX_FORCE_INLINE
void pfxFinalizeCriticalSection(PfxCriticalSection *criticalSection)
{
	DeleteCriticalSection((CRITICAL_SECTION*)criticalSection);
}

static SCE_PFX_FORCE_INLINE
void pfxLockCriticalSection(PfxCriticalSection *criticalSection)
{
	EnterCriticalSection((CRITICAL_SECTION*)criticalSection);
}

static SCE_PFX_FORCE_INLINE
void pfxUnlockCriticalSection(PfxCriticalSection *criticalSection)
{
	LeaveCriticalSection((CRITICAL_SECTION*)criticalSection);
}

#elif defined(__ORBIS__) || defined(__PROSPERO__)

typedef PfxInt32 PfxCriticalSection;

static SCE_PFX_FORCE_INLINE
void pfxInitializeCriticalSection(PfxCriticalSection *criticalSection)
{
	*criticalSection = 0;
}

static SCE_PFX_FORCE_INLINE
void pfxFinalizeCriticalSection(PfxCriticalSection *criticalSection)
{
}

static SCE_PFX_FORCE_INLINE
void pfxLockCriticalSection(PfxCriticalSection *criticalSection)
{
	int count = 0;
	while(0 != pfxAtomicCompareAndSwap(criticalSection,0,1)) {
		_mm_pause();
		if(((++count) % 100) == 0) {
			pfxYieldThread();
		}
	}
}

static SCE_PFX_FORCE_INLINE
void pfxUnlockCriticalSection(PfxCriticalSection *criticalSection)
{
	int count = 0;
	while(1 != pfxAtomicCompareAndSwap(criticalSection,1,0)) {
		_mm_pause();
		if(((++count) % 100) == 0) {
			pfxYieldThread();
		}
	}
}

#endif

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_CRITICAL_SECTION_H
