/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/collision/pfx_intersect_moving_circle_func.h"
#include "../../../include/physics_effects/low_level/collision/pfx_circle_cast.h"
#include "../../../include/physics_effects/util/pfx_static_array.h"
#include "../../base_level/collision/pfx_intersect_common.h"
#include "../../base_level/collision/pfx_large_tri_mesh_impl.h"
#include "../../base_level/collision/pfx_intersect_moving_circle_all_facets.h"
#include "../../base_level/collision/pfx_intersect_moving_circle_large_tri_mesh.h"
#include "../../base_level/broadphase/pfx_check_collidable.h"
#include "../broadphase/pfx_progressive_bvh.h"
#include "../collision/pfx_rigid_body_cache_manager.h"

namespace sce {
namespace pfxv4 {

struct PfxTraverseCircleCallbackParam {
	PfxCircleOutput *circleOut;
	pfxCircleHitCallback callback;
	void *userData;
	const PfxCircleInput *circleIn;
	const PfxRigidState *states;
	const PfxCollidable *collidables;
	PfxFloat t0,t1;
	pfxRayHitDiscardTriangleCallback discardTriangleCallback;
	void *userDataForDiscardingTriangle;
};

static PfxBool findClosestHitCallbackForCache(PfxUInt32 rigidBodyId,const PfxBv &bv,PfxBool areMeshIslandIndicesSaved,PfxRayInput &clipRay,PfxFloat &tmin,void *userData)
{
	PfxTraverseCircleCallbackParam *param = (PfxTraverseCircleCallbackParam*)userData;
	
	const PfxRigidState &state = param->states[rigidBodyId];
	const PfxCollidable &coll = param->collidables[rigidBodyId];
	
	if( pfxCheckContactFilter(state.getContactFilterSelf(),state.getContactFilterTarget(),
			param->circleIn->m_contactFilterSelf,param->circleIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0),state.getCollisionIgnoreGroup(1),
			param->circleIn->m_collisionIgnoreGroup[0],param->circleIn->m_collisionIgnoreGroup[1])) {
		
		PfxTransform3 transform(state.getOrientation(), state.getPosition());
		
		PfxCircleInputInternal icircle;
		
		PfxLargePosition lposA = state.getLargePosition();
		PfxLargePosition tmp = param->circleIn->m_startPosition;
		tmp.changeSegment(lposA.segment);
		icircle.m_segment = lposA.segment;
		icircle.m_startPosition = tmp.offset;
		icircle.m_direction = param->circleIn->m_direction;
		icircle.m_orientation = param->circleIn->m_orientation;
		icircle.m_radius = param->circleIn->m_radius;
		icircle.m_contactFilterSelf = param->circleIn->m_contactFilterSelf;
		icircle.m_contactFilterTarget = param->circleIn->m_contactFilterTarget;
		
		for(PfxUInt32 j=0;j<coll.getNumShapes();j++) {
			const PfxShape &shape = coll.getShape(j);
			
			if (areMeshIslandIndicesSaved && shape.getType() == kPfxShapeLargeTriMesh)
				continue;

			if(!pfxCheckContactFilter(shape.getContactFilterSelf(),shape.getContactFilterTarget(),param->circleIn->m_contactFilterSelf,param->circleIn->m_contactFilterTarget)) {
				continue;
			}
			
			PfxTransform3 offsetTr = appendScale(shape.getOffsetTransform(),shape.getScaleXyz());
			PfxTransform3 worldTransform = transform * offsetTr;

			PfxCircleOutputInternal iout;
			iout.m_variable = param->circleOut->m_variable;

			if(pfxGetIntersectMovingCircleFunc(shape.getType())(icircle,iout,shape,worldTransform, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
				if(iout.m_subData.getType() == PfxSubData::SHAPE_INFO) {
					iout.m_subData.setShapeId(j);
					iout.m_subData.setUserData(shape.getUserData());
				}
				
				param->circleOut->m_contactPoint.segment = icircle.m_segment;
				param->circleOut->m_contactPoint.offset = iout.m_contactPoint;
				param->circleOut->m_contactFlag = iout.m_contactFlag;
				param->circleOut->m_contactNormal = iout.m_contactNormal;
				param->circleOut->m_subData = iout.m_subData;
				param->circleOut->m_variable = iout.m_variable;
				param->circleOut->m_shapeId = j;
				param->circleOut->m_objectId = rigidBodyId;
			}
		}
	}
	
	if (param->circleOut->m_contactFlag && param->circleOut->m_variable < tmin) {
		clipRay.m_direction = param->circleOut->m_variable * param->circleIn->m_direction;
		tmin = param->circleOut->m_variable;
	}

	return true;
}

static PfxBool findClosestHitCallback(PfxUInt32 rigidBodyId, const PfxBv &bv, PfxRayInput &clipRay, PfxFloat &tmin, void *userData) {
	return findClosestHitCallbackForCache(rigidBodyId, bv, false, clipRay, tmin, userData);
}

static PfxBool findAllHitsCallbackForCache(PfxUInt32 rigidBodyId, const PfxBv &bv, PfxBool areMeshIslandIndicesSaved, void *userData)
{
	PfxTraverseCircleCallbackParam *param = (PfxTraverseCircleCallbackParam*)userData;

	const PfxRigidState &state = param->states[rigidBodyId];
	const PfxCollidable &coll = param->collidables[rigidBodyId];

	if (pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->circleIn->m_contactFilterSelf, param->circleIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->circleIn->m_collisionIgnoreGroup[0], param->circleIn->m_collisionIgnoreGroup[1])) {

		PfxTransform3 transform(state.getOrientation(), state.getPosition());

		PfxCircleInputInternal icircle;

		PfxLargePosition lposA = state.getLargePosition();
		PfxLargePosition tmp = param->circleIn->m_startPosition;
		tmp.changeSegment(lposA.segment);
		icircle.m_segment = lposA.segment;
		icircle.m_startPosition = tmp.offset;
		icircle.m_direction = param->circleIn->m_direction;
		icircle.m_orientation = param->circleIn->m_orientation;
		icircle.m_radius = param->circleIn->m_radius;
		icircle.m_contactFilterSelf = param->circleIn->m_contactFilterSelf;
		icircle.m_contactFilterTarget = param->circleIn->m_contactFilterTarget;

		for (PfxUInt32 j = 0; j<coll.getNumShapes(); j++) {
			const PfxShape &shape = coll.getShape(j);

			if (areMeshIslandIndicesSaved && shape.getType() == kPfxShapeLargeTriMesh)
				continue;

			if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->circleIn->m_contactFilterSelf, param->circleIn->m_contactFilterTarget)) {
				continue;
			}

