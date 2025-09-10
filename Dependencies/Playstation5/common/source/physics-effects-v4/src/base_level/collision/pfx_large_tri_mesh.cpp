/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_large_tri_mesh.h"
#include "pfx_large_tri_mesh_impl.h"

namespace sce {
namespace pfxv4 {

PfxUInt32 pfxLargeTriMeshGetNumIslands(const PfxLargeTriMesh &largeMesh)
{
	return ((PfxLargeTriMeshImpl*)&largeMesh)->m_numIslands;
}

void pfxLargeTriMeshGetIslandAabb(const PfxLargeTriMesh &largeMesh,PfxUInt32 islandId,PfxVector3 &aabbMin,PfxVector3 &aabbMax)
{
	SCE_PFX_ASSERT(sizeof(PfxLargeTriMeshImpl) <= sizeof(PfxLargeTriMesh));

	const PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;
	
	SCE_PFX_ASSERT(islandId < largeMeshImpl->m_numIslands);
	
	switch(largeMeshImpl->m_type) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		{
			PfxAabb16 aabb = largeMeshImpl->m_aabbList[islandId];
			aabbMin = largeMeshImpl->getWorldPosition(PfxVecInt3(pfxGetXMin(aabb),pfxGetYMin(aabb),pfxGetZMin(aabb)));
			aabbMax = largeMeshImpl->getWorldPosition(PfxVecInt3(pfxGetXMax(aabb),pfxGetYMax(aabb),pfxGetZMax(aabb)));
		}
		break;
		
		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		{
			const PfxQuantizedTriMeshBvh *island = ((PfxQuantizedTriMeshBvh*)largeMeshImpl->m_islands)+islandId;
			aabbMin = pfxReadVector3(island->m_aabbMin);
			aabbMax = pfxReadVector3(island->m_aabbMax);
		}
		break;
		
		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		{
			const PfxCompressedTriMesh *island = ((PfxCompressedTriMesh*)largeMeshImpl->m_islands)+islandId;
			aabbMin = pfxReadVector3(island->m_aabbMin);
			aabbMax = pfxReadVector3(island->m_aabbMax);
		}
		break;
	}
}

PfxUInt32 pfxLargeTriMeshGetNumVerticesInIslands(const PfxLargeTriMesh &largeMesh,PfxUInt32 islandId)
{
	SCE_PFX_ASSERT(sizeof(PfxLargeTriMeshImpl) <= sizeof(PfxLargeTriMesh));
	
	const PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;
	
	SCE_PFX_ASSERT(islandId < largeMeshImpl->m_numIslands);
	
	PfxUInt32 numVerts = 0;
	
	switch(largeMeshImpl->m_type) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		{
			const PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMeshImpl->m_islands)+islandId;
			numVerts = island->m_numVerts;
		}
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		{
			const PfxQuantizedTriMeshBvh *island = ((PfxQuantizedTriMeshBvh*)largeMeshImpl->m_islands)+islandId;
			numVerts = island->m_numVerts;
		}
		break;
		
		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		{
			const PfxCompressedTriMesh *island = ((PfxCompressedTriMesh*)largeMeshImpl->m_islands)+islandId;
			numVerts = island->m_numVerts;
		}
		break;
	}
	
	return numVerts;
}

PfxVector3 pfxLargeTriMeshGetVertexInIslands(const PfxLargeTriMesh &largeMesh,PfxUInt32 islandId,PfxUInt32 vertexId)
{
	SCE_PFX_ASSERT(sizeof(PfxLargeTriMeshImpl) <= sizeof(PfxLargeTriMesh));
	
	const PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;
	
	SCE_PFX_ASSERT(islandId < largeMeshImpl->m_numIslands);
	
	PfxVector3 vertex(0.0f);
	
	switch(largeMeshImpl->m_type) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		{
			const PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMeshImpl->m_islands)+islandId;
			SCE_PFX_ASSERT(vertexId < island->m_numVerts);
			vertex = island->m_verts[vertexId];
		}
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		{
			const PfxQuantizedTriMeshBvh *island = ((PfxQuantizedTriMeshBvh*)largeMeshImpl->m_islands)+islandId;
			SCE_PFX_ASSERT(vertexId < island->m_numVerts);
			vertex = largeMeshImpl->decodePosition(island->m_verts[vertexId]);
		}
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		{
			const PfxCompressedTriMesh *island = ((PfxCompressedTriMesh*)largeMeshImpl->m_islands)+islandId;
			SCE_PFX_ASSERT(vertexId < island->m_numVerts);
			PfxQuantize3 *qvert = (PfxQuantize3*)largeMeshImpl->m_vertexBuffer + island->m_verts + vertexId;
			vertex = largeMeshImpl->decodePosition(*qvert);
		}
		break;
	}
	
	return vertex;
}

