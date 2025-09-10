/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_GJK_SUPPORT_FUNC_H
#define _SCE_PFX_GJK_SUPPORT_FUNC_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../../include/physics_effects/base_level/collision/pfx_box.h"
#include "../../../include/physics_effects/base_level/collision/pfx_capsule.h"
#include "../../../include/physics_effects/base_level/collision/pfx_cylinder.h"
#include "../../../include/physics_effects/base_level/collision/pfx_sphere.h"
#include "../../../include/physics_effects/base_level/collision/pfx_circle.h"
#include "pfx_convex_mesh_impl.h"
#include "pfx_intersect_common.h"

namespace sce {
namespace pfxv4 {

// Support function
template<typename PfxShapeType>
inline PfxUInt32 getSupportVertex(const PfxShapeType *shape, const PfxVector3 &seperatingAxis, PfxVector3 &supportVertex, PfxFloat margin)
{
	SCE_PFX_ASSERT_MSG(0, "unsupported shape");
	return -1;
}

template<>
inline PfxUInt32 getSupportVertex<PfxConvexMeshImpl>(const PfxConvexMeshImpl *mesh, const PfxVector3 &seperatingAxis, PfxVector3 &supportVertex, PfxFloat margin)
{
	int reti = 0;
#ifdef SCE_PFX_ENABLE_CUBEMAP_CONVEX_OPTIMIZE
	int face = 0; // X
	int sign = 1; // +

	PfxFloat f, maxf;

	f = seperatingAxis[0];
	maxf = f; face = 0; sign = 1;
	f = -seperatingAxis[0];
	if (maxf < f) { maxf = f; face = 1; sign = -1; }
	f = seperatingAxis[1];
	if (maxf < f) { maxf = f; face = 2; sign = 1; }
	f = -seperatingAxis[1];
	if (maxf < f) { maxf = f; face = 3; sign = -1; }
	f = seperatingAxis[2];
	if (maxf < f) { maxf = f; face = 4; sign = 1; }
	f = -seperatingAxis[2];
	if (maxf < f) { maxf = f; face = 5; sign = -1; }

	const float signTbl[2][6] = {
		{ -1.0f,-1.0f,1.0f,-1.0f,1.0f,1.0f },
		{ -1.0f,1.0f,1.0f,1.0f,-1.0f,1.0f },
	};
	const int idxTbl[2][6] = {
		{ 2,2,0,0,0,0 },
		{ 1,1,2,2,1,1 },
	};

	PfxFloat u = 0.5f * (1.0f + signTbl[0][face] * seperatingAxis[idxTbl[0][face]] / seperatingAxis[face >> 1]);
	PfxFloat v = 0.5f * (1.0f + signTbl[1][face] * seperatingAxis[idxTbl[1][face]] / seperatingAxis[face >> 1]);

	int ui = SCE_PFX_CONVEX_CUBEMAP_SIZE * u;
	int vi = SCE_PFX_CONVEX_CUBEMAP_SIZE * v;
	if (ui == SCE_PFX_CONVEX_CUBEMAP_SIZE) ui--;
	if (vi == SCE_PFX_CONVEX_CUBEMAP_SIZE) vi--;

	int lineId = vi;
	int indicator = ui;
	if (mesh->m_cubeMap[face].isRowOrColumn != 0) {
		lineId = ui;
		indicator = vi;
	}
	PfxUInt16 start = mesh->m_cubeMap[face].start[lineId];
	PfxUInt8 num = mesh->m_cubeMap[face].num[lineId];
	PfxUInt8 value = 0;
	for (PfxUInt16 i = 0; i < num; i++) {
		PfxUInt16 data = mesh->m_listBuff[start + i];
		PfxUInt16 pos = data >> 8;
		if (indicator < pos) break;
		value = data & 0xff;
	}
	reti = value;
#else
#ifdef PFX_ENABLE_AVX
	__m128 dmax = _mm_set1_ps(-SCE_PFX_FLT_MAX);
	int i = 0;
	const __m128 vSeperatingAxis = sce_vectormath_asm128(seperatingAxis.get128());
	const __m128 sx = _mm_shuffle_ps(vSeperatingAxis, vSeperatingAxis, _MM_SHUFFLE(0, 0, 0, 0));
	const __m128 sy = _mm_shuffle_ps(vSeperatingAxis, vSeperatingAxis, _MM_SHUFFLE(1, 1, 1, 1));
	const __m128 sz = _mm_shuffle_ps(vSeperatingAxis, vSeperatingAxis, _MM_SHUFFLE(2, 2, 2, 2));
	for (; i <= (int)mesh->m_numVerts - 4; i += 4) {
		__m128 v1 = sce_vectormath_asm128( pfxReadVector3( mesh->m_verts + ( i + 0 ) * 3 ).get128());
		__m128 v2 = sce_vectormath_asm128( pfxReadVector3( mesh->m_verts + ( i + 1 ) * 3 ).get128());
		__m128 v3 = sce_vectormath_asm128( pfxReadVector3( mesh->m_verts + ( i + 2 ) * 3 ).get128());
		__m128 v4 = sce_vectormath_asm128( pfxReadVector3( mesh->m_verts + ( i + 3 ) * 3 ).get128());
		__m128 vxy1 = _mm_movelh_ps( v1, v2 );
		__m128 vxy2 = _mm_movelh_ps(v3, v4);
		__m128 xxxx = _mm_shuffle_ps(vxy1, vxy2, _MM_SHUFFLE(2, 0, 2, 0));
		__m128 yyyy = _mm_shuffle_ps(vxy1, vxy2, _MM_SHUFFLE(3, 1, 3, 1));
		__m128 vzw1 = _mm_movehl_ps(v2, v1);
		__m128 vzw2 = _mm_movehl_ps(v4, v3);
		__m128 zzzz = _mm_shuffle_ps(vzw1, vzw2, _MM_SHUFFLE(2, 0, 2, 0));
		__m128 results = _mm_add_ps(_mm_add_ps(_mm_mul_ps(xxxx, sx), _mm_mul_ps(yyyy, sy)), _mm_mul_ps(zzzz, sz));
		__m128 tmp1 = _mm_shuffle_ps(results, results, _MM_SHUFFLE(1, 0, 3, 2));
		__m128 tmp2 = _mm_max_ps(results, tmp1);
		__m128 tmp3 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(2, 3, 0, 1));
		dmax = _mm_max_ps(dmax, _mm_max_ps(tmp3, tmp2));
		PfxUInt32 mask = _mm_movemask_ps(_mm_cmpeq_ps(results, dmax));
		PfxUInt32 chk = __builtin_ffs(mask);
		if (chk > 0) {
			reti = i + (chk - 1);
		}
	}

