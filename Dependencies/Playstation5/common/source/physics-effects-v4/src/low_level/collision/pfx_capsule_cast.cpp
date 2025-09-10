/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/collision/pfx_intersect_moving_capsule_func.h"
#include "../../../include/physics_effects/low_level/collision/pfx_capsule_cast.h"
#include "../../../include/physics_effects/util/pfx_static_array.h"
#include "../../base_level/collision/pfx_intersect_common.h"
#include "../../base_level/collision/pfx_large_tri_mesh_impl.h"
#include "../../base_level/collision/pfx_intersect_moving_capsule_all_facets.h"
#include "../../base_level/collision/pfx_intersect_moving_capsule_large_tri_mesh.h"
#include "../../base_level/broadphase/pfx_check_collidable.h"
#include "../broadphase/pfx_progressive_bvh.h"
#include "../collision/pfx_rigid_body_cache_manager.h"

namespace sce {
namespace pfxv4 {

struct PfxTraverseCapsuleCallbackParam {
	PfxCapsuleOutput *capsuleOut;
	pfxCapsuleHitCallback callback;
	void *userData;
	const PfxCapsuleInput *capsuleIn;
	const PfxRigidState *states;
	const PfxCollidable *collidables;
	PfxFloat t0,t1;
	pfxRayHitDiscardTriangleCallback discardTriangleCallback;
	void *userDataForDiscardingTriangle;
};

static PfxBool findClosestHitCallbackForCache(PfxUInt32 rigidBodyId,const PfxBv &bv,PfxBool areMeshIslandIndicesSaved,PfxRayInput &clipRay,PfxFloat &tmin,void *userData)
{
	PfxTraverseCapsuleCallbackParam *param = (PfxTraverseCapsuleCallbackParam*)userData;
	
	const PfxRigidState &state = param->states[rigidBodyId];
	const PfxCollidable &coll = param->collidables[rigidBodyId];
	
	if( pfxCheckContactFilter(state.getContactFilterSelf(),state.getContactFilterTarget(),
			param->capsuleIn->m_contactFilterSelf,param->capsuleIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0),state.getCollisionIgnoreGroup(1),
			param->capsuleIn->m_collisionIgnoreGroup[0],param->capsuleIn->m_collisionIgnoreGroup[1])) {
		
		PfxTransform3 transform(state.getOrientation(), state.getPosition());
		
		PfxCapsuleInputInternal icapsule;
		
		PfxLargePosition lposA = state.getLargePosition();
		PfxLargePosition tmp = param->capsuleIn->m_startPosition;
		tmp.changeSegment(lposA.segment);
		icapsule.m_segment = lposA.segment;
		icapsule.m_startPosition = tmp.offset;
		icapsule.m_direction = param->capsuleIn->m_direction;
		icapsule.m_radius = param->capsuleIn->m_radius;
		icapsule.m_halfLength = param->capsuleIn->m_halfLength;
		icapsule.m_orientation = param->capsuleIn->m_orientation;
		icapsule.m_contactFilterSelf = param->capsuleIn->m_contactFilterSelf;
		icapsule.m_contactFilterTarget = param->capsuleIn->m_contactFilterTarget;
		
		for(PfxUInt32 j=0;j<coll.getNumShapes();j++) {
			const PfxShape &shape = coll.getShape(j);
			
			if (areMeshIslandIndicesSaved && shape.getType() == kPfxShapeLargeTriMesh)
				continue;

			if(!pfxCheckContactFilter(shape.getContactFilterSelf(),shape.getContactFilterTarget(),param->capsuleIn->m_contactFilterSelf,param->capsuleIn->m_contactFilterTarget)) {
				continue;
			}
			
			PfxTransform3 offsetTr = appendScale(shape.getOffsetTransform(),shape.getScaleXyz());
			PfxTransform3 worldTransform = transform * offsetTr;
			
			PfxCapsuleOutputInternal iout;
			iout.m_variable = param->capsuleOut->m_variable;
			
			if(pfxGetIntersectMovingCapsuleFunc(shape.getType())(icapsule,iout,shape,worldTransform, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
				if(iout.m_subData.getType() == PfxSubData::SHAPE_INFO) {
					iout.m_subData.setShapeId(j);
					iout.m_subData.setUserData(shape.getUserData());
				}
				
				param->capsuleOut->m_contactPoint.segment = icapsule.m_segment;
				param->capsuleOut->m_contactPoint.offset = iout.m_contactPoint;
				param->capsuleOut->m_contactFlag = iout.m_contactFlag;
				param->capsuleOut->m_contactNormal = iout.m_contactNormal;
				param->capsuleOut->m_subData = iout.m_subData;
				param->capsuleOut->m_variable = iout.m_variable;
				param->capsuleOut->m_shapeId = j;
				param->capsuleOut->m_objectId = rigidBodyId;
			}
		}
	}

	if (tmin > 0.0f && param->capsuleOut->m_contactFlag && param->capsuleOut->m_variable < tmin) {
		clipRay.m_direction = param->capsuleOut->m_variable * param->capsuleIn->m_direction;
		tmin = param->capsuleOut->m_variable;
	}

	return true;
}

static PfxBool findClosestHitCallback(PfxUInt32 rigidBodyId, const PfxBv &bv, PfxRayInput &clipRay, PfxFloat &tmin, void *userData) {
	return findClosestHitCallbackForCache(rigidBodyId, bv, false, clipRay, tmin, userData);
}

static PfxBool findAllHitsCallbackForCache(PfxUInt32 rigidBodyId, const PfxBv &bv, PfxBool areMeshIslandIndicesSaved, void *userData)
{
	PfxTraverseCapsuleCallbackParam *param = (PfxTraverseCapsuleCallbackParam*)userData;

	const PfxRigidState &state = param->states[rigidBodyId];
	const PfxCollidable &coll = param->collidables[rigidBodyId];

	if (pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->capsuleIn->m_contactFilterSelf, param->capsuleIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->capsuleIn->m_collisionIgnoreGroup[0], param->capsuleIn->m_collisionIgnoreGroup[1])) {

		PfxTransform3 transform(state.getOrientation(), state.getPosition());

		PfxCapsuleInputInternal icapsule;

		PfxLargePosition lposA = state.getLargePosition();
		PfxLargePosition tmp = param->capsuleIn->m_startPosition;
		tmp.changeSegment(lposA.segment);
		icapsule.m_segment = lposA.segment;
		icapsule.m_startPosition = tmp.offset;
		icapsule.m_direction = param->capsuleIn->m_direction;
		icapsule.m_radius = param->capsuleIn->m_radius;
		icapsule.m_halfLength = param->capsuleIn->m_halfLength;
		icapsule.m_orientation = param->capsuleIn->m_orientation;
		icapsule.m_contactFilterSelf = param->capsuleIn->m_contactFilterSelf;
		icapsule.m_contactFilterTarget = param->capsuleIn->m_contactFilterTarget;

		for (PfxUInt32 j = 0; j<coll.getNumShapes(); j++) {
			const PfxShape &shape = coll.getShape(j);

			if (areMeshIslandIndicesSaved && shape.getType() == kPfxShapeLargeTriMesh)
				continue;

			if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->capsuleIn->m_contactFilterSelf, param->capsuleIn->m_contactFilterTarget)) {
				continue;
			}

