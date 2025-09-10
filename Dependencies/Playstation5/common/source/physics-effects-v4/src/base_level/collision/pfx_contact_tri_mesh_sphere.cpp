/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "pfx_contact_tri_mesh_sphere.h"
#include "pfx_intersect_common.h"
#include "pfx_mesh_common.h"
#include "pfx_decoded_tri_mesh.h"
#include "pfx_gjk_solver.h"

namespace sce {
namespace pfxv4 {

static bool pfxContactTriangleSphere(PfxContactCache &contacts,PfxUInt32 facetId,
	const PfxVector3 &normal,const PfxVector3 &p0,const PfxVector3 &p1,const PfxVector3 &p2,
	const PfxFloat thickness,const PfxFloat angle0,const PfxFloat angle1,const PfxFloat angle2,
	PfxUInt32 edgeChk,const PfxSphere &sphere)
{
	PfxFloat sphereRadius = sphere.m_radius;

	PfxVector3 facetPnts[3] = {
		p0,p1,p2,
	};

	// 早期判定
	{
		PfxFloat d;

		//PfxPlane planeA0(normal,p0);
		//d = planeA0.onPlane(PfxVector3::zero());
		d = dot(-p0,normal);

		if(d >= sphereRadius) return false;

		//PfxPlane planeA1(-normal,p0-thickness*normal);
		//d = planeA1.onPlane(PfxVector3::zero());
		d = dot(thickness*normal-p0,-normal);

		if(d >= sphereRadius) return false;

		PfxVector3 sideNml[3];
		sideNml[0] = normalize(cross((facetPnts[1] - facetPnts[0]),normal));
		sideNml[1] = normalize(cross((facetPnts[2] - facetPnts[1]),normal));
		sideNml[2] = normalize(cross((facetPnts[0] - facetPnts[2]),normal));

		for(int i=0;i<3;i++) {
			//PfxPlane plane(sideNml[i],facetPnts[i]);
			//d = plane.onPlane(PfxVector3::zero());
			d = dot(-facetPnts[i],sideNml[i]);
			if(d >= sphereRadius) return false;
		}
	}

	// 球と面の最近接点を計算
	PfxVector3 pntA;
	bool insideTriangle = false;
#if 1
	while(1) {
		PfxVector3 ab = p1 - p0;
		PfxVector3 ac = p2 - p0;
		PfxVector3 ap = - p0;
		PfxFloat d1 = dot(ab, ap);
		PfxFloat d2 = dot(ac, ap);
		if(d1 <= 0.0f && d2 <= 0.0f) {
			pntA = p0;
			break;
		}

		PfxVector3 bp = - p1;
		PfxFloat d3 = dot(ab, bp);
		PfxFloat d4 = dot(ac, bp);
		if (d3 >= 0.0f && d4 <= d3) {
			pntA = p1;
			break;
		}

		PfxFloat vc = d1*d4 - d3*d2;
		if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
			PfxFloat v = d1 / (d1 - d3);
			pntA = p0 + v * ab;
			break;
		}

		PfxVector3 cp = - p2;
		PfxFloat d5 = dot(ab, cp);
		PfxFloat d6 = dot(ac, cp);
		if (d6 >= 0.0f && d5 <= d6) {
			pntA = p2;
			break;
		}

		PfxFloat vb = d5*d2 - d1*d6;
		if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
			PfxFloat w = d2 / (d2 - d6);
			pntA = p0 + w * ac;
			break;
		}

		PfxFloat va = d3*d6 - d5*d4;
		if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
			PfxFloat w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
			pntA = p1 + w * (p2 - p1);
			break;
		}

		PfxFloat den = 1.0f / (va + vb + vc);
		PfxFloat v = vb * den;
		PfxFloat w = vc * den;
		pntA = p0 + ab * v + ac * w;
		insideTriangle = true;
		break;
	}
