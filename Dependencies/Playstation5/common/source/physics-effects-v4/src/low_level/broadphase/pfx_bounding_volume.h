/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_BV_H
#define _SCE_PFX_BV_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/base/pfx_large_position.h"
#include "../../../include/physics_effects/base_level/collision/pfx_ray.h"

#define SCE_PFX_BV_EXPAND 0.001f

namespace sce {
namespace pfxv4 {

#if 1

struct PfxBv
{
	PfxLargePosition vmin;
	PfxLargePosition vmax;
	
	PfxBv() {}
	PfxBv(const PfxLargePosition &vmin_,const PfxLargePosition &vmax_) : vmin(vmin_),vmax(vmax_) {}

	PfxLargePosition getCenter() const
	{
		return PfxLargePosition((vmax.segment+vmin.segment).halve(),(vmax.offset+vmin.offset)*0.5f);
	}

	PfxLargePosition getExtent() const
	{
		return PfxLargePosition((vmax.segment-vmin.segment).halve(),(vmax.offset-vmin.offset)*0.5f);
	}
};

inline PfxBv pfxMergeBv(const PfxBv &bvA,const PfxBv &bvB)
{
	PfxBv bv;
	bv.vmin = minPerElem(bvA.vmin,bvB.vmin);
	bv.vmax = maxPerElem(bvA.vmax,bvB.vmax);
	return bv;
}

inline PfxBool pfxTestBv(const PfxBv &bvA,const PfxBv &bvB)
{
	extern PfxFloat gSegmentWidthInv;

#ifdef PFX_ENABLE_AVX
	__m128 offset_test1 = sce_vectormath_asm128(((bvB.vmin.offset - bvA.vmax.offset) * gSegmentWidthInv).get128());
	__m128 segment_test1 = _mm_cvtepi32_ps((bvA.vmax.segment - bvB.vmin.segment).vi128);
	__m128 offset_test2 = sce_vectormath_asm128(((bvB.vmax.offset - bvA.vmin.offset) * gSegmentWidthInv).get128());
	__m128 segment_test2 = _mm_cvtepi32_ps((bvA.vmin.segment - bvB.vmax.segment).vi128);
	int test1 = _mm_movemask_ps(_mm_cmplt_ps(segment_test1,offset_test1));
	int test2 = _mm_movemask_ps(_mm_cmpgt_ps(segment_test2,offset_test2));
	return ((test1 | test2) & 0x07) == 0;
#else
	if(	(PfxFloat)(bvA.vmax.segment.x - bvB.vmin.segment.x) < (bvB.vmin.offset[0] - bvA.vmax.offset[0]) * gSegmentWidthInv ||
		(PfxFloat)(bvA.vmin.segment.x - bvB.vmax.segment.x) > (bvB.vmax.offset[0] - bvA.vmin.offset[0]) * gSegmentWidthInv ) return false;
	if(	(PfxFloat)(bvA.vmax.segment.y - bvB.vmin.segment.y) < (bvB.vmin.offset[1] - bvA.vmax.offset[1]) * gSegmentWidthInv ||
		(PfxFloat)(bvA.vmin.segment.y - bvB.vmax.segment.y) > (bvB.vmax.offset[1] - bvA.vmin.offset[1]) * gSegmentWidthInv ) return false;
	if(	(PfxFloat)(bvA.vmax.segment.z - bvB.vmin.segment.z) < (bvB.vmin.offset[2] - bvA.vmax.offset[2]) * gSegmentWidthInv ||
		(PfxFloat)(bvA.vmin.segment.z - bvB.vmax.segment.z) > (bvB.vmax.offset[2] - bvA.vmin.offset[2]) * gSegmentWidthInv ) return false;
	return true;
#endif
}

inline PfxFloat pfxCalcCombinedVolume(const PfxBv &bvA,const PfxBv &bvB)
{
	PfxLargePosition bvMin = minPerElem(bvA.vmin,bvB.vmin);
	PfxLargePosition bvMax = maxPerElem(bvA.vmax,bvB.vmax);
	PfxVector3 extent = (bvMax - bvMin).convertToVector3();
	return extent[0] + extent[1] + extent[2];
}

inline PfxFloat pfxCalcVolume(const PfxBv &bv)
{
	PfxVector3 extent = (bv.vmax-bv.vmin).convertToVector3();
	return extent[0] * extent[1] * extent[2];
}

inline PfxBool pfxContain(const PfxBv &bvA,const PfxBv &bvB)
{
	extern PfxFloat gSegmentWidthInv;

#ifdef PFX_ENABLE_AVX
	__m128 offset_test1 = sce_vectormath_asm128(((bvB.vmin.offset - bvA.vmin.offset) * gSegmentWidthInv).get128());
	__m128 segment_test1 = _mm_cvtepi32_ps((bvA.vmin.segment - bvB.vmin.segment).vi128);
	__m128 offset_test2 = sce_vectormath_asm128(((bvB.vmax.offset - bvA.vmax.offset) * gSegmentWidthInv).get128());
	__m128 segment_test2 = _mm_cvtepi32_ps((bvA.vmax.segment - bvB.vmax.segment).vi128);
	int test1 = _mm_movemask_ps(_mm_cmple_ps(segment_test1,offset_test1));
	int test2 = _mm_movemask_ps(_mm_cmpge_ps(segment_test2,offset_test2));
	return ((test1 & test2) & 0x07) == 0x07;
#else
	if(	(PfxFloat)(bvA.vmin.segment.x - bvB.vmin.segment.x) <= (bvB.vmin.offset[0] - bvA.vmin.offset[0]) * gSegmentWidthInv &&
		(PfxFloat)(bvA.vmax.segment.x - bvB.vmax.segment.x) >= (bvB.vmax.offset[0] - bvA.vmax.offset[0]) * gSegmentWidthInv &&
		(PfxFloat)(bvA.vmin.segment.y - bvB.vmin.segment.y) <= (bvB.vmin.offset[1] - bvA.vmin.offset[1]) * gSegmentWidthInv &&
		(PfxFloat)(bvA.vmax.segment.y - bvB.vmax.segment.y) >= (bvB.vmax.offset[1] - bvA.vmax.offset[1]) * gSegmentWidthInv &&
		(PfxFloat)(bvA.vmin.segment.z - bvB.vmin.segment.z) <= (bvB.vmin.offset[2] - bvA.vmin.offset[2]) * gSegmentWidthInv &&
		(PfxFloat)(bvA.vmax.segment.z - bvB.vmax.segment.z) >= (bvB.vmax.offset[2] - bvA.vmax.offset[2]) * gSegmentWidthInv ) {
		return true;
	}
	return false;
#endif
}

static SCE_PFX_FORCE_INLINE
PfxFloat pfxLengthSqr(const PfxBv &bvA,const PfxBv &bvB)
{
	PfxLargePosition len;
	len.segment = bvA.vmin.segment + bvA.vmax.segment - bvB.vmin.segment - bvB.vmax.segment;
	len.offset = bvA.vmin.offset + bvA.vmax.offset - bvB.vmin.offset - bvB.vmax.offset;
	return lengthSqr(len.convertToVector3());
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxTestBvSphere(const PfxBv &bv,const PfxLargePosition &center,PfxFloat radius)
{
	PfxLargePosition closestPoint = center;
	closestPoint = maxPerElem(closestPoint,bv.vmin);
	closestPoint = minPerElem(closestPoint,bv.vmax);
	PfxFloat diffSqr = lengthSqr((closestPoint - center).convertToVector3());
	if(diffSqr < radius * radius) {
		return true;
	}
	return false;
}

#ifndef PFX_ENABLE_AVX
static SCE_PFX_FORCE_INLINE
PfxFloat pfxClipRayGetP1Elem2(PfxFloat v1, PfxFloat v2, PfxFloat rangeMaxV2, PfxFloat rangeMinV2)
{
	if ((v1 > 0.f && v2 > 0.f) || (v1 < 0.f && v2 < 0.f))	return rangeMaxV2;
	else													return rangeMinV2;
}

static SCE_PFX_FORCE_INLINE
PfxFloat pfxClipRayGetP2Elem2(PfxFloat v1, PfxFloat v2, PfxFloat rangeMaxV2, PfxFloat rangeMinV2)
{
	if ((v1 > 0.f && v2 > 0.f) || (v1 < 0.f && v2 < 0.f))	return rangeMinV2;
	else													return rangeMaxV2;
}
#endif

static SCE_PFX_FORCE_INLINE
PfxBool pfxTestBvRay(const PfxBv &bv,const PfxRayInput &ray)
{
#ifdef PFX_ENABLE_AVX
	static const __m128 positiveEpsilon = sce_vectormath_asm128(PfxVector3(1e-5f).get128());
	static const __m128 negativeEpsilon = sce_vectormath_asm128(PfxVector3(-1e-5f).get128());
	const __m128 rayDirection = sce_vectormath_asm128(ray.m_direction.get128());

	// Detect the class of ray's direction
	__m128 iGtZero = _mm_cmpgt_ps(rayDirection, positiveEpsilon);
	__m128 iLtZero = _mm_cmplt_ps(rayDirection, negativeEpsilon);
	__m128 iLtZeroShuffled = pfxShufflePs<1, 2, 0, 3>( iLtZero );
	__m128 iGtEqZero = _mm_or_ps( iGtZero, _mm_xor_ps( iLtZero, _mm_castsi128_ps( _mm_set1_epi32( -1 ) ) ) );

	// Detect whether the ray is totally outside of Aabb box
	__m128 rayEndPointOffset = _mm_add_ps(sce_vectormath_asm128(ray.m_startPosition.offset.get128()), rayDirection);
	__m128 rayMaxOffset = _mm_or_ps(_mm_and_ps(sce_vectormath_asm128(ray.m_startPosition.offset.get128()), iGtEqZero), _mm_and_ps(rayEndPointOffset, iLtZero));
	__m128 rayMinOffset = _mm_or_ps(_mm_and_ps(rayEndPointOffset, iGtEqZero), _mm_and_ps(sce_vectormath_asm128(ray.m_startPosition.offset.get128()), iLtZero));

	extern PfxFloat gSegmentWidthInv;
	__m128 vecSegmentWidthInv = _mm_set1_ps(gSegmentWidthInv);
	__m128 maxMinusOffsetTest = _mm_mul_ps(_mm_sub_ps(rayMaxOffset, sce_vectormath_asm128(bv.vmax.offset.get128())), vecSegmentWidthInv);
	__m128 maxSegmentTest = _mm_cvtepi32_ps((bv.vmax.segment - ray.m_startPosition.segment).vi128);
	__m128 minMinusOffsetTest = _mm_mul_ps(_mm_sub_ps(sce_vectormath_asm128(bv.vmin.offset.get128()), rayMinOffset), vecSegmentWidthInv);
	__m128 minSegmentTest = _mm_cvtepi32_ps((ray.m_startPosition.segment - bv.vmin.segment).vi128);

	int maxTest = _mm_movemask_ps(_mm_cmpgt_ps(maxSegmentTest, maxMinusOffsetTest));
	int minTest = _mm_movemask_ps(_mm_cmpgt_ps(minSegmentTest, minMinusOffsetTest));
	int totalTest = maxTest & minTest;
	if ((totalTest & 0x07) != 0x07)
		return false;

	// Vectors from ray's start point to rangeMin and rangeMax
	PfxLargePosition tmp1(ray.m_startPosition);
	PfxLargePosition tmp2(ray.m_startPosition);
	tmp1.changeSegment(bv.vmin.segment);
	tmp2.changeSegment(bv.vmax.segment);

	__m128 rayToMin = _mm_sub_ps(sce_vectormath_asm128(bv.vmin.offset.get128()), sce_vectormath_asm128(tmp1.offset.get128()));
	__m128 rayToMax = _mm_sub_ps(sce_vectormath_asm128(bv.vmax.offset.get128()), sce_vectormath_asm128(tmp2.offset.get128()));
	__m128 rayToMinShuffled = pfxShufflePs<1, 2, 0, 3>( rayToMin );
	__m128 rayToMaxShuffled = pfxShufflePs<1, 2, 0, 3>( rayToMax );

	// The normal vector of ray's direction
	__m128 vDirVer = pfxShufflePs<1, 2, 0, 3>( rayDirection );

	__m128 iSwap = _mm_xor_ps( iGtZero, iLtZeroShuffled );
	__m128 ip1 = _mm_or_ps( _mm_andnot_ps( iSwap, rayToMinShuffled ), _mm_and_ps( iSwap, rayToMaxShuffled ) );
	__m128 ip2 = _mm_or_ps( _mm_andnot_ps( iSwap, rayToMaxShuffled ), _mm_and_ps( iSwap, rayToMinShuffled ) );

	__m128 dotRes1 = (_mm_sub_ps(_mm_mul_ps(ip1, rayDirection), _mm_mul_ps(rayToMin, vDirVer)));
	__m128 dotRes2 = (_mm_sub_ps(_mm_mul_ps(ip2, rayDirection), _mm_mul_ps(rayToMax, vDirVer)));

	return ((_mm_movemask_ps(_mm_cmplt_ps(_mm_mul_ps(dotRes1, dotRes2), positiveEpsilon)) & 0x7) == 0x7);
#else
	PfxLargePosition rayEndPoint = ray.m_startPosition + ray.m_direction;
	PfxLargePosition rayAabbMin = minPerElem(ray.m_startPosition, rayEndPoint);
	PfxLargePosition rayAabbMax = maxPerElem(ray.m_startPosition, rayEndPoint);
	PfxBv rangeBv(bv.vmin, bv.vmax);
	PfxBv rayBv(rayAabbMin, rayAabbMax);
	if (!pfxTestBv(rangeBv, rayBv)) {
		return false;
	}

	PfxLargePosition tmp1 = ray.m_startPosition;
	PfxLargePosition tmp2 = ray.m_startPosition;
	tmp1.changeSegment(bv.vmin.segment);
	tmp2.changeSegment(bv.vmax.segment);

	PfxVector3 rayToMin(bv.vmin.offset - tmp1.offset);
	PfxVector3 rayToMax(bv.vmax.offset - tmp2.offset);

	// xy-plane
	PfxVector3 normalDir(-ray.m_direction.getY(), ray.m_direction.getX(), 0.f);
	PfxVector3 p1(rayToMin.getX(), pfxClipRayGetP1Elem2(ray.m_direction.getX(), ray.m_direction.getY(), rayToMax.getY(), rayToMin.getY()), 0.f);
	PfxVector3 p2(rayToMax.getX(), pfxClipRayGetP2Elem2(ray.m_direction.getX(), ray.m_direction.getY(), rayToMax.getY(), rayToMin.getY()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	// yz-plane
	normalDir = PfxVector3(-ray.m_direction.getZ(), ray.m_direction.getY(), 0.f);
	p1 = PfxVector3(rayToMin.getY(), pfxClipRayGetP1Elem2(ray.m_direction.getY(), ray.m_direction.getZ(), rayToMax.getZ(), rayToMin.getZ()), 0.f);
	p2 = PfxVector3(rayToMax.getY(), pfxClipRayGetP2Elem2(ray.m_direction.getY(), ray.m_direction.getZ(), rayToMax.getZ(), rayToMin.getZ()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	// zx-plane
	normalDir = PfxVector3(-ray.m_direction.getX(), ray.m_direction.getZ(), 0.f);
	p1 = PfxVector3(rayToMin.getZ(), pfxClipRayGetP1Elem2(ray.m_direction.getZ(), ray.m_direction.getX(), rayToMax.getX(), rayToMin.getX()), 0.f);
	p2 = PfxVector3(rayToMax.getZ(), pfxClipRayGetP2Elem2(ray.m_direction.getZ(), ray.m_direction.getX(), rayToMax.getX(), rayToMin.getX()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	return true;
#endif
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxClipRay(const PfxLargePosition &rayStartPoint,const PfxVector3 &rayDirection,const PfxLargePosition &rangeMin,const PfxLargePosition &rangeMax,PfxFloat &tc0,PfxFloat &tc1)
{
	PfxLargePosition rayEndPoint = rayStartPoint + rayDirection;
	PfxLargePosition rayAabbMin = minPerElem(rayStartPoint,rayEndPoint);
	PfxLargePosition rayAabbMax = maxPerElem(rayStartPoint,rayEndPoint);

	PfxBv rangeBv(rangeMin,rangeMax);
	PfxBv rayBv(rayAabbMin,rayAabbMax);

	if(!pfxTestBv(rangeBv,rayBv)) {
		return false;
	}
	
	if(pfxContain(rangeBv,rayBv)) {
		tc0 = 0.0f;
		tc1 = 1.0f;
		return true;
	}
	
	PfxLargePosition tmp1 = rayStartPoint;
	PfxLargePosition tmp2 = rayStartPoint;
	tmp1.changeSegment(rangeBv.vmin.segment);
	tmp2.changeSegment(rangeBv.vmax.segment);
	
	PfxVector3 rayStartPosition_min(tmp1.offset);
	PfxVector3 rayStartPosition_max(tmp2.offset);
	const PfxVector3 ood = recipPerElem(rayDirection);
	const PfxVector3 abs = absPerElem(rayDirection);
	
#ifdef PFX_ENABLE_AVX
	__m128 vAbsResult = _mm_cmplt_ps(sce_vectormath_asm128(abs.get128()), _mm_set1_ps(1e-04f));
	__m128 vBoundaryCheck =
		_mm_and_ps(
		vAbsResult,
			_mm_or_ps(
				_mm_cmplt_ps(sce_vectormath_asm128(rayStartPosition_min.get128()), sce_vectormath_asm128(rangeBv.vmin.offset.get128())),
				_mm_cmplt_ps(sce_vectormath_asm128(rangeBv.vmax.offset.get128()), sce_vectormath_asm128(rayStartPosition_max.get128()))
			)
		);
	if((_mm_movemask_ps(vBoundaryCheck) & 0x07) != 0) return false;
	
	PfxVector3 t1 = mulPerElem((rangeBv.vmin.offset - rayStartPosition_min), ood);
	PfxVector3 t2 = mulPerElem((rangeBv.vmax.offset - rayStartPosition_max), ood);
	__m128 vt1 = sce_vectormath_asm128(t1.get128());
	__m128 vt2 = sce_vectormath_asm128(t2.get128());
	vt1 = _mm_blendv_ps(vt1, _mm_set1_ps(-SCE_PFX_FLT_MAX), vAbsResult);
	vt2 = _mm_blendv_ps(vt2, _mm_set1_ps( SCE_PFX_FLT_MAX), vAbsResult);
	t1.set128(sce_vectormath_asfloat4(vt1));
	t2.set128(sce_vectormath_asfloat4(vt2));
	PfxFloat tmin = maxElem(minPerElem(t1, t2));
	PfxFloat tmax = minElem(maxPerElem(t1, t2));
	if(tmin > tmax || tmax < 0.0f || tmin > 1.0f) return false;
#else
	PfxFloat tmin = -SCE_PFX_FLT_MAX;
	PfxFloat tmax =  SCE_PFX_FLT_MAX;
	
	for(int i=0;i<3;i++) {
		if(fabs(rayDirection[i]) < 1e-04f) {
			if(rayStartPosition_min[i] < rangeBv.vmin.offset[i] || rayStartPosition_max[i] > rangeBv.vmax.offset[i]) {
				return false;
			}
		}
		else {
			PfxFloat t1 = (rangeBv.vmin.offset[i] - rayStartPosition_min[i]) * ood[i];
			PfxFloat t2 = (rangeBv.vmax.offset[i] - rayStartPosition_max[i]) * ood[i];
			if(t1 > t2) SCE_PFX_SWAP(PfxFloat,t1,t2);
			if(t1>tmin) tmin = t1;
			if(t2<tmax) tmax = t2;
			if(tmin>tmax) return false;
		}
	}
	
	if(tmax < 0.0f || tmin > 1.0f) 
		return false;
#endif

	tc0 = SCE_PFX_MAX(tmin,0.0f);
	tc1 = SCE_PFX_MIN(tmax,1.0f);
	
	return true;
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxClipRayByDotProduct(const PfxLargePosition &rayStartPoint, const PfxVector3 &rayDirection, const PfxLargePosition &rangeMin, const PfxLargePosition &rangeMax)
{
#ifdef PFX_ENABLE_AVX
	static const __m128 positiveEpsilon(sce_vectormath_asm128(PfxVector3(1e-5f).get128()));
	static const __m128 negativeEpsilon(sce_vectormath_asm128(PfxVector3(-1e-5f).get128()));

	// Detect the class of ray's direction
	__m128 iGtZero(_mm_cmpgt_ps(sce_vectormath_asm128(rayDirection.get128()), positiveEpsilon));
	__m128 iLtZero(_mm_cmplt_ps(sce_vectormath_asm128(rayDirection.get128()), negativeEpsilon));
	__m128 iLtZeroShuffled( pfxShufflePs<1, 2, 0, 3>( iLtZero ) );
	__m128 iGtEqZero( _mm_or_ps( iGtZero, _mm_xor_ps( iLtZero, _mm_castsi128_ps( _mm_set1_epi32( -1 ) ) ) ) );

	// Detect whether the ray is totally outside of Aabb box
	__m128 rayEndPointOffset = _mm_add_ps(sce_vectormath_asm128(rayStartPoint.offset.get128()), sce_vectormath_asm128(rayDirection.get128()));
	__m128 rayMaxOffset = _mm_or_ps(_mm_and_ps(sce_vectormath_asm128(rayStartPoint.offset.get128()), iGtEqZero), _mm_and_ps(rayEndPointOffset, iLtZero));
	__m128 rayMinOffset = _mm_or_ps(_mm_and_ps(rayEndPointOffset, iGtEqZero), _mm_and_ps(sce_vectormath_asm128(rayStartPoint.offset.get128()), iLtZero));

	extern PfxFloat gSegmentWidthInv;
	const __m128 vecSegmentWidthInv = _mm_set1_ps(gSegmentWidthInv);
	__m128 maxMinusOffsetTest = _mm_mul_ps(_mm_sub_ps(rayMaxOffset, sce_vectormath_asm128(rangeMax.offset.get128())), vecSegmentWidthInv);
	__m128 maxSegmentTest = _mm_cvtepi32_ps((rangeMax.segment - rayStartPoint.segment).vi128);
	__m128 minMinusOffsetTest = _mm_mul_ps(_mm_sub_ps(sce_vectormath_asm128(rangeMin.offset.get128()), rayMinOffset), vecSegmentWidthInv);
	__m128 minSegmentTest = _mm_cvtepi32_ps((rayStartPoint.segment - rangeMin.segment).vi128);

	int maxTest = _mm_movemask_ps(_mm_cmpgt_ps(maxSegmentTest, maxMinusOffsetTest));
	int minTest = _mm_movemask_ps(_mm_cmpgt_ps(minSegmentTest, minMinusOffsetTest));
	int totalTest = maxTest & minTest;
	if ((totalTest & 0x07) != 0x07)
		return false;

	// Vectors from ray's start point to rangeMin and rangeMax
	PfxLargePosition tmp1(rayStartPoint);
	PfxLargePosition tmp2(rayStartPoint);
	tmp1.changeSegment(rangeMin.segment);
	tmp2.changeSegment(rangeMax.segment);

	__m128 rayToMin = _mm_sub_ps(sce_vectormath_asm128(rangeMin.offset.get128()), sce_vectormath_asm128(tmp1.offset.get128()));
	__m128 rayToMax = _mm_sub_ps(sce_vectormath_asm128(rangeMax.offset.get128()), sce_vectormath_asm128(tmp2.offset.get128()));
	__m128 rayToMinShuffled = pfxShufflePs<1, 2, 0, 3>( rayToMin );
	__m128 rayToMaxShuffled = pfxShufflePs<1, 2, 0, 3>( rayToMax );

	// The normal vector of ray's direction
	__m128 vDirVer = pfxShufflePs<1, 2, 0, 3>( sce_vectormath_asm128( rayDirection.get128() ) );

	__m128 iSwap = _mm_xor_ps( iGtZero, iLtZeroShuffled );
	__m128 ip1 = _mm_or_ps( _mm_andnot_ps( iSwap, rayToMinShuffled ), _mm_and_ps( iSwap, rayToMaxShuffled ) );
	__m128 ip2 = _mm_or_ps( _mm_andnot_ps( iSwap, rayToMaxShuffled ), _mm_and_ps( iSwap, rayToMinShuffled ) );

	__m128 dotRes1 = _mm_sub_ps(_mm_mul_ps(ip1, sce_vectormath_asm128(rayDirection.get128())), _mm_mul_ps(rayToMin, vDirVer));
	__m128 dotRes2 = _mm_sub_ps(_mm_mul_ps(ip2, sce_vectormath_asm128(rayDirection.get128())), _mm_mul_ps(rayToMax, vDirVer));

	return ((_mm_movemask_ps(_mm_cmplt_ps(_mm_mul_ps(dotRes1, dotRes2), positiveEpsilon)) & 0x7) == 0x7);
#else
	PfxLargePosition rayEndPoint = rayStartPoint + rayDirection;
	PfxLargePosition rayAabbMin = minPerElem(rayStartPoint, rayEndPoint);
	PfxLargePosition rayAabbMax = maxPerElem(rayStartPoint, rayEndPoint);
	PfxBv rangeBv(rangeMin, rangeMax);
	PfxBv rayBv(rayAabbMin, rayAabbMax);
	if (!pfxTestBv(rangeBv, rayBv)) {
		return false;
	}

	PfxLargePosition tmp1 = rayStartPoint;
	PfxLargePosition tmp2 = rayStartPoint;
	tmp1.changeSegment(rangeMin.segment);
	tmp2.changeSegment(rangeMax.segment);

	PfxVector3 rayToMin(rangeMin.offset - tmp1.offset);
	PfxVector3 rayToMax(rangeMax.offset - tmp2.offset);

	// xy-plane
	PfxVector3 normalDir(-rayDirection.getY(), rayDirection.getX(), 0.f);
	PfxVector3 p1(rayToMin.getX(), pfxClipRayGetP1Elem2(rayDirection.getX(), rayDirection.getY(), rayToMax.getY(), rayToMin.getY()), 0.f);
	PfxVector3 p2(rayToMax.getX(), pfxClipRayGetP2Elem2(rayDirection.getX(), rayDirection.getY(), rayToMax.getY(), rayToMin.getY()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	// yz-plane
	normalDir = PfxVector3(-rayDirection.getZ(), rayDirection.getY(), 0.f);
	p1 = PfxVector3(rayToMin.getY(), pfxClipRayGetP1Elem2(rayDirection.getY(), rayDirection.getZ(), rayToMax.getZ(), rayToMin.getZ()), 0.f);
	p2 = PfxVector3(rayToMax.getY(), pfxClipRayGetP2Elem2(rayDirection.getY(), rayDirection.getZ(), rayToMax.getZ(), rayToMin.getZ()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	// zx-plane
	normalDir = PfxVector3(-rayDirection.getX(), rayDirection.getZ(), 0.f);
	p1 = PfxVector3(rayToMin.getZ(), pfxClipRayGetP1Elem2(rayDirection.getZ(), rayDirection.getX(), rayToMax.getX(), rayToMin.getX()), 0.f);
	p2 = PfxVector3(rayToMax.getZ(), pfxClipRayGetP2Elem2(rayDirection.getZ(), rayDirection.getX(), rayToMax.getX(), rayToMin.getX()), 0.f);
	if (dot(p1, normalDir) * dot(p2, normalDir) > 0.f)	return false;

	return true;
#endif
}


#else

struct PfxBv
{
	PfxSegment centerSegment;
	PfxFloat centerOffset[3];
	PfxFloat extent[3];
	
	PfxBv() {}
	PfxBv(const PfxLargePosition &center_,const PfxVector3 &extent_)
	{
		centerSegment = center_.segment;
		centerOffset[0] = center_.offset[0];
		centerOffset[1] = center_.offset[1];
		centerOffset[2] = center_.offset[2];
		extent[0] = extent_[0];
		extent[1] = extent_[1];
		extent[2] = extent_[2];
	}

	PfxLargePosition getCenter() const {return PfxLargePosition(centerSegment,pfxReadVector3(centerOffset));}

	PfxVector3 getExtent() const {return pfxReadVector3(extent);}
};

inline PfxBv pfxMergeBv(const PfxBv &bvA,const PfxBv &bvB)
{
	PfxLargePosition centerB = bvB.center;
	centerB.changeSegment(bvA.center.segment);
	PfxVector3 aabbMin[3],aabbMax[3]
	
	aabbMin[0] = SCE_PFX_MIN(bvA.centerOffset[0] - bvA.extent[0],centerB.offset[0] - bvB.extent[0]);
	aabbMin[1] = SCE_PFX_MIN(bvA.centerOffset[1] - bvA.extent[1],centerB.offset[1] - bvB.extent[1]);
	aabbMin[2] = SCE_PFX_MIN(bvA.centerOffset[2] - bvA.extent[2],centerB.offset[2] - bvB.extent[2]);
	aabbMax[0] = SCE_PFX_MAX(bvA.centerOffset[0] + bvA.extent[0],centerB.offset[0] + bvB.extent[0]);
	aabbMax[1] = SCE_PFX_MAX(bvA.centerOffset[1] + bvA.extent[1],centerB.offset[1] + bvB.extent[1]);
	aabbMax[2] = SCE_PFX_MAX(bvA.centerOffset[2] + bvA.extent[2],centerB.offset[2] + bvB.extent[2]);

	PfxBv bv;
	bv.centerSegment = bvA.centerSegment;
	bv.centerOffset[0] = 0.5f * (aabbMax[0] + aabbMin[0]);
	bv.centerOffset[1] = 0.5f * (aabbMax[1] + aabbMin[1]);
	bv.centerOffset[2] = 0.5f * (aabbMax[2] + aabbMin[2]);
	bv.extent[0] = 0.5f * (aabbMax[0] - aabbMin[0]);
	bv.extent[1] = 0.5f * (aabbMax[1] - aabbMin[1]);
	bv.extent[2] = 0.5f * (aabbMax[2] - aabbMin[2]);
	return bv;
}

inline PfxBool pfxTestBv(const PfxBv &bvA,const PfxBv &bvB)
{
	PfxLargePosition centerB = bvB.center;
	centerB.changeSegment(bvA.centerSegment);
	if(fabs(bvA.centerOffset[0] - centerB.offset[0]) > (bvA.extent[0] + bvB.extent[0])) return false;
	if(fabs(bvA.centerOffset[1] - centerB.offset[1]) > (bvA.extent[1] + bvB.extent[1])) return false;
	if(fabs(bvA.centerOffset[2] - centerB.offset[2]) > (bvA.extent[2] + bvB.extent[2])) return false;
	return true;
}

inline PfxFloat pfxCalcVolume(const PfxBv &bv)
{
	PfxFloat extent0 = bv.extent[0];
	PfxFloat extent1 = bv.extent[1];
	PfxFloat extent2 = bv.extent[2];
	return 8.0f * extent0 * extent1 * extent2;
}

inline PfxBool pfxContain(const PfxBv &bvA,const PfxBv &bvB)
{
	PfxLargePosition centerB = bvB.center;
	centerB.changeSegment(bvA.centerSegment);
	if(fabs(bvA.centerOffset[0] - centerB.offset[0]) > fabs(bvA.extent[0] - bvB.extent[0])) return false;
	if(fabs(bvA.centerOffset[1] - centerB.offset[1]) > fabs(bvA.extent[1] - bvB.extent[1])) return false;
	if(fabs(bvA.centerOffset[2] - centerB.offset[2]) > fabs(bvA.extent[2] - bvB.extent[2])) return false;
	return true;
}

inline PfxFloat pfxLengthSqr(const PfxBv &bvA,const PfxBv &bvB)
{
	PfxLargePosition centerB = bvB.center;
	centerB.changeSegment(bvA.centerSegment);
	return lengthSqr(pfxReadVector3(bvA.centerOffset) - centerB.offset);
}

#endif

} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_BV_H
