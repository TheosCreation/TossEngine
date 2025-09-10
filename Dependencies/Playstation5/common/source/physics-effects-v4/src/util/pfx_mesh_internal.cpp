/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2024 Sony Interactive Entertainment Inc.
 *                                                
 */

//#define SCE_PFX_USE_PERFCOUNTER
//#define SCE_PFX_DIVIDE_BY_VOLUME
#define SCE_PFX_DIVIDE_BY_CORRECT_PROB

#include "pfx_precompiled.h"
#include "pfx_mesh_internal.h"
#include "../../include/physics_effects/base_level/sort/pfx_sort.h"
#include "../../include/physics_effects/base_level/base/pfx_perf_counter.h"

namespace sce {
namespace pfxv4 {

static PfxFloat epsilon = 0.00001f;

static inline
PfxUInt32 floatFlip(PfxFloat flt)
{
	PfxUInt32 iflt = *((PfxUInt32*)&flt);
	PfxInt32 msb = (PfxInt32)(iflt>>31);
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
		facets.assign(n,dummy);
		work.assign(n,dummy);
		return facets.ptr() && work.ptr();
	}
	
	void set(PfxUInt32 i,const PfxFloat fval,void *ptr)
	{
		pfxSetKey(facets[i],floatFlip(fval));
		PfxUInt64 ptr64 = (uintptr_t)ptr;
		facets[i].set32(0,(PfxUInt32)(ptr64&0xffffffff));
		facets[i].set32(1,(PfxUInt32)(ptr64>>32));
	}

	void set(PfxUInt32 i, PfxUInt32 keyVal, void *ptr)
	{
		pfxSetKey(facets[i], keyVal);
		PfxUInt64 ptr64 = (uintptr_t)ptr;
		facets[i].set32(0, (PfxUInt32)(ptr64 & 0xffffffff));
		facets[i].set32(1, (PfxUInt32)(ptr64 >> 32));
	}
	
	void *getPointer(PfxUInt32 i)
	{
		PfxUInt64 ptr32_0 = (PfxUInt64)facets[i].get32(0);
		PfxUInt64 ptr32_1 = (PfxUInt64)facets[i].get32(1);
		return (void*)uintptr_t((ptr32_1<<32)|ptr32_0);
	}
	
	PfxUInt32 size()
	{
		return facets.size();
	}
	
