/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_BROADPHASE_PAIR_H
#define _SCE_PFX_BROADPHASE_PAIR_H

#include "../sort/pfx_sort_data.h"

namespace sce {
namespace pfxv4 {


/// @brief Broadphase pair
typedef PfxSortData16 PfxBroadphasePair;

SCE_PFX_FORCE_INLINE void pfxSetObjectIdA(PfxBroadphasePair &pair,PfxUInt32 i)	{pair.set16(6,(PfxUInt16)i);}
SCE_PFX_FORCE_INLINE void pfxSetObjectIdB(PfxBroadphasePair &pair,PfxUInt32 i)	{pair.set16(7,(PfxUInt16)i);}
SCE_PFX_FORCE_INLINE void pfxSetMotionMaskA(PfxBroadphasePair &pair,PfxUInt8 i)		{pair.set8(0,i);}
SCE_PFX_FORCE_INLINE void pfxSetMotionMaskB(PfxBroadphasePair &pair,PfxUInt8 i)		{pair.set8(1,i);}
SCE_PFX_FORCE_INLINE void pfxSetBroadphaseFlag(PfxBroadphasePair &pair,PfxUInt8 f)	{pair.set8(2,(pair.get8(2)&0xf0)|(f&0x0f));}
SCE_PFX_FORCE_INLINE void pfxSetActive(PfxBroadphasePair &pair,PfxBool b)			{pair.set8(2,(pair.get8(2)&0x0f)|(PfxUInt8)((b?1:0)<<4));}
SCE_PFX_FORCE_INLINE void pfxSetContactId(PfxBroadphasePair &pair,PfxUInt32 i)		{pair.set32(1,i);}
SCE_PFX_FORCE_INLINE void pfxSetSolverQuality(PfxBroadphasePair &pair, PfxUInt8 q)		{ pair.set8(8, q); }

SCE_PFX_FORCE_INLINE PfxUInt32 pfxGetObjectIdA(const PfxBroadphasePair &pair)	{return pair.get16(6);}
SCE_PFX_FORCE_INLINE PfxUInt32 pfxGetObjectIdB(const PfxBroadphasePair &pair)	{return pair.get16(7);}
SCE_PFX_FORCE_INLINE PfxUInt8  pfxGetMotionMaskA(const PfxBroadphasePair &pair)		{return pair.get8(0);}
SCE_PFX_FORCE_INLINE PfxUInt8  pfxGetMotionMaskB(const PfxBroadphasePair &pair)		{return pair.get8(1);}
SCE_PFX_FORCE_INLINE PfxUInt8  pfxGetBroadphaseFlag(const PfxBroadphasePair &pair)	{return pair.get8(2)&0x0f;}
SCE_PFX_FORCE_INLINE PfxBool   pfxGetActive(const PfxBroadphasePair &pair)			{return (pair.get8(2)>>4)!=0;}
SCE_PFX_FORCE_INLINE PfxUInt32 pfxGetContactId(const PfxBroadphasePair &pair)		{return pair.get32(1);}
SCE_PFX_FORCE_INLINE PfxUInt8  pfxGetSolverQuality(const PfxBroadphasePair &pair)	{ return pair.get8(8); }


} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_BROADPHASE_PAIR_H
