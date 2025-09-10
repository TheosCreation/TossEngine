/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/util/pfx_static_array.h"
#include "pfx_large_tri_mesh_impl.h"
#include "pfx_intersect_common.h"
#include "pfx_mesh_common.h"
#include "pfx_decoded_tri_mesh.h"
#include "pfx_intersect_ray_large_tri_mesh.h"

namespace sce {
namespace pfxv4 {

struct EncStack {
	PfxUInt32 nodeId;
	PfxVector3 aabbMin;
	PfxVector3 aabbMax;
};

PfxBool pfxIntersectRayTriMesh(
	const PfxLargeTriMeshImpl *largeMesh,const PfxExpandedTriMesh *mesh,
	const PfxVector3 &rayStart,const PfxVector3 &rayDir,
	const PfxRayInputInternal &ray,const PfxMatrix3 &rotation_n,
	pfxRayHitCallback callback, void *userData,
	PfxSubData &subData)
{
	(void) largeMesh;
	bool ret = true;

	//-------------------------------------------
	// 判定する面を絞り込む

	PfxUInt8 selFacets[SCE_PFX_NUMMESHFACETS] = {0};

	PfxVector3 aabbMin = minPerElem(rayStart,rayStart + rayDir);
	PfxVector3 aabbMax = maxPerElem(rayStart,rayStart + rayDir);
	PfxUInt32 numSelFacets = pfxGatherFacets(mesh,0.5f*(aabbMin+aabbMax),0.5f*(aabbMax-aabbMin),selFacets);

	if(numSelFacets == 0) {
		return ret;
	}

	//-------------------------------------------
	// レイ交差判定
	
	for(PfxUInt32 f = 0; f < numSelFacets; f++) {
		const PfxExpandedFacet &facet = mesh->m_facets[selFacets[f]];
		
		PfxFloat cur_t = 1.0f;

		PfxVector3 cnt = pfxReadVector3((PfxFloat*)&facet.m_center);
		PfxVector3 hlf = pfxReadVector3((PfxFloat*)&facet.m_half);
		if( !pfxIntersectRayAABBFast(rayStart,rayDir,cnt,hlf,cur_t) )
			continue;
		
		PfxTriangle triangle(
			mesh->m_verts[facet.m_vertIds[0]],
			mesh->m_verts[facet.m_vertIds[1]],
			mesh->m_verts[facet.m_vertIds[2]]);
	
		if(pfxIntersectRayTriangle(rayStart,rayDir,ray.m_facetMode,triangle,cur_t)) {
			PfxRayOutput hit;
			
			PfxFloat s=0.0f,t=0.0f;
			pfxCalcBarycentricCoords(rayStart+cur_t*rayDir,triangle,s,t);
			
			subData.setType(PfxSubData::MESH_INFO);
			subData.setFacetLocalS(s);
			subData.setFacetLocalT(t);
			subData.setFacetId(selFacets[f]);
			subData.setUserData(facet.m_userData);
			
			hit.m_contactPoint.segment = ray.m_segment;
			hit.m_contactPoint.offset = ray.m_startPosition + cur_t * ray.m_direction;
			hit.m_contactFlag = true;
			hit.m_variable = cur_t;
			hit.m_contactNormal = normalize(rotation_n *  facet.m_normal);
			hit.m_subData = subData;
			
			ret = callback(hit, userData);
			if(!ret) break;
		}
	}
	
	return ret;
}

PfxBool pfxIntersectRayTriMesh(
	const PfxLargeTriMeshImpl *largeMesh,const PfxQuantizedTriMeshBvh *mesh,
	const PfxVector3 &rayStart,const PfxVector3 &rayDir,
	const PfxRayInputInternal &ray,const PfxMatrix3 &rotation_n,
	pfxRayHitCallback callback, void *userData,
	PfxSubData &subData)
{
	bool ret = true;

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

		if(!pfxIntersectRayAABBFast(rayStart,rayDir,(aabbMaxA+aabbMinA)*0.5f,(aabbMaxA-aabbMinA)*0.5f,cur_t)) {
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
					decodedMesh.m_verts[vId[v]] = largeMesh->decodePosition(mesh->m_verts[vId[v]]);
					decodedMesh.m_decodedVertex[vId[v]>>5] |= 1 << (vId[v]&0x1f);
				}
			}
			
			PfxTriangle triangle(
				decodedMesh.m_verts[vId[0]],
				decodedMesh.m_verts[vId[1]],
				decodedMesh.m_verts[vId[2]]);
			
			if(pfxIntersectRayTriangle(rayStart,rayDir,ray.m_facetMode,triangle,cur_t)) {
				PfxRayOutput hit;
				
				PfxFloat s=0.0f,t=0.0f;
				pfxCalcBarycentricCoords(rayStart+cur_t*rayDir,triangle,s,t);
				
				subData.setType(PfxSubData::MESH_INFO);
				subData.setFacetLocalS(s);
				subData.setFacetLocalT(t);
				subData.setFacetId(facetsIds[f]);
				subData.setUserData(facet.m_userData);
				
				hit.m_contactPoint.segment = ray.m_segment;
				hit.m_contactPoint.offset = ray.m_startPosition + cur_t * ray.m_direction;
				hit.m_contactFlag = true;
				hit.m_variable = cur_t;
				hit.m_contactNormal = normalize(rotation_n * largeMesh->decodeNormal(facet.m_normal));
				hit.m_subData = subData;
				
				ret = callback(hit, userData);
				if(!ret) break;
			}
		}
	}
	
	return ret;
}

