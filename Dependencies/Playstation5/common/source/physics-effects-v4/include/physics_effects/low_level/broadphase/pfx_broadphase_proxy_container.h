/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_BROADPHASE_PROXY_CONTAINER_H_
#define _SCE_PFX_BROADPHASE_PROXY_CONTAINER_H_

#include "../../base_level/base/pfx_common.h"
#include "../../base_level/rigidbody/pfx_rigid_state.h"
#include "../../base_level/collision/pfx_collidable.h"

namespace sce {
namespace pfxv4 {

/// @brief Broadphase proxy container
/// @details This container stores AABBs of rigid bodies in an acceleration structure such as BVH.
struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxBroadphaseProxyContainer {
	PfxUInt8 reserved[64];
};

/// @brief Query bytes of the work buffer
/// @details Query bytes of the work buffer needed for the broadphase proxy container.
/// @param maxProxies Maximum number of proxies stored in the container
/// @param maxIndexOfProxies Maximum index of proxies ( = rigid body ID )
/// @return Return bytes of the buffer
SCE_PFX_API PfxUInt32 pfxBroadphaseProxyContainerQueryMem(PfxUInt32 maxProxies, PfxUInt32 maxIndexOfProxies);

/// @brief Initialize the broadphase proxy container
/// @details You can create multiple broadphase proxy containers.
/// Each rigid body must have unique ID in global.
/// @param[in,out] broadphaseProxyContainer Broadphase proxy container
/// @param maxProxies Maximum number of rigid bodies stored in the container
/// @param maxIndexOfProxies Maximum index of proxies ( = rigid body ID )
/// @param workBuff Work buffer
/// @param workBytes Bytes of the work buffer
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxBroadphaseProxyContainerInit(PfxBroadphaseProxyContainer &broadphaseProxyContainer,
	PfxUInt32 maxProxies,PfxUInt32 maxIndexOfProxies,void *workBuff,PfxUInt32 workBytes);

/// @brief Terminate the broadphase proxy container
/// @details Terminate the broadphase proxy container.
/// @param[in,out] broadphaseProxyContainer Broadphase proxy container
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxBroadphaseProxyContainerTerm(PfxBroadphaseProxyContainer &broadphaseProxyContainer);

/// @brief Clear the broadphase proxy container
/// @details Clear the content of the broadphase proxy container.
/// @param[in,out] broadphaseProxyContainer Broadphase proxy container
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxBroadphaseProxyContainerClear(PfxBroadphaseProxyContainer &broadphaseProxyContainer);

/// @brief Get the number of proxies
/// @details Get the number of proxies stored in the broadphase proxy container.
/// @param broadphaseProxyContainer Broadphase proxy container
/// @return Return the number of proxies
SCE_PFX_API PfxUInt32 pfxBroadphaseProxyContainerGetNumProxies(const PfxBroadphaseProxyContainer &broadphaseProxyContainer);

/// @brief Get the capacity of proxies
/// @details Get the possible number of proxies stored in the broadphase proxy container.
/// @param broadphaseProxyContainer Broadphase proxy container
/// @return Return the capacity of proxies
SCE_PFX_API PfxUInt32 pfxBroadphaseProxyContainerGetCapacity(const PfxBroadphaseProxyContainer &broadphaseProxyContainer);

/// @brief Insert a proxy
/// @details Insert a proxy to the broadphase proxy container. If a proxy already exists in the container, it is updated.
/// @param[in,out] broadphaseProxyContainer Broadphase proxy container
/// @param proxyId Proxy ID ( = rigid body ID )
/// @param state State
/// @param collidable Collidable
/// @param timeStep Time step
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxBroadphaseProxyContainerInsertProxy(PfxBroadphaseProxyContainer &broadphaseProxyContainer, PfxUInt32 proxyId, PfxRigidState &state, PfxCollidable &collidable, PfxFloat timeStep );

/// @brief Update a proxy
/// @details Update a proxy in the broadphase proxy container.
/// @param[in,out] broadphaseProxyContainer Broadphase proxy container
/// @param proxyId Proxy ID ( = rigid body ID )
/// @param state State
/// @param collidable Collidable
/// @param timeStep Time step
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxBroadphaseProxyContainerUpdateProxy( PfxBroadphaseProxyContainer &broadphaseProxyContainer, PfxUInt32 proxyId, PfxRigidState &state, PfxCollidable &collidable, PfxFloat timeStep );

/// @brief Remove a proxy
/// @details Remove a proxy from the broadphase proxy container
/// @param[in,out] broadphaseProxyContainer Broadphase proxy container
/// @param proxyId Proxy ID ( = rigid body ID )
/// @return Return SCE_PFX_OK(0) upon normal termination.
/// @return Return SCE_PFX_ERR_INVALID_VALUE if the specified proxy is not found.
SCE_PFX_API PfxInt32 pfxBroadphaseProxyContainerRemoveProxy(PfxBroadphaseProxyContainer &broadphaseProxyContainer,PfxUInt32 proxyId);

/// @brief Query size of a proxy AABB
/// @details Query the AABB size of the proxies stored in the broadphase proxy container.
/// @param broadphaseProxyContainer Broadphase proxy container
/// @param proxyId Proxy ID ( = rigid body ID )
/// @param[out] center Center of the AABB
/// @param[out] extent Half extent of the AABB
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxBroadphaseProxyContainerGetAabb(PfxBroadphaseProxyContainer &broadphaseProxyContainer,
	PfxUInt32 proxyId,PfxLargePosition &center,PfxVector3 &extent);

/// @brief Query the broadphase proxy container
/// @details This callback is used to query the broadphase proxy container.
/// @param proxyId Proxy ID ( = rigid body ID )
/// @param aabbMin Minimum position of the AABB
/// @param aabbMax Maximum position of the AABB
/// @param userData User data
/// @return Return true if query continues.
typedef PfxBool(*pfxBroadphaseProxyContainerQueryCallback)(PfxUInt32 proxyId,
	const PfxLargePosition &aabbMin,const PfxLargePosition &aabbMax,void *userData);

/// @brief Query proxies
/// @details Query all proxies stored in the broadphase proxy container.
/// @param broadphaseProxyContainer Broadphase proxy container
/// @param callback Callback
/// @param userData User data transferred to the callback
SCE_PFX_API void pfxBroadphaseProxyContainerTraverse(const PfxBroadphaseProxyContainer &broadphaseProxyContainer,
	pfxBroadphaseProxyContainerQueryCallback callback,void *userData);

/// @brief Query proxies overlapped with the sphere
/// @details Query proxies overlapped with the specified sphere.
/// @param broadphaseProxyContainer Broadphase proxy container
/// @param sphereCenter Center position of the sphere
/// @param sphereRadius Radius of the sphere
/// @param callback Callback
/// @param userData User data transferred to the callback
SCE_PFX_API void pfxBroadphaseProxyContainerQuerySphereOverlap(const PfxBroadphaseProxyContainer &broadphaseProxyContainer,
	const PfxLargePosition &sphereCenter,const PfxFloat sphereRadius,
	pfxBroadphaseProxyContainerQueryCallback callback,void *userData);

/// @brief Query proxies overlapped with the specified AABB
/// @details Query proxies overlapped with the specified AABB.
/// @param broadphaseProxyContainer Broadphase proxy container
/// @param aabbMin Minimum position of the AABB
/// @param aabbMax Maximum position of the AABB
/// @param callback Callback
/// @param userData User data transferred to the callback
SCE_PFX_API void pfxBroadphaseProxyContainerQueryAabbOverlap(const PfxBroadphaseProxyContainer &broadphaseProxyContainer,
	const PfxLargePosition &aabbMin,const PfxLargePosition &aabbMax,
	pfxBroadphaseProxyContainerQueryCallback callback,void *userData);

/// @brief Query proxies intersected with the specified Ray
/// @details Query proxies intersected with the specified Ray.
/// @param broadphaseProxyContainer Broadphase proxy container
/// @param startPosition Start position of the Ray
/// @param direction Direction vector of the Ray
/// @param radius Radius of the Ray
/// @param callback Callback
/// @param userData User data transferred to the callback
SCE_PFX_API void pfxBroadphaseProxyContainerQueryRayIntersect(const PfxBroadphaseProxyContainer &broadphaseProxyContainer,
	const PfxLargePosition &startPosition,const PfxVector3 &direction,PfxFloat radius,
	pfxBroadphaseProxyContainerQueryCallback callback,void *userData);

/// @brief Query the size of a serialized buffer of the broadphase proxy container
/// @details Query the size of a serialized buffer of the broadphase proxy container
/// @param broadphaseProxyContainer Broadphase proxy container
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxUInt32 pfxBroadphaseProxyContainerQuerySerializeBytes(const PfxBroadphaseProxyContainer &broadphaseProxyContainer);

/// @brief Save the broadphase proxy container to a buffer
/// @details Save the broadphase proxy container to a buffer. The size of buffer should be enough for string the container.
/// Use pfxBroadphaseProxyContainerQuerySerializeBytes() to get the proper size of a serialied buffer.
/// @param broadphaseProxyContainer Broadphase proxy container
/// @param buffer Pointer to a buffer storing the serialized container
/// @param bytes Size of a buffer
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxBroadphaseProxyContainerWrite(const PfxBroadphaseProxyContainer &broadphaseProxyContainer, PfxUInt8 *buffer, PfxUInt32 bytes);

/// @brief Load the broadphase proxy container from a buffer
/// @details Load the broadphase proxy container from a buffer. The specified container has to have enough space for a serialized data.
/// @param broadphaseProxyContainer Broadphase proxy container
/// @param buffer Pointer to a buffer storing the serialized container
/// @param bytes Size of a buffer
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxBroadphaseProxyContainerRead(PfxBroadphaseProxyContainer &broadphaseProxyContainer, const PfxUInt8 *buffer, PfxUInt32 bytes);

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_BROADPHASE_PROXY_CONTAINER_H_ */
