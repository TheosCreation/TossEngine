/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2025 Sony Interactive Entertainment Inc.
 *                                                
 */

//#define SCE_PFX_USE_PERFCOUNTER

#include "pfx_precompiled.h"
#include "pfx_mesh_internal.h"
#include "../../include/physics_effects/base_level/base/pfx_perf_counter.h"
#include "../../include/physics_effects/base_level/sort/pfx_sort.h"
#include "../base_level/collision/pfx_intersect_common.h"
#include "../base_level/collision/pfx_convex_mesh_impl.h"
#include "../base_level/collision/pfx_large_tri_mesh_impl.h"

#define SCE_PFX_MESH_CREATE_EPSILON 1e-4f
#define SCE_PFX_MESH_MINIMUM_AREA_FACET 1e-8f

namespace sce {
namespace pfxv4 {

/**
 * check the value of flag in PfxCreateConvexMeshParam
 * 
 * @param param  a reference to the param to be investigated
 * 
 * @return SCE_PFX_OK if nothing wrong with the flag
 *         SCE_PFX_ERR_INVALID_FLAG if something went wrong
 */
PfxInt32 checkFlagsForConvexMesh(const PfxCreateConvexMeshParam& param)
{
	//	check flag ranged
	const PfxUInt32 all_flag =	~(	SCE_PFX_MESH_FLAG_NORMAL_FLIP|
									SCE_PFX_MESH_FLAG_16BIT_INDEX|
									SCE_PFX_MESH_FLAG_32BIT_INDEX|
									SCE_PFX_MESH_FLAG_AUTO_ELIMINATION|
								    SCE_PFX_MESH_FLAG_FILL_REMOVED_FACETS
									);
	if ((param.flag&all_flag)!=0)
	{
		return SCE_PFX_ERR_INVALID_FLAG;
	}
	//	check method of indexing
	if ((param.flag&SCE_PFX_MESH_FLAG_16BIT_INDEX)&&(param.flag&SCE_PFX_MESH_FLAG_32BIT_INDEX))
	{
		return SCE_PFX_ERR_INVALID_FLAG;
	}
	if ( ! (param.flag&SCE_PFX_MESH_FLAG_16BIT_INDEX || param.flag&SCE_PFX_MESH_FLAG_32BIT_INDEX) )
	{
		return SCE_PFX_ERR_INVALID_FLAG;
	}

	//	checking variable type of the indices of triangles
	if ((param.flag & SCE_PFX_MESH_FLAG_32BIT_INDEX) && (param.triangleStrideBytes<sizeof(PfxUInt32)*3))
	{
		return SCE_PFX_ERR_INVALID_FLAG;
	}
	/*	should this operation been taken ? 
	if ((param.flag & SCE_PFX_MESH_FLAG_16BIT_INDEX) && (param.triangleStrideBytes>sizeof(PfxUInt16)*3))
	{
		return SCE_PFX_ERR_INVALID_FLAG;
	}
	*/


	return SCE_PFX_OK;
}

PfxUInt32 checkLargeMeshParams(const PfxCreateLargeTriMeshParam &param)
{
	//	check the params based on numVerts
	PfxUInt32 max_index = 0;

	//	check the params based on numTriangles
	for(PfxUInt32 i=0;i<param.numTriangles;i++)
	{
		void *ids = (void*)((uintptr_t)param.triangles + param.triangleStrideBytes * i);

		PfxUInt32 idx[3];

		if(param.flag & SCE_PFX_MESH_FLAG_32BIT_INDEX)
		{
			idx[0] = ((PfxUInt32*)ids)[0];
			idx[1] = ((PfxUInt32*)ids)[1];
			idx[2] = ((PfxUInt32*)ids)[2];
		}
		else if(param.flag & SCE_PFX_MESH_FLAG_16BIT_INDEX)
		{
			idx[0] = ((PfxUInt16*)ids)[0];
			idx[1] = ((PfxUInt16*)ids)[1];
			idx[2] = ((PfxUInt16*)ids)[2];
		}

		for (PfxUInt32 d=0; d<3; ++d)
		{
			if (idx[d]>=param.numVerts)
			{
				if (param.flag & SCE_PFX_MESH_FLAG_OUTPUT_INFO)
				{
					SCE_PFX_PRINTF("found an invalid index while creating largemsh\n");
				}
				return SCE_PFX_ERR_OUT_OF_RANGE_VERTEX;
			}
			max_index = (max_index<idx[d])?idx[d]:max_index;
		}
	}
	
	if(param.numFacetsLimit == 0) return SCE_PFX_ERR_INVALID_VALUE;
	
	/*	this will not cause serious problem. but the users have to take care about the enormouse number of vertices,
	if (max_index+1<param.numVerts)
	{
		if (param.flag & SCE_PFX_MESH_FLAG_OUTPUT_INFO)
		{
			SCE_PFX_PRINTF("found an exceeding the max number of vertice swhile creating largemsh\n");
		}
		return SCE_PFX_ERR_OUT_OF_RANGE_VERTEX;
	}
	*/

	//	nothing wrong with the params
	return SCE_PFX_OK;
}

/**
 * check the value of flag in PfxCreateLargeTriMeshParam
 * 
 * @param param  a reference to the param to be investigated
 * 
 * @return SCE_PFX_OK if nothing wrong with the flag
 *         SCE_PFX_ERR_INVALID_FLAG if something went wrong
 */
PfxInt32 checkFlagsForLargeTriMesh(const PfxCreateLargeTriMeshParam& param)
{
	//	check flag ranged
	const PfxUInt32 all_flag =	param.flag >> 11;
	if (all_flag)
	{
		return SCE_PFX_ERR_INVALID_FLAG;
	}
	//	check method of indexing
	if ((param.flag&SCE_PFX_MESH_FLAG_16BIT_INDEX)&&(param.flag&SCE_PFX_MESH_FLAG_32BIT_INDEX))
	{
		return SCE_PFX_ERR_INVALID_FLAG;
	}
	if ( ! (param.flag&SCE_PFX_MESH_FLAG_16BIT_INDEX || param.flag&SCE_PFX_MESH_FLAG_32BIT_INDEX) )
	{
		return SCE_PFX_ERR_INVALID_FLAG;
	}

	//	checking variable type of the indices of triangles
	if ((param.flag & SCE_PFX_MESH_FLAG_32BIT_INDEX) && (param.triangleStrideBytes<sizeof(PfxUInt32)*3))
	{
		return SCE_PFX_ERR_INVALID_FLAG;
	}
	/*	should this operation been taken ? 
	if ((param.flag & SCE_PFX_MESH_FLAG_16BIT_INDEX) && (param.triangleStrideBytes>sizeof(PfxUInt16)*3))
	{
		return SCE_PFX_ERR_INVALID_FLAG;
	}
	*/


	return SCE_PFX_OK;
}

///////////////////////////////////////////////////////////////////////////////
//J 凸メッシュ作成時に使用する関数
//E Functions used when creating Convex Mesh

struct VertexInfo {
	PfxVector3 point;
	PfxBool obsolete;
	PfxUInt8 originalVertexId;
	PfxUInt8 assignedVertexId;
};

#ifdef SCE_PFX_ENABLE_CUBEMAP_CONVEX_OPTIMIZE
PfxInt32 constructCubeMap(PfxConvexMeshImpl *convexMeshImpl)
{
	PfxUInt8 cubeMap[6][SCE_PFX_CONVEX_CUBEMAP_SIZE * SCE_PFX_CONVEX_CUBEMAP_SIZE];

	auto calcUV = [&](int uvi)
	{
		return (uvi + 0.5f) / (PfxFloat)SCE_PFX_CONVEX_CUBEMAP_SIZE;
	};

	auto calcSupportVertex = [&](const PfxVector3 &axis) {
		int reti = 0;
		PfxFloat dmax = -SCE_PFX_FLT_MAX;
		for (int i = 0; i < convexMeshImpl->m_numVerts; i++) {
			PfxFloat d = dot(pfxReadVector3(convexMeshImpl->m_verts + i * 3), axis);
			if (d > dmax) {
				dmax = d;
				reti = i;
			}
		}
		return reti;
	};

	// +X
	for (int vi = 0; vi < SCE_PFX_CONVEX_CUBEMAP_SIZE; vi++) {
		for (int ui = 0; ui < SCE_PFX_CONVEX_CUBEMAP_SIZE; ui++) {
			PfxFloat u = calcUV(ui);
			PfxFloat v = calcUV(vi);
			PfxFloat t = 1.0f - 2.0f * v;
			PfxFloat r = 1.0f - 2.0f * u;
			PfxVector3 normal = normalize(PfxVector3(1.0f, t, r));
			int vid = calcSupportVertex(normal);
			cubeMap[0][ui + SCE_PFX_CONVEX_CUBEMAP_SIZE * vi] = vid;
		}
	}

	// -X
	for (int vi = 0; vi < SCE_PFX_CONVEX_CUBEMAP_SIZE; vi++) {
		for (int ui = 0; ui < SCE_PFX_CONVEX_CUBEMAP_SIZE; ui++) {
			PfxFloat u = calcUV(ui);
			PfxFloat v = calcUV(vi);
			PfxFloat t = 1.0f - 2.0f * v;
			PfxFloat r = 2.0f * u - 1.0f;
			PfxVector3 normal = normalize(PfxVector3(-1.0f, t, r));
			int vid = calcSupportVertex(normal);
			cubeMap[1][ui + SCE_PFX_CONVEX_CUBEMAP_SIZE * vi] = vid;
		}
	}

	// +Y
	for (int vi = 0; vi < SCE_PFX_CONVEX_CUBEMAP_SIZE; vi++) {
		for (int ui = 0; ui < SCE_PFX_CONVEX_CUBEMAP_SIZE; ui++) {
			PfxFloat u = calcUV(ui);
			PfxFloat v = calcUV(vi);
			PfxFloat s = 2.0f * u - 1.0f;
			PfxFloat r = 2.0f * v - 1.0f;
			PfxVector3 normal = normalize(PfxVector3(s, 1.0f, r));
			int vid = calcSupportVertex(normal);
			cubeMap[2][ui + SCE_PFX_CONVEX_CUBEMAP_SIZE * vi] = vid;
		}
	}

	// -Y
	for (int vi = 0; vi < SCE_PFX_CONVEX_CUBEMAP_SIZE; vi++) {
		for (int ui = 0; ui < SCE_PFX_CONVEX_CUBEMAP_SIZE; ui++) {
			PfxFloat u = calcUV(ui);
			PfxFloat v = calcUV(vi);
			PfxFloat s = 2.0f * u - 1.0f;
			PfxFloat r = 1.0f - 2.0f * v;
			PfxVector3 normal = normalize(PfxVector3(s, -1.0f, r));
			int vid = calcSupportVertex(normal);
			cubeMap[3][ui + SCE_PFX_CONVEX_CUBEMAP_SIZE * vi] = vid;
		}
	}

	// +Z
	for (int vi = 0; vi < SCE_PFX_CONVEX_CUBEMAP_SIZE; vi++) {
		for (int ui = 0; ui < SCE_PFX_CONVEX_CUBEMAP_SIZE; ui++) {
			PfxFloat u = calcUV(ui);
			PfxFloat v = calcUV(vi);
			PfxFloat s = 2.0f * u - 1.0f;
			PfxFloat t = 1.0f - 2.0f * v;
			PfxVector3 normal = normalize(PfxVector3(s, t, 1.0f));
			int vid = calcSupportVertex(normal);
			cubeMap[4][ui + SCE_PFX_CONVEX_CUBEMAP_SIZE * vi] = vid;
		}
	}

	// -Z
	for (int vi = 0; vi < SCE_PFX_CONVEX_CUBEMAP_SIZE; vi++) {
		for (int ui = 0; ui < SCE_PFX_CONVEX_CUBEMAP_SIZE; ui++) {
			PfxFloat u = calcUV(ui);
			PfxFloat v = calcUV(vi);
			PfxFloat s = 1.0f - 2.0f * u;
			PfxFloat t = 1.0f - 2.0f * v;
			PfxVector3 normal = normalize(PfxVector3(s, t, -1.0f));
			int vid = calcSupportVertex(normal);
			cubeMap[5][ui + SCE_PFX_CONVEX_CUBEMAP_SIZE * vi] = vid;
		}
	}

	// count a buffer size
	int totalCount = 0;
	for (int map = 0; map < 6; map++) {
		int countRow = 0;
		for (int vi = 0; vi < SCE_PFX_CONVEX_CUBEMAP_SIZE; vi++) {
			PfxUInt8 curId = 0xff;
			int nValuesInLine = 0;
			for (int ui = 0; ui < SCE_PFX_CONVEX_CUBEMAP_SIZE; ui++) {
				PfxUInt8 vId = cubeMap[map][ui + SCE_PFX_CONVEX_CUBEMAP_SIZE * vi];
				if (vId != curId) {
					curId = vId;
					nValuesInLine++;
				}
			}
			countRow += nValuesInLine;
		}

		int countCol = 0;
		for (int ui = 0; ui < SCE_PFX_CONVEX_CUBEMAP_SIZE; ui++) {
			PfxUInt8 curId = 0xff;
			int nValuesInLine = 0;
			for (int vi = 0; vi < SCE_PFX_CONVEX_CUBEMAP_SIZE; vi++) {
				PfxUInt8 vId = cubeMap[map][ui + SCE_PFX_CONVEX_CUBEMAP_SIZE * vi];
				if (vId != curId) {
					curId = vId;
					nValuesInLine++;
				}
			}
			countCol += nValuesInLine;
		}

		if (countRow <= countCol) {
			convexMeshImpl->m_cubeMap[map].isRowOrColumn = 0;
			totalCount += countRow;
		}
		else {
			convexMeshImpl->m_cubeMap[map].isRowOrColumn = 1;
			totalCount += countCol;
		}
	}
	PfxUInt16 *listBuff = (PfxUInt16*)SCE_PFX_UTIL_ALLOC(16, sizeof(PfxUInt16) * totalCount);
	if (!listBuff) return SCE_PFX_ERR_OUT_OF_BUFFER;
	PfxUInt16 curLine = 0;

	for (int map = 0; map < 6; map++) {
		if (convexMeshImpl->m_cubeMap[map].isRowOrColumn == 0) {
			for (int vi = 0; vi < SCE_PFX_CONVEX_CUBEMAP_SIZE; vi++) {
				PfxUInt8 curId = 0xff;
				int nValuesInLine = 0;
				convexMeshImpl->m_cubeMap[map].start[vi] = curLine;
				for (int ui = 0; ui < SCE_PFX_CONVEX_CUBEMAP_SIZE; ui++) {
					PfxUInt8 vId = cubeMap[map][ui + SCE_PFX_CONVEX_CUBEMAP_SIZE * vi];
					if (vId != curId) {
						curId = vId;
						listBuff[curLine + nValuesInLine] = ((PfxUInt16)ui << 8) | vId;
						nValuesInLine++;
					}
				}
				convexMeshImpl->m_cubeMap[map].num[vi] = nValuesInLine;
				curLine += nValuesInLine;
				//epxPrintf("cubeMap %d line %d values %d\n", map, vi, nValuesInLine);
			}
		}
		else {
			for (int ui = 0; ui < SCE_PFX_CONVEX_CUBEMAP_SIZE; ui++) {
				PfxUInt8 curId = 0xff;
				int nValuesInLine = 0;
				convexMeshImpl->m_cubeMap[map].start[ui] = curLine;
				for (int vi = 0; vi < SCE_PFX_CONVEX_CUBEMAP_SIZE; vi++) {
					PfxUInt8 vId = cubeMap[map][ui + SCE_PFX_CONVEX_CUBEMAP_SIZE * vi];
					if (vId != curId) {
						curId = vId;
						listBuff[curLine + nValuesInLine] = ((PfxUInt16)vi << 8) | vId;
						nValuesInLine++;
					}
				}
				convexMeshImpl->m_cubeMap[map].num[ui] = nValuesInLine;
				curLine += nValuesInLine;
				//epxPrintf("cubeMap %d line %d values %d\n", map, ui, nValuesInLine);
			}
		}
	}

	convexMeshImpl->m_listBuff = listBuff;

	SCE_PFX_PRINTF("cubeMap %d bytes\n", sizeof(convexMeshImpl->m_cubeMap));
	SCE_PFX_PRINTF("vertex %d totalCount %d (%d bytes)\n", convexMeshImpl->m_numVerts, totalCount, sizeof(PfxUInt16) * totalCount + sizeof(convexMeshImpl->m_cubeMap));

	return SCE_PFX_OK;
}
#endif

PfxInt32 pfxCreateConvexMesh(PfxConvexMesh &convexMesh,const PfxCreateConvexMeshParam &param)
{
	SCE_PFX_ASSERT(sizeof(PfxConvexMeshImpl) <= sizeof(PfxConvexMesh));

	PfxConvexMeshImpl *convexMeshImpl = (PfxConvexMeshImpl*)&convexMesh;

	// zero clear
	memset(convexMeshImpl,0,sizeof(PfxConvexMeshImpl));

	//	check flag
	if (checkFlagsForConvexMesh(param)!=SCE_PFX_OK)
	{
		return SCE_PFX_ERR_INVALID_FLAG;
	}
	
	// Check input
	if(param.numVerts == 0 || param.numTriangles == 0 || !param.verts || !param.triangles)
		return SCE_PFX_ERR_INVALID_VALUE;
	
	if(param.numVerts > SCE_PFX_NUMMESHVERTICES || param.numTriangles > SCE_PFX_NUMMESHFACETS)
		return SCE_PFX_ERR_OUT_OF_RANGE;
	
	PfxArray<VertexInfo> vertList(param.numVerts);
	if(!vertList.ptr()) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxArray<PfxUInt32> facetList(param.numTriangles * 3);
	if (!facetList.ptr()) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxArray<PfxUInt32> userDataList(param.numTriangles);
	if (!userDataList.ptr()) return SCE_PFX_ERR_OUT_OF_BUFFER;
	
	// Temporal buffers
	PfxArray<PfxUInt32> idxList;
	PfxArray<PfxUInt32> vtxGroup;
	idxList.assign( param.numTriangles * 3, 0 );
	vtxGroup.assign( param.numVerts, 0 );

	// Create vertex list
	for( PfxUInt32 i = 0; i < param.numVerts; i++ ) {
		VertexInfo vi;
		PfxFloat *vtx = ( PfxFloat* )( ( uintptr_t )param.verts + param.vertexStrideBytes * i );
		vi.point = PfxVector3( vtx[ 0 ], vtx[ 1 ], vtx[ 2 ] );
		vi.obsolete = false;
		vi.originalVertexId = i;
		vi.assignedVertexId = i;
		vertList.push( vi );
	}

	// Create index list
	for( PfxUInt32 i = 0; i < param.numTriangles; i++ ) {
		void *ids = ( void* )( ( uintptr_t )param.triangles + param.triangleStrideBytes * i );
		PfxUInt32 idx[ 3 ];

		if( param.flag & SCE_PFX_MESH_FLAG_32BIT_INDEX ) {
			if( param.flag & SCE_PFX_MESH_FLAG_NORMAL_FLIP ) {
				idx[ 0 ] = ( ( PfxUInt32* )ids )[ 2 ];
				idx[ 1 ] = ( ( PfxUInt32* )ids )[ 1 ];
				idx[ 2 ] = ( ( PfxUInt32* )ids )[ 0 ];
			}
			else {
				idx[ 0 ] = ( ( PfxUInt32* )ids )[ 0 ];
				idx[ 1 ] = ( ( PfxUInt32* )ids )[ 1 ];
				idx[ 2 ] = ( ( PfxUInt32* )ids )[ 2 ];
			}
		}
		else if( param.flag & SCE_PFX_MESH_FLAG_16BIT_INDEX ) {
			if( param.flag & SCE_PFX_MESH_FLAG_NORMAL_FLIP ) {
				idx[ 0 ] = ( ( PfxUInt16* )ids )[ 2 ];
				idx[ 1 ] = ( ( PfxUInt16* )ids )[ 1 ];
				idx[ 2 ] = ( ( PfxUInt16* )ids )[ 0 ];
			}
			else {
				idx[ 0 ] = ( ( PfxUInt16* )ids )[ 0 ];
				idx[ 1 ] = ( ( PfxUInt16* )ids )[ 1 ];
				idx[ 2 ] = ( ( PfxUInt16* )ids )[ 2 ];
			}
		}
		else {
			return SCE_PFX_ERR_INVALID_FLAG;
		}

		auto checkVtxId = [&]( PfxUInt32 id)
		{
			if(id < SCE_PFX_NUMMESHVERTICES && id < param.numVerts) {
				return true;
			}
			return false;
		};
		
		//	this code will check the overrunning the buffers.
		if ( !checkVtxId(idx[0]) || !checkVtxId(idx[1]) || !checkVtxId(idx[2])) {
			return SCE_PFX_ERR_OUT_OF_RANGE_VERTEX;
		}

		idxList[ i * 3 + 0 ] = idx[ 0 ];
		idxList[ i * 3 + 1 ] = idx[ 1 ];
		idxList[ i * 3 + 2 ] = idx[ 2 ];
	}

	auto findRoot = [ & ]( PfxUInt32 vertexIdA )
	{
		PfxUInt32 curr = vertexIdA;
		PfxUInt32 parent = vtxGroup[ curr ];
		while( parent != curr ) {
			curr = parent;
			parent = vtxGroup[ curr ];
		}
		return parent;
	};

	auto mergeSubTrees = [ & ]( PfxUInt32 vertexIdA, PfxUInt32 vertexIdB )
	{
		PfxUInt32 rootA = findRoot( vertexIdA );
		PfxUInt32 rootB = findRoot( vertexIdB );
		if( rootA != rootB ) {
			vtxGroup[ rootB ] = rootA;
		}
	};

	// clear vertex group list
	for( PfxUInt32 i = 0; i < param.numVerts; i++ ) {
		vtxGroup[ i ] = i;
	}

	PfxUInt32 numValidTriangles = param.numTriangles;
	PfxUInt32 zeroTriangles = 0;
	PfxUInt32 counter = 0;
	do {
		zeroTriangles = 0;

		// Failed to remove all zero triangles due to any reasons
		if( counter > 100 ) {
			return SCE_PFX_ERR_ZERO_AREA_FACET;
		}

		for( PfxUInt32 i = 0; i < param.numTriangles; i++ ) {
			PfxUInt32 idx[ 3 ];
			idx[ 0 ] = idxList[ i * 3 + 0 ];
			idx[ 1 ] = idxList[ i * 3 + 1 ];
			idx[ 2 ] = idxList[ i * 3 + 2 ];

			if(idx[ 0 ] == (PfxUInt32)-1) continue; // removed triangle

			const PfxVector3 pnts[ 3 ] = {
				vertList[ idx[ 0 ] ].point,
				vertList[ idx[ 1 ] ].point,
				vertList[ idx[ 2 ] ].point,
			};

			//J 面積がfacetAreaLimit以下の面を排除
			//E Remove facets less than facetAreaLimit
			PfxFloat areaSqr = lengthSqr( cross( pnts[ 1 ] - pnts[ 0 ], pnts[ 2 ] - pnts[ 0 ] ) );
			if( ( param.flag & SCE_PFX_MESH_FLAG_AUTO_ELIMINATION ) && areaSqr < SCE_PFX_MESH_MINIMUM_AREA_FACET ) {
				if( param.flag & SCE_PFX_MESH_FLAG_FILL_REMOVED_FACETS ) {
					// It finds the shortest edge and removes it.
					PfxFloat l0 = lengthSqr( pnts[ 0 ] - pnts[ 1 ] );
					PfxFloat l1 = lengthSqr( pnts[ 1 ] - pnts[ 2 ] );
					PfxFloat l2 = lengthSqr( pnts[ 2 ] - pnts[ 0 ] );
					if( l0 < l1 ) {
						if( l0 < l2 ) {
							mergeSubTrees( idx[ 0 ], idx[ 1 ] ); // l0 is shortest
						}
						else {
							mergeSubTrees( idx[ 0 ], idx[ 2 ] ); // l2 is shortest
						}
					}
					else {
						if( l1 < l2 ) {
							mergeSubTrees( idx[ 1 ], idx[ 2 ] ); // l1 is shortest
						}
						else {
							mergeSubTrees( idx[ 0 ], idx[ 2 ] ); // l2 is shortest
						}
					}
				}

				// Mark this as the obsolete triangle
				idxList[ i * 3 + 0 ] = (PfxUInt32)-1;
				idxList[ i * 3 + 1 ] = (PfxUInt32)-1;
				idxList[ i * 3 + 2 ] = (PfxUInt32)-1;
				numValidTriangles--;
				zeroTriangles++;

				if( param.flag & SCE_PFX_MESH_FLAG_OUTPUT_INFO ) {
					SCE_PFX_PRINTF( "remove triangle (%d %d %d) : areaSqr %f P1(%.2f %.2f %.2f) P2(%.2f %.2f %.2f) P3(%.2f %.2f %.2f)\n",
						idx[ 0 ], idx[ 1 ], idx[ 2 ], areaSqr,
						( float )pnts[ 0 ][ 0 ], ( float )pnts[ 0 ][ 1 ], ( float )pnts[ 0 ][ 2 ],
						( float )pnts[ 1 ][ 0 ], ( float )pnts[ 1 ][ 1 ], ( float )pnts[ 1 ][ 2 ],
						( float )pnts[ 2 ][ 0 ], ( float )pnts[ 2 ][ 1 ], ( float )pnts[ 2 ][ 2 ] );
				}
			}
		}

		// Shrink same vertices
		for( PfxUInt32 i = 0; i < param.numVerts; i++ ) {
			PfxUInt32 rootId = findRoot( i );
			if( i != rootId ) {
				vertList[ i ].point = vertList[ rootId ].point;
			}
		}

		counter++;
	} while( numValidTriangles > 0 && zeroTriangles > 0 );

	// If all related triangles are removed after filling holes, it setup indices again and just removes small triangles.
	// This code is just a work around to keep existing assets built without an error.
	if( numValidTriangles == 0 ) {
		for( PfxUInt32 i = 0; i < param.numTriangles; i++ ) {
			void *ids = ( void* )( ( uintptr_t )param.triangles + param.triangleStrideBytes * i );
			PfxUInt32 idx[ 3 ];

			if( param.flag & SCE_PFX_MESH_FLAG_32BIT_INDEX ) {
				if( param.flag & SCE_PFX_MESH_FLAG_NORMAL_FLIP ) {
					idx[ 0 ] = ( ( PfxUInt32* )ids )[ 2 ];
					idx[ 1 ] = ( ( PfxUInt32* )ids )[ 1 ];
					idx[ 2 ] = ( ( PfxUInt32* )ids )[ 0 ];
				}
				else {
					idx[ 0 ] = ( ( PfxUInt32* )ids )[ 0 ];
					idx[ 1 ] = ( ( PfxUInt32* )ids )[ 1 ];
					idx[ 2 ] = ( ( PfxUInt32* )ids )[ 2 ];
				}
			}
			else if( param.flag & SCE_PFX_MESH_FLAG_16BIT_INDEX ) {
				if( param.flag & SCE_PFX_MESH_FLAG_NORMAL_FLIP ) {
					idx[ 0 ] = ( ( PfxUInt16* )ids )[ 2 ];
					idx[ 1 ] = ( ( PfxUInt16* )ids )[ 1 ];
					idx[ 2 ] = ( ( PfxUInt16* )ids )[ 0 ];
				}
				else {
					idx[ 0 ] = ( ( PfxUInt16* )ids )[ 0 ];
					idx[ 1 ] = ( ( PfxUInt16* )ids )[ 1 ];
					idx[ 2 ] = ( ( PfxUInt16* )ids )[ 2 ];
				}
			}

			const PfxVector3 pnts[ 3 ] = {
				vertList[ idx[ 0 ] ].point,
				vertList[ idx[ 1 ] ].point,
				vertList[ idx[ 2 ] ].point,
			};

			PfxFloat areaSqr = lengthSqr( cross( pnts[ 1 ] - pnts[ 0 ], pnts[ 2 ] - pnts[ 0 ] ) );
			if( ( param.flag & SCE_PFX_MESH_FLAG_AUTO_ELIMINATION ) && areaSqr < SCE_PFX_MESH_MINIMUM_AREA_FACET ) {
				idxList[ i * 3 + 0 ] = (PfxUInt32)-1;
				idxList[ i * 3 + 1 ] = (PfxUInt32)-1;
				idxList[ i * 3 + 2 ] = (PfxUInt32)-1;
			}
			else {
				idxList[ i * 3 + 0 ] = idx[ 0 ];
				idxList[ i * 3 + 1 ] = idx[ 1 ];
				idxList[ i * 3 + 2 ] = idx[ 2 ];
			}
		}
	}

	// Verify vertices and aggregate same vertices
	for (PfxUInt32 i = 0; i < param.numVerts; i++) {
		if (vertList[i].obsolete) continue;
		for (PfxUInt32 j = i + 1; j < param.numVerts; j++) {
			if (vertList[j].obsolete) continue;
			if (length(vertList[i].point - vertList[j].point) < SCE_PFX_MESH_CREATE_EPSILON) {
				vertList[j].obsolete = true;
				vertList[j].originalVertexId = vertList[i].originalVertexId;
			}
		}
	}

	PfxUInt32 assignedId = 0;
	for (PfxUInt32 i = 0; i < param.numVerts; i++) {
		if (!vertList[i].obsolete) {
			vertList[i].assignedVertexId = assignedId;
			assignedId++;
		}
		else {
			vertList[i].assignedVertexId = vertList[vertList[i].originalVertexId].assignedVertexId;
		}
	}

	for(PfxUInt32 i=0;i<param.numTriangles;i++) {
		PfxUInt32 idx[3];
		idx[ 0 ] = idxList[ i * 3 + 0 ];
		idx[ 1 ] = idxList[ i * 3 + 1 ];
		idx[ 2 ] = idxList[ i * 3 + 2 ];

		if( idx[ 0 ] == (PfxUInt32)-1 ) continue; // removed triangle

		facetList.push(idx[0]);
		facetList.push(idx[1]);
		facetList.push(idx[2]);
		if(param.userData) {
			userDataList.push(param.userData[i]);
		}
	}

	PfxUInt32 numAssignedVerts = assignedId;
	convexMeshImpl->m_numVerts = numAssignedVerts;
	convexMeshImpl->m_verts = (PfxFloat*)SCE_PFX_UTIL_ALLOC(16,sizeof(PfxFloat)*convexMeshImpl->m_numVerts*3);
	if (!convexMeshImpl->m_verts) return SCE_PFX_ERR_OUT_OF_BUFFER;
	for (PfxUInt32 i = 0; i < param.numVerts; i++) {
		if (!vertList[i].obsolete) {
			PfxUInt32 vid = vertList[i].assignedVertexId;
			pfxStoreVector3(vertList[i].point, convexMeshImpl->m_verts + vid * 3);
		}
	}
	
	convexMeshImpl->m_numFacets = facetList.size() / 3;
	convexMeshImpl->m_indices = (PfxUInt8 *)SCE_PFX_UTIL_ALLOC(16,sizeof(PfxUInt8)*convexMeshImpl->m_numFacets * 3);
	if (!convexMeshImpl->m_indices) return SCE_PFX_ERR_OUT_OF_BUFFER;
	for (PfxUInt32 i = 0; i < convexMeshImpl->m_numFacets * 3; i++) {
		convexMeshImpl->m_indices[i] = vertList[facetList[i]].assignedVertexId;
	}
	
	if(!userDataList.empty()) {
		convexMeshImpl->m_userData = (PfxUInt32 *)SCE_PFX_UTIL_ALLOC(16,sizeof(PfxUInt32)*userDataList.size());
		if (!convexMeshImpl->m_userData) return SCE_PFX_ERR_OUT_OF_BUFFER;
		for(PfxUInt32 i=0;i<userDataList.size();i++) {
			convexMeshImpl->m_userData[i] = userDataList[i];
		}
	}
	else {
		convexMeshImpl->m_userData = NULL;
	}
	
	convexMeshImpl->updateAABB();

#ifdef SCE_PFX_ENABLE_CUBEMAP_CONVEX_OPTIMIZE
	PfxInt32 ret = constructCubeMap(convexMeshImpl);
	if (ret != SCE_PFX_OK) return ret;
#endif

	char meshName[SCE_PFX_CONVEXMESH_NAME_STR_MAX + 1];
	for (int i = 0; i < SCE_PFX_CONVEXMESH_NAME_STR_MAX; i++) {
		convexMeshImpl->m_name[i] = meshName[i] = param.name[i];
		meshName[i] = param.name[i];
		if (param.name[i] == '\0') break;
	}
	meshName[SCE_PFX_CONVEXMESH_NAME_STR_MAX] = '\0';

	if(param.flag & SCE_PFX_MESH_FLAG_OUTPUT_INFO) {
		SCE_PFX_PRINTF("Create convex mesh \"%s\" %lu bytes\n", meshName,
			sizeof(PfxConvexMesh)+sizeof(PfxFloat)*convexMeshImpl->m_numVerts*3+sizeof(PfxUInt8)*convexMeshImpl->m_numFacets*3);
	}

	return SCE_PFX_OK;
}

void pfxReleaseConvexMesh(PfxConvexMesh &convexMesh)
{
	SCE_PFX_ASSERT(sizeof(PfxConvexMeshImpl) <= sizeof(PfxConvexMesh));

	PfxConvexMeshImpl *convexMeshImpl = (PfxConvexMeshImpl*)&convexMesh;
	
	if(convexMeshImpl->m_userData) {
		SCE_PFX_UTIL_FREE(convexMeshImpl->m_userData);
	}
	SCE_PFX_UTIL_FREE(convexMeshImpl->m_verts);
	SCE_PFX_UTIL_FREE(convexMeshImpl->m_indices);
	
	convexMeshImpl->m_numVerts = 0;
	convexMeshImpl->m_numFacets = 0;
}

///////////////////////////////////////////////////////////////////////////////
// ラージメッシュ作成時に使用する補助関数

static inline
PfxVector3 floorVec3(const PfxVector3 &v)
{
	return PfxVector3(floorf(v[0]),floorf(v[1]),floorf(v[2]));
}

static inline
PfxVector3 clampVec3(const PfxVector3 &v,const PfxVector3 &vmin,const PfxVector3 &vmax)
{
	return clampPerElem(v, vmin, vmax);
}

///////////////////////////////////////////////////////////////////////////////
//J ラージメッシュ作成時に使用する構造体
//E Structures used when creating Large Mesh

struct PfxMcEdgeEntry {
	PfxUInt8 vertId[2];
	PfxUInt8 facetId[2];
	PfxUInt8 numFacets;
	PfxUInt8 edgeNum[2];
	PfxUInt8 edgeId;
	SCE_PFX_PADDING(1,8)
	PfxVector3 dir;
	PfxMcFacet *facet[2];
	PfxMcEdgeEntry *next;
	SCE_PFX_PADDING(2,12)
};

typedef PfxMcFacet* PfxMcFacetPtr;

struct PfxMcBvhNode {
	PfxVector3 aabbMin;
	PfxVector3 aabbMax;
	PfxUInt32 nodeId;
	PfxUInt32 info;
	PfxMcBvhNode *left,*right,*parent;
	SCE_PFX_PADDING(1,4)

