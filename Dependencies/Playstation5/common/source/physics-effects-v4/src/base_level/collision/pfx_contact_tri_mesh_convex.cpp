/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "pfx_contact_tri_mesh_convex.h"
#include "pfx_intersect_common.h"
#include "pfx_mesh_common.h"
#include "pfx_decoded_tri_mesh.h"
#include "pfx_gjk_solver.h"
#include "pfx_gjk_support_func.h"

// Convex vs Large TriMesh
// pfxContactTriangleConvex is called in the local space of a large mesh
// So it doesn't need to flip triangle's index order

namespace sce {
namespace pfxv4 {

static bool pfxContactTriangleConvex(PfxContactCache &contacts,PfxUInt32 facetId,
							const PfxVector3 &normal,const PfxVector3 &p0,const PfxVector3 &p1,const PfxVector3 &p2,
							const PfxFloat thickness,const PfxFloat angle0,const PfxFloat angle1,const PfxFloat angle2,
							PfxUInt32 edgeChk,
							const PfxConvexMeshImpl &convex,const PfxTransform3 &transformB)
{
	// 早期チェック + 裏側の誤判定を防ぐ
	PfxVector3 sA(0.0f);
	getSupportVertex(&convex, normal, sA, 0.0f);
	PfxFloat projA = dot(sA, normal);
	PfxFloat projB = dot(p0, normal);
	if (projA + thickness < projB) return false;

	// 面法線を分離軸にとる
	getSupportVertex(&convex, -normal, sA, 0.0f);
	projA = dot(sA, normal);

	if (projA > projB) return false; // 面より上にある

	// 膨らませる
	float fat = (length(p0 - p1) + length(p1 - p2) + length(p2 - p0)) * 0.033f;
	PfxVector3 facetPnts[4] = {
		p0, p1, p2, (p0 + p1 + p2) / 3.0f - fat * normal,
	};

	PfxPoint3 pA(0.0f),pB(0.0f);
	PfxVector3 nml(0.0f);

	PfxFatTriangle fatTriangle;
	fatTriangle.points[0] = facetPnts[0];
	fatTriangle.points[1] = facetPnts[1];
	fatTriangle.points[2] = facetPnts[2];
	fatTriangle.points[3] = facetPnts[3];

	PfxMpr<PfxFatTriangle, PfxConvexMeshImpl> collider(&fatTriangle, &convex);

	PfxFloat d;
	PfxVector3 cachedAxis(0.0f);
	PfxUInt32 featureIdA = 0, featureIdB = 0;
	PfxInt32 ret = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, cachedAxis, SCE_PFX_FLT_MAX);
	if(ret != kPfxGjkResultOk || d >= 0.0f) return false;

	PfxVector3 pointsOnTriangle = PfxVector3(pA);
	PfxVector3 axis = nml;

	// Ignore a contact if a normal direction is opposed to a facet normal
	if(dot(-normal, axis) < 0.0f) return false;

	// 面上の最近接点が凸エッジ上でない場合は法線を変える
	if (pfxPointIsOnTriangleEdge(pointsOnTriangle, edgeChk, p0, p1, p2)) {
		axis=-normal;
	}

	PfxSubData subData;
	subData.setFacetId(facetId);
	contacts.addContactPoint(d, axis, pA, pB, featureIdA, featureIdB, subData, PfxSubData());

