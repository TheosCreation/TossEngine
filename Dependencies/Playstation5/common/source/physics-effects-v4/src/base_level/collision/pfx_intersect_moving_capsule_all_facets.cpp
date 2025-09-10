/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/util/pfx_static_array.h"
#include "pfx_intersect_common.h"
#include "pfx_convex_mesh_impl.h"
#include "pfx_large_tri_mesh_impl.h"
#include "pfx_mesh_common.h"
#include "pfx_decoded_tri_mesh.h"
#include "pfx_intersect_moving_capsule_all_facets.h"
#include <float.h>

namespace sce {
namespace pfxv4 {

extern PfxBool pfxIntersectMovingCapsuleTriangle(const PfxCapsuleInputInternal &capsuleIn, const PfxTriangle &triangle, PfxVector3 &contactPoint, PfxFloat &variable, PfxFloat &squaredDistance);

struct EncStack {
	PfxUInt32 nodeId;
	PfxVector3 aabbMin;
	PfxVector3 aabbMax;
};

PfxBool pfxIntersectMovingCapsuleTriMesh(
	const PfxLargeTriMeshImpl *largeMesh,const PfxExpandedTriMesh *mesh,
	const PfxVector3 &rayStartLocal,const PfxVector3 &rayDirLocal,PfxFloat radiusLocal,
	const PfxCapsuleInputInternal &capsuleIn,
	const PfxTransform3 &transform,PfxBool flipTriangle,PfxUInt32 islandId,
	PfxBool(*callback)(const PfxCapsuleOutput &hit, void *userData), void *userData)
{
	(void) largeMesh;
	
	bool ret = true;
	
	//-------------------------------------------
	// 判定する面を絞り込む
	
	PfxUInt8 selFacets[SCE_PFX_NUMMESHFACETS] = {0};
	
	PfxVector3 aabbMin = minPerElem(rayStartLocal,rayStartLocal + rayDirLocal);
	PfxVector3 aabbMax = maxPerElem(rayStartLocal,rayStartLocal + rayDirLocal);
	
	PfxUInt32 numSelFacets = pfxGatherFacets(mesh,0.5f*(aabbMin+aabbMax),0.5f*(aabbMax-aabbMin) + PfxVector3(radiusLocal),selFacets);
	
	if(numSelFacets == 0) {
		return ret;
	}
	
	//-------------------------------------------
	// レイ交差判定
	
	PfxUInt32 ids[3] = {0,1,2};
	
	if(flipTriangle) {ids[0] = 2;ids[2] = 0;}
	
	for(PfxUInt32 f = 0; f < numSelFacets; f++) {
		const PfxExpandedFacet &facet = mesh->m_facets[selFacets[f]];
		
		PfxTriangle triangle(
			transform.getTranslation() + transform.getUpper3x3() * mesh->m_verts[facet.m_vertIds[ids[0]]],
			transform.getTranslation() + transform.getUpper3x3() * mesh->m_verts[facet.m_vertIds[ids[1]]],
			transform.getTranslation() + transform.getUpper3x3() * mesh->m_verts[facet.m_vertIds[ids[2]]]);
		
		PfxVector3 normal = triangle.calcNormal();
		if(dot(normal,capsuleIn.m_startPosition-triangle.points[ 0 ]) <= 0.0f) {
			continue;
		}
		
		PfxVector3 facetAabbMin = minPerElem(minPerElem(triangle.points[0],triangle.points[1]),triangle.points[2]) - PfxVector3(capsuleIn.m_radius);
		PfxVector3 facetAabbMax = maxPerElem(maxPerElem(triangle.points[0],triangle.points[1]),triangle.points[2]) + PfxVector3(capsuleIn.m_radius);
		
		PfxFloat cur_t = 1.0f;
		PfxVector3 contactPoint;
		
		if( !pfxIntersectRayAABBFast(capsuleIn.m_startPosition,capsuleIn.m_direction,0.5f * (facetAabbMax + facetAabbMin),0.5f * (facetAabbMax - facetAabbMin),cur_t) )
			continue;
		
		PfxFloat tmpSquaredDistance = (capsuleIn.m_halfLength + capsuleIn.m_radius) * (capsuleIn.m_halfLength + capsuleIn.m_radius);
		if(pfxIntersectMovingCapsuleTriangle(capsuleIn, triangle, contactPoint, cur_t, tmpSquaredDistance)) {
			PfxCapsuleOutput hit;
			
			hit.m_contactFlag = true;
			hit.m_variable = cur_t;
			hit.m_contactPoint.segment = capsuleIn.m_segment;
			hit.m_contactPoint.offset = contactPoint;
			
			if(cur_t == 0.0f) {
				PfxVector3 normal;
				PfxVector3 point = contactPoint;
				calcPointAndNormalOnCapsule(point, normal, capsuleIn);
				hit.m_contactPoint = point;
				hit.m_contactNormal = -normal;
			}
			else {
				hit.m_contactPoint = contactPoint;
				hit.m_contactNormal = -calcNormalOnCapsule(
					contactPoint - cur_t * capsuleIn.m_direction,
					capsuleIn);
			}
			
			PfxFloat s=0.0f,t=0.0f;
			pfxCalcBarycentricCoords(contactPoint,triangle,s,t);
			hit.m_subData.setType(PfxSubData::MESH_INFO);
			hit.m_subData.setFacetLocalS(s);
			hit.m_subData.setFacetLocalT(t);
			hit.m_subData.setFacetId(selFacets[f]);
			hit.m_subData.setIslandId(islandId);
			hit.m_subData.setUserData(facet.m_userData);

			ret = callback(hit, userData);
			if(!ret) break;
		}
	}
 	
	return ret;
}

PfxBool pfxIntersectMovingCapsuleTriMesh(
	const PfxLargeTriMeshImpl *largeMesh,const PfxQuantizedTriMeshBvh *mesh,
	const PfxVector3 &rayStartLocal,const PfxVector3 &rayDirLocal,PfxFloat radiusLocal,
	const PfxCapsuleInputInternal &capsuleIn,
	const PfxTransform3 &transform,PfxBool flipTriangle,PfxUInt32 islandId,
	PfxBool(*callback)(const PfxCapsuleOutput &hit, void *userData), void *userData)
{
	(void) largeMesh;
	
	bool ret = true;
	
	PfxUInt32 ids[3] = {0,1,2};
	
	if(flipTriangle) {ids[0] = 2;ids[2] = 0;}
	
	EncStack encroot;
	encroot.nodeId = 0;
	encroot.aabbMin = pfxReadVector3(mesh->m_aabbMin);
	encroot.aabbMax = pfxReadVector3(mesh->m_aabbMax);
	
	PfxStaticStack<EncStack> bvhStack;
	bvhStack.push(encroot);
	
	PfxDecodedTriMesh decodedMesh;
	
	while(!bvhStack.empty()) {
		EncStack info = bvhStack.top();
		bvhStack.pop();

		PfxFacetBvhNode &encnode = mesh->m_bvhNodes[info.nodeId];
		
		PfxVector3 quantizedMin(encnode.aabb[0],encnode.aabb[1],encnode.aabb[2]);
		PfxVector3 quantizedMax(encnode.aabb[3],encnode.aabb[4],encnode.aabb[5]);
		PfxVector3 aabbMinA = info.aabbMin + mulPerElem((info.aabbMax-info.aabbMin),quantizedMin / 255.0f);
		PfxVector3 aabbMaxA = info.aabbMax - mulPerElem((info.aabbMax-info.aabbMin),quantizedMax / 255.0f);

		PfxFloat cur_t = 1.0f;
		PfxVector3 contactPoint;

		if(!pfxIntersectRayAABBFast(rayStartLocal,rayDirLocal,(aabbMaxA+aabbMinA)*0.5f,(aabbMaxA-aabbMinA)*0.5f + PfxVector3(radiusLocal),cur_t)) {
			continue;
		}

		PfxUInt32 numFacets=0;
		PfxUInt32 facetsIds[2];

		EncStack encnext;
		encnext.aabbMin = aabbMinA;
		encnext.aabbMax = aabbMaxA;

		PfxUInt8 LStatus = encnode.flag & 0x03;
		PfxUInt8 RStatus = (encnode.flag & 0x0C) >> 2;

		if(LStatus == 0) {
			encnext.nodeId = encnode.left;
			bvhStack.push(encnext);
		}
		else if(LStatus == 1) {
			facetsIds[numFacets++] = encnode.left;
		}
		
		if(RStatus == 0) {
			encnext.nodeId = encnode.right;
			bvhStack.push(encnext);
		}
		else if(RStatus == 1) {
			facetsIds[numFacets++] = encnode.right;
		}
		
		for(PfxUInt32 f=0;f<numFacets;f++) {
			const PfxQuantizedFacetBvh &facet = mesh->m_facets[facetsIds[f]];
			
			// デコード
			PfxDecodedFacet decodedFacet;
			const PfxUInt32 vId[3] = {facet.m_vertIds[0],facet.m_vertIds[1],facet.m_vertIds[2]};
			
			for(int v=0;v<3;v++) {
				if(!decodedMesh.isDecodedVertex(vId[v])) {
					decodedMesh.m_verts[vId[v]] = transform.getTranslation() + transform.getUpper3x3() * largeMesh->decodePosition(mesh->m_verts[vId[v]]);
					decodedMesh.m_decodedVertex[vId[v]>>5] |= 1 << (vId[v]&0x1f);
				}
			}
			
			PfxTriangle triangle(
				decodedMesh.m_verts[vId[ids[0]]],
				decodedMesh.m_verts[vId[ids[1]]],
				decodedMesh.m_verts[vId[ids[2]]]);
			
			PfxVector3 normal = triangle.calcNormal();
			if(dot(normal,capsuleIn.m_startPosition-triangle.points[ 0 ]) <= 0.0f) {
				continue;
			}

			PfxFloat tmpSquaredDistance = (capsuleIn.m_halfLength + capsuleIn.m_radius) * (capsuleIn.m_halfLength + capsuleIn.m_radius);
			if(pfxIntersectMovingCapsuleTriangle(capsuleIn, triangle, contactPoint, cur_t, tmpSquaredDistance)) {
				PfxCapsuleOutput hit;
				
				hit.m_contactFlag = true;
				hit.m_variable = cur_t;
				hit.m_contactPoint.segment = capsuleIn.m_segment;
				hit.m_contactPoint.offset = contactPoint;
				
				if(cur_t == 0.0f) {
					PfxVector3 normal;
					PfxVector3 point = contactPoint;
					calcPointAndNormalOnCapsule(point, normal, capsuleIn);
					hit.m_contactPoint = point;
					hit.m_contactNormal = -normal;
				}
				else {
					hit.m_contactPoint = contactPoint;
					hit.m_contactNormal = -calcNormalOnCapsule(
						contactPoint - cur_t * capsuleIn.m_direction,
						capsuleIn);
				}
				
				PfxFloat s=0.0f,t=0.0f;
				pfxCalcBarycentricCoords(contactPoint,triangle,s,t);
				hit.m_subData.setType(PfxSubData::MESH_INFO);
				hit.m_subData.setFacetLocalS(s);
				hit.m_subData.setFacetLocalT(t);
				hit.m_subData.setFacetId(facetsIds[f]);
				hit.m_subData.setIslandId(islandId);
				hit.m_subData.setUserData(facet.m_userData);

				ret = callback(hit, userData);
				if(!ret) break;
			}
		}
	}
	
	return ret;
}

PfxBool pfxIntersectMovingCapsuleTriMesh(
	const PfxLargeTriMeshImpl *largeMesh,const PfxCompressedTriMesh *mesh,
	const PfxVector3 &rayStartLocal,const PfxVector3 &rayDirLocal,PfxFloat radiusLocal,
	const PfxCapsuleInputInternal &capsuleIn,
	const PfxTransform3 &transform,PfxBool flipTriangle,PfxUInt32 islandId,
	PfxBool(*callback)(const PfxCapsuleOutput &hit, void *userData), void *userData)
{
	(void) largeMesh;
	
	bool ret = true;
	
	PfxUInt32 ids[4] = {0,1,2,3};
	
	if(flipTriangle) {ids[0] = 2;ids[2] = 0;}
	
	EncStack encroot;
	encroot.nodeId = 0;
	encroot.aabbMin = pfxReadVector3(mesh->m_aabbMin);
	encroot.aabbMax = pfxReadVector3(mesh->m_aabbMax);
	
	PfxStaticStack<EncStack> bvhStack;
	bvhStack.push(encroot);
	
	PfxDecodedTriMesh decodedMesh;
	
	while(!bvhStack.empty()) {
		EncStack info = bvhStack.top();
		bvhStack.pop();

		PfxFacetBvhNode &encnode = ((PfxFacetBvhNode*)largeMesh->m_bvhNodeBuffer)[mesh->m_bvhNodes + info.nodeId];

		PfxVector3 quantizedMin(encnode.aabb[0],encnode.aabb[1],encnode.aabb[2]);
		PfxVector3 quantizedMax(encnode.aabb[3],encnode.aabb[4],encnode.aabb[5]);
		PfxVector3 aabbMinA = info.aabbMin + mulPerElem((info.aabbMax-info.aabbMin),quantizedMin / 255.0f);
		PfxVector3 aabbMaxA = info.aabbMax - mulPerElem((info.aabbMax-info.aabbMin),quantizedMax / 255.0f);

		PfxFloat cur_t = 1.0f;
		PfxVector3 contactPoint;

		if(!pfxIntersectRayAABBFast(rayStartLocal,rayDirLocal,(aabbMaxA+aabbMinA)*0.5f,(aabbMaxA-aabbMinA)*0.5f + PfxVector3(radiusLocal),cur_t)) {
			continue;
		}

		PfxUInt32 numFacets=0;
		PfxUInt32 facetsIds[2];

		EncStack encnext;
		encnext.aabbMin = aabbMinA;
		encnext.aabbMax = aabbMaxA;

		PfxUInt8 LStatus = encnode.flag & 0x03;
		PfxUInt8 RStatus = (encnode.flag & 0x0C) >> 2;

		if(LStatus == 0) {
			encnext.nodeId = encnode.left;
			bvhStack.push(encnext);
		}
		else if(LStatus == 1) {
			facetsIds[numFacets++] = encnode.left;
		}
		
		if(RStatus == 0) {
			encnext.nodeId = encnode.right;
			bvhStack.push(encnext);
		}
		else if(RStatus == 1) {
			facetsIds[numFacets++] = encnode.right;
		}
		
		for(PfxUInt32 f=0;f<numFacets;f++) {
			const PfxCompressedFacet2 &facet = ((PfxCompressedFacet2*)largeMesh->m_facetBuffer)[mesh->m_facets + facetsIds[f]];
			
			// デコード
			PfxDecodedFacet decodedFacet;
			const PfxUInt32 vId[4] = {facet.m_vertIds[0],facet.m_vertIds[1],facet.m_vertIds[2],facet.m_vertIds[3]};
			
			for(int v=0;v<4;v++) {
				if(!decodedMesh.isDecodedVertex(vId[v])) {
					decodedMesh.m_verts[vId[v]] = transform.getTranslation() + transform.getUpper3x3() * largeMesh->decodePosition(*((PfxQuantize3*)largeMesh->m_vertexBuffer + mesh->m_verts + vId[v]));
					decodedMesh.m_decodedVertex[vId[v]>>5] |= 1 << (vId[v]&0x1f);
				}
			}
			
			if(facet.m_facetInfo & 0x8000) {
				PfxTriangle triangle1(
					decodedMesh.m_verts[vId[ids[0]]],
					decodedMesh.m_verts[vId[ids[1]]],
					decodedMesh.m_verts[vId[ids[2]]]);
				
				PfxTriangle triangle2(
					decodedMesh.m_verts[vId[ids[2]]],
					decodedMesh.m_verts[vId[ids[3]]],
					decodedMesh.m_verts[vId[ids[0]]]);

				PfxVector3 normal1 = triangle1.calcNormal();
				PfxVector3 normal2 = triangle2.calcNormal();
				
				if(dot(normal1,capsuleIn.m_startPosition-triangle1.points[ 0 ]) > 0.0f) {
					PfxFloat tmpSquaredDistance = (capsuleIn.m_halfLength + capsuleIn.m_radius) * (capsuleIn.m_halfLength + capsuleIn.m_radius);
					if(pfxIntersectMovingCapsuleTriangle(capsuleIn, triangle1, contactPoint, cur_t, tmpSquaredDistance)) {
						PfxCapsuleOutput hit;
						
						hit.m_contactFlag = true;
						hit.m_variable = cur_t;
						hit.m_contactPoint.segment = capsuleIn.m_segment;
						hit.m_contactPoint.offset = contactPoint;
						
						if(cur_t == 0.0f) {
							PfxVector3 normal;
							PfxVector3 point = contactPoint;
							calcPointAndNormalOnCapsule(point, normal, capsuleIn);
							hit.m_contactPoint = point;
							hit.m_contactNormal = -normal;
						}
						else {
							hit.m_contactPoint = contactPoint;
							hit.m_contactNormal = -calcNormalOnCapsule(
								contactPoint - cur_t * capsuleIn.m_direction,
								capsuleIn);
						}
						
						PfxFloat s=0.0f,t=0.0f;
						pfxCalcBarycentricCoords(contactPoint,triangle1,s,t);
						hit.m_subData.setType(PfxSubData::MESH_INFO);
						hit.m_subData.setFacetLocalS(s);
						hit.m_subData.setFacetLocalT(t);
						hit.m_subData.setFacetId(facet.m_facetInfo&0xFF);
						hit.m_subData.setIslandId(islandId);
						hit.m_subData.setUserData(facet.m_userData[0]);
						
						ret = callback(hit, userData);
						if(!ret) break;
					}
				}

				if(dot(normal2,capsuleIn.m_startPosition-triangle2.points[ 0 ]) > 0.0f) {
					PfxFloat tmpSquaredDistance = (capsuleIn.m_halfLength + capsuleIn.m_radius) * (capsuleIn.m_halfLength + capsuleIn.m_radius);
					if(pfxIntersectMovingCapsuleTriangle(capsuleIn, triangle2, contactPoint, cur_t, tmpSquaredDistance)) {
						PfxCapsuleOutput hit;
						
						hit.m_contactFlag = true;
						hit.m_variable = cur_t;
						hit.m_contactPoint.segment = capsuleIn.m_segment;
						hit.m_contactPoint.offset = contactPoint;
						
						if(cur_t == 0.0f) {
							PfxVector3 normal;
							PfxVector3 point = contactPoint;
							calcPointAndNormalOnCapsule(point, normal, capsuleIn);
							hit.m_contactPoint = point;
							hit.m_contactNormal = -normal;
						}
						else {
							hit.m_contactPoint = contactPoint;
							hit.m_contactNormal = -calcNormalOnCapsule(
								contactPoint - cur_t * capsuleIn.m_direction,
								capsuleIn);
						}
						
						PfxFloat s=0.0f,t=0.0f;
						pfxCalcBarycentricCoords(contactPoint,triangle2,s,t);
						hit.m_subData.setType(PfxSubData::MESH_INFO);
						hit.m_subData.setFacetLocalS(s);
						hit.m_subData.setFacetLocalT(t);
						hit.m_subData.setFacetId((facet.m_facetInfo&0xFF)+1);
						hit.m_subData.setIslandId(islandId);
						hit.m_subData.setUserData(facet.m_userData[1]);
						
						ret = callback(hit, userData);
						if(!ret) break;
					}
				}
			}
			else {
				PfxTriangle triangle(
					decodedMesh.m_verts[vId[ids[0]]],
					decodedMesh.m_verts[vId[ids[1]]],
					decodedMesh.m_verts[vId[ids[2]]]);

				PfxVector3 normal = triangle.calcNormal();
				//PfxFloat angle = dot(normal,capsuleIn.m_direction);

				if(dot(normal,capsuleIn.m_startPosition-triangle.points[ 0 ]) > 0.0f) {
					PfxFloat tmpSquaredDistance = (capsuleIn.m_halfLength + capsuleIn.m_radius) * (capsuleIn.m_halfLength + capsuleIn.m_radius);
					if(pfxIntersectMovingCapsuleTriangle(capsuleIn, triangle, contactPoint, cur_t, tmpSquaredDistance)) {
						PfxCapsuleOutput hit;
						
						hit.m_contactFlag = true;
						hit.m_variable = cur_t;
						hit.m_contactPoint.segment = capsuleIn.m_segment;
						hit.m_contactPoint.offset = contactPoint;

						if(cur_t == 0.0f) {
							PfxVector3 normal;
							PfxVector3 point = contactPoint;
							calcPointAndNormalOnCapsule(point, normal, capsuleIn);
							hit.m_contactPoint = point;
							hit.m_contactNormal = -normal;
						}
						else {
							hit.m_contactPoint = contactPoint;
							hit.m_contactNormal = -calcNormalOnCapsule(
								contactPoint - cur_t * capsuleIn.m_direction,
								capsuleIn);
						}
						
						PfxFloat s=0.0f,t=0.0f;
						pfxCalcBarycentricCoords(contactPoint,triangle,s,t);
						hit.m_subData.setType(PfxSubData::MESH_INFO);
						hit.m_subData.setFacetLocalS(s);
						hit.m_subData.setFacetLocalT(t);
						hit.m_subData.setFacetId(facet.m_facetInfo&0xFF);
						hit.m_subData.setIslandId(islandId);
						hit.m_subData.setUserData(facet.m_userData[0]);
						
						ret = callback(hit, userData);
						if(!ret) break;
					}
				}
			}
		}
	}
	
	return ret;
}

PfxBool pfxIntersectMovingCapsuleAllFacetsInLargeTriMeshBvh(const PfxCapsuleInputInternal &capsuleIn,
	PfxBool(*callback)(const PfxCapsuleOutput &hit, void *userData), void *userData,
	const PfxLargeTriMeshImpl *largeMesh,const PfxTransform3 &transform,PfxBool flipTriangle,PfxFloat radiusLocal)
{
	// レイをラージメッシュのローカル座標へ変換
	PfxTransform3 transformLMesh = inverse(transform);
	PfxVector3 startPosLocal = transformLMesh.getUpper3x3() * capsuleIn.m_startPosition + transformLMesh.getTranslation();
	PfxVector3 rayDirLocal = transformLMesh.getUpper3x3() * capsuleIn.m_direction;
	
	{
		PfxStaticStack<EncStack> bvhStack;
		
		EncStack encroot;
		encroot.nodeId = 0;
		encroot.aabbMin = largeMesh->getOffset() - largeMesh->getHalf();
		encroot.aabbMax = largeMesh->getOffset() + largeMesh->getHalf();
		
		bvhStack.push(encroot);
		
		while( ! bvhStack.empty() ) {
			EncStack info = bvhStack.top();
			bvhStack.pop();
			
			PfxIslandBvhNode &encnode = largeMesh->m_bvhNodes[info.nodeId];

			PfxVector3 quantizedMin(encnode.aabb[0],encnode.aabb[1],encnode.aabb[2]);
			PfxVector3 quantizedMax(encnode.aabb[3],encnode.aabb[4],encnode.aabb[5]);
			PfxVector3 aabbMinA = info.aabbMin + mulPerElem((info.aabbMax-info.aabbMin),quantizedMin / 255.0f);
			PfxVector3 aabbMaxA = info.aabbMax - mulPerElem((info.aabbMax-info.aabbMin),quantizedMax / 255.0f);
			
			PfxFloat tmpVariable = 1.0f;
			
			if( !pfxIntersectRayAABBFast( startPosLocal,rayDirLocal, (aabbMaxA+aabbMinA)*0.5f, (aabbMaxA-aabbMinA)*0.5f + PfxVector3(radiusLocal), tmpVariable) )
				continue;
			
			//	informations about back-tracking trees
			EncStack encnext;
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
				
				//	now, check whether this one intersects the island
				if(largeMesh->m_type == SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION) {
					PfxCompressedTriMesh *island = (PfxCompressedTriMesh*)largeMesh->m_islands + islandId;
					PfxVector3 islandAabbMin = pfxReadVector3(island->m_aabbMin);
					PfxVector3 islandAabbMax = pfxReadVector3(island->m_aabbMax);
					
					if( !pfxIntersectRayAABBFast( startPosLocal,rayDirLocal, (islandAabbMax+islandAabbMin)*0.5f, (islandAabbMax-islandAabbMin)*0.5f + PfxVector3(radiusLocal), tmpVariable) )
						continue;
					
					PfxBool ret = pfxIntersectMovingCapsuleTriMesh(
						largeMesh,(PfxCompressedTriMesh*)island,
						startPosLocal,rayDirLocal,radiusLocal, // Moving capsule in the mesh local coordinates
						capsuleIn, // Moving capsule in the world coordinates
						transform,flipTriangle,islandId,callback,userData);
					if(!ret) return ret;
				}
				else if(largeMesh->m_type & 0x01) {
					PfxQuantizedTriMeshBvh *island = (PfxQuantizedTriMeshBvh*)largeMesh->m_islands + islandId;
					PfxVector3 islandAabbMin = pfxReadVector3(island->m_aabbMin);
					PfxVector3 islandAabbMax = pfxReadVector3(island->m_aabbMax);

					if( !pfxIntersectRayAABBFast( startPosLocal,rayDirLocal, (islandAabbMax+islandAabbMin)*0.5f, (islandAabbMax-islandAabbMin)*0.5f + PfxVector3(radiusLocal), tmpVariable) )
						continue;

					PfxBool ret = pfxIntersectMovingCapsuleTriMesh(
						largeMesh,(PfxQuantizedTriMeshBvh*)island,
						startPosLocal,rayDirLocal,radiusLocal, // Moving capsule in the mesh local coordinates
						capsuleIn, // Moving capsule in the world coordinates
						transform,flipTriangle,islandId,callback,userData);
					if(!ret) return ret;
				}
			}
		}
	}

