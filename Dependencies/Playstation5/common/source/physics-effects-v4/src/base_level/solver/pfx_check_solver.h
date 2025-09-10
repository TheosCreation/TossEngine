/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CHECK_SOLVER_H_
#define _SCE_PFX_CHECK_SOLVER_H_

#include "../../../include/physics_effects/base_level/rigidbody/pfx_rigid_state.h"
#include "../../../include/physics_effects/base_level/solver/pfx_constraint_pair.h"

namespace sce {
namespace pfxv4 {

// Solver check table
/*
	-----------------MotionTypeA
	|0	1	0	1	0	0
	|1	1	1	1	0	1
	|0	1	0	1	0	0
	|1	1	1	0	0	1
	|0	0	0	0	0	0
	|0	1	0	1	0	0
MotionTypeB
 */

static SCE_PFX_FORCE_INLINE
PfxBool pfxCheckSolverTable(ePfxMotionType i,ePfxMotionType j)
{
	const PfxUInt64 solverTable = 0x000000053D539014LL;

	SCE_PFX_ASSERT(i < kPfxMotionTypeCount);
	SCE_PFX_ASSERT(j < kPfxMotionTypeCount);

	PfxUInt64 idx = j * kPfxMotionTypeCount + i;
	PfxUInt64 mask = 1LL << (kPfxMotionTypeCount*kPfxMotionTypeCount-1-idx);
	return (solverTable & mask) != 0;
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxCheckSolver(const PfxConstraintPair &pair)
{
	PfxUInt32 motionA = pfxGetMotionMaskA(pair)&SCE_PFX_MOTION_MASK_TYPE;
	PfxUInt32 motionB = pfxGetMotionMaskB(pair)&SCE_PFX_MOTION_MASK_TYPE;
	PfxUInt32 sleepA = pfxGetMotionMaskA(pair)&SCE_PFX_MOTION_MASK_SLEEPING;
	PfxUInt32 sleepB = pfxGetMotionMaskB(pair)&SCE_PFX_MOTION_MASK_SLEEPING;
	
	return
		pfxGetActive(pair) && pfxGetNumConstraints(pair) > 0 &&
		pfxCheckSolverTable((ePfxMotionType)motionA,(ePfxMotionType)motionB) && // モーションタイプ別衝突判定テーブル
		!((sleepA != 0 && sleepB != 0) || (sleepA != 0 && motionB == kPfxMotionTypeFixed) || (sleepB != 0 && motionA == kPfxMotionTypeFixed));// スリープ時のチェック
}

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_CHECK_SOLVER_H_ */
