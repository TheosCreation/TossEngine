/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "pfx_closest_tri_mesh_sphere.h"
#include "pfx_closest_large_tri_mesh.h"
#include "pfx_intersect_common.h"
#include "pfx_mesh_common.h"

namespace sce {
namespace pfxv4 {

template<typename PfxShapeType, typename PfxLargeTriMeshIslandType>
PfxInt32 pfxClosestLargeTriMesh(
				PfxContactCache &contacts,
				PfxBool flipTriangle,
				const PfxLargeTriMeshImpl *lmeshA,
				const PfxTransform3 &transformA,
				const PfxShapeType &sphereB,
				const PfxTransform3 &transformB0,
				const PfxTransform3 &transformB1,
				PfxFloat distanceThreshold)
{
	// Bローカル→Aローカルへの変換
	PfxTransform3 transfromAInv = inverse(transformA);
	PfxTransform3 transformAB0 = transfromAInv * transformB0;
	PfxTransform3 transformAB1 = transfromAInv * transformB1;

	PfxMatrix3 rotationB_n = transpose(inverse(transformB0.getUpper3x3()));

	// -----------------------------------------------------
	// LargeTriMeshに含まれるTriMeshのAABBと凸体のAABBを判定し、
	// 交差するものを個別に衝突判定する。※LargeMesh座標系

	const PfxFloat boudingRadius = length(absPerElem(transformAB0.getUpper3x3()) * PfxVector3(sphereB.m_radius));

	PfxFloat lenSqr = lengthSqr(transformAB1.getTranslation() - transformAB0.getTranslation());
	PfxFloat len = lenSqr < 0.00001f ? 0.0f : sqrtf(lenSqr);
	PfxFloat shapeRadius = boudingRadius + 0.5f*len;
	PfxVector3 shapeCenter = 0.5f * (transformAB0.getTranslation() + transformAB1.getTranslation());
	PfxVector3 aabbMinB = shapeCenter - PfxVector3(shapeRadius);
	PfxVector3 aabbMaxB = shapeCenter + PfxVector3(shapeRadius);

	// -----------------------------------------------------
	// アイランドとの衝突判定

	PfxVecInt3 aabbMinL,aabbMaxL;
	lmeshA->getLocalPosition(aabbMinB,aabbMaxB,aabbMinL,aabbMaxL);

	PfxAabb16 aabbB;

	pfxSetXMin(aabbB,aabbMinL.getX());
	pfxSetXMax(aabbB,aabbMaxL.getX());
	pfxSetYMin(aabbB,aabbMinL.getY());
	pfxSetYMax(aabbB,aabbMaxL.getY());
	pfxSetZMin(aabbB,aabbMinL.getZ());
	pfxSetZMax(aabbB,aabbMaxL.getZ());

	PfxUInt32 numIslands = lmeshA->m_numIslands;

	PfxContactCache localContacts;

	{
		for(PfxUInt32 i=0;i<numIslands;i++) {
			// AABBチェック
			PfxAabb16 aabbA = lmeshA->m_aabbList[i];
			if(pfxGetXMax(aabbA) < pfxGetXMin(aabbB) || pfxGetXMin(aabbA) > pfxGetXMax(aabbB)) continue;
			if(pfxGetYMax(aabbA) < pfxGetYMin(aabbB) || pfxGetYMin(aabbA) > pfxGetYMax(aabbB)) continue;
			if(pfxGetZMax(aabbA) < pfxGetZMin(aabbB) || pfxGetZMin(aabbA) > pfxGetZMax(aabbB)) continue;

			PfxLargeTriMeshIslandType *island = (PfxLargeTriMeshIslandType*)lmeshA->m_islands + i;

			// 衝突判定
			localContacts.reset();

			// Return closest points in the sphere coordinates.
			pfxClosestTriMesh(localContacts,lmeshA,island,flipTriangle,sphereB, transformAB0, transformAB1, shapeCenter, shapeRadius, distanceThreshold);

			// 衝突点を追加
			for(int j=0;j<localContacts.getNumClosestPoints();j++) {
				PfxSubData subData = localContacts.getClosestSubDataA(j);
				subData.setIslandId(i);

				PfxPoint3 wA = transformB0 * localContacts.getClosestLocalPointA(j);
				PfxPoint3 wB = transformB0 * localContacts.getClosestLocalPointB(j);
				PfxVector3 normal = normalize(rotationB_n * localContacts.getClosestNormal(j));

				contacts.addClosestPoint(
					dot(wA-wB,normal),
					normal,
					transformAB0 * localContacts.getClosestLocalPointA(j),
					localContacts.getClosestLocalPointB(j),
					subData,PfxSubData());
			}
		}
	}

	return contacts.getNumClosestPoints();
}

template<typename PfxShapeType, typename PfxLargeTriMeshIslandType>
PfxInt32 pfxClosestLargeTriMeshBvh(
				PfxContactCache &contacts,
				PfxBool flipTriangle,
				const PfxLargeTriMeshImpl *lmeshA,
				const PfxTransform3 &transformA,
				const PfxShapeType &sphereB,
				const PfxTransform3 &transformB0,
				const PfxTransform3 &transformB1,
				PfxFloat distanceThreshold)
{
	// Bローカル→Aローカルへの変換
	PfxTransform3 transfromAInv = inverse(transformA);
	PfxTransform3 transformAB0 = transfromAInv * transformB0;
	PfxTransform3 transformAB1 = transfromAInv * transformB1;

	PfxMatrix3 rotationB_n = transpose(inverse(transformB0.getUpper3x3()));

	// -----------------------------------------------------
	// LargeTriMeshに含まれるTriMeshのAABBと凸体のAABBを判定し、
	// 交差するものを個別に衝突判定する。※LargeMesh座標系

	const PfxFloat boudingRadius = length(absPerElem(transformAB0.getUpper3x3()) * PfxVector3(sphereB.m_radius));

	PfxFloat lenSqr = lengthSqr(transformAB1.getTranslation() - transformAB0.getTranslation());
	PfxFloat len = lenSqr < 0.00001f ? 0.0f : sqrtf(lenSqr);
	PfxFloat shapeRadius = boudingRadius + 0.5f*len;
	PfxVector3 shapeCenter = 0.5f * (transformAB0.getTranslation() + transformAB1.getTranslation());
	PfxVector3 aabbMinB = shapeCenter - PfxVector3(shapeRadius);
	PfxVector3 aabbMaxB = shapeCenter + PfxVector3(shapeRadius);

	// -----------------------------------------------------
	// アイランドとの衝突判定

	PfxContactCache localContacts;

	{
		PfxStaticStack<PfxEncStack> bvhStack;

		PfxEncStack encroot;
		encroot.nodeId = 0;
		encroot.aabbMin = lmeshA->getOffset() - lmeshA->getHalf();
		encroot.aabbMax = lmeshA->getOffset() + lmeshA->getHalf();

		bvhStack.push(encroot);

		while(!bvhStack.empty()) {
			PfxEncStack info = bvhStack.top();
			bvhStack.pop();

			PfxIslandBvhNode &encnode = lmeshA->m_bvhNodes[info.nodeId];

			PfxVector3 quantizedMin(encnode.aabb[0],encnode.aabb[1],encnode.aabb[2]);
			PfxVector3 quantizedMax(encnode.aabb[3],encnode.aabb[4],encnode.aabb[5]);
			PfxVector3 aabbMinA = info.aabbMin + mulPerElem((info.aabbMax-info.aabbMin),quantizedMin / 255.0f);
			PfxVector3 aabbMaxA = info.aabbMax - mulPerElem((info.aabbMax-info.aabbMin),quantizedMax / 255.0f);

			if(aabbMaxA[0] < aabbMinB[0] || aabbMinA[0] > aabbMaxB[0]) continue;
			if(aabbMaxA[1] < aabbMinB[1] || aabbMinA[1] > aabbMaxB[1]) continue;
			if(aabbMaxA[2] < aabbMinB[2] || aabbMinA[2] > aabbMaxB[2]) continue;

			PfxEncStack encnext;
			encnext.aabbMin = aabbMinA;
			encnext.aabbMax = aabbMaxA;

			PfxUInt32 numSelIslands=0;
			PfxUInt32 selIslands[2];

			PfxUInt8 LStatus = encnode.flag & 0x03;
			PfxUInt8 RStatus = (encnode.flag & 0x0C) >> 2;

			if(LStatus == 0) {
				encnext.nodeId = encnode.left;
				bvhStack.push(encnext);
			}
			else if(LStatus == 1) {
				selIslands[numSelIslands++] = encnode.left;
			}

			if(RStatus == 0) {
				encnext.nodeId = encnode.right;
				bvhStack.push(encnext);
			}
			else if(RStatus == 1) {
				selIslands[numSelIslands++] = encnode.right;
			}

			for(PfxUInt32 i=0;i<numSelIslands;i++) {
				PfxUInt32 islandId = selIslands[i];

				// 衝突判定
				localContacts.reset();

				// Return closest points in the sphere coordinates.
				PfxLargeTriMeshIslandType *island = (PfxLargeTriMeshIslandType*)lmeshA->m_islands + islandId;
				pfxClosestTriMesh(localContacts,lmeshA,island,flipTriangle,
					sphereB, transformAB0, transformAB1, shapeCenter, shapeRadius, distanceThreshold);

				// 衝突点を追加
				for(int j=0;j<localContacts.getNumClosestPoints();j++) {
					PfxSubData subData = localContacts.getClosestSubDataA(j);
					subData.setIslandId(islandId);

					PfxPoint3 wA = transformB0 * localContacts.getClosestLocalPointA(j);
					PfxPoint3 wB = transformB0 * localContacts.getClosestLocalPointB(j);
					PfxVector3 normal = normalize(rotationB_n * localContacts.getClosestNormal(j));

					contacts.addClosestPoint(
						dot(wA-wB,normal),
						normal,
						transformAB0 * localContacts.getClosestLocalPointA(j),
						localContacts.getClosestLocalPointB(j),
						subData,PfxSubData());
				}
			}
		}
	}

	return contacts.getNumClosestPoints();
}

template PfxInt32 pfxClosestLargeTriMesh<PfxSphere, PfxExpandedTriMesh>(PfxContactCache &contacts, PfxBool flipTriangle, const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxSphere &sphereB, const PfxTransform3 &transformB0, const PfxTransform3 &transformB1, PfxFloat distanceThreshold);
template PfxInt32 pfxClosestLargeTriMeshBvh<PfxSphere, PfxQuantizedTriMeshBvh>(PfxContactCache &contacts, PfxBool flipTriangle, const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxSphere &sphereB, const PfxTransform3 &transformB0, const PfxTransform3 &transformB1, PfxFloat distanceThreshold);
template PfxInt32 pfxClosestLargeTriMeshBvh<PfxSphere, PfxCompressedTriMesh>(PfxContactCache &contacts, PfxBool flipTriangle, const PfxLargeTriMeshImpl *lmeshA, const PfxTransform3 &transformA, const PfxSphere &sphereB, const PfxTransform3 &transformB0, const PfxTransform3 &transformB1, PfxFloat distanceThreshold);

} //namespace pfxv4
} //namespace sce