PfxBool pfxIntersectRayTriMesh(
	const PfxLargeTriMeshImpl *largeMesh,const PfxCompressedTriMesh *mesh,
	const PfxVector3 &rayStart,const PfxVector3 &rayDir,
	const PfxRayInputInternal &ray,const PfxMatrix3 &rotation_n,
	pfxRayHitCallback callback, void *userData,
	PfxSubData &subData)
{
	bool ret = true;
	
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

		if(!pfxIntersectRayAABBFast(rayStart,rayDir,(aabbMaxA+aabbMinA)*0.5f,(aabbMaxA-aabbMinA)*0.5f,cur_t)) {
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
					decodedMesh.m_verts[vId[v]] = largeMesh->decodePosition(*((PfxQuantize3*)largeMesh->m_vertexBuffer + mesh->m_verts + vId[v]));
					decodedMesh.m_decodedVertex[vId[v]>>5] |= 1 << (vId[v]&0x1f);
				}
			}
			
			if(facet.m_facetInfo & 0x8000) {
				PfxTriangle triangle1(
					decodedMesh.m_verts[vId[0]],
					decodedMesh.m_verts[vId[1]],
					decodedMesh.m_verts[vId[2]]);

				PfxTriangle triangle2(
					decodedMesh.m_verts[vId[2]],
					decodedMesh.m_verts[vId[3]],
					decodedMesh.m_verts[vId[0]]);

				if (pfxIntersectRayTriangle(rayStart, rayDir, ray.m_facetMode, triangle1, cur_t)) {
					PfxRayOutput hit;

					PfxFloat s = 0.0f, t = 0.0f;
					pfxCalcBarycentricCoords(rayStart + cur_t*rayDir, triangle1, s, t);

					subData.setType(PfxSubData::MESH_INFO);
					subData.setFacetLocalS(s);
					subData.setFacetLocalT(t);
					subData.setFacetId(facet.m_facetInfo & 0xFF);
					subData.setUserData(facet.m_userData[0]);

					hit.m_contactPoint.segment = ray.m_segment;
					hit.m_contactPoint.offset = ray.m_startPosition + cur_t * ray.m_direction;
					hit.m_contactFlag = true;
					hit.m_variable = cur_t;
					hit.m_contactNormal = normalize(rotation_n * cross(decodedMesh.m_verts[vId[1]] - decodedMesh.m_verts[vId[0]], decodedMesh.m_verts[vId[2]] - decodedMesh.m_verts[vId[0]]));
					hit.m_subData = subData;

					ret = callback(hit, userData);
					if (!ret) break;
				}
				
				if (pfxIntersectRayTriangle(rayStart, rayDir, ray.m_facetMode, triangle2, cur_t)) {
					PfxRayOutput hit;

					PfxFloat s = 0.0f, t = 0.0f;
					pfxCalcBarycentricCoords(rayStart + cur_t*rayDir, triangle2, s, t);

					subData.setType(PfxSubData::MESH_INFO);
					subData.setFacetLocalS(s);
					subData.setFacetLocalT(t);
					subData.setFacetId((facet.m_facetInfo & 0xFF) + 1);
					subData.setUserData(facet.m_userData[1]);

					hit.m_contactPoint.segment = ray.m_segment;
					hit.m_contactPoint.offset = ray.m_startPosition + cur_t * ray.m_direction;
					hit.m_contactFlag = true;
					hit.m_variable = cur_t;
					hit.m_contactNormal = normalize(rotation_n * cross(decodedMesh.m_verts[vId[3]] - decodedMesh.m_verts[vId[2]], decodedMesh.m_verts[vId[0]] - decodedMesh.m_verts[vId[2]]));
					hit.m_subData = subData;

					ret = callback(hit, userData);
					if (!ret) break;
				}
			}
			else {
				PfxTriangle triangle(
					decodedMesh.m_verts[vId[0]],
					decodedMesh.m_verts[vId[1]],
					decodedMesh.m_verts[vId[2]]);

				if(pfxIntersectRayTriangle(rayStart,rayDir,ray.m_facetMode,triangle,cur_t)) {
					PfxRayOutput hit;
					
					PfxFloat s=0.0f,t=0.0f;
					pfxCalcBarycentricCoords(rayStart+cur_t*rayDir,triangle,s,t);
					
					subData.setType(PfxSubData::MESH_INFO);
					subData.setFacetLocalS(s);
					subData.setFacetLocalT(t);
					subData.setFacetId(facet.m_facetInfo&0xFF);
					subData.setUserData(facet.m_userData[0]);
					
					hit.m_contactPoint.segment = ray.m_segment;
					hit.m_contactPoint.offset = ray.m_startPosition + cur_t * ray.m_direction;
					hit.m_contactFlag = true;
					hit.m_variable = cur_t;
					hit.m_contactNormal = normalize(rotation_n * cross(decodedMesh.m_verts[vId[1]]-decodedMesh.m_verts[vId[0]],decodedMesh.m_verts[vId[2]]-decodedMesh.m_verts[vId[0]]));
					hit.m_subData = subData;
					
					ret = callback(hit, userData);
					if(!ret) break;
				}
			}
		}
	}
	
	return ret;
}