	return true;
}


template<>
PfxInt32 pfxContactTriMesh<PfxConvexMeshImpl, PfxExpandedTriMesh>(
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA,const PfxExpandedTriMesh *meshA,bool flipTriangle,
	const PfxConvexMeshImpl &convexB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold)
{
	(void)flipTriangle;
	(void)largeMeshA;
	(void)distanceThreshold;

	PfxTransform3 transformBA = inverse(transformB);
	PfxMatrix3 rotationBA_n = transpose(inverse(transformBA.getUpper3x3()));

	//-------------------------------------------
	// 判定する面を絞り込む.

	PfxUInt8 SCE_PFX_ALIGNED(16) selFacets[SCE_PFX_NUMMESHFACETS] = { 0 };
	PfxUInt32 numSelFacets = pfxGatherFacets(meshA,
		transformB.getTranslation(),
		absPerElem(transformB.getUpper3x3())*pfxReadVector3(convexB.m_half), selFacets);
	if (numSelFacets == 0) return 0;

	//-------------------------------------------
	// 面ごとに衝突を検出

	PfxContactCache localContacts;

	const int vi[3] = { 0,1,2 };
	const int ei[3] = { 0,1,2 };

	for (PfxUInt32 f = 0; f < numSelFacets; f++) {
		const PfxExpandedFacet &facet = meshA->m_facets[selFacets[f]];

		PfxVector3 facetNormal = normalize(rotationBA_n * pfxReadVector3((PfxFloat*)&facet.m_normal));

		PfxVector3 facetPntsA[3] = {
			transformBA.getTranslation() + transformBA.getUpper3x3() * meshA->m_verts[facet.m_vertIds[vi[0]]],
			transformBA.getTranslation() + transformBA.getUpper3x3() * meshA->m_verts[facet.m_vertIds[vi[1]]],
			transformBA.getTranslation() + transformBA.getUpper3x3() * meshA->m_verts[facet.m_vertIds[vi[2]]],
		};

		const PfxEdge *edge[3] = {
			&meshA->m_edges[facet.m_edgeIds[ei[0]]],
			&meshA->m_edges[facet.m_edgeIds[ei[1]]],
			&meshA->m_edges[facet.m_edgeIds[ei[2]]],
		};

		PfxUInt32 edgeChk = edge[0]->m_angleType | (edge[1]->m_angleType << 2) | (edge[2]->m_angleType << 4);

		pfxContactTriangleConvex(localContacts, selFacets[f],
			facetNormal, facetPntsA[0], facetPntsA[1], facetPntsA[2],
			facet.m_thickness,
			0.5f*SCE_PFX_PI*(edge[0]->m_tilt / 255.0f),
			0.5f*SCE_PFX_PI*(edge[1]->m_tilt / 255.0f),
			0.5f*SCE_PFX_PI*(edge[2]->m_tilt / 255.0f),
			edgeChk, convexB, transformB);
	}

	PfxMatrix3 rotationB_n = transpose(inverse(transformB.getUpper3x3()));

	for (PfxUInt32 i = 0; i<localContacts.getNumContactPoints(); i++) {
		PfxSubData subData = localContacts.getContactSubDataA(i);

		const PfxExpandedFacet &facet = meshA->m_facets[subData.getFacetId()];

		PfxTriangle triangleA(
			transformBA.getTranslation() + transformBA.getUpper3x3() * meshA->m_verts[facet.m_vertIds[vi[0]]],
			transformBA.getTranslation() + transformBA.getUpper3x3() * meshA->m_verts[facet.m_vertIds[vi[1]]],
			transformBA.getTranslation() + transformBA.getUpper3x3() * meshA->m_verts[facet.m_vertIds[vi[2]]]);

		PfxFloat s = 0.0f, t = 0.0f;
		pfxCalcBarycentricCoords(PfxVector3(localContacts.getContactLocalPointA(i)), triangleA, s, t);
		subData.setType(PfxSubData::MESH_INFO);
		subData.setFacetLocalS(s);
		subData.setFacetLocalT(t);
		subData.setUserData(facet.m_userData);

		PfxUInt16 islandId = (PfxUInt16)(meshA - (PfxExpandedTriMesh*)largeMeshA->m_islands);
		PfxUInt64 featureIdA = ((PfxUInt64)islandId << 32) | localContacts.getFeatureIdA(i);

		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			rotationB_n * localContacts.getContactNormal(i),
			transformB * localContacts.getContactLocalPointA(i),
			localContacts.getContactLocalPointB(i),
			featureIdA, localContacts.getFeatureIdB(i),
			subData, PfxSubData());
	}

	return contacts.getNumContactPoints();
}