	void sort()
	{
		pfxSort(facets.ptr(),work.ptr(),facets.size());
	}
};

//static
//bool overlap(const PfxMcIslandBvhNode &nodeA,const PfxMcIslandBvhNode &nodeB)
//{
//	if(nodeA.aabbMax[0] < nodeB.aabbMin[0] || nodeA.aabbMin[0] > nodeB.aabbMax[0]) return false;
//	if(nodeA.aabbMax[1] < nodeB.aabbMin[1] || nodeA.aabbMin[1] > nodeB.aabbMax[1]) return false;
//	if(nodeA.aabbMax[2] < nodeB.aabbMin[2] || nodeA.aabbMin[2] > nodeB.aabbMax[2]) return false;
//	return true;
//}

#if 0 // unused function

static
bool intersect(const PfxMcFacet &facetA,const PfxMcFacet &facetB,PfxFloat &closestDistance)
{
	const PfxFloat epsilon = 0.00001f;

	PfxVector3 pA[3] = {
		facetA.v[0]->coord,
		facetA.v[1]->coord,
		facetA.v[2]->coord
	};
	
	PfxVector3 pB[3] = {
		facetB.v[0]->coord,
		facetB.v[1]->coord,
		facetB.v[2]->coord
	};

	// 面Bが面Aの厚みを考慮した範囲内に有るかどうかチェック

	// 上下面
	{
		PfxPlane planeA(facetA.normal,pA[0]);
		PfxFloat dmin = SCE_PFX_FLT_MAX;
		PfxFloat dmax = -SCE_PFX_FLT_MAX;
		for(int i=0;i<3;i++) {
			PfxFloat d = planeA.onPlane(pB[i]);
			dmin = SCE_PFX_MIN(dmin,d);
			dmax = SCE_PFX_MAX(dmax,d);
		}
		
		if(dmin > -epsilon || dmax < -facetA.thickness) return false;
		
		// 面Aと面Bの最近接距離
		if(dmax > 0.0f) {
			// 面A,Bは交差
			return false;
		}
//		else if(dmax > -epsilon) {
//			// 隣接面
//			return false;
//		}
		else {
			closestDistance = -dmax;
		}
	}
	
	// 側面
	for(int p=0;p<3;p++) {
		PfxVector3 sideVec = normalize(cross((pA[(p+1)%3]-pA[p]),facetA.normal));
		PfxPlane planeA(sideVec,pA[p]);

		PfxFloat dmin = SCE_PFX_FLT_MAX;
		for(int i=0;i<3;i++) {
			PfxFloat d = planeA.onPlane(pB[i]);
			dmin = SCE_PFX_MIN(dmin,d);
		}
		
		if(dmin > -epsilon) return false;
	}
	
	return true;
}

#endif

static inline
PfxInt32 getFacetVertId(const PfxMcFacet &facet,PfxInt32 vertId)
{
	PfxInt32 vid = 0;
	if(vertId == facet.v[1]->vertId) {
		vid = 1;
	}
	else if(vertId == facet.v[2]->vertId) {
		vid = 2;
	}
	
	SCE_PFX_ASSERT(vertId == facet.v[vid]->vertId);
	
	return vid;
}

static
PfxInt32 divideMeshes(
	PfxUInt32 numFacetsLimit,
	PfxMcSortedFacetsList &sortedFacetsList,
	PfxMeshObject &meshObj,
	PfxMcIslandBvhNode &node)
{
	//J 含まれる面数が規定値以下であれば、そこで終了
	//E Process finishes if the number of faces are below the specified number
	if(node.numFacets <= numFacetsLimit) {
		node.islandId = meshObj.numIslands++;
		meshObj.islands[node.islandId] = &node;
		return SCE_PFX_OK;
	}
	
	//J さらに分割
	//E Subdivide
	PfxUInt32 nodeIdA = meshObj.numBvhNodes++;
	PfxUInt32 nodeIdB = meshObj.numBvhNodes++;
	
	PfxMcIslandBvhNode &nodeA = meshObj.bvh[nodeIdA];
	PfxMcIslandBvhNode &nodeB = meshObj.bvh[nodeIdB];
	
	nodeA.parent = &node;
	nodeB.parent = &node;
	node.left = &nodeA;
	node.right = &nodeB;
	
	{
		PfxBool ret = sortedFacetsList.initialize(node.numFacets);
		if(!ret) return SCE_PFX_ERR_OUT_OF_BUFFER;
		
		//J 最も適切と思われる分離軸を探す
		//E Find the most suitable separation axis
		int divAxis = 0;
		{
		#if 1
			PfxVector3 s(0.0f),s2(0.0f);
			for(PfxMcFacet *facet=node.facetList;facet!=NULL;facet=facet->next) {
				PfxVector3 center = (facet->aabbMax + facet->aabbMin) * 0.5f;
				s += center;
				s2 += mulPerElem(center,center);
			}
			PfxVector3 v = s2 - mulPerElem(s,s) / (float)node.numFacets;
			if(v[1] > v[0]) divAxis = 1;
			if(v[2] > v[divAxis]) divAxis = 2;
		#else
			PfxVector3 extent = node.aabbMax - node.aabbMin;
			if(extent[1] > extent[0]) divAxis = 1;
			if(extent[2] > extent[divAxis]) divAxis = 2;
		#endif
			PfxUInt32 f=0;
			for(PfxMcFacet *facet=node.facetList;facet!=NULL;facet=facet->next,f++) {
				PfxVector3 center = (facet->aabbMax + facet->aabbMin) * 0.5f;
				sortedFacetsList.set(f,center[divAxis],facet);
			}
		}
		
		//J 軸上でソート
		sortedFacetsList.sort();
		
		//J 新しいAABBに含まれる面をそれぞれの領域に分配
		PfxUInt32 i = 0;
		{
			PfxVector3 groupAabbMin(SCE_PFX_FLT_MAX);
			PfxVector3 groupAabbMax(-SCE_PFX_FLT_MAX);
			for(;i<node.numFacets/2;i++) {
				PfxMcFacet *facet = (PfxMcFacet*)sortedFacetsList.getPointer(i);
				groupAabbMin = minPerElem(groupAabbMin,facet->aabbMin);
				groupAabbMax = maxPerElem(groupAabbMax,facet->aabbMax);
				facet->next = nodeA.facetList;
				nodeA.facetList = facet;
				nodeA.numFacets++;
			}
			nodeA.aabbMin = groupAabbMin;
			nodeA.aabbMax = groupAabbMax;
		}
		
		{
			PfxVector3 groupAabbMin(SCE_PFX_FLT_MAX);
			PfxVector3 groupAabbMax(-SCE_PFX_FLT_MAX);
			for(;i<node.numFacets;i++) {
				PfxMcFacet *facet = (PfxMcFacet*)sortedFacetsList.getPointer(i);
				groupAabbMin = minPerElem(groupAabbMin,facet->aabbMin);
				groupAabbMax = maxPerElem(groupAabbMax,facet->aabbMax);
				facet->next = nodeB.facetList;
				nodeB.facetList = facet;
				nodeB.numFacets++;
			}
			nodeB.aabbMin = groupAabbMin;
			nodeB.aabbMax = groupAabbMax;
		}
	}
	
	node.numFacets = 0;
	node.facetList = NULL;
	
	PfxInt32 ret = SCE_PFX_OK;

	if(nodeA.numFacets < nodeB.numFacets) {
		if(nodeA.numFacets > 0)
			ret = divideMeshes(numFacetsLimit,sortedFacetsList,meshObj,nodeA);
		if(ret == SCE_PFX_OK && nodeB.numFacets > 0)
			ret = divideMeshes(numFacetsLimit,sortedFacetsList,meshObj,nodeB);
	}
	else {
		if(nodeB.numFacets > 0)
			ret = divideMeshes(numFacetsLimit,sortedFacetsList,meshObj,nodeB);
		if(ret == SCE_PFX_OK && nodeA.numFacets > 0)
			ret = divideMeshes(numFacetsLimit,sortedFacetsList,meshObj,nodeA);
	}
	
	return ret;
}

#ifdef SCE_PFX_DIVIDE_BY_CORRECT_PROB
SCE_PFX_FORCE_INLINE PfxFloat calculateSurfaceArea(const PfxVector3 &v)
{
#ifdef PFX_ENABLE_AVX
	__m128 t = pfxShufflePs<1, 2, 0, 3>( sce_vectormath_asm128( v.get128() ) );
	PfxVector3 shuffledV;
	shuffledV.set128(sce_vectormath_asfloat4(t));
	return sum(mulPerElem(v, shuffledV));
#else
	return v.getX() * v.getY() + v.getY() * v.getZ() + v.getZ() * v.getX();
#endif
}

SCE_PFX_FORCE_INLINE PfxFloat calculateVolume(const PfxVector3 &v)
{
	return v.getX() * v.getY() * v.getZ();
}

SCE_PFX_FORCE_INLINE PfxFloat calculateIntersectProb(const PfxMcIslandBvhNode &worldNode, const PfxVector3 &childMin, const PfxVector3 &childMax)
{
	PfxVector3 childSize = childMax - childMin;

	PfxFloat sa = (worldNode.surfaceArea < 0.00001f ? 0.0f : calculateSurfaceArea(childSize) / worldNode.surfaceArea);
	PfxFloat probHit = sa;

	PfxVector3 localMin = divPerElem(childMin - worldNode.aabbMin, worldNode.aabbSize);
	PfxVector3 localMax = divPerElem(worldNode.aabbMax - childMax, worldNode.aabbSize);

	PfxVector3 v1 = PfxVector3(1.0f) - mulPerElem(localMin, localMin) - mulPerElem(localMax, localMax);
	PfxFloat probOverlap = v1.getX() * v1.getY() * v1.getZ();

	//return probInside;
	//return probInside + (probOverlap - probInside) * probHit;
	return probOverlap * probHit;
	//return probHit;
	//return probOverlap;
}

static PfxInt32 divideMeshesByMinProb(
	PfxUInt32 numFacetsLimit,
	PfxMcSortedFacetsList &sortedFacetsList,
	PfxMeshObject &meshObj,
	PfxMcIslandBvhNode &node,
	const PfxMcIslandBvhNode &nodeRoot)
{
	//J 含まれる面数が規定値以下であれば、そこで終了
	//E Process finishes if the number of faces are below the specified number
	if (node.numFacets <= numFacetsLimit) {
		node.islandId = meshObj.numIslands++;
		meshObj.islands[node.islandId] = &node;
		return SCE_PFX_OK;
	}

	{
		PfxBool ret = sortedFacetsList.initialize(node.numFacets);
		if (!ret) return SCE_PFX_ERR_OUT_OF_BUFFER;
	}

	//J 最も適切と思われる分離軸を探す
	//E Find the most suitable separation axis
	int divAxis = 0;
	{
		PfxVector3 s(0.0f), s2(0.0f);
		for (PfxMcFacet *facet = node.facetList; facet != NULL; facet = facet->next) {
			PfxVector3 center = (facet->aabbMax + facet->aabbMin) * 0.5f;
			s += center;
			s2 += mulPerElem(center, center);
		}
		PfxVector3 v = s2 - mulPerElem(s, s) / (float)node.numFacets;
		if (v[1] > v[0]) divAxis = 1;
		if (v[2] > v[divAxis]) divAxis = 2;

		PfxUInt32 f = 0;
		for (PfxMcFacet *facet = node.facetList; facet != NULL; facet = facet->next, f++) {
			PfxVector3 center = (facet->aabbMax + facet->aabbMin) * 0.5f;
			sortedFacetsList.set(f, center[divAxis], facet);
		}
	}

	//J 軸上でソート
	sortedFacetsList.sort();

	PfxUInt32 nodeIdA = meshObj.numBvhNodes++;
	PfxUInt32 nodeIdB = meshObj.numBvhNodes++;

	PfxMcIslandBvhNode &nodeA = meshObj.bvh[nodeIdA];
	PfxMcIslandBvhNode &nodeB = meshObj.bvh[nodeIdB];

	nodeA.memo = nodeIdA;
	nodeB.memo = nodeIdB;

	nodeA.parent = nodeB.parent = &node;
	nodeA.left = nodeA.right = nodeB.left = nodeB.right = NULL;

	node.left = &nodeA;
	node.right = &nodeB;

	nodeA.facetList = nodeB.facetList = NULL;

	PfxUInt32 separateIndex;
	{
		PfxArray<PfxVector3> lMax(node.numFacets);
		PfxArray<PfxVector3> lMin(node.numFacets);
		PfxArray<PfxVector3> rMax(node.numFacets);
		PfxArray<PfxVector3> rMin(node.numFacets);
		if (lMax.ptr() == NULL || lMin.ptr() == NULL || rMax.ptr() == NULL || rMin.ptr() == NULL)
			return SCE_PFX_ERR_OUT_OF_BUFFER;

		lMin[0] = ((PfxMcFacet*)(sortedFacetsList.getPointer(0)))->aabbMin;
		lMax[0] = ((PfxMcFacet*)(sortedFacetsList.getPointer(0)))->aabbMax;
		rMin[node.numFacets - 1] = ((PfxMcFacet*)(sortedFacetsList.getPointer(node.numFacets - 1)))->aabbMin;
		rMax[node.numFacets - 1] = ((PfxMcFacet*)(sortedFacetsList.getPointer(node.numFacets - 1)))->aabbMax;

		for (PfxUInt32 li = 1, ri = node.numFacets - 2; li < node.numFacets; ++li, --ri) {
			lMin[li] = minPerElem(lMin[li - 1], ((PfxMcFacet*)(sortedFacetsList.getPointer(li)))->aabbMin);
			lMax[li] = maxPerElem(lMax[li - 1], ((PfxMcFacet*)(sortedFacetsList.getPointer(li)))->aabbMax);

			rMin[ri] = minPerElem(rMin[ri + 1], ((PfxMcFacet*)(sortedFacetsList.getPointer(ri)))->aabbMin);
			rMax[ri] = maxPerElem(rMax[ri + 1], ((PfxMcFacet*)(sortedFacetsList.getPointer(ri)))->aabbMax);
		}

		separateIndex = 1;
		PfxFloat minExpt = SCE_PFX_FLT_MAX;
		for (PfxUInt32 i = 1; i < node.numFacets; ++i) {
			PfxFloat lProb = calculateIntersectProb(nodeRoot, lMin[i - 1], lMax[i - 1]);
			PfxFloat rProb = calculateIntersectProb(nodeRoot, rMin[i], rMax[i]);

			PfxFloat lExpt = lProb * PfxFloat(i);
			PfxFloat rExpt = rProb * PfxFloat(node.numFacets - i);

			if (lExpt + rExpt < minExpt) {
				separateIndex = i;
				minExpt = lExpt + rExpt;
			}
		}

		nodeA.aabbMin = lMin[separateIndex - 1];
		nodeA.aabbMax = lMax[separateIndex - 1];
		nodeA.aabbSize = nodeA.aabbMax - nodeA.aabbMin;
		nodeA.surfaceArea = calculateSurfaceArea(nodeA.aabbSize);
		nodeA.volume = calculateVolume(nodeA.aabbSize);

		nodeB.aabbMin = rMin[separateIndex];
		nodeB.aabbMax = rMax[separateIndex];
		nodeB.aabbSize = nodeB.aabbMax - nodeB.aabbMin;
		nodeB.surfaceArea = calculateSurfaceArea(nodeB.aabbSize);
		nodeB.volume = calculateVolume(nodeB.aabbSize);

		nodeA.numFacets = separateIndex;
		nodeB.numFacets = node.numFacets - separateIndex;

		for (PfxUInt32 i = 0; i < separateIndex; ++i) {
			PfxMcFacet *facet = (PfxMcFacet*)sortedFacetsList.getPointer(i);
			facet->next = nodeA.facetList;
			nodeA.facetList = facet;
		}

		for (PfxUInt32 i = separateIndex; i < node.numFacets; ++i) {
			PfxMcFacet *facet = (PfxMcFacet*)sortedFacetsList.getPointer(i);
			facet->next = nodeB.facetList;
			nodeB.facetList = facet;
		}
	}

	//	SCE_PFX_PRINTF("Separating node[%4d] at %4d (%4d/%4d/%4d)\n", node.memo, separateIndex, nodeA.numFacets, nodeB.numFacets, node.numFacets);

	PfxInt32 ret = SCE_PFX_OK;
	if (nodeA.numFacets < nodeB.numFacets) {
		if (nodeA.numFacets > 0)
			ret = divideMeshesByMinProb(numFacetsLimit, sortedFacetsList, meshObj, nodeA, nodeRoot);
		if (ret == SCE_PFX_OK && nodeB.numFacets > 0)
			ret = divideMeshesByMinProb(numFacetsLimit, sortedFacetsList, meshObj, nodeB, nodeRoot);
	}
	else {
		if (nodeB.numFacets > 0)
			ret = divideMeshesByMinProb(numFacetsLimit, sortedFacetsList, meshObj, nodeB, nodeRoot);
		if (ret == SCE_PFX_OK && nodeA.numFacets > 0)
			ret = divideMeshesByMinProb(numFacetsLimit, sortedFacetsList, meshObj, nodeA, nodeRoot);
	}

	return ret;
}
#elif defined(SCE_PFX_DIVIDE_BY_VOLUME)
static PfxInt32 divideMeshesByMinVolume(
	PfxUInt32 numFacetsLimit,
	PfxMcSortedFacetsList &sortedFacetsList,
	PfxMeshObject &meshObj,
	PfxMcIslandBvhNode &node,
	const PfxVector3 &nodeSizeEpsilone)
{
	//J 含まれる面数が規定値以下であれば、そこで終了
	//E Process finishes if the number of faces are below the specified number
	if (node.numFacets <= numFacetsLimit) {
		node.islandId = meshObj.numIslands++;
		meshObj.islands[node.islandId] = &node;
		return SCE_PFX_OK;
	}

	{
		PfxBool ret = sortedFacetsList.initialize(node.numFacets);
		if (!ret) return SCE_PFX_ERR_OUT_OF_BUFFER;
	}

	//J 最も適切と思われる分離軸を探す
	//E Find the most suitable separation axis
	int divAxis = 0;
	{
		PfxVector3 s(0.0f), s2(0.0f);
		for (PfxMcFacet *facet = node.facetList; facet != NULL; facet = facet->next) {
			PfxVector3 center = (facet->aabbMax + facet->aabbMin) * 0.5f;
			s += center;
			s2 += mulPerElem(center, center);
		}
		PfxVector3 v = s2 - mulPerElem(s, s) / (float)node.numFacets;
		if (v[1] > v[0]) divAxis = 1;
		if (v[2] > v[divAxis]) divAxis = 2;

		PfxUInt32 f = 0;
		for (PfxMcFacet *facet = node.facetList; facet != NULL; facet = facet->next, f++) {
			PfxVector3 center = (facet->aabbMax + facet->aabbMin) * 0.5f;
			sortedFacetsList.set(f, center[divAxis], facet);
		}
	}

	//J 軸上でソート
	sortedFacetsList.sort();

	PfxUInt32 nodeIdA = meshObj.numBvhNodes++;
	PfxUInt32 nodeIdB = meshObj.numBvhNodes++;

	PfxMcIslandBvhNode &nodeA = meshObj.bvh[nodeIdA];
	PfxMcIslandBvhNode &nodeB = meshObj.bvh[nodeIdB];

	nodeA.memo = nodeIdA;
	nodeB.memo = nodeIdB;

	nodeA.parent = nodeB.parent = &node;
	nodeA.left = nodeA.right = nodeB.left = nodeB.right = NULL;

	node.left = &nodeA;
	node.right = &nodeB;

	nodeA.facetList = nodeB.facetList = NULL;

	PfxUInt32 separateIndex;
	{
		PfxArray<PfxVector3> lMax(node.numFacets);
		PfxArray<PfxVector3> lMin(node.numFacets);
		PfxArray<PfxVector3> rMax(node.numFacets);
		PfxArray<PfxVector3> rMin(node.numFacets);
		if (lMax.ptr() == NULL || lMin.ptr() == NULL || rMax.ptr() == NULL || rMin.ptr() == NULL)
			return SCE_PFX_ERR_OUT_OF_BUFFER;

		lMin[0] = ((PfxMcFacet*)(sortedFacetsList.getPointer(0)))->aabbMin;
		lMax[0] = ((PfxMcFacet*)(sortedFacetsList.getPointer(0)))->aabbMax;
		rMin[node.numFacets - 1] = ((PfxMcFacet*)(sortedFacetsList.getPointer(node.numFacets - 1)))->aabbMin;
		rMax[node.numFacets - 1] = ((PfxMcFacet*)(sortedFacetsList.getPointer(node.numFacets - 1)))->aabbMax;

		for (PfxUInt32 li = 1, ri = node.numFacets - 2; li < node.numFacets; ++li, --ri) {
			lMin[li] = minPerElem(lMin[li - 1], ((PfxMcFacet*)(sortedFacetsList.getPointer(li)))->aabbMin);
			lMax[li] = maxPerElem(lMax[li - 1], ((PfxMcFacet*)(sortedFacetsList.getPointer(li)))->aabbMax);

			rMin[ri] = minPerElem(rMin[ri + 1], ((PfxMcFacet*)(sortedFacetsList.getPointer(ri)))->aabbMin);
			rMax[ri] = maxPerElem(rMax[ri + 1], ((PfxMcFacet*)(sortedFacetsList.getPointer(ri)))->aabbMax);
		}

		separateIndex = 1;
		PfxFloat minProb = SCE_PFX_FLT_MAX;
		for (PfxUInt32 i = 1; i < node.numFacets; ++i) {
			PfxVector3 lSize = maxPerElem(nodeSizeEpsilone, lMax[i - 1] - lMin[i - 1]);
			PfxVector3 rSize = maxPerElem(nodeSizeEpsilone, rMax[i] - rMin[i]);

			PfxFloat lVolume = lSize.getX() * lSize.getY() * lSize.getZ();
			PfxFloat rVolume = rSize.getX() * rSize.getY() * rSize.getZ();

			PfxFloat lProb = lVolume * PfxFloat(i);
			PfxFloat rProb = rVolume * PfxFloat(node.numFacets - i);

			if (lProb + rProb < minProb) {
				separateIndex = i;
				minProb = lProb + rProb;
			}
		}

		nodeA.aabbMin = lMin[separateIndex - 1];
		nodeA.aabbMax = lMax[separateIndex - 1];

		nodeB.aabbMin = rMin[separateIndex];
		nodeB.aabbMax = rMax[separateIndex];

		nodeA.numFacets = separateIndex;
		nodeB.numFacets = node.numFacets - separateIndex;

		for (PfxUInt32 i = 0; i < separateIndex; ++i) {
			PfxMcFacet *facet = (PfxMcFacet*)sortedFacetsList.getPointer(i);
			facet->next = nodeA.facetList;
			nodeA.facetList = facet;
		}

		for (PfxUInt32 i = separateIndex; i < node.numFacets; ++i) {
			PfxMcFacet *facet = (PfxMcFacet*)sortedFacetsList.getPointer(i);
			facet->next = nodeB.facetList;
			nodeB.facetList = facet;
		}
	}

	//	SCE_PFX_PRINTF("Separating node[%4d] at %4d (%4d/%4d/%4d)\n", node.memo, separateIndex, nodeA.numFacets, nodeB.numFacets, node.numFacets);

	PfxInt32 ret = SCE_PFX_OK;
	if (nodeA.numFacets < nodeB.numFacets) {
		if (nodeA.numFacets > 0)
			ret = divideMeshesByMinVolume(numFacetsLimit, sortedFacetsList, meshObj, nodeA, nodeSizeEpsilone);
		if (ret == SCE_PFX_OK && nodeB.numFacets > 0)
			ret = divideMeshesByMinVolume(numFacetsLimit, sortedFacetsList, meshObj, nodeB, nodeSizeEpsilone);
	}
	else {
		if (nodeB.numFacets > 0)
			ret = divideMeshesByMinVolume(numFacetsLimit, sortedFacetsList, meshObj, nodeB, nodeSizeEpsilone);
		if (ret == SCE_PFX_OK && nodeA.numFacets > 0)
			ret = divideMeshesByMinVolume(numFacetsLimit, sortedFacetsList, meshObj, nodeA, nodeSizeEpsilone);
	}

	return ret;
}
#else
static PfxInt32 divideMeshesByMinSurfaceArea(
	PfxUInt32 numFacetsLimit,
	PfxMcSortedFacetsList &sortedFacetsList,
	PfxMeshObject &meshObj,
	PfxMcIslandBvhNode &node)
{
	//J 含まれる面数が規定値以下であれば、そこで終了
	//E Process finishes if the number of faces are below the specified number
	if (node.numFacets <= numFacetsLimit) {
		node.islandId = meshObj.numIslands++;
		meshObj.islands[node.islandId] = &node;
		return SCE_PFX_OK;
	}

	{
		PfxBool ret = sortedFacetsList.initialize(node.numFacets);
		if (!ret) return SCE_PFX_ERR_OUT_OF_BUFFER;
	}

	//J 最も適切と思われる分離軸を探す
	//E Find the most suitable separation axis
	int divAxis = 0;
	{
		PfxVector3 s(0.0f), s2(0.0f);
		for (PfxMcFacet *facet = node.facetList; facet != NULL; facet = facet->next) {
			PfxVector3 center = (facet->aabbMax + facet->aabbMin) * 0.5f;
			s += center;
			s2 += mulPerElem(center, center);
		}
		PfxVector3 v = s2 - mulPerElem(s, s) / (float)node.numFacets;
		if (v[1] > v[0]) divAxis = 1;
		if (v[2] > v[divAxis]) divAxis = 2;

		PfxUInt32 f = 0;
		for (PfxMcFacet *facet = node.facetList; facet != NULL; facet = facet->next, f++) {
			PfxVector3 center = (facet->aabbMax + facet->aabbMin) * 0.5f;
			sortedFacetsList.set(f, center[divAxis], facet);
		}
	}

	//J 軸上でソート
	sortedFacetsList.sort();/**/

	PfxUInt32 nodeIdA = meshObj.numBvhNodes++;
	PfxUInt32 nodeIdB = meshObj.numBvhNodes++;

	PfxMcIslandBvhNode &nodeA = meshObj.bvh[nodeIdA];
	PfxMcIslandBvhNode &nodeB = meshObj.bvh[nodeIdB];

	nodeA.memo = nodeIdA;
	nodeB.memo = nodeIdB;

	nodeA.parent = nodeB.parent = &node;
	nodeA.left = nodeA.right = nodeB.left = nodeB.right = NULL;

	node.left = &nodeA;
	node.right = &nodeB;

	nodeA.facetList = nodeB.facetList = NULL;

	PfxUInt32 separateIndex;
	{
		PfxArray<PfxVector3> lMax(node.numFacets);
		PfxArray<PfxVector3> lMin(node.numFacets);
		PfxArray<PfxVector3> rMax(node.numFacets);
		PfxArray<PfxVector3> rMin(node.numFacets);
		if (lMax.ptr() == NULL || lMin.ptr() == NULL || rMax.ptr() == NULL || rMin.ptr() == NULL)
			return SCE_PFX_ERR_OUT_OF_BUFFER;

		lMin[0] = ((PfxMcFacet*)(sortedFacetsList.getPointer(0)))->aabbMin;
		lMax[0] = ((PfxMcFacet*)(sortedFacetsList.getPointer(0)))->aabbMax;
		rMin[node.numFacets - 1] = ((PfxMcFacet*)(sortedFacetsList.getPointer(node.numFacets - 1)))->aabbMin;
		rMax[node.numFacets - 1] = ((PfxMcFacet*)(sortedFacetsList.getPointer(node.numFacets - 1)))->aabbMax;

		for (PfxUInt32 li = 1, ri = node.numFacets - 2; li < node.numFacets; ++li, --ri) {
			lMin[li] = minPerElem(lMin[li - 1], ((PfxMcFacet*)(sortedFacetsList.getPointer(li)))->aabbMin);
			lMax[li] = maxPerElem(lMax[li - 1], ((PfxMcFacet*)(sortedFacetsList.getPointer(li)))->aabbMax);

			rMin[ri] = minPerElem(rMin[ri + 1], ((PfxMcFacet*)(sortedFacetsList.getPointer(ri)))->aabbMin);
			rMax[ri] = maxPerElem(rMax[ri + 1], ((PfxMcFacet*)(sortedFacetsList.getPointer(ri)))->aabbMax);
		}

		separateIndex = 1;
		PfxFloat minProb = SCE_PFX_FLT_MAX;
		for (PfxUInt32 i = 1; i < node.numFacets; ++i) {
			PfxVector3 lSize = lMax[i - 1] - lMin[i - 1];
			PfxVector3 rSize = rMax[i] - rMin[i];

#ifdef PFX_ENABLE_AVX
			PfxVector3 shuffledLSize( pfxShufflePs<1, 2, 0, 3>( sce_vectormath_asm128( lSize.get128() ) ) );
			PfxVector3 shuffledRSize( pfxShufflePs<1, 2, 0, 3>( sce_vectormath_asm128( rSize.get128() ) ) );

			PfxFloat lSurfaceArea = sum(mulPerElem(lSize, shuffledLSize));
			PfxFloat rSurfaceArea = sum(mulPerElem(rSize, shuffledRSize));
#else
			PfxFloat lSurfaceArea2 = lSize.getX() * lSize.getY() + lSize.getY() * lSize.getZ() + lSize.getZ() * lSize.getX();
			PfxFloat rSurfaceArea2 = rSize.getX() * rSize.getY() + rSize.getY() * rSize.getZ() + rSize.getZ() * rSize.getX();
#endif

			PfxFloat lProb = lSurfaceArea * PfxFloat(i);
			PfxFloat rProb = rSurfaceArea * PfxFloat(node.numFacets - i);/**/

			if (lProb + rProb < minProb) {
				separateIndex = i;
				minProb = lProb + rProb;
			}
		}

		nodeA.aabbMin = lMin[separateIndex - 1];
		nodeA.aabbMax = lMax[separateIndex - 1];

		nodeB.aabbMin = rMin[separateIndex];
		nodeB.aabbMax = rMax[separateIndex];

		nodeA.numFacets = separateIndex;
		nodeB.numFacets = node.numFacets - separateIndex;

		for (PfxUInt32 i = 0; i < separateIndex; ++i) {
			PfxMcFacet *facet = (PfxMcFacet*)sortedFacetsList.getPointer(i);
			facet->next = nodeA.facetList;
			nodeA.facetList = facet;
		}

		for (PfxUInt32 i = separateIndex; i < node.numFacets; ++i) {
			PfxMcFacet *facet = (PfxMcFacet*)sortedFacetsList.getPointer(i);
			facet->next = nodeB.facetList;
			nodeB.facetList = facet;
		}
	}

	//	SCE_PFX_PRINTF("Separating node[%4d] at %4d (%4d/%4d/%4d)\n", node.memo, separateIndex, nodeA.numFacets, nodeB.numFacets, node.numFacets);

	PfxInt32 ret = SCE_PFX_OK;
	if (nodeA.numFacets < nodeB.numFacets) {
		if (nodeA.numFacets > 0)
			ret = divideMeshesByMinSurfaceArea(numFacetsLimit, sortedFacetsList, meshObj, nodeA);
		if (ret == SCE_PFX_OK && nodeB.numFacets > 0)
			ret = divideMeshesByMinSurfaceArea(numFacetsLimit, sortedFacetsList, meshObj, nodeB);
	}
	else {
		if (nodeB.numFacets > 0)
			ret = divideMeshesByMinSurfaceArea(numFacetsLimit, sortedFacetsList, meshObj, nodeB);
		if (ret == SCE_PFX_OK && nodeA.numFacets > 0)
			ret = divideMeshesByMinSurfaceArea(numFacetsLimit, sortedFacetsList, meshObj, nodeA);
	}/**/

	return ret;
}
#endif

static
void calculateVertexAndEdgeNormals(PfxMeshObject &meshObj)
{
	// エッジの角度と法線を計算
	for(PfxUInt32 i=0;i<meshObj.edgeList.size();i++) {
		PfxMcEdge &edge = meshObj.edgeList[i];
		SCE_PFX_ASSERT(edge.numFacets==1||edge.numFacets==2);
		
		if(edge.numFacets == 1) {
			SCE_PFX_ASSERT(edge.edgeId[0]<3);
			PfxMcFacet &facetA = meshObj.facetList[edge.facetId[0]];
			edge.normal = normalize(cross(facetA.v[(edge.edgeId[0]+1)%3]->coord - facetA.v[edge.edgeId[0]]->coord,facetA.normal));
		}
		else if(edge.numFacets == 2) {
			PfxMcFacet &facetA = meshObj.facetList[edge.facetId[0]];
			PfxMcFacet &facetB = meshObj.facetList[edge.facetId[1]];

			#if 1 // 精度向上のためエッジを法線とする平面上でチェック
			PfxVector3 edgeP1 = facetA.v[edge.edgeId[0]]->coord;
			PfxVector3 edgeP2 = facetA.v[(edge.edgeId[0]+1)%3]->coord;
			PfxVector3 edgeVec = normalize(edgeP2-edgeP1);
			
			PfxVector3 centerA = (facetA.v[0]->coord + facetA.v[1]->coord + facetA.v[2]->coord) / 3.0f;
			centerA -= dot(centerA - edgeP1,edgeVec) * edgeVec;
			
			PfxVector3 centerB = (facetB.v[0]->coord + facetB.v[1]->coord + facetB.v[2]->coord) / 3.0f;
			centerB -= dot(centerB - edgeP1,edgeVec) * edgeVec;
			#else
			// 面の中心
			PfxVector3 centerA = (facetA.v[0]->coord + facetA.v[1]->coord + facetA.v[2]->coord) / 3.0f;
			PfxVector3 centerB = (facetB.v[0]->coord + facetB.v[1]->coord + facetB.v[2]->coord) / 3.0f;
			#endif
			
			// エッジの角度判定
			PfxFloat chk1 = dot(facetA.normal,(centerA + centerB) * 0.5f - centerA);
			PfxFloat chk2 = dot(facetB.normal,(centerA + centerB) * 0.5f - centerB);
			PfxFloat chk3 = dot(facetA.normal,facetB.normal);
			
			edge.angleType = SCE_PFX_EDGE_FLAT;
			edge.angle = 0.5f*acosf(SCE_PFX_CLAMP(chk3,-1.0f,1.0f));
			edge.normal = normalize(facetA.normal + facetB.normal);
			
			if(chk1 < -epsilon && chk2 < -epsilon) {
				edge.angleType = SCE_PFX_EDGE_CONVEX;
			}
			else if(chk1 > epsilon && chk2 > epsilon) {
				edge.angleType = SCE_PFX_EDGE_CONCAVE;
			}
		}
	}

	// 頂点の法線を計算
	for(PfxUInt32 i=0;i<meshObj.vertList.size();i++) {
		if(meshObj.vertList[i].obsolete || !meshObj.triHead[i]) continue;
		
		// Calculate the angle weighted normal
		PfxVector3 awnormal(0.0f);
		if(meshObj.triHead[i]->next) {
			for(PfxMcVert2TriList *f=meshObj.triHead[i];f!=NULL;f=f->next) {
				PfxMcFacet *facet = f->facet;

				int vid = 0;
				if(i == facet->v[1]->vertId) {
					vid = 1;
				}
				else if(i == facet->v[2]->vertId) {
					vid = 2;
				}
				
				PfxFloat dval = dot(
					normalize(facet->v[(vid+1)%3]->coord - facet->v[vid]->coord),
					normalize(facet->v[(vid+2)%3]->coord - facet->v[vid]->coord));
				PfxFloat angle = acosf(SCE_PFX_CLAMP(dval,-1.0f,1.0f));
				
				awnormal += angle * facet->normal;
			}
			meshObj.vertList[i].normal = normalize(awnormal);
		}
		else {
			PfxMcFacet *facet = meshObj.triHead[i]->facet;
			int vid = getFacetVertId(*facet,i);
			
			PfxVector3 center = (facet->v[0]->coord + facet->v[1]->coord + facet->v[2]->coord) / 3.0f;
			
			meshObj.vertList[i].normal = normalize(facet->v[vid]->coord - center);
		}
	}
}

struct BvhNodePair {
	PfxMcIslandBvhNode *nodeA,*nodeB;
	BvhNodePair() {}
	BvhNodePair(PfxMcIslandBvhNode *nA,PfxMcIslandBvhNode *nB) : nodeA(nA),nodeB(nB) {}
};

static
void calculateFacetAabb(PfxMeshObject &meshObj)
{
	PfxVector3 totalAabbMin(SCE_PFX_FLT_MAX);
	PfxVector3 totalAabbMax(-SCE_PFX_FLT_MAX);
	
	// 不正な角度をチェック
	// 表裏一体化した面の場合、厚みが不正になる
	for(PfxUInt32 i=0;i<meshObj.edgeList.size();i++) {
		PfxMcEdge &edge = meshObj.edgeList[i];
		
		if(edge.angle > 1.57f) {
			edge.angle = 0.0f;
		
			if(edge.numFacets == 1) {
				PfxMcFacet &facetA = meshObj.facetList[edge.facetId[0]];
				facetA.thickness = 0.0f;
			}
			else if(edge.numFacets == 2) {
				PfxMcFacet &facetA = meshObj.facetList[edge.facetId[0]];
				PfxMcFacet &facetB = meshObj.facetList[edge.facetId[1]];
				facetA.thickness = 0.0f;
				facetB.thickness = 0.0f;
			}
		}
	}
	
	for(PfxUInt32 f=0;f<meshObj.facetList.size();f++) {
		PfxMcFacet &facet = meshObj.facetList[f];
		if(facet.obsolete) continue;
		
		PfxVector3 pnts[6] = {
			facet.v[0]->coord,
			facet.v[1]->coord,
			facet.v[2]->coord,
			facet.v[0]->coord - facet.thickness * facet.normal,
			facet.v[1]->coord - facet.thickness * facet.normal,
			facet.v[2]->coord - facet.thickness * facet.normal,
		};
		
		/* SCE_PFX_NO_OSS_PART_BEGIN */
		if(facet.e[0]->isAcuteAngle() ||facet.e[1]->isAcuteAngle() || facet.e[2]->isAcuteAngle()) {
			const PfxVector3 normal = facet.normal;
			PfxVector3 sideNml[3] = {
				normalize(cross((pnts[1] - pnts[0]),normal)),
				normalize(cross((pnts[2] - pnts[1]),normal)),
				normalize(cross((pnts[0] - pnts[2]),normal)),
			};
			sideNml[0] = cosf(facet.e[0]->angle)*sideNml[0] - sinf(facet.e[0]->angle)*normal;
			sideNml[1] = cosf(facet.e[1]->angle)*sideNml[1] - sinf(facet.e[1]->angle)*normal;
			sideNml[2] = cosf(facet.e[2]->angle)*sideNml[2] - sinf(facet.e[2]->angle)*normal;
			PfxMatrix3 mtx(0.0f);
			PfxVector3 vec(0.0f);
			mtx.setRow(0,sideNml[0]);
			mtx.setRow(1,sideNml[1]);
			mtx.setRow(2,sideNml[2]);
			vec[0] = dot(pnts[0],sideNml[0]);
			vec[1] = dot(pnts[1],sideNml[1]);
			vec[2] = dot(pnts[2],sideNml[2]);
			PfxVector3 intersection = inverse(mtx) * vec; // 3平面の交点
			PfxFloat intersectionDist = -dot(intersection-facet.v[0]->coord,facet.normal);
			if(facet.thickness < intersectionDist) {
				PfxFloat t = facet.thickness / intersectionDist;
				pnts[3] = pnts[0]+t*(intersection-pnts[0]);
				pnts[4] = pnts[1]+t*(intersection-pnts[1]);
				pnts[5] = pnts[2]+t*(intersection-pnts[2]);
			}
			else {
				pnts[3] = intersection;
				pnts[4] = intersection;
				pnts[5] = intersection;
			}
		}
		/* SCE_PFX_NO_OSS_PART_END */

		// 面のAABBを算出
		meshObj.facetList[f].aabbMin = minPerElem(pnts[5],minPerElem(pnts[4],minPerElem(pnts[3],minPerElem(pnts[2],minPerElem(pnts[1],pnts[0])))));
		meshObj.facetList[f].aabbMax = maxPerElem(pnts[5],maxPerElem(pnts[4],maxPerElem(pnts[3],maxPerElem(pnts[2],maxPerElem(pnts[1],pnts[0])))));
	}
}

static
void calculateAttributes(PfxMeshObject &meshObj,const PfxCreateLargeTriMeshParam &param)
{
	// エッジの角度と頂点の法線を計算
	calculateVertexAndEdgeNormals(meshObj);
	
	// 面のAABBを計算
	calculateFacetAabb(meshObj);
}

static
PfxInt32 buildBvhParts(PfxMeshObject &meshObj,const PfxCreateLargeTriMeshParam &param)
{
	PfxInt32 ret = SCE_PFX_OK;
	
	// アイランド数概算（パーツ分解する際は概算できない…）
	//PfxUInt32 nIslands = SCE_PFX_MAX(1,meshObj.facetList.size() / SCE_PFX_MAX(1,param.numFacetsLimit/2));
	PfxUInt32 nIslands = SCE_PFX_MAX(1,meshObj.facetList.size());
	
	// バッファの確保
	meshObj.bvh.assign(nIslands*2-1,PfxMcIslandBvhNode());
	meshObj.islands.assign(nIslands,NULL);
	meshObj.roots.assign(nIslands,NULL);
	
	PfxMcSortedFacetsList sortedFacetsList;
	sortedFacetsList.initialize(meshObj.facetList.size());
	
	PfxUInt8 *facetInfo = (PfxUInt8*)SCE_PFX_UTIL_ALLOC(16,sizeof(PfxUInt8)*meshObj.facetList.size());
	memset(facetInfo,0,sizeof(PfxUInt8)*meshObj.facetList.size());
	
	meshObj.numIslands = 0;
	meshObj.numBvhNodes = 0;
	meshObj.numRoots = 0;

	int count = 0;
	int maxCount = 0;
	for(PfxUInt32 f=0;f<meshObj.facetList.size();f++) {
		if(!meshObj.facetList[f].obsolete) maxCount++;
	}

	PfxStack<PfxMcFacet*> stk(meshObj.facetList.size());

	while(count < maxCount) {
		// ルートノードを作る
		PfxMcIslandBvhNode &root = meshObj.bvh[meshObj.numBvhNodes++];
		
		root.aabbMin = meshObj.totalAabbMin;
		root.aabbMax = meshObj.totalAabbMax;
#ifdef SCE_PFX_DIVIDE_BY_CORRECT_PROB
		root.aabbSize = root.aabbMax - root.aabbMin;
		root.surfaceArea = calculateSurfaceArea(root.aabbSize);
		root.volume = calculateVolume(root.aabbSize);
#endif
		
		// 連続する面を集める
		stk.clear();
		for(PfxUInt32 f=0;f<meshObj.facetList.size();f++) {
			if(meshObj.facetList[f].obsolete) continue;
			if(facetInfo[f] != 0) continue;
			stk.push(&meshObj.facetList[f]);
			facetInfo[f]= 1;
			count++;
			break;
		}
		
		while(!stk.empty()) {
			PfxMcFacet *facet = stk.top();
			stk.pop();
			
			facet->next = root.facetList;
			root.facetList = facet;
			root.numFacets++;

			for(int i=0;i<3;i++) {
				int fid = facet->neighbor[i];
				if(fid > -1 && facetInfo[fid] == 0) {
					stk.push(&meshObj.facetList[fid]);
					facetInfo[fid] = 1;
					count++;
				}
			}
		}
	
		// BVHの構築
		if ((param.flag & SCE_PFX_MESH_FLAG_OPTIMIZE_BVH_STRUCTURE) == SCE_PFX_MESH_FLAG_OPTIMIZE_BVH_STRUCTURE) {
#ifdef SCE_PFX_DIVIDE_BY_CORRECT_PROB
			ret = divideMeshesByMinProb(param.numFacetsLimit, sortedFacetsList, meshObj, root, root);
#elif defined(SCE_PFX_DIVIDE_BY_VOLUME)
			const PfxVector3 nodeSizeEpsilon(SCE_PFX_MIN(maxElem(root.aabbMax - root.aabbMin) * 0.00001f, (0.00001f)));
			ret = divideMeshesByMinVolume(param.numFacetsLimit, sortedFacetsList, meshObj, root, nodeSizeEpsilon);
#else
			ret = divideMeshesByMinSurfaceArea(param.numFacetsLimit, sortedFacetsList, meshObj, root);
#endif
		}
		else 
			ret = divideMeshes(param.numFacetsLimit,sortedFacetsList,meshObj,root);
		
		meshObj.roots[meshObj.numRoots++] = &root;
	}
	
	SCE_PFX_UTIL_FREE(facetInfo);
	
	return ret;
}

static
PfxInt32 buildBvh(PfxMeshObject &meshObj,const PfxCreateLargeTriMeshParam &param)
{
	// アイランド数概算
	PfxUInt32 nIslands = SCE_PFX_MAX(1,meshObj.facetList.size());
	
	// バッファの確保
	meshObj.bvh.assign(nIslands*2-1,PfxMcIslandBvhNode());
	meshObj.islands.assign(nIslands,NULL);
	
	PfxMcSortedFacetsList sortedFacetsList;
	sortedFacetsList.initialize(meshObj.facetList.size());
	
	// ルートノードを作る
	PfxMcIslandBvhNode &root = meshObj.bvh[0];
	
	root.aabbMin = meshObj.totalAabbMin;
	root.aabbMax = meshObj.totalAabbMax;
#ifdef SCE_PFX_DIVIDE_BY_CORRECT_PROB
	root.aabbSize = root.aabbMax - root.aabbMin;
	root.surfaceArea = calculateSurfaceArea(root.aabbSize);
	root.volume = calculateVolume(root.aabbSize);
#endif
	
	for(PfxUInt32 f=0;f<meshObj.facetList.size();f++) {
		PfxMcFacet &facet = meshObj.facetList[f];
		if(facet.obsolete) continue;
		
		facet.next = root.facetList;
		root.facetList = &facet;
		root.numFacets++;
	}
	
	meshObj.numBvhNodes = 1;
	meshObj.numIslands = 0;
	meshObj.numRoots = 1;
	meshObj.roots.push(&root);
	
	if ((param.flag & SCE_PFX_MESH_FLAG_OPTIMIZE_BVH_STRUCTURE) == SCE_PFX_MESH_FLAG_OPTIMIZE_BVH_STRUCTURE) {
#ifdef SCE_PFX_DIVIDE_BY_CORRECT_PROB
		return divideMeshesByMinProb(param.numFacetsLimit, sortedFacetsList, meshObj, root, root);
#elif defined(SCE_PFX_DIVIDE_BY_VOLUME)
		const PfxVector3 nodeSizeEpsilon(SCE_PFX_MIN(maxElem(root.aabbMax - root.aabbMin) * 0.00001f, (0.00001f)));
		return divideMeshesByMinVolume(param.numFacetsLimit, sortedFacetsList, meshObj, root, nodeSizeEpsilon);
#else
		return divideMeshesByMinSurfaceArea(param.numFacetsLimit, sortedFacetsList, meshObj, root);
#endif
	}
	else 
		return divideMeshes(param.numFacetsLimit,sortedFacetsList,meshObj,root);
}

PfxInt32 pfxMeshObjectBuild(PfxMeshObject &meshObj,const PfxCreateLargeTriMeshParam &param,PfxBool enableMultiParts)
{
	const PfxFloat epsilon = 1e-8f;

	PfxUInt32 edgeWarning = 0;
	
	meshObj.numBvhNodes = 0;
	meshObj.numIslands = 0;

	//J 空バッファの確保
	meshObj.edgeList.assign(param.numTriangles*3,PfxMcEdge());
	meshObj.edgeHead.assign(param.numTriangles*3,NULL);
	meshObj.triEntry.assign(param.numTriangles*3,PfxMcVert2TriList());
	meshObj.triHead.assign(param.numVerts,NULL);
	
	// Temporal buffers
	PfxArray<PfxUInt32> idxList;
	PfxArray<PfxUInt32> vtxGroup;
	idxList.assign( param.numTriangles * 3, 0 );
	vtxGroup.assign( param.numVerts, 0 );

	if(!(meshObj.vertList.ptr() && meshObj.facetList.ptr() && 
		meshObj.edgeList.ptr() && meshObj.edgeHead.ptr() &&
		meshObj.triEntry.ptr()&&meshObj.triHead.ptr())) {
		return SCE_PFX_ERR_OUT_OF_BUFFER;
	}
	
	//J 頂点配列作成
	//E Create vertex array
	for(PfxUInt32 i=0;i<param.numVerts;i++) {
		PfxFloat *vtx = (PfxFloat*)((uintptr_t)param.verts + param.vertexStrideBytes * i);
		PfxMcVert mcv = {};
		mcv.obsolete = false;
		mcv.localId = 0;
		mcv.vertId = i;
		mcv.coord = pfxReadVector3(vtx);
		meshObj.vertList.push(mcv);
	}

	//J 面配列作成
	//E Create facet array

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

		auto checkVtxId = [ & ]( PfxUInt32 id )
		{
			if( id < param.numVerts ) {
				return true;
			}
			return false;
		};

		//	this code will check the overrunning the buffers.
		if( !checkVtxId( idx[ 0 ] ) || !checkVtxId( idx[ 1 ] ) || !checkVtxId( idx[ 2 ] ) ) {
			return SCE_PFX_ERR_OUT_OF_RANGE_VERTEX;
		}

		idxList[ i * 3 + 0 ] = idx[ 0 ];
		idxList[ i * 3 + 1 ] = idx[ 1 ];
		idxList[ i * 3 + 2 ] = idx[ 2 ];
	}