PfxBool pfxIntersectRayAllFacetsInLargeTriMeshBvh(
	const PfxRayInputInternal &ray,
	pfxRayHitCallback callback, void *userData,
	const PfxLargeTriMeshImpl *largeMesh, 
	const PfxTransform3 &transform)
{
	// レイをラージメッシュのローカル座標へ変換
	PfxTransform3 transformLMesh = inverse(transform);
	PfxVector3 rayStartPosition = transformLMesh.getUpper3x3() * ray.m_startPosition + transformLMesh.getTranslation();
	PfxVector3 rayDirection = transformLMesh.getUpper3x3() * ray.m_direction;
	PfxVector3 rayEndPosition = rayStartPosition+rayDirection;
	PfxMatrix3 rotation_n = transpose(transformLMesh.getUpper3x3()); // 法線をワールドへ変換するマトリクス
	
	PfxVector3 aabbMinB = minPerElem(rayStartPosition,rayEndPosition);
	PfxVector3 aabbMaxB = maxPerElem(rayStartPosition,rayEndPosition);
	
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
			
			if(aabbMaxA[0] < aabbMinB[0] || aabbMinA[0] > aabbMaxB[0]) continue;
			if(aabbMaxA[1] < aabbMinB[1] || aabbMinA[1] > aabbMaxB[1]) continue;
			if(aabbMaxA[2] < aabbMinB[2] || aabbMinA[2] > aabbMaxB[2]) continue;

			PfxFloat tmpVariable = 1.0f;
			if( !pfxIntersectRayAABBFast( rayStartPosition,rayDirection, (aabbMaxA+aabbMinA)*0.5f, (aabbMaxA-aabbMinA)*0.5f, tmpVariable) ) {
				continue;
			}

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
				PfxSubData subData;
				subData.setIslandId(islandId);
				
				if(largeMesh->m_type == SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION) {
					PfxCompressedTriMesh *island = (PfxCompressedTriMesh*)largeMesh->m_islands + islandId;
					PfxVector3 islandAabbMin = pfxReadVector3(island->m_aabbMin);
					PfxVector3 islandAabbMax = pfxReadVector3(island->m_aabbMax);

					if( !pfxIntersectRayAABBFast( rayStartPosition,rayDirection, (islandAabbMax+islandAabbMin)*0.5f, (islandAabbMax-islandAabbMin)*0.5f, tmpVariable) ) {
						continue;
					}
					
					PfxBool ret = pfxIntersectRayTriMesh(largeMesh,island,rayStartPosition,rayDirection,ray,rotation_n,callback,userData,subData);
					if(!ret) return ret;
				}
				else if(largeMesh->m_type & 0x01) {
					PfxQuantizedTriMeshBvh *island = (PfxQuantizedTriMeshBvh*)largeMesh->m_islands + islandId;
					PfxVector3 islandAabbMin = pfxReadVector3(island->m_aabbMin);
					PfxVector3 islandAabbMax = pfxReadVector3(island->m_aabbMax);

					if( !pfxIntersectRayAABBFast( rayStartPosition,rayDirection, (islandAabbMax+islandAabbMin)*0.5f, (islandAabbMax-islandAabbMin)*0.5f, tmpVariable) ) {
						continue;
					}

					PfxBool ret = pfxIntersectRayTriMesh(largeMesh,island,rayStartPosition,rayDirection,ray,rotation_n,callback,userData,subData);
					if(!ret) return ret;
				}
			}
		}
	}
	
	return true;
}

PfxBool pfxIntersectRayAllFacetsInLargeTriMeshArray(
	const PfxRayInputInternal &ray,
	pfxRayHitCallback callback, void *userData,
	const PfxLargeTriMeshImpl *largeMesh,const PfxTransform3 &transform)
{
	// レイをラージメッシュのローカル座標へ変換
	PfxTransform3 transformLMesh = inverse(transform);
	PfxVector3 rayStartPosition = transformLMesh.getUpper3x3() * ray.m_startPosition + transformLMesh.getTranslation();
	PfxVector3 rayDirection = transformLMesh.getUpper3x3() * ray.m_direction;
	PfxMatrix3 rotation_n = transpose(transformLMesh.getUpper3x3()); // 法線をワールドへ変換するマトリクス
	
	PfxVecInt3 s,e,aabbMinL,aabbMaxL;

	s = largeMesh->getLocalPosition(rayStartPosition);
	e = largeMesh->getLocalPosition(rayStartPosition+rayDirection);

	aabbMinL = minPerElem(s,e);
	aabbMaxL = maxPerElem(s,e);
	
	PfxUInt32 numIslands = largeMesh->m_numIslands;
	
	{
		for(PfxUInt32 i=0;i<numIslands;i++) {
			PfxAabb16 aabbB = largeMesh->m_aabbList[i];
			if(aabbMaxL.getX() < pfxGetXMin(aabbB) || aabbMinL.getX() > pfxGetXMax(aabbB)) continue;
			if(aabbMaxL.getY() < pfxGetYMin(aabbB) || aabbMinL.getY() > pfxGetYMax(aabbB)) continue;
			if(aabbMaxL.getZ() < pfxGetZMin(aabbB) || aabbMinL.getZ() > pfxGetZMax(aabbB)) continue;
			
			PfxVector3 aabbMin,aabbMax;
			aabbMin = largeMesh->getWorldPosition(PfxVecInt3((PfxFloat)pfxGetXMin(aabbB),(PfxFloat)pfxGetYMin(aabbB),(PfxFloat)pfxGetZMin(aabbB)));
			aabbMax = largeMesh->getWorldPosition(PfxVecInt3((PfxFloat)pfxGetXMax(aabbB),(PfxFloat)pfxGetYMax(aabbB),(PfxFloat)pfxGetZMax(aabbB)));
			
			PfxFloat tmpVariable = 1.0f;
			if( !pfxIntersectRayAABBFast(
				rayStartPosition,rayDirection,
				(aabbMax+aabbMin)*0.5f,
				(aabbMax-aabbMin)*0.5f,
				tmpVariable) )
				continue;
			
			// アイランドとの交差チェック
			void *island=((PfxExpandedTriMesh*)largeMesh->m_islands) + i;
			
			PfxSubData subData;
			subData.setIslandId(i);
			
			PfxBool ret = pfxIntersectRayTriMesh(largeMesh,(PfxExpandedTriMesh*)island,rayStartPosition,rayDirection,ray,rotation_n,callback,userData,subData);
			if(!ret) return ret;
		}
	}
	
	return true;
}

