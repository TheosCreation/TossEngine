/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/low_level/collision/pfx_ray_cast.h"
#include "../../../include/physics_effects/base_level/collision/pfx_intersect_ray_func.h"
#include "../../../include/physics_effects/util/pfx_static_array.h"
#include "../../base_level/collision/pfx_intersect_common.h"
#include "../../base_level/collision/pfx_large_tri_mesh_impl.h"
#include "../../base_level/collision/pfx_intersect_ray_all_facets.h"
#include "../../base_level/broadphase/pfx_check_collidable.h"
#include "../../base_level/collision/pfx_intersect_ray_large_tri_mesh.h"
#include "../broadphase/pfx_progressive_bvh.h"
#include "../collision/pfx_rigid_body_cache_manager.h"

namespace sce {
namespace pfxv4 {

struct PfxTraverseRayCallbackParam {
	PfxRayOutput *out;
	pfxRayHitCallback callback;
	void *userData;
	const PfxRayInput *ray;
	const PfxRigidState *states;
	const PfxCollidable *collidables;
	PfxFloat t0,t1;
	PfxFloat tmin = 1.0f;
	pfxRayHitDiscardTriangleCallback discardTriangleCallback;
	void *userDataForDiscardingTriangle;
};

static PfxBool findClosestHitCallbackForCache(PfxUInt32 rigidBodyId,const PfxBv &bv,PfxBool areMeshIslandIndicesSaved,PfxRayInput &clipRay,PfxFloat &tmin,void *userData)
{
	PfxTraverseRayCallbackParam *param = (PfxTraverseRayCallbackParam*)userData;
	
	const PfxRigidState &state = param->states[rigidBodyId];
	const PfxCollidable &coll = param->collidables[rigidBodyId];
	
	if(	pfxCheckContactFilter(state.getContactFilterSelf(),state.getContactFilterTarget(),
			param->ray->m_contactFilterSelf,param->ray->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0),state.getCollisionIgnoreGroup(1),
			param->ray->m_collisionIgnoreGroup[0],param->ray->m_collisionIgnoreGroup[1])) {
		
		PfxTransform3 transform(state.getOrientation(), state.getPosition());
		
		PfxRayInputInternal iray;
		
		PfxLargePosition lposA = state.getLargePosition();
		PfxLargePosition tmp = param->ray->m_startPosition;
		tmp.changeSegment(lposA.segment);
		iray.m_segment = lposA.segment;
		iray.m_startPosition = tmp.offset;
		iray.m_direction = param->ray->m_direction;
		iray.m_contactFilterSelf = param->ray->m_contactFilterSelf;
		iray.m_contactFilterTarget = param->ray->m_contactFilterTarget;
		iray.m_facetMode = param->ray->m_facetMode;
		
		for(PfxUInt32 j=0;j<coll.getNumShapes();j++) {
			const PfxShape &shape = coll.getShape(j);

			if (areMeshIslandIndicesSaved && shape.getType() == kPfxShapeLargeTriMesh)
				continue;
			
			if(!pfxCheckContactFilter(shape.getContactFilterSelf(),shape.getContactFilterTarget(),param->ray->m_contactFilterSelf,param->ray->m_contactFilterTarget)) {
				continue;
			}
			
			PfxTransform3 offsetTr = appendScale(shape.getOffsetTransform(),shape.getScaleXyz());
			PfxTransform3 worldTransform = transform * offsetTr;
			
			PfxRayOutputInternal iout;
			iout.m_variable = param->out->m_variable;
			
			if(pfxGetIntersectRayFunc(shape.getType())(iray,iout,shape,worldTransform, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
				if(iout.m_subData.getType() == PfxSubData::SHAPE_INFO) {
					iout.m_subData.setShapeId(j);
					iout.m_subData.setUserData(shape.getUserData());
				}
				
				param->out->m_contactPoint.segment = iray.m_segment;
				param->out->m_contactPoint.offset = iray.m_startPosition + iout.m_variable * iray.m_direction;
				param->out->m_contactFlag = iout.m_contactFlag;
				param->out->m_contactNormal = iout.m_contactNormal;
				param->out->m_subData = iout.m_subData;
				param->out->m_variable = iout.m_variable;
				param->out->m_shapeId = j;
				param->out->m_objectId = rigidBodyId;
			}
		}
	}

	if (tmin > 0.0f && param->out->m_contactFlag && param->out->m_variable < tmin) {
		clipRay.m_direction = param->out->m_variable * param->ray->m_direction;
		tmin = param->out->m_variable;
	}
	
	return true;
}

static PfxBool findClosestHitCallback(PfxUInt32 rigidBodyId, const PfxBv &bv, PfxRayInput &clipRay, PfxFloat &tmin, void *userData) {
	return findClosestHitCallbackForCache(rigidBodyId, bv, false, clipRay, tmin, userData);
}

static PfxBool findAllHitsCallbackForCache(PfxUInt32 rigidBodyId, const PfxBv &bv, PfxBool areMeshIslandIndicesSaved, void *userData)
{
	PfxTraverseRayCallbackParam *param = (PfxTraverseRayCallbackParam*)userData;

	const PfxRigidState &state = param->states[rigidBodyId];
	const PfxCollidable &coll = param->collidables[rigidBodyId];

	if (pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->ray->m_contactFilterSelf, param->ray->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->ray->m_collisionIgnoreGroup[0], param->ray->m_collisionIgnoreGroup[1])) {
		PfxTransform3 transform(state.getOrientation(), state.getPosition());

		PfxRayInputInternal iray;

		PfxLargePosition lposA = state.getLargePosition();
		PfxLargePosition tmp = param->ray->m_startPosition;
		tmp.changeSegment(lposA.segment);
		iray.m_segment = lposA.segment;
		iray.m_startPosition = tmp.offset;
		iray.m_direction = param->ray->m_direction;
		iray.m_contactFilterSelf = param->ray->m_contactFilterSelf;
		iray.m_contactFilterTarget = param->ray->m_contactFilterTarget;
		iray.m_facetMode = param->ray->m_facetMode;

		for (PfxUInt32 j = 0; j<coll.getNumShapes(); j++) {
			const PfxShape &shape = coll.getShape(j);

			if (areMeshIslandIndicesSaved && shape.getType() == kPfxShapeLargeTriMesh)
				continue;

			if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->ray->m_contactFilterSelf, param->ray->m_contactFilterTarget)) {
				continue;
			}

