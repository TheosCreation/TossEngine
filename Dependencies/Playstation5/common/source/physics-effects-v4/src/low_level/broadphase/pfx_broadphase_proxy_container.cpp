/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2021 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_progressive_bvh.h"
#include "pfx_broadphase_stage.h"
#include "../../../src/util/pfx_binary_reader_writer.h"
#include "../../../include/physics_effects/low_level/broadphase/pfx_broadphase_proxy_container.h"
#include "../../../include/physics_effects/util/pfx_static_array.h"

#define SCE_PFX_BROADPHASE_PROXY_CONTAINER_HEADER_SIZE 64

namespace sce {
namespace pfxv4 {

PfxUInt32 pfxBroadphaseProxyContainerQueryMem(PfxUInt32 maxProxies, PfxUInt32 maxIndexOfProxies)
{
	return PfxProgressiveBvh::queryBytes(maxProxies, maxIndexOfProxies);
}

PfxInt32 pfxBroadphaseProxyContainerInit(PfxBroadphaseProxyContainer &broadphaseProxyContainer,PfxUInt32 maxProxies,PfxUInt32 maxIndexOfProxies,void *workBuff,PfxUInt32 workBytes)
{
	SCE_PFX_ASSERT(sizeof(PfxProgressiveBvh) <= sizeof(PfxBroadphaseProxyContainer));

	if(maxProxies > SCE_PFX_MAX_RIGIDBODY || maxIndexOfProxies >= SCE_PFX_MAX_RIGIDBODY) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	if(workBytes < pfxBroadphaseProxyContainerQueryMem(maxProxies,maxIndexOfProxies)) {
		return SCE_PFX_ERR_OUT_OF_BUFFER;
	}

	PfxProgressiveBvh *bvh = new(&broadphaseProxyContainer) PfxProgressiveBvh; // to call the constructor
	bvh->initialize(maxProxies,maxIndexOfProxies,workBuff,workBytes);
	
	return SCE_PFX_OK;
}

PfxInt32 pfxBroadphaseProxyContainerTerm(PfxBroadphaseProxyContainer &broadphaseProxyContainer)
{
	return SCE_PFX_OK;
}

PfxInt32 pfxBroadphaseProxyContainerClear(PfxBroadphaseProxyContainer &broadphaseProxyContainer)
{
	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)&broadphaseProxyContainer;
	bvh->clear();
	
	return SCE_PFX_OK;
}

PfxUInt32 pfxBroadphaseProxyContainerGetNumProxies(const PfxBroadphaseProxyContainer &broadphaseProxyContainer)
{
	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)&broadphaseProxyContainer;
	return bvh->getNumProxies();
}

PfxUInt32 pfxBroadphaseProxyContainerGetCapacity(const PfxBroadphaseProxyContainer &broadphaseProxyContainer)
{
	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)&broadphaseProxyContainer;
	return bvh->getCapacity();
}

PfxInt32 pfxBroadphaseProxyContainerGetAabb(PfxBroadphaseProxyContainer &broadphaseProxyContainer,PfxUInt32 proxyId,PfxLargePosition &center,PfxVector3 &extent)
{
	PfxLargePosition vmin,vmax;

	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)&broadphaseProxyContainer;
	if(!bvh->getBv(proxyId,vmin,vmax)) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	
	center = vmax + vmin;
	center.segment = center.segment.halve();
	center.offset = center.offset *= 0.5f;
	PfxLargePosition tmp = vmax - vmin;
	tmp.segment = tmp.segment.halve();
	tmp.offset = tmp.offset *= 0.5f;
	extent = tmp.convertToVector3();
	
	return SCE_PFX_OK;
}

