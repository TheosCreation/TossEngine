/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
*                Copyright (C) 2020 Sony Interactive Entertainment Inc.
*                                                
*/

#ifndef _SCE_PFX_RIGID_BODY_CACHE_MANAGER_H_
#define _SCE_PFX_RIGID_BODY_CACHE_MANAGER_H_

#include "../../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../../include/physics_effects/base_level/rigidbody/pfx_rigid_state.h"
#include "../../../include/physics_effects/base_level/collision/pfx_collidable.h"
#include "../../../include/physics_effects/base_level/collision/pfx_large_tri_mesh.h"
#include "../../../include/physics_effects/low_level/broadphase/pfx_broadphase_proxy_container.h"
#include "../../../include/physics_effects/low_level/collision/pfx_rigid_body_cache_container.h"
#include "../../low_level/broadphase/pfx_bounding_volume.h"

namespace sce {
namespace pfxv4 {

struct PfxRigidBodyCacheRangeRayInternal;

struct PfxTraverseRigidBodyCacheContainerCallbackParam {
	PfxRigidState *states;
	PfxCollidable *collidables;
	PfxBv rangeOfInterest;
};

struct PfxCacheBvNode {
	const PfxBroadphaseProxyContainer *m_proxyContainerPtr;
	PfxUInt32 m_bvNodeIndex;
};

struct PfxCacheLargeTriMesh {
	PfxUInt32 m_meshRbIndex;
	PfxUInt32 m_meshShapeIndex;
	PfxUInt32 m_islandStartIndex;
	PfxUInt32 m_numIslandIndices;
};

typedef PfxBool(*pfxTraverseRigidBodyCacheContainerCallback)(PfxUInt32 rbId, const PfxBv &bv, PfxBool areMeshIslandIndicesSaved, void *userData);
typedef PfxBool(*pfxTraverseRigidBodyCacheContainerCallbackForRayClipping)(PfxUInt32 rbId, const PfxBv &bv, PfxBool areMeshIslandIndicesSaved, PfxRayInput &clipRay, PfxFloat &tmin, void *userData);
typedef PfxBool(*pfxTraverseLargeTriMeshCacheContainerCallback)(const PfxCacheLargeTriMesh &largeTriMeshCache, const PfxUInt32 islandIds[], void *userData);

struct PfxRigidBodyCacheManager {
private:
	PfxUInt32 m_maxRigidBodies;
	PfxUInt32 m_maxLargeTriMeshes;
	PfxUInt32 m_maxLargeTriMeshIslands;
	PfxUInt32 m_numBvNodesBvh;
	PfxUInt32 m_numBvhMeshes;
	PfxUInt32 m_numArrayMeshes;
	PfxUInt32 m_numLargeTriMeshIslands;

	// The array to prevent from duplicate rigid body
	PfxBool *m_rigidBodyExists;

	// The array of flags showing whether the mesh is saved island-by-island
	PfxBool *m_areMeshIslandIndicesSaved;

	// The array saving the bvNodes
	PfxCacheBvNode *m_bvNodesBvh;

	// The array saving the islands of largeTriMeshes
	PfxCacheLargeTriMesh *m_bvhMeshes;
	PfxCacheLargeTriMesh *m_arrayMeshes;
	PfxUInt32 *m_largeTriMeshIslandIndices;
	
	PfxBool SCE_PFX_FORCE_INLINE isIslandExisted(PfxUInt32 rbId, PfxUInt32 shapeId, PfxUInt32 islandId);

	template<typename CacheRangeType>
	PfxInt32 SCE_PFX_FORCE_INLINE addMeshInternal(PfxUInt32 rbId, PfxUInt32 shapeId, const PfxRigidState &state, const PfxShape &shape, const CacheRangeType &range);

	template<typename CacheRangeType>
	PfxInt32 SCE_PFX_FORCE_INLINE addRbInternal(const PfxRigidBodyCacheParam &param, CacheRangeType &range);

	PfxInt32 addMesh(PfxUInt32 rbId, PfxUInt32 shapeId, const PfxRigidState &state, const PfxShape &shape, const PfxTransform3 &rbTransform, PfxBv &rangeBv);
	PfxInt32 addMesh(PfxUInt32 rbId, PfxUInt32 shapeId, const PfxRigidState &state, const PfxShape &shape, const PfxTransform3 &rbTransform, PfxRigidBodyCacheRangeSphere &rangeSphere);
	PfxInt32 addMesh(PfxUInt32 rbId, PfxUInt32 shapeId, const PfxRigidState &state, const PfxShape &shape, const PfxTransform3 &rbTransform, PfxRigidBodyCacheRangeRayInternal &rangeRay);

public:
	static inline PfxUInt32 getBytes(PfxUInt32 maxRigidBodies, PfxUInt32 maxLargeTriMeshes, PfxUInt32 maxIslandsPerMesh);

	PfxInt32 initialize(PfxUInt32 maxRigidBodies, PfxUInt32 maxLargeTriMeshes, PfxUInt32 maxIslandsPerMesh, void *workBuff, PfxUInt32 workBytes);

	void finalize();

	void clear();

	PfxInt32 addRb(const PfxRigidBodyCacheParam &param, const PfxRigidBodyCacheRangeAabb &rangeAabb);
	PfxInt32 addRb(const PfxRigidBodyCacheParam &param, const PfxRigidBodyCacheRangeSphere &rangeSphere);
	PfxInt32 addRb(const PfxRigidBodyCacheParam &param, const PfxRigidBodyCacheRangeRay &rangeRay);
	PfxInt32 addRb(const PfxRigidBodyCacheParam &param, PfxUInt32 rbId);

	void traverseRayOverlap(
		pfxTraverseRigidBodyCacheContainerCallback rigidBodyFuncPtr, 
		pfxTraverseLargeTriMeshCacheContainerCallback largeTriMeshBvhFuncPtr,
		pfxTraverseLargeTriMeshCacheContainerCallback largeTriMeshArrayFuncPtr,
		const PfxTraverseRigidBodyCacheContainerCallbackParam &param, void *userData) const;

	void traverseRayClosest(
		pfxTraverseRigidBodyCacheContainerCallbackForRayClipping rigidBodyFuncPtr,
		pfxTraverseLargeTriMeshCacheContainerCallback largeTriMeshBvhFuncPtr,
		pfxTraverseLargeTriMeshCacheContainerCallback largeTriMeshArrayFuncPtr,
		const PfxTraverseRigidBodyCacheContainerCallbackParam &param, void *userData) const;
};

inline PfxUInt32 PfxRigidBodyCacheManager::getBytes(PfxUInt32 maxRigidBodies, PfxUInt32 maxLargeTriMeshes, PfxUInt32 maxIslandsPerMesh)
{
	PfxUInt32 bytes = 0u;
	bytes += sizeof(PfxBool) * maxRigidBodies;
	bytes += sizeof(PfxBool) * maxRigidBodies;
	bytes += sizeof(PfxCacheBvNode) * maxRigidBodies * 2;
	bytes += sizeof(PfxCacheLargeTriMesh) * maxLargeTriMeshes * 2;
	bytes += sizeof(PfxUInt32) * maxLargeTriMeshes * maxIslandsPerMesh;
	return bytes;
}

} //namespace pfxv4
} //namespace sce

#endif