			PfxTransform3 offsetTr = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
			PfxTransform3 worldTransform = transform * offsetTr;

			PfxRayOutputInternal iout;
			iout.m_variable = 1.0f;

			if (pfxGetIntersectRayFunc(shape.getType())(iray, iout, shape, worldTransform, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
				if (iout.m_subData.getType() == PfxSubData::SHAPE_INFO) {
					iout.m_subData.setShapeId(j);
					iout.m_subData.setUserData(shape.getUserData());
				}

				PfxRayOutput out;
				out.m_contactPoint.segment = iray.m_segment;
				out.m_contactPoint.offset = iray.m_startPosition + iout.m_variable * iray.m_direction;
				out.m_contactFlag = iout.m_contactFlag;
				out.m_contactNormal = iout.m_contactNormal;
				out.m_subData = iout.m_subData;
				out.m_variable = param->t0 + iout.m_variable * (param->t1 - param->t0);
				out.m_shapeId = j;
				out.m_objectId = rigidBodyId;
				if (!param->callback(out, param->userData)) {
					return false;
				}
			}
		}
	}
	return true;
}

static PfxBool findAllHitsCallback(PfxUInt32 rigidBodyId, const PfxBv &bv, void *userData) {
	return findAllHitsCallbackForCache(rigidBodyId, bv, false, userData);
}

