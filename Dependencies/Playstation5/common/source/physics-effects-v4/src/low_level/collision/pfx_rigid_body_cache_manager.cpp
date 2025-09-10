/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
*                Copyright (C) 2020 Sony Interactive Entertainment Inc.
*                                                
*/

#include "pfx_precompiled.h"
#include "pfx_rigid_body_cache_manager.h"
#include "../../low_level/broadphase/pfx_bounding_volume.h"
#include "../../low_level/collision/pfx_bounding_volume_vector3.h"
#include "../broadphase/pfx_progressive_bvh.h"
#include "../../../include/physics_effects/util/pfx_static_array.h"
#include "../../base_level/collision/pfx_large_tri_mesh_impl.h"

namespace sce {
namespace pfxv4 {

PfxInt32 PfxRigidBodyCacheManager::initialize(PfxUInt32 maxRigidBodies, PfxUInt32 maxLargeTriMeshes, PfxUInt32 maxIslandsPerMesh, void *workBuff, PfxUInt32 workBytes)
{
	if (workBytes < PfxRigidBodyCacheManager::getBytes(maxRigidBodies, maxLargeTriMeshes, maxIslandsPerMesh)) {
		return SCE_PFX_ERR_OUT_OF_BUFFER;
	}

	m_maxRigidBodies = maxRigidBodies;
	m_maxLargeTriMeshes = maxLargeTriMeshes;
	m_maxLargeTriMeshIslands = maxLargeTriMeshes * maxIslandsPerMesh;

	m_rigidBodyExists = (PfxBool*)workBuff;
	m_areMeshIslandIndicesSaved = (PfxBool*)(m_rigidBodyExists + m_maxRigidBodies);
	m_bvNodesBvh = (PfxCacheBvNode*)(m_areMeshIslandIndicesSaved + m_maxRigidBodies);
	m_bvhMeshes = (PfxCacheLargeTriMesh*)(m_bvNodesBvh + m_maxRigidBodies);
	m_arrayMeshes = m_bvhMeshes + m_maxLargeTriMeshes;
	m_largeTriMeshIslandIndices = (PfxUInt32*)(m_arrayMeshes + m_maxLargeTriMeshes);

	clear();

	return SCE_PFX_OK;
}

void PfxRigidBodyCacheManager::finalize()
{

}

void PfxRigidBodyCacheManager::clear()
{
	for (PfxUInt32 i = 0; i < m_maxRigidBodies; ++i) {
		m_rigidBodyExists[i] = false;
		m_areMeshIslandIndicesSaved[i] = false;
	}

	m_numBvNodesBvh = m_numLargeTriMeshIslands = 0u;	
	m_numBvhMeshes = m_numArrayMeshes = 0u;
}

struct EncStack {
	PfxUInt32 nodeId;
	PfxVector3 aabbMin;
	PfxVector3 aabbMax;
};

struct PfxRigidBodyCacheRangeRayInternal
{
	PfxRayInput ray;
	PfxFloat radius;
};

struct PfxRangeSphereVector3
{
	PfxVector3 center;
	PfxFloat radius;

	PfxRangeSphereVector3(const PfxRigidBodyCacheRangeSphere &sphere) : center(sphere.center.offset), radius(sphere.radius) {}
};

struct PfxRangeRayVector3
{
	PfxVector3 startPosition;
	PfxVector3 direction;
	PfxFloat radius;