PfxBool pfxIntersectRayTriMesh(
	const PfxLargeTriMeshImpl *largeMesh, const PfxExpandedTriMesh *mesh,
	const PfxVector3 &rayStart, const PfxVector3 &rayDir, const PfxTransform3 &transform,
	const PfxRayInputInternal &ray, const PfxMatrix3 &rotation_n,
	pfxRayHitCallback callback, void *userData,
	PfxSubData &subData,
	pfxRayHitDiscardTriangleCallback discardTriangleCallback,
	void *userDataForDiscardingTriangle)
{
	SCE_PFX_ASSERT(discardTriangleCallback);

	(void)largeMesh;
	bool ret = true;

	//-------------------------------------------
	// 判定する面を絞り込む

	PfxUInt8 selFacets[SCE_PFX_NUMMESHFACETS] = { 0 };

	PfxVector3 aabbMin = minPerElem(rayStart, rayStart + rayDir);
	PfxVector3 aabbMax = maxPerElem(rayStart, rayStart + rayDir);
	PfxUInt32 numSelFacets = pfxGatherFacets(mesh, 0.5f*(aabbMin + aabbMax), 0.5f*(aabbMax - aabbMin), selFacets);

	if (numSelFacets == 0) {
		return ret;
	}

	//-------------------------------------------
	// レイ交差判定

	for (PfxUInt32 f = 0; f < numSelFacets; f++) {
		const PfxExpandedFacet &facet = mesh->m_facets[selFacets[f]];

		PfxFloat cur_t = 1.0f;

		PfxVector3 cnt = pfxReadVector3((PfxFloat*)&facet.m_center);
		PfxVector3 hlf = pfxReadVector3((PfxFloat*)&facet.m_half);
		if (!pfxIntersectRayAABBFast(rayStart, rayDir, cnt, hlf, cur_t))
			continue;

		PfxTriangle triangle(
			mesh->m_verts[facet.m_vertIds[0]],
			mesh->m_verts[facet.m_vertIds[1]],
			mesh->m_verts[facet.m_vertIds[2]]);

		{
			PfxRayHitDiscardTriangleCallbackTriData triangleData;
			triangleData.m_vertices[0] = triangle.points[0];
			triangleData.m_vertices[1] = triangle.points[1];
			triangleData.m_vertices[2] = triangle.points[2];
			triangleData.m_normal = normalize(PfxVector3(facet.m_normal));
			triangleData.m_userData = facet.m_userData;
			if (!(*discardTriangleCallback)(triangleData, transform, userData))
				continue;
		}

		if (pfxIntersectRayTriangle(rayStart, rayDir, ray.m_facetMode, triangle, cur_t)) {
			PfxRayOutput hit;

			PfxFloat s = 0.0f, t = 0.0f;
			pfxCalcBarycentricCoords(rayStart + cur_t*rayDir, triangle, s, t);

			subData.setType(PfxSubData::MESH_INFO);
			subData.setFacetLocalS(s);
			subData.setFacetLocalT(t);
			subData.setFacetId(selFacets[f]);
			subData.setUserData(facet.m_userData);

			hit.m_contactPoint.segment = ray.m_segment;
			hit.m_contactPoint.offset = ray.m_startPosition + cur_t * ray.m_direction;
			hit.m_contactFlag = true;
			hit.m_variable = cur_t;
			hit.m_contactNormal = normalize(rotation_n *  facet.m_normal);
			hit.m_subData = subData;

			ret = callback(hit, userData);
			if (!ret) break;
		}
	}

	return ret;
}

