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
#include "pfx_intersect_moving_circle_all_facets.h"
#include <float.h>

namespace sce {
namespace pfxv4 {

// defined in pfx_intersect_moving_circle_convex.cpp
extern PfxBool pfxIntersectMovingCircleTriangle(const PfxVector3 &rayStartPosition,const PfxVector3 &rayDirection,PfxFloat rayRadius,const PfxTriangle &triangle,PfxFloat &variable,PfxFloat &squaredDistance);

struct EncStack {
	PfxUInt32 nodeId;
	PfxVector3 aabbMin;
	PfxVector3 aabbMax;
};

PfxBool pfxIntersectMovingCircleTriMesh(
	const PfxLargeTriMeshImpl *largeMesh,const PfxExpandedTriMesh *mesh,
	const PfxVector3 &rayStartLocal,const PfxVector3 &rayDirLocal,PfxFloat radiusLocal,
	const PfxCircleInputInternal &circleIn,
	const PfxTransform3 &transform,PfxBool flipTriangle,PfxUInt32 islandId,
	PfxBool(*callback)(const PfxCircleOutput &hit, void *userData), void *userData)
{
	(void) largeMesh;
	
	bool ret = true;
/*	
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
		//  only checking the ray direction against the normal will make test fail if the circle collides with the triangle
		if(dot(normal,circleIn.m_startPosition-triangle.points[ 0 ]) <= 0.0f) {	//  
			continue;											//  
		}														//  
		
		PfxVector3 facetAabbMin = minPerElem(minPerElem(triangle.points[0],triangle.points[1]),triangle.points[2]) - PfxVector3(circleIn.m_radius);
		PfxVector3 facetAabbMax = maxPerElem(maxPerElem(triangle.points[0],triangle.points[1]),triangle.points[2]) + PfxVector3(circleIn.m_radius);
		
		PfxFloat cur_t = 1.0f;
		
		if( !pfxIntersectRayAABBFast(circleIn.m_startPosition,circleIn.m_direction,0.5f * (facetAabbMax + facetAabbMin),0.5f * (facetAabbMax - facetAabbMin),cur_t) )
			continue;
		
		//  The distance between circle and triangle will be equal to radius if the circle touches the triangle and between [0,radius] if the circle penetrates the triangle (which
		// is only possible if the circle intersects at t=0). If we intersect at t=0 we want to report the contact with deepest penetration.
		PfxFloat tmpSquaredDistance(circleIn.m_radius*circleIn.m_radius);																// 
		if(pfxIntersectMovingCircleTriangle(circleIn.m_startPosition,circleIn.m_direction,circleIn.m_radius,triangle,cur_t,tmpSquaredDistance)) {        // 
			PfxVector3 stopPoint = circleIn.m_startPosition + cur_t * circleIn.m_direction;
			
			PfxVector3 contactPoint;
			pfxClosestPointTriangle(stopPoint,triangle,contactPoint);
			
			PfxCircleOutput hit;
			
			hit.m_contactFlag = true;
			hit.m_variable = cur_t;
			hit.m_contactPoint.segment = circleIn.m_segment;
			hit.m_contactPoint.offset = contactPoint;
			
			PfxVector3 normal = stopPoint - contactPoint;
			PfxFloat distanceSqr = lengthSqr(normal);
			if(distanceSqr < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
				hit.m_contactNormal = triangle.calcNormal();
			}
			else {
				hit.m_contactNormal = normal / sqrtf(distanceSqr);
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
*/ 	
	return ret;
}

PfxBool pfxIntersectMovingCircleTriMesh(
	const PfxLargeTriMeshImpl *largeMesh,const PfxQuantizedTriMeshBvh *mesh,
	const PfxVector3 &rayStartLocal,const PfxVector3 &rayDirLocal,PfxFloat radiusLocal,
	const PfxCircleInputInternal &circleIn,
	const PfxTransform3 &transform,PfxBool flipTriangle,PfxUInt32 islandId,
	PfxBool(*callback)(const PfxCircleOutput &hit, void *userData), void *userData)
{
	(void) largeMesh;
	
	bool ret = true;
	
/*	PfxUInt32 ids[3] = {0,1,2};
	
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
			//  only checking the ray direction against the normal will make test fail if the circle collides with the triangle
			if(dot(normal,circleIn.m_startPosition-triangle.points[ 0 ]) <= 0.0f) {	//  
				continue;											//  
			}														//  

			//  The distance between circle and triangle will be equal to radius if the circle touches the triangle and between [0,radius] if the circle penetrates the triangle (which
			// is only possible if the circle intersects at t=0). If we intersect at t=0 we want to report the contact with deepest penetration.
			PfxFloat tmpSquaredDistance(circleIn.m_radius*circleIn.m_radius);																// 
			if(pfxIntersectMovingCircleTriangle(circleIn.m_startPosition,circleIn.m_direction,circleIn.m_radius,triangle,cur_t,tmpSquaredDistance)) {        // 
				PfxVector3 stopPoint = circleIn.m_startPosition + cur_t * circleIn.m_direction;
				
				PfxVector3 contactPoint;
				pfxClosestPointTriangle(stopPoint,triangle,contactPoint);
				
				PfxCircleOutput hit;
				
				hit.m_contactFlag = true;
				hit.m_variable = cur_t;
				hit.m_contactPoint.segment = circleIn.m_segment;
				hit.m_contactPoint.offset = contactPoint;
				
				PfxVector3 normal = stopPoint - contactPoint;
				PfxFloat distanceSqr = lengthSqr(normal);
				if(distanceSqr < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
					hit.m_contactNormal = triangle.calcNormal();
				}
				else {
					hit.m_contactNormal = normal / sqrtf(distanceSqr);
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
*/	
	return ret;
}

PfxBool pfxIntersectMovingCircleTriMesh(
	const PfxLargeTriMeshImpl *largeMesh,const PfxCompressedTriMesh *mesh,
	const PfxVector3 &rayStartLocal,const PfxVector3 &rayDirLocal,PfxFloat radiusLocal,
	const PfxCircleInputInternal &circleIn,
	const PfxTransform3 &transform,PfxBool flipTriangle,PfxUInt32 islandId,
	PfxBool(*callback)(const PfxCircleOutput &hit, void *userData), void *userData)
{
	(void) largeMesh;
	
	bool ret = true;
	
/*	PfxUInt32 ids[4] = {0,1,2,3};
	
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
				
				//  only checking the ray direction against the normal will make test fail if the circle collides with the triangle
				if(dot(normal1,circleIn.m_startPosition-triangle1.points[ 0 ]) > 0.0f) { //  
					//  The distance between circle and triangle will be equal to radius if the circle touches the triangle and between [0,radius] if the circle penetrates the triangle (which
					// is only possible if the circle intersects at t=0). If we intersect at t=0 we want to report the contact with deepest penetration.
					PfxFloat tmpSquaredDistance(circleIn.m_radius*circleIn.m_radius);																// 
					if(pfxIntersectMovingCircleTriangle(circleIn.m_startPosition,circleIn.m_direction,circleIn.m_radius,triangle1,cur_t,tmpSquaredDistance)) {        // 
						PfxVector3 stopPoint = circleIn.m_startPosition + cur_t * circleIn.m_direction;
						
						PfxVector3 contactPoint;
						pfxClosestPointTriangle(stopPoint,triangle1,contactPoint);
						
						PfxCircleOutput hit;
						
						hit.m_contactFlag = true;
						hit.m_variable = cur_t;
						hit.m_contactPoint.segment = circleIn.m_segment;
						hit.m_contactPoint.offset = contactPoint;

						PfxVector3 normal = stopPoint - contactPoint;
						PfxFloat distanceSqr = lengthSqr(normal);
						if(distanceSqr < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
							hit.m_contactNormal = triangle1.calcNormal();
						}
						else {
							hit.m_contactNormal = normal / sqrtf(distanceSqr);
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

				//  only checking the ray direction against the normal will make test fail if the circle collides with the triangle
				if(dot(normal2,circleIn.m_startPosition-triangle2.points[ 0 ]) > 0.0f) { //  
					//  The distance between circle and triangle will be equal to radius if the circle touches the triangle and between [0,radius] if the circle penetrates the triangle (which
					// is only possible if the circle intersects at t=0). If we intersect at t=0 we want to report the contact with deepest penetration.
					PfxFloat tmpSquaredDistance(circleIn.m_radius*circleIn.m_radius);																// 
					if(pfxIntersectMovingCircleTriangle(circleIn.m_startPosition,circleIn.m_direction,circleIn.m_radius,triangle2,cur_t,tmpSquaredDistance)) {        // 
						PfxVector3 stopPoint = circleIn.m_startPosition + cur_t * circleIn.m_direction;
						
						PfxVector3 contactPoint;
						pfxClosestPointTriangle(stopPoint,triangle2,contactPoint);
						
						PfxCircleOutput hit;
						
						hit.m_contactFlag = true;
						hit.m_variable = cur_t;
						hit.m_contactPoint.segment = circleIn.m_segment;
						hit.m_contactPoint.offset = contactPoint;

						PfxVector3 normal = stopPoint - contactPoint;
						PfxFloat distanceSqr = lengthSqr(normal);
						if(distanceSqr < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
							hit.m_contactNormal = triangle2.calcNormal();
						}
						else {
							hit.m_contactNormal = normal / sqrtf(distanceSqr);
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
				//PfxFloat angle = dot(normal,circleIn.m_direction);

				//  only checking the ray direction against the normal will make test fail if the circle collides with the triangle
				if(dot(normal,circleIn.m_startPosition-triangle.points[ 0 ]) > 0.0f) { //  
					//  The distance between circle and triangle will be equal to radius if the circle touches the triangle and between [0,radius] if the circle penetrates the triangle (which
					// is only possible if the circle intersects at t=0). If we intersect at t=0 we want to report the contact with deepest penetration.
					PfxFloat tmpSquaredDistance(circleIn.m_radius*circleIn.m_radius);																// 
					if(pfxIntersectMovingCircleTriangle(circleIn.m_startPosition,circleIn.m_direction,circleIn.m_radius,triangle,cur_t,tmpSquaredDistance)) {        // 
						PfxVector3 stopPoint = circleIn.m_startPosition + cur_t * circleIn.m_direction;
						
						PfxVector3 contactPoint;
						pfxClosestPointTriangle(stopPoint,triangle,contactPoint);
						
						PfxCircleOutput hit;
						
						hit.m_contactFlag = true;
						hit.m_variable = cur_t;
						hit.m_contactPoint.segment = circleIn.m_segment;
						hit.m_contactPoint.offset = contactPoint;

						PfxVector3 normal = stopPoint - contactPoint;
						PfxFloat distanceSqr = lengthSqr(normal);
						if(distanceSqr < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
							hit.m_contactNormal = triangle.calcNormal();
						}
						else {
							hit.m_contactNormal = normal / sqrtf(distanceSqr);
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
*/	
	return ret;
}

PfxBool pfxIntersectMovingCircleAllFacetsInLargeTriMeshBvh(const PfxCircleInputInternal &circleIn,
	PfxBool(*callback)(const PfxCircleOutput &hit, void *userData), void *userData,
	const PfxLargeTriMeshImpl *largeMesh,const PfxTransform3 &transform,PfxBool flipTriangle,PfxFloat radiusLocal)
{
/*	// レイをラージメッシュのローカル座標へ変換
	PfxTransform3 transformLMesh = inverse(transform);
	PfxVector3 startPosLocal = transformLMesh.getUpper3x3() * circleIn.m_startPosition + transformLMesh.getTranslation();
	PfxVector3 rayDirLocal = transformLMesh.getUpper3x3() * circleIn.m_direction;
	
	{
		PfxStaticStack<EncStack> bvhStack;
		
		EncStack encroot;
		encroot.nodeId = 0;
		encroot.aabbMin = largeMesh->m_offset - largeMesh->m_half;
		encroot.aabbMax = largeMesh->m_offset + largeMesh->m_half;
		
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
					
					PfxBool ret = pfxIntersectMovingCircleTriMesh(
						largeMesh,(PfxCompressedTriMesh*)island,
						startPosLocal,rayDirLocal,radiusLocal, // Moving circle in the mesh local coordinates
						circleIn, // Moving circle in the world coordinates
						transform,flipTriangle,islandId,callback,userData);
					if(!ret) return ret;
				}
				else if(largeMesh->m_type & 0x01) {
					PfxQuantizedTriMeshBvh *island = (PfxQuantizedTriMeshBvh*)largeMesh->m_islands + islandId;
					PfxVector3 islandAabbMin = pfxReadVector3(island->m_aabbMin);
					PfxVector3 islandAabbMax = pfxReadVector3(island->m_aabbMax);

					if( !pfxIntersectRayAABBFast( startPosLocal,rayDirLocal, (islandAabbMax+islandAabbMin)*0.5f, (islandAabbMax-islandAabbMin)*0.5f + PfxVector3(radiusLocal), tmpVariable) )
						continue;

					PfxBool ret = pfxIntersectMovingCircleTriMesh(
						largeMesh,(PfxQuantizedTriMeshBvh*)island,
						startPosLocal,rayDirLocal,radiusLocal, // Moving circle in the mesh local coordinates
						circleIn, // Moving circle in the world coordinates
						transform,flipTriangle,islandId,callback,userData);
					if(!ret) return ret;
				}
				else {
					PfxExpandedTriMeshBvh *island = (PfxExpandedTriMeshBvh*)largeMesh->m_islands + islandId;
					PfxVector3 islandAabbMin = pfxReadVector3(island->m_aabbMin);
					PfxVector3 islandAabbMax = pfxReadVector3(island->m_aabbMax);

					if( !pfxIntersectRayAABBFast( startPosLocal,rayDirLocal, (islandAabbMax+islandAabbMin)*0.5f, (islandAabbMax-islandAabbMin)*0.5f + PfxVector3(radiusLocal), tmpVariable) )
						continue;
					
					PfxBool ret = pfxIntersectMovingCircleTriMesh(
						largeMesh,(PfxExpandedTriMeshBvh*)island,
						startPosLocal,rayDirLocal,radiusLocal, // Moving circle in the mesh local coordinates
						circleIn, // Moving circle in the world coordinates
						transform,flipTriangle,islandId,callback,userData);
					if(!ret) return ret;
				}
			}
		}
	}
*/
	return true;
}

PfxBool pfxIntersectMovingCircleAllFacetsInLargeTriMeshArray(const PfxCircleInputInternal &circleIn,
	PfxBool(*callback)(const PfxCircleOutput &hit, void *userData), void *userData,
	const PfxLargeTriMeshImpl *largeMesh,const PfxTransform3 &transform,PfxBool flipTriangle,PfxFloat radiusLocal)
{
/*	// レイをラージメッシュのローカル座標へ変換
	PfxTransform3 transformLMesh = inverse(transform);
	PfxVector3 startPosLocal = transformLMesh.getUpper3x3() * circleIn.m_startPosition + transformLMesh.getTranslation();
	PfxVector3 rayDirLocal = transformLMesh.getUpper3x3() * circleIn.m_direction;
	
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
		void *island=NULL;
		if(largeMesh->m_type & 0x01) {
			island = ((PfxQuantizedTriMesh*)largeMesh->m_islands) + i;
		}
		else {
			island = ((PfxExpandedTriMesh*)largeMesh->m_islands) + i;
		}
		
		if(largeMesh->m_type & 0x01) {
			PfxBool ret = pfxIntersectMovingCircleTriMesh(
				largeMesh,(PfxQuantizedTriMesh*)island,
				startPosLocal,rayDirLocal,radiusLocal, // Moving circle in the mesh local coordinates
				circleIn, // Moving circle in the world coordinates
				transform,flipTriangle,i,callback,userData);
			if(!ret) return ret;
		}
		else {
			PfxBool ret = pfxIntersectMovingCircleTriMesh(
				largeMesh,(PfxExpandedTriMesh*)island,
				startPosLocal,rayDirLocal,radiusLocal, // Moving circle in the mesh local coordinates
				circleIn, // Moving circle in the world coordinates
				transform,flipTriangle,i,callback,userData);
			if(!ret) return ret;
		}
	}
*/	
	return true;
}

PfxBool pfxIntersectMovingCircleAllFacetsInLargeTriMesh(
	const PfxCircleInputInternal &circleIn,
	PfxBool(*callback)(const PfxCircleOutput &hit, void *userData),
	void *userData,
	const void *shape,
	const PfxTransform3 &transform,
	PfxBool flipTriangle,
	PfxFloat radiusLocal)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	
	if(largeMesh->isUsingBvh()) {
		return pfxIntersectMovingCircleAllFacetsInLargeTriMeshBvh( circleIn, callback, userData, largeMesh, transform, flipTriangle, radiusLocal );
	}
	
	return pfxIntersectMovingCircleAllFacetsInLargeTriMeshArray( circleIn, callback, userData, largeMesh, transform, flipTriangle, radiusLocal );
}

} //namespace pfxv4
} //namespace sce