template<>
PfxInt32 pfxContactTriMesh<PfxConvexMeshImpl, PfxQuantizedTriMeshBvh>(
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA,const PfxQuantizedTriMeshBvh *meshA,bool flipTriangle,
	const PfxConvexMeshImpl &convexB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold)
{
	(void)flipTriangle;
	(void)distanceThreshold;

	PfxTransform3 transformBA = inverse(transformB);
	PfxMatrix3 rotationBA_n = transpose(inverse(transformBA.getUpper3x3()));

	//	building aabb of the capsule within A local
	PfxVector3 _aabbB(pfxReadVector3(convexB.m_half));

	PfxVector3 aabbMin = transformB.getTranslation() - absPerElem(transformB.getUpper3x3()) * _aabbB;
	PfxVector3 aabbMax = transformB.getTranslation() + absPerElem(transformB.getUpper3x3()) * _aabbB;

	//	operate back-tracking through Largemsh's BV-tree
	PfxUInt8 SCE_PFX_ALIGNED(16) selFacets[SCE_PFX_NUMMESHFACETS] = { 0 };
	PfxUInt32 numSelFacets = pfxGatherFacets(meshA, aabbMin, aabbMax, selFacets);
	if (numSelFacets == 0) return 0;

	//	operating "separating hyperplane theorem"
	PfxDecodedTriMesh decodedMesh;  // 特に効果なし?
	PfxContactCache localContacts;

	const int vi[3] = { 0,1,2 };
	const int ei[3] = { 0,1,2 };

	for (PfxUInt32 f = 0; f < numSelFacets; f++) {
		const PfxQuantizedFacetBvh &facet = meshA->m_facets[selFacets[f]];

		// デコード
		PfxDecodedFacet decodedFacet;
		const PfxUInt32 vId[3] = { facet.m_vertIds[vi[0]],facet.m_vertIds[vi[1]],facet.m_vertIds[vi[2]] };

		decodedFacet.m_normal = normalize(rotationBA_n * largeMeshA->decodeNormal(facet.m_normal));
		decodedFacet.m_thickness = largeMeshA->decodeFloat(facet.m_thickness);

		//decodedFacet.m_normal = largeMeshA->decodeNormal(facet.m_normal);
		//decodedFacet.m_thickness = largeMeshA->decodeFloat(facet.m_thickness);

		for (int v = 0; v<3; v++) {
			if (!decodedMesh.isDecodedVertex(vId[v])) {
				decodedMesh.m_verts[vId[v]] = transformBA.getTranslation() + transformBA.getUpper3x3() * largeMeshA->decodePosition(meshA->m_verts[vId[v]]);
				decodedMesh.m_decodedVertex[vId[v] >> 5] |= 1 << (vId[v] & 0x1f);
			}
		}

		PfxVector3 facetNormal = decodedFacet.m_normal;

		//PfxVector3 facetPntsA[3];
		//facetPntsA[0] = transformBA.getTranslation() + transformBA.getUpper3x3() * largeMeshA->decodePosition(meshA->m_verts[vId[0]]);
		//facetPntsA[1] = transformBA.getTranslation() + transformBA.getUpper3x3() * largeMeshA->decodePosition(meshA->m_verts[vId[1]]);
		//facetPntsA[2] = transformBA.getTranslation() + transformBA.getUpper3x3() * largeMeshA->decodePosition(meshA->m_verts[vId[2]]);

		PfxVector3 facetPntsA[3] = {
			decodedMesh.m_verts[vId[0]],
			decodedMesh.m_verts[vId[1]],
			decodedMesh.m_verts[vId[2]],
		};

		const PfxEdge *edge[3] = {
			&meshA->m_edges[facet.m_edgeIds[ei[0]]],
			&meshA->m_edges[facet.m_edgeIds[ei[1]]],
			&meshA->m_edges[facet.m_edgeIds[ei[2]]],
		};

		PfxUInt32 edgeChk = edge[0]->m_angleType | (edge[1]->m_angleType << 2) | (edge[2]->m_angleType << 4);

		pfxContactTriangleConvex(localContacts, selFacets[f],
			facetNormal, facetPntsA[0], facetPntsA[1], facetPntsA[2],
			decodedFacet.m_thickness,
			0.5f*SCE_PFX_PI*(edge[0]->m_tilt / 255.0f),
			0.5f*SCE_PFX_PI*(edge[1]->m_tilt / 255.0f),
			0.5f*SCE_PFX_PI*(edge[2]->m_tilt / 255.0f),
			edgeChk, convexB, transformB);
	}

	PfxMatrix3 rotationB_n = transpose(inverse(transformB.getUpper3x3()));

	for (PfxUInt32 i = 0; i<localContacts.getNumContactPoints(); i++) {
		PfxSubData subData = localContacts.getContactSubDataA(i);

		const PfxQuantizedFacetBvh &facet = meshA->m_facets[subData.getFacetId()];

		//PfxTriangle triangleA(
		//	transformBA.getTranslation() + transformBA.getUpper3x3() * largeMeshA->decodePosition(meshA->m_verts[facet.m_vertIds[vi[0]]]),
		//	transformBA.getTranslation() + transformBA.getUpper3x3() * largeMeshA->decodePosition(meshA->m_verts[facet.m_vertIds[vi[1]]]),
		//	transformBA.getTranslation() + transformBA.getUpper3x3() * largeMeshA->decodePosition(meshA->m_verts[facet.m_vertIds[vi[2]]]));

		PfxTriangle triangleA(
			decodedMesh.m_verts[facet.m_vertIds[vi[0]]],
			decodedMesh.m_verts[facet.m_vertIds[vi[1]]],
			decodedMesh.m_verts[facet.m_vertIds[vi[2]]]);

		PfxFloat s = 0.0f, t = 0.0f;
		pfxCalcBarycentricCoords(PfxVector3(localContacts.getContactLocalPointA(i)), triangleA, s, t);
		subData.setType(PfxSubData::MESH_INFO);
		subData.setFacetLocalS(s);
		subData.setFacetLocalT(t);
		subData.setUserData(facet.m_userData);

		PfxUInt16 islandId = (PfxUInt16)(meshA - (PfxQuantizedTriMeshBvh*)largeMeshA->m_islands);
		PfxUInt64 featureIdA = ((PfxUInt64)islandId << 32) | localContacts.getFeatureIdA(i);

		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			rotationB_n * localContacts.getContactNormal(i),
			transformB * localContacts.getContactLocalPointA(i),
			localContacts.getContactLocalPointB(i),
			featureIdA, localContacts.getFeatureIdB(i),
			subData, PfxSubData());
	}

	return contacts.getNumContactPoints();
}

