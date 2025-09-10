/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2023 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../include/physics_effects/util/pfx_util_common.h"
#include "../../include/physics_effects/util/pfx_mesh_serializer.h"
#include "../base_level/collision/pfx_convex_mesh_impl.h"
#include "../base_level/collision/pfx_large_tri_mesh_impl.h"
#include "pfx_heap_manager.h"
#include "pfx_binary_reader_writer.h"

/*
	v3
	0.6 2012/12 First release
	0.7 2013/03 Add user data per each triangle to convex meshes

	v4
	0.8 2018/07 Add a unique name to meshes
*/

#define SCE_PFX_MESH_SERIALIZER_VERSION_MAJOR 0
#define SCE_PFX_MESH_SERIALIZER_VERSION_MINOR 8
#define SCE_PFX_MESH_SERIALIZER_VERSION_MINOR_SUPPORTED 7
#define SCE_PFX_MESH_SERIALIZER_CONVEX_HEADER_BYTES 32
#define SCE_PFX_MESH_SERIALIZER_CONVEX_HEADER_ID "PECV"
#define SCE_PFX_MESH_SERIALIZER_LMESH_HEADER_BYTES 96
#define SCE_PFX_MESH_SERIALIZER_LMESH_HEADER_ID "PELM"

namespace sce {
namespace pfxv4 {

struct PfxConvexMeshSerializeHeader {
	PfxUInt8 headerId[4];
	PfxUInt8 versionMajor;
	PfxUInt8 versionMinor;
	PfxUInt32 totalBytes;
	PfxUInt8 hasUserData;
	PfxFloat half[3];
	PfxUInt8 numVerts = 0;
	PfxUInt8 numFacets = 0;
	PfxUInt8 name[SCE_PFX_CONVEXMESH_NAME_STR_MAX];
};

struct PfxLargeTriMeshSerializeHeader {
	PfxUInt8 headerId[4] = {0};
	PfxUInt8 versionMajor = 0;
	PfxUInt8 versionMinor = 0;
	PfxUInt32 totalBytes = 0;
	PfxUInt8 lmeshType = 0;
	PfxUInt32 numIslands = 0;
	PfxUInt32 numBvhNodes = 0;
	PfxFloat offset[3];
	PfxFloat half[3];
	PfxUInt32 facetBuffBytes = 0;
	PfxUInt32 edgeBuffBytes = 0;
	PfxUInt32 vertexBuffBytes = 0;
	PfxUInt32 bvhNodeBuffBytes = 0;
	PfxFloat defaultThickness = 0.0f;
	PfxUInt8 name[SCE_PFX_LARGETRIMESH_NAME_STR_MAX];
};

static PfxUInt32 calcSerializedBytes(const PfxConvexMesh &convexMesh)
{
	PfxConvexMeshImpl *convexMeshImpl = (PfxConvexMeshImpl*)&convexMesh;

	PfxUInt32 bytes = SCE_PFX_MESH_SERIALIZER_CONVEX_HEADER_BYTES;

	bytes += 4; // 固定
	bytes += 4 * 3 * convexMeshImpl->m_numVerts;// 頂点バッファ
	bytes += 2 * 3 * convexMeshImpl->m_numFacets;// 頂点インデックス
	if( convexMeshImpl->m_userData ) {
        PfxUInt32 numTriangles = convexMeshImpl->m_numFacets;
        bytes += numTriangles * sizeof( PfxUInt32 );
    }
    
	return bytes;
}

static PfxUInt32 calcSerializedBytes(const PfxLargeTriMesh &largeMesh)
{
	PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;
	
	PfxUInt32 bytes = SCE_PFX_MESH_SERIALIZER_LMESH_HEADER_BYTES;

	// 共有ブロック
	if((largeMeshImpl->m_type & 0x02) == 0) {
		bytes += 4; // 固定
		bytes += 12 * largeMeshImpl->m_numIslands;
	}
	else {
		bytes += 8; // 固定
		bytes += 11 * largeMeshImpl->m_numBvhNodes;
	}
	
	// アイランドブロック
	for(PfxUInt32 i=0;i<largeMeshImpl->m_numIslands;i++) {
		if((largeMeshImpl->m_type & 0x04) != 0) {
			bytes += 31; // 固定
			PfxCompressedTriMesh *island = ((PfxCompressedTriMesh*)largeMeshImpl->m_islands) + i;
			bytes += 6 * island->m_numVerts;
			bytes += 15 * island->m_numFacets;
			bytes += 9 * SCE_PFX_MAX(1,island->m_numFacets - 1);
		}
		else {
			if((largeMeshImpl->m_type & 0x02) == 0) {
				bytes += 7; // 固定
				PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMeshImpl->m_islands) + i;
				bytes += 12 * island->m_numVerts;
				bytes += 4 * island->m_numEdges;
				bytes += 50 * island->m_numFacets;
			}
			else {
				bytes += 31; // 固定
				PfxQuantizedTriMeshBvh *island = ((PfxQuantizedTriMeshBvh*)largeMeshImpl->m_islands) + i;
				bytes += 6 * island->m_numVerts;
				bytes += 4 * island->m_numEdges;
				bytes += 16 * island->m_numFacets;
				bytes += 9 * SCE_PFX_MAX(1,island->m_numFacets - 1);
			}
		}
	}