	// エンコード時に使う情報
	
	PfxBool isLeaf()
	{
		return !left && !right;
	}
};

struct PfxMcFacetLocal {
	const PfxMcFacet *facet;
	const PfxMcEdgeEntry *edge[3];
	PfxBool submit;
	PfxInt32 neighbor[3];
};

struct PfxMcFacet2 {
	const PfxMcFacet *facetA;
	const PfxMcFacet *facetB;
	const PfxMcEdgeEntry *edge;
};

///////////////////////////////////////////////////////////////////////////////
// ラージメッシュ作成関数

static
PfxInt32 countIsland(const PfxMcFacet *facets,PfxUInt32 numFacets,PfxUInt32 &numEdges,PfxUInt32 &numVerts,PfxUInt32 &numFacetsX2,PfxUInt32 maxVertices)
{
	numEdges = 0;
	numVerts = 0;
	
	if(!facets) return SCE_PFX_OK;
	
	PfxUInt32 numVertsFlag = (maxVertices*SCE_PFX_NUMMESHFACETS*3+31)/32;
	PfxArray<PfxUInt32> vertsFlag(numVertsFlag);
	vertsFlag.assign(numVertsFlag,0);
	
	PfxArray<PfxMcEdgeEntry*> edgeHead(numFacets*3);
	PfxArray<PfxMcEdgeEntry> edgeList(numFacets*3);
	PfxArray<PfxMcFacetLocal> facetList(numFacets);
	
	if(!(edgeHead.ptr()&&edgeList.ptr()&&facetList.ptr())) return SCE_PFX_ERR_OUT_OF_BUFFER;
	
	PfxMcEdgeEntry* nl = NULL;
	edgeHead.assign(numFacets*3,nl);
	edgeList.assign(numFacets*3,PfxMcEdgeEntry());
	
	PfxUInt32 vcnt = 0;
	PfxUInt32 ecnt = 0;
	PfxUInt32 fcnt = 0;
	for(const PfxMcFacet *facet = facets;facet;facet = facet->next,fcnt++) {
		PfxMcFacetLocal facetLocal;
		facetLocal.facet = facet;
		facetLocal.submit = false;
		facetLocal.neighbor[0] = -1;
		facetLocal.neighbor[1] = -1;
		facetLocal.neighbor[2] = -1;
		facetLocal.edge[0] = nullptr;
		facetLocal.edge[1] = nullptr;
		facetLocal.edge[2] = nullptr;

		// Vertex
		for(int v=0;v<3;v++) {
			PfxMcVert *vert = facet->v[v];
			PfxUInt32 idx = vert->vertId;
			PfxUInt32 mask = 1 << (idx & 31);
			if((vertsFlag[idx>>5] & mask) == 0) {
				if(vcnt >= SCE_PFX_NUMMESHVERTICES) {
					return SCE_PFX_ERR_OUT_OF_RANGE_VERTEX;
				}
                vertsFlag[idx>>5] |= mask;
				vert->localId = vcnt++;// アイランド単位の頂点インデックス
			}
		}
		
		// Edge
		for(int v=0;v<3;v++) {
			PfxUInt32 viMin = SCE_PFX_MIN(facet->v[v]->localId,facet->v[(v+1)%3]->localId);
			PfxUInt32 viMax = SCE_PFX_MAX(facet->v[v]->localId,facet->v[(v+1)%3]->localId);
			PfxUInt32 key = ((0x8da6b343*viMin+0xd8163841*viMax)%(numFacets*3));
			for(PfxMcEdgeEntry *e=edgeHead[key];;e=e->next) {
				if(!e) {
					edgeList[ecnt].vertId[0] = viMin;
					edgeList[ecnt].vertId[1] = viMax;
					edgeList[ecnt].numFacets = 1;
					edgeList[ecnt].facetId[0] = fcnt;
					//edgeList[ecnt].facet[0] = facet;
					edgeList[ecnt].edgeNum[0] = v;
					edgeList[ecnt].edgeId = ecnt;
					edgeList[ecnt].next = edgeHead[key];
					edgeHead[key] = &edgeList[ecnt];
					
					if(ecnt >= SCE_PFX_NUMMESHEDGES) {
						return SCE_PFX_ERR_OUT_OF_RANGE_EDGE;
					}
					ecnt++;
					break;
				}
				
				if(e->vertId[0] == viMin && e->vertId[1] == viMax && e->numFacets==1) {
					e->facetId[1] = fcnt;
					//e->facet[1] = facet;
					e->edgeNum[1] = v;
					e->numFacets = 2;
					
					facetLocal.neighbor[v] = e->facetId[0];
					facetList[e->facetId[0]].neighbor[e->edgeNum[0]] = fcnt;
					
					break;
				}
			}
		}
		
		facetList.push(facetLocal);
	}
	
	numEdges = ecnt;
	numVerts = vcnt;
	
	// Count facet x2
	PfxUInt32 f2cnt = 0;
	for(PfxUInt32 f=0;f<fcnt;f++) {
		PfxMcFacetLocal &flocal = facetList[f];

		if(flocal.submit) continue;
		
		PfxBool submit = false;
		
		for(int v=0;v<3&&!submit;v++) {
			if(flocal.neighbor[v] > 0 && !facetList[flocal.neighbor[v]].submit) {
				flocal.submit = true;
				facetList[flocal.neighbor[v]].submit = true;
				submit = true;
				break;
			}
		}
		
		f2cnt++;
	}
	
	numFacetsX2 = f2cnt;
	
	return SCE_PFX_OK;
}

static
PfxInt32 submitIsland(const PfxCreateLargeTriMeshParam &param,PfxExpandedTriMesh *island,PfxMcFacet *facets,PfxUInt32 numFacets,const PfxLargeTriMeshImpl *largeMeshImpl,PfxUInt32 maxVertices)
{
	if(!facets) return SCE_PFX_OK;
	
	PfxUInt32 numVertsFlag = (maxVertices*SCE_PFX_NUMMESHFACETS*3+31)/32;
	PfxArray<PfxUInt32> vertsFlag(numVertsFlag);
	vertsFlag.assign(numVertsFlag,0);
	
	PfxArray<PfxMcEdgeEntry*> edgeHead(numFacets*3);
	PfxArray<PfxMcEdgeEntry> edgeList(numFacets*3);
	
	if(!(edgeHead.ptr()&&edgeList.ptr())) return SCE_PFX_ERR_OUT_OF_BUFFER;
	
	PfxMcEdgeEntry* nl = NULL;
	edgeHead.assign(numFacets*3,nl);
	edgeList.assign(numFacets*3,PfxMcEdgeEntry());
	
	PfxUInt32 vcnt = 0;
	PfxUInt32 ecnt = 0;
	PfxUInt32 f = 0;
	for(PfxMcFacet *facet = facets;facet;facet = facet->next,f++) {
		PfxMcFacet &iFacet = *facet;
		PfxMcEdge *iEdge[3] = {
			iFacet.e[0],
			iFacet.e[1],
			iFacet.e[2],
		};

		PfxExpandedFacet &oFacet = island->m_facets[f];
		
		oFacet.m_center = (PfxFloat3)(0.5f * (iFacet.aabbMax + iFacet.aabbMin));
		oFacet.m_half = (PfxFloat3)(0.5f * (iFacet.aabbMax - iFacet.aabbMin) + PfxVector3(0.00001f)); // Slightly stretch to avoid collision hole
		oFacet.m_normal = (PfxFloat3)iFacet.normal;
		oFacet.m_thickness = iFacet.thickness;
		
		// Vertex
		for(int v=0;v<3;v++) {
			PfxMcVert *vert = facet->v[v];
			PfxUInt32 idx = vert->vertId;
			PfxUInt32 mask = 1 << (idx & 31);
			if((vertsFlag[idx>>5] & mask) == 0) {
				if(vcnt >= SCE_PFX_NUMMESHVERTICES) {
					return SCE_PFX_ERR_OUT_OF_RANGE_VERTEX;
				}
				vertsFlag[idx>>5] |= mask;
				SCE_PFX_ASSERT(vcnt < island->m_numVerts);
				vert->localId = vcnt++;// アイランド単位の頂点インデックス
				island->m_verts[vert->localId] = (PfxFloat3)vert->coord;
			}

			oFacet.m_vertIds[v] = (PfxUInt8)vert->localId;
		}
		
		// Edge
		for(int v=0;v<3;v++) {
			PfxUInt32 viMin = SCE_PFX_MIN(oFacet.m_vertIds[v],oFacet.m_vertIds[(v+1)%3]);
			PfxUInt32 viMax = SCE_PFX_MAX(oFacet.m_vertIds[v],oFacet.m_vertIds[(v+1)%3]);
			PfxUInt32 key = ((0x8da6b343*viMin+0xd8163841*viMax)%(island->m_numFacets*3));
			for(PfxMcEdgeEntry *e=edgeHead[key];;e=e->next) {
				if(!e) {
					edgeList[ecnt].vertId[0] = viMin;
					edgeList[ecnt].vertId[1] = viMax;
					edgeList[ecnt].facetId[0] = f;
					edgeList[ecnt].facet[0] = facet;
					edgeList[ecnt].numFacets = 1;
					edgeList[ecnt].edgeNum[0] = v;
					edgeList[ecnt].edgeId = ecnt;
					if(viMin == oFacet.m_vertIds[v]) {
						edgeList[ecnt].dir = normalize(facet->v[(v+1)%3]->coord-facet->v[v]->coord);
					}
					else {
						edgeList[ecnt].dir = normalize(facet->v[v]->coord-facet->v[(v+1)%3]->coord);
					}
					edgeList[ecnt].next = edgeHead[key];
					edgeHead[key] = &edgeList[ecnt];
					
					PfxEdge edge;
					edge.m_angleType = iEdge[v]->angleType;
					// 厚み角の設定 0～π/2を0～255の整数値に変換して格納
					edge.m_tilt = (PfxUInt8)((iEdge[v]->angle/(0.5f*SCE_PFX_PI))*255.0f);
					edge.m_vertId[0] = viMin;
					edge.m_vertId[1] = viMax;
					
					oFacet.m_edgeIds[v] = ecnt;
					island->m_edges[ecnt] = edge;
					ecnt++;
					SCE_PFX_ASSERT(ecnt <= island->m_numEdges);
					break;
				}
				
				if(e->vertId[0] == viMin && e->vertId[1] == viMax && e->numFacets==1) {
					e->facetId[1] = f;
					e->facet[1] = facet;
					e->edgeNum[1] = v;
					e->numFacets = 2;
					oFacet.m_edgeIds[v] = e->edgeId;
					break;
				}
			}
		}

		oFacet.m_userData = facet->userData;
	}

	return SCE_PFX_OK;
}

static inline
PfxUInt32 floatFlip(PfxFloat flt)
{
	PfxUInt32 iflt = *((PfxUInt32*)&flt);
	PfxInt32 msb = (PfxInt32)(iflt >> 31);
	PfxUInt32 msbMask = -msb;
	PfxUInt32 maskSignFlip = msbMask | 0x80000000;
	return iflt ^ maskSignFlip;
}

struct PfxMcSortedFacetsList {
	PfxArray<PfxSortData16>  facets;
	PfxArray<PfxSortData16>  work;

