/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_JOINT_CONSTRAINT_H
#define _SCE_PFX_JOINT_CONSTRAINT_H

#include "../base/pfx_common.h"
#include "../base/pfx_simd_utils.h"
#include "pfx_constraint_row.h"

namespace sce {
namespace pfxv4 {

#define SCE_PFX_JOINT_LOCK_FREE		0 ///< @brief Joint lock free
#define SCE_PFX_JOINT_LOCK_LIMIT	1 ///< @brief Joint lock limited
#define SCE_PFX_JOINT_LOCK_FIX		2 ///< @brief Joint lock locked

#define SCE_PFX_JOINT_LINEAR_SLOP	0.01f ///< @brief Linear slop of a joint constraint
#define SCE_PFX_JOINT_ANGULAR_SLOP	0.01f ///< @brief Angular slop of a joint constraint

///////////////////////////////////////////////////////////////////////////////// Joint Constraint

/// @brief The structure of the joint constraint
/// @details This is the structure for representing the joint constraint
struct SCE_PFX_API PfxJointConstraint {
	PfxInt8 m_lock;               ///< @brief Lock type
	PfxInt8 m_warmStarting;       ///< @brief Enable warm starting
	SCE_PFX_PADDING(1,2)
	PfxFloat m_movableLowerLimit; ///< @brief Lower limit
	PfxFloat m_movableUpperLimit; ///< @brief Upper limit
	PfxFloat m_velocityBias;      ///< @brief Bias factor used to fix a position error in velocity solver
	PfxFloat m_positionBias;      ///< @brief Bias factor used to fix a position error in position solver
	PfxFloat m_damping;           ///< @brief Damping
	PfxFloat m_maxImpulse;        ///< @brief Maximum impulse
	PfxFloat m_lowerLimit;        ///< @brief 
	PfxFloat m_upperLimit;        ///< @brief 
	PfxFloat m_jacobian;             ///< @brief Last jacobian
	PfxConstraintRow m_constraintRow; ///< @brief Constraint
	
	SCE_PFX_EXCLUDE_DOC(
		void reset() {
			m_lock = SCE_PFX_JOINT_LOCK_FIX;
			m_warmStarting = 1;
			m_movableLowerLimit = 0.0f;
			m_movableUpperLimit = 0.0f;
			m_velocityBias = 0.2f;
			m_positionBias = 1.0f;
			m_damping = 0.0f;
			m_maxImpulse = SCE_PFX_FLT_MAX;
			m_lowerLimit = 0.0f;
			m_upperLimit = 0.0f;
			m_jacobian = 0.0f;
			memset(&m_constraintRow,0,sizeof(PfxConstraintRow));
		}
	)
};


} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_JOINT_CONSTRAINT_H