	return bytes;
}

void loadConvexMeshHeader(PfxConvexMeshSerializeHeader &hinfo, const PfxUInt8 *meshBuff,PfxUInt32 meshBytes)
{
	const PfxUInt8 *p = meshBuff;
	readUInt8(&p,hinfo.headerId[0]);
	readUInt8(&p,hinfo.headerId[1]);
	readUInt8(&p,hinfo.headerId[2]);
	readUInt8(&p,hinfo.headerId[3]);
	readUInt8(&p,hinfo.versionMajor);
	readUInt8(&p,hinfo.versionMinor);
	readFloat32Array(&p,hinfo.half,3);
	readUInt8(&p,hinfo.numVerts);
	readUInt8(&p,hinfo.numFacets);
	readUInt8(&p,hinfo.hasUserData);
	readUInt32(&p,hinfo.totalBytes);
	readUInt8Array(&p, hinfo.name, SCE_PFX_CONVEXMESH_NAME_STR_MAX);
}

PfxInt32 verifyConvexMeshHeader(PfxConvexMeshSerializeHeader &hinfo, const PfxUInt8 *meshBuff,PfxUInt32 meshBytes)
{
	if(!meshBuff || meshBytes < SCE_PFX_MESH_SERIALIZER_CONVEX_HEADER_BYTES) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	loadConvexMeshHeader(hinfo, meshBuff, meshBytes);

	const PfxUInt8 checkHeaderId[] = SCE_PFX_MESH_SERIALIZER_CONVEX_HEADER_ID;

	if(!(hinfo.headerId[0] == checkHeaderId[0] && hinfo.headerId[1] == checkHeaderId[1] && 
		hinfo.headerId[2] == checkHeaderId[2] && hinfo.headerId[3] == checkHeaderId[3] )){
		return SCE_PFX_ERR_SERIALIZE_INVALID_HEADER;
	}
	else if(!((hinfo.versionMajor == SCE_PFX_MESH_SERIALIZER_VERSION_MAJOR && hinfo.versionMinor == SCE_PFX_MESH_SERIALIZER_VERSION_MINOR) ||
		(hinfo.versionMajor == SCE_PFX_MESH_SERIALIZER_VERSION_MAJOR && hinfo.versionMinor == SCE_PFX_MESH_SERIALIZER_VERSION_MINOR_SUPPORTED))) {
		return SCE_PFX_ERR_SERIALIZE_INVALID_VERSION;
	}
	else if(hinfo.totalBytes != meshBytes) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	return SCE_PFX_OK;
}

PfxInt32 pfxSerializeConvexMesh(const PfxConvexMesh &convexMesh,PfxUInt8 *meshBuff,PfxUInt32 meshBytes)
{
	if(meshBytes < calcSerializedBytes(convexMesh)) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxConvexMeshImpl *convexMeshImpl = (PfxConvexMeshImpl*)&convexMesh;

	const PfxUInt8 headerId[] = SCE_PFX_MESH_SERIALIZER_CONVEX_HEADER_ID;
	const PfxUInt8 versionMajor = SCE_PFX_MESH_SERIALIZER_VERSION_MAJOR;
	const PfxUInt8 versionMinor = SCE_PFX_MESH_SERIALIZER_VERSION_MINOR;

	PfxUInt8 *p = meshBuff;
	PfxUInt8 *pTotalBytes;

	PfxUInt8 hasUserData = (convexMeshImpl->m_userData != NULL);

	// ヘッダを書き込む
	writeUInt8(&p,headerId[0]);
	writeUInt8(&p,headerId[1]);
	writeUInt8(&p,headerId[2]);
	writeUInt8(&p,headerId[3]);
	writeUInt8(&p,versionMajor);
	writeUInt8(&p,versionMinor);
	writeFloat32Array(&p,convexMeshImpl->m_half,3);
	writeUInt8(&p,convexMeshImpl->m_numVerts);
	writeUInt8(&p,convexMeshImpl->m_numFacets);
	writeUInt8(&p,hasUserData);
	pTotalBytes = p;
	writeUInt32(&p,0); // 後で書き込む(pTotalBytes)
	writeUInt8Array(&p, convexMeshImpl->m_name, SCE_PFX_CONVEXMESH_NAME_STR_MAX);

	PfxUInt32 padding = (PfxUInt32)(SCE_PFX_MESH_SERIALIZER_CONVEX_HEADER_BYTES-(p- meshBuff));
	for(PfxUInt32 i=0;i<padding;i++) {
		writeUInt8(&p,0);
	}
	
	// メッシュデータブロックを書き込む
	writeUInt32(&p,4+12*convexMeshImpl->m_numVerts+2*3*convexMeshImpl->m_numFacets);
	for(PfxUInt32 j=0;j<convexMeshImpl->m_numVerts;j++) {
		writeFloat32Array(&p,convexMeshImpl->m_verts + j*3,3);
	}
	
	for(PfxUInt32 j=0;j<convexMeshImpl->m_numFacets * 3;j++) {
		writeUInt16(&p,(PfxUInt16)convexMeshImpl->m_indices[j]);
	}

    if(hasUserData) {
        PfxUInt32 numTriangles = convexMeshImpl->m_numFacets;
        for(PfxUInt32 j=0;j<numTriangles;j++) {
            writeUInt32(&p,convexMeshImpl->m_userData[j]);
        }
    }

	SCE_PFX_ASSERT(meshBytes == p - meshBuff);
	
	writeUInt32(&pTotalBytes, meshBytes); // トータルサイズ書き込み
	
	//SCE_PFX_PRINTF("Serialized convex mesh %u bytes\n",bytes);

	return SCE_PFX_OK;
}

PfxInt32 pfxRestoreConvexMesh(PfxConvexMesh &convexMesh,PfxUInt8 *outBuff,PfxUInt32 outBytes, const PfxUInt8 *meshBuff,PfxUInt32 meshBytes)
{
	if(!meshBuff || meshBytes < SCE_PFX_MESH_SERIALIZER_CONVEX_HEADER_BYTES) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	
	if(!SCE_PFX_PTR_IS_ALIGNED16(outBuff)) return SCE_PFX_ERR_INVALID_ALIGN;

	PfxInt32 ret = pfxQueryBytesToRestoreConvexMesh(meshBuff, meshBytes);

	if(ret < 0) return ret;

	if(outBytes <  ret) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxConvexMeshImpl *convexMeshImpl = (PfxConvexMeshImpl*)&convexMesh;
	memset(convexMeshImpl,0,sizeof(PfxConvexMeshImpl));

	PfxConvexMeshSerializeHeader hinfo;

	loadConvexMeshHeader(hinfo, meshBuff, meshBytes);
	
	convexMeshImpl->m_half[0] = hinfo.half[0];
	convexMeshImpl->m_half[1] = hinfo.half[1];
	convexMeshImpl->m_half[2] = hinfo.half[2];
	convexMeshImpl->m_numVerts = hinfo.numVerts;

	// In PFX v4, the number of facets is serialized instead of the number of indices
	if (hinfo.versionMajor == SCE_PFX_MESH_SERIALIZER_VERSION_MAJOR && hinfo.versionMinor == SCE_PFX_MESH_SERIALIZER_VERSION_MINOR_SUPPORTED) {
		convexMeshImpl->m_numFacets = hinfo.numFacets / 3; // it comes as the number of indices in V3
	}
	else {
		convexMeshImpl->m_numFacets = hinfo.numFacets;
	}

	for (int c = 0; c < SCE_PFX_CONVEXMESH_NAME_STR_MAX; c++) {
		convexMeshImpl->m_name[c] = hinfo.name[c];
	}

	const PfxUInt8 *p = meshBuff + SCE_PFX_MESH_SERIALIZER_CONVEX_HEADER_BYTES;

	PfxHeapManager heap(outBuff, outBytes);

	// 必要なバッファをアロケート
	convexMeshImpl->m_verts = heap.allocateArray<PfxFloat>(convexMeshImpl->m_numVerts * 3, 16);
	convexMeshImpl->m_indices = heap.allocateArray<PfxUInt8>(convexMeshImpl->m_numFacets * 3, 16);
	if(!convexMeshImpl->m_verts || !convexMeshImpl->m_indices) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxUInt32 meshBlockBytes = 0;
	readUInt32(&p,meshBlockBytes);
	for(PfxUInt32 j=0;j<convexMeshImpl->m_numVerts;j++) {
		readFloat32Array(&p,convexMeshImpl->m_verts + j*3,3);
	}
	
	for(PfxUInt32 j=0;j<convexMeshImpl->m_numFacets * 3;j++) {
		PfxUInt16 idx;
		readUInt16(&p,idx);
		convexMeshImpl->m_indices[j] = (PfxUInt8)idx;
	}
	
    if(hinfo.hasUserData) {
        PfxUInt32 numTriangles = convexMeshImpl->m_numFacets;
        convexMeshImpl->m_userData = heap.allocateArray<PfxUInt32>(numTriangles, 16);
        if(!convexMeshImpl->m_userData) return SCE_PFX_ERR_OUT_OF_BUFFER;
        for(PfxUInt32 j=0;j<numTriangles;j++) {
            readUInt32(&p,convexMeshImpl->m_userData[j]);
        }
    }
    else {
    	convexMeshImpl->m_userData = NULL;
    }
	
	return SCE_PFX_OK;
}

void loadLargeTriMeshHeader(PfxLargeTriMeshSerializeHeader &hinfo, const PfxUInt8 *meshBuff,PfxUInt32 meshBytes)
{
	const PfxUInt8 *p = meshBuff;
	readUInt8(&p,hinfo.headerId[0]);
	readUInt8(&p,hinfo.headerId[1]);
	readUInt8(&p,hinfo.headerId[2]);
	readUInt8(&p,hinfo.headerId[3]);
	readUInt8(&p,hinfo.versionMajor);
	readUInt8(&p,hinfo.versionMinor);
	readUInt8(&p,hinfo.lmeshType);
	readFloat32Array(&p,hinfo.offset,3);
	readFloat32Array(&p,hinfo.half,3);
	readUInt32(&p,hinfo.numIslands);
	readUInt32(&p,hinfo.facetBuffBytes);
	readUInt32(&p,hinfo.edgeBuffBytes);
	readUInt32(&p,hinfo.vertexBuffBytes);
	readUInt32(&p,hinfo.bvhNodeBuffBytes);
	readUInt32(&p,hinfo.totalBytes);
	readFloat32(&p,hinfo.defaultThickness);

	if(hinfo.versionMajor == SCE_PFX_MESH_SERIALIZER_VERSION_MAJOR && hinfo.versionMinor == SCE_PFX_MESH_SERIALIZER_VERSION_MINOR_SUPPORTED) {
		// previous version 0.7
		PfxUInt32 padding = (PfxUInt32)(64 - (p - meshBuff));
		p += padding;
	}
	else {
		readUInt8Array(&p, hinfo.name, SCE_PFX_LARGETRIMESH_NAME_STR_MAX);

		PfxUInt32 padding = (PfxUInt32)(SCE_PFX_MESH_SERIALIZER_LMESH_HEADER_BYTES - (p - meshBuff));
		p += padding;
	}

	if ((hinfo.lmeshType & 0x02) != 0) {
		PfxUInt32 islandBlockBytes = 0;
		readUInt32(&p, islandBlockBytes);
		readUInt32(&p, hinfo.numBvhNodes);
	}
}

PfxInt32 verifyLargeTriMeshHeader(PfxLargeTriMeshSerializeHeader &hinfo, const PfxUInt8 *meshBuff,PfxUInt32 meshBytes)
{
	if(!meshBuff || meshBytes < SCE_PFX_MESH_SERIALIZER_LMESH_HEADER_BYTES) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	loadLargeTriMeshHeader(hinfo, meshBuff, meshBytes);

	const PfxUInt8 checkHeaderId[] = SCE_PFX_MESH_SERIALIZER_LMESH_HEADER_ID;

	if(!(hinfo.headerId[0] == checkHeaderId[0] && hinfo.headerId[1] == checkHeaderId[1] && 
		hinfo.headerId[2] == checkHeaderId[2] && hinfo.headerId[3] == checkHeaderId[3] )){
		return SCE_PFX_ERR_SERIALIZE_INVALID_HEADER;
	}
	else if(!((hinfo.versionMajor == SCE_PFX_MESH_SERIALIZER_VERSION_MAJOR && hinfo.versionMinor == SCE_PFX_MESH_SERIALIZER_VERSION_MINOR) ||
		(hinfo.versionMajor == SCE_PFX_MESH_SERIALIZER_VERSION_MAJOR && hinfo.versionMinor == SCE_PFX_MESH_SERIALIZER_VERSION_MINOR_SUPPORTED))) {
		return SCE_PFX_ERR_SERIALIZE_INVALID_VERSION;
	}
	else if(hinfo.totalBytes != meshBytes) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	return SCE_PFX_OK;
}

PfxInt32 pfxSerializeLargeTriMesh(const PfxLargeTriMesh &largeMesh,PfxUInt8 *meshBuff,PfxUInt32 meshBytes)
{
	if(meshBytes < calcSerializedBytes(largeMesh)) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;

	const PfxUInt8 headerId[] = SCE_PFX_MESH_SERIALIZER_LMESH_HEADER_ID;
	const PfxUInt8 versionMajor = SCE_PFX_MESH_SERIALIZER_VERSION_MAJOR;
	const PfxUInt8 versionMinor = SCE_PFX_MESH_SERIALIZER_VERSION_MINOR;

	PfxUInt8 *p = meshBuff;
	PfxUInt8 *pTotalBytes;

	// ヘッダを書き込む
	writeUInt8(&p,headerId[0]);
	writeUInt8(&p,headerId[1]);
	writeUInt8(&p,headerId[2]);
	writeUInt8(&p,headerId[3]);
	writeUInt8(&p,versionMajor);
	writeUInt8(&p,versionMinor);
	writeUInt8(&p,largeMeshImpl->m_type);
	writeFloat32Array(&p,(PfxFloat*)&largeMeshImpl->m_offset,3);
	writeFloat32Array(&p,(PfxFloat*)&largeMeshImpl->m_half,3);
	writeUInt32(&p,largeMeshImpl->m_numIslands);
	writeUInt32(&p,largeMeshImpl->m_facetBuffBytes);
	writeUInt32(&p,largeMeshImpl->m_edgeBuffBytes);
	writeUInt32(&p,largeMeshImpl->m_vertexBuffBytes);
	writeUInt32(&p,largeMeshImpl->m_bvhNodeBuffBytes);
	pTotalBytes = p;
	writeUInt32(&p,0); // 後で書き込む(pTotalBytes)
	
	writeFloat32(&p,largeMeshImpl->m_defaultThickness); // 厚みのデフォルト値
	writeUInt8Array(&p, largeMeshImpl->m_name, SCE_PFX_LARGETRIMESH_NAME_STR_MAX);

	PfxUInt32 padding = (PfxUInt32)(SCE_PFX_MESH_SERIALIZER_LMESH_HEADER_BYTES-(p- meshBuff));
	for(PfxUInt32 i=0;i<padding;i++) {
		writeUInt8(&p,0);
	}
	
	// 共有ブロックを書き込む(x2 pattern)
	if((largeMeshImpl->m_type & 0x02) == 0) {
		// Array
		writeUInt32(&p,8+12*largeMeshImpl->m_numIslands);
		for(PfxUInt32 i=0;i<largeMeshImpl->m_numIslands;i++) {
			PfxAabb16 aabb = largeMeshImpl->m_aabbList[i];
			writeUInt16(&p,pfxGetXMin(aabb));
			writeUInt16(&p,pfxGetXMax(aabb));
			writeUInt16(&p,pfxGetYMin(aabb));
			writeUInt16(&p,pfxGetYMax(aabb));
			writeUInt16(&p,pfxGetZMin(aabb));
			writeUInt16(&p,pfxGetZMax(aabb));
		}
	}
	else {
		// Bvh
		writeUInt32(&p,8+11*largeMeshImpl->m_numBvhNodes);
		writeUInt32(&p,largeMeshImpl->m_numBvhNodes);
		for(PfxUInt32 i=0;i<largeMeshImpl->m_numBvhNodes;i++) {
			PfxIslandBvhNode encnode = largeMeshImpl->m_bvhNodes[i];
			writeUInt8(&p,encnode.aabb[0]);
			writeUInt8(&p,encnode.aabb[1]);
			writeUInt8(&p,encnode.aabb[2]);
			writeUInt8(&p,encnode.aabb[3]);
			writeUInt8(&p,encnode.aabb[4]);
			writeUInt8(&p,encnode.aabb[5]);
			writeUInt8(&p,encnode.flag);
			writeUInt16(&p,encnode.left);
			writeUInt16(&p,encnode.right);
		}
	}
	
	// アイランドブロックを書き込む(x4 pattern)
	for(PfxUInt32 i=0;i<largeMeshImpl->m_numIslands;i++) {
		PfxUInt8 *pIslandBlockBytes = p;
		
		if((largeMeshImpl->m_type & 0x04) != 0) {
			// High Compression
			const PfxCompressedTriMesh *island = ((PfxCompressedTriMesh*)largeMeshImpl->m_islands) + i;
			
			writeUInt32(&p,0); // 後で書き込む(pIslandBlockBytes)
			writeUInt8(&p,island->m_numVerts);
			writeUInt8(&p,island->m_numEdges);
			writeUInt8(&p,island->m_numFacets);
			writeFloat32Array(&p,island->m_aabbMin,3);
			writeFloat32Array(&p,island->m_aabbMax,3);
			for(PfxUInt32 j=0;j<island->m_numVerts;j++) {
				const PfxQuantize3 *vertex = ((PfxQuantize3*)largeMeshImpl->m_vertexBuffer) + island->m_verts + j;
				writeInt16Array(&p,(PfxInt16*)vertex,3);
			}
			for(PfxUInt32 j=0;j<island->m_numFacets;j++) {
				const PfxCompressedFacet2 *facet = ((PfxCompressedFacet2*)largeMeshImpl->m_facetBuffer) + island->m_facets + j;
				PfxUInt8 facetOffset = facet->m_facetInfo & 0x3F;
				PfxUInt8 edgeInfo0 = facet->m_edgeInfo & 0xFF;
				PfxUInt8 edgeInfo1 = (facet->m_edgeInfo>>8);
				if(facet->m_facetInfo & 0x8000) {
					facetOffset |= 0x80;
				}
				writeUInt8(&p,facetOffset);
				writeUInt8(&p,edgeInfo0);
				writeUInt8(&p,edgeInfo1);
				writeUInt8Array(&p,facet->m_vertIds,4);
				writeUInt32(&p,facet->m_userData[0]);
				writeUInt32(&p,facet->m_userData[1]);
			}
			for(PfxUInt32 j=0;j<(PfxUInt32)SCE_PFX_MAX(1,island->m_numFacets-1);j++) {
				const PfxFacetBvhNode *encnode = ((PfxFacetBvhNode*)largeMeshImpl->m_bvhNodeBuffer) + island->m_bvhNodes + j;
				writeUInt8(&p,encnode->aabb[0]);
				writeUInt8(&p,encnode->aabb[1]);
				writeUInt8(&p,encnode->aabb[2]);
				writeUInt8(&p,encnode->aabb[3]);
				writeUInt8(&p,encnode->aabb[4]);
				writeUInt8(&p,encnode->aabb[5]);
				writeUInt8(&p,encnode->flag);
				writeUInt8(&p,encnode->left);
				writeUInt8(&p,encnode->right);
			}
		}
		else {
			if((largeMeshImpl->m_type & 0x02) == 0) {
				// Array
				const PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMeshImpl->m_islands) + i;
					
				writeUInt32(&p,0); // 後で書き込む(pIslandBlockBytes)
				writeUInt8(&p,island->m_numVerts);
				writeUInt8(&p,island->m_numEdges);
				writeUInt8(&p,island->m_numFacets);
				for(PfxUInt32 j=0;j<island->m_numVerts;j++) {
					writeFloat32Array(&p,(PfxFloat*)&island->m_verts[j],3);
				}
				for(PfxUInt32 j=0;j<island->m_numEdges;j++) {
					writeUInt8(&p,island->m_edges[j].m_vertId[0]);
					writeUInt8(&p,island->m_edges[j].m_vertId[1]);
					writeUInt8(&p,island->m_edges[j].m_angleType);
					writeUInt8(&p,island->m_edges[j].m_tilt);
				}
				for(PfxUInt32 j=0;j<island->m_numFacets;j++) {
					writeFloat32Array(&p,(PfxFloat*)&island->m_facets[j].m_normal,3);
					writeFloat32Array(&p,(PfxFloat*)&island->m_facets[j].m_half,3);
					writeFloat32Array(&p,(PfxFloat*)&island->m_facets[j].m_center,3);
					writeFloat32(&p,island->m_facets[j].m_thickness);
					writeUInt8Array(&p,island->m_facets[j].m_vertIds,3);
					writeUInt8Array(&p,island->m_facets[j].m_edgeIds,3);
					writeUInt32(&p,island->m_facets[j].m_userData);
				}
			}
			else {
				// BVH
				const PfxQuantizedTriMeshBvh *island = ((PfxQuantizedTriMeshBvh*)largeMeshImpl->m_islands) + i;
					
				writeUInt32(&p,0); // 後で書き込む(pIslandBlockBytes)
				writeUInt8(&p,island->m_numVerts);
				writeUInt8(&p,island->m_numEdges);
				writeUInt8(&p,island->m_numFacets);
				writeFloat32Array(&p,island->m_aabbMin,3);
				writeFloat32Array(&p,island->m_aabbMax,3);
				for(PfxUInt32 j=0;j<island->m_numVerts;j++) {
					writeInt16Array(&p,(PfxInt16*)&island->m_verts[j],3);
				}
				for(PfxUInt32 j=0;j<island->m_numEdges;j++) {
					writeUInt8(&p,island->m_edges[j].m_vertId[0]);
					writeUInt8(&p,island->m_edges[j].m_vertId[1]);
					writeUInt8(&p,island->m_edges[j].m_angleType);
					writeUInt8(&p,island->m_edges[j].m_tilt);
				}
				for(PfxUInt32 j=0;j<island->m_numFacets;j++) {
					writeInt16Array(&p,(PfxInt16*)&island->m_facets[j].m_normal,2);
					writeInt16(&p,island->m_facets[j].m_thickness.elem);
					writeUInt8Array(&p,island->m_facets[j].m_vertIds,3);
					writeUInt8Array(&p,island->m_facets[j].m_edgeIds,3);
					writeUInt32(&p,island->m_facets[j].m_userData);
				}
				for(PfxUInt32 j=0;j<(PfxUInt32)SCE_PFX_MAX(1,island->m_numFacets-1);j++) {
					PfxFacetBvhNode encnode = island->m_bvhNodes[j];
					writeUInt8(&p,encnode.aabb[0]);
					writeUInt8(&p,encnode.aabb[1]);
					writeUInt8(&p,encnode.aabb[2]);
					writeUInt8(&p,encnode.aabb[3]);
					writeUInt8(&p,encnode.aabb[4]);
					writeUInt8(&p,encnode.aabb[5]);
					writeUInt8(&p,encnode.flag);
					writeUInt8(&p,encnode.left);
					writeUInt8(&p,encnode.right);
				}
			}
			
			writeUInt32(&pIslandBlockBytes,(PfxUInt32)(p-pIslandBlockBytes)); // アイランドサイズ書き込み
		}
	}

