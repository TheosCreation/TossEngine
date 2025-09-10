/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_MESH_COMMON_H
#define _SCE_PFX_MESH_COMMON_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../../include/physics_effects/util/pfx_static_array.h"
#include "pfx_tri_mesh.h"
#include "pfx_intersect_common.h"

#define SCE_PFX_THICKNESS_THRESHOLD 0.05f

namespace sce {
namespace pfxv4 {

static inline
PfxQuantize3 operator +( const PfxQuantize3 &q1, const PfxQuantize3 &q2 )
{
    return PfxQuantize3(q1.elem[0] + q2.elem[0],q1.elem[1] + q2.elem[1],q1.elem[2] + q2.elem[2]);
}

static inline
PfxQuantize3 operator -( const PfxQuantize3 &q1, const PfxQuantize3 &q2 )
{
    return PfxQuantize3(q1.elem[0] - q2.elem[0],q1.elem[1] - q2.elem[1],q1.elem[2] - q2.elem[2]);
}

static inline
PfxQuantize3 pfxHalve( const PfxQuantize3 &q )
{
	return PfxQuantize3(q.elem[0] >> 1,q.elem[1] >> 1,q.elem[2] >> 1);
}

struct PfxClosestPoints {
	PfxPoint3 pA[4],pB[4];
	PfxFloat distSqr[4];
	PfxFloat closestDistSqr;
	int numPoints;
	SCE_PFX_PADDING(1,8)
	
	PfxClosestPoints()
	{
		numPoints = 0;
		closestDistSqr = SCE_PFX_FLT_MAX;
	}
	
	void set(int i,const PfxPoint3 &pointA,const PfxPoint3 &pointB,PfxFloat dSqr)
	{
		pA[i] = pointA;
		pB[i] = pointB;
		distSqr[i] = dSqr;
	}
	
