/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/rigidbody/pfx_rigid_state.h"
#include "../../../src/util/pfx_binary_reader_writer.h"

namespace sce {
namespace pfxv4 {

extern PfxInt32 gSegmentWidth;
extern PfxFloat gSegmentWidthInv;

void PfxRigidState::reset()
{
	m_sleepCount = 0;
	m_motionType = kPfxMotionTypeActive;
	m_solverQuality = kPfxSolverQualityLevel1;
	m_flags = 0;
	m_proxyShift = 1;
	m_isInitialized = 1;
	m_allowSkippingNarrowPhaseDetection = 1; // [SMS_CHANGE]
	m_rigidBodyId = 0;
	m_contactFilterSelf = 0xffffffff;
	m_contactFilterTarget = 0xffffffff;
	m_collisionIgnoreGroup[0] = 0xffff;
	m_collisionIgnoreGroup[1] = 0xffff;
	m_islandRootId = 0;
	m_linearDamping = 1.0f;
	m_angularDamping = 0.99f;
	m_maxLinearVelocity = 340.0f;
	m_maxAngularVelocity = SCE_PFX_PI * 60.0f;
	m_position = PfxLargePosition(0.0f);
	m_orientation = PfxQuat::identity();
	m_linearVelocity = PfxVector3::zero();
	m_angularVelocity = PfxVector3::zero();
	m_userParam[0] = m_userParam[1] = m_userParam[2] = m_userParam[3] = 0;
}

void PfxRigidState::sleep()
{
	switch (getMotionType()) {
		case kPfxMotionTypeFixed:
		case kPfxMotionTypeTrigger:
		return;
		
		default:
		if(m_useSleep) {
			m_sleeping=1;
			m_sleepCount=0;
		}
		break;
	}
}

void PfxRigidState::movePosition(const PfxLargePosition &pos,PfxFloat timeStep)
{
	switch (getMotionType()) {
		case kPfxMotionTypeFixed:
		return;
	
		default:
		if(timeStep > 0.0f) {
			PfxLargePosition p = pos;
			p.changeSegment(m_position.segment);
			setLinearVelocity((p.offset - m_position.offset) / timeStep);
		}
		break;
	}
}

void PfxRigidState::moveOrientation(const PfxQuat &rot,PfxFloat timeStep)
{
	if(getMotionType() == kPfxMotionTypeFixed) return;


	SCE_PFX_ASSERT(timeStep > 0.0f);

	PfxQuat ori1 = getOrientation();
	PfxQuat ori2 = rot;

	if(dot(ori2,ori1) < 0.0f) {
		ori2 = -rot;
	}

#if 0
	PfxQuat dq = ( ori2 - ori1 ) / timeStep;
	dq = dq * 2.0f * conj(ori1);
	PfxVector3 omega = dq.getXYZ();
#else
	PfxQuat dQuat = normalize(ori2 * conj(ori1));
	PfxFloat dAngle = 2.0f * acosf(dQuat.getW()) / timeStep;
	PfxFloat l = 1.0f - dQuat.getW() * dQuat.getW();
	PfxVector3 omega(0.0f);
	const PfxFloat epsilon=0.000001f;
	if(l > epsilon) {
		omega = dAngle * dQuat.getXYZ() / sqrtf(l);
	}
	else {
		PfxQuat dq = (ori2 - ori1) / timeStep;
		dq = dq * 2.0f * conj(ori1);
		omega = dq.getXYZ();
	}
#endif

	setAngularVelocity(omega);
}

void PfxRigidState::updateSegment()
{
	if(gSegmentWidth > 0) {
		PfxVector3 diff = pfxFloorPerElem(m_position.offset * gSegmentWidthInv);
		m_position.segment.x += (PfxInt32)(diff[0]);
		m_position.segment.y += (PfxInt32)(diff[1]);
		m_position.segment.z += (PfxInt32)(diff[2]);
		m_position.offset -= diff * (PfxFloat)gSegmentWidth;
	}
}

void PfxRigidState::save(PfxUInt8 *pout, PfxUInt32 bytes) const
{
	SCE_PFX_ASSERT(bytes == bytesOfRigidState);

	PfxUInt8 *p = pout;

	writeUInt8(&p, m_flags);
	writeUInt8(&p, m_motionType);
	writeUInt16(&p, m_sleepCount);
	writeUInt32(&p, m_rigidBodyId);
	writeUInt32(&p, m_contactFilterSelf);
	writeUInt32(&p, m_contactFilterTarget);
	writeUInt16Array(&p, m_collisionIgnoreGroup, 2);
	writeUInt8(&p, m_solverQuality);
	writeUInt16(&p, m_islandRootId);
	writeFloat32(&p, m_linearDamping);
	writeFloat32(&p, m_angularDamping);
	writeFloat32(&p, m_maxLinearVelocity);
	writeFloat32(&p, m_maxAngularVelocity);
	writeUInt32Array(&p, m_userParam, 4);
	writeInt32Array(&p, (PfxInt32*)&m_position.segment, 3);
	writeFloat32Array(&p, (PfxFloat*)&m_position.offset, 3);
	writeFloat32Array(&p, (PfxFloat*)&m_orientation, 4);
	writeFloat32Array(&p, (PfxFloat*)&m_linearVelocity, 3);
	writeFloat32Array(&p, (PfxFloat*)&m_angularVelocity, 3);
}

void PfxRigidState::load(const PfxUInt8 *pout, PfxUInt32 bytes)
{
	SCE_PFX_ASSERT(bytes == bytesOfRigidState);

	const PfxUInt8 *p = pout;

	readUInt8(&p, m_flags);
	readUInt8(&p, m_motionType);
	readUInt16(&p, m_sleepCount);
	readUInt32(&p, m_rigidBodyId);
	readUInt32(&p, m_contactFilterSelf);
	readUInt32(&p, m_contactFilterTarget);
	readUInt16Array(&p, m_collisionIgnoreGroup, 2);
	readUInt16(&p, m_islandRootId);
	readUInt8(&p, m_solverQuality);
	readFloat32(&p, m_linearDamping);
	readFloat32(&p, m_angularDamping);
	readFloat32(&p, m_maxLinearVelocity);
	readFloat32(&p, m_maxAngularVelocity);
	readUInt32Array(&p, m_userParam, 4);
	readInt32Array(&p, (PfxInt32*)&m_position.segment, 3);
	readFloat32Array(&p, (PfxFloat*)&m_position.offset, 3);
	readFloat32Array(&p, (PfxFloat*)&m_orientation, 4);
	readFloat32Array(&p, (PfxFloat*)&m_linearVelocity, 3);
	readFloat32Array(&p, (PfxFloat*)&m_angularVelocity, 3);
}

} // namespace pfxv4
} // namespace sce