template<>
PfxInt32 pfxContactTriMesh<PfxConvexMeshImpl, PfxCompressedTriMesh>(
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA, const PfxCompressedTriMesh *meshA, bool flipTriangle,
	const PfxConvexMeshImpl &convexB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold)
{
	(void) distanceThreshold;

	PfxTransform3 transformBA = inverse(transformB);

	//	building aabb of the capsule within A local
	PfxVector3 _aabbB(pfxReadVector3(convexB.m_half));

	PfxVector3 aabbMin = transformB.getTranslation() - absPerElem(transformB.getUpper3x3()) * _aabbB;
	PfxVector3 aabbMax = transformB.getTranslation() + absPerElem(transformB.getUpper3x3()) * _aabbB;

	//	operate back-tracking through Largemsh's BV-tree
	PfxUInt8 SCE_PFX_ALIGNED(16) selFacets[SCE_PFX_NUMMESHFACETS] = {0};
	PfxUInt32 numSelFacets = pfxGatherFacets(meshA,(PfxFacetBvhNode*)largeMeshA->m_bvhNodeBuffer,aabbMin,aabbMax,selFacets);
	if(numSelFacets == 0) return 0;

	//	operating "separating hyperplane theorem"
	PfxDecodedTriMesh decodedMesh;
	PfxContactCache localContacts;

	int vi[4] = {0,1,2,3};
	if (flipTriangle) { vi[0] = 2; vi[2] = 0; }

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

		PfxVector3 facetPntsA[4] = {
			decodedMesh.m_verts[vId[0]],
			decodedMesh.m_verts[vId[1]],
			decodedMesh.m_verts[vId[2]],
			decodedMesh.m_verts[vId[3]],
		};

		if(facet.m_facetInfo & 0x8000) {
			// double facets
			PfxVector3 facetNormal0 = normalize(cross(facetPntsA[1]-facetPntsA[0],facetPntsA[2]-facetPntsA[0]));
			PfxVector3 facetNormal1 = normalize(cross(facetPntsA[3]-facetPntsA[2],facetPntsA[0]-facetPntsA[2]));

			PfxUInt32 edgeChk0 = facet.m_edgeInfo & 0x3f;
			PfxUInt32 edgeChk1 = facet.m_edgeInfo>>4;

			if (flipTriangle) {
				edgeChk0 = ((edgeChk0 >> 2) & 0x0c) | ((edgeChk0 << 2) & 0x30) | (edgeChk0 & 0x03);
				edgeChk1 = ((edgeChk1 >> 2) & 0x0c) | ((edgeChk1 << 2) & 0x30) | (edgeChk1 & 0x03);
			}

			PfxUInt8 facetId0 = selFacets[f];
			PfxUInt8 facetId1 = selFacets[f] | 0x80;

			pfxContactTriangleConvex(localContacts, facetId0,
								facetNormal0,facetPntsA[0],facetPntsA[1],facetPntsA[2],
								largeMeshA->m_defaultThickness,0.0f,0.0f,0.0f,
								edgeChk0,convexB,transformB);

			pfxContactTriangleConvex(localContacts, facetId1,
								facetNormal1,facetPntsA[0],facetPntsA[2],facetPntsA[3],
								largeMeshA->m_defaultThickness,0.0f,0.0f,0.0f,
								edgeChk1,convexB,transformB);
		}
		else {
			// signle facet
			PfxVector3 facetNormal = normalize(cross(facetPntsA[1]-facetPntsA[0],facetPntsA[2]-facetPntsA[0]));

			PfxUInt32 edgeChk = facet.m_edgeInfo & 0x3f;

			if (flipTriangle) {
				edgeChk = ((edgeChk >> 2) & 0x0c) | ((edgeChk << 2) & 0x30) | (edgeChk & 0x03);
			}

			pfxContactTriangleConvex(localContacts, selFacets[f],
								facetNormal,facetPntsA[0],facetPntsA[1],facetPntsA[2],
								largeMeshA->m_defaultThickness,0.0f,0.0f,0.0f,
								edgeChk,convexB,transformB);
		}
	}

	PfxMatrix3 rotationB_n = transpose(inverse(transformB.getUpper3x3()));

	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subData = localContacts.getContactSubDataA(i);

		PfxUInt8 facetId = subData.getFacetId();

		const PfxCompressedFacet2 &facet = ((PfxCompressedFacet2*)largeMeshA->m_facetBuffer)[meshA->m_facets + (facetId&0x7f)];

		PfxVector3 p[3];
		PfxUInt32 userData;

		if(facetId&0x80) {
			// facet 1
			p[0] = decodedMesh.m_verts[facet.m_vertIds[vi[0]]];
			p[1] = decodedMesh.m_verts[facet.m_vertIds[vi[2]]];
			p[2] = decodedMesh.m_verts[facet.m_vertIds[vi[3]]];
			userData = facet.m_userData[1];
			facetId = (facet.m_facetInfo&0xFF)+1; // Calculate the actual facet index
		}
		else {
			// facet 0
			p[0] = decodedMesh.m_verts[facet.m_vertIds[vi[0]]];
			p[1] = decodedMesh.m_verts[facet.m_vertIds[vi[1]]];
			p[2] = decodedMesh.m_verts[facet.m_vertIds[vi[2]]];
			userData = facet.m_userData[0];
			facetId = (facet.m_facetInfo&0xFF); // Calculate the actual facet index
		}

		PfxTriangle triangleA(p[0],p[1],p[2]);

		PfxFloat s=0.0f,t=0.0f;
		pfxCalcBarycentricCoords(PfxVector3(localContacts.getContactLocalPointA(i)),triangleA,s,t);
		subData.setType(PfxSubData::MESH_INFO);
		subData.setFacetLocalS(s);
		subData.setFacetLocalT(t);
		subData.setUserData(userData);
		subData.setFacetId(facetId);

		PfxUInt16 islandId = (PfxUInt16)(meshA - (PfxCompressedTriMesh*)largeMeshA->m_islands);
		PfxUInt64 featureIdA = ((PfxUInt64)islandId << 32) | localContacts.getFeatureIdA(i);

		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			rotationB_n * localContacts.getContactNormal(i),
			transformB * localContacts.getContactLocalPointA(i),
			localContacts.getContactLocalPointB(i),
			featureIdA, localContacts.getFeatureIdB(i),
			subData,PfxSubData());
	}

	return contacts.getNumContactPoints();
}

} //namespace pfxv4
} //namespace sce
