/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_intersect_common.h"
#include "pfx_intersect_ray_convex.h"
#include "pfx_convex_mesh_impl.h"
#include "pfx_large_tri_mesh_impl.h"
#include "pfx_mesh_common.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectRayConvex(const PfxRayInputInternal &ray,PfxRayOutputInternal &out,const void *shape,const PfxTransform3 &transform)
{
	const PfxConvexMeshImpl *convex = (const PfxConvexMeshImpl*)shape;
	
	// レイをConvexのローカル座標へ変換
	PfxTransform3 transformConvex = inverse(transform);
	PfxVector3 startPosL = transformConvex.getUpper3x3() * ray.m_startPosition + transformConvex.getTranslation();
	PfxVector3 rayDirL = transformConvex.getUpper3x3() * ray.m_direction;
	PfxMatrix3 rotation_n = transpose(transformConvex.getUpper3x3()); // 法線をワールドへ変換するマトリクス
	
	// レイとConvexの交差判定
#if 1
	PfxFloat tmpVariable(0.0f);
	PfxUInt32 faceId = 0;
	
	bool ret = false;

	for (PfxUInt32 f = 0; f < (PfxUInt32)convex->m_numFacets; f++) {
		PfxTriangle triangle(
			pfxReadVector3(convex->m_verts + convex->m_indices[f * 3] * 3),
			pfxReadVector3(convex->m_verts + convex->m_indices[f * 3 + 1] * 3),
			pfxReadVector3(convex->m_verts + convex->m_indices[f * 3 + 2] * 3));

		if (pfxIntersectRayTriangleWithoutBackFace(startPosL, rayDirL, triangle, tmpVariable) && tmpVariable < out.m_variable) {
			out.m_variable = tmpVariable;
			faceId = f;
			ret = true;
		}
	}

	if(ret) {
		out.m_contactFlag = true;
		
		PfxTriangle triangle(
			pfxReadVector3(convex->m_verts + convex->m_indices[faceId*3  ] * 3),
			pfxReadVector3(convex->m_verts + convex->m_indices[faceId*3+1] * 3),
			pfxReadVector3(convex->m_verts + convex->m_indices[faceId*3+2] * 3));
		
		out.m_contactNormal = normalize(rotation_n * cross(triangle.points[2]-triangle.points[1],triangle.points[0]-triangle.points[1]));
		
		PfxFloat s=0.0f,t=0.0f;
		pfxCalcBarycentricCoords(startPosL+tmpVariable*rayDirL,triangle,s,t);
		out.m_subData.setType(PfxSubData::MESH_INFO);
		out.m_subData.setFacetLocalS(s);
		out.m_subData.setFacetLocalT(t);
		out.m_subData.setFacetId(faceId);
		if(convex->m_userData) {
			out.m_subData.setUserData(convex->m_userData[faceId]);
		}
	}
	
	return ret;
#else
	PfxFloat tEnter=-SCE_PFX_FLT_MAX,tLeave=SCE_PFX_FLT_MAX;
	PfxVector3 intersectNormal;
	PfxUInt32 faceId = 0;
	
	for(PfxUInt32 f=0;f<(PfxUInt32)convex->m_numIndices/3;f++) {
		PfxTriangle triangle(
			pfxReadVector3(convex->m_verts + convex->m_indices[f*3  ] * 3),
			pfxReadVector3(convex->m_verts + convex->m_indices[f*3+1] * 3),
			pfxReadVector3(convex->m_verts + convex->m_indices[f*3+2] * 3));

		PfxVector3 n = cross(triangle.points[1]-triangle.points[0],triangle.points[2]-triangle.points[0]);
		PfxFloat norm = dot(n,triangle.points[0] - startPosL);
		PfxFloat den = dot(n,rayDirL);
		
		if(den * den < SCE_PFX_RAY_TRIANGLE_EPSILON * SCE_PFX_RAY_TRIANGLE_EPSILON) { // レイと面が平行
			if(norm < 0.0f) {
				return false;
			}
			else {
				continue;
			}
		}
		
		PfxFloat t = norm / den;
		if(den < 0.0f) {
			if(tEnter < t) {
				tEnter = t;
				intersectNormal = n;
				faceId = f;
			}
		}
		else {
			tLeave = SCE_PFX_MIN(tLeave,t);
		}
		
		if(tEnter > tLeave) return false;
	}
	
	if(tEnter < 0.0f || tEnter >= out.m_variable) {
		return false;
	}

	out.m_contactFlag = true;
	out.m_variable = tEnter;
	out.m_contactNormal = normalize(rotation_n * intersectNormal);
	
	PfxTriangle triangle(
		pfxReadVector3(convex->m_verts + convex->m_indices[faceId*3  ] * 3),
		pfxReadVector3(convex->m_verts + convex->m_indices[faceId*3+1] * 3),
		pfxReadVector3(convex->m_verts + convex->m_indices[faceId*3+2] * 3));
	
	PfxFloat s=0.0f,t=0.0f;
	pfxCalcBarycentricCoords(startPosL+tEnter*rayDirL,triangle,s,t);
	out.m_subData.setType(PfxSubData::MESH_INFO);
	out.m_subData.setFacetLocalS(s);
	out.m_subData.setFacetLocalT(t);
	out.m_subData.setFacetId(faceId);
	if(convex->m_userData) {
		out.m_subData.setUserData(convex->m_userData[faceId]);
	}
	
	return true;
#endif
}
} //namespace pfxv4
} //namespace sce