PfxInt32 pfxBroadphaseProxyContainerInsertProxy(PfxBroadphaseProxyContainer &broadphaseProxyContainer, PfxUInt32 proxyId, PfxRigidState &state, PfxCollidable &collidable, PfxFloat timeStep )
{
	if( !state.getIsInitialized() ) return SCE_PFX_ERR_INVALID_VALUE;
	
	PfxSegment segment = state.getSegment();
	PfxVector3 center, extent;

	calculateAabb( state, collidable, timeStep, center, extent );

	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)&broadphaseProxyContainer;
	if(!bvh->submit(state.getRigidBodyId(),
		state.getContactFilterSelf(),
		state.getContactFilterTarget(),
		state.getMotionMask(),
		state.getSolverQuality(),
		state.getCollisionIgnoreGroup(0),
		state.getCollisionIgnoreGroup(1),
		PfxLargePosition( segment, center ),
		extent )) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	state.setProxyShiftFlag( true );

	return SCE_PFX_OK;
}

PfxInt32 pfxBroadphaseProxyContainerUpdateProxy( PfxBroadphaseProxyContainer &broadphaseProxyContainer, PfxUInt32 proxyId, PfxRigidState &state, PfxCollidable &collidable, PfxFloat timeStep )
{
	if( !state.getIsInitialized() ) return SCE_PFX_ERR_INVALID_VALUE;

	PfxSegment segment = state.getSegment();
	PfxVector3 center, extent;

	calculateAabb( state, collidable, timeStep, center, extent );

	PfxProgressiveBvh *bvh = ( PfxProgressiveBvh* )&broadphaseProxyContainer;
	if( bvh->update( state.getRigidBodyId(),
		state.getContactFilterSelf(),
		state.getContactFilterTarget(),
		state.getMotionMask(),
		state.getSolverQuality(),
		state.getCollisionIgnoreGroup( 0 ),
		state.getCollisionIgnoreGroup( 1 ),
		PfxLargePosition( segment, center ),
		extent,
		true ) == PfxProgressiveBvh::kUpdateBvhAABBChanged) {
		state.setProxyShiftFlag( true );
	}

	return SCE_PFX_OK;
}

PfxInt32 pfxBroadphaseProxyContainerRemoveProxy(PfxBroadphaseProxyContainer &broadphaseProxyContainer,PfxUInt32 proxyId)
{
	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)&broadphaseProxyContainer;

	if(bvh->remove(proxyId)) {
		return SCE_PFX_OK;
	}
	
	return SCE_PFX_ERR_INVALID_VALUE;
}

struct DbvhCallbackParam {
	pfxBroadphaseProxyContainerQueryCallback userCallback;
	void *userData;
};

static PfxBool traverseProxyContainerCallback(PfxUInt32 proxyId,const PfxBv &bv,void *userData)
{
	DbvhCallbackParam *param = (DbvhCallbackParam*)userData;
	
	return ((pfxBroadphaseProxyContainerQueryCallback)param->userCallback)(proxyId,bv.vmin,bv.vmax,param->userData);
}

void pfxBroadphaseProxyContainerTraverse(const PfxBroadphaseProxyContainer &broadphaseProxyContainer,pfxBroadphaseProxyContainerQueryCallback callback,void *userData)
{
	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)&broadphaseProxyContainer;
	
	if(!callback || bvh->empty()) {
		return;
	}
	
	DbvhCallbackParam param;
	param.userCallback = callback;
	param.userData = userData;
	bvh->traverse(traverseProxyContainerCallback,&param);
}

void pfxBroadphaseProxyContainerQuerySphereOverlap(const PfxBroadphaseProxyContainer &broadphaseProxyContainer,
	const PfxLargePosition &sphereCenter,const PfxFloat sphereRadius,
	pfxBroadphaseProxyContainerQueryCallback callback,void *userData)
{
	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)&broadphaseProxyContainer;

	if(!callback || bvh->empty()) {
		return;
	}

	DbvhCallbackParam param;
	param.userCallback = callback;
	param.userData = userData;
	
	bvh->traverseSphereOverlap(traverseProxyContainerCallback,sphereCenter,sphereRadius,&param);
}

