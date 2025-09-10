/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_core_sphere.h"
#include "../src/util/pfx_binary_reader_writer.h"

namespace sce {
namespace pfxv4 {

void PfxCoreSphere::save(PfxUInt8 *pout, PfxUInt32 bytes) const
{
	SCE_PFX_ASSERT(bytes == bytesOfCoreSphere);

	PfxUInt8 *p = pout;
	writeFloat32(&p, m_radius);
	writeFloat32Array(&p, m_offsetPosition, 3);
	writeUInt32(&p, m_contactFilterSelf);
	writeUInt32(&p, m_contactFilterTarget);
}

void PfxCoreSphere::load(const PfxUInt8 *pout, PfxUInt32 bytes)
{
	SCE_PFX_ASSERT(bytes == bytesOfCoreSphere);

	const PfxUInt8 *p = pout;
	readFloat32(&p, m_radius);
	readFloat32Array(&p, m_offsetPosition, 3);
	readUInt32(&p, m_contactFilterSelf);
	readUInt32(&p, m_contactFilterTarget);
}

} //namespace pfxv4
} //namespace sce