			PfxTransform3 offsetTr = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
			PfxTransform3 worldTransform = transform * offsetTr;

			PfxCircleOutputInternal iout;
			iout.m_variable = 1.0f;

			if (pfxGetIntersectMovingCircleFunc(shape.getType())(icircle, iout, shape, worldTransform, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
				if (iout.m_subData.getType() == PfxSubData::SHAPE_INFO) {
					iout.m_subData.setShapeId(j);
					iout.m_subData.setUserData(shape.getUserData());
				}

				PfxCircleOutput out;
				out.m_contactPoint.segment = icircle.m_segment;
				out.m_contactPoint.offset = iout.m_contactPoint;
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
	PfxTraverseCircleCallbackParam *param = (PfxTraverseCircleCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->circleIn->m_contactFilterSelf, param->circleIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->circleIn->m_collisionIgnoreGroup[0], param->circleIn->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->circleIn->m_contactFilterSelf, param->circleIn->m_contactFilterTarget))
		return true;

	// CircleCast from local to world
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition circleLargeStartPosition = param->circleIn->m_startPosition;
	circleLargeStartPosition.changeSegment(state.getSegment());

	PfxCircleInputInternal circleIn;
	circleIn.m_segment = circleLargeStartPosition.segment;
	circleIn.m_startPosition = circleLargeStartPosition.offset;
	circleIn.m_direction = param->circleIn->m_direction;
	circleIn.m_orientation = param->circleIn->m_orientation;
	circleIn.m_contactFilterSelf = param->circleIn->m_contactFilterSelf;
	circleIn.m_contactFilterTarget = param->circleIn->m_contactFilterTarget;
	circleIn.m_radius = param->circleIn->m_radius;

	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
	PfxFloat radiusLocal = param->circleIn->m_radius / minElem(absPerElem(scale));

	PfxCircleOutputInternal circleOut;
	circleOut.m_variable = param->circleOut->m_variable;

	if (pfxIntersectMovingCircleIslandsOfLargeTriMeshBvh(circleIn, circleOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, flipTriangle, radiusLocal, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
		param->circleOut->m_contactPoint.segment = circleLargeStartPosition.segment;
		param->circleOut->m_contactPoint.offset = circleOut.m_contactPoint;
		param->circleOut->m_contactFlag = circleOut.m_contactFlag;
		param->circleOut->m_contactNormal = circleOut.m_contactNormal;
		param->circleOut->m_subData = circleOut.m_subData;
		param->circleOut->m_variable = circleOut.m_variable;
		param->circleOut->m_shapeId = largeTriMeshCache.m_meshShapeIndex;
		param->circleOut->m_objectId = largeTriMeshCache.m_meshRbIndex;
	}
	return true;
}

static PfxBool findClosestHitLargeTriMeshCallbackForArray(const PfxCacheLargeTriMesh &largeTriMeshCache, const PfxUInt32 islandIds[], void *userData)
{
	PfxTraverseCircleCallbackParam *param = (PfxTraverseCircleCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->circleIn->m_contactFilterSelf, param->circleIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->circleIn->m_collisionIgnoreGroup[0], param->circleIn->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->circleIn->m_contactFilterSelf, param->circleIn->m_contactFilterTarget))
		return true;

	// CircleCast from local to world
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition circleLargeStartPosition = param->circleIn->m_startPosition;
	circleLargeStartPosition.changeSegment(state.getSegment());

	PfxCircleInputInternal circleIn;
	circleIn.m_segment = circleLargeStartPosition.segment;
	circleIn.m_startPosition = circleLargeStartPosition.offset;
	circleIn.m_direction = param->circleIn->m_direction;
	circleIn.m_orientation = param->circleIn->m_orientation;
	circleIn.m_contactFilterSelf = param->circleIn->m_contactFilterSelf;
	circleIn.m_contactFilterTarget = param->circleIn->m_contactFilterTarget;
	circleIn.m_radius = param->circleIn->m_radius;

	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
	PfxFloat radiusLocal = param->circleIn->m_radius / minElem(absPerElem(scale));

	PfxCircleOutputInternal circleOut;
	circleOut.m_variable = param->circleOut->m_variable;

	if (pfxIntersectMovingCircleIslandsOfLargeTriMeshArray(circleIn, circleOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, flipTriangle, radiusLocal, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
		param->circleOut->m_contactPoint.segment = circleLargeStartPosition.segment;
		param->circleOut->m_contactPoint.offset = circleOut.m_contactPoint;
		param->circleOut->m_contactFlag = circleOut.m_contactFlag;
		param->circleOut->m_contactNormal = circleOut.m_contactNormal;
		param->circleOut->m_subData = circleOut.m_subData;
		param->circleOut->m_variable = circleOut.m_variable;
		param->circleOut->m_shapeId = largeTriMeshCache.m_meshShapeIndex;
		param->circleOut->m_objectId = largeTriMeshCache.m_meshRbIndex;
	}
	return true;
}

static PfxBool findAllHitsLargeTriMeshCallbackForBvh(const PfxCacheLargeTriMesh &largeTriMeshCache, const PfxUInt32 islandIds[], void *userData)
{
	PfxTraverseCircleCallbackParam *param = (PfxTraverseCircleCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->circleIn->m_contactFilterSelf, param->circleIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->circleIn->m_collisionIgnoreGroup[0], param->circleIn->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->circleIn->m_contactFilterSelf, param->circleIn->m_contactFilterTarget))
		return true;

	// CircleCast from local to world
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition circleLargeStartPosition = param->circleIn->m_startPosition;
	circleLargeStartPosition.changeSegment(state.getSegment());

	PfxCircleInputInternal circleIn;
	circleIn.m_segment = circleLargeStartPosition.segment;
	circleIn.m_startPosition = circleLargeStartPosition.offset;
	circleIn.m_direction = param->circleIn->m_direction;
	circleIn.m_orientation = param->circleIn->m_orientation;
	circleIn.m_radius = param->circleIn->m_radius;
	circleIn.m_contactFilterSelf = param->circleIn->m_contactFilterSelf;
	circleIn.m_contactFilterTarget = param->circleIn->m_contactFilterTarget;

	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
	PfxFloat radiusLocal = param->circleIn->m_radius / minElem(absPerElem(scale));

	PfxCircleOutputInternal circleOut;
	circleOut.m_variable = 1.0f;

	if (pfxIntersectMovingCircleIslandsOfLargeTriMeshBvh(circleIn, circleOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, flipTriangle, radiusLocal, param->discardTriangleCallback, param->userDataForDiscardingTriangle))
	{
		PfxCircleOutput out;
		out.m_contactPoint.segment = circleIn.m_segment;
		out.m_contactPoint.offset = circleOut.m_contactPoint;
		out.m_contactFlag = circleOut.m_contactFlag;
		out.m_contactNormal = circleOut.m_contactNormal;
		out.m_subData = circleOut.m_subData;
		out.m_variable = param->t0 + circleOut.m_variable * (param->t1 - param->t0);
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
	PfxTraverseCircleCallbackParam *param = (PfxTraverseCircleCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->circleIn->m_contactFilterSelf, param->circleIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->circleIn->m_collisionIgnoreGroup[0], param->circleIn->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->circleIn->m_contactFilterSelf, param->circleIn->m_contactFilterTarget))
		return true;

	// CircleCast from local to world
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition circleLargeStartPosition = param->circleIn->m_startPosition;
	circleLargeStartPosition.changeSegment(state.getSegment());

	PfxCircleInputInternal circleIn;
	circleIn.m_segment = circleLargeStartPosition.segment;
	circleIn.m_startPosition = circleLargeStartPosition.offset;
	circleIn.m_direction = param->circleIn->m_direction;
	circleIn.m_orientation = param->circleIn->m_orientation;
	circleIn.m_radius = param->circleIn->m_radius;
	circleIn.m_contactFilterSelf = param->circleIn->m_contactFilterSelf;
	circleIn.m_contactFilterTarget = param->circleIn->m_contactFilterTarget;

	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
	PfxFloat radiusLocal = param->circleIn->m_radius / minElem(absPerElem(scale));

	PfxCircleOutputInternal circleOut;
	circleOut.m_variable = 1.0f;

	if (pfxIntersectMovingCircleIslandsOfLargeTriMeshArray(circleIn, circleOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, flipTriangle, radiusLocal, param->discardTriangleCallback, param->userDataForDiscardingTriangle))
	{
		PfxCircleOutput out;
		out.m_contactPoint.segment = circleIn.m_segment;
		out.m_contactPoint.offset = circleOut.m_contactPoint;
		out.m_contactFlag = circleOut.m_contactFlag;
		out.m_contactNormal = circleOut.m_contactNormal;
		out.m_subData = circleOut.m_subData;
		out.m_variable = param->t0 + circleOut.m_variable * (param->t1 - param->t0);
		out.m_shapeId = largeTriMeshCache.m_meshShapeIndex;
		out.m_objectId = largeTriMeshCache.m_meshRbIndex;
		if (!param->callback(out, param->userData)) {
			return false;
		}
	}
	return true;
}

// It returns no intersection if the start position is in a shape.
PfxInt32 pfxCastSingleCircle(const PfxCircleInput &circleIn,PfxCircleOutput &circleOut,const PfxCircleCastParam &param)
{
	if(!param.states || !param.collidables) return SCE_PFX_ERR_INVALID_VALUE;
	if (param.containerType == PfxCircleCastParam::PROXY_CONTAINER && param.proxyContainer == NULL) {
		return SCE_PFX_ERR_INVALID_TYPE;
	}
	if (param.containerType == PfxCircleCastParam::CACHE_CONTAINER && param.cacheContainer == NULL)	return SCE_PFX_ERR_INVALID_TYPE;

	if(lengthSqr(circleIn.m_direction) < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) return SCE_PFX_ERR_INVALID_VALUE;
	
	circleOut.m_variable = 1.0f;
	circleOut.m_contactFlag = false;
	
	PfxLargePosition rangeMin = param.rangeMin - PfxVector3(circleIn.m_radius);
	PfxLargePosition rangeMax = param.rangeMax + PfxVector3(circleIn.m_radius);
	
	if (pfxClipRayByDotProduct(circleIn.m_startPosition, circleIn.m_direction, rangeMin, rangeMax)) {

		PfxFloat t0 = 0.0f;
		PfxFloat t1 = 1.0f;
		pfxClipRay(circleIn.m_startPosition, circleIn.m_direction, rangeMin, rangeMax, t0, t1);

		PfxCircleInput circleIn_ = circleIn;
		circleIn_.m_startPosition = circleIn.m_startPosition + t0 * circleIn.m_direction;
		circleIn_.m_direction = (t1-t0) * circleIn.m_direction;
		PfxCircleOutput circleOut_ = circleOut;
		PfxTraverseCircleCallbackParam param_;
		param_.circleIn = &circleIn_;
		param_.circleOut = &circleOut_;
		param_.t0 = t0;
		param_.t1 = t1;
		param_.states = param.states;
		param_.collidables = param.collidables;
		param_.discardTriangleCallback = param.discardTriangleCallback;
		param_.userDataForDiscardingTriangle = param.userDataForDiscardingTriangle;
		
		if (param.containerType == PfxCircleCastParam::PROXY_CONTAINER) {
			PfxRayInput ray; // Ray for bvh traversal.
			ray.m_contactFilterSelf = circleIn_.m_contactFilterSelf;
			ray.m_contactFilterTarget = circleIn_.m_contactFilterTarget;
			ray.m_direction = circleIn_.m_direction;
			ray.m_startPosition = circleIn_.m_startPosition;

			PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)param.proxyContainer;
			bvh->traverseRayClosest(findClosestHitCallback, ray, circleIn.m_radius, &param_);
		}

		else if (param.containerType == PfxCircleCastParam::CACHE_CONTAINER) {
			PfxLargePosition circleEndPosition = circleIn_.m_startPosition + circleIn_.m_direction;
			PfxTraverseRigidBodyCacheContainerCallbackParam cacheParam;
			cacheParam.states = param.states;
			cacheParam.collidables = param.collidables;
			cacheParam.rangeOfInterest.vmin = minPerElem(circleIn.m_startPosition, circleEndPosition) - PfxVector3(circleIn.m_radius);
			cacheParam.rangeOfInterest.vmax = maxPerElem(circleIn.m_startPosition, circleEndPosition) + PfxVector3(circleIn.m_radius);

			const PfxRigidBodyCacheManager *cacheManager = ((const PfxRigidBodyCacheManager*)param.cacheContainer);
			cacheManager->traverseRayClosest(findClosestHitCallbackForCache, findClosestHitLargeTriMeshCallbackForBvh, findClosestHitLargeTriMeshCallbackForArray, cacheParam, &param_);
		}

		if (circleOut_.m_contactFlag && circleOut_.m_variable < circleOut.m_variable)
			circleOut = circleOut_;

		circleOut.m_variable = t0 + circleOut.m_variable * (t1 - t0);
	}