PfxUInt32 pfxLargeTriMeshGetNumFacetsInIslands(const PfxLargeTriMesh &largeMesh,PfxUInt32 islandId)
{
	SCE_PFX_ASSERT(sizeof(PfxLargeTriMeshImpl) <= sizeof(PfxLargeTriMesh));
	
	const PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;
	
	SCE_PFX_ASSERT(islandId < largeMeshImpl->m_numIslands);
	
	PfxUInt32 numFacets = 0;
	
	switch(largeMeshImpl->m_type) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		{
			const PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMeshImpl->m_islands)+islandId;
			numFacets = island->m_numFacets;
		}
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		{
			const PfxQuantizedTriMeshBvh *island = ((PfxQuantizedTriMeshBvh*)largeMeshImpl->m_islands)+islandId;
			numFacets = island->m_numFacets;
		}
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		{
			const PfxCompressedTriMesh *island = (PfxCompressedTriMesh*)largeMeshImpl->m_islands + islandId;
			const PfxCompressedFacet2 *facet = (PfxCompressedFacet2*)largeMeshImpl->m_facetBuffer + island->m_facets + island->m_numFacets - 1;
			PfxUInt32 offsetId = facet->m_facetInfo & 0xFF;
			numFacets = (facet->m_facetInfo&0x8000)?offsetId+2:offsetId+1;
		}
		break;
	}
	
	return numFacets;
}

static inline PfxUInt8 encodeEdgeType(PfxUInt8 edgeType0,PfxUInt8 edgeType1,PfxUInt8 edgeType2)
{
	return (edgeType0&0x03) | ((edgeType1&0x03)<<2) | ((edgeType2&0x03)<<4);
}