			PfxTransform3 offsetTr = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
			PfxTransform3 worldTransform = transform * offsetTr;

			PfxCapsuleOutputInternal iout;
			iout.m_variable = 1.0f;

			if (pfxGetIntersectMovingCapsuleFunc(shape.getType())(icapsule, iout, shape, worldTransform, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
				if (iout.m_subData.getType() == PfxSubData::SHAPE_INFO) {
					iout.m_subData.setShapeId(j);
					iout.m_subData.setUserData(shape.getUserData());
				}

				PfxCapsuleOutput out;
				out.m_contactPoint.segment = icapsule.m_segment;
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
	PfxTraverseCapsuleCallbackParam *param = (PfxTraverseCapsuleCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->capsuleIn->m_contactFilterSelf, param->capsuleIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->capsuleIn->m_collisionIgnoreGroup[0], param->capsuleIn->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->capsuleIn->m_contactFilterSelf, param->capsuleIn->m_contactFilterTarget))
		return true;

	// CapsuleCast from world to local
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localtoWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition capsuleLargeStartPosition = param->capsuleIn->m_startPosition;
	capsuleLargeStartPosition.changeSegment(state.getSegment());

	PfxCapsuleInputInternal capsuleIn;
	capsuleIn.m_segment = capsuleLargeStartPosition.segment;
	capsuleIn.m_startPosition = capsuleLargeStartPosition.offset;
	capsuleIn.m_direction = param->capsuleIn->m_direction;
	capsuleIn.m_orientation = param->capsuleIn->m_orientation;
	capsuleIn.m_radius = param->capsuleIn->m_radius;
	capsuleIn.m_halfLength = param->capsuleIn->m_halfLength;
	capsuleIn.m_contactFilterSelf = param->capsuleIn->m_contactFilterSelf;
	capsuleIn.m_contactFilterTarget = param->capsuleIn->m_contactFilterTarget;

	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
	PfxFloat radiusLocal = calcRadiusOfSweptCapsule(capsuleIn) / minElem(absPerElem(scale));

	PfxCapsuleOutputInternal capsuleOut;
	capsuleOut.m_variable = param->capsuleOut->m_variable;

	if (pfxIntersectMovingCapsuleIslandsOfLargeTriMeshBvh(capsuleIn, capsuleOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localtoWorldTransform, flipTriangle, radiusLocal, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
		param->capsuleOut->m_contactPoint.segment = capsuleLargeStartPosition.segment;
		param->capsuleOut->m_contactPoint.offset = capsuleOut.m_contactPoint;
		param->capsuleOut->m_contactFlag = capsuleOut.m_contactFlag;
		param->capsuleOut->m_contactNormal = capsuleOut.m_contactNormal;
		param->capsuleOut->m_subData = capsuleOut.m_subData;
		param->capsuleOut->m_variable = capsuleOut.m_variable;
		param->capsuleOut->m_shapeId = largeTriMeshCache.m_meshShapeIndex;
		param->capsuleOut->m_objectId = largeTriMeshCache.m_meshRbIndex;
	}
	return true;
}

static PfxBool findClosestHitLargeTriMeshCallbackForArray(const PfxCacheLargeTriMesh &largeTriMeshCache, const PfxUInt32 islandIds[], void *userData)
{
	PfxTraverseCapsuleCallbackParam *param = (PfxTraverseCapsuleCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->capsuleIn->m_contactFilterSelf, param->capsuleIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->capsuleIn->m_collisionIgnoreGroup[0], param->capsuleIn->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->capsuleIn->m_contactFilterSelf, param->capsuleIn->m_contactFilterTarget))
		return true;

	// CapsuleCast from world to local
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition capsuleLargeStartPosition = param->capsuleIn->m_startPosition;
	capsuleLargeStartPosition.changeSegment(state.getSegment());

	PfxCapsuleInputInternal capsuleIn;
	capsuleIn.m_segment = capsuleLargeStartPosition.segment;
	capsuleIn.m_startPosition = capsuleLargeStartPosition.offset;
	capsuleIn.m_direction = param->capsuleIn->m_direction;
	capsuleIn.m_radius = param->capsuleIn->m_radius;
	capsuleIn.m_halfLength = param->capsuleIn->m_halfLength;
	capsuleIn.m_contactFilterSelf = param->capsuleIn->m_contactFilterSelf;
	capsuleIn.m_contactFilterTarget = param->capsuleIn->m_contactFilterTarget;

	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.f;
	PfxFloat radiusLocal = calcRadiusOfSweptCapsule(capsuleIn) / minElem(absPerElem(scale));

	PfxCapsuleOutputInternal capsuleOut;
	capsuleOut.m_variable = param->capsuleOut->m_variable;

	if (pfxIntersectMovingCapsuleIslandsOfLargeTriMeshArray(capsuleIn, capsuleOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, flipTriangle, radiusLocal, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
		param->capsuleOut->m_contactPoint.segment = capsuleLargeStartPosition.segment;
		param->capsuleOut->m_contactPoint.offset = capsuleOut.m_contactPoint;
		param->capsuleOut->m_contactFlag = capsuleOut.m_contactFlag;
		param->capsuleOut->m_contactNormal = capsuleOut.m_contactNormal;
		param->capsuleOut->m_subData = capsuleOut.m_subData;
		param->capsuleOut->m_variable = capsuleOut.m_variable;
		param->capsuleOut->m_shapeId = largeTriMeshCache.m_meshShapeIndex;
		param->capsuleOut->m_objectId = largeTriMeshCache.m_meshRbIndex;
	}
	return true;
}

static PfxBool findAllHitsLargeTriMeshCallbackForBvh(const PfxCacheLargeTriMesh &largeTriMeshCache, const PfxUInt32 islandIds[], void *userData)
{
	PfxTraverseCapsuleCallbackParam *param = (PfxTraverseCapsuleCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->capsuleIn->m_contactFilterSelf, param->capsuleIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->capsuleIn->m_collisionIgnoreGroup[0], param->capsuleIn->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->capsuleIn->m_contactFilterSelf, param->capsuleIn->m_contactFilterTarget))
		return true;

	// CapsuleCast from local to world
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition capsuleLargeStartPosition = param->capsuleIn->m_startPosition;
	capsuleLargeStartPosition.changeSegment(state.getSegment());

	PfxCapsuleInputInternal capsuleIn;
	capsuleIn.m_segment = capsuleLargeStartPosition.segment;
	capsuleIn.m_startPosition = capsuleLargeStartPosition.offset;
	capsuleIn.m_direction = param->capsuleIn->m_direction;
	capsuleIn.m_radius = param->capsuleIn->m_radius;
	capsuleIn.m_halfLength = param->capsuleIn->m_halfLength;
	capsuleIn.m_contactFilterSelf = param->capsuleIn->m_contactFilterSelf;
	capsuleIn.m_contactFilterTarget = param->capsuleIn->m_contactFilterTarget;

	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.f;
	PfxFloat radiusLocal = calcRadiusOfSweptCapsule(capsuleIn) / minElem(absPerElem(scale));

	PfxCapsuleOutputInternal capsuleOut;
	capsuleOut.m_variable = 1.f;

	if (pfxIntersectMovingCapsuleIslandsOfLargeTriMeshBvh(capsuleIn, capsuleOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, flipTriangle, radiusLocal, param->discardTriangleCallback, param->userDataForDiscardingTriangle))
	{
		PfxCapsuleOutput out;
		out.m_contactPoint.segment = capsuleIn.m_segment;
		out.m_contactPoint.offset = capsuleOut.m_contactPoint;
		out.m_contactFlag = capsuleOut.m_contactFlag;
		out.m_contactNormal = capsuleOut.m_contactNormal;
		out.m_subData = capsuleOut.m_subData;
		out.m_variable = param->t0 + capsuleOut.m_variable * (param->t1 - param->t0);
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
	PfxTraverseCapsuleCallbackParam *param = (PfxTraverseCapsuleCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->capsuleIn->m_contactFilterSelf, param->capsuleIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->capsuleIn->m_collisionIgnoreGroup[0], param->capsuleIn->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->capsuleIn->m_contactFilterSelf, param->capsuleIn->m_contactFilterTarget))
		return true;

	// CapsuleCast from local to world
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition capsuleLargeStartPosition = param->capsuleIn->m_startPosition;
	capsuleLargeStartPosition.changeSegment(state.getSegment());

	PfxCapsuleInputInternal capsuleIn;
	capsuleIn.m_segment = capsuleLargeStartPosition.segment;
	capsuleIn.m_startPosition = capsuleLargeStartPosition.offset;
	capsuleIn.m_direction = param->capsuleIn->m_direction;
	capsuleIn.m_radius = param->capsuleIn->m_radius;
	capsuleIn.m_halfLength = param->capsuleIn->m_halfLength;
	capsuleIn.m_contactFilterSelf = param->capsuleIn->m_contactFilterSelf;
	capsuleIn.m_contactFilterTarget = param->capsuleIn->m_contactFilterTarget;

	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.f;
	PfxFloat radiusLocal = calcRadiusOfSweptCapsule(capsuleIn) / minElem(absPerElem(scale));

	PfxCapsuleOutputInternal capsuleOut;
	capsuleOut.m_variable = 1.f;

	if (pfxIntersectMovingCapsuleIslandsOfLargeTriMeshArray(capsuleIn, capsuleOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, flipTriangle, radiusLocal, param->discardTriangleCallback, param->userDataForDiscardingTriangle))
	{
		PfxCapsuleOutput out;
		out.m_contactPoint.segment = capsuleIn.m_segment;
		out.m_contactPoint.offset = capsuleOut.m_contactPoint;
		out.m_contactFlag = capsuleOut.m_contactFlag;
		out.m_contactNormal = capsuleOut.m_contactNormal;
		out.m_subData = capsuleOut.m_subData;
		out.m_variable = param->t0 + capsuleOut.m_variable * (param->t1 - param->t0);
		out.m_shapeId = largeTriMeshCache.m_meshShapeIndex;
		out.m_objectId = largeTriMeshCache.m_meshRbIndex;
		if (!param->callback(out, param->userData)) {
			return false;
		}
	}
	return true;
}

PfxInt32 pfxCastSingleCapsule(const PfxCapsuleInput &capsuleIn,PfxCapsuleOutput &capsuleOut,const PfxCapsuleCastParam &param)
{
	if(!param.states || !param.collidables) return SCE_PFX_ERR_INVALID_VALUE;
	if (param.containerType == PfxCapsuleCastParam::PROXY_CONTAINER && param.proxyContainer == NULL) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	if (param.containerType == PfxCapsuleCastParam::CACHE_CONTAINER && param.cacheContainer == NULL)	return SCE_PFX_ERR_INVALID_VALUE;

	if (lengthSqr(capsuleIn.m_direction) < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) return SCE_PFX_ERR_INVALID_VALUE;

	capsuleOut.m_variable = 1.0f;
	capsuleOut.m_contactFlag = false;

	PfxFloat rayRadius = calcRadiusOfSweptCapsule(capsuleIn);
	
	PfxLargePosition rangeMin = param.rangeMin - PfxVector3(rayRadius);
	PfxLargePosition rangeMax = param.rangeMax + PfxVector3(rayRadius);
	
	// レイを領域でクリップ
	if (pfxClipRayByDotProduct(capsuleIn.m_startPosition, capsuleIn.m_direction, rangeMin, rangeMax)) {
		PfxFloat t0 = 0.0f;
		PfxFloat t1 = 1.0f;
		pfxClipRay(capsuleIn.m_startPosition, capsuleIn.m_direction, rangeMin, rangeMax, t0, t1);

		PfxCapsuleInput capsuleIn_ = capsuleIn;
		capsuleIn_.m_startPosition = capsuleIn.m_startPosition + t0 * capsuleIn.m_direction;
		capsuleIn_.m_direction = (t1-t0) * capsuleIn.m_direction;
		PfxCapsuleOutput capsuleOut_ = capsuleOut;
		PfxTraverseCapsuleCallbackParam param_;
		param_.capsuleIn = &capsuleIn_;
		param_.capsuleOut = &capsuleOut_;
		param_.t0 = t0;
		param_.t1 = t1;
		param_.states = param.states;
		param_.collidables = param.collidables;
		param_.discardTriangleCallback = param.discardTriangleCallback;
		param_.userDataForDiscardingTriangle = param.userDataForDiscardingTriangle;
		
		if (param.containerType == PfxCapsuleCastParam::PROXY_CONTAINER) {
			PfxRayInput ray; // Ray for bvh traversal.
			ray.m_contactFilterSelf = capsuleIn_.m_contactFilterSelf;
			ray.m_contactFilterTarget = capsuleIn_.m_contactFilterTarget;
			ray.m_direction = capsuleIn_.m_direction;
			ray.m_startPosition = capsuleIn_.m_startPosition;

			PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)param.proxyContainer;
			bvh->traverseRayClosest(findClosestHitCallback, ray, rayRadius, &param_);
		}

		else if (param.containerType == PfxCapsuleCastParam::CACHE_CONTAINER) {
			PfxLargePosition capsuleEndPosition = capsuleIn_.m_startPosition + capsuleIn_.m_direction;
			PfxTraverseRigidBodyCacheContainerCallbackParam cacheParam;
			cacheParam.states = param.states;
			cacheParam.collidables = param.collidables;
			cacheParam.rangeOfInterest.vmin = minPerElem(capsuleIn.m_startPosition, capsuleEndPosition) - PfxVector3(rayRadius);
			cacheParam.rangeOfInterest.vmax = maxPerElem(capsuleIn.m_startPosition, capsuleEndPosition) + PfxVector3(rayRadius);

			const PfxRigidBodyCacheManager *cacheManager = ((const PfxRigidBodyCacheManager*)param.cacheContainer);
			cacheManager->traverseRayClosest(findClosestHitCallbackForCache, findClosestHitLargeTriMeshCallbackForBvh, findClosestHitLargeTriMeshCallbackForArray, cacheParam, &param_);
		}
		
		if (capsuleOut_.m_contactFlag && capsuleOut_.m_variable < capsuleOut.m_variable)
			capsuleOut = capsuleOut_;

		capsuleOut.m_variable = t0 + capsuleOut.m_variable * (t1 - t0);
	}