void pfxBroadphaseProxyContainerQueryAabbOverlap(const PfxBroadphaseProxyContainer &broadphaseProxyContainer,
	const PfxLargePosition &aabbMin,const PfxLargePosition &aabbMax,
	pfxBroadphaseProxyContainerQueryCallback callback,void *userData)
{
	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)&broadphaseProxyContainer;

	if(!callback || bvh->empty()) {
		return;
	}
	
	DbvhCallbackParam param;
	param.userCallback = callback;
	param.userData = userData;
	
	PfxBv bv;
	bv.vmin = aabbMin;
	bv.vmax = aabbMax;
	
	bvh->traverseProxyOverlap(traverseProxyContainerCallback,bv,&param);
}

void pfxBroadphaseProxyContainerQueryRayIntersect(const PfxBroadphaseProxyContainer &broadphaseProxyContainer,
	const PfxLargePosition &startPosition,const PfxVector3 &direction,PfxFloat radius,
	pfxBroadphaseProxyContainerQueryCallback callback,void *userData)
{
	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)&broadphaseProxyContainer;

	if(!callback || bvh->empty()) {
		return;
	}
	
	DbvhCallbackParam param;
	param.userCallback = callback;
	param.userData = userData;
	
	PfxRayInput ray;
	ray.m_startPosition = startPosition;
	ray.m_direction = direction;
	
	bvh->traverseRayOverlap(traverseProxyContainerCallback,ray,radius,&param);
}

PfxUInt32 pfxBroadphaseProxyContainerQuerySerializeBytes(const PfxBroadphaseProxyContainer &broadphaseProxyContainer)
{
	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)&broadphaseProxyContainer;

	return SCE_PFX_BROADPHASE_PROXY_CONTAINER_HEADER_SIZE + 
		bvh->querySerializeBytes();
}

PfxInt32 pfxBroadphaseProxyContainerWrite(const PfxBroadphaseProxyContainer &broadphaseProxyContainer, PfxUInt8 *buffer, PfxUInt32 bytes)
{
	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)&broadphaseProxyContainer;

	if(!buffer) return SCE_PFX_ERR_INVALID_VALUE;
	if(bytes < pfxBroadphaseProxyContainerQuerySerializeBytes(broadphaseProxyContainer)) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxUInt32 proxyContainerBytes = bvh->querySerializeBytes();
	PfxUInt32 capacity = bvh->getCapacity();
	PfxUInt32 maxProxyId = bvh->getMaxProxies();

	PfxUInt8 *p = buffer;

	writeUInt32(&p, proxyContainerBytes);
	writeUInt32(&p, capacity);
	writeUInt32(&p, maxProxyId);

	PfxUInt32 padding = (PfxUInt32)(SCE_PFX_BROADPHASE_PROXY_CONTAINER_HEADER_SIZE - (p - buffer));
	p += padding;

	PfxInt32 ret = bvh->save(p, proxyContainerBytes);
	if(ret != SCE_PFX_OK) return ret;
	
	return SCE_PFX_OK;
}

PfxInt32 pfxBroadphaseProxyContainerRead(PfxBroadphaseProxyContainer &broadphaseProxyContainer, const PfxUInt8 *buffer, PfxUInt32 bytes)
{
	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)&broadphaseProxyContainer;

	if (!buffer || bytes < SCE_PFX_BROADPHASE_PROXY_CONTAINER_HEADER_SIZE) return SCE_PFX_ERR_INVALID_VALUE;

	const PfxUInt8 *p = buffer;

	PfxUInt32 proxyContainerBytes = 0;
	PfxUInt32 capacity = 0;
	PfxUInt32 maxProxyId = 0;

	readUInt32(&p, proxyContainerBytes);
	readUInt32(&p, capacity);
	readUInt32(&p, maxProxyId);

	PfxUInt32 padding = (PfxUInt32)(SCE_PFX_BROADPHASE_PROXY_CONTAINER_HEADER_SIZE - (p - buffer));
	p += padding;

	PfxInt32 ret = bvh->load(p, proxyContainerBytes);
	if(ret != SCE_PFX_OK) return ret;

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
