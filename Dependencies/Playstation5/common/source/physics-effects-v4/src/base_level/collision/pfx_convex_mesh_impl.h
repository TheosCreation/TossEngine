/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CONVEX_MESH_IMPL_H
#define _SCE_PFX_CONVEX_MESH_IMPL_H

#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/collision/pfx_convex_mesh.h"

#define SCE_PFX_CONVEX_CUBEMAP_SIZE (64)

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Convex Mesh

struct PfxCompressedCubeMap {
	PfxUInt16 start[SCE_PFX_CONVEX_CUBEMAP_SIZE];
	PfxUInt8 num[SCE_PFX_CONVEX_CUBEMAP_SIZE];
	PfxUInt8 isRowOrColumn; // 0:Row 1:Column
};

struct PfxConvexMeshImpl
{
    PfxUInt8 *m_indices;
    PfxFloat *m_verts;
    PfxUInt32 *m_userData;
#ifdef SCE_PFX_ENABLE_CUBEMAP_CONVEX_OPTIMIZE
	PfxUInt16 *m_listBuff = nullptr;
	PfxCompressedCubeMap m_cubeMap[6];
#endif
	PfxFloat m_half[3];
	PfxUInt8 m_numVerts = 0;
	PfxUInt8 m_numFacets = 0;
	PfxUInt8 m_name[SCE_PFX_CONVEXMESH_NAME_STR_MAX] = {'\0'};
	
	inline void updateAABB();
};

inline
void PfxConvexMeshImpl::updateAABB()
{
	PfxVector3 halfMax(0.0f);
	for(int i=0;i<m_numVerts;i++) {
		halfMax = maxPerElem(absPerElem(pfxReadVector3(m_verts + i * 3)),halfMax);
	}
	pfxStoreVector3(halfMax,m_half);
}

} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_CONVEX_MESH_IMPL_H