	void add(const PfxPoint3 &pointA,const PfxPoint3 &pointB,PfxFloat dSqr)
	{
		const PfxFloat epsilon = 0.00001f;
		if(closestDistSqr < dSqr) return;
		
		int replaceId = -1;
		PfxFloat distMax = -SCE_PFX_FLT_MAX;
		for(int i=0;i<numPoints;i++) {
			if(lengthSqr(pA[i]-pointA) < epsilon && lengthSqr(pB[i]-pointB) < epsilon && distSqr[i] < dSqr) {
				return;
			}
			if(distMax < distSqr[i]) {
				distMax = distSqr[i];
				replaceId = i;
			}
		}
		
		replaceId = (numPoints<4)?(numPoints++):replaceId;

		closestDistSqr = dSqr + epsilon;

		set(replaceId,pointA,pointB,dSqr);
	}
};

static SCE_PFX_FORCE_INLINE
PfxUInt32 pfxGatherFacets(
	const PfxExpandedTriMesh *mesh,
	const PfxVector3 &aabbCenter,
	const PfxVector3 &aabbHalf,
	PfxUInt8 *selFacets)
{
	PfxUInt32 numSelFacets = 0;
	PfxVecInt3 vi3;
	for(int f=0;f<mesh->m_numFacets;f++) {
		const PfxExpandedFacet &facet = mesh->m_facets[f];
		PfxVector3 cnt = pfxReadVector3((PfxFloat*)&facet.m_center);
		PfxVector3 hlf = pfxReadVector3((PfxFloat*)&facet.m_half);
		PfxVector3 results = absPerElem(cnt - aabbCenter) - (hlf + aabbHalf);
		if(results[0] > 0.0f || results[1] > 0.0f || results[2] > 0.0f) continue;
		selFacets[numSelFacets++] = f;
	}
	return numSelFacets;
}

struct PfxEncStack {
	PfxUInt32 nodeId;
	PfxVector3 aabbMin;
	PfxVector3 aabbMax;
};

static SCE_PFX_FORCE_INLINE
PfxUInt32 pfxGatherFacets(
	const PfxQuantizedTriMeshBvh *mesh,
	const PfxVector3 &aabbMinB,
	const PfxVector3 &aabbMaxB,
	PfxUInt8 *selFacets)
{
	PfxEncStack encroot;
	encroot.nodeId = 0;
	encroot.aabbMin = pfxReadVector3(mesh->m_aabbMin);
	encroot.aabbMax = pfxReadVector3(mesh->m_aabbMax);
	
	PfxUInt32 numSelFacets = 0;
	PfxStaticStack<PfxEncStack> bvhStack;
	
	bvhStack.push(encroot);
	
	#ifdef PFX_ENABLE_AVX
	const __m128 vAabbMinB = sce_vectormath_asm128(aabbMinB.get128());
	const __m128 vAabbMaxB = sce_vectormath_asm128(aabbMaxB.get128());
	const __m128 v255Inv = _mm_rcp_ps(_mm_set1_ps(255.0f));
	#endif
	
	while(!bvhStack.empty()) {
		PfxEncStack info = bvhStack.top();
		bvhStack.pop();
		
		PfxFacetBvhNode &encnode = mesh->m_bvhNodes[info.nodeId];
		
		#ifdef PFX_ENABLE_AVX
		__m128 vQuantizedMin = _mm_cvtepi32_ps(_mm_set_epi32(0,encnode.aabb[2],encnode.aabb[1],encnode.aabb[0]));
		__m128 vQuantizedMax = _mm_cvtepi32_ps(_mm_set_epi32(0,encnode.aabb[5],encnode.aabb[4],encnode.aabb[3]));
		PfxVector3 vtmp = mulPerElem(info.aabbMax-info.aabbMin,PfxVector3(sce_vectormath_asfloat4(v255Inv)));
		PfxVector3 aabbMinA = info.aabbMin + mulPerElem(vtmp,PfxVector3(sce_vectormath_asfloat4(vQuantizedMin)));
		PfxVector3 aabbMaxA = info.aabbMax - mulPerElem(vtmp,PfxVector3(sce_vectormath_asfloat4(vQuantizedMax)));
		const __m128 vAabbMinA = sce_vectormath_asm128(aabbMinA.get128());
		const __m128 vAabbMaxA = sce_vectormath_asm128(aabbMaxA.get128());
		int test1 = _mm_movemask_ps(_mm_cmplt_ps(vAabbMaxA,vAabbMinB));
		int test2 = _mm_movemask_ps(_mm_cmpgt_ps(vAabbMinA,vAabbMaxB));
		if(((test1 | test2) & 0x07) != 0) continue;
		#else
		PfxVector3 quantizedMin(encnode.aabb[0],encnode.aabb[1],encnode.aabb[2]);
		PfxVector3 quantizedMax(encnode.aabb[3],encnode.aabb[4],encnode.aabb[5]);
		PfxVector3 aabbMinA = info.aabbMin + mulPerElem((info.aabbMax-info.aabbMin),quantizedMin / 255.0f);
		PfxVector3 aabbMaxA = info.aabbMax - mulPerElem((info.aabbMax-info.aabbMin),quantizedMax / 255.0f);
		if(aabbMaxA[0] < aabbMinB[0] || aabbMinA[0] > aabbMaxB[0]) continue;
		if(aabbMaxA[1] < aabbMinB[1] || aabbMinA[1] > aabbMaxB[1]) continue;
		if(aabbMaxA[2] < aabbMinB[2] || aabbMinA[2] > aabbMaxB[2]) continue;
		#endif
		
		PfxEncStack encnext;
		encnext.aabbMin = aabbMinA;
		encnext.aabbMax = aabbMaxA;
		
		PfxUInt8 LStatus = encnode.flag & 0x03;
		PfxUInt8 RStatus = (encnode.flag & 0x0C) >> 2;
		
		if(LStatus == 0) {
			encnext.nodeId = encnode.left;
			bvhStack.push(encnext);
		}
		else if(LStatus == 1) {
			selFacets[numSelFacets++] = encnode.left;
		}
		
		if(RStatus == 0) {
			encnext.nodeId = encnode.right;
			bvhStack.push(encnext);
		}
		else if(RStatus == 1) {
			selFacets[numSelFacets++] = encnode.right;
		}
	}
	return numSelFacets;

}

static SCE_PFX_FORCE_INLINE
PfxUInt32 pfxGatherFacets(
	const PfxCompressedTriMesh *mesh,const PfxFacetBvhNode *bvhNodes,
	const PfxVector3 &aabbMinB,
	const PfxVector3 &aabbMaxB,
	PfxUInt8 *selFacets)
{
	PfxEncStack encroot;
	encroot.nodeId = 0;
	encroot.aabbMin = pfxReadVector3(mesh->m_aabbMin);
	encroot.aabbMax = pfxReadVector3(mesh->m_aabbMax);
	
	PfxUInt32 numSelFacets = 0;
	PfxStaticStack<PfxEncStack> bvhStack;
	
	bvhStack.push(encroot);
	
	#ifdef PFX_ENABLE_AVX
	const __m128 vAabbMinB = sce_vectormath_asm128(aabbMinB.get128());
	const __m128 vAabbMaxB = sce_vectormath_asm128(aabbMaxB.get128());
	const __m128 v255Inv = _mm_rcp_ps(_mm_set1_ps(255.0f));
	#endif
	
	while(!bvhStack.empty()) {
		PfxEncStack info = bvhStack.top();
		bvhStack.pop();
		
		const PfxFacetBvhNode &encnode = *(bvhNodes + mesh->m_bvhNodes + info.nodeId);
		
		#ifdef PFX_ENABLE_AVX
		__m128 vQuantizedMin = _mm_cvtepi32_ps(_mm_set_epi32(0,encnode.aabb[2],encnode.aabb[1],encnode.aabb[0]));
		__m128 vQuantizedMax = _mm_cvtepi32_ps(_mm_set_epi32(0,encnode.aabb[5],encnode.aabb[4],encnode.aabb[3]));
		PfxVector3 vtmp = mulPerElem(info.aabbMax-info.aabbMin,PfxVector3(sce_vectormath_asfloat4(v255Inv)));
		PfxVector3 aabbMinA = info.aabbMin + mulPerElem(vtmp,PfxVector3(sce_vectormath_asfloat4(vQuantizedMin)));
		PfxVector3 aabbMaxA = info.aabbMax - mulPerElem(vtmp,PfxVector3(sce_vectormath_asfloat4(vQuantizedMax)));
		const __m128 vAabbMinA = sce_vectormath_asm128(aabbMinA.get128());
		const __m128 vAabbMaxA = sce_vectormath_asm128(aabbMaxA.get128());
		int test1 = _mm_movemask_ps(_mm_cmplt_ps(vAabbMaxA,vAabbMinB));
		int test2 = _mm_movemask_ps(_mm_cmpgt_ps(vAabbMinA,vAabbMaxB));
		if(((test1 | test2) & 0x07) != 0) continue;
		#else
		PfxVector3 quantizedMin(encnode.aabb[0],encnode.aabb[1],encnode.aabb[2]);
		PfxVector3 quantizedMax(encnode.aabb[3],encnode.aabb[4],encnode.aabb[5]);
		PfxVector3 aabbMinA = info.aabbMin + mulPerElem((info.aabbMax-info.aabbMin),quantizedMin / 255.0f);
		PfxVector3 aabbMaxA = info.aabbMax - mulPerElem((info.aabbMax-info.aabbMin),quantizedMax / 255.0f);
		if(aabbMaxA[0] < aabbMinB[0] || aabbMinA[0] > aabbMaxB[0]) continue;
		if(aabbMaxA[1] < aabbMinB[1] || aabbMinA[1] > aabbMaxB[1]) continue;
		if(aabbMaxA[2] < aabbMinB[2] || aabbMinA[2] > aabbMaxB[2]) continue;
		#endif
		
		PfxEncStack encnext;
		encnext.aabbMin = aabbMinA;
		encnext.aabbMax = aabbMaxA;
		
		PfxUInt8 LStatus = encnode.flag & 0x03;
		PfxUInt8 RStatus = (encnode.flag & 0x0C) >> 2;
		
		if(LStatus == 0) {
			encnext.nodeId = encnode.left;
			bvhStack.push(encnext);
		}
		else if(LStatus == 1) {
			selFacets[numSelFacets++] = encnode.left;
		}
		
		if(RStatus == 0) {
			encnext.nodeId = encnode.right;
			bvhStack.push(encnext);
		}
		else if(RStatus == 1) {
			selFacets[numSelFacets++] = encnode.right;
		}
	}
	return numSelFacets;
}

static SCE_PFX_FORCE_INLINE
PfxUInt32 pfxGatherFacetsInBSphere(
	const PfxExpandedTriMesh *mesh,
	const PfxVector3 &sphereCenter,
	PfxFloat sphereRadius,
	PfxUInt8 *selFacets)
{
	PfxUInt32 numSelFacets = 0;
	PfxVecInt3 vi3;
	PfxFloat radSqr = sphereRadius * sphereRadius;
	for(int f=0;f<mesh->m_numFacets;f++) {
		const PfxExpandedFacet &facet = mesh->m_facets[f];
		PfxVector3 cnt = pfxReadVector3((PfxFloat*)&facet.m_center);
		PfxVector3 hlf = pfxReadVector3((PfxFloat*)&facet.m_half);
		PfxVector3 p = sphereCenter-cnt;
		PfxVector3 s;
		pfxClosestPointAABB(p,hlf,s);
		if(lengthSqr(p-s) > radSqr) continue;
		selFacets[numSelFacets++] = f;
	}
	return numSelFacets;
}

static SCE_PFX_FORCE_INLINE
void pfxGetProjAxisPnts6(
	const PfxVector3 *verts,const PfxVector3 &axis,
	PfxFloat &distMin,PfxFloat &distMax)
{
	PfxMatrix3 m0 = transpose(PfxMatrix3(verts[0],verts[1],verts[2]));
	PfxMatrix3 m1 = transpose(PfxMatrix3(verts[3],verts[4],verts[5]));
	PfxVector3 v0 = m0 * axis;
	PfxVector3 v1 = m1 * axis;
	distMin = minElem(minPerElem(v0,v1));
	distMax = maxElem(maxPerElem(v0,v1));
}

static SCE_PFX_FORCE_INLINE
void pfxGetProjAxisPnts3(
	const PfxVector3 *verts,const PfxVector3 &axis,
	PfxFloat &distMin,PfxFloat &distMax)
{
	PfxFloat p0 = dot(axis, verts[0]);
	PfxFloat p1 = dot(axis, verts[1]);
	PfxFloat p2 = dot(axis, verts[2]);
	distMin = SCE_PFX_MIN(p2,SCE_PFX_MIN(p0,p1));
	distMax = SCE_PFX_MAX(p2,SCE_PFX_MAX(p0,p1));
}

static SCE_PFX_FORCE_INLINE
void pfxGetProjAxisPnts2(
	const PfxVector3 *verts,const PfxVector3 &axis,
	PfxFloat &distMin,PfxFloat &distMax)
{
	PfxFloat p0 = dot(axis, verts[0]);
	PfxFloat p1 = dot(axis, verts[1]);
	distMin = SCE_PFX_MIN(p0,p1);
	distMax = SCE_PFX_MAX(p0,p1);
}

///////////////////////////////////////////////////////////////////////////////
// ２つのベクトルの向きをチェック

static SCE_PFX_FORCE_INLINE
bool pfxIsSameDirection(const PfxVector3 &vecA,const PfxVector3 &vecB)
{
	return fabsf(dot(vecA,vecB)) > 0.9999f;
}

///////////////////////////////////////////////////////////////////////////////
// 面ローカルの座標を算出

static SCE_PFX_FORCE_INLINE
void pfxCalcBarycentricCoords(
	const PfxVector3 &pointOnTriangle,
	const PfxTriangle &triangle,
	PfxFloat &s,PfxFloat &t)
{
	PfxVector3 p0 = triangle.points[0];
	PfxVector3 p1 = triangle.points[1];
	PfxVector3 p2 = triangle.points[2];

	#if 0
	PfxVector3 v0 = p1 - p0;
	PfxVector3 v1 = p2 - p0;
	PfxVector3 dir = pointOnTriangle - p0;
	PfxVector3 v = cross( v0, v1 );
	PfxVector3 crS = cross( v, v0 );
	PfxVector3 crT = cross( v, v1 );
	s = dot( crT, dir ) / dot( crT, v0 );
	t = dot( crS, dir ) / dot( crS, v1 );
	#else
	PfxVector3 v = cross((p1 - p0),(p2 - p0));
	PfxFloat tmp1 = dot(v,v);
	PfxVector3 tmp2 = cross(p0-pointOnTriangle,v);
	s = dot(tmp2,(p2-pointOnTriangle)) / tmp1;
	t = dot(tmp2,(pointOnTriangle-p1)) / tmp1;
	#endif
}

// a,bからなる直線上に点pがあるかどうかを判定
static SCE_PFX_FORCE_INLINE
bool pfxPointOnLine(const PfxVector3 &p,const PfxVector3 &a,const PfxVector3 &b)
{
	PfxVector3 ab = normalize(b-a);
	PfxVector3 q = a + ab * dot(p-a,ab);
	return lengthSqr(p-q) < 0.00001f;
}

static SCE_PFX_FORCE_INLINE
bool pfxPointIsOnTriangleEdge(const PfxVector3 &q, PfxUInt32 edgeChk, const PfxVector3 &p0, const PfxVector3 &p1, const PfxVector3 &p2)
{
	// Note : p is on the triangle
	const float epsilon = 1e-4f;
	PfxVector3 v = cross((p1 - p0), (p2 - p0));
	PfxFloat tmp1 = dot(v, v);
	PfxVector3 tmp2 = cross(p0 - q, v);
	PfxFloat s = dot(tmp2, (p2 - q)) / tmp1;
	PfxFloat t = dot(tmp2, (q - p1)) / tmp1;

	if (s < epsilon) {
		if (t < epsilon) {
			return (edgeChk & 0x33) == 0; // point0
		}
		else if (t > 1.0f - epsilon) {
			return (edgeChk & 0x3c) == 0; // point2
		}
		else {
			return (edgeChk & 0x30) == 0; // edge2
		}
	}
	else if (t < epsilon) {
		if (s < epsilon) {
			return (edgeChk & 0x33) == 0; // point0
		}
		else if (s > 1.0f - epsilon) {
			return (edgeChk & 0x0f) == 0; // point1
		}
		else {
			return (edgeChk & 0x03) == 0; // edge0
		}
	}
	else if (s + t > 1.0 - epsilon) {
		return (edgeChk & 0x0c) == 0; // edge 1
	}

	return false;
}

// 線分a,b上に点pがあるかどうかを判定
static SCE_PFX_FORCE_INLINE
bool pfxPointOnSegment(const PfxVector3 &p,const PfxVector3 &a,const PfxVector3 &b)
{
	PfxVector3 ab = b-a;
	PfxVector3 ap = p-a;
	PfxFloat denom = dot(ab,ab);
	PfxFloat num = dot(ap,ab);
	PfxFloat t = num/denom;
	if(t < 0.0f || t > 1.0f) return false;
	return (dot(ap,ap)-num*t) < 0.00001f;
}

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_MESH_COMMON_H
