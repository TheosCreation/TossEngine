/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_SPHERE_CAST_H
#define _SCE_PFX_SPHERE_CAST_H

#include "../../base_level/rigidbody/pfx_rigid_state.h"
#include "../../base_level/collision/pfx_collidable.h"
#include "../../base_level/collision/pfx_ray.h"
#include "../../low_level/broadphase/pfx_broadphase_proxy_container.h"
#include "../../low_level/collision/pfx_rigid_body_cache_container.h"

///////////////////////////////////////////////////////////////////////////////
// SphereCast

namespace sce {
namespace pfxv4 {

/// @brief Parameters used for spherecast
/// @details Input parameter for pfxCastSingleSphere().
struct SCE_PFX_API PfxSphereCastParam {
	PfxRigidState *states = nullptr;								///< @brief Array of the rigid body states
	PfxCollidable *collidables = nullptr;							///< @brief Array of the collision shapes
	PfxLargePosition rangeMin = PfxLargePosition();					///< @brief Minimum position of the area
	PfxLargePosition rangeMax = PfxLargePosition();					///< @brief Maximum position of the area
	union {
		PfxBroadphaseProxyContainer *proxyContainer = nullptr;		///< @brief Broadphase proxy container
		PfxRigidBodyCacheContainer *cacheContainer;					///< @brief Rigid body cache container
	};
	enum {
		PROXY_CONTAINER,
		CACHE_CONTAINER
	} containerType = PROXY_CONTAINER;									///< @brief Type of the container
	pfxRayHitDiscardTriangleCallback discardTriangleCallback = nullptr;	///< @brief Callback to discard triangles
	void *userDataForDiscardingTriangle = nullptr;						///< @brief User data for discarding triangles
};

/// @brief Cast single sphere
/// @details Return the closest intersection from the starting position of the single sphere.
/// @param sphereIn Sphere
/// @param[out] sphereOut Closest intersection
/// @param param Input parameter
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxCastSingleSphere(const PfxSphereInput &sphereIn,PfxSphereOutput &sphereOut,const PfxSphereCastParam &param);

/// @brief Spherecast intersection callback
/// @details This callback is called when an intersection is detected.
/// @param hit Intersection information
/// @param userData User data
/// @return Return true if sphere casting continues. Return false to stop.
typedef PfxBool(*pfxSphereHitCallback)(const PfxSphereOutput &hit,void *userData);

/// @brief Cast single sphere
/// @details Find intersection between the sphere and the collision shapes.
/// Calls callback when it finds a hit in random order.
/// @param sphereIn Sphere
/// @param callback Callback
/// @param userData User data transferred to the callback
/// @param param Input parameter
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxCastSingleSphere(const PfxSphereInput &sphereIn,pfxSphereHitCallback callback,void *userData,const PfxSphereCastParam &param);

} //namespace pfxv4
} //namespace sce
 
#endif // _SCE_PFX_SPHERE_CAST_H