#else
	while(1) {
		// ３角形面上の投影点
		PfxVector3 proj = dot(normal,p0) * normal;

		// エッジP0,P1のボロノイ領域
		PfxVector3 edgeP01 = p1 - p0;
		PfxVector3 edgeP01_normal = cross(edgeP01,normal);

		PfxFloat voronoiEdgeP01_check1 = dot(proj - p0,edgeP01_normal);
		PfxFloat voronoiEdgeP01_check2 = dot(proj - p0,edgeP01);
		PfxFloat voronoiEdgeP01_check3 = dot(proj - p1,-edgeP01);

		if(voronoiEdgeP01_check1 > 0.0f && voronoiEdgeP01_check2 > 0.0f && voronoiEdgeP01_check3 > 0.0f) {
			pfxClosestPointLine(p0,edgeP01,proj,pntA);
			break;
		}

		// エッジP1,P2のボロノイ領域
		PfxVector3 edgeP12 = p2 - p1;
		PfxVector3 edgeP12_normal = cross(edgeP12,normal);

		PfxFloat voronoiEdgeP12_check1 = dot(proj - p1,edgeP12_normal);
		PfxFloat voronoiEdgeP12_check2 = dot(proj - p1,edgeP12);
		PfxFloat voronoiEdgeP12_check3 = dot(proj - p2,-edgeP12);

		if(voronoiEdgeP12_check1 > 0.0f && voronoiEdgeP12_check2 > 0.0f && voronoiEdgeP12_check3 > 0.0f) {
			pfxClosestPointLine(p1,edgeP12,proj,pntA);
			break;
		}

		// エッジP2,P0のボロノイ領域
		PfxVector3 edgeP20 = p0 - p2;
		PfxVector3 edgeP20_normal = cross(edgeP20,normal);

		PfxFloat voronoiEdgeP20_check1 = dot(proj - p2,edgeP20_normal);
		PfxFloat voronoiEdgeP20_check2 = dot(proj - p2,edgeP20);
		PfxFloat voronoiEdgeP20_check3 = dot(proj - p0,-edgeP20);

		if(voronoiEdgeP20_check1 > 0.0f && voronoiEdgeP20_check2 > 0.0f && voronoiEdgeP20_check3 > 0.0f) {
			pfxClosestPointLine(p2,edgeP20,proj,pntA);
			break;
		}

		// ３角形面の内側
		if(voronoiEdgeP01_check1 <= 0.0f && voronoiEdgeP12_check1 <= 0.0f && voronoiEdgeP20_check1 <= 0.0f) {
			pntA = proj;
			insideTriangle = true;
			break;
		}

		// 頂点P0のボロノイ領域
		if(voronoiEdgeP01_check2 <= 0.0f && voronoiEdgeP20_check3 <= 0.0f) {
			pntA = p0;
			break;
		}

		// 頂点P1のボロノイ領域
		if(voronoiEdgeP01_check3 <= 0.0f && voronoiEdgeP12_check2 <= 0.0f) {
			pntA = p1;
			break;
		}

		// 頂点P2のボロノイ領域
		if(voronoiEdgeP20_check2 <= 0.0f && voronoiEdgeP12_check3 <= 0.0f) {
			pntA = p2;
			break;
		}
	}
#endif

	PfxVector3 distVec = pntA;
	PfxFloat l = lengthSqr(distVec);

	if(!insideTriangle && l >= sphereRadius * sphereRadius) return false;

	// 分離軸
	PfxVector3 sepAxis = (l < 0.00001f || insideTriangle) ? -normal : distVec * rsqrtf(PfxFloatInVec(l));

	// 球上の衝突点
	PfxVector3 pointsOnSphere = sphereRadius * sepAxis;
	PfxVector3 pointsOnTriangle = pntA;

	// 面上の最近接点が凸エッジ上でない場合は法線を変える
	if (pfxPointIsOnTriangleEdge(pointsOnTriangle, edgeChk, p0, p1, p2)) {
		sepAxis=-normal;
	}

	PfxSubData subData;
	subData.setFacetId(facetId);
	contacts.addContactPoint(-length(pointsOnSphere-pointsOnTriangle),sepAxis,PfxPoint3(pointsOnTriangle),PfxPoint3(pointsOnSphere),subData,PfxSubData());

	return true;
}