	SCE_PFX_ASSERT(meshBytes == p- meshBuff);
	
	writeUInt32(&pTotalBytes, meshBytes); // トータルサイズ書き込み

	//SCE_PFX_PRINTF("Serialized large mesh %u bytes\n",bytes);

	return SCE_PFX_OK;
}

PfxInt32 pfxRestoreLargeTriMesh(PfxLargeTriMesh &largeMesh,PfxUInt8 *outBuff,PfxUInt32 outBytes, const PfxUInt8 *meshBuff,PfxUInt32 meshBytes)
{
	if(!meshBuff || meshBytes < SCE_PFX_MESH_SERIALIZER_LMESH_HEADER_BYTES) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	
	if(!SCE_PFX_PTR_IS_ALIGNED16(outBuff)) return SCE_PFX_ERR_INVALID_ALIGN;

	PfxInt32 ret = pfxQueryBytesToRestoreLargeTriMesh(meshBuff, meshBytes);

	if(ret < 0) return ret;

	if(outBytes <  ret) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;
	memset(largeMeshImpl,0,sizeof(PfxLargeTriMeshImpl));
	
	PfxLargeTriMeshSerializeHeader hinfo;

	loadLargeTriMeshHeader(hinfo, meshBuff, meshBytes);

	largeMeshImpl->m_type = hinfo.lmeshType;
	largeMeshImpl->m_offset[0] = hinfo.offset[0];
	largeMeshImpl->m_offset[1] = hinfo.offset[1];
	largeMeshImpl->m_offset[2] = hinfo.offset[2];
	largeMeshImpl->m_half[0] = hinfo.half[0];
	largeMeshImpl->m_half[1] = hinfo.half[1];
	largeMeshImpl->m_half[2] = hinfo.half[2];
	largeMeshImpl->m_length = length(largeMeshImpl->getHalf());
	largeMeshImpl->m_numIslands = hinfo.numIslands;
	largeMeshImpl->m_facetBuffBytes = hinfo.facetBuffBytes;
	largeMeshImpl->m_edgeBuffBytes = hinfo.edgeBuffBytes;
	largeMeshImpl->m_vertexBuffBytes = hinfo.vertexBuffBytes;
	largeMeshImpl->m_bvhNodeBuffBytes = hinfo.bvhNodeBuffBytes;
	largeMeshImpl->m_defaultThickness = hinfo.defaultThickness;

	const PfxUInt8 *p = nullptr;

	if(hinfo.versionMajor == SCE_PFX_MESH_SERIALIZER_VERSION_MAJOR && hinfo.versionMinor == SCE_PFX_MESH_SERIALIZER_VERSION_MINOR_SUPPORTED) {
		p = meshBuff + 64;
	}
	else {
		for (int c = 0; c < SCE_PFX_LARGETRIMESH_NAME_STR_MAX; c++) {
			largeMeshImpl->m_name[c] = hinfo.name[c];
		}
		p = meshBuff + SCE_PFX_MESH_SERIALIZER_LMESH_HEADER_BYTES;
	}

	PfxHeapManager heap(outBuff, outBytes);

	// 共有ブロックを読み込む
	PfxUInt32 islandBlockBytes = 0;
	readUInt32(&p,islandBlockBytes);
	
	if((largeMeshImpl->m_type & 0x02) == 0) {
		largeMeshImpl->m_aabbList = heap.allocateArray<PfxAabb16>(largeMeshImpl->m_numIslands, 16);
		if(!largeMeshImpl->m_aabbList) return SCE_PFX_ERR_OUT_OF_BUFFER;
		
		for(PfxUInt32 i=0;i<largeMeshImpl->m_numIslands;i++) {
			PfxUInt16 xmin,xmax,ymin,ymax,zmin,zmax;
			PfxAabb16 aabb;
			readUInt16(&p,xmin); pfxSetXMin(aabb,xmin);
			readUInt16(&p,xmax); pfxSetXMax(aabb,xmax);
			readUInt16(&p,ymin); pfxSetYMin(aabb,ymin);
			readUInt16(&p,ymax); pfxSetYMax(aabb,ymax);
			readUInt16(&p,zmin); pfxSetZMin(aabb,zmin);
			readUInt16(&p,zmax); pfxSetZMax(aabb,zmax);
			largeMeshImpl->m_aabbList[i] = aabb;
		}
	}
	else {
		readUInt32(&p,largeMeshImpl->m_numBvhNodes);
		
		largeMeshImpl->m_bvhNodes = heap.allocateArray<PfxIslandBvhNode>(largeMeshImpl->m_numBvhNodes, 16);
		if(!largeMeshImpl->m_bvhNodes) return SCE_PFX_ERR_OUT_OF_BUFFER;
		
		for(PfxUInt32 i=0;i<largeMeshImpl->m_numBvhNodes;i++) {
			PfxIslandBvhNode encnode;
			readUInt8(&p,encnode.aabb[0]);
			readUInt8(&p,encnode.aabb[1]);
			readUInt8(&p,encnode.aabb[2]);
			readUInt8(&p,encnode.aabb[3]);
			readUInt8(&p,encnode.aabb[4]);
			readUInt8(&p,encnode.aabb[5]);
			readUInt8(&p,encnode.flag);
			readUInt16(&p,encnode.left);
			readUInt16(&p,encnode.right);
			largeMeshImpl->m_bvhNodes[i] = encnode;
		}
	}
	
	// アイランドブロック用バッファの確保
	switch(largeMeshImpl->m_type) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		largeMeshImpl->m_islands = heap.allocateArray<PfxExpandedTriMesh>(hinfo.numIslands, 16);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		largeMeshImpl->m_islands = heap.allocateArray<PfxQuantizedTriMeshBvh>(hinfo.numIslands, 16);
		break;
		
		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		largeMeshImpl->m_islands = heap.allocateArray<PfxCompressedTriMesh>(hinfo.numIslands, 16);
		break;
		
		default:
		SCE_PFX_ASSERT_MSG(false, "unsupported PFX mesh type");
		largeMeshImpl->m_islands = NULL;
		break;
	}
	if(!largeMeshImpl->m_islands) return SCE_PFX_ERR_OUT_OF_BUFFER;
	
	largeMeshImpl->m_facetBuffer = largeMeshImpl->m_facetBuffBytes > 0 ? heap.allocate( largeMeshImpl->m_facetBuffBytes, 16 ) : nullptr;
	largeMeshImpl->m_edgeBuffer = largeMeshImpl->m_edgeBuffBytes > 0 ? heap.allocate( largeMeshImpl->m_edgeBuffBytes, 16 ) : nullptr;
	largeMeshImpl->m_vertexBuffer = largeMeshImpl->m_vertexBuffBytes > 0 ? heap.allocate( largeMeshImpl->m_vertexBuffBytes, 16 ) : nullptr;
	largeMeshImpl->m_bvhNodeBuffer = largeMeshImpl->m_bvhNodeBuffBytes > 0 ? heap.allocate( largeMeshImpl->m_bvhNodeBuffBytes, 16 ) : nullptr;

	if( largeMeshImpl->m_facetBuffBytes > 0 && !largeMeshImpl->m_facetBuffer ) return SCE_PFX_ERR_OUT_OF_BUFFER;
	if( largeMeshImpl->m_edgeBuffBytes > 0 && !largeMeshImpl->m_edgeBuffer ) return SCE_PFX_ERR_OUT_OF_BUFFER;
	if( largeMeshImpl->m_vertexBuffBytes > 0 && !largeMeshImpl->m_vertexBuffer ) return SCE_PFX_ERR_OUT_OF_BUFFER;
	if( largeMeshImpl->m_bvhNodeBuffBytes > 0 && !largeMeshImpl->m_bvhNodeBuffer ) return SCE_PFX_ERR_OUT_OF_BUFFER;

	// アイランドブロックを読み込む
	if((largeMeshImpl->m_type & 0x04) != 0) {
		// High Compression
		PfxUInt32 idxFacetBuffer = 0;
		PfxUInt32 idxVertexBuffer = 0;
		PfxUInt32 idxBvhNodeBuffer = 0;

		for(PfxUInt32 i=0;i<largeMeshImpl->m_numIslands;i++) {
			PfxCompressedTriMesh *island = ((PfxCompressedTriMesh*)largeMeshImpl->m_islands) + i;
			island->m_facets = idxFacetBuffer;
			island->m_verts = idxVertexBuffer;
			island->m_bvhNodes = idxBvhNodeBuffer;

			PfxUInt32 islandBlockBytes;
			readUInt32(&p,islandBlockBytes);
			readUInt8(&p,island->m_numVerts);
			readUInt8(&p,island->m_numEdges);
			readUInt8(&p,island->m_numFacets);
			readFloat32Array(&p,island->m_aabbMin,3);
			readFloat32Array(&p,island->m_aabbMax,3);
			for(PfxUInt32 j=0;j<island->m_numVerts;j++) {
				PfxQuantize3 *vertex = ((PfxQuantize3*)largeMeshImpl->m_vertexBuffer) + island->m_verts + j;
				readInt16Array(&p,(PfxInt16*)vertex,3);
			}
			for(PfxUInt32 j=0;j<island->m_numFacets;j++) {
				PfxCompressedFacet2 *facet = ((PfxCompressedFacet2*)largeMeshImpl->m_facetBuffer) + island->m_facets + j;
				PfxUInt8 facetOffset=0,edgeInfo0=0,edgeInfo1=0;
				readUInt8(&p,facetOffset);
				readUInt8(&p,edgeInfo0);
				readUInt8(&p,edgeInfo1);
				facet->m_facetInfo = facetOffset & 0x3F;
				if(facetOffset & 0x80) {
					facet->m_facetInfo |= 0x8000;
				}
				facet->m_edgeInfo = (edgeInfo1<<8) | edgeInfo0;
				readUInt8Array(&p,facet->m_vertIds,4);
				readUInt32(&p,facet->m_userData[0]);
				readUInt32(&p,facet->m_userData[1]);
			}
			for(PfxUInt32 j=0;j<(PfxUInt32)SCE_PFX_MAX(1,island->m_numFacets-1);j++) {
				PfxFacetBvhNode *encnode = ((PfxFacetBvhNode*)largeMeshImpl->m_bvhNodeBuffer) + island->m_bvhNodes + j;
				readUInt8(&p,encnode->aabb[0]);
				readUInt8(&p,encnode->aabb[1]);
				readUInt8(&p,encnode->aabb[2]);
				readUInt8(&p,encnode->aabb[3]);
				readUInt8(&p,encnode->aabb[4]);
				readUInt8(&p,encnode->aabb[5]);
				readUInt8(&p,encnode->flag);
				readUInt8(&p,encnode->left);
				readUInt8(&p,encnode->right);
			}
			
			idxFacetBuffer += island->m_numFacets;
			idxVertexBuffer += island->m_numVerts;
			idxBvhNodeBuffer += SCE_PFX_MAX(1,island->m_numFacets-1);
			
			SCE_PFX_ASSERT(idxFacetBuffer * sizeof(PfxCompressedFacet2) <= largeMeshImpl->m_facetBuffBytes);
			SCE_PFX_ASSERT(idxVertexBuffer * sizeof(PfxQuantize3) <= largeMeshImpl->m_vertexBuffBytes);
			SCE_PFX_ASSERT(idxBvhNodeBuffer * sizeof(PfxFacetBvhNode) <= largeMeshImpl->m_bvhNodeBuffBytes);
		}
	}
	else {
		if((largeMeshImpl->m_type & 0x02) == 0) {
			// Array
			PfxUInt8 *ptrFacetBuffer = (PfxUInt8*)largeMeshImpl->m_facetBuffer;
			PfxUInt8 *ptrEdgeBuffer = (PfxUInt8*)largeMeshImpl->m_edgeBuffer;
			PfxUInt8 *ptrVertexBuffer = (PfxUInt8*)largeMeshImpl->m_vertexBuffer;

			for(PfxUInt32 i=0;i<largeMeshImpl->m_numIslands;i++) {
				PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMeshImpl->m_islands) + i;
				island->m_facets = (PfxExpandedFacet*)ptrFacetBuffer;
				island->m_edges = (PfxEdge*)ptrEdgeBuffer;
				island->m_verts = (PfxFloat3*)ptrVertexBuffer;
					
				PfxUInt32 islandBlockBytes;
				readUInt32(&p,islandBlockBytes);
				readUInt8(&p,island->m_numVerts);
				readUInt8(&p,island->m_numEdges);
				readUInt8(&p,island->m_numFacets);
				for(PfxUInt32 j=0;j<island->m_numVerts;j++) {
					readFloat32Array(&p,(PfxFloat*)&island->m_verts[j],3);
				}
				for(PfxUInt32 j=0;j<island->m_numEdges;j++) {
					readUInt8(&p,island->m_edges[j].m_vertId[0]);
					readUInt8(&p,island->m_edges[j].m_vertId[1]);
					readUInt8(&p,island->m_edges[j].m_angleType);
					readUInt8(&p,island->m_edges[j].m_tilt);
				}
				for(PfxUInt32 j=0;j<island->m_numFacets;j++) {
					readFloat32Array(&p,(PfxFloat*)&island->m_facets[j].m_normal,3);
					readFloat32Array(&p,(PfxFloat*)&island->m_facets[j].m_half,3);
					readFloat32Array(&p,(PfxFloat*)&island->m_facets[j].m_center,3);
					readFloat32(&p,island->m_facets[j].m_thickness);
					readUInt8Array(&p,island->m_facets[j].m_vertIds,3);
					readUInt8Array(&p,island->m_facets[j].m_edgeIds,3);
					readUInt32(&p,island->m_facets[j].m_userData);
				}
					
				ptrFacetBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxExpandedFacet)*island->m_numFacets);
				ptrEdgeBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxEdge)*island->m_numEdges);
				ptrVertexBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxFloat3)*island->m_numVerts);
				
				SCE_PFX_ASSERT(((uintptr_t)ptrFacetBuffer - (uintptr_t)largeMeshImpl->m_facetBuffer) <= largeMeshImpl->m_facetBuffBytes);
				SCE_PFX_ASSERT(((uintptr_t)ptrEdgeBuffer - (uintptr_t)largeMeshImpl->m_edgeBuffer) <= largeMeshImpl->m_edgeBuffBytes);
				SCE_PFX_ASSERT(((uintptr_t)ptrVertexBuffer - (uintptr_t)largeMeshImpl->m_vertexBuffer) <= largeMeshImpl->m_vertexBuffBytes);
			}
		}
		else {
			// BVH
			PfxUInt8 *ptrFacetBuffer = (PfxUInt8*)largeMeshImpl->m_facetBuffer;
			PfxUInt8 *ptrEdgeBuffer = (PfxUInt8*)largeMeshImpl->m_edgeBuffer;
			PfxUInt8 *ptrVertexBuffer = (PfxUInt8*)largeMeshImpl->m_vertexBuffer;
			PfxUInt8 *ptrBvhNodeBuffer = (PfxUInt8*)largeMeshImpl->m_bvhNodeBuffer;

			for(PfxUInt32 i=0;i<largeMeshImpl->m_numIslands;i++) {
				PfxQuantizedTriMeshBvh *island = ((PfxQuantizedTriMeshBvh*)largeMeshImpl->m_islands)+i;
				island->m_facets = (PfxQuantizedFacetBvh*)ptrFacetBuffer;
				island->m_edges = (PfxEdge*)ptrEdgeBuffer;
				island->m_verts = (PfxQuantize3*)ptrVertexBuffer;
				island->m_bvhNodes = (PfxFacetBvhNode*)ptrBvhNodeBuffer;

				PfxUInt32 islandBlockBytes;
				readUInt32(&p,islandBlockBytes);
				readUInt8(&p,island->m_numVerts);
				readUInt8(&p,island->m_numEdges);
				readUInt8(&p,island->m_numFacets);
				readFloat32Array(&p,island->m_aabbMin,3);
				readFloat32Array(&p,island->m_aabbMax,3);
				for(PfxUInt32 j=0;j<island->m_numVerts;j++) {
					readInt16Array(&p,(PfxInt16*)&island->m_verts[j],3);
				}
				for(PfxUInt32 j=0;j<island->m_numEdges;j++) {
					readUInt8(&p,island->m_edges[j].m_vertId[0]);
					readUInt8(&p,island->m_edges[j].m_vertId[1]);
					readUInt8(&p,island->m_edges[j].m_angleType);
					readUInt8(&p,island->m_edges[j].m_tilt);
				}
				for(PfxUInt32 j=0;j<island->m_numFacets;j++) {
					readInt16Array(&p,(PfxInt16*)&island->m_facets[j].m_normal,2);
					readInt16(&p,island->m_facets[j].m_thickness.elem);
					readUInt8Array(&p,island->m_facets[j].m_vertIds,3);
					readUInt8Array(&p,island->m_facets[j].m_edgeIds,3);
					readUInt32(&p,island->m_facets[j].m_userData);
				}
				for(PfxUInt32 j=0;j<(PfxUInt32)SCE_PFX_MAX(1,island->m_numFacets-1);j++) {
					PfxFacetBvhNode encnode;
					readUInt8(&p,encnode.aabb[0]);
					readUInt8(&p,encnode.aabb[1]);
					readUInt8(&p,encnode.aabb[2]);
					readUInt8(&p,encnode.aabb[3]);
					readUInt8(&p,encnode.aabb[4]);
					readUInt8(&p,encnode.aabb[5]);
					readUInt8(&p,encnode.flag);
					readUInt8(&p,encnode.left);
					readUInt8(&p,encnode.right);
					island->m_bvhNodes[j] = encnode;
				}
					
				ptrFacetBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxQuantizedFacetBvh)*island->m_numFacets);
				ptrEdgeBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxEdge)*island->m_numEdges);
				ptrVertexBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxQuantize3)*island->m_numVerts);
				ptrBvhNodeBuffer += SCE_PFX_BYTES_ALIGN16(sizeof(PfxFacetBvhNode)*SCE_PFX_MAX(1,island->m_numFacets-1));

				SCE_PFX_ASSERT(((uintptr_t)ptrFacetBuffer - (uintptr_t)largeMeshImpl->m_facetBuffer) <= largeMeshImpl->m_facetBuffBytes);
				SCE_PFX_ASSERT(((uintptr_t)ptrEdgeBuffer - (uintptr_t)largeMeshImpl->m_edgeBuffer) <= largeMeshImpl->m_edgeBuffBytes);
				SCE_PFX_ASSERT(((uintptr_t)ptrVertexBuffer - (uintptr_t)largeMeshImpl->m_vertexBuffer) <= largeMeshImpl->m_vertexBuffBytes);
				SCE_PFX_ASSERT(((uintptr_t)ptrBvhNodeBuffer - (uintptr_t)largeMeshImpl->m_bvhNodeBuffer) <= largeMeshImpl->m_bvhNodeBuffBytes);
			}
		}
	}
	
	//SCE_PFX_PRINTF("Restore a large mesh completed. bytes = %u islands = %u triangles = %u\n",meshBytes,largeMeshImpl->m_numIslands,totalFacets);

	return SCE_PFX_OK;
}