PfxBool pfxIntersectRayTriMesh(
	const PfxLargeTriMeshImpl *largeMesh, const PfxQuantizedTriMeshBvh *mesh,
	const PfxVector3 &rayStart, const PfxVector3 &rayDir, const PfxTransform3 &transform,
	const PfxRayInputInternal &ray, const PfxMatrix3 &rotation_n,
	pfxRayHitCallback callback, void *userData,
	PfxSubData &subData,
	pfxRayHitDiscardTriangleCallback discardTriangleCallback,
	void *userDataForDiscardingTriangle)
{
	SCE_PFX_ASSERT(discardTriangleCallback);

	bool ret = true;

	EncStack encroot;
	encroot.nodeId = 0;
	encroot.aabbMin = pfxReadVector3(mesh->m_aabbMin);
	encroot.aabbMax = pfxReadVector3(mesh->m_aabbMax);

	PfxStaticStack<EncStack> bvhStack;
	bvhStack.push(encroot);

	PfxDecodedTriMesh decodedMesh;

	while (!bvhStack.empty()) {
		EncStack info = bvhStack.top();
		bvhStack.pop();

		PfxFacetBvhNode &encnode = mesh->m_bvhNodes[info.nodeId];

		PfxVector3 quantizedMin(encnode.aabb[0], encnode.aabb[1], encnode.aabb[2]);
		PfxVector3 quantizedMax(encnode.aabb[3], encnode.aabb[4], encnode.aabb[5]);
		PfxVector3 aabbMinA = info.aabbMin + mulPerElem((info.aabbMax - info.aabbMin), quantizedMin / 255.0f);
		PfxVector3 aabbMaxA = info.aabbMax - mulPerElem((info.aabbMax - info.aabbMin), quantizedMax / 255.0f);

		PfxFloat cur_t = 1.0f;

		if (!pfxIntersectRayAABBFast(rayStart, rayDir, (aabbMaxA + aabbMinA)*0.5f, (aabbMaxA - aabbMinA)*0.5f, cur_t)) {
			continue;
		}

		PfxUInt32 numFacets = 0;
		PfxUInt32 facetsIds[2];

		EncStack encnext;
		encnext.aabbMin = aabbMinA;
		encnext.aabbMax = aabbMaxA;

		PfxUInt8 LStatus = encnode.flag & 0x03;
		PfxUInt8 RStatus = (encnode.flag & 0x0C) >> 2;

		if (LStatus == 0) {
			encnext.nodeId = encnode.left;
			bvhStack.push(encnext);
		}
		else if (LStatus == 1) {
			facetsIds[numFacets++] = encnode.left;
		}

		if (RStatus == 0) {
			encnext.nodeId = encnode.right;
			bvhStack.push(encnext);
		}
		else if (RStatus == 1) {
			facetsIds[numFacets++] = encnode.right;
		}

		for (PfxUInt32 f = 0; f<numFacets; f++) {
			const PfxQuantizedFacetBvh &facet = mesh->m_facets[facetsIds[f]];

			// デコード
			PfxDecodedFacet decodedFacet;
			const PfxUInt32 vId[3] = { facet.m_vertIds[0],facet.m_vertIds[1],facet.m_vertIds[2] };

			for (int v = 0; v<3; v++) {
				if (!decodedMesh.isDecodedVertex(vId[v])) {
					decodedMesh.m_verts[vId[v]] = largeMesh->decodePosition(mesh->m_verts[vId[v]]);
					decodedMesh.m_decodedVertex[vId[v] >> 5] |= 1 << (vId[v] & 0x1f);
				}
			}

			PfxTriangle triangle(
				decodedMesh.m_verts[vId[0]],
				decodedMesh.m_verts[vId[1]],
				decodedMesh.m_verts[vId[2]]);

			{
				PfxRayHitDiscardTriangleCallbackTriData triangleData;
				triangleData.m_vertices[0] = triangle.points[0];
				triangleData.m_vertices[1] = triangle.points[1];
				triangleData.m_vertices[2] = triangle.points[2];
				triangleData.m_normal = normalize(largeMesh->decodeNormal(facet.m_normal));
				triangleData.m_userData = facet.m_userData;
				if (!(*discardTriangleCallback)(triangleData, transform, userData))
					continue;
			}

			if (pfxIntersectRayTriangle(rayStart, rayDir, ray.m_facetMode, triangle, cur_t)) {
				PfxRayOutput hit;

				PfxFloat s = 0.0f, t = 0.0f;
				pfxCalcBarycentricCoords(rayStart + cur_t*rayDir, triangle, s, t);

				subData.setType(PfxSubData::MESH_INFO);
				subData.setFacetLocalS(s);
				subData.setFacetLocalT(t);
				subData.setFacetId(facetsIds[f]);
				subData.setUserData(facet.m_userData);

				hit.m_contactPoint.segment = ray.m_segment;
				hit.m_contactPoint.offset = ray.m_startPosition + cur_t * ray.m_direction;
				hit.m_contactFlag = true;
				hit.m_variable = cur_t;
				hit.m_contactNormal = normalize(rotation_n * largeMesh->decodeNormal(facet.m_normal));
				hit.m_subData = subData;

				ret = callback(hit, userData);
				if (!ret) break;
			}
		}
	}

	return ret;
}

