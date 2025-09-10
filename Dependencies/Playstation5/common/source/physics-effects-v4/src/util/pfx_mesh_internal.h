/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_MESH_INTERNAL_H
#define _SCE_PFX_MESH_INTERNAL_H

#include "../../include/physics_effects/util/pfx_static_array.h"
#include "../../include/physics_effects/util/pfx_mesh_creator.h"
#include "../base_level/collision/pfx_intersect_common.h"
#include "pfx_array.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
//J メッシュ変換用の中間メッシュ
//J 適切なデータ構造へ変換するため、頂点、エッジ、面の接続関係を構築する。
//E Construct a structure of connectivity between vertices, edges and facets
//E before converting to an appropriate data.

struct PfxMcVert {
	PfxInt32 vertId;	// 頂点ID
	PfxUInt8  localId;	// アイランドローカルな頂点ID
	PfxBool obsolete;	// 廃棄フラグ
	PfxVector3 coord;	// 座標
	PfxVector3 normal;	// 法線
	PfxQuantize3 quant; // 量子化座標
};

struct PfxMcEdge {
	PfxUInt32 vertId[2];  // 頂点ID
	PfxUInt32 edgeId[2];  // エッジID
	PfxUInt32 facetId[2]; // 面ID
	PfxUInt32 numFacets;  // 共有面の数
	PfxUInt32 angleType;  // 辺の種類
	PfxFloat angle;		  // 辺の角度
	PfxVector3 normal;	  // 法線
	PfxMcEdge *next;      // 次のエッジへのリンク
	
	PfxBool isAcuteAngle()
	{
		return angleType == SCE_PFX_EDGE_CONVEX && angle > 0.0f;
	}
};

struct PfxMcFacet {
	PfxMcVert *v[3];			// 頂点
	PfxMcEdge *e[3];			// エッジ
	PfxUInt32 id;				// 面ID
	PfxInt32 neighbor[3];		// 隣接面のインデックス
	PfxInt32 neighborEdgeId[3];	// 隣接面のエッジインデックス
	PfxFloat thickness;			// 厚み
	PfxBool obsolete;			// 廃棄フラグ
	PfxUInt32  userData;		// ユーザデータ
	PfxVector3 normal;			// 法線
	PfxVector3 aabbMin;			// AABB最小座標
	PfxVector3 aabbMax;			// AABB最大座標
	PfxMcFacet *next;			// リンク
};

struct PfxMcVert2TriList {
	PfxMcFacet *facet;
	PfxMcVert2TriList *next;

	PfxMcVert2TriList()
	{
		facet = NULL;
		next = NULL;
	}
};

struct PfxMcIslandBvhNode {
	PfxVector3 aabbMin;
	PfxVector3 aabbMax;
	PfxVector3 aabbSize;
	PfxMcIslandBvhNode *left;
	PfxMcIslandBvhNode *right;
	PfxMcIslandBvhNode *parent;
	PfxMcFacet *facetList;
	PfxUInt32 numFacets;
	PfxUInt32 islandId;
	PfxUInt32 memo;
	PfxFloat surfaceArea;
	PfxFloat volume;


	PfxMcIslandBvhNode()
	{
		left = right = parent = NULL;
		numFacets = 0;
		facetList = NULL;
		islandId = 0;
		memo = 0;
		surfaceArea = volume = 0.0f;
	}
	
	PfxBool isLeaf() const
	{
		return !left && !right;
	}
};

struct PfxMeshObject {
	PfxArray<PfxMcVert>  vertList;
	PfxArray<PfxMcFacet> facetList;
	
	// 頂点からエッジへの参照リスト
	PfxArray<PfxMcEdge>  edgeList;
	PfxArray<PfxMcEdge*> edgeHead;
	
	// 頂点から面への参照リスト
	PfxArray<PfxMcVert2TriList> triEntry;
	PfxArray<PfxMcVert2TriList*> triHead;
	
	// BVH
	PfxArray<PfxMcIslandBvhNode> bvh;
	PfxArray<PfxMcIslandBvhNode*> islands;
	PfxArray<PfxMcIslandBvhNode*> roots;
	PfxUInt32 numIslands;
	PfxUInt32 numBvhNodes;
	PfxUInt32 numRoots;
	
	// 全体のAABB
	PfxVector3 totalAabbMin;
	PfxVector3 totalAabbMax;
};

PfxInt32 pfxMeshObjectBuild(PfxMeshObject &meshObj,const PfxCreateLargeTriMeshParam &param,PfxBool enableMultiParts = false);

void pfxMeshObjectPrint(const PfxMeshObject &meshObj);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_MESH_INTERNAL_H
