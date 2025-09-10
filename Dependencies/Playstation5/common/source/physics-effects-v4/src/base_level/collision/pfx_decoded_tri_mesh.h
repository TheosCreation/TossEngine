/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_DECODED_TRI_MESH_H
#define _SCE_PFX_DECODED_TRI_MESH_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"

namespace sce {
namespace pfxv4 {

struct PfxDecodedFacet
{
	PfxFloat m_thickness;
	SCE_PFX_PADDING(1,12)
	PfxVector3 m_normal;
};

struct PfxDecodedTriMesh
{
	PfxVector3 m_verts[SCE_PFX_NUMMESHVERTICES];

	PfxUInt32 m_decodedVertex[(SCE_PFX_NUMMESHVERTICES+31)/32];
	
	PfxBool isDecodedVertex(PfxUInt32 i)
	{
		return (m_decodedVertex[i>>5] & (1<<(i&0x1f))) != 0;
	}
	
	PfxDecodedTriMesh()
	{
		for(int i=0;i<(SCE_PFX_NUMMESHVERTICES+31)/32;i++) {
			m_decodedVertex[i] = 0;
		}
	}
};

} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_DECODED_TRI_MESH_H