	return true;
}

PfxBool pfxIntersectMovingCapsuleAllFacetsInLargeTriMeshArray(const PfxCapsuleInputInternal &capsuleIn,
	PfxBool(*callback)(const PfxCapsuleOutput &hit, void *userData), void *userData,
	const PfxLargeTriMeshImpl *largeMesh,const PfxTransform3 &transform,PfxBool flipTriangle,PfxFloat radiusLocal)
{
	// レイをラージメッシュのローカル座標へ変換
	PfxTransform3 transformLMesh = inverse(transform);
	PfxVector3 startPosLocal = transformLMesh.getUpper3x3() * capsuleIn.m_startPosition + transformLMesh.getTranslation();
	PfxVector3 rayDirLocal = transformLMesh.getUpper3x3() * capsuleIn.m_direction;
	
	PfxUInt32 numIslands = largeMesh->m_numIslands;
	for(PfxUInt32 i=0;i<numIslands;i++) {
		PfxAabb16 aabbB = largeMesh->m_aabbList[i];
		
		PfxVector3 aabbMin,aabbMax;
		aabbMin = largeMesh->getWorldPosition(PfxVecInt3((PfxFloat)pfxGetXMin(aabbB),(PfxFloat)pfxGetYMin(aabbB),(PfxFloat)pfxGetZMin(aabbB)));
		aabbMax = largeMesh->getWorldPosition(PfxVecInt3((PfxFloat)pfxGetXMax(aabbB),(PfxFloat)pfxGetYMax(aabbB),(PfxFloat)pfxGetZMax(aabbB)));
		
		PfxFloat tmpVariable = 1.0f;
		
		if( !pfxIntersectRayAABBFast(startPosLocal,rayDirLocal,(aabbMax+aabbMin)*0.5f,(aabbMax-aabbMin)*0.5f + PfxVector3(radiusLocal),tmpVariable) )
			continue;
		
		// アイランドとの交差チェック
		void *island=((PfxExpandedTriMesh*)largeMesh->m_islands) + i;
		
		PfxBool ret = pfxIntersectMovingCapsuleTriMesh(
			largeMesh,(PfxExpandedTriMesh*)island,
			startPosLocal,rayDirLocal,radiusLocal, // Moving capsule in the mesh local coordinates
			capsuleIn, // Moving capsule in the world coordinates
			transform,flipTriangle,i,callback,userData);
		if(!ret) return ret;
	}
	
	return true;
}

PfxBool pfxIntersectMovingCapsuleAllFacetsInLargeTriMesh(
	const PfxCapsuleInputInternal &capsuleIn,
	PfxBool(*callback)(const PfxCapsuleOutput &hit, void *userData),
	void *userData,
	const void *shape,
	const PfxTransform3 &transform,
	PfxBool flipTriangle,
	PfxFloat radiusLocal)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	
	if(largeMesh->isUsingBvh()) {
		return pfxIntersectMovingCapsuleAllFacetsInLargeTriMeshBvh( capsuleIn, callback, userData, largeMesh, transform, flipTriangle, radiusLocal );
	}
	
	return pfxIntersectMovingCapsuleAllFacetsInLargeTriMeshArray( capsuleIn, callback, userData, largeMesh, transform, flipTriangle, radiusLocal );
}

} //namespace pfxv4
} //namespace sce
