/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint.h"
#include "../../../src/util/pfx_binary_reader_writer.h"

namespace sce {
namespace pfxv4 {

void PfxJoint::save(PfxUInt8 *pout, PfxUInt32 bytes) const
{
	SCE_PFX_ASSERT(bytes == bytesOfJoint);

	PfxUInt8 *p = pout;

	PfxVector3 colA0 = m_frameA.getCol0();
	PfxVector3 colA1 = m_frameA.getCol1();
	PfxVector3 colA2 = m_frameA.getCol2();

	PfxVector3 colB0 = m_frameB.getCol0();
	PfxVector3 colB1 = m_frameB.getCol1();
	PfxVector3 colB2 = m_frameB.getCol2();

	writeFloat32Array(&p, (PfxFloat*)&m_anchorA, 3);
	writeFloat32Array(&p, (PfxFloat*)&m_anchorB, 3);
	writeFloat32Array(&p, (PfxFloat*)&colA0, 3);
	writeFloat32Array(&p, (PfxFloat*)&colA1, 3);
	writeFloat32Array(&p, (PfxFloat*)&colA2, 3);
	writeFloat32Array(&p, (PfxFloat*)&colB0, 3);
	writeFloat32Array(&p, (PfxFloat*)&colB1, 3);
	writeFloat32Array(&p, (PfxFloat*)&colB2, 3);
	writeFloat32Array(&p, (PfxFloat*)&m_targetFrame, 4);
	writeFloat32Array(&p, m_parameters, 4);
	writeUInt8(&p, m_active);
	writeUInt8(&p, m_type);
	writeUInt8(&p, m_numConstraints);
	writeUInt32(&p, m_rigidBodyIdA);
	writeUInt32(&p, m_rigidBodyIdB);
	writeUInt32(&p, m_userData);
	for(int i=0;i<6;i++) {
		const PfxJointConstraint &jointConstraint = m_constraints[i];
		writeInt8(&p, jointConstraint.m_lock);
		writeInt8(&p, jointConstraint.m_warmStarting);
		writeFloat32(&p, jointConstraint.m_movableLowerLimit);
		writeFloat32(&p, jointConstraint.m_movableUpperLimit);
		writeFloat32(&p, jointConstraint.m_velocityBias);
		writeFloat32(&p, jointConstraint.m_positionBias);
		writeFloat32(&p, jointConstraint.m_damping);
		writeFloat32(&p, jointConstraint.m_maxImpulse);
		writeFloat32(&p, jointConstraint.m_lowerLimit);
		writeFloat32(&p, jointConstraint.m_upperLimit);
		writeFloat32(&p, jointConstraint.m_jacobian);
		writeFloat32Array(&p, jointConstraint.m_constraintRow.m_normal, 3);
		writeFloat32(&p, jointConstraint.m_constraintRow.m_rhs);
		writeFloat32(&p, jointConstraint.m_constraintRow.m_jacDiagInv);
		writeFloat32(&p, jointConstraint.m_constraintRow.m_accumImpulse);
	}
}

void PfxJoint::load(const PfxUInt8 *pout, PfxUInt32 bytes)
{
	SCE_PFX_ASSERT(bytes == bytesOfJoint);

	const PfxUInt8 *p = pout;

	PfxVector3 colA0 ,colA1,colA2;
	PfxVector3 colB0 ,colB1,colB2;

	readFloat32Array(&p, (PfxFloat*)&m_anchorA, 3);
	readFloat32Array(&p, (PfxFloat*)&m_anchorB, 3);
	readFloat32Array(&p, (PfxFloat*)&colA0, 3);
	readFloat32Array(&p, (PfxFloat*)&colA1, 3);
	readFloat32Array(&p, (PfxFloat*)&colA2, 3);
	readFloat32Array(&p, (PfxFloat*)&colB0, 3);
	readFloat32Array(&p, (PfxFloat*)&colB1, 3);
	readFloat32Array(&p, (PfxFloat*)&colB2, 3);
	readFloat32Array(&p, (PfxFloat*)&m_targetFrame, 4);
	readFloat32Array(&p, m_parameters, 4);
	readUInt8(&p, m_active);
	readUInt8(&p, m_type);
	readUInt8(&p, m_numConstraints);
	readUInt32(&p, m_rigidBodyIdA);
	readUInt32(&p, m_rigidBodyIdB);
	readUInt32(&p, m_userData);
	// 143 bytes

	m_frameA = PfxMatrix3(colA0 ,colA1,colA2);
	m_frameB = PfxMatrix3(colB0 ,colB1,colB2);

	for(int i=0;i<6;i++) {
		PfxJointConstraint &jointConstraint = m_constraints[i];
		readInt8(&p, jointConstraint.m_lock);
		readInt8(&p, jointConstraint.m_warmStarting);
		readFloat32(&p, jointConstraint.m_movableLowerLimit);
		readFloat32(&p, jointConstraint.m_movableUpperLimit);
		readFloat32(&p, jointConstraint.m_velocityBias);
		readFloat32(&p, jointConstraint.m_positionBias);
		readFloat32(&p, jointConstraint.m_damping);
		readFloat32(&p, jointConstraint.m_maxImpulse);
		readFloat32(&p, jointConstraint.m_lowerLimit);
		readFloat32(&p, jointConstraint.m_upperLimit);
		readFloat32(&p, jointConstraint.m_jacobian);
		readFloat32Array(&p, jointConstraint.m_constraintRow.m_normal, 3);
		readFloat32(&p, jointConstraint.m_constraintRow.m_rhs);
		readFloat32(&p, jointConstraint.m_constraintRow.m_jacDiagInv);
		readFloat32(&p, jointConstraint.m_constraintRow.m_accumImpulse);
		// 62 bytes
	}
}

} //namespace pfxv4
} //namespace sce
