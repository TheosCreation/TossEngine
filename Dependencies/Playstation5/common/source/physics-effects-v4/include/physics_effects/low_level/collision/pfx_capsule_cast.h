/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CAPSULE_CAST_H
#define _SCE_PFX_CAPSULE_CAST_H

#include "../../base_level/rigidbody/pfx_rigid_state.h"
#include "../../base_level/collision/pfx_collidable.h"
#include "../../base_level/collision/pfx_ray.h"
#include "../../low_level/broadphase/pfx_broadphase_proxy_container.h"
#include "../../low_level/collision/pfx_rigid_body_cache_container.h"

///////////////////////////////////////////////////////////////////////////////
// CapsuleCast

namespace sce {
namespace pfxv4 {

/// @brief Parameters used for capsulecast
/// @details Input parameter for pfxCastSingleCapsule().
struct SCE_PFX_API PfxCapsuleCastParam {
	PfxRigidState *states = nullptr;							///< @brief Array of the rigid body states
	PfxCollidable *collidables = nullptr;						///< @brief Array of the collision shapes
	PfxLargePosition rangeMin = PfxLargePosition();				///< @brief Minimum position of the area
	PfxLargePosition rangeMax = PfxLargePosition();				///< @brief Maximum position of the area
	union {
		PfxBroadphaseProxyContainer *proxyContainer = nullptr;	///< @brief Broadphase proxy container
		PfxRigidBodyCacheContainer *cacheContainer;
	};
	enum ContainerType {
		PROXY_CONTAINER,
		CACHE_CONTAINER
	} containerType = PROXY_CONTAINER;									///< @brief Type of the container
	pfxRayHitDiscardTriangleCallback discardTriangleCallback = nullptr;	///< @brief Callback to discard triangles
	void *userDataForDiscardingTriangle = nullptr;						///< @brief User data for discarding triangles
};

/// @brief Cast single capsule
/// @details Return the closest intersection from the starting position of the single capsule.
/// @param capsuleIn Capsule
/// @param[out] capsuleOut Closest intersection
/// @param param Input parameter
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxCastSingleCapsule(const PfxCapsuleInput &capsuleIn,PfxCapsuleOutput &capsuleOut,const PfxCapsuleCastParam &param);

/// @brief Capsulecast intersection callback
/// @details This callback is called when an intersection is detected.
/// @param hit Intersection information
/// @param userData User data
/// @return Return true if capsule casting continues. Return false to stop.
typedef PfxBool(*pfxCapsuleHitCallback)(const PfxCapsuleOutput &hit,void *userData);

/// @brief Cast single capsule
/// @details Find intersection between the capsule and the collision shapes.
/// Calls callback when it finds a hit in random order.
/// @param capsuleIn Capsule
/// @param callback Callback
/// @param userData User data transferred to the callback
/// @param param Input parameter
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxCastSingleCapsule(const PfxCapsuleInput &capsuleIn,pfxCapsuleHitCallback callback,void *userData,const PfxCapsuleCastParam &param);

} //namespace pfxv4
} //namespace sce
 
#endif // _SCE_PFX_CAPSULE_CAST_H