PfxLargeTriMeshFacet pfxLargeTriMeshGetFacetInIslands(const PfxLargeTriMesh &largeMesh,PfxUInt32 islandId,PfxUInt32 facetId)
{
	SCE_PFX_ASSERT(sizeof(PfxLargeTriMeshImpl) <= sizeof(PfxLargeTriMesh));
	
	const PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;
	
	SCE_PFX_ASSERT(islandId < largeMeshImpl->m_numIslands);

	PfxLargeTriMeshFacet result;
	memset(&result,0,sizeof(PfxLargeTriMeshFacet));
	
	switch(largeMeshImpl->m_type) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		{
			const PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMeshImpl->m_islands)+islandId;
			SCE_PFX_ASSERT(facetId < island->m_numFacets);
			const PfxExpandedFacet *facet = island->m_facets + facetId;
			result.normal = facet->m_normal;
			result.userData = facet->m_userData;
			result.thickness = facet->m_thickness;
			result.vertIds[0] = facet->m_vertIds[0];
			result.vertIds[1] = facet->m_vertIds[1];
			result.vertIds[2] = facet->m_vertIds[2];
			result.edgeTilt[0] = 0.5f*SCE_PFX_PI/255.0f*island->m_edges[facet->m_edgeIds[0]].m_tilt;
			result.edgeTilt[1] = 0.5f*SCE_PFX_PI/255.0f*island->m_edges[facet->m_edgeIds[1]].m_tilt;
			result.edgeTilt[2] = 0.5f*SCE_PFX_PI/255.0f*island->m_edges[facet->m_edgeIds[2]].m_tilt;
			result.edgeType = encodeEdgeType(
				island->m_edges[facet->m_edgeIds[0]].m_angleType,
				island->m_edges[facet->m_edgeIds[1]].m_angleType,
				island->m_edges[facet->m_edgeIds[2]].m_angleType);
			if(island->m_edges[facet->m_edgeIds[0]].m_angleType != SCE_PFX_EDGE_CONVEX) result.edgeTilt[0] = 0.0f;
			if(island->m_edges[facet->m_edgeIds[1]].m_angleType != SCE_PFX_EDGE_CONVEX) result.edgeTilt[1] = 0.0f;
			if(island->m_edges[facet->m_edgeIds[2]].m_angleType != SCE_PFX_EDGE_CONVEX) result.edgeTilt[2] = 0.0f;
		}
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		{
			const PfxQuantizedTriMeshBvh *island = ((PfxQuantizedTriMeshBvh*)largeMeshImpl->m_islands)+islandId;
			SCE_PFX_ASSERT(facetId < island->m_numFacets);
			const PfxQuantizedFacetBvh *facet = island->m_facets + facetId;
			result.normal = largeMeshImpl->decodeNormal(facet->m_normal);
			result.userData = facet->m_userData;
			result.thickness = largeMeshImpl->decodeFloat(facet->m_thickness);
			result.vertIds[0] = facet->m_vertIds[0];
			result.vertIds[1] = facet->m_vertIds[1];
			result.vertIds[2] = facet->m_vertIds[2];
			result.edgeTilt[0] = 0.5f*SCE_PFX_PI/255.0f*island->m_edges[facet->m_edgeIds[0]].m_tilt;
			result.edgeTilt[1] = 0.5f*SCE_PFX_PI/255.0f*island->m_edges[facet->m_edgeIds[1]].m_tilt;
			result.edgeTilt[2] = 0.5f*SCE_PFX_PI/255.0f*island->m_edges[facet->m_edgeIds[2]].m_tilt;
			result.edgeType = encodeEdgeType(
				island->m_edges[facet->m_edgeIds[0]].m_angleType,
				island->m_edges[facet->m_edgeIds[1]].m_angleType,
				island->m_edges[facet->m_edgeIds[2]].m_angleType);
			if(island->m_edges[facet->m_edgeIds[0]].m_angleType != SCE_PFX_EDGE_CONVEX) result.edgeTilt[0] = 0.0f;
			if(island->m_edges[facet->m_edgeIds[1]].m_angleType != SCE_PFX_EDGE_CONVEX) result.edgeTilt[1] = 0.0f;
			if(island->m_edges[facet->m_edgeIds[2]].m_angleType != SCE_PFX_EDGE_CONVEX) result.edgeTilt[2] = 0.0f;
		}
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		{
			const PfxCompressedTriMesh *island = ((PfxCompressedTriMesh*)largeMeshImpl->m_islands)+islandId;
			
			int f=0;
			int sid = -1;
			for(;f<island->m_numFacets;f++) {
				const PfxCompressedFacet2 *facet = (PfxCompressedFacet2*)largeMeshImpl->m_facetBuffer + island->m_facets + f;
				if(facet->m_facetInfo & 0x8000) {
					if((facet->m_facetInfo&0xFF) == facetId) {
						sid = 0;
						break;
					}
					else if((facet->m_facetInfo&0xFF)+1 == facetId) {
						sid = 1;
						break;
					}
				}
				else {
					if((facet->m_facetInfo&0xFF) == facetId) {
						sid = 0;
						break;
					}
				}
			}
			
			SCE_PFX_ASSERT(sid>=0); // There isn't the requested facet.
			
			const PfxCompressedFacet2 *facet = (PfxCompressedFacet2*)largeMeshImpl->m_facetBuffer + island->m_facets + f;
			
			result.edgeTilt[0] = 0.0f;
			result.edgeTilt[1] = 0.0f;
			result.edgeTilt[2] = 0.0f;
			result.thickness = largeMeshImpl->m_defaultThickness;
			
			if(sid == 0) {
				result.vertIds[0] = facet->m_vertIds[0];
				result.vertIds[1] = facet->m_vertIds[1];
				result.vertIds[2] = facet->m_vertIds[2];
				result.edgeType = (PfxUInt8)(facet->m_edgeInfo & 0x3F);
				result.userData = facet->m_userData[0];
			}
			else {
				result.vertIds[0] = facet->m_vertIds[2];
				result.vertIds[1] = facet->m_vertIds[3];
				result.vertIds[2] = facet->m_vertIds[0];
				result.edgeType = (PfxUInt8)(((facet->m_edgeInfo>>6)&0x0F)|((facet->m_edgeInfo>>4)&0x03));
				result.userData = facet->m_userData[1];
			}
			
			PfxQuantize3 *qvert1 = (PfxQuantize3*)largeMeshImpl->m_vertexBuffer + island->m_verts + result.vertIds[0];
			PfxQuantize3 *qvert2 = (PfxQuantize3*)largeMeshImpl->m_vertexBuffer + island->m_verts + result.vertIds[1];
			PfxQuantize3 *qvert3 = (PfxQuantize3*)largeMeshImpl->m_vertexBuffer + island->m_verts + result.vertIds[2];
			PfxVector3 vertex1 = largeMeshImpl->decodePosition(*qvert1);
			PfxVector3 vertex2 = largeMeshImpl->decodePosition(*qvert2);
			PfxVector3 vertex3 = largeMeshImpl->decodePosition(*qvert3);
			
			result.normal = normalize(cross(vertex2-vertex1,vertex3-vertex1));
		}
		break;
	}
	
	return result;
}

