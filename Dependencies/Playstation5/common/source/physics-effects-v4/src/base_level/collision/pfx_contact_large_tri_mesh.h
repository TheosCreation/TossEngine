/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CONTACT_LARGE_TRI_MESH_H
#define _SCE_PFX_CONTACT_LARGE_TRI_MESH_H

#include "../../../include/physics_effects/base_level/collision/pfx_shape.h"
#include "../../../include/physics_effects/base_level/collision/pfx_contact_cache.h"
#include "pfx_large_tri_mesh_impl.h"

namespace sce {
namespace pfxv4 {

template<typename PfxShapeType, typename PfxLargeTriMeshIslandType>
PfxInt32 pfxContactLargeTriMesh(
				PfxContactCache &contacts,
				PfxBool flipTriangle,
				const PfxLargeTriMeshImpl *lmeshA,
				const PfxTransform3 &transformA,
				const PfxShapeType &shapeB,
				const PfxTransform3 &transformB,
				PfxFloat distanceThreshold);

template<typename PfxShapeType, typename PfxLargeTriMeshIslandType>
PfxInt32 pfxContactLargeTriMeshBvh(
				PfxContactCache &contacts,
				PfxBool flipTriangle,
				const PfxLargeTriMeshImpl *lmeshA,
				const PfxTransform3 &transformA,
				const PfxShapeType &shapeB,
				const PfxTransform3 &transformB,
				PfxFloat distanceThreshold);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_CONTACT_LARGE_TRI_MESH_H