template<>
PfxInt32 pfxContactTriMesh<PfxSphere, PfxExpandedTriMesh> (
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA, const PfxExpandedTriMesh *meshA, bool flipTriangle,
	const PfxSphere &sphereB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold)
{
	(void) largeMeshA;
	(void) distanceThreshold;

	PfxTransform3 transformBA = inverse(transformB);
	PfxMatrix3 rotationBA_n = transpose(inverse(transformBA.getUpper3x3()));

	//-------------------------------------------
	// 判定する面を絞り込む

	PfxVector3 center = transformB.getTranslation();
	PfxVector3 half = absPerElem(transformB.getUpper3x3()) * PfxVector3(sphereB.m_radius);
	PfxUInt8 selFacets[SCE_PFX_NUMMESHFACETS] = {0};
	PfxUInt32 numSelFacets = pfxGatherFacets(meshA,center,half,selFacets);
	if(numSelFacets == 0) return 0;

	//-----------------------------------------------
	// 判定

	PfxContactCache localContacts;

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

			PfxVector3 sepAxis,pntA,pntB;

			PfxUInt32 edgeChk = edge[0]->m_angleType | (edge[1]->m_angleType << 2) | (edge[2]->m_angleType << 4);

			pfxContactTriangleSphere(localContacts,selFacets[f],
									facetNormal,facetPnts[0],facetPnts[1],facetPnts[2],
									facet.m_thickness,
									0.5f*SCE_PFX_PI*(edge[0]->m_tilt/255.0f),
									0.5f*SCE_PFX_PI*(edge[1]->m_tilt/255.0f),
									0.5f*SCE_PFX_PI*(edge[2]->m_tilt/255.0f),
									edgeChk,sphereB.m_radius);
		}
	}

	PfxMatrix3 rotationB_n = transpose(inverse(transformB.getUpper3x3()));

	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subData = localContacts.getContactSubDataA(i);

		const PfxExpandedFacet &facet = meshA->m_facets[subData.getFacetId()];

		PfxTriangle triangleA(
			meshA->m_verts[facet.m_vertIds[vi[0]]],
			meshA->m_verts[facet.m_vertIds[vi[1]]],
			meshA->m_verts[facet.m_vertIds[vi[2]]]);

		PfxFloat s=0.0f,t=0.0f;
		pfxCalcBarycentricCoords(PfxVector3(localContacts.getContactLocalPointA(i)),triangleA,s,t);
		subData.setType(PfxSubData::MESH_INFO);
		subData.setFacetLocalS(s);
		subData.setFacetLocalT(t);
		subData.setUserData(facet.m_userData);

		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			rotationB_n * localContacts.getContactNormal(i),
			transformB * localContacts.getContactLocalPointA(i),
			localContacts.getContactLocalPointB(i),
			subData,PfxSubData());
	}

	return contacts.getNumContactPoints();
}

template<>
PfxInt32 pfxContactTriMesh<PfxSphere, PfxQuantizedTriMeshBvh> (
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA, const PfxQuantizedTriMeshBvh *meshA, bool flipTriangle,
	const PfxSphere &sphereB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold)
{
	(void) distanceThreshold;

	PfxTransform3 transformBA = inverse(transformB);
	PfxMatrix3 rotationBA_n = transpose(inverse(transformBA.getUpper3x3()));

	PfxVector3 center = transformB.getTranslation();
	PfxVector3 half = absPerElem(transformB.getUpper3x3()) * PfxVector3(sphereB.m_radius);
	PfxUInt8 selFacets[SCE_PFX_NUMMESHFACETS] = {0};
	PfxUInt32 numSelFacets = pfxGatherFacets(meshA,center - half,center + half,selFacets);
	if(numSelFacets == 0) return 0;

	//-------------------------------------------
	//	operating "separating hyperplane theorem"

	PfxDecodedTriMesh decodedMesh;
	PfxContactCache localContacts;

	int vi[3] = {0,1,2};
	int ei[3] = {0,1,2};
	if(flipTriangle) {vi[0]=2;vi[2]=0;ei[0]=1;ei[1]=0;}

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

			PfxVector3 sepAxis,pntA,pntB;

			PfxUInt32 edgeChk = edge[0]->m_angleType | (edge[1]->m_angleType << 2) | (edge[2]->m_angleType << 4);

			#if 1
			pfxContactTriangleSphere(localContacts,selFacets[f],
									decodedFacet.m_normal,facetPnts[0],facetPnts[1],facetPnts[2],
									decodedFacet.m_thickness,
									0.5f*SCE_PFX_PI*(edge[0]->m_tilt/255.0f),
									0.5f*SCE_PFX_PI*(edge[1]->m_tilt/255.0f),
									0.5f*SCE_PFX_PI*(edge[2]->m_tilt/255.0f),
									edgeChk,sphereB.m_radius);
			#else
			pfxContactTriangleSphereMpr(localContacts,selFacets[f],
									decodedFacet.m_normal,facetPnts[0],facetPnts[1],facetPnts[2],
									decodedFacet.m_thickness,
									0.5f*SCE_PFX_PI*(edge[0]->m_tilt/255.0f),
									0.5f*SCE_PFX_PI*(edge[1]->m_tilt/255.0f),
									0.5f*SCE_PFX_PI*(edge[2]->m_tilt/255.0f),
									edgeChk,sphereB);
			#endif
		}
	}

	PfxMatrix3 rotationB_n = transpose(inverse(transformB.getUpper3x3()));

	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subData = localContacts.getContactSubDataA(i);

		const PfxQuantizedFacetBvh &facet = meshA->m_facets[subData.getFacetId()];

		PfxTriangle triangleA(
			decodedMesh.m_verts[facet.m_vertIds[vi[0]]],
			decodedMesh.m_verts[facet.m_vertIds[vi[1]]],
			decodedMesh.m_verts[facet.m_vertIds[vi[2]]]);

		PfxFloat s=0.0f,t=0.0f;
		pfxCalcBarycentricCoords(PfxVector3(localContacts.getContactLocalPointA(i)),triangleA,s,t);
		subData.setType(PfxSubData::MESH_INFO);
		subData.setFacetLocalS(s);
		subData.setFacetLocalT(t);
		subData.setUserData(facet.m_userData);

		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			rotationB_n * localContacts.getContactNormal(i),
			transformB * localContacts.getContactLocalPointA(i),
			localContacts.getContactLocalPointB(i),
			subData,PfxSubData());
	}

	return contacts.getNumContactPoints();
}