PfxBool pfxIntersectRayTriMesh(
	const PfxLargeTriMeshImpl *largeMesh, const PfxCompressedTriMesh *mesh,
	const PfxVector3 &rayStart, const PfxVector3 &rayDir, const PfxTransform3 &transform,
	const PfxRayInputInternal &ray, const PfxMatrix3 &rotation_n,
	pfxRayHitCallback callback, void *userData,
	PfxSubData &subData,
	pfxRayHitDiscardTriangleCallback discardTriangleCallback,
	void *userDataForDiscardingTriangle)
{
	SCE_PFX_ASSERT(discardTriangleCallback);

	bool ret = true;

	EncStack encroot;
	encroot.nodeId = 0;
	encroot.aabbMin = pfxReadVector3(mesh->m_aabbMin);
	encroot.aabbMax = pfxReadVector3(mesh->m_aabbMax);

	PfxStaticStack<EncStack> bvhStack;
	bvhStack.push(encroot);

	PfxDecodedTriMesh decodedMesh;

	while (!bvhStack.empty()) {
		EncStack info = bvhStack.top();
		bvhStack.pop();

		PfxFacetBvhNode &encnode = ((PfxFacetBvhNode*)largeMesh->m_bvhNodeBuffer)[mesh->m_bvhNodes + info.nodeId];

		PfxVector3 quantizedMin(encnode.aabb[0], encnode.aabb[1], encnode.aabb[2]);
		PfxVector3 quantizedMax(encnode.aabb[3], encnode.aabb[4], encnode.aabb[5]);
		PfxVector3 aabbMinA = info.aabbMin + mulPerElem((info.aabbMax - info.aabbMin), quantizedMin / 255.0f);
		PfxVector3 aabbMaxA = info.aabbMax - mulPerElem((info.aabbMax - info.aabbMin), quantizedMax / 255.0f);

		PfxFloat cur_t = 1.0f;

		if (!pfxIntersectRayAABBFast(rayStart, rayDir, (aabbMaxA + aabbMinA)*0.5f, (aabbMaxA - aabbMinA)*0.5f, cur_t)) {
			continue;
		}

		PfxUInt32 numFacets = 0;
		PfxUInt32 facetsIds[2];

		EncStack encnext;
		encnext.aabbMin = aabbMinA;
		encnext.aabbMax = aabbMaxA;

		PfxUInt8 LStatus = encnode.flag & 0x03;
		PfxUInt8 RStatus = (encnode.flag & 0x0C) >> 2;

		if (LStatus == 0) {
			encnext.nodeId = encnode.left;
			bvhStack.push(encnext);
		}
		else if (LStatus == 1) {
			facetsIds[numFacets++] = encnode.left;
		}

		if (RStatus == 0) {
			encnext.nodeId = encnode.right;
			bvhStack.push(encnext);
		}
		else if (RStatus == 1) {
			facetsIds[numFacets++] = encnode.right;
		}

		for (PfxUInt32 f = 0; f<numFacets; f++) {
			const PfxCompressedFacet2 &facet = ((PfxCompressedFacet2*)largeMesh->m_facetBuffer)[mesh->m_facets + facetsIds[f]];

			// デコード
			PfxDecodedFacet decodedFacet;
			const PfxUInt32 vId[4] = { facet.m_vertIds[0],facet.m_vertIds[1],facet.m_vertIds[2],facet.m_vertIds[3] };

			for (int v = 0; v<4; v++) {
				if (!decodedMesh.isDecodedVertex(vId[v])) {
					decodedMesh.m_verts[vId[v]] = largeMesh->decodePosition(*((PfxQuantize3*)largeMesh->m_vertexBuffer + mesh->m_verts + vId[v]));
					decodedMesh.m_decodedVertex[vId[v] >> 5] |= 1 << (vId[v] & 0x1f);
				}
			}

			if (facet.m_facetInfo & 0x8000) {
				PfxTriangle triangle1(
					decodedMesh.m_verts[vId[0]],
					decodedMesh.m_verts[vId[1]],
					decodedMesh.m_verts[vId[2]]);

				PfxTriangle triangle2(
					decodedMesh.m_verts[vId[2]],
					decodedMesh.m_verts[vId[3]],
					decodedMesh.m_verts[vId[0]]);

				{
					PfxRayHitDiscardTriangleCallbackTriData triangleData;
					triangleData.m_vertices[0] = triangle1.points[0];
					triangleData.m_vertices[1] = triangle1.points[1];
					triangleData.m_vertices[2] = triangle1.points[2];
					triangleData.m_normal = normalize(cross(decodedMesh.m_verts[vId[1]] - decodedMesh.m_verts[vId[0]], decodedMesh.m_verts[vId[2]] - decodedMesh.m_verts[vId[0]]));
					triangleData.m_userData = facet.m_userData[0];
					if ((*discardTriangleCallback)(triangleData, transform, userDataForDiscardingTriangle))
					{
						if (pfxIntersectRayTriangle(rayStart, rayDir, ray.m_facetMode, triangle1, cur_t)) {
							PfxRayOutput hit;

							PfxFloat s = 0.0f, t = 0.0f;
							pfxCalcBarycentricCoords(rayStart + cur_t*rayDir, triangle1, s, t);

							subData.setType(PfxSubData::MESH_INFO);
							subData.setFacetLocalS(s);
							subData.setFacetLocalT(t);
							subData.setFacetId(facet.m_facetInfo & 0xFF);
							subData.setUserData(facet.m_userData[0]);

							hit.m_contactPoint.segment = ray.m_segment;
							hit.m_contactPoint.offset = ray.m_startPosition + cur_t * ray.m_direction;
							hit.m_contactFlag = true;
							hit.m_variable = cur_t;
							hit.m_contactNormal = normalize(rotation_n * cross(decodedMesh.m_verts[vId[1]] - decodedMesh.m_verts[vId[0]], decodedMesh.m_verts[vId[2]] - decodedMesh.m_verts[vId[0]]));
							hit.m_subData = subData;

							ret = callback(hit, userData);
							if (!ret) break;
						}
					}
				}

				{
					PfxRayHitDiscardTriangleCallbackTriData triangleData;
					triangleData.m_vertices[0] = triangle2.points[0];
					triangleData.m_vertices[1] = triangle2.points[1];
					triangleData.m_vertices[2] = triangle2.points[2];
					triangleData.m_normal = normalize(cross(decodedMesh.m_verts[vId[3]] - decodedMesh.m_verts[vId[2]], decodedMesh.m_verts[vId[0]] - decodedMesh.m_verts[vId[2]]));
					triangleData.m_userData = facet.m_userData[1];
					if ((*discardTriangleCallback)(triangleData, transform, userDataForDiscardingTriangle))
					{
						if (pfxIntersectRayTriangle(rayStart, rayDir, ray.m_facetMode, triangle2, cur_t)) {
							PfxRayOutput hit;

							PfxFloat s = 0.0f, t = 0.0f;
							pfxCalcBarycentricCoords(rayStart + cur_t*rayDir, triangle2, s, t);

							subData.setType(PfxSubData::MESH_INFO);
							subData.setFacetLocalS(s);
							subData.setFacetLocalT(t);
							subData.setFacetId((facet.m_facetInfo & 0xFF) + 1);
							subData.setUserData(facet.m_userData[1]);

							hit.m_contactPoint.segment = ray.m_segment;
							hit.m_contactPoint.offset = ray.m_startPosition + cur_t * ray.m_direction;
							hit.m_contactFlag = true;
							hit.m_variable = cur_t;
							hit.m_contactNormal = normalize(rotation_n * cross(decodedMesh.m_verts[vId[3]] - decodedMesh.m_verts[vId[2]], decodedMesh.m_verts[vId[0]] - decodedMesh.m_verts[vId[2]]));
							hit.m_subData = subData;

							ret = callback(hit, userData);
							if (!ret) break;
						}
					}
				}
			}
			else {
				PfxTriangle triangle(
					decodedMesh.m_verts[vId[0]],
					decodedMesh.m_verts[vId[1]],
					decodedMesh.m_verts[vId[2]]);

				{
					PfxRayHitDiscardTriangleCallbackTriData triangleData;
					triangleData.m_vertices[0] = triangle.points[0];
					triangleData.m_vertices[1] = triangle.points[1];
					triangleData.m_vertices[2] = triangle.points[2];
					triangleData.m_normal = normalize(cross(decodedMesh.m_verts[vId[1]] - decodedMesh.m_verts[vId[0]], decodedMesh.m_verts[vId[2]] - decodedMesh.m_verts[vId[0]]));
					triangleData.m_userData = facet.m_userData[0];
					if ((*discardTriangleCallback)(triangleData, transform, userDataForDiscardingTriangle))
						continue;
				}

				if (pfxIntersectRayTriangle(rayStart, rayDir, ray.m_facetMode, triangle, cur_t)) {
					PfxRayOutput hit;

					PfxFloat s = 0.0f, t = 0.0f;
					pfxCalcBarycentricCoords(rayStart + cur_t*rayDir, triangle, s, t);

					subData.setType(PfxSubData::MESH_INFO);
					subData.setFacetLocalS(s);
					subData.setFacetLocalT(t);
					subData.setFacetId(facet.m_facetInfo & 0xFF);
					subData.setUserData(facet.m_userData[0]);

					hit.m_contactPoint.segment = ray.m_segment;
					hit.m_contactPoint.offset = ray.m_startPosition + cur_t * ray.m_direction;
					hit.m_contactFlag = true;
					hit.m_variable = cur_t;
					hit.m_contactNormal = normalize(rotation_n * cross(decodedMesh.m_verts[vId[1]] - decodedMesh.m_verts[vId[0]], decodedMesh.m_verts[vId[2]] - decodedMesh.m_verts[vId[0]]));
					hit.m_subData = subData;

					ret = callback(hit, userData);
					if (!ret) break;
				}
			}
		}
	}

	return ret;
}