	// At first, it calculates offset and extent which are used to calculate quantized values
	PfxVector3 totalAabbMin(SCE_PFX_FLT_MAX);
	PfxVector3 totalAabbMax(-SCE_PFX_FLT_MAX);

	for( PfxUInt32 i = 0; i < param.numTriangles; i++ ) {
		const PfxVector3 pnts[ 3 ] = {
			meshObj.vertList[ idxList[ i * 3 + 0 ] ].coord,
			meshObj.vertList[ idxList[ i * 3 + 1 ] ].coord,
			meshObj.vertList[ idxList[ i * 3 + 2 ] ].coord,
		};

		PfxVector3 normal = normalize( cross( pnts[ 2 ] - pnts[ 1 ], pnts[ 0 ] - pnts[ 1 ] ) );
		PfxFloat thickness = param.defaultThickness;

		if( lengthSqr( normal ) < epsilon ) continue;

		PfxVector3 thicknessPoints[ 3 ] = {
			pnts[ 0 ] - thickness * normal,pnts[ 1 ] - thickness * normal,pnts[ 2 ] - thickness * normal
		};

		PfxVector3 facetAabbMin = minPerElem( thicknessPoints[ 2 ], minPerElem( thicknessPoints[ 1 ], minPerElem( thicknessPoints[ 0 ], minPerElem( pnts[ 2 ], minPerElem( pnts[ 1 ], pnts[ 0 ] ) ) ) ) );
		PfxVector3 facetAabbMax = maxPerElem( thicknessPoints[ 2 ], maxPerElem( thicknessPoints[ 1 ], maxPerElem( thicknessPoints[ 0 ], maxPerElem( pnts[ 2 ], maxPerElem( pnts[ 1 ], pnts[ 0 ] ) ) ) ) );

		totalAabbMin = minPerElem( totalAabbMin, facetAabbMin );
		totalAabbMax = maxPerElem( totalAabbMax, facetAabbMax );
	}