static PfxBool findClosestHitLargeTriMeshCallbackForBvh(const PfxCacheLargeTriMesh &largeTriMeshCache, const PfxUInt32 islandIds[], void *userData)
{
	PfxTraverseRayCallbackParam *param = (PfxTraverseRayCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->ray->m_contactFilterSelf, param->ray->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->ray->m_collisionIgnoreGroup[0], param->ray->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->ray->m_contactFilterSelf, param->ray->m_contactFilterTarget))
		return true;

	// Ray from local to world
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition rayLargeStartPosition = param->ray->m_startPosition;
	rayLargeStartPosition.changeSegment(state.getSegment());

	PfxRayInputInternal rayIn;
	rayIn.m_segment = rayLargeStartPosition.segment;
	rayIn.m_startPosition = rayLargeStartPosition.offset;
	rayIn.m_direction = param->ray->m_direction;
	rayIn.m_contactFilterSelf = param->ray->m_contactFilterSelf;
	rayIn.m_contactFilterTarget = param->ray->m_contactFilterTarget;
	rayIn.m_facetMode = param->ray->m_facetMode;

	PfxRayOutputInternal rayOut;
	rayOut.m_variable = param->out->m_variable;

	if (pfxIntersectRayIslandsOfLargeTriMeshBvh(rayIn, rayOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, param->discardTriangleCallback, param->userDataForDiscardingTriangle))
	{
		param->out->m_contactPoint = rayLargeStartPosition;
		param->out->m_contactPoint.offset += rayOut.m_variable * rayIn.m_direction;
		param->out->m_contactNormal = rayOut.m_contactNormal;
		param->out->m_contactFlag = rayOut.m_contactFlag;
		param->out->m_subData = rayOut.m_subData;
		param->out->m_variable = rayOut.m_variable;
		param->out->m_shapeId = largeTriMeshCache.m_meshShapeIndex;
		param->out->m_objectId = largeTriMeshCache.m_meshRbIndex;
	}

	return true;
}

static PfxBool findClosestHitLargeTriMeshCallbackForArray(const PfxCacheLargeTriMesh &largeTriMeshCache, const PfxUInt32 islandIds[], void *userData)
{
	PfxTraverseRayCallbackParam *param = (PfxTraverseRayCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->ray->m_contactFilterSelf, param->ray->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->ray->m_collisionIgnoreGroup[0], param->ray->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->ray->m_contactFilterSelf, param->ray->m_contactFilterTarget))
		return true;

	// Ray from local to world
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition rayLargeStartPosition = param->ray->m_startPosition;
	rayLargeStartPosition.changeSegment(state.getSegment());

	PfxRayInputInternal rayIn;
	rayIn.m_segment = rayLargeStartPosition.segment;
	rayIn.m_startPosition = rayLargeStartPosition.offset;
	rayIn.m_direction = param->ray->m_direction;
	rayIn.m_contactFilterSelf = param->ray->m_contactFilterSelf;
	rayIn.m_contactFilterTarget = param->ray->m_contactFilterTarget;
	rayIn.m_facetMode = param->ray->m_facetMode;

	PfxRayOutputInternal rayOut;
	rayOut.m_variable = param->out->m_variable;

	if(pfxIntersectRayIslandsOfLargeTriMeshArray(rayIn, rayOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, param->discardTriangleCallback, param->userDataForDiscardingTriangle))
	{
		param->out->m_contactPoint = rayLargeStartPosition;
		param->out->m_contactPoint.offset += rayOut.m_variable * rayIn.m_direction;
		param->out->m_contactNormal = rayOut.m_contactNormal;
		param->out->m_contactFlag = rayOut.m_contactFlag;
		param->out->m_subData = rayOut.m_subData;
		param->out->m_variable = rayOut.m_variable;
		param->out->m_shapeId = largeTriMeshCache.m_meshShapeIndex;
		param->out->m_objectId = largeTriMeshCache.m_meshRbIndex;
	}

	return true;
}

static PfxBool findAllHitsLargeTriMeshCallbackForBvh(const PfxCacheLargeTriMesh &largeTriMeshCache, const PfxUInt32 islandIds[], void *userData)
{
	PfxTraverseRayCallbackParam *param = (PfxTraverseRayCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->ray->m_contactFilterSelf, param->ray->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->ray->m_collisionIgnoreGroup[0], param->ray->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->ray->m_contactFilterSelf, param->ray->m_contactFilterTarget))
		return true;

	// Ray from local to world
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition rayLargeStartPosition = param->ray->m_startPosition;
	rayLargeStartPosition.changeSegment(state.getSegment());

	PfxRayInputInternal rayIn;
	rayIn.m_segment = rayLargeStartPosition.segment;
	rayIn.m_startPosition = rayLargeStartPosition.offset;
	rayIn.m_direction = param->ray->m_direction;
	rayIn.m_contactFilterSelf = param->ray->m_contactFilterSelf;
	rayIn.m_contactFilterTarget = param->ray->m_contactFilterTarget;
	rayIn.m_facetMode = param->ray->m_facetMode;

	PfxRayOutputInternal rayOut;
	rayOut.m_variable = 1.f;

	if (pfxIntersectRayIslandsOfLargeTriMeshBvh(rayIn, rayOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, param->discardTriangleCallback, param->userDataForDiscardingTriangle))
	{
		PfxRayOutput out;
		out.m_contactPoint = rayLargeStartPosition;
		out.m_contactPoint.offset += rayOut.m_variable * rayIn.m_direction;
		out.m_contactFlag = rayOut.m_contactFlag;
		out.m_contactNormal = rayOut.m_contactNormal;
		out.m_subData = rayOut.m_subData;
		out.m_variable = param->t0 + rayOut.m_variable * (param->t1 - param->t0);
		out.m_shapeId = largeTriMeshCache.m_meshShapeIndex;
		out.m_objectId = largeTriMeshCache.m_meshRbIndex;
		if (!param->callback(out, param->userData)) {
			return false;
		}
	}
	return true;
}

