/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CIRCLE_CAST_H
#define _SCE_PFX_CIRCLE_CAST_H

#include "../../base_level/rigidbody/pfx_rigid_state.h"
#include "../../base_level/collision/pfx_collidable.h"
#include "../../base_level/collision/pfx_ray.h"
#include "../../low_level/broadphase/pfx_broadphase_proxy_container.h"
#include "../../low_level/collision/pfx_rigid_body_cache_container.h"

///////////////////////////////////////////////////////////////////////////////
// CapsuleCast

namespace sce {
namespace pfxv4 {

/// @brief Parameters used for circlecast
/// @details Input parameter for pfxCastSingleCircle().
struct SCE_PFX_API PfxCircleCastParam {
	PfxRigidState *states = nullptr;							///< @brief Array of the rigid body states
	PfxCollidable *collidables = nullptr;						///< @brief Array of the collision shapes
	PfxLargePosition rangeMin = PfxLargePosition();				///< @brief Minimum position of the area
	PfxLargePosition rangeMax = PfxLargePosition();				///< @brief Maximum position of the area
	union {
		PfxBroadphaseProxyContainer *proxyContainer = nullptr;	///< @brief Broadphase proxy container
		PfxRigidBodyCacheContainer *cacheContainer;				///< @brief Rigid body cache container
	};
	enum ContainerType {
		PROXY_CONTAINER,
		CACHE_CONTAINER
	} containerType = PROXY_CONTAINER;									///< @brief Type of the container
	pfxRayHitDiscardTriangleCallback discardTriangleCallback = nullptr;	///< @brief Callback to discard triangles
	void *userDataForDiscardingTriangle = nullptr;						///< @brief User data for discarding triangle
};

/// @brief Cast single circle
/// @details Return the closest intersection from the starting position of the single circle.
/// @param circleIn Circle
/// @param[out] circleOut Closest intersection
/// @param param Input parameter
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxCastSingleCircle(const PfxCircleInput &circleIn,PfxCircleOutput &circleOut,const PfxCircleCastParam &param);

/// @brief Circlecast intersection callback
/// @details This callback is called when an intersection is detected.
/// @param hit Intersection information
/// @param userData User data
/// @return Return true if circle casting continues. Return false to stop.
typedef PfxBool(*pfxCircleHitCallback)(const PfxCircleOutput &hit,void *userData);

/// @brief Cast single circle
/// @details Find intersection between the circle and the collision shapes.
/// Calls callback when it finds a hit in random order.
/// @param circleIn Circle
/// @param callback Callback
/// @param userData User data transferred to the callback
/// @param param Input parameter
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxCastSingleCircle(const PfxCircleInput &circleIn,pfxCircleHitCallback callback,void *userData,const PfxCircleCastParam &param);

} //namespace pfxv4
} //namespace sce
 
#endif // _SCE_PFX_CIRCLE_CAST_H
