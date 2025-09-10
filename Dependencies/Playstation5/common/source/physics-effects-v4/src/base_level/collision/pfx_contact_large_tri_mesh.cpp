/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2023 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "pfx_contact_tri_mesh_sphere.h"
#include "pfx_contact_tri_mesh_box.h"
#include "pfx_contact_tri_mesh_capsule.h"
#include "pfx_contact_tri_mesh_cylinder.h"
#include "pfx_contact_tri_mesh_convex.h"
#include "pfx_contact_large_tri_mesh.h"
#include "pfx_intersect_common.h"
#include "pfx_mesh_common.h"

namespace sce {
namespace pfxv4 {

template<typename PfxShapeType>
PfxVector3 getShapeExtend(const PfxShapeType &shape)
{
	return PfxVector3::zero();
}

template<>
PfxVector3 getShapeExtend<PfxSphere>(const PfxSphere &sphere)
{
	return PfxVector3(sphere.m_radius);
}

template<>
PfxVector3 getShapeExtend<PfxBox>(const PfxBox &box)
{
	return box.m_half;
}

template<>
PfxVector3 getShapeExtend<PfxCapsule>(const PfxCapsule &capsule)
{
	return PfxVector3(capsule.m_halfLen + capsule.m_radius, capsule.m_radius, capsule.m_radius);
}

template<>
PfxVector3 getShapeExtend<PfxCylinder>(const PfxCylinder &cylinder)
{
	return PfxVector3(cylinder.m_halfLen, cylinder.m_radius, cylinder.m_radius);
}

template<>
PfxVector3 getShapeExtend<PfxConvexMeshImpl>(const PfxConvexMeshImpl &convex)
{
	return pfxReadVector3(convex.m_half);
}

template<typename PfxShapeType, typename PfxLargeTriMeshIslandType>
PfxInt32 pfxContactLargeTriMesh(
				PfxContactCache &contacts,
				PfxBool flipTriangle,
				const PfxLargeTriMeshImpl *lmeshA,
				const PfxTransform3 &transformA,
				const PfxShapeType &shapeB,
				const PfxTransform3 &transformB,
				PfxFloat distanceThreshold)
{
	PfxTransform3 transformAB;
	PfxMatrix3 matrixAB;
	PfxVector3 offsetAB;

	PfxMatrix3 rotationA_n = transpose(inverse(transformA.getUpper3x3()));

	// Bローカル→Aローカルへの変換
	transformAB = inverse(transformA) * transformB;
	matrixAB = transformAB.getUpper3x3();
	offsetAB = transformAB.getTranslation();

	// -----------------------------------------------------
	// LargeTriMeshに含まれるTriMeshのAABBと凸体のAABBを判定し、
	// 交差するものを個別に衝突判定する。※LargeMesh座標系

	PfxVector3 shapeHalf(0.0f);
	PfxVector3 shapeCenter = offsetAB;

	shapeHalf = absPerElem(matrixAB) * getShapeExtend<PfxShapeType>(shapeB);

	// -----------------------------------------------------
	// アイランドとの衝突判定

	PfxVecInt3 aabbMinL,aabbMaxL;
	lmeshA->getLocalPosition((shapeCenter-shapeHalf),(shapeCenter+shapeHalf),aabbMinL,aabbMaxL);

	PfxUInt32 numIslands = lmeshA->m_numIslands;

	PfxContactCache localContacts;

	{
		for(PfxUInt32 i=0;i<numIslands;i++) {
			// AABBチェック
			PfxAabb16 aabbB = lmeshA->m_aabbList[i];
			if(aabbMaxL.getX() < pfxGetXMin(aabbB) || aabbMinL.getX() > pfxGetXMax(aabbB)) continue;
			if(aabbMaxL.getY() < pfxGetYMin(aabbB) || aabbMinL.getY() > pfxGetYMax(aabbB)) continue;
			if(aabbMaxL.getZ() < pfxGetZMin(aabbB) || aabbMinL.getZ() > pfxGetZMax(aabbB)) continue;

			localContacts.reset();

			PfxLargeTriMeshIslandType *island = (PfxLargeTriMeshIslandType*)lmeshA->m_islands + i;

			pfxContactTriMesh(localContacts, lmeshA, island, flipTriangle, shapeB, transformAB, distanceThreshold);

			// 衝突点を追加
			for(PfxUInt32 j=0;j<localContacts.getNumContactPoints();j++) {
				PfxSubData subA = localContacts.getContactSubDataA(j);
				subA.setIslandId(i);

				PfxPoint3 wA = transformA * localContacts.getContactLocalPointA(j);
				PfxPoint3 wB = transformB * localContacts.getContactLocalPointB(j);
				PfxVector3 normal = normalize(rotationA_n * localContacts.getContactNormal(j));

				contacts.addContactPoint(
					dot(wA-wB,normal),
					normal,
					localContacts.getContactLocalPointA(j),
					localContacts.getContactLocalPointB(j),
					localContacts.getFeatureIdA(j),
					localContacts.getFeatureIdB(j),
					subA,PfxSubData());
			}
		}
	}

	//SCE_PFX_PRINTF("traversal %d/%d islands %d\n", n1, n2, lmeshA->m_numIslands);

	return contacts.getNumContactPoints();
}

template<typename PfxShapeType, typename PfxLargeTriMeshIslandType>
PfxInt32 pfxContactLargeTriMeshBvh(
	PfxContactCache &contacts,
	PfxBool flipTriangle,
	const PfxLargeTriMeshImpl *lmeshA,
	const PfxTransform3 &transformA,
	const PfxShapeType &shapeB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold)
{
	PfxTransform3 transformAB;
	PfxMatrix3 matrixAB;
	PfxVector3 offsetAB;
	PfxContactCache localContacts;

	// Bローカル→Aローカルへの変換
	transformAB = inverse(transformA) * transformB;
	matrixAB = transformAB.getUpper3x3();
	offsetAB = transformAB.getTranslation();

	PfxMatrix3 rotationA_n = transpose(inverse(transformA.getUpper3x3()));

	// -----------------------------------------------------
	// LargeTriMeshに含まれるTriMeshのAABBと凸体のAABBを判定し、
	// 交差するものを個別に衝突判定する。※LargeMesh座標系

	PfxVector3 shapeHalf(0.0f);
	PfxVector3 shapeCenter = offsetAB;

	shapeHalf = absPerElem(matrixAB) * getShapeExtend<PfxShapeType>(shapeB);

	// -----------------------------------------------------
	// アイランドとの衝突判定


	{
		PfxStaticStack<PfxEncStack> bvhStack;

		PfxEncStack encroot;
		encroot.nodeId = 0;
		encroot.aabbMin = lmeshA->getOffset() - lmeshA->getHalf();
		encroot.aabbMax = lmeshA->getOffset() + lmeshA->getHalf();

		bvhStack.push(encroot);

#ifdef PFX_ENABLE_AVX
		const __m128 vAabbMinB = sce_vectormath_asm128((shapeCenter - shapeHalf).get128());
		const __m128 vAabbMaxB = sce_vectormath_asm128((shapeCenter + shapeHalf).get128());
		const __m128 v255Inv = _mm_rcp_ps(_mm_set1_ps(255.0f));
#else
		PfxVector3 aabbMinB = shapeCenter - shapeHalf;
		PfxVector3 aabbMaxB = shapeCenter + shapeHalf;
#endif

		while (!bvhStack.empty()) {
			PfxEncStack info = bvhStack.top();
			bvhStack.pop();

			PfxIslandBvhNode &encnode = lmeshA->m_bvhNodes[info.nodeId];

#ifdef PFX_ENABLE_AVX
			__m128 vQuantizedMin = _mm_cvtepi32_ps(_mm_set_epi32(0, encnode.aabb[2], encnode.aabb[1], encnode.aabb[0]));
			__m128 vQuantizedMax = _mm_cvtepi32_ps(_mm_set_epi32(0, encnode.aabb[5], encnode.aabb[4], encnode.aabb[3]));
			PfxVector3 vtmp = mulPerElem(info.aabbMax - info.aabbMin, PfxVector3(sce_vectormath_asfloat4(v255Inv)));
			PfxVector3 aabbMinA = info.aabbMin + mulPerElem(vtmp, PfxVector3(sce_vectormath_asfloat4(vQuantizedMin)));
			PfxVector3 aabbMaxA = info.aabbMax - mulPerElem(vtmp, PfxVector3(sce_vectormath_asfloat4(vQuantizedMax)));
			const __m128 vAabbMinA = sce_vectormath_asm128(aabbMinA.get128());
			const __m128 vAabbMaxA = sce_vectormath_asm128(aabbMaxA.get128());
			if (((_mm_movemask_ps(_mm_cmplt_ps(vAabbMaxA, vAabbMinB)) | _mm_movemask_ps(_mm_cmpgt_ps(vAabbMinA, vAabbMaxB))) & 0x07) != 0) continue;
#else
			PfxVector3 quantizedMin(encnode.aabb[0], encnode.aabb[1], encnode.aabb[2]);
			PfxVector3 quantizedMax(encnode.aabb[3], encnode.aabb[4], encnode.aabb[5]);
			PfxVector3 aabbMinA = info.aabbMin + mulPerElem((info.aabbMax - info.aabbMin), quantizedMin / 255.0f);
			PfxVector3 aabbMaxA = info.aabbMax - mulPerElem((info.aabbMax - info.aabbMin), quantizedMax / 255.0f);
			if (aabbMaxA[0] < aabbMinB[0] || aabbMinA[0] > aabbMaxB[0]) continue;
			if (aabbMaxA[1] < aabbMinB[1] || aabbMinA[1] > aabbMaxB[1]) continue;
			if (aabbMaxA[2] < aabbMinB[2] || aabbMinA[2] > aabbMaxB[2]) continue;
#endif

			PfxEncStack encnext;
			encnext.aabbMin = aabbMinA;
			encnext.aabbMax = aabbMaxA;

			PfxUInt32 numSelIslands = 0;
			PfxUInt32 selIslands[2];

			PfxUInt8 LStatus = encnode.flag & 0x03;
			PfxUInt8 RStatus = (encnode.flag & 0x0C) >> 2;

			if (LStatus == 0) {
				encnext.nodeId = encnode.left;
				bvhStack.push(encnext);
			}
			else if (LStatus == 1) {
				selIslands[numSelIslands++] = encnode.left;
			}

			if (RStatus == 0) {
				encnext.nodeId = encnode.right;
				bvhStack.push(encnext);
			}
			else if (RStatus == 1) {
				selIslands[numSelIslands++] = encnode.right;
			}

			for (PfxUInt32 i = 0; i<numSelIslands; i++) {
				PfxUInt32 islandId = selIslands[i];

				// 衝突判定
				localContacts.reset();

				PfxLargeTriMeshIslandType *island = (PfxLargeTriMeshIslandType*)lmeshA->m_islands + islandId;

				pfxContactTriMesh(localContacts, lmeshA, island, flipTriangle, shapeB, transformAB, distanceThreshold);

				// 衝突点を追加
				for (PfxUInt32 j = 0; j<localContacts.getNumContactPoints(); j++) {
					PfxSubData subA = localContacts.getContactSubDataA(j);
					subA.setIslandId(islandId);

					PfxPoint3 wA = transformA * localContacts.getContactLocalPointA(j);
					PfxPoint3 wB = transformB * localContacts.getContactLocalPointB(j);
					PfxVector3 normal = normalize(rotationA_n * localContacts.getContactNormal(j));

					contacts.addContactPoint(
						dot(wA - wB, normal),
						normal,
						localContacts.getContactLocalPointA(j),
						localContacts.getContactLocalPointB(j),
						localContacts.getFeatureIdA(j),
						localContacts.getFeatureIdB(j),
						subA, PfxSubData());
				}
			}
		}
	}

	//SCE_PFX_PRINTF("traversal %d/%d islands %d\n", n1, n2, lmeshA->m_numIslands);

	return contacts.getNumContactPoints();
}

template PfxInt32 pfxContactLargeTriMesh<PfxSphere, PfxExpandedTriMesh>(PfxContactCache &contacts, PfxBool flipTriangle, const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxSphere &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMesh<PfxBox, PfxExpandedTriMesh>(PfxContactCache &contacts, PfxBool flipTriangle, const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxBox &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMesh<PfxCapsule, PfxExpandedTriMesh>(PfxContactCache &contacts, PfxBool flipTriangle, const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxCapsule &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMesh<PfxCylinder, PfxExpandedTriMesh>(PfxContactCache &contacts, PfxBool flipTriangle, const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxCylinder &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMesh<PfxConvexMeshImpl, PfxExpandedTriMesh>(PfxContactCache &contacts, PfxBool flipTriangle, const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxConvexMeshImpl &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMeshBvh<PfxSphere, PfxQuantizedTriMeshBvh>(PfxContactCache &contacts, PfxBool flipTriangle,	const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxSphere &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMeshBvh<PfxBox, PfxQuantizedTriMeshBvh>(PfxContactCache &contacts, PfxBool flipTriangle,	const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxBox &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMeshBvh<PfxCapsule, PfxQuantizedTriMeshBvh>(PfxContactCache &contacts, PfxBool flipTriangle,	const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxCapsule &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMeshBvh<PfxCylinder, PfxQuantizedTriMeshBvh>(PfxContactCache &contacts,	PfxBool flipTriangle,	const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxCylinder &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMeshBvh<PfxConvexMeshImpl, PfxQuantizedTriMeshBvh>(PfxContactCache &contacts,	PfxBool flipTriangle,	const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxConvexMeshImpl &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMeshBvh<PfxSphere, PfxCompressedTriMesh>(PfxContactCache &contacts,	PfxBool flipTriangle,	const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxSphere &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMeshBvh<PfxBox, PfxCompressedTriMesh>(PfxContactCache &contacts, PfxBool flipTriangle,	const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxBox &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMeshBvh<PfxCapsule, PfxCompressedTriMesh>(PfxContactCache &contacts, PfxBool flipTriangle,	const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxCapsule &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMeshBvh<PfxCylinder, PfxCompressedTriMesh>(PfxContactCache &contacts, PfxBool flipTriangle,	const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxCylinder &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);
template PfxInt32 pfxContactLargeTriMeshBvh<PfxConvexMeshImpl, PfxCompressedTriMesh>(PfxContactCache &contacts,	PfxBool flipTriangle,	const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxConvexMeshImpl &shapeB, const PfxTransform3 &transformB, PfxFloat distanceThreshold);

} //namespace pfxv4
} //namespace sce