	PfxBool initialize(PfxUInt32 n)
	{
		PfxSortData16 dummy = {};
		facets.assign(n, dummy);
		work.assign(n, dummy);
		return facets.ptr() && work.ptr();
	}

	void set(PfxUInt32 i, const PfxFloat fval, void *ptr)
	{
		pfxSetKey(facets[i], floatFlip(fval));
		//		pfxSetKey(facets[i], *((PfxUInt32*)&fval));
		PfxUInt64 ptr64 = (uintptr_t)ptr;
		facets[i].set32(0, (PfxUInt32)(ptr64 & 0xffffffff));
		facets[i].set32(1, (PfxUInt32)(ptr64 >> 32));
	}

	void set(PfxUInt32 i, const PfxFloat fval, void *ptr, PfxUInt32 userData)
	{
		set(i, fval, ptr);
		facets[i].set32(2, userData);
	}

	void set(PfxUInt32 i, PfxUInt32 keyVal, void *ptr, PfxUInt32 userData)
	{
		pfxSetKey(facets[i], keyVal);
		PfxUInt64 ptr64 = (uintptr_t)ptr;
		facets[i].set32(0, (PfxUInt32)(ptr64 & 0xffffffff));
		facets[i].set32(1, (PfxUInt32)(ptr64 >> 32));
		facets[i].set32(2, userData);
	}

