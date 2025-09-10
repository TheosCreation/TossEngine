/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/sort/pfx_sort.h"

#ifdef _WIN32
	#include <malloc.h>
	#define SCE_PFX_ALLOCA(a) _alloca(a)
#elif defined(__ORBIS__) || defined(__PROSPERO__) // [SMS_CHANGE] Prospero support
	#include <stdlib.h>
	#define SCE_PFX_ALLOCA(a) alloca(a)
#else
	#include <alloca.h>
	#define SCE_PFX_ALLOCA(a) alloca(a)
#endif

namespace sce {
namespace pfxv4 {

#define Key(a) pfxGetKey(a)

///////////////////////////////////////////////////////////////////////////////
// Radix Sort

template <class SortData>
void pfxRadixSort(SortData *data,SortData *buff,PfxUInt32 n)
{
	const PfxUInt32 bits = 8;
	const PfxUInt32 nSize = 256;
	const PfxUInt32 nLoop = 4;
	PfxInt32 *count = (PfxInt32*)SCE_PFX_ALLOCA(sizeof(PfxInt32) * nSize); // 1KB

	SortData *sbuff[2] = {data,buff};

	int sw = 0;

	for(PfxUInt32 pass=0;pass<nLoop;pass++) {
		for(PfxUInt32 j=0;j<nSize;j++) {
			count[j] = 0;
		}
		for(PfxUInt32 i=0;i<n;i++) {
			count[((Key(sbuff[sw][i])>>(pass*bits))&0xff)]++;
		}

		PfxInt32 sum = 0;
		for(PfxUInt32 j=0;j<nSize;j++) {
			PfxInt32 temp = count[j] + sum;
			count[j] = sum;
			sum = temp;
		}

		for(PfxUInt32 i=0;i<n;i++) {
			PfxInt32 id = (Key(sbuff[sw][i])>>(pass*bits))&0xff;
			PfxInt32 dst = count[id]++;
			sbuff[1-sw][dst] = sbuff[sw][i];
		}
		sw = 1 - sw;
	}
}
///////////////////////////////////////////////////////////////////////////////
// Single Sort

void pfxSort(PfxSortData16 *data,PfxSortData16 *buff,PfxUInt32 n)
{
	pfxRadixSort(data,buff,n);
}

void pfxSort(PfxSortData32 *data,PfxSortData32 *buff,PfxUInt32 n)
{
	pfxRadixSort(data,buff,n);
}

} //namespace pfxv4
} //namespace sce