	return SCE_PFX_OK;
}

PfxInt32 pfxCastSingleCircle(const PfxCircleInput &circleIn,pfxCircleHitCallback callback,void *userData,const PfxCircleCastParam &param)
{
	if (!callback)	return SCE_PFX_OK;
	if(!param.states || !param.collidables) return SCE_PFX_ERR_INVALID_VALUE;
	if (param.containerType == PfxCircleCastParam::PROXY_CONTAINER && param.proxyContainer == NULL) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	if (param.containerType == PfxCircleCastParam::CACHE_CONTAINER && param.cacheContainer == NULL)	return SCE_PFX_ERR_INVALID_VALUE;

	if(lengthSqr(circleIn.m_direction) < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) return SCE_PFX_ERR_INVALID_VALUE;
		
	PfxLargePosition rangeMin = param.rangeMin - PfxVector3(circleIn.m_radius);
	PfxLargePosition rangeMax = param.rangeMax + PfxVector3(circleIn.m_radius);
	
	if (pfxClipRayByDotProduct(circleIn.m_startPosition, circleIn.m_direction, rangeMin, rangeMax)) {
		PfxFloat t0 = 0.0f;
		PfxFloat t1 = 1.0f;
		pfxClipRay(circleIn.m_startPosition, circleIn.m_direction, rangeMin, rangeMax, t0, t1);

		PfxCircleInput circleIn_ = circleIn;
		circleIn_.m_startPosition = circleIn.m_startPosition + t0 * circleIn.m_direction;
		circleIn_.m_direction = (t1-t0) * circleIn.m_direction;
		PfxTraverseCircleCallbackParam param_;
		param_.circleIn = &circleIn_;
		param_.t0 = t0;
		param_.t1 = t1;
		param_.states = param.states;
		param_.collidables = param.collidables;
		param_.callback = callback;
		param_.userData = userData;
		param_.discardTriangleCallback = param.discardTriangleCallback;
		param_.userDataForDiscardingTriangle = param.userDataForDiscardingTriangle;
		
		if (param.containerType == PfxCircleCastParam::PROXY_CONTAINER) {
			PfxRayInput ray; // Ray for bvh traversal.
			ray.m_contactFilterSelf = circleIn_.m_contactFilterSelf;
			ray.m_contactFilterTarget = circleIn_.m_contactFilterTarget;
			ray.m_direction = circleIn_.m_direction;
			ray.m_startPosition = circleIn_.m_startPosition;

			PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)param.proxyContainer;
			bvh->traverseRayOverlap(findAllHitsCallback, ray, circleIn.m_radius, &param_);
		}

		else if (param.containerType == PfxCircleCastParam::CACHE_CONTAINER) {
			PfxLargePosition circleEndPosition = circleIn_.m_startPosition + circleIn_.m_direction;
			PfxTraverseRigidBodyCacheContainerCallbackParam cacheParam;
			cacheParam.states = param.states;
			cacheParam.collidables = param.collidables;
			cacheParam.rangeOfInterest.vmin = minPerElem(circleIn.m_startPosition, circleEndPosition) - PfxVector3(circleIn.m_radius);
			cacheParam.rangeOfInterest.vmax = maxPerElem(circleIn.m_startPosition, circleEndPosition) + PfxVector3(circleIn.m_radius);

			const PfxRigidBodyCacheManager *cacheManager = ((const PfxRigidBodyCacheManager*)param.cacheContainer);
			cacheManager->traverseRayOverlap(findAllHitsCallbackForCache, findAllHitsLargeTriMeshCallbackForBvh, findAllHitsLargeTriMeshCallbackForArray, cacheParam, &param_);
		}
	}

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
