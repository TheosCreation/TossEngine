/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CONTACT_TRI_MESH_CAPSULE_H
#define _SCE_PFX_CONTACT_TRI_MESH_CAPSULE_H

#include "../../../include/physics_effects/base_level/collision/pfx_capsule.h"
#include "../../../include/physics_effects/base_level/collision/pfx_contact_cache.h"
#include "pfx_large_tri_mesh_impl.h"
#include "pfx_collision_templates.h"

namespace sce {
namespace pfxv4 {

template<>
PfxInt32 pfxContactTriMesh<PfxCapsule, PfxExpandedTriMesh> (
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA, const PfxExpandedTriMesh *meshA, bool flipTriangle,
	const PfxCapsule &capsuleB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold);

template<>
PfxInt32 pfxContactTriMesh<PfxCapsule, PfxQuantizedTriMeshBvh> (
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA, const PfxQuantizedTriMeshBvh *meshA, bool flipTriangle,
	const PfxCapsule &capsuleB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold);

template<>
PfxInt32 pfxContactTriMesh<PfxCapsule, PfxCompressedTriMesh> (
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA, const PfxCompressedTriMesh *meshA, bool flipTriangle,
	const PfxCapsule &capsuleB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_CONTACT_TRI_MESH_CAPSULE_H
