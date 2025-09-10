/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/collision/pfx_intersect_moving_sphere_func.h"
#include "../../../include/physics_effects/low_level/collision/pfx_sphere_cast.h"
#include "../../../include/physics_effects/util/pfx_static_array.h"
#include "../../base_level/collision/pfx_intersect_common.h"
#include "../../base_level/collision/pfx_large_tri_mesh_impl.h"
#include "../../base_level/collision/pfx_intersect_moving_sphere_all_facets.h"
#include "../../base_level/collision/pfx_intersect_moving_sphere_large_tri_mesh.h"
#include "../../base_level/broadphase/pfx_check_collidable.h"
#include "../broadphase/pfx_progressive_bvh.h"
#include "../collision/pfx_rigid_body_cache_manager.h"

namespace sce {
namespace pfxv4 {

struct PfxTraverseSphereCallbackParam {
	PfxSphereOutput *sphereOut;
	pfxSphereHitCallback callback;
	void *userData;
	const PfxSphereInput *sphereIn;
	const PfxRigidState *states;
	const PfxCollidable *collidables;
	PfxFloat t0,t1;
	pfxRayHitDiscardTriangleCallback discardTriangleCallback;
	void *userDataForDiscardingTriangle;
};

static PfxBool findClosestHitCallbackForCache(PfxUInt32 rigidBodyId,const PfxBv &bv,PfxBool areMeshIslandIndicesSaved,PfxRayInput &clipRay,PfxFloat &tmin,void *userData)
{
	PfxTraverseSphereCallbackParam *param = (PfxTraverseSphereCallbackParam*)userData;
	
	const PfxRigidState &state = param->states[rigidBodyId];
	const PfxCollidable &coll = param->collidables[rigidBodyId];
	
	if( pfxCheckContactFilter(state.getContactFilterSelf(),state.getContactFilterTarget(),
			param->sphereIn->m_contactFilterSelf,param->sphereIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0),state.getCollisionIgnoreGroup(1),
			param->sphereIn->m_collisionIgnoreGroup[0],param->sphereIn->m_collisionIgnoreGroup[1])) {
		
		PfxTransform3 transform(state.getOrientation(), state.getPosition());
		
		PfxSphereInputInternal isphere;
		
		PfxLargePosition lposA = state.getLargePosition();
		PfxLargePosition tmp = param->sphereIn->m_startPosition;
		tmp.changeSegment(lposA.segment);
		isphere.m_segment = lposA.segment;
		isphere.m_startPosition = tmp.offset;
		isphere.m_direction = param->sphereIn->m_direction;
		isphere.m_radius = param->sphereIn->m_radius;
		isphere.m_contactFilterSelf = param->sphereIn->m_contactFilterSelf;
		isphere.m_contactFilterTarget = param->sphereIn->m_contactFilterTarget;
		
		for(PfxUInt32 j=0;j<coll.getNumShapes();j++) {
			const PfxShape &shape = coll.getShape(j);
			
			if (areMeshIslandIndicesSaved && shape.getType() == kPfxShapeLargeTriMesh)
				continue;

			if(!pfxCheckContactFilter(shape.getContactFilterSelf(),shape.getContactFilterTarget(),param->sphereIn->m_contactFilterSelf,param->sphereIn->m_contactFilterTarget)) {
				continue;
			}
			
			PfxTransform3 offsetTr = appendScale(shape.getOffsetTransform(),shape.getScaleXyz());
			PfxTransform3 worldTransform = transform * offsetTr;

			PfxSphereOutputInternal iout;
			iout.m_variable = param->sphereOut->m_variable;

			if(pfxGetIntersectMovingSphereFunc(shape.getType())(isphere,iout,shape,worldTransform, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
				if(iout.m_subData.getType() == PfxSubData::SHAPE_INFO) {
					iout.m_subData.setShapeId(j);
					iout.m_subData.setUserData(shape.getUserData());
				}
				
				param->sphereOut->m_contactPoint.segment = isphere.m_segment;
				param->sphereOut->m_contactPoint.offset = iout.m_contactPoint;
				param->sphereOut->m_contactFlag = iout.m_contactFlag;
				param->sphereOut->m_contactNormal = iout.m_contactNormal;
				param->sphereOut->m_subData = iout.m_subData;
				param->sphereOut->m_variable = iout.m_variable;
				param->sphereOut->m_shapeId = j;
				param->sphereOut->m_objectId = rigidBodyId;
			}
		}
	}
	
	if (tmin > 0.0f && param->sphereOut->m_contactFlag && param->sphereOut->m_variable < tmin) {
		clipRay.m_direction = param->sphereOut->m_variable * param->sphereIn->m_direction;
		tmin = param->sphereOut->m_variable;
	}

	return true;
}

static PfxBool findClosestHitCallback(PfxUInt32 rigidBodyId, const PfxBv &bv, PfxRayInput &clipRay, PfxFloat &tmin, void *userData) {
	return findClosestHitCallbackForCache(rigidBodyId, bv, false, clipRay, tmin, userData);
}

static PfxBool findAllHitsCallbackForCache(PfxUInt32 rigidBodyId, const PfxBv &bv, PfxBool areMeshIslandIndicesSaved, void *userData)
{
	PfxTraverseSphereCallbackParam *param = (PfxTraverseSphereCallbackParam*)userData;

	const PfxRigidState &state = param->states[rigidBodyId];
	const PfxCollidable &coll = param->collidables[rigidBodyId];

	if (pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->sphereIn->m_contactFilterSelf, param->sphereIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->sphereIn->m_collisionIgnoreGroup[0], param->sphereIn->m_collisionIgnoreGroup[1])) {

		PfxTransform3 transform(state.getOrientation(), state.getPosition());

		PfxSphereInputInternal isphere;

		PfxLargePosition lposA = state.getLargePosition();
		PfxLargePosition tmp = param->sphereIn->m_startPosition;
		tmp.changeSegment(lposA.segment);
		isphere.m_segment = lposA.segment;
		isphere.m_startPosition = tmp.offset;
		isphere.m_direction = param->sphereIn->m_direction;
		isphere.m_radius = param->sphereIn->m_radius;
		isphere.m_contactFilterSelf = param->sphereIn->m_contactFilterSelf;
		isphere.m_contactFilterTarget = param->sphereIn->m_contactFilterTarget;

		for (PfxUInt32 j = 0; j<coll.getNumShapes(); j++) {
			const PfxShape &shape = coll.getShape(j);

			if (areMeshIslandIndicesSaved && shape.getType() == kPfxShapeLargeTriMesh)
				continue;

			if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->sphereIn->m_contactFilterSelf, param->sphereIn->m_contactFilterTarget)) {
				continue;
			}

