/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "pfx_closest_tri_mesh_sphere.h"
#include "pfx_intersect_common.h"
#include "pfx_mesh_common.h"
#include "pfx_decoded_tri_mesh.h"

namespace sce {
namespace pfxv4 {

static void pfxDistanceTriangleSphere(
	PfxFloat &closestDistance,PfxVector3 &closestNormal,PfxPoint3 &closestPointA,PfxPoint3 &closestPointB,
	const PfxVector3 &normal,const PfxVector3 &p0,const PfxVector3 &p1,const PfxVector3 &p2,
	PfxUInt32 edgeChk,PfxFloat sphereRadius)
{
	const PfxFloat epsilon = 0.00001f;
	const PfxFloat epsilonSqr = epsilon * epsilon;

	closestDistance = SCE_PFX_FLT_MAX;

	// 最も浅い貫通深度とそのときの分離軸
	PfxFloat distMin = -SCE_PFX_FLT_MAX;
	PfxVector3 axisMin(0.0f),pA(0.0f),pB(0.0f);

	// 分離軸判定
	{
		PfxVector3 facetPnts[3] = {p0,p1,p2};

		// トライアングル法線
		{
			const PfxVector3 &sepAxis = normal; // 表

			// 分離平面
			//PfxPlane plane(sepAxis,(facetPnts[0]+facetPnts[1]+facetPnts[2])/3.0f);

			// Sphere(B)を分離軸に投影して範囲を取得
			//	PfxFloat test = plane.onPlane(spherePos);
			PfxFloat test = dot(-(facetPnts[0]+facetPnts[1]+facetPnts[2])/3.0f,sepAxis);
			PfxFloat BMin = test - sphereRadius;

			if(BMin > distMin) {
				distMin = BMin;
				axisMin = -sepAxis;
				pA = test * axisMin;
				pB = sphereRadius * axisMin;
			}
		}

		// トライアングル頂点
		for(int i=0;i<3;i++) {
			PfxFloat chk = lengthSqr(-facetPnts[i]);
			if(chk < epsilonSqr) continue;

			PfxVector3 sepAxis = (-facetPnts[i]) * rsqrtf(PfxFloatInVec(chk));
			if(dot(normal,sepAxis) < 0.0f) sepAxis = -sepAxis;

			// Triangleを分離軸に投影
			PfxFloat AMin=SCE_PFX_FLT_MAX,AMax=-SCE_PFX_FLT_MAX;
			pfxGetProjAxisPnts3(facetPnts,sepAxis,AMin,AMax);

			// 球を分離軸に投影
			PfxFloat BMin = -sphereRadius;

			PfxFloat d = BMin - AMax;
			if(d > distMin) {
				distMin = d;
				axisMin = -sepAxis;
				pA = facetPnts[i];
				pB = sphereRadius * axisMin;
			}
		}

		// トライアングルエッジ
		for(int e=0;e<3;e++) {
			PfxVector3 ab = facetPnts[(e+1)%3]-facetPnts[e];
			PfxVector3 q = facetPnts[e] + (dot(-facetPnts[e],ab) / dot(ab,ab)) * ab;

			PfxFloat chk = lengthSqr(-q);
			if(chk < epsilonSqr) continue;

			PfxVector3 sepAxis = (-q) * rsqrtf(PfxFloatInVec(chk));
			if(dot(normal,sepAxis) < 0.0f) sepAxis = -sepAxis;

			// Triangleを分離軸に投影
			PfxFloat AMin=SCE_PFX_FLT_MAX,AMax=-SCE_PFX_FLT_MAX;
			pfxGetProjAxisPnts3(facetPnts,sepAxis,AMin,AMax);

			// 球を分離軸に投影
			PfxFloat BMin = -sphereRadius;

			PfxFloat d = BMin - AMax;
			if(d > distMin) {
				distMin = d;
				axisMin = -sepAxis;
				pA = q;
				pB = sphereRadius * axisMin;
			}
		}
	}

	closestPointA = PfxPoint3(pA);
	closestPointB = PfxPoint3(pB);
	closestDistance = distMin;
	closestNormal = axisMin;
}

template<>
PfxInt32 pfxClosestTriMesh<PfxSphere, PfxExpandedTriMesh> (
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA,const PfxExpandedTriMesh *meshA,bool flipTriangle,
	const PfxSphere &sphereB,
	const PfxTransform3 &transformB0, const PfxTransform3 &transformB1,
	const PfxVector3 &bsphereCenter,PfxFloat bsphereRadius,
	PfxFloat distanceThreshold )
{
	(void) largeMeshA;
	(void) distanceThreshold;

	PfxTransform3 transformBA = inverse(transformB0);
	PfxMatrix3 rotationBA_n = transpose(inverse(transformBA.getUpper3x3()));

	//-------------------------------------------
	// 判定する面を絞り込む

	PfxVector3 aabbCenter = bsphereCenter;
	PfxVector3 aabbExtent(bsphereRadius);

	PfxUInt8 selFacets[SCE_PFX_NUMMESHFACETS] = {0};
	PfxUInt32 numSelFacets = pfxGatherFacets(meshA,aabbCenter,aabbExtent,selFacets);

	if(numSelFacets == 0) {
		return 0;
	}

	//-----------------------------------------------
	// 判定

	const PfxFloat bsphereRadiusSqr = lengthSqr(absPerElem(transformBA.getUpper3x3()) * PfxVector3(bsphereRadius));
	const PfxPoint3 bsphereCenterB = transformBA * PfxPoint3(bsphereCenter);

	int vi[3] = {0,1,2};
	int ei[3] = {0,1,2};
	if(flipTriangle) {vi[0]=2;vi[2]=0;ei[0]=1;ei[1]=0;}

	// TriangleMeshの面->sphereの判定
	// In a sphere coordinate
	{
		for(PfxUInt32 f = 0; f < numSelFacets; f++ ) {
			const PfxExpandedFacet &facet = meshA->m_facets[selFacets[f]];

			const PfxVector3 facetNormal = normalize(rotationBA_n * pfxReadVector3((PfxFloat*)&facet.m_normal));

			const PfxVector3 facetPnts[3] = {
				transformBA.getTranslation() + transformBA.getUpper3x3() * meshA->m_verts[facet.m_vertIds[vi[0]]],
				transformBA.getTranslation() + transformBA.getUpper3x3() * meshA->m_verts[facet.m_vertIds[vi[1]]],
				transformBA.getTranslation() + transformBA.getUpper3x3() * meshA->m_verts[facet.m_vertIds[vi[2]]],
			};

			const PfxEdge *edge[3] = {
				&meshA->m_edges[facet.m_edgeIds[ei[0]]],
				&meshA->m_edges[facet.m_edgeIds[ei[1]]],
				&meshA->m_edges[facet.m_edgeIds[ei[2]]],
			};

			PfxFloat distance;
			PfxVector3 normal;
			PfxPoint3 pointA,pointB;

			PfxUInt32 edgeChk = edge[0]->m_angleType | (edge[1]->m_angleType << 2) | (edge[2]->m_angleType << 4);

			pfxDistanceTriangleSphere(distance,normal,pointA,pointB,
									facetNormal,facetPnts[0],facetPnts[1],facetPnts[2],
									edgeChk,sphereB.m_radius);

			if(distance > -1e-4f && dot(facetNormal,normal) < 0.25f && lengthSqr(pointA-bsphereCenterB) < bsphereRadiusSqr) {
				PfxSubData subData;
				PfxFloat s=0.0f,t=0.0f;
				pfxCalcBarycentricCoords(PfxVector3(pointA),PfxTriangle(facetPnts[0],facetPnts[1],facetPnts[2]),s,t);
				subData.setType(PfxSubData::MESH_INFO);
				subData.setFacetId(selFacets[f]);
				subData.setFacetLocalS(s);
				subData.setFacetLocalT(t);
				subData.setUserData(facet.m_userData);
				
				PfxUInt8 ccdPriority = 0;
				
				// Check if this triangle is in a range of a moving-sphere line
				PfxFloat distanceToFacet;
				PfxVector3 startPos = transformBA.getTranslation() + transformBA.getUpper3x3() * transformB0.getTranslation();
				PfxVector3 endPos = transformBA.getTranslation() + transformBA.getUpper3x3() * transformB1.getTranslation();
				PfxTriangle facetTri(facetPnts[0], facetPnts[1], facetPnts[2]);
				if (pfxDistanceTriangleSegment(startPos, endPos - startPos, facetTri, distanceToFacet) && distanceToFacet < sphereB.m_radius) {
					ccdPriority = 1 << 3;
				}

				// Check an angle between the sphere direction and the closest direction (normal)
				PfxVector3 sphereDirection = normalize(endPos - startPos);
				PfxFloat dirAngle = SCE_PFX_MAX(dot(normal, sphereDirection), 0.0f);
				PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
				ccdPriority = ccdPriority | priority;
				subData.setCcdPriority(ccdPriority);

				contacts.addClosestPoint(
					distance,
					normal,
					pointA,
					pointB,
					subData,PfxSubData());
			}
		}
	}

	return contacts.getNumClosestPoints();
}

template<>
PfxInt32 pfxClosestTriMesh<PfxSphere, PfxQuantizedTriMeshBvh> (
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA,const PfxQuantizedTriMeshBvh *meshA,bool flipTriangle,
	const PfxSphere &sphereB,
	const PfxTransform3 &transformB0, const PfxTransform3 &transformB1,
	const PfxVector3 &bsphereCenter,PfxFloat bsphereRadius,
	PfxFloat distanceThreshold )
{
	(void) distanceThreshold;

	PfxTransform3 transformBA = inverse(transformB0);
	PfxMatrix3 rotationBA_n = transpose(inverse(transformBA.getUpper3x3()));

	//	operate back-tracking through Largemsh's BV-tree
	PfxVector3 aabbMinB(bsphereCenter - PfxVector3(bsphereRadius));
	PfxVector3 aabbMaxB(bsphereCenter + PfxVector3(bsphereRadius));
	PfxUInt8 selFacets[SCE_PFX_NUMMESHFACETS] = {0};
	PfxUInt32 numSelFacets = pfxGatherFacets(meshA,aabbMinB,aabbMaxB,selFacets);
	if(numSelFacets == 0) return 0;

	//-------------------------------------------
	//	operating "separating hyperplane theorem"

	PfxDecodedTriMesh decodedMesh;

	int vi[3] = {0,1,2};
	int ei[3] = {0,1,2};
	if(flipTriangle) {vi[0]=2;vi[2]=0;ei[0]=1;ei[1]=0;}

	const PfxFloat bsphereRadiusSqr = lengthSqr(absPerElem(transformBA.getUpper3x3()) * PfxVector3(bsphereRadius));
	const PfxPoint3 bsphereCenterB = transformBA * PfxPoint3(bsphereCenter);

	// TriangleMeshの面->sphereの判定
	// In a sphere coordinate
	{
		for(PfxUInt32 f = 0; f < numSelFacets; f++ ) {
			const PfxQuantizedFacetBvh &facet = meshA->m_facets[selFacets[f]];

			// デコード
			PfxDecodedFacet decodedFacet;
			const PfxUInt32 vId[3] = {facet.m_vertIds[vi[0]],facet.m_vertIds[vi[1]],facet.m_vertIds[vi[2]]};

			decodedFacet.m_normal = normalize(rotationBA_n * largeMeshA->decodeNormal(facet.m_normal));
			decodedFacet.m_thickness = largeMeshA->decodeFloat(facet.m_thickness);

			for(int v=0;v<3;v++) {
				if(!decodedMesh.isDecodedVertex(vId[v])) {
					PfxVector3 vert = largeMeshA->decodePosition(meshA->m_verts[vId[v]]);
					decodedMesh.m_verts[vId[v]] = transformBA.getTranslation() + transformBA.getUpper3x3() * vert;
					decodedMesh.m_decodedVertex[vId[v]>>5] |= 1 << (vId[v]&0x1f);
				}
			}

			const PfxVector3 facetPnts[3] = {
				decodedMesh.m_verts[vId[0]],
				decodedMesh.m_verts[vId[1]],
				decodedMesh.m_verts[vId[2]],
			};

			const PfxEdge *edge[3] = {
				&meshA->m_edges[facet.m_edgeIds[ei[0]]],
				&meshA->m_edges[facet.m_edgeIds[ei[1]]],
				&meshA->m_edges[facet.m_edgeIds[ei[2]]],
			};

			PfxFloat distance;
			PfxVector3 normal;
			PfxPoint3 pointA,pointB;

			PfxUInt32 edgeChk = edge[0]->m_angleType | (edge[1]->m_angleType << 2) | (edge[2]->m_angleType << 4);

			pfxDistanceTriangleSphere(distance,normal,pointA,pointB,
									decodedFacet.m_normal,facetPnts[0],facetPnts[1],facetPnts[2],
									edgeChk,sphereB.m_radius);

			if(distance > -1e-4f && dot(decodedFacet.m_normal,normal) < 0.25f && lengthSqr(pointA-bsphereCenterB) < bsphereRadiusSqr) {
				PfxSubData subData;
				PfxFloat s=0.0f,t=0.0f;
				pfxCalcBarycentricCoords(PfxVector3(pointA),PfxTriangle(facetPnts[0],facetPnts[1],facetPnts[2]),s,t);
				subData.setType(PfxSubData::MESH_INFO);
				subData.setFacetId(selFacets[f]);
				subData.setFacetLocalS(s);
				subData.setFacetLocalT(t);
				subData.setUserData(facet.m_userData);

				PfxUInt8 ccdPriority = 0;

				// Check if this triangle is in a range of a moving-sphere line
				PfxFloat distanceToFacet;
				PfxVector3 startPos = transformBA.getTranslation() + transformBA.getUpper3x3() * transformB0.getTranslation();
				PfxVector3 endPos = transformBA.getTranslation() + transformBA.getUpper3x3() * transformB1.getTranslation();
				PfxTriangle facetTri(facetPnts[0], facetPnts[1], facetPnts[2]);
				if (pfxDistanceTriangleSegment(startPos, endPos - startPos, facetTri, distanceToFacet) && distanceToFacet < sphereB.m_radius) {
					ccdPriority = 1 << 3;
				}

				// Check an angle between the sphere direction and the closest direction (normal)
				PfxVector3 sphereDirection = normalize(endPos - startPos);
				PfxFloat dirAngle = SCE_PFX_MAX(dot(normal, sphereDirection), 0.0f);
				PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
				ccdPriority = ccdPriority | priority;
				subData.setCcdPriority(ccdPriority);

				contacts.addClosestPoint(
					distance,
					normal,
					pointA,
					pointB,
					subData,PfxSubData());
			}
		}
	}

	return contacts.getNumClosestPoints();
}

template<>
PfxInt32 pfxClosestTriMesh<PfxSphere, PfxCompressedTriMesh> (
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA,const PfxCompressedTriMesh *meshA,bool flipTriangle,
	const PfxSphere &sphereB,
	const PfxTransform3 &transformB0, const PfxTransform3 &transformB1,
	const PfxVector3 &bsphereCenter,PfxFloat bsphereRadius,
	PfxFloat distanceThreshold )
{
	(void) distanceThreshold;

	PfxTransform3 transformBA = inverse(transformB0);

	//	operate back-tracking through Largemsh's BV-tree
	PfxVector3 aabbMinB(bsphereCenter - PfxVector3(bsphereRadius));
	PfxVector3 aabbMaxB(bsphereCenter + PfxVector3(bsphereRadius));
	PfxUInt8 selFacets[SCE_PFX_NUMMESHFACETS] = {0};
	PfxUInt32 numSelFacets = pfxGatherFacets(meshA,(PfxFacetBvhNode*)largeMeshA->m_bvhNodeBuffer,aabbMinB,aabbMaxB,selFacets);
	if(numSelFacets == 0) return 0;

	//-------------------------------------------
	//	operating "separating hyperplane theorem"

	PfxDecodedTriMesh decodedMesh;

	const PfxFloat bsphereRadiusSqr = lengthSqr(absPerElem(transformBA.getUpper3x3()) * PfxVector3(bsphereRadius));
	const PfxPoint3 bsphereCenterB = transformBA * PfxPoint3(bsphereCenter);

	int vi[4] = {0,1,2,3};
	if(flipTriangle) {vi[0]=2;vi[2]=0;}

	// TriangleMeshの面->sphereの判定
	// In a sphere coordinate
	{
		for(PfxUInt32 f = 0; f < numSelFacets; f++ ) {
			const PfxCompressedFacet2 &facet = ((PfxCompressedFacet2*)largeMeshA->m_facetBuffer)[meshA->m_facets + selFacets[f]];

			const PfxUInt32 vId[4] = {facet.m_vertIds[vi[0]],facet.m_vertIds[vi[1]],facet.m_vertIds[vi[2]],facet.m_vertIds[vi[3]]};

			for(int v=0;v<4;v++) {
				if(!decodedMesh.isDecodedVertex(vId[v])) {
					PfxVector3 vert = largeMeshA->decodePosition(*((PfxQuantize3*)largeMeshA->m_vertexBuffer + meshA->m_verts + vId[v]));
					decodedMesh.m_verts[vId[v]] = transformBA.getTranslation() + transformBA.getUpper3x3() * vert;
					decodedMesh.m_decodedVertex[vId[v]>>5] |= 1 << (vId[v]&0x1f);
				}
			}

			PfxVector3 facetPnts[4] = {
				decodedMesh.m_verts[vId[0]],
				decodedMesh.m_verts[vId[1]],
				decodedMesh.m_verts[vId[2]],
				decodedMesh.m_verts[vId[3]],
			};

			if(facet.m_facetInfo & 0x8000) {
				// double facets
				PfxVector3 facetNormal0 = normalize(cross(facetPnts[1]-facetPnts[0],facetPnts[2]-facetPnts[0]));
				PfxVector3 facetNormal1 = normalize(cross(facetPnts[3]-facetPnts[2],facetPnts[0]-facetPnts[2]));

				PfxUInt32 edgeChk0 = facet.m_edgeInfo & 0x3f;
				PfxUInt32 edgeChk1 = facet.m_edgeInfo>>4;

				if(flipTriangle) {
					edgeChk0 = ((edgeChk0>>2)&0x0c) | ((edgeChk0<<2)&0x30) | (edgeChk0&0x03);
					edgeChk1 = ((edgeChk1>>2)&0x0c) | ((edgeChk1<<2)&0x30) | (edgeChk1&0x03);
				}

				PfxVector3 edgeAngle(0.0f);

				//PfxUInt8 facetId0 = selFacets[f];
				//PfxUInt8 facetId1 = selFacets[f] | 0x80;

				PfxFloat distance[2];
				PfxVector3 normal[2];
				PfxPoint3 pointA[2],pointB[2];

				pfxDistanceTriangleSphere(distance[0],normal[0],pointA[0],pointB[0],
										facetNormal0,facetPnts[0],facetPnts[1],facetPnts[2],
										edgeChk0,sphereB.m_radius);

				pfxDistanceTriangleSphere(distance[1],normal[1],pointA[1],pointB[1],
										facetNormal1,facetPnts[0],facetPnts[2],facetPnts[3],
										edgeChk1,sphereB.m_radius);

				if(distance[0] > -1e-4f && dot(facetNormal0,normal[0]) < 0.25f && lengthSqr(pointA[0]-bsphereCenterB) < bsphereRadiusSqr) {
					PfxSubData subData;
					PfxFloat s=0.0f,t=0.0f;
					pfxCalcBarycentricCoords(PfxVector3(pointA[0]),PfxTriangle(facetPnts[0],facetPnts[1],facetPnts[2]),s,t);
					subData.setType(PfxSubData::MESH_INFO);
					subData.setFacetId(facet.m_facetInfo&0xFF);
					subData.setFacetLocalS(s);
					subData.setFacetLocalT(t);
					subData.setUserData(facet.m_userData[0]);
					
					PfxUInt8 ccdPriority = 0;
					
					// Check if this triangle is in a range of a moving-sphere line
					PfxFloat distanceToFacet;
					PfxVector3 startPos = transformBA.getTranslation() + transformBA.getUpper3x3() * transformB0.getTranslation();
					PfxVector3 endPos = transformBA.getTranslation() + transformBA.getUpper3x3() * transformB1.getTranslation();
					PfxTriangle facetTri(facetPnts[0], facetPnts[1], facetPnts[2]);
					if (pfxDistanceTriangleSegment(startPos, endPos - startPos, facetTri, distanceToFacet) && distanceToFacet < sphereB.m_radius) {
						ccdPriority = 1 << 3;
					}

					// Check an angle between the sphere direction and the closest direction (normal)
					PfxVector3 sphereDirection = normalize(endPos - startPos);
					PfxFloat dirAngle = SCE_PFX_MAX(dot(normal[0], sphereDirection), 0.0f);
					PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
					ccdPriority = ccdPriority | priority;
					subData.setCcdPriority(ccdPriority);

					contacts.addClosestPoint(distance[0],normal[0],pointA[0],pointB[0],subData,PfxSubData());
				}

				if(distance[1] > -sphereB.m_radius && dot(facetNormal1,normal[1]) < 0.25f && lengthSqr(pointA[1]-bsphereCenterB) < bsphereRadiusSqr) {
					PfxSubData subData;
					PfxFloat s=0.0f,t=0.0f;
					pfxCalcBarycentricCoords(PfxVector3(pointA[1]),PfxTriangle(facetPnts[0],facetPnts[2],facetPnts[3]),s,t);
					subData.setType(PfxSubData::MESH_INFO);
					subData.setFacetId((facet.m_facetInfo&0xFF)+1);
					subData.setFacetLocalS(s);
					subData.setFacetLocalT(t);
					subData.setUserData(facet.m_userData[1]);
					
					PfxUInt8 ccdPriority = 0;
					
					// Check if this triangle is in a range of a moving-sphere line
					PfxFloat distanceToFacet;
					PfxVector3 startPos = transformBA.getTranslation() + transformBA.getUpper3x3() * transformB0.getTranslation();
					PfxVector3 endPos = transformBA.getTranslation() + transformBA.getUpper3x3() * transformB1.getTranslation();
					PfxTriangle facetTri(facetPnts[0],facetPnts[2],facetPnts[3]);
					if (pfxDistanceTriangleSegment(startPos, endPos - startPos, facetTri, distanceToFacet) && distanceToFacet < sphereB.m_radius) {
						ccdPriority = 1 << 3;
					}

					// Check an angle between the sphere direction and the closest direction (normal)
					PfxVector3 sphereDirection = normalize(endPos - startPos);
					PfxFloat dirAngle = SCE_PFX_MAX(dot(normal[1], sphereDirection), 0.0f);
					PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
					ccdPriority = ccdPriority | priority;
					subData.setCcdPriority(ccdPriority);

					contacts.addClosestPoint(distance[1],normal[1],pointA[1],pointB[1],subData,PfxSubData());
				}
			}
			else {
				// signle facet
				PfxVector3 facetNormal = normalize(cross(facetPnts[1]-facetPnts[0],facetPnts[2]-facetPnts[0]));

				PfxUInt32 edgeChk = facet.m_edgeInfo & 0x3f;

				if(flipTriangle) {
					edgeChk = ((edgeChk>>2)&0x0c) | ((edgeChk<<2)&0x30) | (edgeChk&0x03);
				}

				PfxFloat distance;
				PfxVector3 normal;
				PfxPoint3 pointA,pointB;

				pfxDistanceTriangleSphere(distance,normal,pointA,pointB,
										facetNormal,facetPnts[0],facetPnts[1],facetPnts[2],
										edgeChk,sphereB.m_radius);

				if(distance > -1e-4f && dot(facetNormal,normal) < 0.25f && lengthSqr(pointA-bsphereCenterB) < bsphereRadiusSqr) {
					PfxSubData subData;
					PfxFloat s=0.0f,t=0.0f;
					pfxCalcBarycentricCoords(PfxVector3(pointA),PfxTriangle(facetPnts[0],facetPnts[1],facetPnts[2]),s,t);
					subData.setType(PfxSubData::MESH_INFO);
					subData.setFacetId(facet.m_facetInfo&0xFF);
					subData.setFacetLocalS(s);
					subData.setFacetLocalT(t);
					subData.setUserData(facet.m_userData[0]);
					
					PfxUInt8 ccdPriority = 0;
					
					// Check if this triangle is in a range of a moving-sphere line
					PfxFloat distanceToFacet;
					PfxVector3 startPos = transformBA.getTranslation() + transformBA.getUpper3x3() * transformB0.getTranslation();
					PfxVector3 endPos = transformBA.getTranslation() + transformBA.getUpper3x3() * transformB1.getTranslation();
					PfxTriangle facetTri(facetPnts[0], facetPnts[1], facetPnts[2]);
					if (pfxDistanceTriangleSegment(startPos, endPos - startPos, facetTri, distanceToFacet) && distanceToFacet < sphereB.m_radius) {
						ccdPriority = 1 << 3;
					}
					
					// Check an angle between the sphere direction and the closest direction (normal)
					PfxVector3 sphereDirection = normalize(endPos - startPos);
					PfxFloat dirAngle = SCE_PFX_MAX(dot(normal, sphereDirection), 0.0f);
					PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
					ccdPriority = ccdPriority | priority;
					subData.setCcdPriority(ccdPriority);
					
					contacts.addClosestPoint(distance,normal,pointA,pointB,subData,PfxSubData());
				}
			}
		}
	}

	return contacts.getNumClosestPoints();
}

} //namespace pfxv4
} //namespace sce