	void remove(PfxUInt32 i)
	{
		facets.remove(i);
		work.remove(i);
	}

	void *getPointer(PfxUInt32 i)
	{
		PfxUInt64 ptr32_0 = (PfxUInt64)facets[i].get32(0);
		PfxUInt64 ptr32_1 = (PfxUInt64)facets[i].get32(1);
		return (void*)uintptr_t((ptr32_1 << 32) | ptr32_0);
	}

	PfxUInt32 getKey(PfxUInt32 i)
	{
		return pfxGetKey(facets[i]);
	}

	PfxUInt32 getUserData(PfxUInt32 i)
	{
		return facets[i].get32(2);
	}

	PfxUInt32 size()
	{
		return facets.size();
	}

	void sort()
	{
		pfxSort(facets.ptr(), work.ptr(), facets.size());
	}
};

static SCE_PFX_FORCE_INLINE
PfxUInt32 encodeMortonCode2(PfxInt32 x) {
	x = SCE_PFX_MIN(x, 0x000003ff);		// x = ---- ---- ---- ---- ---- --98 7654 3210
	x = (x ^ (x << 10)) & 0x000f801f;	// x = ---- ---- ---- 9876 5--- ---- ---4 3210
	x = (x ^ (x << 4)) & 0x00e181c3;	// x = ---- ---- 987- ---6 5--- ---4 32-- --10
	x = (x ^ (x << 2)) & 0x03248649;	// x = ---- --98 --7- -6-- 5--- -43- -2-- 1--0
	x = (x ^ (x << 2)) & 0x09249249;	// x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
	return x;
}

static SCE_PFX_FORCE_INLINE
PfxUInt32 toMortonCode(const PfxVector3 &v) {
	PfxVecInt3 iv(v);
	return ((encodeMortonCode2(iv.getX()) << 2 | encodeMortonCode2(iv.getZ()) << 1 | encodeMortonCode2(iv.getY())) & 0x3fffffff);
}


static
PfxInt32 submitIsland(const PfxCreateLargeTriMeshParam &param,PfxQuantizedTriMeshBvh *island,PfxMcFacet *facets,PfxUInt32 numFacets,const PfxLargeTriMeshImpl *largeMeshImpl,PfxUInt32 maxVertices)
{
	SCE_PFX_ASSERT(island->m_numFacets == numFacets);
	
	if(!facets) return SCE_PFX_OK;
	
	if(numFacets == 1) {
		PfxFacetBvhNode encnode;
		encnode.flag = 0x09;
		island->m_bvhNodes[0] = encnode;
	}
	else {
		// BVHの構築
		PfxMcBvhNode *bvhNodes = (PfxMcBvhNode*)SCE_PFX_UTIL_ALLOC(16,sizeof(PfxMcBvhNode)*(island->m_numFacets*2-1));
		if(!bvhNodes) return SCE_PFX_ERR_OUT_OF_BUFFER;
		
		PfxArray<PfxUInt32> bvhNodeIndices(island->m_numFacets*2-1);
		if(!bvhNodeIndices.ptr()) return SCE_PFX_ERR_OUT_OF_BUFFER;

		if (((param.flag & SCE_PFX_MESH_FLAG_OPTIMIZE_BVH_STRUCTURE) == SCE_PFX_MESH_FLAG_OPTIMIZE_BVH_STRUCTURE) ||
			((param.flag & SCE_PFX_MESH_FLAG_FAST_BVH_CONSTRUCTION) == SCE_PFX_MESH_FLAG_FAST_BVH_CONSTRUCTION)) {
			PfxMcSortedFacetsList sortedFacetsList;
			if (!sortedFacetsList.initialize(island->m_numFacets))
				return SCE_PFX_ERR_OUT_OF_BUFFER;

			{
				PfxUInt32 f = 0;
				PfxVector3 centerMin(SCE_PFX_FLT_MAX), centerMax(-SCE_PFX_FLT_MAX);
				for (const PfxMcFacet *facet = facets; facet; facet = facet->next, f++) {
					PfxVector3 center = (facet->aabbMin + facet->aabbMax) * 0.5f;
					centerMin = minPerElem(centerMin, center);
					centerMax = maxPerElem(centerMax, center);
				}
				PfxVector3 centerSize = centerMax - centerMin;
				PfxFloat maxCenterSize = maxElem(centerSize);

				f = 0;
				PfxVector3 invSize(PfxFloat(0x3ff) / maxCenterSize);
				for (PfxMcFacet * facet = facets; facet != NULL; facet = facet->next, f++) {
					PfxVector3 center = (facet->aabbMin + facet->aabbMax) * 0.5f;
					PfxVector3 normalizedCenter = mulPerElem(center - centerMin, invSize);
					PfxUInt32 mortonCode = toMortonCode(normalizedCenter);
					sortedFacetsList.set(f, mortonCode, facet, f);
				}
			}

			sortedFacetsList.sort();

			// AABB配列を作成する
			PfxUInt32 numBvhNodes = numFacets;
			facets = NULL;
			for (PfxInt32 f = numFacets - 1; f >= 0; --f) {
				PfxMcFacet *facet = ((PfxMcFacet*)(sortedFacetsList.getPointer(f)));
				bvhNodes[f].aabbMin = facet->aabbMin;
				bvhNodes[f].aabbMax = facet->aabbMax;
				bvhNodes[f].nodeId = f;
				bvhNodes[f].left = NULL;
				bvhNodes[f].right = NULL;
				bvhNodes[f].parent = NULL;
				bvhNodeIndices.push(f);
				facet->next = facets;
				facets = facet;
			}

			// 最下層から順に最もAABBサイズの小さくなる組み合わせを検出する ※O(n^3)
			while (bvhNodeIndices.size() > 1) {
				// AABBが最小となるペアを検出
				PfxUInt32 i = 0, j = 0;
				{
					PfxUInt32 pMin = 0, qMin = 0;
					PfxFloat volumeMin = SCE_PFX_FLT_MAX;
					for (PfxUInt32 p = 0; p<bvhNodeIndices.size(); p++) {
						for (PfxUInt32 q = p + 1; q<bvhNodeIndices.size() && q <= p + 4; q++) {
							PfxMcBvhNode &nodeA = bvhNodes[bvhNodeIndices[p]];
							PfxMcBvhNode &nodeB = bvhNodes[bvhNodeIndices[q]];
							PfxVector3 aabbMin = minPerElem(nodeA.aabbMin, nodeB.aabbMin);
							PfxVector3 aabbMax = maxPerElem(nodeA.aabbMax, nodeB.aabbMax);
							PfxVector3 extent = aabbMax - aabbMin;
							PfxFloat volume = extent[0] + extent[1] + extent[2];
							if (volume < volumeMin) {
								volumeMin = volume;
								pMin = p;
								qMin = q;
							}
						}
					}
					i = pMin;
					j = qMin;

					SCE_PFX_ASSERT(i != j);
				}

				// 新しいノードを作成
				PfxUInt32 newId = numBvhNodes++;
				PfxMcBvhNode &newNode = bvhNodes[newId];
				PfxMcBvhNode &leftNode = bvhNodes[bvhNodeIndices[i]];
				PfxMcBvhNode &rightNode = bvhNodes[bvhNodeIndices[j]];
				newNode.aabbMin = minPerElem(leftNode.aabbMin, rightNode.aabbMin);
				newNode.aabbMax = maxPerElem(leftNode.aabbMax, rightNode.aabbMax);
				newNode.nodeId = newId;
				newNode.left = &leftNode;
				newNode.right = &rightNode;
				newNode.parent = NULL;
				leftNode.parent = &newNode;
				rightNode.parent = &newNode;

				// ペアのノードを除去
				bvhNodeIndices[i] = newId;
				for (PfxUInt32 k = j + 1; k < bvhNodeIndices.size(); ++k) {
					bvhNodeIndices[k - 1] = bvhNodeIndices[k];
				}
				bvhNodeIndices.remove(bvhNodeIndices.size() - 1);
			}
		}
		else
		{
			// AABB配列を作成する
			PfxUInt32 numBvhNodes = numFacets;
			PfxUInt32 f = 0;
			for (const PfxMcFacet *facet = facets; facet; facet = facet->next, f++) {
				bvhNodes[f].aabbMin = facet->aabbMin;
				bvhNodes[f].aabbMax = facet->aabbMax;
				bvhNodes[f].nodeId = f;
				bvhNodes[f].left = NULL;
				bvhNodes[f].right = NULL;
				bvhNodes[f].parent = NULL;
				bvhNodeIndices.push(f);
			}

			// 最下層から順に最もAABBサイズの小さくなる組み合わせを検出する ※O(n^3)
			while (bvhNodeIndices.size() > 1) {
				// AABBが最小となるペアを検出
				PfxUInt32 i = 0, j = 0;
				{
					PfxUInt32 pMin = 0, qMin = 0;
					PfxFloat volumeMin = SCE_PFX_FLT_MAX;
					for (unsigned int p = 0; p < bvhNodeIndices.size(); p++) {
						for (unsigned int q = p + 1; q < bvhNodeIndices.size(); q++) {
							PfxMcBvhNode &nodeA = bvhNodes[bvhNodeIndices[p]];
							PfxMcBvhNode &nodeB = bvhNodes[bvhNodeIndices[q]];
							PfxVector3 aabbMin = minPerElem(nodeA.aabbMin, nodeB.aabbMin);
							PfxVector3 aabbMax = maxPerElem(nodeA.aabbMax, nodeB.aabbMax);
							PfxVector3 extent = aabbMax - aabbMin;
							PfxFloat volume = extent[0] + extent[1] + extent[2];
							if (volume < volumeMin) {
								volumeMin = volume;
								pMin = p;
								qMin = q;
							}
						}
					}
					i = pMin;
					j = qMin;

					SCE_PFX_ASSERT(i != j);
				}

				// 新しいノードを作成
				PfxUInt32 newId = numBvhNodes++;
				PfxMcBvhNode &newNode = bvhNodes[newId];
				PfxMcBvhNode &leftNode = bvhNodes[bvhNodeIndices[i]];
				PfxMcBvhNode &rightNode = bvhNodes[bvhNodeIndices[j]];
				newNode.aabbMin = minPerElem(leftNode.aabbMin, rightNode.aabbMin);
				newNode.aabbMax = maxPerElem(leftNode.aabbMax, rightNode.aabbMax);
				newNode.nodeId = newId;
				newNode.left = &leftNode;
				newNode.right = &rightNode;
				newNode.parent = NULL;
				leftNode.parent = &newNode;
				rightNode.parent = &newNode;

				// ペアのノードを除去
				bvhNodeIndices.remove(SCE_PFX_MAX(i, j));
				bvhNodeIndices.remove(SCE_PFX_MIN(i, j));

				// 新しいノードを登録
				bvhNodeIndices.push(newId);
			}
		}

		//PfxMcBvhNode *root = &bvhNodes[bvhNodeIndices[0]];
		//printBvh(root);

		//SCE_PFX_PRINTF("numFacets %d numBvhNodes %d\n",island->m_numFacets,numBvhNodes);
		//SCE_PFX_PRINTF("m_bvhRootId %u/%u\n",bvhNodeIndices[0],island->m_numFacets*2-2);

		//island->m_bvhRootId = bvhNodeIndices[0];

		// BVHをエンコード
		PfxMcBvhNode *root = &bvhNodes[bvhNodeIndices[0]];

		// encode nodes
		PfxStaticStack<PfxMcBvhNode*> nodeStack;
		nodeStack.push(root);
		
		PfxUInt32 numEncNodes = 0;
		
		while(!nodeStack.empty()) {
			PfxMcBvhNode *node = nodeStack.top();
			nodeStack.pop();
			
			node->info = numEncNodes;
			PfxFacetBvhNode encnode;
			
			if(node->parent) {
				PfxVector3 aabbMinP = node->parent->aabbMin;
				PfxVector3 aabbMaxP = node->parent->aabbMax;
				PfxVector3 lenP = aabbMaxP - aabbMinP;
				PfxVector3 aabbMinN = node->aabbMin;
				PfxVector3 aabbMaxN = node->aabbMax;
				PfxVector3 quantizedMin = floorVec3(255.0f * clampVec3(divPerElem(aabbMinN - aabbMinP,lenP),PfxVector3::zero(),PfxVector3(1.0f)));
				PfxVector3 quantizedMax = floorVec3(255.0f * clampVec3(divPerElem(aabbMaxP - aabbMaxN,lenP),PfxVector3::zero(),PfxVector3(1.0f)));
				encnode.aabb[0] = (PfxUInt8)quantizedMin[0];
				encnode.aabb[1] = (PfxUInt8)quantizedMin[1];
				encnode.aabb[2] = (PfxUInt8)quantizedMin[2];
				encnode.aabb[3] = (PfxUInt8)quantizedMax[0];
				encnode.aabb[4] = (PfxUInt8)quantizedMax[1];
				encnode.aabb[5] = (PfxUInt8)quantizedMax[2];

				// AABBの縮退を防ぐ
				if(encnode.aabb[0] > 0) encnode.aabb[0]--;
				if(encnode.aabb[1] > 0) encnode.aabb[1]--;
				if(encnode.aabb[2] > 0) encnode.aabb[2]--;
				if(encnode.aabb[3] > 0) encnode.aabb[3]--;
				if(encnode.aabb[4] > 0) encnode.aabb[4]--;
				if(encnode.aabb[5] > 0) encnode.aabb[5]--;

				// デコードした値で書き換えておく
				PfxVector3 decMin = aabbMinP + mulPerElem((aabbMaxP-aabbMinP),PfxVector3(encnode.aabb[0],encnode.aabb[1],encnode.aabb[2]) / 255.0f);
				PfxVector3 decMax = aabbMaxP - mulPerElem((aabbMaxP-aabbMinP),PfxVector3(encnode.aabb[3],encnode.aabb[4],encnode.aabb[5]) / 255.0f);

				SCE_PFX_ASSERT(decMin[0] <= aabbMinN[0]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(decMin[1] <= aabbMinN[1]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(decMin[2] <= aabbMinN[2]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(aabbMaxN[0] <= decMax[0]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(aabbMaxN[1] <= decMax[1]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(aabbMaxN[2] <= decMax[2]+SCE_PFX_MESH_CREATE_EPSILON);

				node->aabbMin = decMin;
				node->aabbMax = decMax;
				
				// Connect to the parent
				SCE_PFX_ASSERT(node->parent->info < numFacets-1);
				PfxFacetBvhNode *encparent = island->m_bvhNodes + node->parent->info;
				if(node->parent->left == node) {
					encparent->left = numEncNodes;
				}
				else {
					encparent->right = numEncNodes;
				}
			}
			
			if(node->left->isLeaf()) {
				encnode.flag |= 0x01;
				encnode.left = node->left->nodeId;
			}
			else {
				nodeStack.push(node->left);
			}
			
			if(node->right->isLeaf()) {
				encnode.flag |= 0x04;
				encnode.right = node->right->nodeId;
			}
			else {
				nodeStack.push(node->right);
			}
			
			island->m_bvhNodes[numEncNodes++] = encnode;
		}

		SCE_PFX_UTIL_FREE(bvhNodes);
	}
	
	// アイランド作成
	{
		PfxUInt32 numVertsFlag = (maxVertices*SCE_PFX_NUMMESHFACETS*3+31)/32;
		PfxArray<PfxUInt32> vertsFlag(numVertsFlag);
		vertsFlag.assign(numVertsFlag,0);

		PfxArray<PfxMcEdgeEntry*> edgeHead(numFacets*3);
		PfxArray<PfxMcEdgeEntry> edgeList(numFacets*3);
		
		if(!(edgeHead.ptr()&&edgeList.ptr())) return SCE_PFX_ERR_OUT_OF_BUFFER;
		
		PfxMcEdgeEntry* nl = NULL;
		edgeHead.assign(numFacets*3,nl);
		edgeList.assign(numFacets*3,PfxMcEdgeEntry());
		
		PfxUInt32 vcnt = 0;
		PfxUInt32 ecnt = 0;
		PfxUInt32 f = 0;
		for(PfxMcFacet *facet = facets;facet;facet = facet->next,f++) {
			PfxMcFacet &iFacet = *facet;
			PfxMcEdge *iEdge[3] = {
				iFacet.e[0],
				iFacet.e[1],
				iFacet.e[2],
			};
			
			PfxQuantizedFacetBvh &oFacet = island->m_facets[f];
			
			oFacet.m_normal = largeMeshImpl->quantizeNormal(iFacet.normal);
			oFacet.m_thickness = largeMeshImpl->quantizeFloat(iFacet.thickness);
			
			// Vertex
			for(int v=0;v<3;v++) {
				PfxMcVert *vert = facet->v[v];
				PfxUInt32 idx = vert->vertId;
				PfxUInt32 mask = 1 << (idx & 31);
				if((vertsFlag[idx>>5] & mask) == 0) {
					if(vcnt >= SCE_PFX_NUMMESHVERTICES) {
						return SCE_PFX_ERR_OUT_OF_RANGE_VERTEX;
					}
					vertsFlag[idx>>5] |= mask;
					SCE_PFX_ASSERT(vcnt < island->m_numVerts);
					vert->localId = vcnt++;// アイランド単位の頂点インデックス
					island->m_verts[vert->localId] = vert->quant;
				}

				oFacet.m_vertIds[v] = (PfxUInt8)vert->localId;
			}
			//量子化後の面積チェック
			const PfxFloat epsilon = SCE_PFX_MESH_MINIMUM_AREA_FACET;
			PfxVector3 tp0 = largeMeshImpl->decodePosition(island->m_verts[oFacet.m_vertIds[0]]);
			PfxVector3 tp1 = largeMeshImpl->decodePosition(island->m_verts[oFacet.m_vertIds[1]]);
			PfxVector3 tp2 = largeMeshImpl->decodePosition(island->m_verts[oFacet.m_vertIds[2]]);
			PfxFloat areaSqr = lengthSqr(cross(tp1-tp0,tp2-tp0));
			if(areaSqr<epsilon) {
				return SCE_PFX_ERR_ZERO_AREA_FACET;
			}

			// Edge
			for(int v=0;v<3;v++) {
				PfxUInt32 viMin = SCE_PFX_MIN(oFacet.m_vertIds[v],oFacet.m_vertIds[(v+1)%3]);
				PfxUInt32 viMax = SCE_PFX_MAX(oFacet.m_vertIds[v],oFacet.m_vertIds[(v+1)%3]);
				PfxUInt32 key = ((0x8da6b343*viMin+0xd8163841*viMax)%(island->m_numFacets*3));
				for(PfxMcEdgeEntry *e=edgeHead[key];;e=e->next) {
					if(!e) {
						edgeList[ecnt].vertId[0] = viMin;
						edgeList[ecnt].vertId[1] = viMax;
						edgeList[ecnt].facetId[0] = f;
						edgeList[ecnt].facet[0] = facet;
						edgeList[ecnt].numFacets = 1;
						edgeList[ecnt].edgeNum[0] = v;
						edgeList[ecnt].edgeId = ecnt;
						if(viMin == oFacet.m_vertIds[v]) {
							edgeList[ecnt].dir = normalize(facet->v[(v+1)%3]->coord-facet->v[v]->coord);
						}
						else {
							edgeList[ecnt].dir = normalize(facet->v[v]->coord-facet->v[(v+1)%3]->coord);
						}
						edgeList[ecnt].next = edgeHead[key];
						edgeHead[key] = &edgeList[ecnt];
						
						PfxEdge edge;
						edge.m_angleType = iEdge[v]->angleType;
						// 厚み角の設定 0～π/2を0～255の整数値に変換して格納
						edge.m_tilt = (PfxUInt8)((iEdge[v]->angle/(0.5f*SCE_PFX_PI))*255.0f);
						edge.m_vertId[0] = viMin;
						edge.m_vertId[1] = viMax;
						
						oFacet.m_edgeIds[v] = ecnt;
						island->m_edges[ecnt] = edge;
						ecnt++;
						SCE_PFX_ASSERT(ecnt <= island->m_numEdges);
						break;
					}
					
					if(e->vertId[0] == viMin && e->vertId[1] == viMax && e->numFacets==1) {
						e->facetId[1] = f;
						e->facet[1] = facet;
						e->edgeNum[1] = v;
						e->numFacets = 2;
						oFacet.m_edgeIds[v] = e->edgeId;
						break;
					}
				}
			}

			oFacet.m_userData = facet->userData;
		}
		
		SCE_PFX_ASSERT(island->m_numEdges == ecnt);

		PfxQuantize3 q(2,2,2);
		//PfxVector3 stretch = largeMeshImpl->decodeVector(q);

		// Slightly stretch edge of the island to avoid collision hole
		/*
		for(PfxUInt32 e=0;e<ecnt;e++) {
			if(edgeList[e].numFacets == 1) {
				PfxMcFacet &iFacet = *edgeList[e].facet[0];
				PfxVector3 center = 0.5f * (iFacet.aabbMax + iFacet.aabbMin);
				
				int vfId0 = edgeList[e].edgeNum[0];
				int vfId1 = (edgeList[e].edgeNum[0]+1)%3;
				
				PfxVector3 p0 = iFacet.v[vfId0]->coord;
				PfxVector3 p1 = iFacet.v[vfId1]->coord;
				
				p0 += normalize(p0 - center) * maxElem(stretch);
				p1 += normalize(p1 - center) * maxElem(stretch);
				
				island->m_verts[iFacet.v[vfId0]->localId] = largeMeshImpl->quantizePosition(p0);
				island->m_verts[iFacet.v[vfId1]->localId] = largeMeshImpl->quantizePosition(p1);
			}
		}
		*/
	}

	return SCE_PFX_OK;
}

PfxInt32 submitIsland(
	const PfxCreateLargeTriMeshParam &param,
	PfxCompressedTriMesh *island,
	PfxQuantize3 *vertArry,PfxCompressedFacet2 *facet2Array,PfxFacetBvhNode *bvhNodeArray,
	PfxMcFacet *facets,PfxUInt32 numFacets,PfxUInt32 numFacetsX2,
	const PfxLargeTriMeshImpl *largeMeshImpl,PfxUInt32 maxVertices)
{
	SCE_PFX_ASSERT(island->m_numFacets == numFacetsX2);

	if(!facets) return SCE_PFX_OK;
	
	// アイランド作成
	PfxUInt32 numVertsFlag = (maxVertices*SCE_PFX_NUMMESHFACETS*3+31)/32;
	PfxArray<PfxUInt32> vertsFlag(numVertsFlag);
	vertsFlag.assign(numVertsFlag,0);

	PfxArray<PfxMcEdgeEntry*> edgeHead(numFacets*3);
	PfxArray<PfxMcEdgeEntry> edgeList(numFacets*3);
	PfxArray<PfxMcFacetLocal> facetList(numFacets);
	
	if(!(edgeHead.ptr()&&edgeList.ptr()&&facetList.ptr())) return SCE_PFX_ERR_OUT_OF_BUFFER;
	
	PfxMcEdgeEntry* nl = NULL;
	edgeHead.assign(numFacets*3,nl);
	edgeList.assign(numFacets*3,PfxMcEdgeEntry());
	
	PfxUInt32 vcnt = 0;
	PfxUInt32 ecnt = 0;
	PfxUInt32 fcnt = 0;
	
	for(PfxMcFacet *facet = facets;facet;facet = facet->next,fcnt++) {
		PfxMcFacetLocal facetLocal;
		facetLocal.facet = facet;
		facetLocal.submit = false;
		facetLocal.neighbor[0] = -1;
		facetLocal.neighbor[1] = -1;
		facetLocal.neighbor[2] = -1;
		facetLocal.edge[0] = nullptr;
		facetLocal.edge[1] = nullptr;
		facetLocal.edge[2] = nullptr;

		// Vertex
		for(int v=0;v<3;v++) {
			PfxMcVert *vert = facet->v[v];
			PfxUInt32 idx = vert->vertId;
			PfxUInt32 mask = 1 << (idx & 31);
			if((vertsFlag[idx>>5] & mask) == 0) {
				if(vcnt >= SCE_PFX_NUMMESHVERTICES) {
					return SCE_PFX_ERR_OUT_OF_RANGE_VERTEX;
				}
				vertsFlag[idx>>5] |= mask;
				SCE_PFX_ASSERT(vcnt < island->m_numVerts);
				vert->localId = vcnt++;// アイランド単位の頂点インデックス
				vertArry[island->m_verts + vert->localId] = vert->quant;
			}
		}
		
		// Edge
		for(int v=0;v<3;v++) {
			PfxUInt32 viMin = SCE_PFX_MIN(facet->v[v]->localId,facet->v[(v+1)%3]->localId);
			PfxUInt32 viMax = SCE_PFX_MAX(facet->v[v]->localId,facet->v[(v+1)%3]->localId);
			PfxUInt32 key = ((0x8da6b343*viMin+0xd8163841*viMax)%(numFacets*3));
			for(PfxMcEdgeEntry *e=edgeHead[key];;e=e->next) {
				if(!e) {
					edgeList[ecnt].vertId[0] = viMin;
					edgeList[ecnt].vertId[1] = viMax;
					edgeList[ecnt].numFacets = 1;
					edgeList[ecnt].facetId[0] = fcnt;
					edgeList[ecnt].facet[0] = facet;
					edgeList[ecnt].edgeNum[0] = v;
					edgeList[ecnt].edgeId = ecnt;
					edgeList[ecnt].next = edgeHead[key];
					edgeHead[key] = &edgeList[ecnt];
					
					if(ecnt >= SCE_PFX_NUMMESHEDGES) {
						return SCE_PFX_ERR_OUT_OF_RANGE_EDGE;
					}
					ecnt++;
					break;
				}
				
				if(e->vertId[0] == viMin && e->vertId[1] == viMax && e->numFacets==1) {
					e->facetId[1] = fcnt;
					e->facet[1] = facet;
					e->edgeNum[1] = v;
					e->numFacets = 2;
					
					facetLocal.neighbor[v] = e->facetId[0];
					facetLocal.edge[v] = e;
					facetList[e->facetId[0]].neighbor[e->edgeNum[0]] = fcnt;
					facetList[e->facetId[0]].edge[e->edgeNum[0]] = e;
					break;
				}
			}
		}
		facetList.push(facetLocal);
	}
	
	// Create facet2
	PfxArray<PfxMcFacet2> facet2List(numFacetsX2);
	
	PfxUInt32 f2cnt = 0;
	for(PfxUInt32 f=0;f<fcnt;f++) {
		PfxMcFacetLocal &flocal = facetList[f];
		
		if(flocal.submit) continue;
		
		PfxBool isDouble = false;
		int v=0;
		for(;v<3;v++) {
			if(flocal.neighbor[v] > 0 && !facetList[flocal.neighbor[v]].submit) {
				flocal.submit = true;
				facetList[flocal.neighbor[v]].submit = true;
				isDouble = true;
				//SCE_PFX_PRINTF("submit %d %d\n",f,flocal.neighbor[v]);
				break;
			}
		}
		
		SCE_PFX_ASSERT(f2cnt < numFacetsX2);
		f2cnt++;
		
		PfxMcFacet2 facet2;
		
		if(isDouble) {
			facet2.edge = flocal.edge[v];
			facet2.facetA = flocal.facet;
			facet2.facetB = facetList[flocal.edge[v]->facetId[1]].facet;
		}
		else {
			facet2.edge = NULL;
			facet2.facetA = flocal.facet;
			facet2.facetB = NULL;
		}
		
		facet2List.push(facet2);
	}
	
	for(PfxUInt32 f=0,offsetId=0;f<f2cnt;f++) {
		PfxMcFacet2 &iFacet2 = facet2List[f];
		PfxCompressedFacet2 &oFacet2 = facet2Array[island->m_facets + f];
		
		// encode 2 facets
		if(iFacet2.edge) {
			SCE_PFX_ASSERT(iFacet2.edge->facet[0] == iFacet2.facetA);
			SCE_PFX_ASSERT(iFacet2.edge->facet[1] == iFacet2.facetB);
			
			int eIdA = iFacet2.edge->edgeNum[0];
			int eIdB = iFacet2.edge->edgeNum[1];

			int order[3] = {0,1,2};
			if(eIdA==0) {
				order[0] = 1;
				order[1] = 2;
				order[2] = 0;
			}
			else if(eIdA==1) {
				order[0] = 2;
				order[1] = 0;
				order[2] = 1;
			}
			
			oFacet2.m_vertIds[0] = iFacet2.facetA->v[order[0]]->localId;
			oFacet2.m_vertIds[1] = iFacet2.facetA->v[order[1]]->localId;
			oFacet2.m_vertIds[2] = iFacet2.facetA->v[order[2]]->localId;
			oFacet2.m_vertIds[3] = iFacet2.facetB->v[(eIdB+2)%3]->localId;
			oFacet2.m_edgeInfo =
				 (iFacet2.facetA->e[order[0]]->angleType & 0x03) |
				((iFacet2.facetA->e[order[1]]->angleType & 0x03) << 2) |
				((iFacet2.facetA->e[order[2]]->angleType & 0x03) << 4) |
				((iFacet2.facetB->e[(eIdB+1)%3]->angleType & 0x03) << 6) |
				((iFacet2.facetB->e[(eIdB+2)%3]->angleType & 0x03) << 8);
			oFacet2.m_facetInfo = offsetId | 0x8000;
			offsetId+=2;
			oFacet2.m_userData[0] = iFacet2.facetA->userData;
			oFacet2.m_userData[1] = iFacet2.facetB->userData;
		}
		else {
			oFacet2.m_vertIds[0] = iFacet2.facetA->v[0]->localId;
			oFacet2.m_vertIds[1] = iFacet2.facetA->v[1]->localId;
			oFacet2.m_vertIds[2] = iFacet2.facetA->v[2]->localId;
			oFacet2.m_vertIds[3] = oFacet2.m_vertIds[2]; // dummy
			oFacet2.m_edgeInfo =
				 (iFacet2.facetA->e[0]->angleType & 0x03) |
				((iFacet2.facetA->e[1]->angleType & 0x03) << 2) |
				((iFacet2.facetA->e[2]->angleType & 0x03) << 4);
			oFacet2.m_facetInfo = offsetId++;
			oFacet2.m_userData[0] = iFacet2.facetA->userData;
			oFacet2.m_userData[1] = 0;
		}
	}
	
	if(numFacetsX2 == 1) {
		PfxFacetBvhNode encnode;
		encnode.flag = 0x09;
		bvhNodeArray[island->m_bvhNodes] = encnode;
	}
	else {
		// BVHの構築
		PfxMcBvhNode *bvhNodes = (PfxMcBvhNode*)SCE_PFX_UTIL_ALLOC(16,sizeof(PfxMcBvhNode)*(island->m_numFacets * 2 - 1));
		if(!bvhNodes) return SCE_PFX_ERR_OUT_OF_BUFFER;
		
		PfxArray<PfxUInt32> bvhNodeIndices(island->m_numFacets * 2 - 1);
		if(!bvhNodeIndices.ptr()) return SCE_PFX_ERR_OUT_OF_BUFFER;
	
		if (((param.flag & SCE_PFX_MESH_FLAG_OPTIMIZE_BVH_STRUCTURE) == SCE_PFX_MESH_FLAG_OPTIMIZE_BVH_STRUCTURE) ||
			((param.flag & SCE_PFX_MESH_FLAG_FAST_BVH_CONSTRUCTION) == SCE_PFX_MESH_FLAG_FAST_BVH_CONSTRUCTION)) {
			PfxMcSortedFacetsList sortedFacetsList;
			if (!sortedFacetsList.initialize(island->m_numFacets))
				return SCE_PFX_ERR_OUT_OF_BUFFER;

			PfxArray<PfxVector3> facetAabbMin(island->m_numFacets), facetAabbMax(island->m_numFacets);
			if (!facetAabbMin.ptr() || !facetAabbMax.ptr())	return SCE_PFX_ERR_OUT_OF_BUFFER;

			// AABB配列を作成する
			

			{
				
				PfxVector3 centerMin(SCE_PFX_FLT_MAX), centerMax(-SCE_PFX_FLT_MAX);
				for (PfxUInt32 f = 0; f < island->m_numFacets; ++f) {
					PfxMcFacet2 &facet2 = facet2List[f];
					if (facet2.edge) {
						facetAabbMin[f] = minPerElem(facet2.facetA->aabbMin, facet2.facetB->aabbMin);
						facetAabbMax[f] = maxPerElem(facet2.facetA->aabbMax, facet2.facetB->aabbMax);
					}
					else {
						facetAabbMin[f] = facet2.facetA->aabbMin;
						facetAabbMax[f] = facet2.facetA->aabbMax;
					}
					PfxVector3 center = (facetAabbMin[f] + facetAabbMax[f]) * 0.5f;
					centerMin = minPerElem(centerMin, center);
					centerMax = maxPerElem(centerMax, center);
				}
				PfxVector3 centerSize = centerMax - centerMin;
				PfxFloat maxCenterSize = maxElem(centerSize);

				PfxVector3 invSize(PfxFloat(0x3ff) / maxCenterSize);
				for(PfxUInt32 f = 0 ; f < island->m_numFacets;++f) {
					PfxVector3 center = (facetAabbMin[f] + facetAabbMax[f]) * 0.5f;
					PfxVector3 normalizedCenter = mulPerElem(center - centerMin, invSize);
					PfxUInt32 mortonCode = toMortonCode(normalizedCenter);
					sortedFacetsList.set(f, mortonCode, &(facet2List[f]), f);
				}
			}

			sortedFacetsList.sort();

			// AABB配列を作成する
			PfxUInt32 numBvhNodes = island->m_numFacets;
			for (PfxInt32 f = island->m_numFacets - 1; f >= 0; --f) {
				bvhNodes[f].aabbMin = facetAabbMin[f];
				bvhNodes[f].aabbMax = facetAabbMax[f];
				bvhNodes[f].nodeId = f;
				bvhNodes[f].left = NULL;
				bvhNodes[f].right = NULL;
				bvhNodes[f].parent = NULL;
				bvhNodeIndices.push(f);
			}

			// 最下層から順に最もAABBサイズの小さくなる組み合わせを検出する ※O(n^3)
			while (bvhNodeIndices.size() > 1) {
				// AABBが最小となるペアを検出
				PfxUInt32 i = 0, j = 0;
				{
					PfxUInt32 pMin = 0, qMin = 0;
					PfxFloat volumeMin = SCE_PFX_FLT_MAX;
					for (PfxUInt32 p = 0; p<bvhNodeIndices.size(); p++) {
						for (PfxUInt32 q = p + 1; q<bvhNodeIndices.size() && q <= p + 4; q++) {
							PfxMcBvhNode &nodeA = bvhNodes[bvhNodeIndices[p]];
							PfxMcBvhNode &nodeB = bvhNodes[bvhNodeIndices[q]];
							PfxVector3 aabbMin = minPerElem(nodeA.aabbMin, nodeB.aabbMin);
							PfxVector3 aabbMax = maxPerElem(nodeA.aabbMax, nodeB.aabbMax);
							PfxVector3 extent = aabbMax - aabbMin;
							PfxFloat volume = extent[0] + extent[1] + extent[2];
							if (volume < volumeMin) {
								volumeMin = volume;
								pMin = p;
								qMin = q;
							}
						}
					}
					i = pMin;
					j = qMin;

					SCE_PFX_ASSERT(i != j);
				}

				// 新しいノードを作成
				PfxUInt32 newId = numBvhNodes++;
				PfxMcBvhNode &newNode = bvhNodes[newId];
				PfxMcBvhNode &leftNode = bvhNodes[bvhNodeIndices[i]];
				PfxMcBvhNode &rightNode = bvhNodes[bvhNodeIndices[j]];
				newNode.aabbMin = minPerElem(leftNode.aabbMin, rightNode.aabbMin);
				newNode.aabbMax = maxPerElem(leftNode.aabbMax, rightNode.aabbMax);
				newNode.nodeId = newId;
				newNode.left = &leftNode;
				newNode.right = &rightNode;
				newNode.parent = NULL;
				leftNode.parent = &newNode;
				rightNode.parent = &newNode;

				// ペアのノードを除去
				bvhNodeIndices[i] = newId;
				for (PfxUInt32 k = j + 1; k < bvhNodeIndices.size(); ++k) {
					bvhNodeIndices[k - 1] = bvhNodeIndices[k];
				}
				bvhNodeIndices.remove(bvhNodeIndices.size() - 1);
			}
		}
		else
		{
			// AABB配列を作成する
			PfxUInt32 numBvhNodes = island->m_numFacets;
			for (PfxUInt32 f = 0; f < island->m_numFacets; f++) {
				PfxMcFacet2 &facet2 = facet2List[f];
				if (facet2.edge) {
					bvhNodes[f].aabbMin = minPerElem(facet2.facetA->aabbMin, facet2.facetB->aabbMin);
					bvhNodes[f].aabbMax = maxPerElem(facet2.facetA->aabbMax, facet2.facetB->aabbMax);
				}
				else {
					bvhNodes[f].aabbMin = facet2.facetA->aabbMin;
					bvhNodes[f].aabbMax = facet2.facetA->aabbMax;
				}
				bvhNodes[f].nodeId = f;
				bvhNodes[f].left = NULL;
				bvhNodes[f].right = NULL;
				bvhNodes[f].parent = NULL;
				bvhNodeIndices.push(f);
			}

			// 最下層から順に最もAABBサイズの小さくなる組み合わせを検出する ※O(n^3)
			while (bvhNodeIndices.size() > 1) {
				// AABBが最小となるペアを検出
				PfxUInt32 i = 0, j = 0;
				{
					PfxUInt32 pMin = 0, qMin = 0;
					PfxFloat volumeMin = SCE_PFX_FLT_MAX;
					for (unsigned int p = 0; p < bvhNodeIndices.size(); p++) {
						for (unsigned int q = p + 1; q < bvhNodeIndices.size(); q++) {
							PfxMcBvhNode &nodeA = bvhNodes[bvhNodeIndices[p]];
							PfxMcBvhNode &nodeB = bvhNodes[bvhNodeIndices[q]];
							PfxVector3 aabbMin = minPerElem(nodeA.aabbMin, nodeB.aabbMin);
							PfxVector3 aabbMax = maxPerElem(nodeA.aabbMax, nodeB.aabbMax);
							PfxVector3 extent = aabbMax - aabbMin;
							PfxFloat volume = extent[0] + extent[1] + extent[2];
							if (volume < volumeMin) {
								volumeMin = volume;
								pMin = p;
								qMin = q;
							}
						}
					}
					i = pMin;
					j = qMin;

					SCE_PFX_ASSERT(i != j);
				}

				// 新しいノードを作成
				PfxUInt32 newId = numBvhNodes++;
				PfxMcBvhNode &newNode = bvhNodes[newId];
				PfxMcBvhNode &leftNode = bvhNodes[bvhNodeIndices[i]];
				PfxMcBvhNode &rightNode = bvhNodes[bvhNodeIndices[j]];
				newNode.aabbMin = minPerElem(leftNode.aabbMin, rightNode.aabbMin);
				newNode.aabbMax = maxPerElem(leftNode.aabbMax, rightNode.aabbMax);
				newNode.nodeId = newId;
				newNode.left = &leftNode;
				newNode.right = &rightNode;
				newNode.parent = NULL;
				leftNode.parent = &newNode;
				rightNode.parent = &newNode;

				// ペアのノードを除去
				bvhNodeIndices.remove(SCE_PFX_MAX(i, j));
				bvhNodeIndices.remove(SCE_PFX_MIN(i, j));

				// 新しいノードを登録
				bvhNodeIndices.push(newId);
			}
		}

		//PfxMcBvhNode *root = &bvhNodes[bvhNodeIndices[0]];
		//printBvh(root);

		//SCE_PFX_PRINTF("numFacets %d numBvhNodes %d\n",island->m_numFacets,numBvhNodes);
		//SCE_PFX_PRINTF("m_bvhRootId %u/%u\n",bvhNodeIndices[0],island->m_numFacets*2-2);

		//island->m_bvhRootId = bvhNodeIndices[0];

		// BVHをエンコード
		PfxMcBvhNode *root = &bvhNodes[bvhNodeIndices[0]];

		// encode nodes
		PfxStaticStack<PfxMcBvhNode*> nodeStack;
		nodeStack.push(root);
		
		PfxUInt32 numEncNodes = 0;
		
		while(!nodeStack.empty()) {
			PfxMcBvhNode *node = nodeStack.top();
			nodeStack.pop();
			
			node->info = numEncNodes;
			PfxFacetBvhNode encnode;
			
			if(node->parent) {
				PfxVector3 aabbMinP = node->parent->aabbMin;
				PfxVector3 aabbMaxP = node->parent->aabbMax;
				PfxVector3 lenP = aabbMaxP - aabbMinP;
				PfxVector3 aabbMinN = node->aabbMin;
				PfxVector3 aabbMaxN = node->aabbMax;
				PfxVector3 quantizedMin = floorVec3(255.0f * clampVec3(divPerElem(aabbMinN - aabbMinP,lenP),PfxVector3::zero(),PfxVector3(1.0f)));
				PfxVector3 quantizedMax = floorVec3(255.0f * clampVec3(divPerElem(aabbMaxP - aabbMaxN,lenP),PfxVector3::zero(),PfxVector3(1.0f)));
				encnode.aabb[0] = (PfxUInt8)quantizedMin[0];
				encnode.aabb[1] = (PfxUInt8)quantizedMin[1];
				encnode.aabb[2] = (PfxUInt8)quantizedMin[2];
				encnode.aabb[3] = (PfxUInt8)quantizedMax[0];
				encnode.aabb[4] = (PfxUInt8)quantizedMax[1];
				encnode.aabb[5] = (PfxUInt8)quantizedMax[2];

				// AABBの縮退を防ぐ
				if(encnode.aabb[0] > 0) encnode.aabb[0]--;
				if(encnode.aabb[1] > 0) encnode.aabb[1]--;
				if(encnode.aabb[2] > 0) encnode.aabb[2]--;
				if(encnode.aabb[3] > 0) encnode.aabb[3]--;
				if(encnode.aabb[4] > 0) encnode.aabb[4]--;
				if(encnode.aabb[5] > 0) encnode.aabb[5]--;

				// デコードした値で書き換えておく
				PfxVector3 decMin = aabbMinP + mulPerElem((aabbMaxP-aabbMinP),PfxVector3(encnode.aabb[0],encnode.aabb[1],encnode.aabb[2]) / 255.0f);
				PfxVector3 decMax = aabbMaxP - mulPerElem((aabbMaxP-aabbMinP),PfxVector3(encnode.aabb[3],encnode.aabb[4],encnode.aabb[5]) / 255.0f);

				SCE_PFX_ASSERT(decMin[0] <= aabbMinN[0]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(decMin[1] <= aabbMinN[1]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(decMin[2] <= aabbMinN[2]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(aabbMaxN[0] <= decMax[0]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(aabbMaxN[1] <= decMax[1]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(aabbMaxN[2] <= decMax[2]+SCE_PFX_MESH_CREATE_EPSILON);

				node->aabbMin = decMin;
				node->aabbMax = decMax;
				
				// Connect to the parent
				SCE_PFX_ASSERT((PfxInt32)node->parent->info < (PfxInt32)island->m_numFacets-1);
				PfxFacetBvhNode *encparent = bvhNodeArray + (island->m_bvhNodes + node->parent->info);
				if(node->parent->left == node) {
					encparent->left = numEncNodes;
				}
				else {
					encparent->right = numEncNodes;
				}
			}
			
			if(node->left->isLeaf()) {
				encnode.flag |= 0x01;
				encnode.left = node->left->nodeId;
			}
			else {
				if(nodeStack.isFull()) return SCE_PFX_ERR_OUT_OF_BUFFER;
				nodeStack.push(node->left);
			}
			
			if(node->right->isLeaf()) {
				encnode.flag |= 0x04;
				encnode.right = node->right->nodeId;
			}
			else {
				if(nodeStack.isFull()) return SCE_PFX_ERR_OUT_OF_BUFFER;
				nodeStack.push(node->right);
			}
			
			bvhNodeArray[island->m_bvhNodes + numEncNodes++] = encnode;
		}

		SCE_PFX_UTIL_FREE(bvhNodes);
	}

	return SCE_PFX_OK;
}

///////////////////////////////////////////////////////////////////////////////
// ラージメッシュ

PfxInt32 pfxCreateLargeTriMesh(PfxLargeTriMesh &largeMesh,const PfxCreateLargeTriMeshParam &param)
{
	SCE_PFX_ASSERT(sizeof(PfxLargeTriMeshImpl) <= sizeof(PfxLargeTriMesh));

	PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;
	
	// zero clear
	memset(largeMeshImpl,0,sizeof(PfxLargeTriMeshImpl));

	//	check flag
	if (checkFlagsForLargeTriMesh(param)!=SCE_PFX_OK)
	{
		return SCE_PFX_ERR_INVALID_FLAG;
	}
	// Check input
	if(param.numVerts == 0 || param.numTriangles == 0 || !param.verts || !param.triangles)
	{
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	if (param.numVerts>(SCE_PFX_MAX_LARGETRIMESH_ISLANDS*SCE_PFX_NUMMESHVERTICES) || 
		param.numTriangles>(SCE_PFX_MAX_LARGETRIMESH_ISLANDS*SCE_PFX_NUMMESHFACETS))
	{
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	if(param.numFacetsLimit == 0 || param.numFacetsLimit > SCE_PFX_NUMMESHFACETS)
	{
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	PfxUInt32 checkparam = checkLargeMeshParams(param);
	if (checkparam!=SCE_PFX_OK)
	{
		return checkparam;
	}
	
	PfxMeshObject meshObj;
	
	SCE_PFX_PUSH_PERF("buildMesh");
	// 中間メッシュデータの構築
	PfxUInt32 ret = pfxMeshObjectBuild(meshObj,param);
	SCE_PFX_POP_PERF();
	if(ret != SCE_PFX_OK) return ret;
	
	SCE_PFX_ASSERT(meshObj.numBvhNodes > 0);
	
	// ラージメッシュのオフセットとサイズを設定
	largeMeshImpl->setOffset(( meshObj.totalAabbMax + meshObj.totalAabbMin ) * 0.5f);
	largeMeshImpl->setHalf(( meshObj.totalAabbMax - meshObj.totalAabbMin ) * 0.5f);
	largeMeshImpl->m_length = length(largeMeshImpl->getHalf());
	
	largeMeshImpl->m_defaultThickness = param.defaultThickness;
	
	// Check Islands
	//for(PfxInt32 i=0;i<islands.numIslands;i++) {
	//	SCE_PFX_PRINTF("island %d\n",i);
	//	for(PfxInt32 f=0;f<islands.facetsInIsland[i].size();f++) {
	//		PfxMcFacet *facet = islands.facetsInIsland[i][f];
	//		SCE_PFX_PRINTF("   %d %d %d\n",facet->v[0]->i,facet->v[1]->i,facet->v[2]->i);
	//	}
	//}

	//	building bv-tree or aabb array
	if((param.flag & (SCE_PFX_MESH_FLAG_STRUCTURE_STANDARD_BVH | SCE_PFX_MESH_FLAG_STRUCTURE_COMPRESSED_BVH)) && meshObj.numIslands == 1) {
		largeMeshImpl->m_bvhNodes = (PfxIslandBvhNode*)SCE_PFX_UTIL_ALLOC(16,sizeof(PfxIslandBvhNode));
		largeMeshImpl->m_numBvhNodes = 1;
		if(!largeMeshImpl->m_bvhNodes) return SCE_PFX_ERR_OUT_OF_BUFFER;
		
		PfxIslandBvhNode encnode;
		encnode.flag |= 0x09;
		largeMeshImpl->m_bvhNodes[0] = encnode;
	}
	else if(param.flag & (SCE_PFX_MESH_FLAG_STRUCTURE_STANDARD_BVH | SCE_PFX_MESH_FLAG_STRUCTURE_COMPRESSED_BVH)) {
		// BVHをエンコード
		SCE_PFX_ASSERT(meshObj.numIslands > 1);
		largeMeshImpl->m_bvhNodes = (PfxIslandBvhNode*)SCE_PFX_UTIL_ALLOC(16,sizeof(PfxIslandBvhNode)*(meshObj.numIslands-1));
		largeMeshImpl->m_numBvhNodes = meshObj.numIslands-1;

		if(!largeMeshImpl->m_bvhNodes) return SCE_PFX_ERR_OUT_OF_BUFFER;

		PfxMcIslandBvhNode *root = &meshObj.bvh[0];

		// encode nodes
		PfxStaticStack<PfxMcIslandBvhNode*> nodeStack;
		nodeStack.push(root);
		
		PfxUInt32 numEncNodes = 0;
		
		while(!nodeStack.empty()) {
			PfxMcIslandBvhNode *node = nodeStack.top();
			nodeStack.pop();
			
			node->memo = numEncNodes;
			PfxIslandBvhNode encnode;
			
			if(node->parent) {
				PfxVector3 aabbMinP = node->parent->aabbMin;
				PfxVector3 aabbMaxP = node->parent->aabbMax;
				PfxVector3 lenP = aabbMaxP - aabbMinP;
				PfxVector3 aabbMinN = node->aabbMin;
				PfxVector3 aabbMaxN = node->aabbMax;
				PfxVector3 quantizedMin = floorVec3(255.0f * clampVec3(divPerElem(aabbMinN - aabbMinP,lenP),PfxVector3::zero(),PfxVector3(1.0f)));
				PfxVector3 quantizedMax = floorVec3(255.0f * clampVec3(divPerElem(aabbMaxP - aabbMaxN,lenP),PfxVector3::zero(),PfxVector3(1.0f)));
				encnode.aabb[0] = (PfxUInt8)quantizedMin[0];
				encnode.aabb[1] = (PfxUInt8)quantizedMin[1];
				encnode.aabb[2] = (PfxUInt8)quantizedMin[2];
				encnode.aabb[3] = (PfxUInt8)quantizedMax[0];
				encnode.aabb[4] = (PfxUInt8)quantizedMax[1];
				encnode.aabb[5] = (PfxUInt8)quantizedMax[2];

				// デコードした値で書き換えておく
				PfxVector3 decMin = aabbMinP + mulPerElem((aabbMaxP-aabbMinP),quantizedMin / 255.0f);
				PfxVector3 decMax = aabbMaxP - mulPerElem((aabbMaxP-aabbMinP),quantizedMax / 255.0f);

				SCE_PFX_ASSERT(decMin[0] <= node->aabbMin[0]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(decMin[1] <= node->aabbMin[1]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(decMin[2] <= node->aabbMin[2]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(node->aabbMax[0] <= decMax[0]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(node->aabbMax[1] <= decMax[1]+SCE_PFX_MESH_CREATE_EPSILON);
				SCE_PFX_ASSERT(node->aabbMax[2] <= decMax[2]+SCE_PFX_MESH_CREATE_EPSILON);

				node->aabbMin = decMin;
				node->aabbMax = decMax;
				
				// Connect to the parent
				SCE_PFX_ASSERT(node->parent->memo < largeMeshImpl->m_numBvhNodes);
				PfxIslandBvhNode *encparent = largeMeshImpl->m_bvhNodes + node->parent->memo;
				if(node->parent->left == node) {
					encparent->left = numEncNodes;
				}
				else {
					encparent->right = numEncNodes;
				}
			}
			
			if(node->left->isLeaf()) {
				encnode.flag |= 0x01;
				encnode.left = node->left->islandId;
			}
			else {
				if(nodeStack.isFull()) return SCE_PFX_ERR_OUT_OF_BUFFER;
				nodeStack.push(node->left);
			}
			
			if(node->right->isLeaf()) {
				encnode.flag |= 0x04;
				encnode.right = node->right->islandId;
			}
			else {
				if(nodeStack.isFull()) return SCE_PFX_ERR_OUT_OF_BUFFER;
				nodeStack.push(node->right);
			}
			
			largeMeshImpl->m_bvhNodes[numEncNodes++] = encnode;
		}
	}
	else {
		largeMeshImpl->m_aabbList = (PfxAabb16*)SCE_PFX_UTIL_ALLOC(16,sizeof(PfxAabb16)*meshObj.numIslands);
		
		if(!largeMeshImpl->m_aabbList) return SCE_PFX_ERR_OUT_OF_BUFFER;
		
		// AABB配列を作成する
		PfxUInt32 numIslands = meshObj.numIslands;
		for(PfxUInt32 i=0;i<numIslands;i++) {
			PfxVector3 aabbMin = meshObj.islands[i]->aabbMin;
			PfxVector3 aabbMax = meshObj.islands[i]->aabbMax;
			
			PfxVecInt3 aabbMinL,aabbMaxL;
			largeMeshImpl->getLocalPosition(aabbMin,aabbMax,aabbMinL,aabbMaxL);
			
			pfxSetXMin(largeMeshImpl->m_aabbList[i],aabbMinL.getX());
			pfxSetXMax(largeMeshImpl->m_aabbList[i],aabbMaxL.getX());
			pfxSetYMin(largeMeshImpl->m_aabbList[i],aabbMinL.getY());
			pfxSetYMax(largeMeshImpl->m_aabbList[i],aabbMaxL.getY());
			pfxSetZMin(largeMeshImpl->m_aabbList[i],aabbMinL.getZ());
			pfxSetZMax(largeMeshImpl->m_aabbList[i],aabbMaxL.getZ());
		}
	}
	
	SCE_PFX_PUSH_PERF("createLargeMesh");

	// ラージメッシュの作成
	if(meshObj.numIslands <= SCE_PFX_MAX_LARGETRIMESH_ISLANDS) {
		// ラージメッシュのタイプを判定する
		largeMeshImpl->m_type = 0;
		
		if (param.flag & SCE_PFX_MESH_FLAG_STRUCTURE_COMPRESSED_BVH) {
			largeMeshImpl->m_type = 0x07;
			largeMeshImpl->m_islands	= SCE_PFX_UTIL_ALLOC(16,sizeof(PfxCompressedTriMesh)*meshObj.numIslands);
			if(!largeMeshImpl->m_islands) return SCE_PFX_ERR_OUT_OF_BUFFER;
			memset(largeMeshImpl->m_islands,0,sizeof(PfxCompressedTriMesh)*meshObj.numIslands);
			largeMeshImpl->m_numIslands = meshObj.numIslands;
		}
		else if (param.flag & SCE_PFX_MESH_FLAG_STRUCTURE_STANDARD_BVH) {
			largeMeshImpl->m_type = 0x03;
			largeMeshImpl->m_islands	= SCE_PFX_UTIL_ALLOC(16,sizeof(PfxQuantizedTriMeshBvh)*meshObj.numIslands);
			if(!largeMeshImpl->m_islands) return SCE_PFX_ERR_OUT_OF_BUFFER;
			memset(largeMeshImpl->m_islands,0,sizeof(PfxQuantizedTriMeshBvh)*meshObj.numIslands);
			largeMeshImpl->m_numIslands = meshObj.numIslands;
		}
		else {
			largeMeshImpl->m_islands = SCE_PFX_UTIL_ALLOC(16,sizeof(PfxExpandedTriMesh)*meshObj.numIslands);
			if(!largeMeshImpl->m_islands) return SCE_PFX_ERR_OUT_OF_BUFFER;
			memset(largeMeshImpl->m_islands,0,sizeof(PfxExpandedTriMesh)*meshObj.numIslands);
			largeMeshImpl->m_numIslands = meshObj.numIslands;
		}
		
		SCE_PFX_PUSH_PERF("countIslands");

		PfxUInt32 facetBuffBytes=0,edgeBuffBytes=0,vertexBuffBytes=0,bvhNodeBuffBytes=0;
		for(PfxUInt32 i=0;i<meshObj.numIslands;i++) {
			// 頂点、エッジ数をアイランドにセット
			PfxUInt32 numFacets = meshObj.islands[i]->numFacets;
			PfxUInt32 numFacetsX2 = 0;
			PfxUInt32 numEdges=0,numVerts=0;
			PfxInt32 ret = countIsland(meshObj.islands[i]->facetList,meshObj.islands[i]->numFacets,numEdges,numVerts,numFacetsX2,meshObj.vertList.size());
			if(ret != SCE_PFX_OK) return ret;
			
			//SCE_PFX_PRINTF("island %u numFacets %u numFacetsX2 %u numEdges %u numVerts %u\n",i,numFacets,numFacetsX2,numEdges,numVerts);
			
			if(param.flag & SCE_PFX_MESH_FLAG_STRUCTURE_COMPRESSED_BVH) {
				PfxCompressedTriMesh *island = ((PfxCompressedTriMesh*)largeMeshImpl->m_islands)+i;
				facetBuffBytes += sizeof(PfxCompressedFacet2)*numFacetsX2; // todo
				bvhNodeBuffBytes += sizeof(PfxFacetBvhNode)*SCE_PFX_MAX(1,numFacetsX2-1); // todo
				island->m_numFacets = numFacetsX2;
				island->m_numEdges = numEdges;
				island->m_numVerts = numVerts;
			}
			else if (param.flag & SCE_PFX_MESH_FLAG_STRUCTURE_STANDARD_BVH) {
				PfxQuantizedTriMeshBvh *island = ((PfxQuantizedTriMeshBvh*)largeMeshImpl->m_islands)+i;
				facetBuffBytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxQuantizedFacetBvh)*numFacets);
				bvhNodeBuffBytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxFacetBvhNode)*SCE_PFX_MAX(1,numFacets-1));
				island->m_numFacets = numFacets;
				island->m_numEdges = numEdges;
				island->m_numVerts = numVerts;
			}
			else {
				PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMeshImpl->m_islands)+i;
				facetBuffBytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxExpandedFacet)*numFacets);
				island->m_numFacets = numFacets;
				island->m_numEdges = numEdges;
				island->m_numVerts = numVerts;
			}
			edgeBuffBytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxEdge)*numEdges);
			
			if (param.flag & SCE_PFX_MESH_FLAG_STRUCTURE_COMPRESSED_BVH) {
				vertexBuffBytes += sizeof(PfxQuantize3)*numVerts;
			}
			else if (param.flag & SCE_PFX_MESH_FLAG_STRUCTURE_STANDARD_BVH) {
				vertexBuffBytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxQuantize3)*numVerts);
			}
			else {
				vertexBuffBytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxFloat3)*numVerts);
			}
		}

		SCE_PFX_POP_PERF();
		
		// 共有バッファを確保
		largeMeshImpl->m_facetBuffer = SCE_PFX_UTIL_ALLOC(16,facetBuffBytes);
		largeMeshImpl->m_edgeBuffer = SCE_PFX_UTIL_ALLOC(16,edgeBuffBytes);
		largeMeshImpl->m_vertexBuffer = SCE_PFX_UTIL_ALLOC(16,vertexBuffBytes);
		largeMeshImpl->m_bvhNodeBuffer = SCE_PFX_UTIL_ALLOC(16,bvhNodeBuffBytes);
		largeMeshImpl->m_facetBuffBytes	= facetBuffBytes;
		largeMeshImpl->m_edgeBuffBytes	= edgeBuffBytes;
		largeMeshImpl->m_vertexBuffBytes = vertexBuffBytes;
		largeMeshImpl->m_bvhNodeBuffBytes = bvhNodeBuffBytes;
		
		if(!(largeMeshImpl->m_facetBuffer&&largeMeshImpl->m_edgeBuffer&&largeMeshImpl->m_vertexBuffer&&largeMeshImpl->m_bvhNodeBuffer)) return SCE_PFX_ERR_OUT_OF_BUFFER;
		
		// 共有バッファを各アイランドに割り当てる
		if (param.flag & SCE_PFX_MESH_FLAG_STRUCTURE_COMPRESSED_BVH) {
			PfxUInt32 idxFacetBuffer = 0;
			PfxUInt32 idxVertexBuffer = 0;
			PfxUInt32 idxBvhNodeBuffer = 0;
			
			for(PfxUInt32 i=0;i<meshObj.numIslands;i++) {
				PfxCompressedTriMesh *island = ((PfxCompressedTriMesh*)largeMeshImpl->m_islands)+i;
				island->m_facets = idxFacetBuffer;
				island->m_verts = idxVertexBuffer;
				island->m_bvhNodes = idxBvhNodeBuffer;
				
				idxFacetBuffer += island->m_numFacets;
				idxVertexBuffer += island->m_numVerts;
				idxBvhNodeBuffer += SCE_PFX_MAX(1,island->m_numFacets-1);
				
				SCE_PFX_ASSERT(idxFacetBuffer * sizeof(PfxCompressedFacet2) <= facetBuffBytes);
				SCE_PFX_ASSERT(idxVertexBuffer * sizeof(PfxQuantize3) <= vertexBuffBytes);
				SCE_PFX_ASSERT(idxBvhNodeBuffer * sizeof(PfxFacetBvhNode) <= bvhNodeBuffBytes);
			}
		}
		else if (param.flag & SCE_PFX_MESH_FLAG_STRUCTURE_STANDARD_BVH) {
			PfxUInt8 *ptrFacetBuffer = (PfxUInt8*)largeMeshImpl->m_facetBuffer;
			PfxUInt8 *ptrEdgeBuffer = (PfxUInt8*)largeMeshImpl->m_edgeBuffer;
			PfxUInt8 *ptrVertexBuffer = (PfxUInt8*)largeMeshImpl->m_vertexBuffer;
			PfxUInt8 *ptrBvhNodeBuffer = (PfxUInt8*)largeMeshImpl->m_bvhNodeBuffer;
				
			for(PfxUInt32 i=0;i<meshObj.numIslands;i++) {
				PfxQuantizedTriMeshBvh *island = ((PfxQuantizedTriMeshBvh*)largeMeshImpl->m_islands)+i;
				island->m_facets = (PfxQuantizedFacetBvh*)ptrFacetBuffer;
				island->m_edges = (PfxEdge*)ptrEdgeBuffer;
				island->m_verts = (PfxQuantize3*)ptrVertexBuffer;
				island->m_bvhNodes = (PfxFacetBvhNode*)ptrBvhNodeBuffer;
						
				ptrFacetBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxQuantizedFacetBvh)*island->m_numFacets);
				ptrEdgeBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxEdge)*island->m_numEdges);
				ptrVertexBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxQuantize3)*island->m_numVerts);
				ptrBvhNodeBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxFacetBvhNode)*SCE_PFX_MAX(1,island->m_numFacets-1));
						
