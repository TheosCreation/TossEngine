/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_CONSTRAINT_PAIR_H
#define _SCE_PFX_CONSTRAINT_PAIR_H

#include "../sort/pfx_sort_data.h"
#include "../broadphase/pfx_broadphase_pair.h"

namespace sce {
namespace pfxv4 {


/// @brief Constraint pair
typedef PfxSortData16 PfxConstraintPair;

//J	PfxBroadphasePairと共通
//E Same as PfxBroadphasePair

SCE_PFX_FORCE_INLINE void pfxSetConstraintId(PfxConstraintPair &pair,PfxUInt32 i)	{pair.set32(1,i);}
SCE_PFX_FORCE_INLINE void pfxSetNumConstraints(PfxConstraintPair &pair,PfxUInt8 n)	{pair.set8(3,n);}

SCE_PFX_FORCE_INLINE PfxUInt32 pfxGetConstraintId(const PfxConstraintPair &pair)	{return pair.get32(1);}
SCE_PFX_FORCE_INLINE PfxUInt8  pfxGetNumConstraints(const PfxConstraintPair &pair)	{return pair.get8(3);}


} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_CONSTRAINT_PAIR_H