	meshObj.totalAabbMin = totalAabbMin;
	meshObj.totalAabbMax = totalAabbMax;

	// Correct gap of a quantized position and a original position
	PfxVector3 offset = ( meshObj.totalAabbMax + meshObj.totalAabbMin ) * 0.5f;
	PfxVector3 half = ( meshObj.totalAabbMax - meshObj.totalAabbMin ) * 0.5f;

	auto quantizeVertex = [ & ]( PfxMcVert &vert )
	{
		// quantize
		const PfxVector3 sz( SCE_PFX_QUANTIZE_MAX );
		PfxVector3 tmp1 = divPerElem( vert.coord - offset, 2.0f*half );
		tmp1 = mulPerElem( sz, minPerElem( maxPerElem( tmp1, PfxVector3( -0.5f ) ), PfxVector3( 0.5f ) ) ); // clamp -0.5 , 0.5
		PfxQuantize3 q( ( PfxInt16 )tmp1[ 0 ], ( PfxInt16 )tmp1[ 1 ], ( PfxInt16 )tmp1[ 2 ] );
		vert.quant = q;

		// restore
		const PfxVector3 szInv( 1.0f / ( PfxFloat )SCE_PFX_QUANTIZE_MAX ), lp( ( PfxFloat )q.elem[ 0 ], ( PfxFloat )q.elem[ 1 ], ( PfxFloat )q.elem[ 2 ] );
		PfxVector3 tmp2 = mulPerElem( lp, szInv );
		vert.coord = mulPerElem( tmp2, 2.0f*half ) + offset;
	};

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

