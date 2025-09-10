/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_COMPOUND_CONTAINER_IMPL_H_
#define _SCE_PFX_COMPOUND_CONTAINER_IMPL_H_

#include "../../../include/physics_effects/base_level/solver/pfx_constraint_pair.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint.h"
#include "../../../include/physics_effects/low_level/rigidbody/pfx_compound_container.h"
#include "../../low_level/collision/pfx_contact_manager.h"
#include "../../low_level/solver/pfx_pair_graph.h"

namespace sce {
namespace pfxv4 {

struct PfxCompoundChild {
	PfxUInt32 rigidBodyId;
	PfxVector3 offsetPosition;
	PfxQuat offsetOrientation;
};

struct PfxCompoundContactInfo {
	PfxFloat localPoint[4][3];	// 元の衝突座標 (Vector3)
};

struct PfxCompoundJointInfo {
	PfxFloat anchor[3];			// 元のアンカー座標 (Vector3)
	PfxFloat frame[4];			// 元のフレーム (Quat)
};

struct PfxCompoundPair {
	enum {kContact, kJoint} type;
	PfxUInt32 pairId;			// ペアのID
	PfxUInt32 originalId;		// 付け替えられた剛体ID
	union {
		PfxCompoundContactInfo contactInfo;
		PfxCompoundJointInfo jointInfo;
	};
};

struct PfxCompoundContainerImpl {
	PfxUInt32 m_flags;
	PfxUInt32 m_compoundId;
	PfxUInt32 m_numCompoundPairs;
	PfxUInt32 m_maxCompoundPairs;
	PfxUInt32 m_numChilds;
	PfxUInt32 m_maxChilds;
	
	PfxCompoundChild *m_compoundChilds;
	PfxCompoundPair *m_compoundPairs;
	
	PfxBool find(PfxUInt32 rigidBodyId)
	{
		for (PfxUInt32 i = 0; i < m_numChilds; i++) {
			if (m_compoundChilds[i].rigidBodyId == rigidBodyId) {
				return true;
			}
		}
		return false;
	}

	PfxBool find(PfxUInt32 rigidBodyId, PfxUInt32 &childId)
	{
		for (PfxUInt32 i = 0; i < m_numChilds; i++) {
			if (m_compoundChilds[i].rigidBodyId == rigidBodyId) {
				childId = i;
				return true;
			}
		}
		childId = (PfxUInt32)-1;
		return false;
	}
};

// Internal functions
void pfxDistributeBodies(PfxCompoundContainerImpl *compoundContainerImpl, PfxRigidState *states);

void pfxModifyPairsForCompound(PfxCompoundContainerImpl *compoundContainerImpl,
	PfxRigidState *states,
	PfxConstraintPair *contactPairs, PfxUInt32 numContactPairs, PfxContactManager *contactManager,
	PfxConstraintPair *jointPairs, PfxUInt32 numJointPairs, PfxJoint *joints);

void pfxModifyPairsForCompound(PfxCompoundContainerImpl *compoundContainerImpl,
	void *workBuff, PfxUInt32 workBytes,
	PfxRigidState *states, PfxUInt32 numRigidBodies,
	PfxPairSimpleData *contactPairDatem, PfxConstraintPair *contactPairs, PfxUInt32 numContactPairDatem, PfxContactManager *contactManager,
	PfxPairSimpleData *jointPairDatem, PfxConstraintPair *jointPairs, PfxUInt32 numJointPairDatem, PfxJoint *joints);

void pfxRestorePairsFromCompound(PfxCompoundContainerImpl *compoundContainerImpl,
	PfxConstraintPair *contactPairs, PfxUInt32 numContactPairs, PfxContactManager *contactManager,
	PfxConstraintPair *jointPairs, PfxUInt32 numJointPairs, PfxJoint *joints);

void pfxPrintCompoundContainer(PfxCompoundContainerImpl *compoundContainerImpl, PfxRigidState *states, PfxRigidBody *bodies);

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_COMPOUND_CONTAINER_IMPL_H_ */