PfxBool pfxIntersectRayAllFacetsInLargeTriMeshBvh(
	const PfxRayInputInternal &ray,
	pfxRayHitCallback callback, void *userData,
	const PfxLargeTriMeshImpl *largeMesh, const PfxTransform3 &transform,
	pfxRayHitDiscardTriangleCallback discardTriangleCallback,
	void *userDataForDiscardingTriangle)
{
	SCE_PFX_ASSERT(discardTriangleCallback);

	// レイをラージメッシュのローカル座標へ変換
	PfxTransform3 transformLMesh = inverse(transform);
	PfxVector3 rayStartPosition = transformLMesh.getUpper3x3() * ray.m_startPosition + transformLMesh.getTranslation();
	PfxVector3 rayDirection = transformLMesh.getUpper3x3() * ray.m_direction;
	PfxVector3 rayEndPosition = rayStartPosition + rayDirection;
	PfxMatrix3 rotation_n = transpose(transformLMesh.getUpper3x3()); // 法線をワールドへ変換するマトリクス

	PfxVector3 aabbMinB = minPerElem(rayStartPosition, rayEndPosition);
	PfxVector3 aabbMaxB = maxPerElem(rayStartPosition, rayEndPosition);

	{
		PfxStaticStack<EncStack> bvhStack;

		EncStack encroot;
		encroot.nodeId = 0;
		encroot.aabbMin = largeMesh->getOffset() - largeMesh->getHalf();
		encroot.aabbMax = largeMesh->getOffset() + largeMesh->getHalf();

		bvhStack.push(encroot);

		while (!bvhStack.empty()) {
			EncStack info = bvhStack.top();
			bvhStack.pop();

			PfxIslandBvhNode &encnode = largeMesh->m_bvhNodes[info.nodeId];

			PfxVector3 quantizedMin(encnode.aabb[0], encnode.aabb[1], encnode.aabb[2]);
			PfxVector3 quantizedMax(encnode.aabb[3], encnode.aabb[4], encnode.aabb[5]);
			PfxVector3 aabbMinA = info.aabbMin + mulPerElem((info.aabbMax - info.aabbMin), quantizedMin / 255.0f);
			PfxVector3 aabbMaxA = info.aabbMax - mulPerElem((info.aabbMax - info.aabbMin), quantizedMax / 255.0f);

			if (aabbMaxA[0] < aabbMinB[0] || aabbMinA[0] > aabbMaxB[0]) continue;
			if (aabbMaxA[1] < aabbMinB[1] || aabbMinA[1] > aabbMaxB[1]) continue;
			if (aabbMaxA[2] < aabbMinB[2] || aabbMinA[2] > aabbMaxB[2]) continue;

			PfxFloat tmpVariable = 1.0f;
			if (!pfxIntersectRayAABBFast(rayStartPosition, rayDirection, (aabbMaxA + aabbMinA)*0.5f, (aabbMaxA - aabbMinA)*0.5f, tmpVariable)) {
				continue;
			}

			//	informations about back-tracking trees
			EncStack encnext;
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

				//	now, check whether this one intersects the island
				PfxSubData subData;
				subData.setIslandId(islandId);

				if (largeMesh->m_type == SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION) {
					PfxCompressedTriMesh *island = (PfxCompressedTriMesh*)largeMesh->m_islands + islandId;
					PfxVector3 islandAabbMin = pfxReadVector3(island->m_aabbMin);
					PfxVector3 islandAabbMax = pfxReadVector3(island->m_aabbMax);

					if (!pfxIntersectRayAABBFast(rayStartPosition, rayDirection, (islandAabbMax + islandAabbMin)*0.5f, (islandAabbMax - islandAabbMin)*0.5f, tmpVariable)) {
						continue;
					}

					PfxBool ret = pfxIntersectRayTriMesh(largeMesh, island, rayStartPosition, rayDirection, transform, ray, rotation_n, callback, userData, subData, discardTriangleCallback, userDataForDiscardingTriangle);
					if (!ret) return ret;
				}
				else if (largeMesh->m_type & 0x01) {
					PfxQuantizedTriMeshBvh *island = (PfxQuantizedTriMeshBvh*)largeMesh->m_islands + islandId;
					PfxVector3 islandAabbMin = pfxReadVector3(island->m_aabbMin);
					PfxVector3 islandAabbMax = pfxReadVector3(island->m_aabbMax);

					if (!pfxIntersectRayAABBFast(rayStartPosition, rayDirection, (islandAabbMax + islandAabbMin)*0.5f, (islandAabbMax - islandAabbMin)*0.5f, tmpVariable)) {
						continue;
					}

					PfxBool ret = pfxIntersectRayTriMesh(largeMesh, island, rayStartPosition, rayDirection, transform, ray, rotation_n, callback, userData, subData, discardTriangleCallback, userDataForDiscardingTriangle);
					if (!ret) return ret;
				}
			}
		}
	}

	return true;
}

