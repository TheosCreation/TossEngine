/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CONTACT_TRI_MESH_CONVEX_H
#define _SCE_PFX_CONTACT_TRI_MESH_CONVEX_H

#include "../../../include/physics_effects/base_level/collision/pfx_contact_cache.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "pfx_large_tri_mesh_impl.h"
#include "pfx_convex_mesh_impl.h"
#include "pfx_collision_templates.h"

namespace sce {
namespace pfxv4 {

template<>
PfxInt32 pfxContactTriMesh<PfxConvexMeshImpl, PfxExpandedTriMesh>(
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA,const PfxExpandedTriMesh *meshA,bool flipTriangle,
	const PfxConvexMeshImpl &convexB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold);

template<>
PfxInt32 pfxContactTriMesh<PfxConvexMeshImpl, PfxQuantizedTriMeshBvh>(
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA,const PfxQuantizedTriMeshBvh *meshA,bool flipTriangle,
	const PfxConvexMeshImpl &convexB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold);

template<>
PfxInt32 pfxContactTriMesh<PfxConvexMeshImpl, PfxCompressedTriMesh>(
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA, const PfxCompressedTriMesh *meshA, bool flipTriangle,
	const PfxConvexMeshImpl &convexB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_CONTACT_TRI_MESH_CONVEX_H
