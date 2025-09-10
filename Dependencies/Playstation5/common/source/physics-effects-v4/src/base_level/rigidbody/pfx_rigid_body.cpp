/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/rigidbody/pfx_rigid_body.h"
#include "../../../src/util/pfx_binary_reader_writer.h"

namespace sce {
namespace pfxv4 {

PfxFloat    PfxRigidBody::getMassInv() const
{
	return m_mass==0?0.0f:1.0f/m_mass;
}

PfxMatrix3	PfxRigidBody::getInertiaInv() const
{
	if(m_mass==0) return PfxMatrix3(0.0f);
	
	PfxMatrix3 inertiaInv = inverse(m_inertia);
	if(m_arrangeInertia & SCE_PFX_FIX_ROTATION_X) {
		inertiaInv[0][0] = 0.0f;
	}
	if(m_arrangeInertia & SCE_PFX_FIX_ROTATION_Y) {
		inertiaInv[1][1] = 0.0f;
	}
	if(m_arrangeInertia & SCE_PFX_FIX_ROTATION_Z) {
		inertiaInv[2][2] = 0.0f;
	}
	return inertiaInv;
}

void PfxRigidBody::save(PfxUInt8 *pout, PfxUInt32 bytes) const
{
	SCE_PFX_ASSERT(bytes == bytesOfRigidBody);

	PfxUInt8 *p = pout;

	PfxVector3 col0 = m_inertia.getCol0();
	PfxVector3 col1 = m_inertia.getCol1();
	PfxVector3 col2 = m_inertia.getCol2();
	writeFloat32Array(&p, (PfxFloat*)&col0, 3);
	writeFloat32Array(&p, (PfxFloat*)&col1, 3);
	writeFloat32Array(&p, (PfxFloat*)&col2, 3);
	writeFloat32(&p, m_mass);
	writeFloat32(&p, m_restitution);
	writeFloat32(&p, m_friction);
	writeFloat32(&p, m_rollingFriction);
	writeUInt32(&p, m_arrangeInertia);
}

void PfxRigidBody::load(const PfxUInt8 *pout, PfxUInt32 bytes)
{
	SCE_PFX_ASSERT(bytes == bytesOfRigidBody);

	const PfxUInt8 *p = pout;

	PfxVector3 col0,col1,col2;

	readFloat32Array(&p, (PfxFloat*)&col0, 3);
	readFloat32Array(&p, (PfxFloat*)&col1, 3);
	readFloat32Array(&p, (PfxFloat*)&col2, 3);
	readFloat32(&p, m_mass);
	readFloat32(&p, m_restitution);
	readFloat32(&p, m_friction);
	readFloat32(&p, m_rollingFriction);
	readUInt32(&p, m_arrangeInertia);

	m_inertia = PfxMatrix3(col0,col1,col2);
}

} // namespace pfxv4
} // namespace sce
