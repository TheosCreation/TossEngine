/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2022 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_INTERSECT_COMMON_H
#define _SCE_PFX_INTERSECT_COMMON_H

#include "pfx_tri_mesh.h"
#include "../../../include/physics_effects/base_level/collision/pfx_ray.h"

#include "../../../include/physics_effects/base_level/collision/pfx_circle.h"
#include "pfx_ellipse.h"
#include "pfx_line_segment.h"

namespace sce {
namespace pfxv4 {

#define SCE_PFX_INTERSECT_COMMON_EPSILON 0.00001f
#define SCE_PFX_RAY_TRIANGLE_EPSILON 0.00001f

// Internally used intersect functions

struct PfxTriangle
{
	PfxVector3 points[3];
	
	PfxTriangle(const PfxVector3 &p0,const PfxVector3 &p1,const PfxVector3 &p2)
	{
		points[0] = p0;
		points[1] = p1;
		points[2] = p2;
	}

	PfxTriangle(const PfxPoint3 &p0,const PfxPoint3 &p1,const PfxPoint3 &p2)
	{
		points[0] = PfxVector3(p0);
		points[1] = PfxVector3(p1);
		points[2] = PfxVector3(p2);
	}

	PfxTriangle(const PfxFloat3 &p0,const PfxFloat3 &p1,const PfxFloat3 &p2)
	{
		points[0] = p0;
		points[1] = p1;
		points[2] = p2;
	}
	
	PfxVector3 calcNormal() const
	{
		return normalize(cross(points[1]-points[0],points[2]-points[0]));
	}
};

struct PfxFatTriangle
{
	PfxVector3 points[4];
};

struct PfxPlane
{
	PfxVector3 normal; // normal
	PfxVector3 point; // point on the plane
	
	PfxPlane(const PfxVector3 &n,const PfxVector3 &q)
	{
		normal = n;
		point = q;
	}
	
	PfxPlane(const PfxTriangle &triangle)
	{
		normal = triangle.calcNormal();
		point = (triangle.points[0] + triangle.points[1] + triangle.points[2])/3.0f;
	}
	
