/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_SOLVER_BODY_H
#define _SCE_PFX_SOLVER_BODY_H

#include "../base/pfx_common.h"

namespace sce {
namespace pfxv4 {

struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxSolverBody{
	PfxVector3 m_deltaLinearVelocity;
	PfxVector3 m_deltaAngularVelocity;
	PfxQuat    m_orientation;
	PfxMatrix3 m_inertiaInv;
	PfxFloat   m_massInv;
	PfxUInt32  m_motionType;
};

} //namespace pfxv4
} //namespace sce
#endif // _SCE_PFX_SOLVER_BODY_H
