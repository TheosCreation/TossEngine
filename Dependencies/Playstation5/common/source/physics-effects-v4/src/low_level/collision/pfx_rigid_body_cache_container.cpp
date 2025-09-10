/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
*                Copyright (C) 2020 Sony Interactive Entertainment Inc.
*                                                
*/

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/low_level/collision/pfx_rigid_body_cache_container.h"
#include "../../../src/low_level/collision/pfx_rigid_body_cache_manager.h"

namespace sce {
namespace pfxv4 {

PfxUInt32 pfxRigidBodyCacheContainerQueryMem(PfxUInt32 maxRigidBodies, PfxUInt32 maxLargeTriMeshes, PfxUInt32 maxBatchesPerMesh)
{
	PfxUInt32 bytes = 0u;
	bytes += PfxRigidBodyCacheManager::getBytes(maxRigidBodies, maxLargeTriMeshes, maxBatchesPerMesh);
	bytes += 16;
	return bytes;
}

PfxUInt32 pfxRigidBodyCacheContainerQueryMem(PfxUInt32 maxRigidBodies, PfxUInt32 maxLargeTriMeshes)
{
	return pfxRigidBodyCacheContainerQueryMem(maxRigidBodies, maxLargeTriMeshes, 256);
}

PfxInt32 pfxRigidBodyCacheContainerInit(PfxRigidBodyCacheContainer &rigidBodyCacheContainer, PfxUInt32 maxRigidBodies, PfxUInt32 maxLargeTriMeshes, PfxUInt32 maxBatchesPerMesh, void *workBuff, PfxUInt32 workBytes)
{
	SCE_PFX_ASSERT(sizeof(PfxRigidBodyCacheContainer) > sizeof(PfxRigidBodyCacheManager));

	if (workBytes < pfxRigidBodyCacheContainerQueryMem(maxRigidBodies, maxLargeTriMeshes, maxBatchesPerMesh)) {
		return SCE_PFX_ERR_OUT_OF_BUFFER;
	}

	return ((PfxRigidBodyCacheManager*)&rigidBodyCacheContainer)->initialize(maxRigidBodies, maxLargeTriMeshes, maxBatchesPerMesh, workBuff, workBytes);
}

PfxInt32 pfxRigidBodyCacheContainerInit(PfxRigidBodyCacheContainer &rigidBodyCacheContainer, PfxUInt32 maxRigidBodies, PfxUInt32 maxLargeTriMeshes, void *workBuff, PfxUInt32 workBytes)
{
	return pfxRigidBodyCacheContainerInit(rigidBodyCacheContainer, maxRigidBodies, maxLargeTriMeshes, 256u, workBuff, workBytes);
}

PfxInt32 pfxRigidBodyCacheContainerTerm(PfxRigidBodyCacheContainer &rigidBodyCacheContainer)
{
	((PfxRigidBodyCacheManager*)&rigidBodyCacheContainer)->finalize();

	return SCE_PFX_OK;
}

PfxInt32 pfxRigidBodyCacheContainerClear(PfxRigidBodyCacheContainer &rigidBodyCacheContainer)
{
	((PfxRigidBodyCacheManager*)&rigidBodyCacheContainer)->clear();

	return SCE_PFX_OK;
}

PfxInt32 pfxRigidBodyCacheContainerAdd(PfxRigidBodyCacheContainer &rigidBodyCacheContainer, const PfxRigidBodyCacheParam &param, const PfxRigidBodyCacheRangeAabb &rangeAabb)
{
	return ((PfxRigidBodyCacheManager*)&rigidBodyCacheContainer)->addRb(param, rangeAabb);
}

PfxInt32 pfxRigidBodyCacheContainerAdd(PfxRigidBodyCacheContainer &rigidBodyCacheContainer, const PfxRigidBodyCacheParam &param, const PfxRigidBodyCacheRangeSphere &rangeSphere)
{
	return ((PfxRigidBodyCacheManager*)&rigidBodyCacheContainer)->addRb(param, rangeSphere);
}

PfxInt32 pfxRigidBodyCacheContainerAdd(PfxRigidBodyCacheContainer &rigidBodyCacheContainer, const PfxRigidBodyCacheParam &param, const PfxRigidBodyCacheRangeRay &rangeRay)
{
	return ((PfxRigidBodyCacheManager*)&rigidBodyCacheContainer)->addRb(param, rangeRay);
}

PfxInt32 pfxRigidBodyCacheContainerAdd(PfxRigidBodyCacheContainer &rigidBodyCacheContainer, const PfxRigidBodyCacheParam &param, PfxUInt32 rigidBodyIndex)
{
	return ((PfxRigidBodyCacheManager*)&rigidBodyCacheContainer)->addRb(param, rigidBodyIndex);
}

} //namespace pfxv4
} //namespace sce