	int rest = (int)mesh->m_numVerts - i;
	if (rest > 0) {
		PfxFloat d1 = dot(pfxReadVector3(mesh->m_verts + (i) * 3), seperatingAxis);
		PfxFloat d2 = rest > 1 ? dot(pfxReadVector3(mesh->m_verts + (i + 1) * 3), seperatingAxis) : -SCE_PFX_FLT_MAX;
		PfxFloat d3 = rest > 2 ? dot(pfxReadVector3(mesh->m_verts + (i + 2) * 3), seperatingAxis) : -SCE_PFX_FLT_MAX;
		__m128 results = _mm_set_ps(-SCE_PFX_FLT_MAX, d3, d2, d1);
		__m128 tmp1 = _mm_shuffle_ps(results, results, _MM_SHUFFLE(1, 0, 3, 2));
		__m128 tmp2 = _mm_max_ps(results, tmp1);
		__m128 tmp3 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(2, 3, 0, 1));
		dmax = _mm_max_ps(dmax, _mm_max_ps(tmp3, tmp2));
		PfxUInt32 mask = _mm_movemask_ps(_mm_cmpeq_ps(results, dmax));
		PfxUInt32 chk = __builtin_ffs(mask);
		if (chk > 0) {
			reti = i + (chk - 1);
		}
	}
#else
	PfxFloat dmax = dot(pfxReadVector3(mesh->m_verts), seperatingAxis);
	for (int i = 1; i<mesh->m_numVerts; i++) {
		PfxFloat d = dot(pfxReadVector3(mesh->m_verts + i * 3), seperatingAxis);
		if (d > dmax) {
			dmax = d;
			reti = i;
		}
	}
#endif
#endif

	supportVertex = pfxReadVector3(mesh->m_verts + reti * 3) + margin * seperatingAxis;