	if( param.flag & ( SCE_PFX_MESH_FLAG_STRUCTURE_STANDARD_BVH | SCE_PFX_MESH_FLAG_STRUCTURE_COMPRESSED_BVH ) ) {
		for( PfxUInt32 i = 0; i < meshObj.vertList.size(); i++ ) {
			quantizeVertex( meshObj.vertList[ i ] );
		}
	}

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
				meshObj.vertList[ idx[ 0 ] ].coord,
				meshObj.vertList[ idx[ 1 ] ].coord,
				meshObj.vertList[ idx[ 2 ] ].coord,
			};

			//J 面積がfacetAreaLimit以下の面を排除
			//E Remove facets less than facetAreaLimit
			PfxFloat areaSqr = lengthSqr( cross( pnts[ 1 ] - pnts[ 0 ], pnts[ 2 ] - pnts[ 0 ] ) );
			if( ( ( param.flag & SCE_PFX_MESH_FLAG_AUTO_ELIMINATION ) && areaSqr < param.facetAreaLimit * param.facetAreaLimit ) || areaSqr < epsilon ) {
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
				meshObj.vertList[ i ].coord = meshObj.vertList[ rootId ].coord;
				meshObj.vertList[ i ].quant = meshObj.vertList[ rootId ].quant;
			}
		}

		counter++;
	} while( numValidTriangles > 0 && zeroTriangles > 0 );

	// If all related triangles are removed after filling holes, it setup indices again and just removes small triangles.
	// This code is just a work around to keep existing assets built without an error.
	if( numValidTriangles == 0) {
		for( PfxUInt32 i = 0; i < param.numVerts; i++ ) {
			PfxFloat *vtx = ( PfxFloat* )( ( uintptr_t )param.verts + param.vertexStrideBytes * i );
			meshObj.vertList[ i ].coord = pfxReadVector3( vtx );
			if( param.flag & ( SCE_PFX_MESH_FLAG_STRUCTURE_STANDARD_BVH | SCE_PFX_MESH_FLAG_STRUCTURE_COMPRESSED_BVH ) ) {
				quantizeVertex( meshObj.vertList[ i ] );
			}
		}

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
				meshObj.vertList[ idx[ 0 ] ].coord,
				meshObj.vertList[ idx[ 1 ] ].coord,
				meshObj.vertList[ idx[ 2 ] ].coord,
			};

			PfxFloat areaSqr = lengthSqr( cross( pnts[ 1 ] - pnts[ 0 ], pnts[ 2 ] - pnts[ 0 ] ) );
			if( ( ( param.flag & SCE_PFX_MESH_FLAG_AUTO_ELIMINATION ) && areaSqr < param.facetAreaLimit * param.facetAreaLimit ) || areaSqr < epsilon ) {
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

	// Here, it gathers and stores actual facets
	for(PfxUInt32 i=0;i<param.numTriangles;i++) {
		PfxUInt32 idx[ 3 ];
		idx[ 0 ] = idxList[ i * 3 + 0 ];
		idx[ 1 ] = idxList[ i * 3 + 1 ];
		idx[ 2 ] = idxList[ i * 3 + 2 ];

		if(idx[ 0 ] == (PfxUInt32)-1) continue; // removed triangle

		const PfxVector3 pnts[ 3 ] = {
			meshObj.vertList[ idx[ 0 ] ].coord,
			meshObj.vertList[ idx[ 1 ] ].coord,
			meshObj.vertList[ idx[ 2 ] ].coord,
		};

		PfxMcFacet facet;
		facet.id = meshObj.facetList.size();
		facet.v[0] = &meshObj.vertList[idx[0]];
		facet.v[1] = &meshObj.vertList[idx[1]];
		facet.v[2] = &meshObj.vertList[idx[2]];
		facet.e[0] = facet.e[1] = facet.e[2] = NULL;
		facet.normal = normalize(cross(pnts[2]-pnts[1],pnts[0]-pnts[1]));
		facet.obsolete = false;
		facet.thickness = param.defaultThickness;
		facet.neighbor[0] = facet.neighbor[1] = facet.neighbor[2] = -1;
		facet.neighborEdgeId[0] = facet.neighborEdgeId[1] = facet.neighborEdgeId[2] = -1;
		facet.userData = param.userData ? param.userData[i] : 0;
		facet.next = NULL;
		
		PfxVector3 thicknessPoints[3] = {
			pnts[0] - facet.thickness * facet.normal,pnts[1] - facet.thickness * facet.normal,pnts[2] - facet.thickness * facet.normal
		};
		
		facet.aabbMin = minPerElem(thicknessPoints[2],minPerElem(thicknessPoints[1],minPerElem(thicknessPoints[0],minPerElem(pnts[2],minPerElem(pnts[1],pnts[0])))));
		facet.aabbMax = maxPerElem(thicknessPoints[2],maxPerElem(thicknessPoints[1],maxPerElem(thicknessPoints[0],maxPerElem(pnts[2],maxPerElem(pnts[1],pnts[0])))));

		meshObj.facetList.push(facet);
	}
	
	if( param.statistics ) {
		param.statistics->numTotalConvertedTriangles = meshObj.facetList.size();
		param.statistics->numRemovedTriangles = param.numTriangles - meshObj.facetList.size();
	}
	
	if(meshObj.facetList.size() == 0) {
		// 登録する面がない
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	// 頂点から面への参照リストを作成
	{
		// 頂点から面への参照リストを作成
		PfxInt32 cnt = 0;
		for(PfxUInt32 i=0;i<meshObj.facetList.size();i++) {
			for(PfxUInt32 v=0;v<3;v++) {
				PfxUInt32 vertId = meshObj.facetList[i].v[v]->vertId;
				meshObj.triEntry[cnt].facet = &meshObj.facetList[i];
				meshObj.triEntry[cnt].next = meshObj.triHead[vertId];
				meshObj.triHead[vertId] = &meshObj.triEntry[cnt++];
			}
		}
		
		// 同一頂点をまとめる
		PfxUInt32 numRemovedVertices = 0;
		if(param.flag & SCE_PFX_MESH_FLAG_AUTO_ELIMINATION) {
			for(PfxUInt32 i=0;i<param.numVerts;i++) {
				if(meshObj.vertList[i].obsolete) continue;
				if(!meshObj.triHead[i]) {
					meshObj.vertList[i].obsolete = true;
					continue;
				}
				
				for(PfxUInt32 j=i+1;j<param.numVerts;j++) {
					if(meshObj.vertList[j].obsolete) continue;
					
					PfxFloat lenSqr = lengthSqr(meshObj.vertList[i].coord-meshObj.vertList[j].coord);
					
					if(lenSqr < param.facetAreaLimit * param.facetAreaLimit) { // 同一頂点が１面に保持されないようにする
						meshObj.vertList[j].obsolete = true; // 同一点なので廃棄フラグを立てる
						if(meshObj.triHead[j]) {//	this is needed because if there will be the vertices that are not held by the facets.
							PfxMcVert2TriList *f=meshObj.triHead[j];
							PfxMcVert2TriList *jend=meshObj.triHead[j];
							for(;f!=NULL;f=f->next) {
								jend = f;
								for(PfxInt32 k=0;k<3;k++) {
									if(f->facet->v[k] == &meshObj.vertList[j]) {
										f->facet->v[k] = &meshObj.vertList[i]; // 頂点を付け替える
										break;
									}
								}
							}
							jend->next = meshObj.triHead[i];
							meshObj.triHead[i] = meshObj.triHead[j];
						}
						if(param.flag & SCE_PFX_MESH_FLAG_OUTPUT_INFO) SCE_PFX_PRINTF("remove vertex %d\n",j);
						numRemovedVertices++;
					}
				}
			}
		}
		if( param.statistics ) {
			param.statistics->numTotalConvertedVertices = param.numVerts - numRemovedVertices;
			param.statistics->numRemovedVertices = numRemovedVertices;
		}
	}

	// エッジ配列の作成
	PfxUInt32 ecnt = 0;
	for(PfxUInt32 i=0;i<meshObj.facetList.size();i++) {
		PfxMcFacet &f = meshObj.facetList[i];
		
		for(PfxUInt32 v=0;v<3;v++) {
			uintptr_t vp1 = ((uintptr_t)f.v[v]-(uintptr_t)&meshObj.vertList[0])/sizeof(PfxMcVert);
			uintptr_t vp2 = ((uintptr_t)f.v[(v+1)%3]-(uintptr_t)&meshObj.vertList[0])/sizeof(PfxMcVert);
			PfxUInt32 viMin = (PfxUInt32)SCE_PFX_MIN(vp1,vp2);
			PfxUInt32 viMax = (PfxUInt32)SCE_PFX_MAX(vp1,vp2);
			PfxInt32 key = ((0x8da6b343*viMin+0xd8163841*viMax)%(meshObj.facetList.size()*3));
			for(PfxMcEdge *e = meshObj.edgeHead[key];;e=e->next) {
				if(!e) {
					meshObj.edgeList[ecnt].vertId[0] = viMin;
					meshObj.edgeList[ecnt].vertId[1] = viMax;
					meshObj.edgeList[ecnt].facetId[0] = i;
					meshObj.edgeList[ecnt].facetId[1] = 0;
					meshObj.edgeList[ecnt].edgeId[0] = v;
					meshObj.edgeList[ecnt].edgeId[1] = 0;
					meshObj.edgeList[ecnt].numFacets = 1;
					meshObj.edgeList[ecnt].next = meshObj.edgeHead[key];
					meshObj.edgeList[ecnt].angleType = SCE_PFX_EDGE_CONVEX;
					meshObj.edgeList[ecnt].angle = 0.0f;
					meshObj.edgeHead[key] = &meshObj.edgeList[ecnt];
					f.e[v] = &meshObj.edgeList[ecnt];
					ecnt++;
					break;
				}
				
				if(e->vertId[0] == viMin && e->vertId[1] == viMax && e->numFacets == 1) {
					e->facetId[1] = i;
					e->edgeId[1] = v;
					e->numFacets = 2;
					f.e[v] = e;
					f.neighbor[v] = e->facetId[0];
					f.neighborEdgeId[v] = e->edgeId[0];
					meshObj.facetList[e->facetId[0]].neighbor[e->edgeId[0]] = i;
					meshObj.facetList[e->facetId[0]].neighborEdgeId[e->edgeId[0]] = e->edgeId[1];
					break;
				}
				
				if(e->vertId[0] == viMin && e->vertId[1] == viMax && e->numFacets == 2) {
					edgeWarning++;
				}
			}
		}
	}
	meshObj.edgeList.assign(ecnt);
	
    if(edgeWarning > 0 && param.flag & SCE_PFX_MESH_FLAG_OUTPUT_INFO) {
    	SCE_PFX_PRINTF(" Warning: %d edges shared by more than 3 facets (while processing \"%s\")\n", edgeWarning, param.name);
	}

	if( param.statistics ) {
		param.statistics->numEdgesSharedByMoreThan2Triangles = edgeWarning;
	}

	SCE_PFX_PUSH_PERF("buildBvh");

	// メッシュの構造を計算する
	calculateAttributes(meshObj,param);
	
	// bvhを構築
	PfxInt32 ret = SCE_PFX_OK;
	if(enableMultiParts) {
		ret = buildBvhParts(meshObj,param);
	}
	else {
		ret = buildBvh(meshObj,param);
	}

	SCE_PFX_POP_PERF();
	
    if(param.flag & SCE_PFX_MESH_FLAG_OUTPUT_INFO) {
		SCE_PFX_PRINTF("sub-meshes %u nodes %u islands %u\n",meshObj.numRoots,meshObj.numBvhNodes,meshObj.numIslands);
	}
	
	return ret;
}

static void printBvh(const PfxMcIslandBvhNode *node,int depth)
{
	char lineBuff[1024];
	char *c = lineBuff;
	char *e = lineBuff + 1024;
	
	for(int t=0;t<depth;t++) {
		*(c++) = '-';
		*(c++) = '-';
	}
	
	if(node->isLeaf()) {
		sprintf_s(c,e-c,"%lx parent:%lx child:%lx,%lx aabb %f %f %f , %f %f %f islandId %u numFacets %u \n",
			(uintptr_t)node, (uintptr_t)node->parent, (uintptr_t)node->left, (uintptr_t)node->right,
			(float)node->aabbMin[0],(float)node->aabbMin[1],(float)node->aabbMin[2],
			(float)node->aabbMax[0],(float)node->aabbMax[1],(float)node->aabbMax[2],
			node->islandId,node->numFacets);
	}
	else {
		sprintf_s(c,e-c,"%lx parent:%lx child:%lx,%lx aabb %f %f %f , %f %f %f\n",
			(uintptr_t)node, (uintptr_t)node->parent, (uintptr_t)node->left, (uintptr_t)node->right,
			(float)node->aabbMin[0],(float)node->aabbMin[1],(float)node->aabbMin[2],
			(float)node->aabbMax[0],(float)node->aabbMax[1],(float)node->aabbMax[2]);
	}
	
	SCE_PFX_PRINTF("%s",lineBuff);
	
	if(node->left) {
		printBvh(node->left,depth+1);
	}
	if(node->right) {
		printBvh(node->right,depth+1);
	}
}

void pfxMeshObjectPrint(const PfxMeshObject &meshObj)
{
	printBvh(&meshObj.bvh[0],0);
}

} //namespace pfxv4
} //namespace sce