static PfxBool findAllHitsLargeTriMeshCallbackForArray(const PfxCacheLargeTriMesh &largeTriMeshCache, const PfxUInt32 islandIds[], void *userData)
{
	PfxTraverseRayCallbackParam *param = (PfxTraverseRayCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->ray->m_contactFilterSelf, param->ray->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->ray->m_collisionIgnoreGroup[0], param->ray->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->ray->m_contactFilterSelf, param->ray->m_contactFilterTarget))
		return true;
	
	// Ray from local to world
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition rayLargeStartPosition = param->ray->m_startPosition;
	rayLargeStartPosition.changeSegment(state.getSegment());

	PfxRayInputInternal rayIn;
	rayIn.m_segment = rayLargeStartPosition.segment;
	rayIn.m_startPosition = rayLargeStartPosition.offset;
	rayIn.m_direction = param->ray->m_direction;
	rayIn.m_contactFilterSelf = param->ray->m_contactFilterSelf;
	rayIn.m_contactFilterTarget = param->ray->m_contactFilterTarget;
	rayIn.m_facetMode = param->ray->m_facetMode;

	PfxRayOutputInternal rayOut;
	rayOut.m_variable = 1.f;

	if (pfxIntersectRayIslandsOfLargeTriMeshArray(rayIn, rayOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, param->discardTriangleCallback, param->userDataForDiscardingTriangle))
	{
		PfxRayOutput out;
		out.m_contactPoint = rayLargeStartPosition;
		out.m_contactPoint.offset += rayOut.m_variable * rayIn.m_direction;
		out.m_contactFlag = rayOut.m_contactFlag;
		out.m_contactNormal = rayOut.m_contactNormal;
		out.m_subData = rayOut.m_subData;
		out.m_variable = param->t0 + rayOut.m_variable * (param->t1 - param->t0);
		out.m_shapeId = largeTriMeshCache.m_meshShapeIndex;
		out.m_objectId = largeTriMeshCache.m_meshRbIndex;
		if (!param->callback(out, param->userData)) {
			return false;
		}
	}
	return true;
}

