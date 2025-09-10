/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_RIGID_BODY_CACHE_CONTAINER_H_
#define _SCE_PFX_RIGID_BODY_CACHE_CONTAINER_H_


#include "../../base_level/base/pfx_common.h"
#include "../../base_level/rigidbody/pfx_rigid_state.h"
#include "../../base_level/collision/pfx_collidable.h"
#include "../../base_level/base/pfx_large_position.h"
#include "../../low_level/broadphase/pfx_broadphase_proxy_container.h"

namespace sce {
namespace pfxv4 {

/// @name Cache container for ray cast

/// @brief Rigid body cache container
/// @details This container caches the rigid bodies for ray cast.
struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxRigidBodyCacheContainer{
	PfxUInt8 reserved[128];
};

/// @brief Callback for rigid body cache
/// @details This callback is called when a rigid body in the specified range is found.
/// @param state Rigid body state
/// @param collidable Collision shapes
/// @param userData User data
/// @return Return true to store this rigid body into the cache container. Otherwise, this rigid body will not be stored.
typedef PfxBool(*pfxRigidBodyCacheFilterCallback)(const PfxRigidState &state, const PfxCollidable &collidable, void *userData);

/// @brief Parameters used for pfxRigidBodyCacheContainerAdd()
/// @details Input parameter for pfxRigidBodyCacheContainerAdd().
struct SCE_PFX_API PfxRigidBodyCacheParam {
	PfxUInt32 numRigidBodies = 0;			///< @brief Number of rigid bodies
	PfxRigidState *states = nullptr;		///< @brief Array of the rigid body states
	PfxCollidable *collidables = nullptr;	///< @brief Array of the collision shapes
	PfxBroadphaseProxyContainer *proxyContainer = nullptr;	///< @brief Broadphase proxy container
	pfxRigidBodyCacheFilterCallback callback = nullptr;		///< @brief Callback to filter rigid bodies
	void *userData = nullptr;								///< @brief User data transferred to the callback
};

/// @brief Aabb range used for pfxRigidBodyCacheContainerAdd()
/// @details Specifies an aabb range for pfxRigidBodyCacheContainerAdd().
struct PfxRigidBodyCacheRangeAabb {
	PfxLargePosition vmin;	///< @brief Minimum position of the range.
	PfxLargePosition vmax;	///< @brief Maximum position of the range.
};

/// @brief Sphere range used for pfxRigidBodyCacheContainerAdd()
/// @details Specifies the sphere range for pfxRigidBodyCacheContainerAdd().
struct PfxRigidBodyCacheRangeSphere {
	PfxLargePosition center;	///< @brief Center of the sphere range
	PfxFloat radius;			///< @brief Radius of the sphere range
};

/// @brief Ray range used for pfxRigidBodyCacheContainerAdd()
/// @details Specifies the ray range for pfxRigidBodyCacheContainerAdd().
struct PfxRigidBodyCacheRangeRay {
	PfxLargePosition startPosition;		///< @brief Start position of the ray range
	PfxVector3 direction;				///< @brief Direction of the ray range
	PfxFloat radius;					///< @brief Radius of the ray range
};

/// @brief Query bytes of the work buffer
/// @details Query bytes of the work buffer needed for rigid body cache container. 
/// @param maxRigidBodies Maximum number of rigid bodies stored in the container
/// @param maxLargeTriMeshes Maximum number of large triangle meshes stored in the container
/// @return Return bytes of the buffer
SCE_PFX_API PfxUInt32 pfxRigidBodyCacheContainerQueryMem(PfxUInt32 maxRigidBodies, PfxUInt32 maxLargeTriMeshes);

/// @brief Query bytes of the work buffer
/// @details Query bytes of the work buffer needed for rigid body cache container. 
/// If pfxRigidBodyCacheContainerAdd() returns SCE_PFX_ERR_OUT_OF_BUFFER, raising the value of maxBatchesPerMesh to greater than 256 may solve the error.
/// @param maxRigidBodies Maximum number of rigid bodies stored in the container
/// @param maxLargeTriMeshes Maximum number of large triangle meshes stored in the container
/// @param maxBatchesPerMesh Maximum number of batches to which a large triangle mesh can be divided. Its default value is 256.
/// @return Return bytes of the buffer
SCE_PFX_API PfxUInt32 pfxRigidBodyCacheContainerQueryMem(PfxUInt32 maxRigidBodies, PfxUInt32 maxLargeTriMeshes, PfxUInt32 maxBatchesPerMesh);

/// @brief Initialize the rigid body cache container
/// @details Initialize the rigid body cache container.
/// @param[in,out] rigidBodyCacheContainer rigid body container
/// @param maxRigidBodies Maximum number of rigid bodies stored in the container
/// @param maxLargeTriMeshes Maximum number of large triangle meshes stored in the container
/// @param workBuff Work buffer
/// @param workBytes Bytes of the work buffer
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxRigidBodyCacheContainerInit(PfxRigidBodyCacheContainer &rigidBodyCacheContainer, PfxUInt32 maxRigidBodies, PfxUInt32 maxLargeTriMeshes, void *workBuff, PfxUInt32 workBytes);

/// @brief Initialize the rigid body cache container
/// @details Initialize the rigid body cache container.
/// If pfxRigidBodyCacheContainerAdd() returns SCE_PFX_ERR_OUT_OF_BUFFER, raising the value of maxBatchesPerMesh to greater than 256 may solve the error.
/// @param[in,out] rigidBodyCacheContainer rigid body container
/// @param maxRigidBodies Maximum number of rigid bodies stored in the container
/// @param maxLargeTriMeshes Maximum number of large triangle meshes stored in the container
/// @param maxBatchesPerMesh Maximum number of batches to which a large triangle mesh can be divided. Its default value is 256.
/// @param workBuff Work buffer
/// @param workBytes Bytes of the work buffer
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxRigidBodyCacheContainerInit(PfxRigidBodyCacheContainer &rigidBodyCacheContainer, PfxUInt32 maxRigidBodies, PfxUInt32 maxLargeTriMeshes, PfxUInt32 maxBatchesPerMesh, void *workBuff, PfxUInt32 workBytes);

/// @brief Terminate the rigid body cache container
/// @details Terminate the rigid body cache container.
/// @param[in,out] rigidBodyCacheContainer Rigid body cache container
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxRigidBodyCacheContainerTerm(PfxRigidBodyCacheContainer &rigidBodyCacheContainer);

/// @brief Clear the rigid body cache container
/// @details Clear the content of the rigid body cache container.
/// @param[in,out] rigidBodyCacheContainer Rigid body cache container
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxRigidBodyCacheContainerClear(PfxRigidBodyCacheContainer &rigidBodyCacheContainer);

/// @brief Add the rigid bodies in the aabb range to the cache container
/// @details Add the rigid bodies in the aabb range to the cache container
/// @param[in,out] rigidBodyCacheContainer Rigid body cache container
/// @param param Input parameter
/// @param rangeAabb Aabb range
/// @return Return SCE_PFX_OK(0) upon normal termination.
/// @return Return SCE_PFX_ERR_OUT_OF_BUFFER if there isn't enough work buffer.
/// @return Return SCE_PFX_ERR_INVALID_VALUE if param contains NULL pointer. 
SCE_PFX_API PfxInt32 pfxRigidBodyCacheContainerAdd(PfxRigidBodyCacheContainer &rigidBodyCacheContainer, const PfxRigidBodyCacheParam &param, const PfxRigidBodyCacheRangeAabb &rangeAabb);

/// @brief Add the rigid bodies in the sphere range to the cache container
/// @details Add the rigid bodies in the sphere range to the cache container
/// @param[in,out] rigidBodyCacheContainer Rigid body cache container
/// @param param Input parameter
/// @param rangeSphere Sphere range
/// @return Return SCE_PFX_OK(0) upon normal termination.
/// @return Return SCE_PFX_ERR_OUT_OF_BUFFER if there isn't enough buffer.
/// @return Return SCE_PFX_ERR_INVALID_VALUE if param contains NULL pointer.
SCE_PFX_API PfxInt32 pfxRigidBodyCacheContainerAdd(PfxRigidBodyCacheContainer &rigidBodyCacheContainer, const PfxRigidBodyCacheParam &param, const PfxRigidBodyCacheRangeSphere &rangeSphere);

/// @brief Add the rigid bodies in the ray range to the cache container
/// @details Add the rigid bodies in the ray range to the cache container
/// @param[in,out] rigidBodyCacheContainer Rigid body cache container
/// @param param Input parameter
/// @param rangeRay Ray range
/// @return Return SCE_PFX_OK(0) upon normal termination.
/// @return Return SCE_PFX_ERR_OUT_OF_BUFFER if there isn't enough buffer.
/// @return Return SCE_PFX_ERR_INVALID_VALUE if param contains NULL pointer.
SCE_PFX_API PfxInt32 pfxRigidBodyCacheContainerAdd(PfxRigidBodyCacheContainer &rigidBodyCacheContainer, const PfxRigidBodyCacheParam &param, const PfxRigidBodyCacheRangeRay &rangeRay);

/// @brief Add the rigid bodies in the ray range to the cache container
/// @details Add the rigid bodies in the ray range to the cache container
/// @param[in,out] rigidBodyCacheContainer Rigid body cache container
/// @param param Input parameter
/// @param rigidBodyId Id of the rigid body to be cached
/// @return Return SCE_PFX_OK(0) upon normal termination.
/// @return Return SCE_PFX_ERR_OUT_OF_BUFFER if there isn't enough buffer.
/// @return Return SCE_PFX_ERR_INVALID_VALUE if param contains NULL pointer.
SCE_PFX_API PfxInt32 pfxRigidBodyCacheContainerAdd(PfxRigidBodyCacheContainer &rigidBodyCacheContainer, const PfxRigidBodyCacheParam &param, PfxUInt32 rigidBodyIndex);


} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_CONTACT_CONTAINER_H_ */
