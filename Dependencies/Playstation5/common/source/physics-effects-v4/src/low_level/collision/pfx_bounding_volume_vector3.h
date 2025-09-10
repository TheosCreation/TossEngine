/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
*                Copyright (C) 2020 Sony Interactive Entertainment Inc.
*                                                
*/

#ifndef _SCE_PFX_BOUNDING_VOLUME_VECTOR3_H
#define _SCE_PFX_BOUNDING_VOLUME_VECTOR3_H

namespace sce {
namespace pfxv4 {

#define SCE_PFX_INTERSECT_COMMON_EPSILON 0.00001f

struct PfxBvVector3
{
	PfxVector3 vmin;
	PfxVector3 vmax;

	PfxBvVector3(const PfxBv &bv) : vmin(bv.vmin.offset), vmax(bv.vmax.offset) {}
	PfxBvVector3(const PfxBvVector3 &bv) : vmin(bv.vmin), vmax(bv.vmax) {}
	PfxBvVector3(const PfxVector3 &min, const PfxVector3 &max) : vmin(min), vmax(max) {}
	PfxBvVector3() : vmin(0.f), vmax(0.f) {}
};


static SCE_PFX_FORCE_INLINE PfxBool pfxTestFuncVector3(const PfxBvVector3 &bvA, const PfxBvVector3 &bvB)
{
#ifdef PFX_ENABLE_AVX
	int test1 = _mm_movemask_ps(_mm_cmplt_ps(sce_vectormath_asm128(bvA.vmax.get128()), sce_vectormath_asm128(bvB.vmin.get128())));
	int test2 = _mm_movemask_ps(_mm_cmplt_ps(sce_vectormath_asm128(bvB.vmax.get128()), sce_vectormath_asm128(bvA.vmin.get128())));
	return ((test1 | test2) & 0x07) == 0;
#else
	if (bvA.vmax[0] < bvB.vmin[0] || bvA.vmin[0] > bvB.vmax[0])	return false;
	if (bvA.vmax[1] < bvB.vmin[1] || bvA.vmin[1] > bvB.vmax[1])	return false;
	if (bvA.vmax[2] < bvB.vmin[2] || bvA.vmin[2] > bvB.vmax[2])	return false;
	return true;
#endif
}

static SCE_PFX_FORCE_INLINE PfxBool pfxClipRayByDotProductVector3(const PfxVector3 &rayStartPoint, const PfxVector3 &rayDirectionVec, const PfxVector3 &rangeMin, const PfxVector3 &rangeMax)
{
#ifdef PFX_ENABLE_AVX
	const __m128 bvMin = (sce_vectormath_asm128(rangeMin.get128()));
	const __m128 bvMax = (sce_vectormath_asm128(rangeMax.get128()));

	// Detect whether the ray is totally outside of Aabb box
	const __m128 rayDirection = sce_vectormath_asm128(rayDirectionVec.get128());
	const __m128 rayStartPosition = sce_vectormath_asm128(rayStartPoint.get128());
	__m128 rayEndPoint = _mm_add_ps(rayStartPosition, rayDirection);

	__m128 rayMax = _mm_max_ps(rayStartPosition, rayEndPoint);
	__m128 rayMin = _mm_min_ps(rayStartPosition, rayEndPoint);

	int test1 = _mm_movemask_ps(_mm_cmplt_ps(bvMax, rayMin));
	int test2 = _mm_movemask_ps(_mm_cmplt_ps(rayMax, bvMin));
	if (((test1 | test2) & 0x07) != 0)
		return false;

	static const __m128 positiveEpsilon = sce_vectormath_asm128(PfxVector3(1e-5f).get128());
	static const __m128 negativeEpsilon = sce_vectormath_asm128(PfxVector3(-1e-5f).get128());

	// Detect the class of ray's direction
	__m128 iGtZero = (_mm_cmpgt_ps(rayDirection, positiveEpsilon));
	__m128 iLtZero = (_mm_cmplt_ps(rayDirection, negativeEpsilon));
	__m128 iLtZeroShuffled = pfxShufflePs<1, 2, 0, 3>( iLtZero );

	// Vectors from ray's start point to rangeMin and rangeMax
	__m128 rayToMin = _mm_sub_ps(bvMin, rayStartPosition);
	__m128 rayToMax = _mm_sub_ps(bvMax, rayStartPosition);
	__m128 rayToMinShuffled = pfxShufflePs<1, 2, 0, 3>( rayToMin );
	__m128 rayToMaxShuffled = pfxShufflePs<1, 2, 0, 3>( rayToMax );

	// The normal vector of ray's direction
	__m128 vDirVer = pfxShufflePs<1, 2, 0, 3>( rayDirection );

	__m128 iSwap = _mm_xor_ps( iGtZero, iLtZeroShuffled );
	__m128 ip1 = _mm_or_ps( _mm_andnot_ps( iSwap, rayToMinShuffled ), _mm_and_ps( iSwap, rayToMaxShuffled ) );
	__m128 ip2 = _mm_or_ps( _mm_andnot_ps( iSwap, rayToMaxShuffled ), _mm_and_ps( iSwap, rayToMinShuffled ) );

	__m128 dotRes1 = _mm_sub_ps(_mm_mul_ps(ip1, rayDirection), _mm_mul_ps(rayToMin, vDirVer));
	__m128 dotRes2 = _mm_sub_ps(_mm_mul_ps(ip2, rayDirection), _mm_mul_ps(rayToMax, vDirVer));

	return ((_mm_movemask_ps(_mm_cmplt_ps(_mm_mul_ps(dotRes1, dotRes2), positiveEpsilon)) & 0x7) == 0x7);
#else
	PfxBv tempBvA(rangeMin, rangeMax);

	PfxVector3 rayEndPoint = rayDirectionVec + rayStartPoint;
	PfxVector3 rayAabbMin = minPerElem(rayStartPoint, rayEndPoint);
	PfxVector3 rayAabbMax = maxPerElem(rayStartPoint, rayEndPoint);
	PfxBv rayBv(rayAabbMin, rayAabbMax);
	if (!pfxTestFuncVector3(tempBvA, rayBv)) {
		return false;
}

	PfxVector3 rayToMin(tempBvA.vmin.offset - rayStartPoint);
	PfxVector3 rayToMax(tempBvA.vmax.offset - rayStartPoint);

	// xy-plane
	PfxVector3 normalDir(-rayDirectionVec.getY(), rayDirectionVec.getX(), 0.f);
	PfxVector3 p1(rayToMin.getX(), pfxClipRayGetP1Elem2(rayDirectionVec.getX(), rayDirectionVec.getY(), rayToMax.getY(), rayToMin.getY()), 0.f);
	PfxVector3 p2(rayToMax.getX(), pfxClipRayGetP2Elem2(rayDirectionVec.getX(), rayDirectionVec.getY(), rayToMax.getY(), rayToMin.getY()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	// yz-plane
	normalDir = PfxVector3(-rayDirectionVec.getZ(), rayDirectionVec.getY(), 0.f);
	p1 = PfxVector3(rayToMin.getY(), pfxClipRayGetP1Elem2(rayDirectionVec.getY(), rayDirectionVec.getZ(), rayToMax.getZ(), rayToMin.getZ()), 0.f);
	p2 = PfxVector3(rayToMax.getY(), pfxClipRayGetP2Elem2(rayDirectionVec.getY(), rayDirectionVec.getZ(), rayToMax.getZ(), rayToMin.getZ()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	// zx-plane
	normalDir = PfxVector3(-rayDirectionVec.getX(), rayDirectionVec.getZ(), 0.f);
	p1 = PfxVector3(rayToMin.getZ(), pfxClipRayGetP1Elem2(rayDirectionVec.getZ(), rayDirectionVec.getX(), rayToMax.getX(), rayToMin.getX()), 0.f);
	p2 = PfxVector3(rayToMax.getZ(), pfxClipRayGetP2Elem2(rayDirectionVec.getZ(), rayDirectionVec.getX(), rayToMax.getX(), rayToMin.getX()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	return true;
#endif
}

static SCE_PFX_FORCE_INLINE PfxBool pfxTestRayBvByDotProductVector3(const PfxVector3 &rayStartPoint, const PfxVector3 &rayDirectionVec, const PfxBvVector3 &rayAabb, const PfxVector3 &rangeAabbMin, const PfxVector3 &rangeAabbMax)
{
#ifdef PFX_ENABLE_AVX
	const __m128 bvMin = (sce_vectormath_asm128(rangeAabbMin.get128()));
	const __m128 bvMax = (sce_vectormath_asm128(rangeAabbMax.get128()));

	// Detect whether the ray is totally outside of Aabb box
	__m128 rayMin = (sce_vectormath_asm128(rayAabb.vmin.get128()));
	__m128 rayMax = (sce_vectormath_asm128(rayAabb.vmax.get128()));

	int test1 = _mm_movemask_ps(_mm_cmplt_ps(bvMax, rayMin));
	int test2 = _mm_movemask_ps(_mm_cmplt_ps(rayMax, bvMin));
	if (((test1 | test2) & 0x07) != 0)
		return false;

	static const __m128 positiveEpsilon = sce_vectormath_asm128(PfxVector3(1e-5f).get128());
	static const __m128 negativeEpsilon = sce_vectormath_asm128(PfxVector3(-1e-5f).get128());
	const __m128 rayDirection = sce_vectormath_asm128(rayDirectionVec.get128());
	const __m128 rayStartPosition = sce_vectormath_asm128(rayStartPoint.get128());

	// Detect the class of ray's direction
	__m128 iGtZero = (_mm_cmpgt_ps(rayDirection, positiveEpsilon));
	__m128 iLtZero = (_mm_cmplt_ps(rayDirection, negativeEpsilon));
	__m128 iLtZeroShuffled = pfxShufflePs<1, 2, 0, 3>(iLtZero );

	// Vectors from ray's start point to rangeMin and rangeMax
	__m128 rayToMin = _mm_sub_ps(bvMin, rayStartPosition);
	__m128 rayToMax = _mm_sub_ps(bvMax, rayStartPosition);
	__m128 rayToMinShuffled = pfxShufflePs<1, 2, 0, 3>( rayToMin );
	__m128 rayToMaxShuffled = pfxShufflePs<1, 2, 0, 3>( rayToMax );

	// The normal vector of ray's direction
	__m128 vDirVer = pfxShufflePs<1, 2, 0, 3>( rayDirection );

	__m128 iSwap = _mm_xor_ps( iGtZero, iLtZeroShuffled );
	__m128 ip1 = _mm_or_ps( _mm_andnot_ps( iSwap, rayToMinShuffled ), _mm_and_ps( iSwap, rayToMaxShuffled ) );
	__m128 ip2 = _mm_or_ps( _mm_andnot_ps( iSwap, rayToMaxShuffled ), _mm_and_ps( iSwap, rayToMinShuffled ) );

	__m128 dotRes1 = _mm_sub_ps(_mm_mul_ps(ip1, rayDirection), _mm_mul_ps(rayToMin, vDirVer));
	__m128 dotRes2 = _mm_sub_ps(_mm_mul_ps(ip2, rayDirection), _mm_mul_ps(rayToMax, vDirVer));

	return ((_mm_movemask_ps(_mm_cmplt_ps(_mm_mul_ps(dotRes1, dotRes2), positiveEpsilon)) & 0x7) == 0x7);
#else
	PfxBv rangeAabb(rangeAabbMin, rangeAabbMax);

	if (!pfxTestFuncVector3(rangeAabb, rayAabb)) {
		return false;
	}

	PfxVector3 rayToMin(rangeAabbMin - rayStartPoint);
	PfxVector3 rayToMax(rangeAabbMax - rayStartPoint);

	// xy-plane
	PfxVector3 normalDir(-rayDirectionVec.getY(), rayDirectionVec.getX(), 0.f);
	PfxVector3 p1(rayToMin.getX(), pfxClipRayGetP1Elem2(rayDirectionVec.getX(), rayDirectionVec.getY(), rayToMax.getY(), rayToMin.getY()), 0.f);
	PfxVector3 p2(rayToMax.getX(), pfxClipRayGetP2Elem2(rayDirectionVec.getX(), rayDirectionVec.getY(), rayToMax.getY(), rayToMin.getY()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	// yz-plane
	normalDir = PfxVector3(-rayDirectionVec.getZ(), rayDirectionVec.getY(), 0.f);
	p1 = PfxVector3(rayToMin.getY(), pfxClipRayGetP1Elem2(rayDirectionVec.getY(), rayDirectionVec.getZ(), rayToMax.getZ(), rayToMin.getZ()), 0.f);
	p2 = PfxVector3(rayToMax.getY(), pfxClipRayGetP2Elem2(rayDirectionVec.getY(), rayDirectionVec.getZ(), rayToMax.getZ(), rayToMin.getZ()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	// zx-plane
	normalDir = PfxVector3(-rayDirectionVec.getX(), rayDirectionVec.getZ(), 0.f);
	p1 = PfxVector3(rayToMin.getZ(), pfxClipRayGetP1Elem2(rayDirectionVec.getZ(), rayDirectionVec.getX(), rayToMax.getX(), rayToMin.getX()), 0.f);
	p2 = PfxVector3(rayToMax.getZ(), pfxClipRayGetP2Elem2(rayDirectionVec.getZ(), rayDirectionVec.getX(), rayToMax.getX(), rayToMin.getX()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	return true;
#endif
}

static SCE_PFX_FORCE_INLINE PfxBool pfxTestRayBvByDotProductVector3(const PfxVector3 &rayStartPoint, const PfxVector3 &rayDirectionVec, const PfxBvVector3 &rayAabb, const PfxBvVector3 &rangeAabb)
{
	return pfxTestRayBvByDotProductVector3(rayStartPoint, rayDirectionVec, rayAabb, rangeAabb.vmin, rangeAabb.vmax);
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxClipRayAabbVector3(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	const PfxVector3 &AABBmin,
	const PfxVector3 &AABBmax,
	PfxFloat &variable)
{
	PfxVector3 dir = rayDirection;
	PfxVector3 absDir = absPerElem(dir);

	PfxVector3 t1 = divPerElem(AABBmin - rayStartPosition, dir);
	PfxVector3 t2 = divPerElem(AABBmax - rayStartPosition, dir);

#ifdef PFX_ENABLE_AVX
	const __m128 v_rayStartPosition = sce_vectormath_asm128(rayStartPosition.get128());
	__m128 v_chk1 = _mm_cmplt_ps(sce_vectormath_asm128(absDir.get128()), _mm_set1_ps(SCE_PFX_INTERSECT_COMMON_EPSILON));
	__m128 v_chk2 = _mm_or_ps(
		_mm_cmplt_ps(v_rayStartPosition, sce_vectormath_asm128(AABBmin.get128())),
		_mm_cmpgt_ps(v_rayStartPosition, sce_vectormath_asm128(AABBmax.get128()))
	);
	if ((_mm_movemask_ps(_mm_and_ps(v_chk1, v_chk2)) & 0x07) != 0) return false;
	__m128 v_t1 = _mm_blendv_ps(sce_vectormath_asm128(t1.get128()), _mm_set1_ps(-SCE_PFX_FLT_MAX), v_chk1);
	__m128 v_t2 = _mm_blendv_ps(sce_vectormath_asm128(t2.get128()), _mm_set1_ps(SCE_PFX_FLT_MAX), v_chk1);

	union {
		float x, y, z, w;
		__m128i i;
		__m128 f;
	} v_tmin, v_tmax, v_max_tmin, v_min_tmax;

	v_tmin.f = _mm_min_ps(v_t1, v_t2);
	v_tmax.f = _mm_max_ps(v_t1, v_t2);

	v_max_tmin.f = _mm_max_ps(_mm_max_ps(v_tmin.f, _mm_castsi128_ps(_mm_srli_si128(v_tmin.i, 4))), _mm_castsi128_ps(_mm_srli_si128(v_tmin.i, 8)));
	v_min_tmax.f = _mm_min_ps(_mm_min_ps(v_tmax.f, _mm_castsi128_ps(_mm_srli_si128(v_tmax.i, 4))), _mm_castsi128_ps(_mm_srli_si128(v_tmax.i, 8)));

	PfxFloat max_tmin = v_max_tmin.x;
	PfxFloat min_tmax = v_min_tmax.x;
#else
	if (absDir[0] < SCE_PFX_INTERSECT_COMMON_EPSILON) {
		if (rayStartPosition[0] < AABBmin[0] || rayStartPosition[0] > AABBmax[0]) {
			return false;
		}
		t1[0] = -SCE_PFX_FLT_MAX;
		t2[0] = SCE_PFX_FLT_MAX;
	}

	if (absDir[1] < SCE_PFX_INTERSECT_COMMON_EPSILON) {
		if (rayStartPosition[1] < AABBmin[1] || rayStartPosition[1] > AABBmax[1]) {
			return false;
		}
		t1[1] = -SCE_PFX_FLT_MAX;
		t2[1] = SCE_PFX_FLT_MAX;
	}

	if (absDir[2] < SCE_PFX_INTERSECT_COMMON_EPSILON) {
		if (rayStartPosition[2] < AABBmin[2] || rayStartPosition[2] > AABBmax[2]) {
			return false;
		}
		t1[2] = -SCE_PFX_FLT_MAX;
		t2[2] = SCE_PFX_FLT_MAX;
	}

	PfxFloat max_tmin = maxElem(minPerElem(t1, t2));
	PfxFloat min_tmax = minElem(maxPerElem(t1, t2));
#endif

	if (max_tmin > min_tmax) return false;

	variable = max_tmin;

	return true;
}
}
}

#endif