PfxInt32 pfxCastSingleRay(const PfxRayInput &ray,PfxRayOutput &out,const PfxRayCastParam &param)
{
	if (!param.states || !param.collidables) return SCE_PFX_ERR_INVALID_VALUE;
	if (param.containerType == PfxRayCastParam::PROXY_CONTAINER && param.proxyContainer == NULL) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	if (param.containerType == PfxRayCastParam::CACHE_CONTAINER && param.cacheContainer == NULL)	return SCE_PFX_ERR_INVALID_VALUE;
	
	out.m_variable = 1.0f;
	out.m_contactFlag = false;
	
	// レイを領域でクリップ
	if(pfxClipRayByDotProduct(ray.m_startPosition, ray.m_direction, param.rangeMin, param.rangeMax)) {
		PfxFloat t0 = 0.0f;
		PfxFloat t1 = 1.0f;
		pfxClipRay(ray.m_startPosition, ray.m_direction, param.rangeMin, param.rangeMax, t0, t1);

		PfxRayInput ray_ = ray;
		ray_.m_startPosition = ray.m_startPosition + t0 * ray.m_direction;
		ray_.m_direction = (t1-t0) * ray.m_direction;
		PfxRayOutput out_ = out;
		PfxTraverseRayCallbackParam param_;
		param_.ray = &ray_;
		param_.out = &out_;
		param_.t0 = t0;
		param_.t1 = t1;
		param_.tmin = 1.0f;
		param_.states = param.states;
		param_.collidables = param.collidables;
		param_.discardTriangleCallback = param.discardTriangleCallback;
		param_.userDataForDiscardingTriangle = param.userDataForDiscardingTriangle;
		
		if (param.containerType == PfxRayCastParam::PROXY_CONTAINER) {
			PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)param.proxyContainer;
			bvh->traverseRayClosest(findClosestHitCallback, ray_, 0.0f, &param_);
		}

		else if (param.containerType == PfxRayCastParam::CACHE_CONTAINER) {
			PfxLargePosition rayEndPosition = ray_.m_startPosition + ray_.m_direction;
			PfxTraverseRigidBodyCacheContainerCallbackParam cacheParam;
			cacheParam.states = param.states;
			cacheParam.collidables = param.collidables;
			cacheParam.rangeOfInterest.vmin = minPerElem(ray_.m_startPosition, rayEndPosition);
			cacheParam.rangeOfInterest.vmax = maxPerElem(ray_.m_startPosition, rayEndPosition);

			// Cast to all cache containers
			const PfxRigidBodyCacheManager *cacheManager = ((const PfxRigidBodyCacheManager*)param.cacheContainer);
			cacheManager->traverseRayClosest(findClosestHitCallbackForCache, findClosestHitLargeTriMeshCallbackForBvh, findClosestHitLargeTriMeshCallbackForArray, cacheParam, &param_);
		}
		if (out_.m_contactFlag && out_.m_variable < out.m_variable)
			out = out_;

		out.m_variable = t0 + out.m_variable * (t1 - t0);
	}

	return SCE_PFX_OK;
}

PfxInt32 pfxCastSingleRay(const PfxRayInput &ray,pfxRayHitCallback callback,void *userData,const PfxRayCastParam &param)
{
	if(!callback) return SCE_PFX_OK;
	if (!param.states || !param.collidables)	return SCE_PFX_ERR_INVALID_VALUE;
	if (param.containerType == PfxRayCastParam::PROXY_CONTAINER && param.proxyContainer == NULL) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	if (param.containerType == PfxRayCastParam::CACHE_CONTAINER && param.cacheContainer == NULL)	return SCE_PFX_ERR_INVALID_VALUE;

	// レイを領域でクリップ
	if (pfxClipRayByDotProduct(ray.m_startPosition, ray.m_direction, param.rangeMin, param.rangeMax)) {
		PfxFloat t0 = 0.0f;
		PfxFloat t1 = 1.0f;
		pfxClipRay(ray.m_startPosition, ray.m_direction, param.rangeMin, param.rangeMax, t0, t1);

		PfxRayInput ray_ = ray;
		ray_.m_startPosition = ray.m_startPosition + t0 * ray.m_direction;
		ray_.m_direction = (t1 - t0) * ray.m_direction;
		PfxTraverseRayCallbackParam param_;
		param_.ray = &ray_;
		param_.t0 = t0;
		param_.t1 = t1;
		param_.states = param.states;
		param_.collidables = param.collidables;
		param_.callback = callback;
		param_.userData = userData;
		param_.discardTriangleCallback = param.discardTriangleCallback;
		param_.userDataForDiscardingTriangle = param.userDataForDiscardingTriangle;

		if (param.containerType == PfxRayCastParam::PROXY_CONTAINER) {
			PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)param.proxyContainer;
			bvh->traverseRayOverlap(findAllHitsCallback, ray, 0.0f, &param_);
		}

		else if (param.containerType == PfxRayCastParam::CACHE_CONTAINER) {
			PfxLargePosition rayEndPosition = ray_.m_startPosition + ray_.m_direction;
			PfxTraverseRigidBodyCacheContainerCallbackParam cacheParam;
			cacheParam.states = param.states;
			cacheParam.collidables = param.collidables;
			cacheParam.rangeOfInterest.vmin = minPerElem(ray_.m_startPosition, rayEndPosition);
			cacheParam.rangeOfInterest.vmax = maxPerElem(ray_.m_startPosition, rayEndPosition);

			const PfxRigidBodyCacheManager *cacheManager = ((const PfxRigidBodyCacheManager*)param.cacheContainer);
			cacheManager->traverseRayOverlap(findAllHitsCallbackForCache, findAllHitsLargeTriMeshCallbackForBvh, findAllHitsLargeTriMeshCallbackForArray, cacheParam, &param_);
		}
	}

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