				SCE_PFX_ASSERT(((uintptr_t)ptrFacetBuffer - (uintptr_t)largeMeshImpl->m_facetBuffer) <= facetBuffBytes);
				SCE_PFX_ASSERT(((uintptr_t)ptrEdgeBuffer - (uintptr_t)largeMeshImpl->m_edgeBuffer) <= edgeBuffBytes);
				SCE_PFX_ASSERT(((uintptr_t)ptrVertexBuffer - (uintptr_t)largeMeshImpl->m_vertexBuffer) <= vertexBuffBytes);
				SCE_PFX_ASSERT(((uintptr_t)ptrBvhNodeBuffer - (uintptr_t)largeMeshImpl->m_bvhNodeBuffer) <= bvhNodeBuffBytes);
			}
		}
		else {
			PfxUInt8 *ptrFacetBuffer = (PfxUInt8*)largeMeshImpl->m_facetBuffer;
			PfxUInt8 *ptrEdgeBuffer = (PfxUInt8*)largeMeshImpl->m_edgeBuffer;
			PfxUInt8 *ptrVertexBuffer = (PfxUInt8*)largeMeshImpl->m_vertexBuffer;
				
			for(PfxUInt32 i=0;i<meshObj.numIslands;i++) {
				PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMeshImpl->m_islands)+i;
				island->m_facets = (PfxExpandedFacet*)ptrFacetBuffer;
				island->m_edges = (PfxEdge*)ptrEdgeBuffer;
				island->m_verts = (PfxFloat3*)ptrVertexBuffer;
						
				ptrFacetBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxExpandedFacet)*island->m_numFacets);
				ptrEdgeBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxEdge)*island->m_numEdges);
				ptrVertexBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxFloat3)*island->m_numVerts);
						
				SCE_PFX_ASSERT(((uintptr_t)ptrFacetBuffer - (uintptr_t)largeMeshImpl->m_facetBuffer) <= facetBuffBytes);
				SCE_PFX_ASSERT(((uintptr_t)ptrEdgeBuffer - (uintptr_t)largeMeshImpl->m_edgeBuffer) <= edgeBuffBytes);
				SCE_PFX_ASSERT(((uintptr_t)ptrVertexBuffer - (uintptr_t)largeMeshImpl->m_vertexBuffer) <= vertexBuffBytes);
			}
		}
		
		// アイランドを作成
		PfxUInt32 maxFacets=0,maxVerts=0,maxEdges=0;
		
		SCE_PFX_PUSH_PERF("submitIslands");

		if (param.flag & SCE_PFX_MESH_FLAG_STRUCTURE_COMPRESSED_BVH) {
			for(PfxUInt32 i=0;i<meshObj.numIslands;i++) {
				PfxCompressedTriMesh *island = ((PfxCompressedTriMesh*)largeMeshImpl->m_islands)+i;
				pfxStoreVector3(meshObj.islands[i]->aabbMin,island->m_aabbMin);
				pfxStoreVector3(meshObj.islands[i]->aabbMax,island->m_aabbMax);
				
				PfxInt32 ret = submitIsland(param,
					island,
					(PfxQuantize3*)largeMeshImpl->m_vertexBuffer,
					(PfxCompressedFacet2*)largeMeshImpl->m_facetBuffer,
					(PfxFacetBvhNode*)largeMeshImpl->m_bvhNodeBuffer,
					meshObj.islands[i]->facetList,meshObj.islands[i]->numFacets,island->m_numFacets,
					largeMeshImpl,
					meshObj.vertList.size());
				if(ret != SCE_PFX_OK) {
					pfxReleaseLargeTriMesh(largeMesh);
					return ret;
				}
				
				maxFacets = SCE_PFX_MAX(maxFacets,island->m_numFacets);
				maxVerts = SCE_PFX_MAX(maxVerts,island->m_numVerts);
				maxEdges = SCE_PFX_MAX(maxEdges,island->m_numEdges);
			}
		}
		else if (param.flag & SCE_PFX_MESH_FLAG_STRUCTURE_STANDARD_BVH) {
			for(PfxUInt32 i=0;i<meshObj.numIslands;i++) {
				PfxQuantizedTriMeshBvh *island = ((PfxQuantizedTriMeshBvh*)largeMeshImpl->m_islands)+i;
				pfxStoreVector3(meshObj.islands[i]->aabbMin,island->m_aabbMin);
				pfxStoreVector3(meshObj.islands[i]->aabbMax,island->m_aabbMax);
				PfxInt32 ret = submitIsland(param,island,meshObj.islands[i]->facetList,meshObj.islands[i]->numFacets,largeMeshImpl,meshObj.vertList.size());
				if(ret != SCE_PFX_OK) {
					pfxReleaseLargeTriMesh(largeMesh);
					return ret;
				}
						
				maxFacets = SCE_PFX_MAX(maxFacets,island->m_numFacets);
				maxVerts = SCE_PFX_MAX(maxVerts,island->m_numVerts);
				maxEdges = SCE_PFX_MAX(maxEdges,island->m_numEdges);
			}
		}
		else {
			for(PfxUInt32 i=0;i<meshObj.numIslands;i++) {
				PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMeshImpl->m_islands)+i;
				PfxInt32 ret = submitIsland(param,island,meshObj.islands[i]->facetList,meshObj.islands[i]->numFacets,largeMeshImpl,meshObj.vertList.size());
				if(ret != SCE_PFX_OK) {
					pfxReleaseLargeTriMesh(largeMesh);
					return ret;
				}
						
				maxFacets = SCE_PFX_MAX(maxFacets,island->m_numFacets);
				maxVerts = SCE_PFX_MAX(maxVerts,island->m_numVerts);
				maxEdges = SCE_PFX_MAX(maxEdges,island->m_numEdges);
			}
		}
		SCE_PFX_POP_PERF();

		if(param.flag & SCE_PFX_MESH_FLAG_OUTPUT_INFO) {
			SCE_PFX_PRINTF("generate \"%s\" completed!\n\tinput mesh verts %u triangles %u\n\tislands %u max triangles %u verts %u edges %u\n",
				param.name,param.numVerts,param.numTriangles,(PfxUInt32)largeMeshImpl->m_numIslands,maxFacets,maxVerts,maxEdges);
			
			PfxUInt32 totalbytes = sizeof(PfxLargeTriMesh);
			SCE_PFX_PRINTF("large mesh        %lu bytes\n",sizeof(PfxLargeTriMesh));
			
			if (param.flag & SCE_PFX_MESH_FLAG_STRUCTURE_COMPRESSED_BVH) {
				PfxUInt32 islandBytes;
				islandBytes = sizeof(PfxCompressedTriMesh) * largeMeshImpl->m_numIslands;
				PfxUInt32 bvhTreeBytes = sizeof(PfxIslandBvhNode) * largeMeshImpl->m_numBvhNodes;
				totalbytes += islandBytes + bvhTreeBytes + facetBuffBytes + edgeBuffBytes + vertexBuffBytes + bvhNodeBuffBytes;
				SCE_PFX_PRINTF("bvh tree          %u bytes\n",bvhTreeBytes);
				SCE_PFX_PRINTF("island buffer     %u bytes\n",islandBytes);
				SCE_PFX_PRINTF("facet buffer      %u bytes\n",facetBuffBytes);
				SCE_PFX_PRINTF("edge buffer       %u bytes\n",edgeBuffBytes);
				SCE_PFX_PRINTF("vertex buffer     %u bytes\n",vertexBuffBytes);
				SCE_PFX_PRINTF("bvh node buffer   %u bytes\n",bvhNodeBuffBytes);
			}
			else if (param.flag & SCE_PFX_MESH_FLAG_STRUCTURE_STANDARD_BVH) {
				PfxUInt32 bvhIslandBytes;
				bvhIslandBytes = sizeof(PfxQuantizedTriMeshBvh) * meshObj.numIslands;
				PfxUInt32 bvhTreeBytes = sizeof(PfxIslandBvhNode) * largeMeshImpl->m_numBvhNodes;
				totalbytes += bvhIslandBytes + bvhTreeBytes + facetBuffBytes + edgeBuffBytes + vertexBuffBytes + bvhNodeBuffBytes;
				SCE_PFX_PRINTF("bvh tree          %u bytes\n",bvhTreeBytes);
				SCE_PFX_PRINTF("bvh island buffer %u bytes\n",bvhIslandBytes);
				SCE_PFX_PRINTF("facet buffer      %u bytes\n",facetBuffBytes);
				SCE_PFX_PRINTF("edge buffer       %u bytes\n",edgeBuffBytes);
				SCE_PFX_PRINTF("vertex buffer     %u bytes\n",vertexBuffBytes);
				SCE_PFX_PRINTF("bvh node buffer   %u bytes\n",bvhNodeBuffBytes);
			}
			else {
				PfxUInt32 islandBytes;
				islandBytes = sizeof(PfxExpandedTriMesh) * largeMeshImpl->m_numIslands;
				PfxUInt32 aabbListBytes = sizeof(PfxAabb16) * largeMeshImpl->m_numIslands;
				totalbytes += aabbListBytes + islandBytes + facetBuffBytes + edgeBuffBytes + vertexBuffBytes;
				SCE_PFX_PRINTF("aabb buffer       %u bytes\n",aabbListBytes);
				SCE_PFX_PRINTF("island buffer     %u bytes\n",islandBytes);
				SCE_PFX_PRINTF("facet buffer      %u bytes\n",facetBuffBytes);
				SCE_PFX_PRINTF("edge buffer       %u bytes\n",edgeBuffBytes);
				SCE_PFX_PRINTF("vertex buffer     %u bytes\n",vertexBuffBytes);
			}
			SCE_PFX_PRINTF("----------------------------\n");
			SCE_PFX_PRINTF("total             %u bytes (%.2f bytes/triangle)\n",totalbytes,totalbytes/(float)param.numTriangles);
		}
	}
	else {
		pfxReleaseLargeTriMesh(largeMesh);
		return SCE_PFX_ERR_OUT_OF_RANGE_ISLAND;
	}

	char meshName[SCE_PFX_LARGETRIMESH_NAME_STR_MAX + 1];
	for (int i = 0; i < SCE_PFX_LARGETRIMESH_NAME_STR_MAX; i++) {
		largeMeshImpl->m_name[i] = meshName[i] = param.name[i];
		if (param.name[i] == '\0') break;
	}
	meshName[SCE_PFX_LARGETRIMESH_NAME_STR_MAX] = '\0';

	SCE_PFX_POP_PERF();

 //   if(nEdgeErrors) {