PfxBool pfxIntersectRayAllFacetsInLargeTriMeshArray(
	const PfxRayInputInternal &ray,
	pfxRayHitCallback callback, void *userData,
	const PfxLargeTriMeshImpl *largeMesh, const PfxTransform3 &transform,
	pfxRayHitDiscardTriangleCallback discardTriangleCallback,
	void *userDataForDiscardingTriangle)
{
	SCE_PFX_ASSERT(discardTriangleCallback);

	// レイをラージメッシュのローカル座標へ変換
	PfxTransform3 transformLMesh = inverse(transform);
	PfxVector3 rayStartPosition = transformLMesh.getUpper3x3() * ray.m_startPosition + transformLMesh.getTranslation();
	PfxVector3 rayDirection = transformLMesh.getUpper3x3() * ray.m_direction;
	PfxMatrix3 rotation_n = transpose(transformLMesh.getUpper3x3()); // 法線をワールドへ変換するマトリクス

	PfxVecInt3 s, e, aabbMinL, aabbMaxL;

	s = largeMesh->getLocalPosition(rayStartPosition);
	e = largeMesh->getLocalPosition(rayStartPosition + rayDirection);

	aabbMinL = minPerElem(s, e);
	aabbMaxL = maxPerElem(s, e);

	PfxUInt32 numIslands = largeMesh->m_numIslands;

	{
		for (PfxUInt32 i = 0; i<numIslands; i++) {
			PfxAabb16 aabbB = largeMesh->m_aabbList[i];
			if (aabbMaxL.getX() < pfxGetXMin(aabbB) || aabbMinL.getX() > pfxGetXMax(aabbB)) continue;
			if (aabbMaxL.getY() < pfxGetYMin(aabbB) || aabbMinL.getY() > pfxGetYMax(aabbB)) continue;
			if (aabbMaxL.getZ() < pfxGetZMin(aabbB) || aabbMinL.getZ() > pfxGetZMax(aabbB)) continue;

			PfxVector3 aabbMin, aabbMax;
			aabbMin = largeMesh->getWorldPosition(PfxVecInt3((PfxFloat)pfxGetXMin(aabbB), (PfxFloat)pfxGetYMin(aabbB), (PfxFloat)pfxGetZMin(aabbB)));
			aabbMax = largeMesh->getWorldPosition(PfxVecInt3((PfxFloat)pfxGetXMax(aabbB), (PfxFloat)pfxGetYMax(aabbB), (PfxFloat)pfxGetZMax(aabbB)));

			PfxFloat tmpVariable = 1.0f;
			if (!pfxIntersectRayAABBFast(
				rayStartPosition, rayDirection,
				(aabbMax + aabbMin)*0.5f,
				(aabbMax - aabbMin)*0.5f,
				tmpVariable))
				continue;

			// アイランドとの交差チェック
			void *island = ((PfxExpandedTriMesh*)largeMesh->m_islands) + i;

			PfxSubData subData;
			subData.setIslandId(i);

			PfxBool ret = pfxIntersectRayTriMesh(largeMesh, (PfxExpandedTriMesh*)island, rayStartPosition, rayDirection, transform, ray, rotation_n, callback, userData, subData, discardTriangleCallback, userDataForDiscardingTriangle);
			if (!ret) return ret;
		}
	}

	return true;
}

PfxBool pfxIntersectRayAllFacetsInLargeTriMesh(
	const PfxRayInputInternal &ray,
	PfxBool(*callback)(const PfxRayOutput &hit, void *userData),
	void *userData,
	const void *shape,
	const PfxTransform3 &transform, 
	pfxRayHitDiscardTriangleCallback discardTriangleCallback,
	void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	
	if(largeMesh->isUsingBvh()) {
		if(discardTriangleCallback)
			return pfxIntersectRayAllFacetsInLargeTriMeshBvh( ray, callback, userData, largeMesh, transform, discardTriangleCallback, userDataForDiscardingTriangle);

		else 
			return pfxIntersectRayAllFacetsInLargeTriMeshBvh(ray, callback, userData, largeMesh, transform);
	}
	
	if(discardTriangleCallback)
		return pfxIntersectRayAllFacetsInLargeTriMeshArray( ray, callback, userData, largeMesh, transform, discardTriangleCallback, userDataForDiscardingTriangle);

	else 
		return pfxIntersectRayAllFacetsInLargeTriMeshArray(ray, callback, userData, largeMesh, transform);
}

} //namespace pfxv4
} //namespace sce