	return reti;
}

template<>
inline PfxUInt32 getSupportVertex<PfxTriangle>(const PfxTriangle *triangle, const PfxVector3 &seperatingAxis, PfxVector3 &supportVertex, PfxFloat margin)
{
	int reti = 0;

	PfxFloat d[3];
	d[0] = dot(triangle->points[0], seperatingAxis);
	d[1] = dot(triangle->points[1], seperatingAxis);
	d[2] = dot(triangle->points[2], seperatingAxis);

	if (d[reti] < d[1]) { reti = 1; }
	if (d[reti] < d[2]) { reti = 2; }

	supportVertex = triangle->points[reti] + margin * seperatingAxis;

	return reti;
}

template<>
inline PfxUInt32 getSupportVertex<PfxFatTriangle>(const PfxFatTriangle *fatTriangle, const PfxVector3 &seperatingAxis, PfxVector3 &supportVertex, PfxFloat margin)
{
	int reti = 0;

#ifdef PFX_ENABLE_AVX
	__m128 vSeperatingAxis = sce_vectormath_asm128(seperatingAxis.get128());
	__m128 sx = _mm_shuffle_ps(vSeperatingAxis, vSeperatingAxis, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 sy = _mm_shuffle_ps(vSeperatingAxis, vSeperatingAxis, _MM_SHUFFLE(1, 1, 1, 1));
	__m128 sz = _mm_shuffle_ps(vSeperatingAxis, vSeperatingAxis, _MM_SHUFFLE(2, 2, 2, 2));
	__m128 v1 = sce_vectormath_asm128(fatTriangle->points[0].get128());
	__m128 v2 = sce_vectormath_asm128(fatTriangle->points[1].get128());
	__m128 v3 = sce_vectormath_asm128(fatTriangle->points[2].get128());
	__m128 v4 = sce_vectormath_asm128(fatTriangle->points[3].get128());
	__m128 vxy1 = _mm_movelh_ps(v1, v2);
	__m128 vxy2 = _mm_movelh_ps(v3, v4);
	__m128 xxxx = _mm_shuffle_ps(vxy1, vxy2, _MM_SHUFFLE(2, 0, 2, 0));
	__m128 yyyy = _mm_shuffle_ps(vxy1, vxy2, _MM_SHUFFLE(3, 1, 3, 1));
	__m128 vzw1 = _mm_movehl_ps(v2, v1);
	__m128 vzw2 = _mm_movehl_ps(v4, v3);
	__m128 zzzz = _mm_shuffle_ps(vzw1, vzw2, _MM_SHUFFLE(2, 0, 2, 0));
	__m128 results = _mm_add_ps(_mm_add_ps(_mm_mul_ps(xxxx, sx), _mm_mul_ps(yyyy, sy)), _mm_mul_ps(zzzz, sz));
	__m128 tmp1 = _mm_shuffle_ps(results, results, _MM_SHUFFLE(1, 0, 3, 2));
	__m128 tmp2 = _mm_max_ps(results, tmp1);
	__m128 tmp3 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(2, 3, 0, 1));
	__m128 dmax = _mm_max_ps(tmp3, tmp2);
	PfxUInt32 mask = _mm_movemask_ps(_mm_cmpeq_ps(results, dmax));
	PfxUInt32 chk = __builtin_ffs(mask);
	if (chk > 0) {
		reti = chk - 1;
	}
#else
	PfxFloat d[4];
	d[0] = dot(fatTriangle->points[0], seperatingAxis);
	d[1] = dot(fatTriangle->points[1], seperatingAxis);
	d[2] = dot(fatTriangle->points[2], seperatingAxis);
	d[3] = dot(fatTriangle->points[3], seperatingAxis);

	if (d[reti] < d[1]) { reti = 1; }
	if (d[reti] < d[2]) { reti = 2; }
	if (d[reti] < d[3]) { reti = 3; }
#endif

	supportVertex = fatTriangle->points[reti] + margin * seperatingAxis;

	return reti;
}

template<>
inline PfxUInt32 getSupportVertex<PfxSphere>(const PfxSphere *sphere, const PfxVector3 &seperatingAxis, PfxVector3 &supportVertex, PfxFloat margin)
{
	supportVertex = seperatingAxis * (sphere->m_radius + margin);

	return 0;
}

