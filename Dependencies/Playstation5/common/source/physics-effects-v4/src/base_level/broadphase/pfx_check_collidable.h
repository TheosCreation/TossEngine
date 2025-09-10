/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CHECK_COLLIDABLE_H
#define _SCE_PFX_CHECK_COLLIDABLE_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../../include/physics_effects/base_level/rigidbody/pfx_rigid_state.h"
#include "../../../include/physics_effects/base_level/broadphase/pfx_broadphase_pair.h"

namespace sce {
namespace pfxv4 {

// Collidable check table
/*
	-----------------MotionTypeA
	|0	1	0	1	1	1
	|1	1	1	1	1	1
	|0	1	0	1	1	0
	|1	1	1	0	1	1
	|1	1	1	1	0	1
	|1	1	0	1	1	1
MotionTypeB
 */

static SCE_PFX_FORCE_INLINE
PfxBool pfxCheckContactFilter(PfxUInt32 filterSelfA,PfxUInt32 filterTargetA,PfxUInt32 filterSelfB,PfxUInt32 filterTargetB)
{
#ifdef SCE_PFX_ENABLE_CONTACT_FILTER_METHOD_AND
	return ((filterSelfA&filterTargetB) && (filterTargetA&filterSelfB));
#else
	return ((filterSelfA&filterTargetB) || (filterTargetA&filterSelfB));
#endif
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxCheckCollidableTable(ePfxMotionType i,ePfxMotionType j)
{
	const PfxUInt64 collidableTable = 0x00000005FF5BBF77LL;

	SCE_PFX_ASSERT(i < kPfxMotionTypeCount);
	SCE_PFX_ASSERT(j < kPfxMotionTypeCount);

	PfxUInt64 idx = j * kPfxMotionTypeCount + i;
	PfxUInt64 mask = 1LL << (kPfxMotionTypeCount*kPfxMotionTypeCount-1-idx);

	return (collidableTable & mask) != 0;
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxCheckCollisionIgnoreGroup(PfxUInt16 ignoreGroupA0,PfxUInt16 ignoreGroupA1,PfxUInt16 ignoreGroupB0,PfxUInt16 ignoreGroupB1)
{
    // collision gets ignored if they are in the same group and both groups are valid
    const PfxUInt16 noGroup(PfxUInt16(-1));
#ifdef SCE_PFX_ENABLE_IGNORE_GROUP_AS_EXCEPTION
	// [SMS_CHANGE]
	// Instead of having two ignore groups, we use the first group (0) as an ignore group and the second group(1) as an ignore exception group
	// Things in same ignore group do not collide with each other UNLESS they have the same ignore exception group
	// [HM] 5/8/19 It looks it isn't used anywhere in the codes. But I still leave it just in case.
	if( ( ignoreGroupA0 != noGroup ) && ( ignoreGroupB0 != noGroup ) && ( ignoreGroupA0 == ignoreGroupB0 ) )
	{
		if( ( ignoreGroupA1 == noGroup ) || ( ignoreGroupB1 == noGroup ) || ( ignoreGroupA1 != ignoreGroupB1 ) )
		{
			return false;
		}
	}
#else
	if( !( ( ignoreGroupA0 == noGroup ) || ( ignoreGroupB0 == noGroup ) || ( ignoreGroupA0 != ignoreGroupB0 ) ) ) return false;
	if( !( ( ignoreGroupA0 == noGroup ) || ( ignoreGroupB1 == noGroup ) || ( ignoreGroupA0 != ignoreGroupB1 ) ) ) return false;
	if( !( ( ignoreGroupA1 == noGroup ) || ( ignoreGroupB0 == noGroup ) || ( ignoreGroupA1 != ignoreGroupB0 ) ) ) return false;
	if( !( ( ignoreGroupA1 == noGroup ) || ( ignoreGroupB1 == noGroup ) || ( ignoreGroupA1 != ignoreGroupB1 ) ) ) return false;
#endif
	return true;
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxCheckCollidableInBroadphase(
	PfxUInt32 filterSelfA,PfxUInt32 filterTargetA,PfxUInt8 motionMaskA,PfxUInt16 ignoreGroupA0,PfxUInt16 ignoreGroupA1,
	PfxUInt32 filterSelfB,PfxUInt32 filterTargetB,PfxUInt8 motionMaskB,PfxUInt16 ignoreGroupB0,PfxUInt16 ignoreGroupB1)
{
	ePfxMotionType motionTypeA = (ePfxMotionType)(motionMaskA&SCE_PFX_MOTION_MASK_TYPE);
	ePfxMotionType motionTypeB = (ePfxMotionType)(motionMaskB&SCE_PFX_MOTION_MASK_TYPE);

	return
		pfxCheckCollidableTable(motionTypeA,motionTypeB) && // モーションタイプ別衝突判定テーブル
		pfxCheckContactFilter(filterSelfA,filterTargetA,filterSelfB,filterTargetB) && // 衝突フィルター
		pfxCheckCollisionIgnoreGroup(ignoreGroupA0,ignoreGroupA1,ignoreGroupB0,ignoreGroupB1);
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxCheckCollidableInCollision(const PfxBroadphasePair &pair)
{
	PfxUInt32 motionTypeA = pfxGetMotionMaskA(pair)&SCE_PFX_MOTION_MASK_TYPE;
	PfxUInt32 motionTypeB = pfxGetMotionMaskB(pair)&SCE_PFX_MOTION_MASK_TYPE;
	PfxUInt32 sleepA = pfxGetMotionMaskA(pair)&SCE_PFX_MOTION_MASK_SLEEPING;
	PfxUInt32 sleepB = pfxGetMotionMaskB(pair)&SCE_PFX_MOTION_MASK_SLEEPING;

	return
		pfxCheckCollidableTable((ePfxMotionType)motionTypeA,(ePfxMotionType)motionTypeB) && // モーションタイプ別衝突判定テーブル
		!((sleepA != 0 && sleepB != 0) || (sleepA != 0 && motionTypeB == kPfxMotionTypeFixed) || (sleepB != 0 && motionTypeA == kPfxMotionTypeFixed)); // スリープ時のチェック
}

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_CHECK_COLLIDABLE_H