			PfxTransform3 offsetTr = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
			PfxTransform3 worldTransform = transform * offsetTr;

			PfxSphereOutputInternal iout;
			iout.m_variable = 1.0f;

			if (pfxGetIntersectMovingSphereFunc(shape.getType())(isphere, iout, shape, worldTransform, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
				if (iout.m_subData.getType() == PfxSubData::SHAPE_INFO) {
					iout.m_subData.setShapeId(j);
					iout.m_subData.setUserData(shape.getUserData());
				}

				PfxSphereOutput out;
				out.m_contactPoint.segment = isphere.m_segment;
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
	PfxTraverseSphereCallbackParam *param = (PfxTraverseSphereCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->sphereIn->m_contactFilterSelf, param->sphereIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->sphereIn->m_collisionIgnoreGroup[0], param->sphereIn->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->sphereIn->m_contactFilterSelf, param->sphereIn->m_contactFilterTarget))
		return true;

	// SphereCast from local to world	
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition sphereLargeStartPosition = param->sphereIn->m_startPosition;
	sphereLargeStartPosition.changeSegment(state.getSegment());

	PfxSphereInputInternal sphereIn;
	sphereIn.m_segment = sphereLargeStartPosition.segment;
	sphereIn.m_startPosition = sphereLargeStartPosition.offset;
	sphereIn.m_direction = param->sphereIn->m_direction;
	sphereIn.m_radius = param->sphereIn->m_radius;
	sphereIn.m_contactFilterSelf = param->sphereIn->m_contactFilterSelf;
	sphereIn.m_contactFilterTarget = param->sphereIn->m_contactFilterTarget;

	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
	PfxFloat sphereLocalRadius = param->sphereIn->m_radius / minElem(absPerElem(scale));

	PfxSphereOutputInternal sphereOut;
	sphereOut.m_variable = param->sphereOut->m_variable;

	if (pfxIntersectMovingSphereIslandsOfLargeTriMeshBvh(sphereIn, sphereOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, flipTriangle, sphereLocalRadius, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
		param->sphereOut->m_contactPoint.segment = sphereLargeStartPosition.segment;
		param->sphereOut->m_contactPoint.offset = sphereOut.m_contactPoint;
		param->sphereOut->m_contactFlag = sphereOut.m_contactFlag;
		param->sphereOut->m_contactNormal = sphereOut.m_contactNormal;
		param->sphereOut->m_subData = sphereOut.m_subData;
		param->sphereOut->m_variable = sphereOut.m_variable;
		param->sphereOut->m_shapeId = largeTriMeshCache.m_meshShapeIndex;
		param->sphereOut->m_objectId = largeTriMeshCache.m_meshRbIndex;
	}
	return true;
}

static PfxBool findClosestHitLargeTriMeshCallbackForArray(const PfxCacheLargeTriMesh &largeTriMeshCache, const PfxUInt32 islandIds[], void *userData)
{
	PfxTraverseSphereCallbackParam *param = (PfxTraverseSphereCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->sphereIn->m_contactFilterSelf, param->sphereIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->sphereIn->m_collisionIgnoreGroup[0], param->sphereIn->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->sphereIn->m_contactFilterSelf, param->sphereIn->m_contactFilterTarget))
		return true;

	// SphereCast from local to world
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition sphereLargeStartPosition = param->sphereIn->m_startPosition;
	sphereLargeStartPosition.changeSegment(state.getSegment());

	PfxSphereInputInternal sphereIn;
	sphereIn.m_segment = sphereLargeStartPosition.segment;
	sphereIn.m_startPosition = sphereLargeStartPosition.offset;
	sphereIn.m_direction = param->sphereIn->m_direction;
	sphereIn.m_radius = param->sphereIn->m_radius;
	sphereIn.m_contactFilterSelf = param->sphereIn->m_contactFilterSelf;
	sphereIn.m_contactFilterTarget = param->sphereIn->m_contactFilterTarget;

	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
	PfxFloat sphereLocalRadius = param->sphereIn->m_radius / minElem(absPerElem(scale));

	PfxSphereOutputInternal sphereOut;
	sphereOut.m_variable = param->sphereOut->m_variable;

	if (pfxIntersectMovingSphereIslandsOfLargeTriMeshArray(sphereIn, sphereOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, flipTriangle, sphereLocalRadius, param->discardTriangleCallback, param->userDataForDiscardingTriangle)) {
		param->sphereOut->m_contactPoint.segment = sphereLargeStartPosition.segment;
		param->sphereOut->m_contactPoint.offset = sphereOut.m_contactPoint;
		param->sphereOut->m_contactFlag = sphereOut.m_contactFlag;
		param->sphereOut->m_contactNormal = sphereOut.m_contactNormal;
		param->sphereOut->m_subData = sphereOut.m_subData;
		param->sphereOut->m_variable = sphereOut.m_variable;
		param->sphereOut->m_shapeId = largeTriMeshCache.m_meshShapeIndex;
		param->sphereOut->m_objectId = largeTriMeshCache.m_meshRbIndex;
	}
	return true;
}


static PfxBool findAllHitsLargeTriMeshCallbackForBvh(const PfxCacheLargeTriMesh &largeTriMeshCache, const PfxUInt32 islandIds[], void *userData)
{
	PfxTraverseSphereCallbackParam *param = (PfxTraverseSphereCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->sphereIn->m_contactFilterSelf, param->sphereIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->sphereIn->m_collisionIgnoreGroup[0], param->sphereIn->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->sphereIn->m_contactFilterSelf, param->sphereIn->m_contactFilterTarget))
		return true;