	PfxRangeRayVector3(const PfxRigidBodyCacheRangeRayInternal &sphereCast) :
		startPosition(sphereCast.ray.m_startPosition.offset),
		direction(sphereCast.ray.m_direction),
		radius(sphereCast.radius)
	{}
};

static SCE_PFX_FORCE_INLINE PfxBool pfxTestFuncVector3(const PfxBvVector3 &bvA, const PfxRangeSphereVector3 &sphereB)
{
	PfxVector3 closestPoint = sphereB.center;
	closestPoint = maxPerElem(closestPoint, bvA.vmin);
	closestPoint = minPerElem(closestPoint, bvA.vmax);
	PfxFloat diffSqr = lengthSqr(closestPoint - sphereB.center);
	return (diffSqr < sphereB.radius * sphereB.radius);
}

static SCE_PFX_FORCE_INLINE PfxBool pfxTestFuncVector3(const PfxBvVector3 &bvA, const PfxRangeRayVector3 &rayB)
{
#ifdef PFX_ENABLE_AVX
	const __m128 rayRadius = _mm_set1_ps(rayB.radius);
	const __m128 bvMin = _mm_sub_ps(sce_vectormath_asm128(bvA.vmin.get128()), rayRadius);
	const __m128 bvMax = _mm_add_ps(sce_vectormath_asm128(bvA.vmax.get128()), rayRadius);

	static const __m128 positiveEpsilon = (sce_vectormath_asm128(PfxVector3(1e-5f).get128()));
	static const __m128 negativeEpsilon = (sce_vectormath_asm128(PfxVector3(-1e-5f).get128()));
	const __m128 rayDirection = sce_vectormath_asm128(rayB.direction.get128());
	const __m128 rayStartPosition = sce_vectormath_asm128(rayB.startPosition.get128());

	// Detect the class of ray's direction
	__m128 iGtZero = (_mm_cmpgt_ps(rayDirection, positiveEpsilon));
	__m128 iLtZero = (_mm_cmplt_ps(rayDirection, negativeEpsilon));
	__m128 iLtZeroShuffled = pfxShufflePs<1, 2, 0, 3>( iLtZero );

	// Detect whether the ray is totally outside of Aabb box
	__m128 rayEndPoint = _mm_add_ps(rayStartPosition, rayDirection);

	__m128 rayMax = _mm_max_ps(rayStartPosition, rayEndPoint);
	__m128 rayMin = _mm_min_ps(rayStartPosition, rayEndPoint);

	int test1 = _mm_movemask_ps(_mm_cmplt_ps(bvMax, rayMin));
	int test2 = _mm_movemask_ps(_mm_cmplt_ps(rayMax, bvMin));
	if (((test1 | test2) & 0x07) != 0)
		return false;

	// Vectors from ray's start point to rangeMin and rangeMax
	__m128 rayToMin = _mm_sub_ps(bvMin, rayStartPosition);
	__m128 rayToMax = _mm_sub_ps(bvMax, rayStartPosition);
	__m128 rayToMinShuffled = pfxShufflePs<1, 2, 0, 3>( rayToMin );
	__m128 rayToMaxShuffled = pfxShufflePs<1, 2, 0, 3>( rayToMax );

	// The normal vector of ray's direction
	__m128 vDirVer = pfxShufflePs<1, 2, 0, 3>( rayDirection );

	__m128 iSwap = _mm_xor_ps( iGtZero, iLtZeroShuffled );
	__m128 ip1 = _mm_or_ps( _mm_andnot_ps( iSwap, rayToMinShuffled ), _mm_and_ps( iSwap, rayToMaxShuffled ) );
	__m128 ip2 = _mm_or_ps( _mm_andnot_ps( iSwap, rayToMaxShuffled ), _mm_and_ps( iSwap, rayToMinShuffled ) );

	__m128 dotRes1 = _mm_sub_ps(_mm_mul_ps(ip1, rayDirection), _mm_mul_ps(rayToMin, vDirVer));
	__m128 dotRes2 = _mm_sub_ps(_mm_mul_ps(ip2, rayDirection), _mm_mul_ps(rayToMax, vDirVer));

	return ((_mm_movemask_ps(_mm_cmplt_ps(_mm_mul_ps(dotRes1, dotRes2), positiveEpsilon)) & 0x7) == 0x7);
#else
	PfxBvVector3 tempBvA(bvA);
	tempBvA.vmin -= PfxVector3(rayB.radius);
	tempBvA.vmax += PfxVector3(rayB.radius);

	PfxVector3 rayEndPoint = rayB.direction + rayB.startPosition;
	PfxVector3 rayAabbMin = minPerElem(rayB.startPosition, rayEndPoint);
	PfxVector3 rayAabbMax = maxPerElem(rayB.startPosition, rayEndPoint);
	PfxBvVector3 rayBv(rayAabbMin, rayAabbMax);
	if (!pfxTestFuncVector3(tempBvA, rayBv)) {
		return false;
	}

	PfxVector3 rayToMin(tempBvA.vmin - rayB.startPosition);
	PfxVector3 rayToMax(tempBvA.vmax - rayB.startPosition);

	// xy-plane
	PfxVector3 normalDir(-rayB.direction.getY(), rayB.direction.getX(), 0.f);
	PfxVector3 p1(rayToMin.getX(), pfxClipRayGetP1Elem2(rayB.direction.getX(), rayB.direction.getY(), rayToMax.getY(), rayToMin.getY()), 0.f);
	PfxVector3 p2(rayToMax.getX(), pfxClipRayGetP2Elem2(rayB.direction.getX(), rayB.direction.getY(), rayToMax.getY(), rayToMin.getY()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	// yz-plane
	normalDir = PfxVector3(-rayB.direction.getZ(), rayB.direction.getY(), 0.f);
	p1 = PfxVector3(rayToMin.getY(), pfxClipRayGetP1Elem2(rayB.direction.getY(), rayB.direction.getZ(), rayToMax.getZ(), rayToMin.getZ()), 0.f);
	p2 = PfxVector3(rayToMax.getY(), pfxClipRayGetP2Elem2(rayB.direction.getY(), rayB.direction.getZ(), rayToMax.getZ(), rayToMin.getZ()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	// zx-plane
	normalDir = PfxVector3(-rayB.direction.getX(), rayB.direction.getZ(), 0.f);
	p1 = PfxVector3(rayToMin.getZ(), pfxClipRayGetP1Elem2(rayB.direction.getZ(), rayB.direction.getX(), rayToMax.getX(), rayToMin.getX()), 0.f);
	p2 = PfxVector3(rayToMax.getZ(), pfxClipRayGetP2Elem2(rayB.direction.getZ(), rayB.direction.getX(), rayToMax.getX(), rayToMin.getX()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	return true;
#endif
}

PfxBool SCE_PFX_FORCE_INLINE PfxRigidBodyCacheManager::isIslandExisted(PfxUInt32 rbId, PfxUInt32 shapeId, PfxUInt32 islandId) {
	for (PfxUInt32 i = 0; i < m_numBvhMeshes - 1; ++i) {
		PfxCacheLargeTriMesh &largeTriMeshCache = m_bvhMeshes[i];

		if (largeTriMeshCache.m_meshRbIndex == rbId && largeTriMeshCache.m_meshShapeIndex) {
			PfxUInt32 *islandIds = m_largeTriMeshIslandIndices + largeTriMeshCache.m_islandStartIndex;
			for (PfxUInt32 j = 0; j < largeTriMeshCache.m_numIslandIndices; ++j) {
				if (islandIds[j] == islandId)
					return true;
			}
		}
	}
	return false;
}

template<typename CacheRangeType>
PfxInt32 SCE_PFX_FORCE_INLINE PfxRigidBodyCacheManager::addMeshInternal(PfxUInt32 rbId, PfxUInt32 shapeId, const PfxRigidState &state, const PfxShape &shape, const CacheRangeType &range)
{
	// Start to add the islands
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape.getLargeTriMesh();

	if (largeMesh->isUsingBvh()) {
		PfxCacheLargeTriMesh *curLargeTriMeshCache;
		if (m_numBvhMeshes >= m_maxLargeTriMeshes)	return SCE_PFX_ERR_OUT_OF_BUFFER;
		curLargeTriMeshCache = m_bvhMeshes + (m_numBvhMeshes++);
		curLargeTriMeshCache->m_meshRbIndex = rbId;
		curLargeTriMeshCache->m_meshShapeIndex = shapeId;
		curLargeTriMeshCache->m_islandStartIndex = m_numLargeTriMeshIslands;

		EncStack encroot;
		encroot.nodeId = 0;
		encroot.aabbMin = largeMesh->getOffset() - largeMesh->getHalf();
		encroot.aabbMax = largeMesh->getOffset() + largeMesh->getHalf();

		PfxStaticStack<EncStack> bvhStack;
		bvhStack.push(encroot);

		do
		{
			EncStack info = bvhStack.top();
			bvhStack.pop();
			PfxIslandBvhNode &encnode = largeMesh->m_bvhNodes[info.nodeId];

			PfxVector3 quantizedMin(encnode.aabb[0], encnode.aabb[1], encnode.aabb[2]);
			PfxVector3 quantizedMax(encnode.aabb[3], encnode.aabb[4], encnode.aabb[5]);

			PfxBvVector3 nodeRange;
			nodeRange.vmin = info.aabbMin + mulPerElem((info.aabbMax - info.aabbMin), quantizedMin / 255.f);
			nodeRange.vmax = info.aabbMax - mulPerElem((info.aabbMax - info.aabbMin), quantizedMax / 255.f);
			if (!pfxTestFuncVector3(nodeRange, range))	continue;

			// Information about back-tracking trees
			EncStack encnext;
			encnext.aabbMin = nodeRange.vmin;
			encnext.aabbMax = nodeRange.vmax;

			PfxUInt8 lStatus = encnode.flag & 0x03;
			PfxUInt8 rStatus = (encnode.flag & 0x0C) >> 2;

			if (lStatus == 0) {
				encnext.nodeId = encnode.left;
				bvhStack.push(encnext);
			}
			else if (lStatus == 1) {
				if (m_numLargeTriMeshIslands >= m_maxLargeTriMeshIslands)	return SCE_PFX_ERR_OUT_OF_BUFFER;
				if (!isIslandExisted(rbId, shapeId, encnode.left)) {
					m_largeTriMeshIslandIndices[m_numLargeTriMeshIslands++] = encnode.left;
				}
			}

			if (rStatus == 0) {
				encnext.nodeId = encnode.right;
				bvhStack.push(encnext);
			}
			else if (rStatus == 1) {
				if (m_numLargeTriMeshIslands >= m_maxLargeTriMeshIslands)	return SCE_PFX_ERR_OUT_OF_BUFFER;
				if (!isIslandExisted(rbId, shapeId, encnode.right)) {
					m_largeTriMeshIslandIndices[m_numLargeTriMeshIslands++] = encnode.right;
				}
			}

		} while (!bvhStack.empty());
		curLargeTriMeshCache->m_numIslandIndices = m_numLargeTriMeshIslands - curLargeTriMeshCache->m_islandStartIndex;
	}
	else { // if (largeMesh->isUsingBvh())
		PfxCacheLargeTriMesh *curLargeTriMeshCache;
		if (m_numArrayMeshes >= m_maxLargeTriMeshes)	return SCE_PFX_ERR_OUT_OF_BUFFER;
		curLargeTriMeshCache = m_arrayMeshes + (m_numArrayMeshes++);
		curLargeTriMeshCache->m_meshRbIndex = rbId;
		curLargeTriMeshCache->m_meshShapeIndex = shapeId;
		curLargeTriMeshCache->m_islandStartIndex = m_numLargeTriMeshIslands;

		PfxBvVector3 islandBvVector3;

		PfxUInt32 numIslands = largeMesh->m_numIslands;
		for (PfxUInt32 islandId = 0; islandId < numIslands; ++islandId) {
			const PfxAabb16 &islandAabb = largeMesh->m_aabbList[islandId];
			PfxVecInt3 islandMin((PfxFloat)pfxGetXMin(islandAabb), (PfxFloat)pfxGetYMin(islandAabb), (PfxFloat)pfxGetZMin(islandAabb));
			PfxVecInt3 islandMax((PfxFloat)pfxGetXMax(islandAabb), (PfxFloat)pfxGetYMax(islandAabb), (PfxFloat)pfxGetZMax(islandAabb));
			islandBvVector3.vmin = largeMesh->getWorldPosition(islandMin);
			islandBvVector3.vmax = largeMesh->getWorldPosition(islandMax);
			if (!pfxTestFuncVector3(islandBvVector3, range))	continue;

			if (m_numLargeTriMeshIslands >= m_maxLargeTriMeshIslands)	return SCE_PFX_ERR_OUT_OF_BUFFER;
			m_largeTriMeshIslandIndices[m_numLargeTriMeshIslands++] = islandId;
		}
		curLargeTriMeshCache->m_numIslandIndices = m_numLargeTriMeshIslands - curLargeTriMeshCache->m_islandStartIndex;
	}
	return SCE_PFX_OK;
}

PfxInt32 PfxRigidBodyCacheManager::addMesh(PfxUInt32 rbId, PfxUInt32 shapeId, const PfxRigidState &state, const PfxShape &shape, const PfxTransform3 &rbTransform, PfxBv &rangeBv)
{
	// Change the segment
	rangeBv.vmax.changeSegment(state.getSegment());
	rangeBv.vmin.changeSegment(state.getSegment());

	// Change to local coordinates
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform(rbTransform * shapeTransform);
	PfxTransform3 worldToLocalTransform(inverse(localToWorldTransform));
	PfxMatrix3 worldToLocalRotationMatrix(worldToLocalTransform.getUpper3x3());
	PfxVector3 worldToLocalTranslation(worldToLocalTransform.getTranslation());

	PfxBvVector3 rangeBvVector3;
	rangeBvVector3.vmax = worldToLocalRotationMatrix * rangeBv.vmax.offset + worldToLocalTranslation;
	rangeBvVector3.vmin = worldToLocalRotationMatrix * rangeBv.vmin.offset + worldToLocalTranslation;
	
	return addMeshInternal(rbId, shapeId, state, shape, rangeBvVector3);
}

PfxInt32 PfxRigidBodyCacheManager::addMesh(PfxUInt32 rbId, PfxUInt32 shapeId, const PfxRigidState &state, const PfxShape &shape, const PfxTransform3 &rbTransform, PfxRigidBodyCacheRangeSphere &rangeSphere)
{
	// Change the segment
	rangeSphere.center.changeSegment(state.getSegment());
	PfxRangeSphereVector3 rangeSphereVector3(rangeSphere);

	// Change to local coordinates
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;
	PfxTransform3 worldToLocalTransform = inverse(localToWorldTransform);
	PfxMatrix3 worldToLocalRotationMatrix(worldToLocalTransform.getUpper3x3());
	PfxVector3 worldToLocalTranslation(worldToLocalTransform.getTranslation());

	rangeSphereVector3.center = worldToLocalRotationMatrix * rangeSphereVector3.center + worldToLocalTranslation;
	rangeSphereVector3.radius /= minElem(shape.getScaleXyz());

	return addMeshInternal(rbId, shapeId, state, shape, rangeSphereVector3);
}

PfxInt32 PfxRigidBodyCacheManager::addMesh(PfxUInt32 rbId, PfxUInt32 shapeId, const PfxRigidState &state, const PfxShape &shape, const PfxTransform3 &rbTransform, PfxRigidBodyCacheRangeRayInternal &rangeRay)
{
	// Change the segment
	rangeRay.ray.m_startPosition.changeSegment(state.getSegment());
	PfxRangeRayVector3 rangeRayVector3(rangeRay);

	// Change to local coordinates
	PfxTransform3 shapeTransform = appendScale(shape.getOffsetTransform(), shape.getScaleXyz());
	PfxTransform3 localToWorldTransform = rbTransform * shapeTransform;
	PfxTransform3 worldToLocalTransform = inverse(localToWorldTransform);
	PfxMatrix3 worldToLocalRotationMatrix(worldToLocalTransform.getUpper3x3());
	PfxVector3 worldToLocalTranslation(worldToLocalTransform.getTranslation());

	rangeRayVector3.startPosition = worldToLocalRotationMatrix * rangeRayVector3.startPosition + worldToLocalTranslation;
	rangeRayVector3.direction = worldToLocalRotationMatrix * rangeRayVector3.direction;
	rangeRayVector3.radius /= minElem(shape.getScaleXyz());
	
	return addMeshInternal(rbId, shapeId, state, shape, rangeRayVector3);
}

static SCE_PFX_FORCE_INLINE PfxBool pfxTestFunc(const PfxBv &bvA, const PfxBv &bvB)
{
	return pfxTestBv(bvA, bvB);
}

static SCE_PFX_FORCE_INLINE PfxBool pfxTestFunc(const PfxBv &bvA, const PfxRigidBodyCacheRangeSphere &sphereB)
{
	return pfxTestBvSphere(bvA, sphereB.center, sphereB.radius);
}

static SCE_PFX_FORCE_INLINE PfxBool pfxTestFunc(const PfxBv &bvA, const PfxRigidBodyCacheRangeRayInternal &sphereCastB)
{
	PfxBv tempBv(bvA);
	tempBv.vmin.offset -= PfxVector3(sphereCastB.radius);
	tempBv.vmax.offset += PfxVector3(sphereCastB.radius);
	return pfxTestBvRay(tempBv, sphereCastB.ray);
}

template<typename CacheRangeType>
PfxInt32 SCE_PFX_FORCE_INLINE PfxRigidBodyCacheManager::addRbInternal(const PfxRigidBodyCacheParam &param, CacheRangeType &range)
{
	if (param.states == NULL || param.collidables == NULL || param.proxyContainer == NULL)
		return SCE_PFX_ERR_INVALID_VALUE;

	PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)param.proxyContainer;

	PfxUInt32 nodeId = bvh->m_bvNodes[0].childL;
	while(nodeId != 0xffff) {
		const PfxProgressiveBvh::PfxBvNode &node = bvh->m_bvNodes[nodeId];
		PfxBv bv = node.getBv();

		if (node.valid == 1 && pfxTestFunc(node.getBv(), range)) {
			if(node.isLeaf()) {
				if (node.proxyId != 0xffff) {
					PfxUInt32 rbId = node.proxyId;
					const PfxRigidState &state(param.states[rbId]);
					const PfxCollidable &collidable(param.collidables[rbId]);

					if (param.callback)
						if (!(*param.callback)(state, collidable, param.userData))
							continue;
					
					PfxTransform3 rbTransform(state.getOrientation(), state.getPosition());

					// If this mesh has not been added by using index
					if (!(m_rigidBodyExists[rbId] && !m_areMeshIslandIndicesSaved[rbId])) {
						for (PfxUInt32 i = 0; i < collidable.getNumShapes(); ++i) {
							const PfxShape &shape(collidable.getShape(i));

							if (shape.getType() == kPfxShapeLargeTriMesh) {
								PfxInt32 res = addMesh(rbId, i, state, shape, rbTransform, range);
								if (res == SCE_PFX_ERR_OUT_OF_BUFFER) {
									return SCE_PFX_ERR_OUT_OF_BUFFER;
								}
							}
						}
					}

					if (!m_rigidBodyExists[rbId])
					{
						// Cache the rigid body 
						if (m_numBvNodesBvh >= m_maxRigidBodies)	return SCE_PFX_ERR_OUT_OF_BUFFER;

						PfxCacheBvNode &bvNodeCacheBvh = m_bvNodesBvh[m_numBvNodesBvh++];
						bvNodeCacheBvh.m_proxyContainerPtr = param.proxyContainer;
						bvNodeCacheBvh.m_bvNodeIndex = nodeId;
						m_rigidBodyExists[rbId] = true;
						m_areMeshIslandIndicesSaved[rbId] = true;
					}
				}
				nodeId = node.destination;
			}
			else {
				nodeId = node.childL;
			}
		}
		else {
			nodeId = node.destination;
		}
	}

	return SCE_PFX_OK;
}

PfxInt32 PfxRigidBodyCacheManager::addRb(const PfxRigidBodyCacheParam &param, const PfxRigidBodyCacheRangeAabb &rangeAabb)
{
	PfxBv rangeBv;
	rangeBv.vmin = rangeAabb.vmin;
	rangeBv.vmax = rangeAabb.vmax;
	return addRbInternal(param, rangeBv);
}

PfxInt32 PfxRigidBodyCacheManager::addRb(const PfxRigidBodyCacheParam &param, const PfxRigidBodyCacheRangeSphere &rangeSphere)
{
	PfxRigidBodyCacheRangeSphere tempRangeSphere(rangeSphere);
	return addRbInternal(param, tempRangeSphere);
}

PfxInt32 PfxRigidBodyCacheManager::addRb(const PfxRigidBodyCacheParam &param, const PfxRigidBodyCacheRangeRay &rangeRay)
{
	PfxRigidBodyCacheRangeRayInternal rangeRayInternal;
	rangeRayInternal.ray.m_startPosition = rangeRay.startPosition;
	rangeRayInternal.ray.m_direction = rangeRay.direction;
	rangeRayInternal.radius = rangeRay.radius;
	return addRbInternal(param, rangeRayInternal);
}

PfxInt32 PfxRigidBodyCacheManager::addRb(const PfxRigidBodyCacheParam &param, PfxUInt32 rbId)
{
	if (!m_rigidBodyExists[rbId]) {
		if (m_numBvNodesBvh >= m_maxRigidBodies)	return SCE_PFX_ERR_OUT_OF_BUFFER;

		PfxProgressiveBvh *bvh = (PfxProgressiveBvh*)param.proxyContainer;

		int num = 1 << bvh->m_layers;
		int start = num - 1;
		for (int nodeId = start; nodeId < start + num; ++nodeId) {
			const PfxProgressiveBvh::PfxBvNode &node = bvh->m_bvNodes[nodeId];

			if (node.proxyId == rbId) {
				PfxCacheBvNode &bvNodeCacheBvh = m_bvNodesBvh[m_numBvNodesBvh++];
				bvNodeCacheBvh.m_proxyContainerPtr = param.proxyContainer;
				bvNodeCacheBvh.m_bvNodeIndex = nodeId;
				m_rigidBodyExists[rbId] = true;
				m_areMeshIslandIndicesSaved[rbId] = false;
				return SCE_PFX_OK;
			}
		}
		return SCE_PFX_ERR_INVALID_RIGID_BODY_INDEX;
	}
	return SCE_PFX_OK;
}

void PfxRigidBodyCacheManager::traverseRayOverlap(
	pfxTraverseRigidBodyCacheContainerCallback rigidBodyFuncPtr, 
	pfxTraverseLargeTriMeshCacheContainerCallback largeTriMeshBvhFuncPtr,
	pfxTraverseLargeTriMeshCacheContainerCallback largeTriMeshArrayFuncPtr,
	const PfxTraverseRigidBodyCacheContainerCallbackParam &param, void *userData) const
{
	if (rigidBodyFuncPtr)
	{
		for (PfxUInt32 i = 0; i < m_numBvNodesBvh; ++i) {
			PfxCacheBvNode &bvNodeCache = m_bvNodesBvh[i];
			const PfxProgressiveBvh::PfxBvNode &bvNode = ((PfxProgressiveBvh*)bvNodeCache.m_proxyContainerPtr)->m_bvNodes[bvNodeCache.m_bvNodeIndex];

			if (pfxTestBv(bvNode.bv, param.rangeOfInterest))
				if (!(*rigidBodyFuncPtr)(bvNode.proxyId, bvNode.bv, m_areMeshIslandIndicesSaved[bvNode.proxyId], userData))
					return;
		}
	}

	if (largeTriMeshBvhFuncPtr)
	{
		for (PfxUInt32 i = 0; i < m_numBvhMeshes; ++i) {
			PfxCacheLargeTriMesh &largeTriMeshCache = m_bvhMeshes[i];
			if (!(*largeTriMeshBvhFuncPtr)(largeTriMeshCache, m_largeTriMeshIslandIndices, userData))
				return;
		}
	}

	if (largeTriMeshArrayFuncPtr)
	{
		for (PfxUInt32 i = 0; i < m_numArrayMeshes; ++i) {
			PfxCacheLargeTriMesh &largeTriMeshCache = m_arrayMeshes[i];
			if (!(*largeTriMeshArrayFuncPtr)(largeTriMeshCache, m_largeTriMeshIslandIndices, userData))
				return;
		}
	}
}

void PfxRigidBodyCacheManager::traverseRayClosest(
	pfxTraverseRigidBodyCacheContainerCallbackForRayClipping rigidBodyFuncPtr,
	pfxTraverseLargeTriMeshCacheContainerCallback largeTriMeshBvhFuncPtr,
	pfxTraverseLargeTriMeshCacheContainerCallback largeTriMeshArrayFuncPtr,
	const PfxTraverseRigidBodyCacheContainerCallbackParam &param, void *userData) const
{
	PfxRayInput clipRay; // dummy
	PfxFloat tmin = 0.0f;

	if (rigidBodyFuncPtr)
	{
		for (PfxUInt32 i = 0; i < m_numBvNodesBvh; ++i) {
			PfxCacheBvNode &bvNodeCache = m_bvNodesBvh[i];
			const PfxProgressiveBvh::PfxBvNode &bvNode = ((PfxProgressiveBvh*)bvNodeCache.m_proxyContainerPtr)->m_bvNodes[bvNodeCache.m_bvNodeIndex];

			if (pfxTestBv(bvNode.bv, param.rangeOfInterest))
				if (!(*rigidBodyFuncPtr)(bvNode.proxyId, bvNode.bv, m_areMeshIslandIndicesSaved[bvNode.proxyId], clipRay, tmin, userData))
					return;
		}
	}

	if (largeTriMeshBvhFuncPtr)
	{
		for (PfxUInt32 i = 0; i < m_numBvhMeshes; ++i) {
			PfxCacheLargeTriMesh &largeTriMeshCache = m_bvhMeshes[i];
			if (!(*largeTriMeshBvhFuncPtr)(largeTriMeshCache, m_largeTriMeshIslandIndices, userData))
				return;
		}
	}

	if (largeTriMeshArrayFuncPtr)
	{
		for (PfxUInt32 i = 0; i < m_numArrayMeshes; ++i) {
			PfxCacheLargeTriMesh &largeTriMeshCache = m_arrayMeshes[i];
			if (!(*largeTriMeshArrayFuncPtr)(largeTriMeshCache, m_largeTriMeshIslandIndices, userData))
				return;
		}
	}
}

}
}