template<>
PfxInt32 pfxContactTriMesh<PfxSphere, PfxCompressedTriMesh> (
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA, const PfxCompressedTriMesh *meshA, bool flipTriangle,
	const PfxSphere &sphereB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold)
{
	(void) distanceThreshold;

	PfxTransform3 transformBA = inverse(transformB);

	PfxVector3 center = transformB.getTranslation();
	PfxVector3 half = absPerElem(transformB.getUpper3x3()) * PfxVector3(sphereB.m_radius);
	PfxUInt8 selFacets[SCE_PFX_NUMMESHFACETS] = {0};
	PfxUInt32 numSelFacets = pfxGatherFacets(meshA,(PfxFacetBvhNode*)largeMeshA->m_bvhNodeBuffer,center - half,center + half,selFacets);
	if(numSelFacets == 0) return 0;

	//-------------------------------------------
	//	operating "separating hyperplane theorem"

	PfxDecodedTriMesh decodedMesh;
	PfxContactCache localContacts;

	int vi[4] = {0,1,2,3};
	if(flipTriangle) {vi[0]=2;vi[2]=0;}

	// TriangleMeshの面->sphereの判定
	// In a sphere coordinate
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

			if(flipTriangle) {
				edgeChk0 = ((edgeChk0>>2)&0x0c) | ((edgeChk0<<2)&0x30) | (edgeChk0&0x03);
				edgeChk1 = ((edgeChk1>>2)&0x0c) | ((edgeChk1<<2)&0x30) | (edgeChk1&0x03);
			}

			PfxVector3 edgeAngle(0.0f);

			PfxUInt8 facetId0 = selFacets[f];
			PfxUInt8 facetId1 = selFacets[f] | 0x80;

			pfxContactTriangleSphere(localContacts,facetId0,
									facetNormal0,facetPntsA[0],facetPntsA[1],facetPntsA[2],
									largeMeshA->m_defaultThickness,0.0f,0.0f,0.0f,
									edgeChk0,sphereB.m_radius);

			pfxContactTriangleSphere(localContacts,facetId1,
									facetNormal1,facetPntsA[0],facetPntsA[2],facetPntsA[3],
									largeMeshA->m_defaultThickness,0.0f,0.0f,0.0f,
									edgeChk1,sphereB.m_radius);
		}
		else {
			// signle facet
			PfxVector3 facetNormal = normalize(cross(facetPntsA[1]-facetPntsA[0],facetPntsA[2]-facetPntsA[0]));

			PfxUInt32 edgeChk = facet.m_edgeInfo & 0x3f;

			if(flipTriangle) {
				edgeChk = ((edgeChk>>2)&0x0c) | ((edgeChk<<2)&0x30) | (edgeChk&0x03);
			}

			pfxContactTriangleSphere(localContacts,selFacets[f],
									facetNormal,facetPntsA[0],facetPntsA[1],facetPntsA[2],
									largeMeshA->m_defaultThickness,0.0f,0.0f,0.0f,
									edgeChk,sphereB.m_radius);
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

		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			rotationB_n * localContacts.getContactNormal(i),
			transformB * localContacts.getContactLocalPointA(i),
			localContacts.getContactLocalPointB(i),
			subData,PfxSubData());
	}

	return contacts.getNumContactPoints();
}

} //namespace pfxv4
} //namespace sce