	// SphereCast from local to world	
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition sphereLargeStartPosition = param->sphereIn->m_startPosition;
	sphereLargeStartPosition.changeSegment(state.getSegment());

	PfxSphereInputInternal sphereIn;
	sphereIn.m_segment = sphereLargeStartPosition.segment;
	sphereIn.m_startPosition = sphereLargeStartPosition.offset;
	sphereIn.m_direction = param->sphereIn->m_direction;
	sphereIn.m_radius = param->sphereIn->m_radius;
	sphereIn.m_contactFilterSelf = param->sphereIn->m_contactFilterSelf;
	sphereIn.m_contactFilterTarget = param->sphereIn->m_contactFilterTarget;

	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.f;
	PfxFloat sphereLocalRadius = param->sphereIn->m_radius / minElem(absPerElem(scale));

	PfxSphereOutputInternal sphereOut;
	sphereOut.m_variable = 1.f;

	if (pfxIntersectMovingSphereIslandsOfLargeTriMeshBvh(sphereIn, sphereOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, flipTriangle, sphereLocalRadius, param->discardTriangleCallback, param->userDataForDiscardingTriangle))
	{
		PfxSphereOutput out;
		out.m_contactPoint.segment = sphereIn.m_segment;
		out.m_contactPoint.offset = sphereOut.m_contactPoint;
		out.m_contactFlag = sphereOut.m_contactFlag;
		out.m_contactNormal = sphereOut.m_contactNormal;
		out.m_subData = sphereOut.m_subData;
		out.m_variable = param->t0 + sphereOut.m_variable * (param->t1 - param->t0);
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
	PfxTraverseSphereCallbackParam *param = (PfxTraverseSphereCallbackParam*)userData;

	const PfxRigidState &state = param->states[largeTriMeshCache.m_meshRbIndex];
	const PfxCollidable &collidable = param->collidables[largeTriMeshCache.m_meshRbIndex];

	// Filtering the rigid body
	if (!(pfxCheckContactFilter(state.getContactFilterSelf(), state.getContactFilterTarget(),
			param->sphereIn->m_contactFilterSelf, param->sphereIn->m_contactFilterTarget) &&
		pfxCheckCollisionIgnoreGroup(state.getCollisionIgnoreGroup(0), state.getCollisionIgnoreGroup(1),
			param->sphereIn->m_collisionIgnoreGroup[0], param->sphereIn->m_collisionIgnoreGroup[1])))
		return true;

	// Filtering the shape
	const PfxShape &shape = collidable.getShape(largeTriMeshCache.m_meshShapeIndex);
	if (!pfxCheckContactFilter(shape.getContactFilterSelf(), shape.getContactFilterTarget(), param->sphereIn->m_contactFilterSelf, param->sphereIn->m_contactFilterTarget))
		return true;

	// SphereCast from local to world	
	PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;

	PfxLargePosition sphereLargeStartPosition = param->sphereIn->m_startPosition;
	sphereLargeStartPosition.changeSegment(state.getSegment());

	PfxSphereInputInternal sphereIn;
	sphereIn.m_segment = sphereLargeStartPosition.segment;
	sphereIn.m_startPosition = sphereLargeStartPosition.offset;
	sphereIn.m_direction = param->sphereIn->m_direction;
	sphereIn.m_radius = param->sphereIn->m_radius;
	sphereIn.m_contactFilterSelf = param->sphereIn->m_contactFilterSelf;
	sphereIn.m_contactFilterTarget = param->sphereIn->m_contactFilterTarget;

	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.f;
	PfxFloat sphereLocalRadius = param->sphereIn->m_radius / minElem(absPerElem(scale));

	PfxSphereOutputInternal sphereOut;
	sphereOut.m_variable = 1.f;

	if (pfxIntersectMovingSphereIslandsOfLargeTriMeshArray(sphereIn, sphereOut, shape.getLargeTriMesh(), islandIds + largeTriMeshCache.m_islandStartIndex, largeTriMeshCache.m_numIslandIndices, localToWorldTransform, flipTriangle, sphereLocalRadius, param->discardTriangleCallback, param->userDataForDiscardingTriangle))
	{
		PfxSphereOutput out;
		out.m_contactPoint.segment = sphereIn.m_segment;
		out.m_contactPoint.offset = sphereOut.m_contactPoint;
		out.m_contactFlag = sphereOut.m_contactFlag;
		out.m_contactNormal = sphereOut.m_contactNormal;
		out.m_subData = sphereOut.m_subData;
		out.m_variable = param->t0 + sphereOut.m_variable * (param->t1 - param->t0);
		out.m_shapeId = largeTriMeshCache.m_meshShapeIndex;
		out.m_objectId = largeTriMeshCache.m_meshRbIndex;
		if (!param->callback(out, param->userData)) {
			return false;
		}
	}

	return true;
}

// It returns no intersection if the start position is in a shape.
PfxInt32 pfxCastSingleSphere(const PfxSphereInput &sphereIn,PfxSphereOutput &sphereOut,const PfxSphereCastParam &param)
{
	if (!param.states || !param.collidables) return SCE_PFX_ERR_INVALID_VALUE;
	if (param.containerType == PfxSphereCastParam::PROXY_CONTAINER && param.proxyContainer == NULL) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	if (param.containerType == PfxSphereCastParam::CACHE_CONTAINER && param.cacheContainer == NULL)	return SCE_PFX_ERR_INVALID_VALUE;
	
	if(lengthSqr(sphereIn.m_direction) < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) return SCE_PFX_ERR_INVALID_VALUE;
		
	sphereOut.m_variable = 1.0f;
	sphereOut.m_contactFlag = false;
	
	PfxLargePosition rangeMin = param.rangeMin - PfxVector3(sphereIn.m_radius);
	PfxLargePosition rangeMax = param.rangeMax + PfxVector3(sphereIn.m_radius);
	
	// レイを領域でクリップ
	if (pfxClipRayByDotProduct(sphereIn.m_startPosition, sphereIn.m_direction, rangeMin, rangeMax)) {

		PfxFloat t0 = 0.0f;
		PfxFloat t1 = 1.0f;
		pfxClipRay(sphereIn.m_startPosition, sphereIn.m_direction, rangeMin, rangeMax, t0, t1);

		PfxSphereInput sphereIn_ = sphereIn;
		sphereIn_.m_startPosition = sphereIn.m_startPosition + t0 * sphereIn.m_direction;
		sphereIn_.m_direction = (t1 - t0) * sphereIn.m_direction;
		PfxSphereOutput sphereOut_ = sphereOut;
		PfxTraverseSphereCallbackParam param_;
		param_.sphereIn = &sphereIn_;
		param_.sphereOut = &sphereOut_;
		param_.t0 = t0;
		param_.t1 = t1;
		param_.states = param.states;
		param_.collidables = param.collidables;
		param_.discardTriangleCallback = param.discardTriangleCallback;
		param_.userDataForDiscardingTriangle = param.userDataForDiscardingTriangle;

		if (param.containerType == PfxSphereCastParam::PROXY_CONTAINER) {
			PfxRayInput ray; // Ray for bvh traversal.
			ray.m_contactFilterSelf = sphereIn_.m_contactFilterSelf;
			ray.m_contactFilterTarget = sphereIn_.m_contactFilterTarget;
			ray.m_direction = sphereIn_.m_direction;
			ray.m_startPosition = sphereIn_.m_startPosition;

			PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)param.proxyContainer;
			bvh->traverseRayClosest(findClosestHitCallback, ray, sphereIn.m_radius, &param_);
		}

		else if (param.containerType == PfxSphereCastParam::CACHE_CONTAINER) {
			PfxLargePosition sphereEndPosition = sphereIn_.m_startPosition + sphereIn_.m_direction;
			PfxTraverseRigidBodyCacheContainerCallbackParam cacheParam;
			cacheParam.states = param.states;
			cacheParam.collidables = param.collidables;
			cacheParam.rangeOfInterest.vmin = minPerElem(sphereIn.m_startPosition, sphereEndPosition) - PfxVector3(sphereIn.m_radius);
			cacheParam.rangeOfInterest.vmax = maxPerElem(sphereIn.m_startPosition, sphereEndPosition) + PfxVector3(sphereIn.m_radius);

			const PfxRigidBodyCacheManager *cacheManager = ((const PfxRigidBodyCacheManager*)param.cacheContainer);
			cacheManager->traverseRayClosest(findClosestHitCallbackForCache, findClosestHitLargeTriMeshCallbackForBvh, findClosestHitLargeTriMeshCallbackForArray, cacheParam, &param_);
		}

		if (sphereOut_.m_contactFlag && sphereOut_.m_variable < sphereOut.m_variable)
			sphereOut = sphereOut_;

		sphereOut.m_variable = t0 + sphereOut.m_variable * (t1 - t0);
	}