template<>
inline PfxUInt32 getSupportVertex<PfxBox>(const PfxBox *box, const PfxVector3 &seperatingAxis, PfxVector3 &supportVertex, PfxFloat margin)
{
	PfxVector3 boxHalf = box->m_half + PfxVector3(margin);
	supportVertex = copySignPerElem(boxHalf, seperatingAxis);
	PfxUInt32 reti = 0;
	reti |= seperatingAxis[0] > 0.0f ? 1 : 0;
	reti |= seperatingAxis[1] > 0.0f ? 2 : 0;
	reti |= seperatingAxis[2] > 0.0f ? 4 : 0;
	return reti;
}

template<>
inline PfxUInt32 getSupportVertex<PfxCapsule>(const PfxCapsule *capsule, const PfxVector3 &seperatingAxis, PfxVector3 &supportVertex, PfxFloat margin)
{
	PfxVector3 u(1.0f,0.0f,0.0f);
	PfxFloat udotv = dot(seperatingAxis,u);
	PfxVector3 dir = u * (udotv > 0.0f ? capsule->m_halfLen : -capsule->m_halfLen);
	supportVertex = dir + seperatingAxis * (capsule->m_radius + margin);
	return -1;
}

template<>
inline PfxUInt32 getSupportVertex<PfxCylinder>(const PfxCylinder *cylinder, const PfxVector3 &seperatingAxis, PfxVector3 &supportVertex, PfxFloat margin)
{
	PfxVector3 u(1.0f,0.0f,0.0f);
	PfxFloat udotv = dot(seperatingAxis,u);
	PfxFloat s = seperatingAxis[1]*seperatingAxis[1]+seperatingAxis[2]*seperatingAxis[2];
	if(s < 0.000001f) {
		supportVertex = u * (udotv > 0.0f ? cylinder->m_halfLen + margin : -cylinder->m_halfLen-margin);
	}
	else {
		PfxVector3 dir = u * (udotv > 0.0f ? cylinder->m_halfLen : -cylinder->m_halfLen);
		PfxVector3 vYZ = seperatingAxis;
		vYZ[0] = 0.0f;
		vYZ *= rsqrtf(PfxFloatInVec(s));
		supportVertex = dir + vYZ * (cylinder->m_radius) + seperatingAxis * margin;
	}
	return -1;
}

template<>
inline PfxUInt32 getSupportVertex<PfxCircle>(const PfxCircle *circle, const PfxVector3 &separatingAxis, PfxVector3 &supportVector, PfxFloat margin)
{
	PfxVector3 pointOnCircleNormal = circle->m_normal * (dot(separatingAxis, circle->m_normal));
	PfxVector3 vecToCircleEdge = separatingAxis - pointOnCircleNormal;
	if (lengthSqr(vecToCircleEdge) < 0.00001f * 0.00001f)
	{
		supportVector = separatingAxis * (0.001f + margin);
	}
	else
	{
		supportVector = normalize(vecToCircleEdge) * circle->m_radius + separatingAxis * margin;
	}
	return -1;
}

// Center
template<typename PfxShapeType>
inline void getCenter(const PfxShapeType *shape, PfxVector3 &center)
{
	center = PfxVector3::zero();
}

template<>
inline void getCenter<PfxConvexMeshImpl>(const PfxConvexMeshImpl *mesh, PfxVector3 &center)
{
	PfxVector3 avg(0.0f);
	for (int i = 0; i<mesh->m_numVerts; i++) {
		avg += pfxReadVector3(mesh->m_verts + i * 3);
	}
	center = avg / (PfxFloat)mesh->m_numVerts;
}

template<>
inline void getCenter<PfxTriangle>(const PfxTriangle *triangle, PfxVector3 &center)
{
	center = ( triangle->points[0] + triangle->points[1] + triangle->points[2]) / 3.0f;
}

template<>
inline void getCenter<PfxFatTriangle>(const PfxFatTriangle *fatTriangle, PfxVector3 &center)
{
	center = ( fatTriangle->points[0] + fatTriangle->points[1] + fatTriangle->points[2]  + fatTriangle->points[3]) / 4.0f;
}

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_GJK_SUPPORT_FUNC_H