	return SCE_PFX_OK;
}

PfxInt32 pfxCastSingleCapsule(const PfxCapsuleInput &capsuleIn,pfxCapsuleHitCallback callback,void *userData,const PfxCapsuleCastParam &param)
{
	if (!callback)	return SCE_PFX_OK;
	if(!param.states || !param.collidables) return SCE_PFX_ERR_INVALID_VALUE;
	if (param.containerType == PfxCapsuleCastParam::PROXY_CONTAINER && param.proxyContainer == NULL) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	if (param.containerType == PfxCapsuleCastParam::CACHE_CONTAINER && param.cacheContainer == NULL)	return SCE_PFX_ERR_INVALID_VALUE;
	
	if(lengthSqr(capsuleIn.m_direction) < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) return SCE_PFX_ERR_INVALID_VALUE;
	
	PfxFloat rayRadius = calcRadiusOfSweptCapsule(capsuleIn);
	
	PfxLargePosition rangeMin = param.rangeMin - PfxVector3(rayRadius);
	PfxLargePosition rangeMax = param.rangeMax + PfxVector3(rayRadius);
	
	// レイを領域でクリップ
	if (pfxClipRayByDotProduct(capsuleIn.m_startPosition, capsuleIn.m_direction, rangeMin, rangeMax)) {

		PfxFloat t0 = 0.0f;
		PfxFloat t1 = 1.0f;
		pfxClipRay(capsuleIn.m_startPosition, capsuleIn.m_direction, rangeMin, rangeMax, t0, t1);

		PfxCapsuleInput capsuleIn_ = capsuleIn;
		capsuleIn_.m_startPosition = capsuleIn.m_startPosition + t0 * capsuleIn.m_direction;
		capsuleIn_.m_direction = (t1-t0) * capsuleIn.m_direction;
		PfxTraverseCapsuleCallbackParam param_;
		param_.capsuleIn = &capsuleIn_;
		param_.t0 = t0;
		param_.t1 = t1;
		param_.states = param.states;
		param_.collidables = param.collidables;
		param_.callback = callback;
		param_.userData = userData;
		param_.discardTriangleCallback = param.discardTriangleCallback;
		param_.userDataForDiscardingTriangle = param.userDataForDiscardingTriangle;
		
		if (param.containerType == PfxCapsuleCastParam::PROXY_CONTAINER) {
			PfxRayInput ray; // Ray for bvh traversal.
			ray.m_contactFilterSelf = capsuleIn_.m_contactFilterSelf;
			ray.m_contactFilterTarget = capsuleIn_.m_contactFilterTarget;
			ray.m_direction = capsuleIn_.m_direction;
			ray.m_startPosition = capsuleIn_.m_startPosition;

			PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)param.proxyContainer;
			bvh->traverseRayOverlap(findAllHitsCallback, ray, rayRadius, &param_);
		}

		else if (param.containerType == PfxCapsuleCastParam::CACHE_CONTAINER) {
			PfxLargePosition capsuleEndPosition = capsuleIn_.m_startPosition + capsuleIn_.m_direction;
			PfxTraverseRigidBodyCacheContainerCallbackParam cacheParam;
			cacheParam.states = param.states;
			cacheParam.collidables = param.collidables;
			cacheParam.rangeOfInterest.vmin = minPerElem(capsuleIn.m_startPosition, capsuleEndPosition) - PfxVector3(rayRadius);
			cacheParam.rangeOfInterest.vmax = maxPerElem(capsuleIn.m_startPosition, capsuleEndPosition) + PfxVector3(rayRadius);

			const PfxRigidBodyCacheManager *cacheManager = ((const PfxRigidBodyCacheManager*)param.cacheContainer);
			cacheManager->traverseRayOverlap(findAllHitsCallbackForCache, findAllHitsLargeTriMeshCallbackForBvh, findAllHitsLargeTriMeshCallbackForArray, cacheParam, &param_);
		}
	}

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