	return SCE_PFX_OK;
}

PfxInt32 pfxCastSingleSphere(const PfxSphereInput &sphereIn, pfxSphereHitCallback callback, void *userData, const PfxSphereCastParam &param)
{
	if (!callback)	return SCE_PFX_OK;
	if (!param.states || !param.collidables)	return SCE_PFX_ERR_INVALID_VALUE;
	if (param.containerType == PfxSphereCastParam::PROXY_CONTAINER && param.proxyContainer == NULL) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	if (param.containerType == PfxSphereCastParam::CACHE_CONTAINER && param.cacheContainer == NULL)	return SCE_PFX_ERR_INVALID_VALUE;

	if (lengthSqr(sphereIn.m_direction) < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) return SCE_PFX_ERR_INVALID_VALUE;

	PfxLargePosition rangeMin = param.rangeMin - PfxVector3(sphereIn.m_radius);
	PfxLargePosition rangeMax = param.rangeMax + PfxVector3(sphereIn.m_radius);

	// レイを領域でクリップ
	if (pfxClipRayByDotProduct(sphereIn.m_startPosition, sphereIn.m_direction, rangeMin, rangeMax)) {

		PfxFloat t0 = 0.0f;
		PfxFloat t1 = 1.0f;
		pfxClipRay(sphereIn.m_startPosition, sphereIn.m_direction, rangeMin, rangeMax, t0, t1);

		PfxSphereInput sphereIn_ = sphereIn;
		sphereIn_.m_startPosition = sphereIn.m_startPosition + t0 * sphereIn.m_direction;
		sphereIn_.m_direction = (t1 - t0) * sphereIn.m_direction;
		PfxTraverseSphereCallbackParam param_;
		param_.sphereIn = &sphereIn_;
		param_.t0 = t0;
		param_.t1 = t1;
		param_.states = param.states;
		param_.collidables = param.collidables;
		param_.callback = callback;
		param_.userData = userData;
		param_.discardTriangleCallback = param.discardTriangleCallback;
		param_.userDataForDiscardingTriangle = param.userDataForDiscardingTriangle;

		if (param.containerType == PfxSphereCastParam::PROXY_CONTAINER) {
			PfxRayInput ray; // Ray for bvh traversal.
			ray.m_contactFilterSelf = sphereIn_.m_contactFilterSelf;
			ray.m_contactFilterTarget = sphereIn_.m_contactFilterTarget;
			ray.m_direction = sphereIn_.m_direction;
			ray.m_startPosition = sphereIn_.m_startPosition;

			PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)param.proxyContainer;
			bvh->traverseRayOverlap(findAllHitsCallback, ray, sphereIn.m_radius, &param_);
		}

		else if (param.containerType == PfxSphereCastParam::CACHE_CONTAINER) {
			PfxLargePosition sphereEndPosition = sphereIn_.m_startPosition + sphereIn_.m_direction;
			PfxTraverseRigidBodyCacheContainerCallbackParam cacheParam;
			cacheParam.states = param.states;
			cacheParam.collidables = param.collidables;
			cacheParam.rangeOfInterest.vmin = minPerElem(sphereIn.m_startPosition, sphereEndPosition) - PfxVector3(sphereIn.m_radius);
			cacheParam.rangeOfInterest.vmax = maxPerElem(sphereIn.m_startPosition, sphereEndPosition) + PfxVector3(sphereIn.m_radius);

			const PfxRigidBodyCacheManager *cacheManager = ((const PfxRigidBodyCacheManager*)param.cacheContainer);
			cacheManager->traverseRayOverlap(findAllHitsCallbackForCache, findAllHitsLargeTriMeshCallbackForBvh, findAllHitsLargeTriMeshCallbackForArray, cacheParam, &param_);
		}
	}

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
