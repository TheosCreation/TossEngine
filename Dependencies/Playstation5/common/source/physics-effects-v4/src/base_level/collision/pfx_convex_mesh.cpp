/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_convex_mesh.h"
#include "pfx_convex_mesh_impl.h"

namespace sce {
namespace pfxv4 {

PfxUInt32 pfxConvexMeshGetNumVertices(const PfxConvexMesh &convexMesh)
{
	SCE_PFX_ASSERT(sizeof(PfxConvexMeshImpl) <= sizeof(PfxConvexMesh));
	return ((PfxConvexMeshImpl*)&convexMesh)->m_numVerts;
}

PfxVector3 pfxConvexMeshGetVertex(const PfxConvexMesh &convexMesh,PfxUInt32 vertexId)
{
	SCE_PFX_ASSERT(sizeof(PfxConvexMeshImpl) <= sizeof(PfxConvexMesh));
	return pfxReadVector3(((PfxConvexMeshImpl*)&convexMesh)->m_verts + vertexId * 3);
}

PfxUInt32 pfxConvexMeshGetNumIndices(const PfxConvexMesh &convexMesh)
{
	SCE_PFX_ASSERT(sizeof(PfxConvexMeshImpl) <= sizeof(PfxConvexMesh));
	return ((PfxConvexMeshImpl*)&convexMesh)->m_numFacets * 3;
}

PfxUInt16 pfxConvexMeshGetIndex(const PfxConvexMesh &convexMesh,PfxUInt32 indexId)
{
	SCE_PFX_ASSERT(sizeof(PfxConvexMeshImpl) <= sizeof(PfxConvexMesh));
	return ((PfxConvexMeshImpl*)&convexMesh)->m_indices[indexId];
}

PfxBool pfxConvexMeshHasPerTriangleUserData(const PfxConvexMesh &convexMesh)
{
	SCE_PFX_ASSERT(sizeof(PfxConvexMeshImpl) <= sizeof(PfxConvexMesh));
	return ((PfxConvexMeshImpl*)&convexMesh)->m_userData != NULL;
}

PfxUInt32 pfxConvexMeshGetUserDataForTriangle(const PfxConvexMesh &convexMesh,PfxUInt16 triangleId)
{
	SCE_PFX_ASSERT(sizeof(PfxConvexMeshImpl) <= sizeof(PfxConvexMesh));
	if(((PfxConvexMeshImpl*)&convexMesh)->m_userData == NULL || 
		triangleId >= ((PfxConvexMeshImpl*)&convexMesh)->m_numFacets) {
		return 0xFFFFFFFF;
	}
	
	return ((PfxConvexMeshImpl*)&convexMesh)->m_userData[triangleId];
}

const PfxUInt8 *pfxConvexMeshGetName(const PfxConvexMesh &convexMesh)
{
	SCE_PFX_ASSERT(sizeof(PfxConvexMeshImpl) <= sizeof(PfxConvexMesh));
	return ((PfxConvexMeshImpl*)&convexMesh)->m_name;
}

} //namespace pfxv4
} //namespace sce