PfxUInt32 pfxQuerySerializedBytesOfConvexMesh(const PfxConvexMesh &convexMesh)
{
	PfxConvexMeshImpl *convexMeshImpl = (PfxConvexMeshImpl*)&convexMesh;

	PfxUInt32 bytes = SCE_PFX_MESH_SERIALIZER_CONVEX_HEADER_BYTES;

	bytes += 4; // 固定
	bytes += 4 * 3 * convexMeshImpl->m_numVerts;// 頂点バッファ
	bytes += 2 * 3 * convexMeshImpl->m_numFacets;// 頂点インデックス
	if( convexMeshImpl->m_userData ) {
        PfxUInt32 numTriangles = convexMeshImpl->m_numFacets;
        bytes += numTriangles * sizeof( PfxUInt32 );
    }
    
	return bytes;
}

PfxUInt32 pfxQuerySerializedBytesOfLargeTriMesh(const PfxLargeTriMesh &largeMesh)
{
	PfxLargeTriMeshImpl *largeMeshImpl = (PfxLargeTriMeshImpl*)&largeMesh;
	
	PfxUInt32 bytes = SCE_PFX_MESH_SERIALIZER_LMESH_HEADER_BYTES;

	// 共有ブロック
	if((largeMeshImpl->m_type & 0x02) == 0) {
		bytes += 4; // 固定
		bytes += 12 * largeMeshImpl->m_numIslands;
	}
	else {
		bytes += 8; // 固定
		bytes += 11 * largeMeshImpl->m_numBvhNodes;
	}
	
	// アイランドブロック
	for(PfxUInt32 i=0;i<largeMeshImpl->m_numIslands;i++) {
		if((largeMeshImpl->m_type & 0x04) != 0) {
			bytes += 31; // 固定
			PfxCompressedTriMesh *island = ((PfxCompressedTriMesh*)largeMeshImpl->m_islands) + i;
			bytes += 6 * island->m_numVerts;
			bytes += 15 * island->m_numFacets;
			bytes += 9 * SCE_PFX_MAX(1,island->m_numFacets - 1);
		}
		else {
			if((largeMeshImpl->m_type & 0x02) == 0) {
				bytes += 7; // 固定
				PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMeshImpl->m_islands) + i;
				bytes += 12 * island->m_numVerts;
				bytes += 4 * island->m_numEdges;
				bytes += 50 * island->m_numFacets;
			}
			else {
				bytes += 31; // 固定
				PfxQuantizedTriMeshBvh *island = ((PfxQuantizedTriMeshBvh*)largeMeshImpl->m_islands) + i;
				bytes += 6 * island->m_numVerts;
				bytes += 4 * island->m_numEdges;
				bytes += 16 * island->m_numFacets;
				bytes += 9 * SCE_PFX_MAX(1,island->m_numFacets - 1);
			}
		}
	}

	return bytes;
}