	PfxFloat onPlane(const PfxVector3 &p) const
	{
		return dot((p-point),normal);
	}
};

static SCE_PFX_FORCE_INLINE
PfxBool pfxIntersectRayAABBFast(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	const PfxVector3 &AABBmin,
	const PfxVector3 &AABBmax)
{
#ifdef PFX_ENABLE_AVX
	PfxVector3 rayEndPoint = rayStartPosition + rayDirection;
	PfxVector3 rayAabbMin = minPerElem(rayStartPosition, rayEndPoint);
	PfxVector3 rayAabbMax = maxPerElem(rayStartPosition, rayEndPoint);
	const __m128 vAabbMinA = sce_vectormath_asm128(rayAabbMin.get128());
	const __m128 vAabbMaxA = sce_vectormath_asm128(rayAabbMax.get128());
	const __m128 vAabbMinB = sce_vectormath_asm128(AABBmin.get128());
	const __m128 vAabbMaxB = sce_vectormath_asm128(AABBmax.get128());

	if ((_mm_movemask_ps(_mm_or_ps(_mm_cmplt_ps(vAabbMaxA, vAabbMinB),_mm_cmpgt_ps(vAabbMinA, vAabbMaxB))) & 0x07) != 0) return false;

	static const __m128 positiveEpsilon = sce_vectormath_asm128(PfxVector3(1e-5f).get128());
	static const __m128 negativeEpsilon = sce_vectormath_asm128(PfxVector3(-1e-5f).get128());
	const __m128 dir = sce_vectormath_asm128(rayDirection.get128());

	// Detect the class of ray's direction
	__m128 iGtZero = _mm_cmpgt_ps(dir, positiveEpsilon);
	__m128 iLtZero = _mm_cmplt_ps(dir, negativeEpsilon);
	__m128 iLtZeroShuffled = pfxShufflePs<1, 2, 0, 3>( iLtZero );

	__m128 rayToMin = _mm_sub_ps(sce_vectormath_asm128(AABBmin.get128()), sce_vectormath_asm128(rayStartPosition.get128()));
	__m128 rayToMax = _mm_sub_ps(sce_vectormath_asm128(AABBmax.get128()), sce_vectormath_asm128(rayStartPosition.get128()));
	__m128 rayToMinShuffled = pfxShufflePs<1, 2, 0, 3>( rayToMin );
	__m128 rayToMaxShuffled = pfxShufflePs<1, 2, 0, 3>( rayToMax );

	// The normal vector of ray's direction
	__m128 vDirVer = pfxShufflePs<1, 2, 0, 3>( dir );

	__m128 iSwap = _mm_xor_ps( iGtZero, iLtZeroShuffled );
	__m128 ip1 = _mm_or_ps( _mm_andnot_ps( iSwap, rayToMinShuffled ), _mm_and_ps( iSwap, rayToMaxShuffled ) );
	__m128 ip2 = _mm_or_ps( _mm_andnot_ps( iSwap, rayToMaxShuffled ), _mm_and_ps( iSwap, rayToMinShuffled ) );

	__m128 dotRes1 = (_mm_sub_ps(_mm_mul_ps(ip1, dir), _mm_mul_ps(rayToMin, vDirVer)));
	__m128 dotRes2 = (_mm_sub_ps(_mm_mul_ps(ip2, dir), _mm_mul_ps(rayToMax, vDirVer)));

	return ((_mm_movemask_ps(_mm_cmplt_ps(_mm_mul_ps(dotRes1, dotRes2), positiveEpsilon)) & 0x7) == 0x7);
#else
	PfxVector3 rayEndPoint = rayStartPosition + rayDirection;
	PfxVector3 rayAabbMin = minPerElem(rayStartPosition, rayEndPoint);
	PfxVector3 rayAabbMax = maxPerElem(rayStartPosition, rayEndPoint);

	if (rayAabbMax[0] < AABBmin[0] || rayAabbMin[0] > AABBmax[0]) return false;
	if (rayAabbMax[1] < AABBmin[1] || rayAabbMin[1] > AABBmax[1]) return false;
	if (rayAabbMax[2] < AABBmin[2] || rayAabbMin[2] > AABBmax[2]) return false;

	PfxVector3 rayToMin(AABBmin - rayStartPosition);
	PfxVector3 rayToMax(AABBmax - rayStartPosition);

	auto clipRayGetP1Elem2 = [](PfxFloat v1, PfxFloat v2, PfxFloat rangeMaxV2, PfxFloat rangeMinV2)
	{
		if ((v1 > 0.f && v2 > 0.f) || (v1 < 0.f && v2 < 0.f))	return rangeMaxV2;
		else													return rangeMinV2;
	};

	auto clipRayGetP2Elem2 = [](PfxFloat v1, PfxFloat v2, PfxFloat rangeMaxV2, PfxFloat rangeMinV2)
	{
		if ((v1 > 0.f && v2 > 0.f) || (v1 < 0.f && v2 < 0.f))	return rangeMinV2;
		else													return rangeMaxV2;
	};

	// xy-plane
	PfxVector3 normalDir(-rayDirection.getY(), rayDirection.getX(), 0.f);
	PfxVector3 p1(rayToMin.getX(), clipRayGetP1Elem2(rayDirection.getX(), rayDirection.getY(), rayToMax.getY(), rayToMin.getY()), 0.f);
	PfxVector3 p2(rayToMax.getX(), clipRayGetP2Elem2(rayDirection.getX(), rayDirection.getY(), rayToMax.getY(), rayToMin.getY()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	// yz-plane
	normalDir = PfxVector3(-rayDirection.getZ(), rayDirection.getY(), 0.f);
	p1 = PfxVector3(rayToMin.getY(), clipRayGetP1Elem2(rayDirection.getY(), rayDirection.getZ(), rayToMax.getZ(), rayToMin.getZ()), 0.f);
	p2 = PfxVector3(rayToMax.getY(), clipRayGetP2Elem2(rayDirection.getY(), rayDirection.getZ(), rayToMax.getZ(), rayToMin.getZ()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	// zx-plane
	normalDir = PfxVector3(-rayDirection.getX(), rayDirection.getZ(), 0.f);
	p1 = PfxVector3(rayToMin.getZ(), clipRayGetP1Elem2(rayDirection.getZ(), rayDirection.getX(), rayToMax.getX(), rayToMin.getX()), 0.f);
	p2 = PfxVector3(rayToMax.getZ(), clipRayGetP2Elem2(rayDirection.getZ(), rayDirection.getX(), rayToMax.getX(), rayToMin.getX()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	return true;
#endif
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxIntersectRayAABBFast(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	const PfxVector3 &AABBcenter,
	const PfxVector3 &AABBhalf,
	PfxFloat &variable)
{
	PfxVector3 AABBmin = AABBcenter - AABBhalf;
	PfxVector3 AABBmax = AABBcenter + AABBhalf;
	
	PfxVector3 dir = rayDirection;
	PfxVector3 absDir = absPerElem(dir);

	PfxVector3 t1 = divPerElem(AABBmin - rayStartPosition, dir);
	PfxVector3 t2 = divPerElem(AABBmax - rayStartPosition, dir);
	
	#ifdef PFX_ENABLE_AVX
	const __m128 v_rayStartPosition = sce_vectormath_asm128(rayStartPosition.get128());
	__m128 v_chk1 = _mm_cmplt_ps(sce_vectormath_asm128(absDir.get128()),_mm_set1_ps(SCE_PFX_INTERSECT_COMMON_EPSILON));
	__m128 v_chk2 = _mm_or_ps(
						_mm_cmplt_ps(v_rayStartPosition,sce_vectormath_asm128(AABBmin.get128())),
						_mm_cmpgt_ps(v_rayStartPosition,sce_vectormath_asm128(AABBmax.get128()))
					);
	if((_mm_movemask_ps(_mm_and_ps(v_chk1,v_chk2)) & 0x07) != 0) return false;
	__m128 v_t1 = _mm_blendv_ps(sce_vectormath_asm128(t1.get128()),_mm_set1_ps(-SCE_PFX_FLT_MAX),v_chk1);
	__m128 v_t2 = _mm_blendv_ps(sce_vectormath_asm128(t2.get128()),_mm_set1_ps( SCE_PFX_FLT_MAX),v_chk1);

	union {
		float x,y,z,w;
		__m128i i;
		__m128 f;
	} v_tmin,v_tmax,v_max_tmin,v_min_tmax;

	v_tmin.f = _mm_min_ps(v_t1,v_t2);
	v_tmax.f = _mm_max_ps(v_t1,v_t2);

	v_max_tmin.f = _mm_max_ps(_mm_max_ps(v_tmin.f,_mm_castsi128_ps(_mm_srli_si128(v_tmin.i,4))),_mm_castsi128_ps(_mm_srli_si128(v_tmin.i,8)));
	v_min_tmax.f = _mm_min_ps(_mm_min_ps(v_tmax.f,_mm_castsi128_ps(_mm_srli_si128(v_tmax.i,4))),_mm_castsi128_ps(_mm_srli_si128(v_tmax.i,8)));

	PfxFloat max_tmin = v_max_tmin.x;
	PfxFloat min_tmax = v_min_tmax.x;
	#else
	if(absDir[0] < SCE_PFX_INTERSECT_COMMON_EPSILON) {
		if(rayStartPosition[0] < AABBmin[0] || rayStartPosition[0] > AABBmax[0]) {
			return false;
		}
		t1[0] = -SCE_PFX_FLT_MAX;
		t2[0] =  SCE_PFX_FLT_MAX;
	}

	if(absDir[1] < SCE_PFX_INTERSECT_COMMON_EPSILON) {
		if(rayStartPosition[1] < AABBmin[1] || rayStartPosition[1] > AABBmax[1]) {
			return false;
		}
		t1[1] = -SCE_PFX_FLT_MAX;
		t2[1] =  SCE_PFX_FLT_MAX;
	}

	if(absDir[2] < SCE_PFX_INTERSECT_COMMON_EPSILON) {
		if(rayStartPosition[2] < AABBmin[2] || rayStartPosition[2] > AABBmax[2]) {
			return false;
		}
		t1[2] = -SCE_PFX_FLT_MAX;
		t2[2] =  SCE_PFX_FLT_MAX;
	}
	
	PfxFloat max_tmin = maxElem(minPerElem(t1,t2));
	PfxFloat min_tmax = minElem(maxPerElem(t1,t2));
	#endif
	
	if(max_tmin > min_tmax) return false;

	variable = max_tmin;

	return true;
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxIntersectRayAABB(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	const PfxVector3 &AABBcenter,
	const PfxVector3 &AABBhalf,
	PfxFloat &variable,
	PfxVector3 &normal)
{
	PfxVector3 AABBmin = AABBcenter - AABBhalf;
	PfxVector3 AABBmax = AABBcenter + AABBhalf;
	
	PfxVector3 dir = rayDirection;
	PfxVector3 absDir = absPerElem(dir);
	PfxVector3 sign = copySignPerElem(PfxVector3(1.0f),dir);

	// 始点がBoxの内側にあるか判定
	if( AABBmin[0] < rayStartPosition[0] && rayStartPosition[0] < AABBmax[0] &&
		AABBmin[1] < rayStartPosition[1] && rayStartPosition[1] < AABBmax[1] &&
		AABBmin[2] < rayStartPosition[2] && rayStartPosition[2] < AABBmax[2]) {
		return false;
	}

	PfxVector3 t1 = divPerElem(AABBmin - rayStartPosition, dir);
	PfxVector3 t2 = divPerElem(AABBmax - rayStartPosition, dir);

	#ifdef PFX_ENABLE_AVX
	const __m128 v_rayStartPosition = sce_vectormath_asm128(rayStartPosition.get128());
	__m128 v_chk1 = _mm_cmplt_ps(sce_vectormath_asm128(absDir.get128()),_mm_set1_ps(SCE_PFX_INTERSECT_COMMON_EPSILON));
	__m128 v_chk2 = _mm_or_ps(
						_mm_cmplt_ps(v_rayStartPosition,sce_vectormath_asm128(AABBmin.get128())),
						_mm_cmpgt_ps(v_rayStartPosition,sce_vectormath_asm128(AABBmax.get128()))
					);
	if((_mm_movemask_ps(_mm_and_ps(v_chk1,v_chk2)) & 0x07) != 0) return false;
	__m128 v_t1 = _mm_blendv_ps(sce_vectormath_asm128(t1.get128()),_mm_set1_ps(-SCE_PFX_FLT_MAX),v_chk1);
	__m128 v_t2 = _mm_blendv_ps(sce_vectormath_asm128(t2.get128()),_mm_set1_ps( SCE_PFX_FLT_MAX),v_chk1);
	t1.set128(sce_vectormath_asfloat4(v_t1));
	t2.set128(sce_vectormath_asfloat4(v_t2));
	#else
	if(absDir[0] < SCE_PFX_INTERSECT_COMMON_EPSILON) {
		if(rayStartPosition[0] < AABBmin[0] || rayStartPosition[0] > AABBmax[0]) {
			return false;
		}
		t1[0] = -SCE_PFX_FLT_MAX;
		t2[0] =  SCE_PFX_FLT_MAX;
	}

	if(absDir[1] < SCE_PFX_INTERSECT_COMMON_EPSILON) {
		if(rayStartPosition[1] < AABBmin[1] || rayStartPosition[1] > AABBmax[1]) {
			return false;
		}
		t1[1] = -SCE_PFX_FLT_MAX;
		t2[1] =  SCE_PFX_FLT_MAX;
	}

	if(absDir[2] < SCE_PFX_INTERSECT_COMMON_EPSILON) {
		if(rayStartPosition[2] < AABBmin[2] || rayStartPosition[2] > AABBmax[2]) {
			return false;
		}
		t1[2] = -SCE_PFX_FLT_MAX;
		t2[2] =  SCE_PFX_FLT_MAX;
	}
	#endif
	
	PfxVector3 tmin = minPerElem(t1,t2);
	PfxVector3 tmax = maxPerElem(t1,t2);
	
	normal = PfxVector3::zero();
	
	if(maxElem(tmin) > minElem(tmax)) return false;
	
	if(tmin[0] > tmin[1]) {
		if(tmin[0] > tmin[2]) {
			variable = tmin[0];
			normal[0] = -sign[0];
		}
		else {
			variable = tmin[2];
			normal[2] = -sign[2];
		}
	}
	else {
		if(tmin[1] > tmin[2]) {
			variable = tmin[1];
			normal[1] = -sign[1];
		}
		else {
			variable = tmin[2];
			normal[2] = -sign[2];
		}
	}

	return true;
}

static SCE_PFX_FORCE_INLINE
void pfxClosestPointLine(
	const PfxVector3 &point,
	const PfxVector3 &linePoint,
	const PfxVector3 &lineDirection,
	PfxVector3 &closestPoint)
{
	PfxFloat s = divf(dot(point - linePoint,lineDirection) , dot(lineDirection,lineDirection));
	s = SCE_PFX_CLAMP(s,0.0f,1.0f);
	closestPoint = linePoint + s * lineDirection;
}

#define SCE_PFX_INTERSECT_RESULT_NO_INTERSECT					PfxUInt8(0)
#define SCE_PFX_INTERSECT_RESULT_ONE_PLANE						PfxUInt8(1)
#define SCE_PFX_INTERSECT_RESULT_ONE_LINE						PfxUInt8(2)
#define SCE_PFX_INTERSECT_RESULT_TWO_LINES						PfxUInt8(3)
#define SCE_PFX_INTERSECT_RESULT_ONE_POINT						PfxUInt8(4)
#define SCE_PFX_INTERSECT_RESULT_TWO_POINTS						PfxUInt8(5)
#define SCE_PFX_INTERSECT_RESULT_ONE_POINT_OUTSIDE				PfxUInt8(8)
#define SCE_PFX_INTERSECT_RESULT_ONE_CIRCLE						PfxUInt8(16)
#define SCE_PFX_INTERSECT_RESULT_ONE_CIRCLE_OUTSIDE				PfxUInt8(17)
#define SCE_PFX_INTERSECT_RESULT_ONE_ELLIPSE					PfxUInt8(32)
#define SCE_PFX_INTERSECT_RESULT_ONE_ELLIPSE_PARTIALLY_OUTSIDE	PfxUInt8(64)
#define SCE_PFX_INTERSECT_RESULT_ONE_ELLIPSE_OUTSIDE			PfxUInt8(65)


/// Return values: 
///			SCE_PFX_INTERSECT_RESULT_NO_INTERSECT
///			SCE_PFX_INTERSECT_RESULT_ONE_LINE
///			SCE_PFX_INTERSECT_RESULT_ONE_POINT
///			SCE_PFX_INTERSECT_RESULT_ONE_POINT_OUTSIDE
static SCE_PFX_FORCE_INLINE
PfxUInt8 pfxIntersectLineAndPlane(
	const PfxVector3 &linePoint,
	const PfxVector3 &lineDirection,
	const PfxVector3 &planePoint,
	const PfxVector3 &planeNormal,
	PfxVector3 &intersectCenter)
{
	PfxFloat pn = dot(planePoint - linePoint, planeNormal);
	PfxFloat dn = dot(lineDirection, planeNormal);

	// If the line direction is perpendicular with the plane normal 
	if (fabs(dn) < SCE_PFX_INTERSECT_COMMON_EPSILON) {

		// If the line is on the plane
		if (fabs(pn) < SCE_PFX_INTERSECT_COMMON_EPSILON)		return SCE_PFX_INTERSECT_RESULT_ONE_LINE;
		else													return SCE_PFX_INTERSECT_RESULT_NO_INTERSECT;
	}

	// dot(lineP + lineD * t - planeP, planeN) = 0, solve for t
	PfxFloat t = pn / dn;
	intersectCenter = linePoint + lineDirection * t;

	return ((t >= 0.f && t <= 1.f) ? SCE_PFX_INTERSECT_RESULT_ONE_POINT : SCE_PFX_INTERSECT_RESULT_ONE_POINT_OUTSIDE);
}

/// Return values: 
///			SCE_PFX_INTERSECT_RESULT_NO_INTERSECT
///			SCE_PFX_INTERSECT_RESULT_ONE_LINE
///			SCE_PFX_INTERSECT_RESULT_ONE_POINT
///			SCE_PFX_INTERSECT_RESULT_ONE_PLANE
static SCE_PFX_FORCE_INLINE
PfxUInt8 pfxIntersectTriangleAndPlane(
	const PfxTriangle &triangle,
	const PfxVector3 &planePoint,
	const PfxVector3 &planeNormal,
	PfxVector3 &intersectPoint,
	PfxLineSegment &intersectLineSegment)
{
	PfxUInt32 numLines = 0u, numPoints = 0u;
	PfxUInt32 lineIds[3];
	PfxUInt32 pointIds[3];
	PfxVector3 intersects[3];
	PfxUInt32 vId[4] = { 0, 1, 2, 0 };

	for (PfxUInt32 i = 0; i < 3; ++i){
		PfxUInt8 res = pfxIntersectLineAndPlane(triangle.points[vId[i]], triangle.points[vId[i + 1]] - triangle.points[vId[i]], planePoint, planeNormal, intersects[i]);
		if (res == SCE_PFX_INTERSECT_RESULT_ONE_LINE)			lineIds[numLines++] = i;
		else if (res == SCE_PFX_INTERSECT_RESULT_ONE_POINT)		pointIds[numPoints++] = i;
	}
	
	// The plane overlaps the triangle
	if (numLines >= 2)
	{
		return SCE_PFX_INTERSECT_RESULT_ONE_PLANE;
	}

	// The triangle intersects the plane on one edge
	else if (numLines == 1)
	{
		intersectLineSegment.m_point1 = triangle.points[vId[lineIds[0]]];
		intersectLineSegment.m_point2 = triangle.points[vId[lineIds[0] + 1]];
		return SCE_PFX_INTERSECT_RESULT_ONE_LINE;
	}

	// The plane intersects the triangle on one edge and one vertex
	else if (numPoints == 3)
	{
		PfxVector3 v = intersects[1] - intersects[0];
		if (lengthSqr(v) < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
			intersectLineSegment.m_point1 = intersects[0];
			intersectLineSegment.m_point2 = intersects[2];
		}
		else {
			intersectLineSegment.m_point1 = intersects[0];
			intersectLineSegment.m_point2 = intersects[1];
		}
		return SCE_PFX_INTERSECT_RESULT_ONE_LINE;
	}

	// The plane intersects the triangle on two edge
	else if (numPoints == 2)
	{
		PfxVector3 v = intersects[pointIds[1]] - intersects[pointIds[0]];
		if (lengthSqr(v) < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
			intersectPoint = intersects[pointIds[0]];
			return SCE_PFX_INTERSECT_RESULT_ONE_POINT;
		}
		else {
			intersectLineSegment.m_point1 = intersects[pointIds[0]];
			intersectLineSegment.m_point2 = intersects[pointIds[1]];
			return SCE_PFX_INTERSECT_RESULT_ONE_LINE;
		}	
	}
	else if (numPoints == 1)
	{
		intersectPoint = intersects[pointIds[0]];
		return SCE_PFX_INTERSECT_RESULT_ONE_POINT;
	}

	return SCE_PFX_INTERSECT_RESULT_NO_INTERSECT;
}

/// Return values: 
///			SCE_PFX_INTERSECT_RESULT_NO_INTERSECT
///			SCE_PFX_INTERSECT_RESULT_ONE_POINT
///			SCE_PFX_INTERSECT_RESULT_ONE_CIRCLE
static SCE_PFX_FORCE_INLINE
PfxUInt8 pfxIntersectSphereAndPlane(
	const PfxVector3 &sphereCenter,
	PfxFloat sphereRadius,
	const PfxVector3 &planePoint,
	const PfxVector3 &planeNormal,
	PfxVector3 &intersectCenter,
	PfxCircle &intersectCircle)
{
	PfxFloat t = dot(planePoint - sphereCenter, planeNormal);
	if (fabs(t) < sphereRadius) {
		intersectCenter = sphereCenter + planeNormal * t;
		intersectCircle.m_radius = sqrtf(sphereRadius * sphereRadius - t * t);
		intersectCircle.m_normal = planeNormal;
		return SCE_PFX_INTERSECT_RESULT_ONE_CIRCLE;
	}
	else if (fabs(t) == sphereRadius) {
		intersectCenter = sphereCenter + planeNormal * t;
		intersectCircle.m_radius = 0.f;
		intersectCircle.m_normal = planeNormal;
		return SCE_PFX_INTERSECT_RESULT_ONE_POINT;
	}

	return SCE_PFX_INTERSECT_RESULT_NO_INTERSECT;
}

/// Return values: 
///			SCE_PFX_INTERSECT_RESULT_NO_INTERSECT
///			SCE_PFX_INTERSECT_RESULT_ONE_LINE
///			SCE_PFX_INTERSECT_RESULT_TWO_LINES
///			SCE_PFX_INTERSECT_RESULT_ONE_CIRCLE
///			SCE_PFX_INTERSECT_RESULT_ONE_CIRCLE_OUTSIDE
///			SCE_PFX_INTERSECT_RESULT_ONE_ELLIPSE
///			SCE_PFX_INTERSECT_RESULT_ONE_ELLIPSE_PARTIALLY_OUTSIDE
///			SCE_PFX_INTERSECT_RESULT_ONE_ELLIPSE_OUTSIDE
static SCE_PFX_FORCE_INLINE
PfxUInt8 pfxIntersectTubeAndPlane(
	const PfxVector3 &tubeP1,
	const PfxVector3 &tubeP2,
	PfxFloat tubeRadius,
	const PfxVector3 &planePoint,
	const PfxVector3 &planeNormal,
	PfxVector3 &intersectCenter,
	PfxEllipse &intersectEllipse,
	PfxLineSegment &intersectLineSegment1,
	PfxLineSegment &intersectLineSegment2)
{
	PfxVector3 tubeDir = tubeP2 - tubeP1;
	PfxFloat tubeLengthSqr = lengthSqr(tubeDir);
	PfxFloat tubeLength = sqrtf(tubeLengthSqr);

	PfxFloat pn = dot(planePoint - tubeP1, planeNormal);
	PfxFloat dn = dot(tubeDir, planeNormal);

	// The plane is parallel to the axis (i.e., planeNormal perpendicular to tubeDir)
	if (fabs(dn) < SCE_PFX_INTERSECT_COMMON_EPSILON) {

		// The plane does not intersect with the tube
		if (fabs(pn) > tubeRadius) {
			return SCE_PFX_INTERSECT_RESULT_NO_INTERSECT;
		}

		// The plane intersects the tube on only one line
		else if (fabs(pn) == tubeRadius) {
			intersectLineSegment1.m_point1 = tubeP1 + planeNormal * pn;
			intersectLineSegment1.m_point2 = tubeP2 + planeNormal * pn;
			return SCE_PFX_INTERSECT_RESULT_ONE_LINE;
		}

		// The plane intersects the tube on two lines
		else {
			PfxVector3 vecOnPlane = normalize(cross(planeNormal, tubeDir));  // The vector perpendicular to the planeNormal and tubeDir
			PfxFloat d = sqrtf(tubeRadius * tubeRadius - pn * pn);

			PfxVector3 vec1 = pn * planeNormal + vecOnPlane * d;
			PfxVector3 vec2 = pn * planeNormal - vecOnPlane * d;
			
			intersectLineSegment1.m_point1 = tubeP1 + vec1;
			intersectLineSegment1.m_point2 = tubeP2 + vec1;

			intersectLineSegment2.m_point1 = tubeP1 + vec2;
			intersectLineSegment2.m_point2 = tubeP2 + vec2;
			return SCE_PFX_INTERSECT_RESULT_TWO_LINES;
		}
	}

	// The intersection between tube's axis and the plane
	PfxFloat intersectCenterOnAxis = pn / dn; // 0~1
	intersectCenter = tubeP1 + tubeDir * intersectCenterOnAxis;

	// Compute the minor axis
	PfxVector3 minorAxis = cross(planeNormal, tubeDir);
	if (lengthSqr(minorAxis) < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {

		// The plane is perpendicular to the axis (i.e., planeNormal is parallel to tubeDir)
		minorAxis = cross(planeNormal, PfxVector3::zAxis());
		if (lengthSqr(minorAxis) < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON)
			minorAxis = cross(planeNormal, PfxVector3::yAxis());

		intersectEllipse.m_minorAxis = normalize(minorAxis);
		intersectEllipse.m_minorRadius = tubeRadius;
		intersectEllipse.m_majorAxis = cross(planeNormal, intersectEllipse.m_minorAxis);
		intersectEllipse.m_majorRadius = tubeRadius;

		if (intersectCenterOnAxis >= 0.f && intersectCenterOnAxis <= 1.f)
			return SCE_PFX_INTERSECT_RESULT_ONE_CIRCLE;
		
		else 
			return SCE_PFX_INTERSECT_RESULT_ONE_CIRCLE_OUTSIDE;
	}

	intersectEllipse.m_minorAxis = minorAxis / length(minorAxis);
	intersectEllipse.m_minorRadius = tubeRadius;

	// Compute the major axis
	intersectEllipse.m_majorAxis = cross(planeNormal, intersectEllipse.m_minorAxis);
	intersectEllipse.m_majorRadius = (tubeRadius * tubeLength / dn);

	SCE_PFX_ASSERT(fabs(length(intersectEllipse.m_majorAxis) - 1.f) < SCE_PFX_INTERSECT_COMMON_EPSILON);

	// Detect the status of ellipse
	PfxFloat majorRadiusOnAxis = dot(intersectEllipse.m_majorAxis, tubeDir) / tubeLengthSqr * intersectEllipse.m_majorRadius;
	PfxFloat majorP1OnAxis = intersectCenterOnAxis + majorRadiusOnAxis;
	PfxFloat majorP2OnAxis = intersectCenterOnAxis - majorRadiusOnAxis;
	
	if (majorP1OnAxis < 0.f && majorP2OnAxis < 0.f)			return SCE_PFX_INTERSECT_RESULT_ONE_ELLIPSE_OUTSIDE;
	if (majorP1OnAxis > 1.f && majorP2OnAxis > 1.f)			return SCE_PFX_INTERSECT_RESULT_ONE_ELLIPSE_OUTSIDE;

	if (majorP1OnAxis >= 0.f && majorP1OnAxis <= 1.f &&	majorP2OnAxis >= 0.f && majorP2OnAxis <= 1.f)
		return SCE_PFX_INTERSECT_RESULT_ONE_ELLIPSE;

	return SCE_PFX_INTERSECT_RESULT_ONE_ELLIPSE_PARTIALLY_OUTSIDE;
}


/// Return values: 
///			SCE_PFX_INTERSECT_RESULT_NO_INTERSECT
///			SCE_PFX_INTERSECT_RESULT_ONE_POINT
///			SCE_PFX_INTERSECT_RESULT_TWO_POINTS
///			SCE_PFX_INTERSECT_RESULT_ONE_CIRCLE
static SCE_PFX_FORCE_INLINE
PfxUInt8 pfxIntersectHollowCircleAndPlane(
	const PfxVector3 &circleCenter,
	const PfxCircle &circle,
	const PfxVector3 &planePoint,
	const PfxVector3 &planeNormal,
	PfxVector3 &intersectP1,
	PfxVector3 &intersectP2)
{
	PfxVector3 h = cross(circle.m_normal, planeNormal);

	// The circle is parallel to the plane
	if (lengthSqr(h) < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {

		// The circle is on the plane
		if (fabs(dot(normalize(circleCenter - planePoint), planeNormal)) < SCE_PFX_INTERSECT_COMMON_EPSILON){
			return SCE_PFX_INTERSECT_RESULT_ONE_CIRCLE;
		}

		// Otherwise, no intersection
		else
			return SCE_PFX_INTERSECT_RESULT_NO_INTERSECT;
	}

	h /= length(h);
	PfxVector3 v = cross(circle.m_normal, h);
	SCE_PFX_ASSERT(fabs(length(v) - 1.f) < SCE_PFX_INTERSECT_COMMON_EPSILON);

	PfxFloat pn = dot(planePoint - circleCenter, planeNormal);
	PfxFloat vn = dot(v, planeNormal);
	SCE_PFX_ASSERT(fabs(vn) > SCE_PFX_INTERSECT_COMMON_EPSILON);

	PfxFloat t = pn / vn;
	
	if (t > circle.m_radius) {
		return SCE_PFX_INTERSECT_RESULT_NO_INTERSECT;
	}

	else if (t == circle.m_radius) {
		intersectP1 = circleCenter + v * t;
		return SCE_PFX_INTERSECT_RESULT_ONE_POINT;
	}

	else {
		PfxFloat s = sqrtf(circle.m_radius * circle.m_radius - t * t);
		intersectP1 = circleCenter + v * t + h * s;
		intersectP2 = circleCenter + v * t - h * s;
		return SCE_PFX_INTERSECT_RESULT_TWO_POINTS;
	}
}

/// Return values: 
///			SCE_PFX_INTERSECT_RESULT_NO_INTERSECT
///			SCE_PFX_INTERSECT_RESULT_ONE_POINT
///			SCE_PFX_INTERSECT_RESULT_ONE_LINE
///			SCE_PFX_INTERSECT_RESULT_ONE_PLANE
static SCE_PFX_FORCE_INLINE
PfxUInt8 pfxIntersectFilledCircleAndPlane(
	const PfxVector3 &circleCenter,
	const PfxCircle &circle,
	const PfxVector3 &planePoint,
	const PfxVector3 &planeNormal,
	PfxVector3 &intersectPoint,
	PfxLineSegment &intersectLineSegment)
{
	PfxVector3 h = cross(circle.m_normal, planeNormal);

	// The circle is parallel to the plane
	if (lengthSqr(h) < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {

		// The circle is on the plane
		if (fabs(dot(normalize(circleCenter - planePoint), planeNormal)) < SCE_PFX_INTERSECT_COMMON_EPSILON){
			return SCE_PFX_INTERSECT_RESULT_ONE_PLANE;
		}

		// Otherwise, no intersection
		else
			return SCE_PFX_INTERSECT_RESULT_NO_INTERSECT;
	}

	h /= length(h);
	PfxVector3 v = cross(circle.m_normal, h);
	SCE_PFX_ASSERT(fabs(length(v) - 1.f) < SCE_PFX_INTERSECT_COMMON_EPSILON);

	PfxFloat pn = dot(planePoint - circleCenter, planeNormal);
	PfxFloat vn = dot(v, planeNormal);
	SCE_PFX_ASSERT(fabs(vn) > SCE_PFX_INTERSECT_COMMON_EPSILON);

	PfxFloat t = pn / vn;

	if (t > circle.m_radius) {
		return SCE_PFX_INTERSECT_RESULT_NO_INTERSECT;
	}

	else if (t == circle.m_radius) {
		intersectPoint = circleCenter + v * t;
		return SCE_PFX_INTERSECT_RESULT_ONE_POINT;
	}

	else {
		PfxFloat s = sqrtf(circle.m_radius * circle.m_radius - t * t);
		intersectLineSegment.m_point1 = circleCenter + v * t + h * s;
		intersectLineSegment.m_point2 = circleCenter + v * t - h * s;
		return SCE_PFX_INTERSECT_RESULT_ONE_LINE;
	}
}


static SCE_PFX_FORCE_INLINE
PfxBool pfxClosestPointOnClippedTriangle(
	const PfxVector3 &rayStartPoint,
	const PfxVector3 &normalizedRayDir,
	const PfxTriangle &triangle,
	PfxVector3 &closestPoint)
{
	PfxVector3 intersectPoint;
	PfxLineSegment intersectLineSegment;

	PfxUInt8 res = pfxIntersectTriangleAndPlane(triangle, rayStartPoint, normalizedRayDir, intersectPoint, intersectLineSegment);

	// Intersect on one point
	if (res == SCE_PFX_INTERSECT_RESULT_ONE_POINT) {
		closestPoint = intersectPoint;
		return true;
	}

	// Intersect on one line
	else if (res == SCE_PFX_INTERSECT_RESULT_ONE_LINE) {
		pfxClosestPointLine(rayStartPoint, intersectLineSegment.m_point1, intersectLineSegment.m_point2 - intersectLineSegment.m_point1, closestPoint);
		return true;
	}

	return false;
}

static SCE_PFX_FORCE_INLINE
void pfxClosestTwoLines(
	const PfxVector3 &p1,const PfxVector3 &q1, // line1
	const PfxVector3 &p2,const PfxVector3 &q2, // line2
	PfxVector3 &s1,PfxVector3 &s2)
{
#if 0
	PfxVector3 v1 = q1 - p1;
	PfxVector3 v2 = q2 - p2;
	PfxVector3 r = p1 - p2;

	PfxFloat a = dot(v1,v1);
	PfxFloat e = dot(v2,v2);
	PfxFloat f = dot(v2,r);
	PfxFloat b = dot(v1,v2);
	PfxFloat c = dot(v1,r);

	PfxFloat den = a*e-b*b;
	
	PfxFloat s,t;
	
	if(den != 0.0f) {
		s = SCE_PFX_CLAMP((b*f-c*e)/den,0.0f,1.0f);
	}
	else {
		s = 0.0f;
	}
	
	t = (b*s+f)/e;
	
	if(t < 0.0f) {
		t = 0.0f;
		s = SCE_PFX_CLAMP(-c/a,0.0f,1.0f);
	}
	else if(t > 1.0f) {
		t = 1.0f;
		s = SCE_PFX_CLAMP((b-c)/a,0.0f,1.0f);
	}
	
	s1 = p1 + s * v1;
	s2 = p2 + t * v2;
#else
	PfxVector3 v1 = q1 - p1;
	PfxVector3 v2 = q2 - p2;
	PfxVector3 r  = p1 - p2;

	PfxFloat a = dot(v1,v1);
	PfxFloat b = dot(v1,v2);
	PfxFloat c = dot(v2,v2);
	PfxFloat d = dot(v1,r);
	PfxFloat e = dot(v2,r);
	PfxFloat det = -a*c+b*b;
	PfxFloat s=0.0f,t=0.0f;

	if(det*det > SCE_PFX_INTERSECT_COMMON_EPSILON) {
		s = (c*d-b*e)/det;
	}

	s = SCE_PFX_CLAMP(s,0.0f,1.0f);
	t = (e+s*b)/c;
	t = SCE_PFX_CLAMP(t,0.0f,1.0f);
	s = (-d+t*b)/a;
	s = SCE_PFX_CLAMP(s,0.0f,1.0f);

	s1 = p1 + s * v1;
	s2 = p2 + t * v2;
#endif
}

// It returns a point if a point is inside of a box.
// It returns a closest point on a box if a point is outside of a box.
static SCE_PFX_FORCE_INLINE
void pfxClosestPointAABB(const PfxVector3 &point, const PfxVector3 &AABBhalf, PfxVector3 &s)
{
	s = point;
	s = maxPerElem(s,-AABBhalf);
	s = minPerElem(s,AABBhalf);
}

// Check if a point is inside of a box. (=true)
static SCE_PFX_FORCE_INLINE
PfxBool pfxPointTestAABB(const PfxVector3 &point, const PfxVector3 &AABBhalf)
{
    return allElemLessThan(-AABBhalf, point) && allElemLessThan(point, AABBhalf);
}

static SCE_PFX_FORCE_INLINE
void pfxClosestPointOnAABBSurface(const PfxVector3 &point, const PfxVector3 &AABBhalf, PfxVector3 &s)
{
	if(pfxPointTestAABB(point, AABBhalf)) {
		// If a point is inside of a box. a closest point on the surface of a box will be calculated.
		PfxVector3 pointSignsPerElem = copySignPerElem(PfxVector3(1.f),point);
	    PfxVector3 absPoint = mulPerElem(point, pointSignsPerElem);
	    PfxVector3 distanceFromSurface = AABBhalf - absPoint;
		PfxFloatInVec minDistance = minElem( distanceFromSurface );

	#if defined(PFX_ENABLE_AVX) && 0 // BUG: The following codes have to be fixed because the absDelta may contain multiple non-zero elements
		vec_uint4 minMask = sce_vectormath_cmpeq( distanceFromSurface.get128(), minDistance.get128() );
		PfxVector3 absDelta( sce_vectormath_and( minDistance.get128(), minMask ) );
	#else
		PfxVector3 absDelta(0.f);
		PfxBool nonZeroElemExisted = false;
		if (distanceFromSurface[0] <= minDistance.getAsFloat()) { absDelta[0] = minDistance.getAsFloat(); nonZeroElemExisted = true; }
		if (distanceFromSurface[1] <= minDistance.getAsFloat() && !nonZeroElemExisted) { absDelta[1] = minDistance.getAsFloat(); nonZeroElemExisted = true; }
		if (distanceFromSurface[2] <= minDistance.getAsFloat() && !nonZeroElemExisted) { absDelta[2] = minDistance.getAsFloat(); nonZeroElemExisted = true; }
	#endif
		s = point + mulPerElem(pointSignsPerElem, absDelta);
	}
	else {
		pfxClosestPointAABB(point, AABBhalf, s);
	}
}

static SCE_PFX_FORCE_INLINE
PfxUInt32 pfxClosestPointTriangle(
	const PfxVector3 &point,
	const PfxTriangle &triangle,
	PfxVector3 &s)
{
	/*
		PfxUInt32 feature = (result & 0xffff0000) >> 16; // 0:facet 1:edge 2:vertex
		PfxUInt32 featureId = result & 0x0000ffff; // index of feature
	*/
#if 1
	PfxVector3 a = triangle.points[0];
	PfxVector3 b = triangle.points[1];
	PfxVector3 c = triangle.points[2];
	PfxVector3 ab = b - a;
	PfxVector3 ac = c - a;
	PfxVector3 ap = point - a;
	PfxFloat d1 = dot(ab, ap);
	PfxFloat d2 = dot(ac, ap);
	if(d1 <= 0.0f && d2 <= 0.0f) {
		s = a;
		return 2<<16; // v 2:0
	}

	PfxVector3 bp = point - b;
	PfxFloat d3 = dot(ab, bp);
	PfxFloat d4 = dot(ac, bp);
	if (d3 >= 0.0f && d4 <= d3) {
		s = b;
		return (2<<16)|1; // v 2:1
	}

	PfxFloat vc = d1*d4 - d3*d2;
	if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
		PfxFloat v = d1 / (d1 - d3);
		s = a + v * ab;
		return 1<<16; // e 1:0
	}

	PfxVector3 cp = point - c;
	PfxFloat d5 = dot(ab, cp);
	PfxFloat d6 = dot(ac, cp);
	if (d6 >= 0.0f && d5 <= d6) {
		s = c;
		return (2<<16)|2; // v 2:2
	}

	PfxFloat vb = d5*d2 - d1*d6;
	if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
		PfxFloat w = d2 / (d2 - d6);
		s = a + w * ac;
		return (1<<16)|2; // e 1:2
	}

	PfxFloat va = d3*d6 - d5*d4;
	if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
		PfxFloat w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		s = b + w * (c - b);
		return (1<<16)|1; // e 1:1
	}

	PfxFloat den = 1.0f / (va + vb + vc);
	PfxFloat v = vb * den;
	PfxFloat w = vc * den;
	s = a + ab * v + ac * w;
	return 0;
#else
	// ３角形面上の投影点
	PfxVector3 normal = normalize(cross(triangle.points[1]-triangle.points[0],triangle.points[2]-triangle.points[0]));
	PfxVector3 proj = point - dot(normal,point - triangle.points[0]) * normal;
	
	// エッジP0,P1のボロノイ領域
	PfxVector3 edgeP01 = triangle.points[1] - triangle.points[0];
	PfxVector3 edgeP01_normal = cross(edgeP01,normal);
	
	PfxFloat voronoiEdgeP01_check1 = dot(proj - triangle.points[0],edgeP01_normal);
	PfxFloat voronoiEdgeP01_check2 = dot(proj - triangle.points[0],edgeP01);
	PfxFloat voronoiEdgeP01_check3 = dot(proj - triangle.points[1],-edgeP01);
	
	if(voronoiEdgeP01_check1 > 0.0f && voronoiEdgeP01_check2 > 0.0f && voronoiEdgeP01_check3 > 0.0f) {
		pfxClosestPointLine(triangle.points[0],edgeP01,proj,s);
		return 1<<16; // e 1:0
	}
	
	// エッジP1,P2のボロノイ領域
	PfxVector3 edgeP12 = triangle.points[2] - triangle.points[1];
	PfxVector3 edgeP12_normal = cross(edgeP12,normal);
	
	PfxFloat voronoiEdgeP12_check1 = dot(proj - triangle.points[1],edgeP12_normal);
	PfxFloat voronoiEdgeP12_check2 = dot(proj - triangle.points[1],edgeP12);
	PfxFloat voronoiEdgeP12_check3 = dot(proj - triangle.points[2],-edgeP12);
	
	if(voronoiEdgeP12_check1 > 0.0f && voronoiEdgeP12_check2 > 0.0f && voronoiEdgeP12_check3 > 0.0f) {
		pfxClosestPointLine(triangle.points[1],edgeP12,proj,s);
		return (1<<16)|1; // e 1:1
	}
	
	// エッジP2,P0のボロノイ領域
	PfxVector3 edgeP20 = triangle.points[0] - triangle.points[2];
	PfxVector3 edgeP20_normal = cross(edgeP20,normal);
	
	PfxFloat voronoiEdgeP20_check1 = dot(proj - triangle.points[2],edgeP20_normal);
	PfxFloat voronoiEdgeP20_check2 = dot(proj - triangle.points[2],edgeP20);
	PfxFloat voronoiEdgeP20_check3 = dot(proj - triangle.points[0],-edgeP20);
	
	if(voronoiEdgeP20_check1 > 0.0f && voronoiEdgeP20_check2 > 0.0f && voronoiEdgeP20_check3 > 0.0f) {
		pfxClosestPointLine(triangle.points[2],edgeP20,proj,s);
		return (1<<16)|2; // e 1:2
	}
	
	// ３角形面の内側
	if(voronoiEdgeP01_check1 <= 0.0f && voronoiEdgeP12_check1 <= 0.0f && voronoiEdgeP20_check1 <= 0.0f) {
		s = proj;
		return 0;
	}
	
	// 頂点P0のボロノイ領域
	if(voronoiEdgeP01_check2 <= 0.0f && voronoiEdgeP20_check3 <= 0.0f) {
		s =  triangle.points[0];
		return 2<<16; // v 2:0
	}
	
	// 頂点P1のボロノイ領域
	if(voronoiEdgeP01_check3 <= 0.0f && voronoiEdgeP12_check2 <= 0.0f) {
		s = triangle.points[1];
		return (2<<16)|1; // v 2:1
	}
	
	// 頂点P2のボロノイ領域
	if(voronoiEdgeP20_check2 <= 0.0f && voronoiEdgeP12_check3 <= 0.0f) {
		s = triangle.points[2];
		return (2<<16)|2; // v 2:2
	}
	
	return 0;
#endif
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxIntersectRayTriangleFrontAndBack(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	const PfxTriangle &triangle,
	PfxFloat &variable)
{
	PfxFloatInVec v,w;
	PfxVector3 ab = triangle.points[1] - triangle.points[0];
	PfxVector3 ac = triangle.points[2] - triangle.points[0];

	PfxVector3 n = cross(ab,ac);

	PfxFloatInVec d = dot(-rayDirection,n);
	
	if(fabsf(d) < 0.00001f) return false;

	PfxVector3 ap = rayStartPosition - triangle.points[0];
	PfxFloatInVec t = divf(dot(ap, n), d);

	if(t < 0.0f || t > 1.0f) return false;

	variable = t;

	PfxVector3 e = cross(-rayDirection,ap);
	v = divf(dot(ac,e), d);
	if(v < -SCE_PFX_RAY_TRIANGLE_EPSILON || v > 1.0f+SCE_PFX_RAY_TRIANGLE_EPSILON) return false;

	w = -divf(dot(ab, e), d);
	if(w < -SCE_PFX_RAY_TRIANGLE_EPSILON || v+w > 1.0f+SCE_PFX_RAY_TRIANGLE_EPSILON) return false;

	return true;
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxIntersectRayTriangleWithoutFrontFace(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	const PfxTriangle &triangle,
	PfxFloat &variable)
{
	PfxFloatInVec v,w;
	PfxVector3 ab = triangle.points[1] - triangle.points[0];
	PfxVector3 ac = triangle.points[2] - triangle.points[0];

	PfxVector3 n = cross(ab,ac);

	PfxFloatInVec d = dot(-rayDirection,n);
	
	if(d >= 0.0f) return false;

	PfxVector3 ap = rayStartPosition - triangle.points[0];
	PfxFloatInVec t = divf(dot(ap,n),d);

	if(t < 0.0f || t > 1.0f) return false;

	variable = t;

	PfxVector3 e = cross(-rayDirection,ap);
	v = divf(dot(ac,e), d);
	if(v < -SCE_PFX_RAY_TRIANGLE_EPSILON || v > 1.0f+SCE_PFX_RAY_TRIANGLE_EPSILON) return false;

	w = -divf(dot(ab,e), d);
	if(w < -SCE_PFX_RAY_TRIANGLE_EPSILON || v+w > 1.0f+SCE_PFX_RAY_TRIANGLE_EPSILON) return false;

	return true;
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxIntersectRayTriangleWithoutBackFace(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	const PfxTriangle &triangle,
	PfxFloat &variable)
{
	PfxFloatInVec v,w;
	PfxVector3 ab = triangle.points[1] - triangle.points[0];
	PfxVector3 ac = triangle.points[2] - triangle.points[0];

	PfxVector3 n = cross(ab,ac);

	PfxFloatInVec d = dot(-rayDirection,n);
	
	if(d <= 0.0f) return false;

	PfxVector3 ap = rayStartPosition - triangle.points[0];
	PfxFloatInVec t = divf(dot(ap, n), d);

	if(t < 0.0f || t > 1.0f) return false;

	variable = t;

	PfxVector3 e = cross(-rayDirection,ap);
	v = divf(dot(ac, e), d);
	if(v < -SCE_PFX_RAY_TRIANGLE_EPSILON || v > 1.0f+SCE_PFX_RAY_TRIANGLE_EPSILON) return false;

	w = -divf(dot(ab, e), d);
	if(w < -SCE_PFX_RAY_TRIANGLE_EPSILON || v+w > 1.0f+SCE_PFX_RAY_TRIANGLE_EPSILON) return false;

	return true;
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxIntersectRayTriangle(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	PfxUInt32 facetMode,
	const PfxTriangle &triangle,
	PfxFloat &variable)
{
	if(facetMode == SCE_PFX_RAY_FACET_MODE_FRONT_ONLY && 
		pfxIntersectRayTriangleWithoutBackFace(rayStartPosition,rayDirection,triangle,variable)) {
		return true;
	}
	else if(facetMode == SCE_PFX_RAY_FACET_MODE_BACK_ONLY && 
		pfxIntersectRayTriangleWithoutFrontFace(rayStartPosition,rayDirection,triangle,variable)) {
		return true;
	}
	else if(facetMode == SCE_PFX_RAY_FACET_MODE_FRONT_AND_BACK && 
		pfxIntersectRayTriangleFrontAndBack(rayStartPosition,rayDirection,triangle,variable)) {
		return true;
	}
	return false;
}

PfxBool pfxIntersectRaySphere(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	const PfxVector3 &spherePosition,
	PfxFloat sphereRadius,
	PfxFloat &variable);

PfxBool pfxIntersectRayCapsule(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	const PfxVector3 &capsuleP1,
	const PfxVector3 &capsuleP2,
	PfxFloat capsuleRadius,
	PfxFloat &variable);

void pfxClosestPointCylinder(
	const PfxVector3 &position,
	const PfxVector3 &cylinderP1,
	const PfxVector3 &cylinderP2,
	PfxFloat cylinderRadius,
	PfxVector3 &s);

void pfxClosestPointAndNormalOnCylinder(
	const PfxVector3 &position,
	const PfxVector3 &cylinderP1,
	const PfxVector3 &cylinderP2,
	PfxFloat cylinderRadius,
	PfxVector3 &p,
	PfxVector3 &n);

PfxBool pfxIntersectRayCylinder(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	const PfxVector3 &cylinderP1,
	const PfxVector3 &cylinderP2,
	PfxFloat cylinderRadius,
	PfxFloat &variable);

template <class CapsuleInput>
PfxVector3 calcNormalOnCapsule(const PfxVector3 &pointOnCapsule, const CapsuleInput &capsuleIn)
{
	PfxVector3 pointLocal = rotate(conj(capsuleIn.m_orientation), pointOnCapsule - capsuleIn.m_startPosition);
	
	return calcNormalOnCapsuleLocal(pointLocal, capsuleIn);
}

template <class CapsuleInput>
PfxVector3 calcNormalOnCapsuleLocal(const PfxVector3 &pointLocal, const CapsuleInput &capsuleIn)
{
	PfxVector3 normal(0.0f, 0.0f, 1.0f);

	if (fabsf(pointLocal[1]) < SCE_PFX_INTERSECT_COMMON_EPSILON && fabsf(pointLocal[2]) < SCE_PFX_INTERSECT_COMMON_EPSILON) {
		return normal;
	}

	if (fabsf(pointLocal[0]) <= capsuleIn.m_halfLength) {
		// point is on the body of a capsule
		normal = normalize(PfxVector3(0.0f, pointLocal[1], pointLocal[2]));
	}
	else if (pointLocal[0] > capsuleIn.m_halfLength) {
		// point is on the hemisphere(+X) of a capsule
		normal = normalize(pointLocal - PfxVector3(capsuleIn.m_halfLength, 0.0f, 0.0f));
	}
	else if (pointLocal[0] < capsuleIn.m_halfLength) {
		// point is on the hemisphere(-X) of a capsule
		normal = normalize(pointLocal - PfxVector3(-capsuleIn.m_halfLength, 0.0f, 0.0f));
	}

	return rotate(capsuleIn.m_orientation, normal);
}

template <class CapsuleInput>
PfxFloat calcRadiusOfSweptCapsule(const CapsuleInput &capsuleIn)
{
	PfxVector3 capL = rotate(capsuleIn.m_orientation, PfxVector3(capsuleIn.m_halfLength, 0.0f, 0.0f));
	PfxVector3 projL = dot(capL, capsuleIn.m_direction) / dot(capsuleIn.m_direction, capsuleIn.m_direction) * capsuleIn.m_direction;
	PfxFloat l1 = length(capL - projL);
	PfxFloat l2 = length(projL);
	return SCE_PFX_MAX(l1, l2) + capsuleIn.m_radius;
}

// An arbitrary point is translated to a point on the surface of a capsule
// If a point is inside of a capsule, it returns an input point.
template <class CapsuleInput>
void calcPointAndNormalOnCapsule(PfxVector3 &point, PfxVector3 &normal, const CapsuleInput &capsuleIn)
{
	PfxVector3 capsuleP1 = capsuleIn.m_startPosition + rotate(capsuleIn.m_orientation, PfxVector3(-capsuleIn.m_halfLength, 0.0f, 0.0f));
	PfxVector3 capsuleP2 = capsuleIn.m_startPosition + rotate(capsuleIn.m_orientation, PfxVector3( capsuleIn.m_halfLength, 0.0f, 0.0f));
	PfxVector3 capDir(capsuleP2 - capsuleP1);
	PfxFloat chk1 = dot(capDir, point - capsuleP2);
	PfxFloat chk2 = dot(-capDir, point - capsuleP1);
	if (chk1 > 0.0f) {
		PfxFloat distanceSqr = lengthSqr(point - capsuleP2);
		if(distanceSqr < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
			normal = rotate(capsuleIn.m_orientation, PfxVector3::zAxis());
		}
		else {
			normal = normalize(point - capsuleP2);
		}
		
		if(distanceSqr > capsuleIn.m_radius * capsuleIn.m_radius) {
			point = capsuleP2 + capsuleIn.m_radius * normal;
		}
	}
	else if (chk2 > 0.0f) {
		PfxFloat distanceSqr = lengthSqr(point - capsuleP1);
		if(distanceSqr < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
			normal = rotate(capsuleIn.m_orientation, PfxVector3::zAxis());
		}
		else {
			normal = normalize(point - capsuleP1);
		}

		if(distanceSqr > capsuleIn.m_radius * capsuleIn.m_radius) {
			point = capsuleP1 + capsuleIn.m_radius * normal;
		}
	}
	else {
		PfxVector3 v = point - capsuleP1;
		PfxVector3 d = normalize(capDir);
		PfxVector3 q = dot(v, d) * d;
		
		normal = v - q;
		PfxFloat distanceSqr = lengthSqr(normal);
		if (distanceSqr < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
			normal = rotate(capsuleIn.m_orientation, PfxVector3::zAxis());
		}
		else {
			normal = normalize(normal);
		}
		
		if(distanceSqr > capsuleIn.m_radius * capsuleIn.m_radius) {
			point = capsuleP1 + q + capsuleIn.m_radius * normal;
		}
	}
}

static inline PfxTransform3 pfxRemoveScale(const PfxTransform3 &trns, PfxFloat scale)
{
	PfxFloatInVec scaleInv(1.0f/scale);
	return PfxTransform3(
		trns.getCol0() * scaleInv,
		trns.getCol1() * scaleInv,
		trns.getCol2() * scaleInv,
		trns.getCol3());
}

static inline PfxFloat pfxDistancePointLine(const PfxVector3 &targetPosition, const PfxVector3 &startPosition, const PfxVector3 &dir)
{
	PfxFloat vv = dot(dir, dir);

	if (fabsf(vv) < SCE_PFX_INTERSECT_COMMON_EPSILON) {
		return length(targetPosition - startPosition);
	}

	PfxFloat t = dot((targetPosition - startPosition), dir) / vv;
	PfxVector3 closestPointOnLine = startPosition + t * dir;

	return length(targetPosition - closestPointOnLine);
}

static inline PfxFloat pfxDistanceTriangleLine(const PfxTriangle &triangle, const PfxVector3 &startPosition, const PfxVector3 &dir)
{
	PfxVector3 n = cross(triangle.points[1]-triangle.points[0],triangle.points[2]-triangle.points[0]);
	PfxFloat vn = dot(dir, n);
	
	if (fabsf(vn) < SCE_PFX_INTERSECT_COMMON_EPSILON) {
		return dot((startPosition - triangle.points[0]), n) / length(n);
	}
	
	PfxFloat t = dot((triangle.points[0] - startPosition), n) / vn;
	PfxVector3 pointOnPlane = startPosition + t * dir;
	
	PfxVector3 closestPointOnTriangle;

	pfxClosestPointTriangle(pointOnPlane, triangle, closestPointOnTriangle);
	
	return length(pointOnPlane - closestPointOnTriangle);
}

static inline PfxBool pfxDistanceTriangleSegment(const PfxVector3 &startPosition, const PfxVector3 &dir, const PfxTriangle &triangle, PfxFloat &distance)
{
	PfxVector3 ab = triangle.points[1] - triangle.points[0];
	PfxVector3 ac = triangle.points[2] - triangle.points[0];

	PfxVector3 n = cross(ab, ac);

	PfxFloat d = dot(-dir, n);

	if (fabsf(d) < SCE_PFX_INTERSECT_COMMON_EPSILON) { // this segment is parallel to a triangle
		PfxVector3 chk;
		chk[0] = dot(dir, triangle.points[0] - startPosition);
		chk[1] = dot(dir, triangle.points[1] - startPosition);
		chk[2] = dot(dir, triangle.points[2] - startPosition);
		if(minElem(chk) > 1.0f || maxElem(chk) < 0.0f) {
			return false;
		}
		distance = fabsf(dot((startPosition - triangle.points[0]), n) / length(n));
		return true;
	}

	PfxVector3 ap = startPosition - triangle.points[0];
	PfxFloat t = dot(ap, n) / d;

	//if (t < 0.0f || t > 1.0f) return false;
	t = SCE_PFX_CLAMP(t, 0.0f, 1.0f);

	PfxVector3 pointOnSegment = startPosition + t * dir;

	PfxVector3 closestPointOnTriangle;

	pfxClosestPointTriangle(pointOnSegment, triangle, closestPointOnTriangle);

	distance = length(pointOnSegment - closestPointOnTriangle);

	return true;
}

static inline PfxUInt32 pfxCalcDirectionIndex(const PfxVector3 &axis)
{
	int face = 0; // X
	PfxFloat f, maxf;

	f = axis[0];
	maxf = f; face = 0;
	f = -axis[0];
	if (maxf < f) { maxf = f; face = 1; }
	f = axis[1];
	if (maxf < f) { maxf = f; face = 2; }
	f = -axis[1];
	if (maxf < f) { maxf = f; face = 3; }
	f = axis[2];
	if (maxf < f) { maxf = f; face = 4; }
	f = -axis[2];
	if (maxf < f) { maxf = f; face = 5; }

	const float signTbl[2][6] = {
		{ -1.0f,-1.0f,1.0f,-1.0f,1.0f,1.0f },
		{ -1.0f,1.0f,1.0f,1.0f,-1.0f,1.0f },
	};
	const int idxTbl[2][6] = {
		{ 2,2,0,0,0,0 },
		{ 1,1,2,2,1,1 },
	};

	PfxFloat u = 0.5f * (1.0f + signTbl[0][face] * axis[idxTbl[0][face]] / axis[face >> 1]);
	PfxFloat v = 0.5f * (1.0f + signTbl[1][face] * axis[idxTbl[1][face]] / axis[face >> 1]);

	PfxUInt32 ui = (PfxUInt32)(255.0f * u);
	PfxUInt32 vi = (PfxUInt32)(255.0f * v);

	return (face << 8) | (ui << 4) | vi;
}

} //namespace pfxv4
} //namespace sce


#endif // _SCE_PFX_INTERSECT_COMMON_H