PfxUInt32 pfxLargeTriMeshSetVertexInIslands(PfxLargeTriMesh &largeMesh,PfxUInt32 islandId,PfxUInt32 vertexId,const PfxVector3 &vertex)
{
	SCE_PFX_ASSERT(sizeof(PfxLargeTriMeshImpl) <= sizeof(PfxLargeTriMesh));
	
	PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;
	
	if(largeMeshImpl->m_type != SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY) {
		return SCE_PFX_ERR_INVALID_TYPE;
	}
	
	if(islandId >= largeMeshImpl->m_numIslands || vertexId >= ((PfxExpandedTriMesh*)largeMeshImpl->m_islands)[islandId].m_numVerts ) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	
	((PfxExpandedTriMesh*)largeMeshImpl->m_islands)[islandId].m_verts[vertexId] = vertex;
	
	return SCE_PFX_OK;
}

PfxUInt32 pfxLargeTriMeshRebuild(PfxLargeTriMesh &largeMesh)
{
	SCE_PFX_ASSERT(sizeof(PfxLargeTriMeshImpl) <= sizeof(PfxLargeTriMesh));
	
	PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;
	
	if(largeMeshImpl->m_type != SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY) {
		return SCE_PFX_ERR_INVALID_TYPE;
	}
	
	PfxVector3 lmeshAabbMin(SCE_PFX_FLT_MAX);
	PfxVector3 lmeshAabbMax(-SCE_PFX_FLT_MAX);
	for(PfxUInt32 i=0;i<largeMeshImpl->m_numIslands;i++) {
		PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMeshImpl->m_islands) + i;
		for(PfxUInt32 v=0;v<island->m_numVerts;v++) {
			lmeshAabbMin = minPerElem(lmeshAabbMin,island->m_verts[v]);
			lmeshAabbMax = maxPerElem(lmeshAabbMax,island->m_verts[v]);
		}
	}
	
	largeMeshImpl->setOffset((lmeshAabbMax + lmeshAabbMin) * 0.5f);
	largeMeshImpl->setHalf((lmeshAabbMax - lmeshAabbMin) * 0.5f);
	
	for(PfxUInt32 i=0;i<largeMeshImpl->m_numIslands;i++) {
		PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMeshImpl->m_islands) + i;
		PfxVector3 islandAabbMin(SCE_PFX_FLT_MAX);
		PfxVector3 islandAabbMax(-SCE_PFX_FLT_MAX);
		for(PfxUInt32 f=0;f<island->m_numFacets;f++) {
			PfxExpandedFacet &facet = island->m_facets[f];
			PfxVector3 p0 = island->m_verts[facet.m_vertIds[0]];
			PfxVector3 p1 = island->m_verts[facet.m_vertIds[1]];
			PfxVector3 p2 = island->m_verts[facet.m_vertIds[2]];
			PfxVector3 facetAabbMin = minPerElem(minPerElem(p0,p1),p2);
			PfxVector3 facetAabbMax = maxPerElem(maxPerElem(p0,p1),p2);
			facet.m_center = (facetAabbMax + facetAabbMin ) * 0.5f;
			facet.m_half = (facetAabbMax - facetAabbMin ) * 0.5f;
			facet.m_normal = normalize(cross(p1-p0,p2-p0));
			islandAabbMin = minPerElem(islandAabbMin,facetAabbMin);
			islandAabbMax = maxPerElem(islandAabbMax,facetAabbMax);
		}
		
		PfxVecInt3 aabbMinL,aabbMaxL;
		largeMeshImpl->getLocalPosition(islandAabbMin,islandAabbMax,aabbMinL,aabbMaxL);
		pfxSetXMin(largeMeshImpl->m_aabbList[i],aabbMinL.getX());
		pfxSetXMax(largeMeshImpl->m_aabbList[i],aabbMaxL.getX());
		pfxSetYMin(largeMeshImpl->m_aabbList[i],aabbMinL.getY());
		pfxSetYMax(largeMeshImpl->m_aabbList[i],aabbMaxL.getY());
		pfxSetZMin(largeMeshImpl->m_aabbList[i],aabbMinL.getZ());
		pfxSetZMax(largeMeshImpl->m_aabbList[i],aabbMaxL.getZ());
	}

	return SCE_PFX_OK;
}

const PfxUInt8 *pfxLargeTriMeshGetName(const PfxLargeTriMesh &largeMesh)
{
	SCE_PFX_ASSERT(sizeof(PfxLargeTriMeshImpl) <= sizeof(PfxLargeTriMesh));

	PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;

	return largeMeshImpl->m_name;
}

} //namespace pfxv4
} //namespace sce