PfxInt32 pfxQueryBytesToRestoreConvexMesh(const PfxUInt8 *meshBuff,PfxUInt32 meshBytes)
{
	if(!meshBuff || meshBytes < SCE_PFX_MESH_SERIALIZER_CONVEX_HEADER_BYTES) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	
	PfxConvexMeshSerializeHeader hinfo;

	PfxInt32 ret = verifyConvexMeshHeader(hinfo, meshBuff, meshBytes);
	if(SCE_PFX_OK != ret) {
		return ret;
	}

	//const PfxUInt8 *p = meshBuff + SCE_PFX_MESH_SERIALIZER_CONVEX_HEADER_BYTES;

	// Calculate the size of buffer needed to recover a mesh
	PfxUInt32 bytes = 0;
	bytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxFloat)*hinfo.numVerts * 3);
	bytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxUInt8)*hinfo.numFacets * 3);
	
	if(hinfo.hasUserData) {
		bytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxUInt32)*hinfo.numFacets);
	}

	return (PfxInt32)bytes;
}

PfxInt32 pfxQueryBytesToRestoreLargeTriMesh(const PfxUInt8 *meshBuff,PfxUInt32 meshBytes)
{
	if(!meshBuff || meshBytes < SCE_PFX_MESH_SERIALIZER_LMESH_HEADER_BYTES) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	PfxLargeTriMeshSerializeHeader hinfo;

	PfxInt32 ret = verifyLargeTriMeshHeader(hinfo, meshBuff, meshBytes);
	if(SCE_PFX_OK != ret) {
		return ret;
	}

	//const PfxUInt8 *p = meshBuff + SCE_PFX_MESH_SERIALIZER_LMESH_HEADER_BYTES;

	// Calculate the size of buffer needed to recover a mesh
	PfxUInt32 bytes = 0;

	if((hinfo.lmeshType & 0x02) == 0) {
		bytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxAabb16)*hinfo.numIslands);
	}
	else {
		bytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxIslandBvhNode)*hinfo.numBvhNodes);
	}

	switch(hinfo.lmeshType) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		bytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxExpandedTriMesh)*hinfo.numIslands);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		bytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxQuantizedTriMeshBvh)*hinfo.numIslands);
		break;
		
		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		bytes += SCE_PFX_BYTES_ALIGN16(sizeof(PfxCompressedTriMesh)*hinfo.numIslands);
		break;
	}

	bytes += SCE_PFX_BYTES_ALIGN16(hinfo.facetBuffBytes);
	bytes += SCE_PFX_BYTES_ALIGN16(hinfo.edgeBuffBytes);
	bytes += SCE_PFX_BYTES_ALIGN16(hinfo.vertexBuffBytes);
	bytes += SCE_PFX_BYTES_ALIGN16(hinfo.bvhNodeBuffBytes);

	return (PfxInt32)bytes;
}

} //namespace pfxv4
} //namespace sce