//        SCE_PFX_PRINTF(" Warning: %d edges shared by more than 2 vertices (while processing \"%s\")\n", nEdgeErrors, meshName);
	//}

	//SCE_PFX_ASSERT( (param.flag & SCE_PFX_MESH_FLAG_USE_BVH) ? largeMeshImpl->m_bvhNodes&&largeMeshImpl->m_islands : largeMeshImpl->m_islands );

	return SCE_PFX_OK;
}

void pfxReleaseLargeTriMesh(PfxLargeTriMesh &largeMesh)
{
	SCE_PFX_ASSERT(sizeof(PfxLargeTriMeshImpl) <= sizeof(PfxLargeTriMesh));

	PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;

	SCE_PFX_UTIL_FREE(largeMeshImpl->m_facetBuffer);
	SCE_PFX_UTIL_FREE(largeMeshImpl->m_edgeBuffer);
	SCE_PFX_UTIL_FREE(largeMeshImpl->m_vertexBuffer);
	SCE_PFX_UTIL_FREE(largeMeshImpl->m_bvhNodeBuffer);
	SCE_PFX_UTIL_FREE(largeMeshImpl->m_aabbList);
	SCE_PFX_UTIL_FREE(largeMeshImpl->m_islands);
	SCE_PFX_UTIL_FREE(largeMeshImpl->m_bvhNodes);

	largeMeshImpl->m_numIslands = 0;
	largeMeshImpl->m_numBvhNodes = 0;
}

} //namespace pfxv4
} //namespace sce
