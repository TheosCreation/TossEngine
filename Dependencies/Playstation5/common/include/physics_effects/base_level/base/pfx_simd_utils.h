/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_SIMD_UTILS_H
#define _SCE_PFX_SIMD_UTILS_H

#include "pfx_common.h"
#include "pfx_vec_int3.h"

namespace sce {
namespace pfxv4 {

static SCE_PFX_FORCE_INLINE
PfxVector3 pfxFloorPerElem(const PfxVector3 &vec)
{
	#ifdef PFX_ENABLE_AVX
	return PfxVector3(sce_vectormath_floorf4(vec.get128()));
	#else
	return PfxVector3(floorf(vec[0]),floorf(vec[1]),floorf(vec[2]));
	#endif
}

static SCE_PFX_FORCE_INLINE
PfxVector3 pfxCeilPerElem(const PfxVector3 &vec)
{
	#ifdef PFX_ENABLE_AVX
	return PfxVector3(sce_vectormath_ceilf4(vec.get128()));
	#else
	return PfxVector3(ceilf(vec[0]),ceilf(vec[1]),ceilf(vec[2]));
	#endif
}

static SCE_PFX_FORCE_INLINE PfxVector3 pfxReadVector3(const PfxFloat* fptr)
{
	PfxVector3 v;
	PfxFloat f[4] = { fptr[0], fptr[1], fptr[2], 0.0f };
	loadXYZ(v, f);
	return v;
}

static SCE_PFX_FORCE_INLINE PfxPoint3 pfxReadPoint3(const PfxFloat* fptr)
{
	PfxPoint3 v;
	PfxFloat f[4] = { fptr[0], fptr[1], fptr[2], 0.0f };
	loadXYZ(v, f);
	return v;
}

static SCE_PFX_FORCE_INLINE PfxQuat pfxReadQuat(const PfxFloat* fptr)
{
	PfxQuat vq;
	loadXYZW(vq, fptr);
	return vq;
}

static SCE_PFX_FORCE_INLINE void pfxStoreVector3(const PfxVector3 &src, PfxFloat* fptr)
{
#if 0
	storeXYZ(src, fptr);
#else
	fptr[0] = src[0]; 
	fptr[1] = src[1];
	fptr[2] = src[2];
#endif
}

static SCE_PFX_FORCE_INLINE void pfxStorePoint3(const PfxPoint3 &src, PfxFloat* fptr)
{
#if 0
	storeXYZ(src, fptr);
#else
	fptr[0] = src[0]; 
	fptr[1] = src[1];
	fptr[2] = src[2];
#endif
}

static SCE_PFX_FORCE_INLINE void pfxStoreQuat(const PfxQuat &src, PfxFloat* fptr)
{
#if 0
	storeXYZW(src, fptr);
#else
	fptr[0] = src[0]; 
	fptr[1] = src[1];
	fptr[2] = src[2];
	fptr[3] = src[3];
#endif
}

static SCE_PFX_FORCE_INLINE
void pfxGetPlaneSpace(const PfxVector3& n, PfxVector3& t1, PfxVector3& t2)
{
	if(fabsf(n[2]) > 0.707f) {
		// y-z plane
		PfxFloatInVec a(n[1]*n[1] + n[2]*n[2]);
		PfxFloatInVec k = rsqrtf(a);
		t1 = PfxVector3(0.0f,-n[2],n[1]) * k;
		t2 = cross(n,t1);
	}
	else {
		// x-y plane
		PfxFloatInVec a(n[0]*n[0] + n[1]*n[1]);
		PfxFloatInVec k = rsqrtf(a);
		t1 = PfxVector3(-n[1],n[0],0.0f) * k;
		t2 = cross(n,t1);
	}
}

static SCE_PFX_FORCE_INLINE
void pfxGetRotationAngleAndAxis(const PfxQuat &unitQuat,PfxFloat &angle,PfxVector3 &axis)
{
	const PfxFloat epsilon=0.00001f;

	if(fabsf(unitQuat.getW()) < 1.0f-epsilon && lengthSqr(unitQuat.getXYZ()) > epsilon) {
		PfxFloatInVec angleHalf = acosf(unitQuat.getW());
		PfxFloatInVec sinAngleHalf = sinf(angleHalf);

		if(fabsf(sinAngleHalf) > 1.0e-10f) {
			axis = unitQuat.getXYZ() / sinAngleHalf;
		} else {
			axis = unitQuat.getXYZ();
		}
		angle = 2.0f*angleHalf;
	} else {
		angle = 0.0f;
		axis = PfxVector3::xAxis();
	}
}

static SCE_PFX_FORCE_INLINE
PfxFloat pfxSafeAtan2(PfxFloat y,PfxFloat x)
{
	if(SCE_PFX_SQR(x) < 0.000001f || SCE_PFX_SQR(y) < 0.000001f) {
		return 0.0f;
	}
	return atan2f(y,x);
}

static SCE_PFX_FORCE_INLINE
PfxVector3 pfxSafeNormalize(const PfxVector3 &vec)
{
	float lenSqr = lengthSqr( vec );

	if( lenSqr > 0.000001f ) {
		return normalize(vec);
	}else {
		return PfxVector3::xAxis();
	}
}

static SCE_PFX_FORCE_INLINE
PfxVecInt3 pfxConvertCoordWorldToLocal(const PfxVector3 &coord,const PfxVector3 &center,const PfxVector3 &half)
{
	const PfxVector3 sz(65535.0f);
	PfxVector3 q = divPerElem(coord - center + half,2.0f*half);
	q = clampPerElem(q,PfxVector3::zero(),PfxVector3(1.0f));
	q = mulPerElem(q,sz);
	return PfxVecInt3(q);
}

static SCE_PFX_FORCE_INLINE
void pfxConvertCoordWorldToLocal(
	const PfxVector3 &center,const PfxVector3 &half,
	const PfxVector3 &coordMin,const PfxVector3 &coordMax,
	PfxVecInt3 &localMin,PfxVecInt3 &localMax)
{
	const PfxVector3 sz(65535.0f);
	PfxVector3 qmin = divPerElem(coordMin - center + half,2.0f*half);
	qmin = clampPerElem(qmin,PfxVector3::zero(),PfxVector3(1.0f)); // clamp 0.0 - 1.0
	qmin = mulPerElem(qmin,sz);
	
	PfxVector3 qmax = divPerElem(coordMax - center + half,2.0f*half);
	qmax = clampPerElem(qmax,PfxVector3::zero(),PfxVector3(1.0f)); // clamp 0.0 - 1.0
	qmax = mulPerElem(qmax,sz);
	localMin = PfxVecInt3(pfxFloorPerElem(qmin));
	localMax = PfxVecInt3(pfxCeilPerElem(qmax));
}

static SCE_PFX_FORCE_INLINE
PfxVector3 pfxConvertCoordLocalToWorld(const PfxVecInt3 &coord,const PfxVector3 &center,const PfxVector3 &half)
{
	PfxVector3 sz(65535.0f),vcoord(coord);
	PfxVector3 q = divPerElem(vcoord,sz);
	return mulPerElem(q,2.0f*half) + center - half;
}

static SCE_PFX_FORCE_INLINE
PfxTransform3 pfxIntegrateTransform(PfxFloat lambda,const PfxTransform3 &tr,const PfxVector3 &linVel,const PfxVector3 &angVel)
{
	PfxQuat orientation(tr.getUpper3x3());
	PfxQuat dq = PfxQuat(angVel,0.0f) * orientation * 0.5f;
	return PfxTransform3(
		normalize(orientation + dq * lambda),
		tr.getTranslation() + linVel * lambda);
}

static SCE_PFX_FORCE_INLINE
PfxTransform3 pfxInterpolateTransform( PfxFloat lambda, const PfxTransform3 &tr0, const PfxTransform3 &tr1 )
{
	const PfxQuat quat0( tr0.getUpper3x3() );
	const PfxQuat quat1( tr1.getUpper3x3() );
	const PfxQuat newOrientation = slerp( lambda, quat0, quat1 );
	const PfxVector3 newPosition = lerp( lambda, tr0.getTranslation(), tr1.getTranslation() );
	return PfxTransform3( newOrientation, newPosition );
}

static SCE_PFX_FORCE_INLINE
PfxBool pfxIsOrthogonal(const PfxMatrix3 &m)
{
	const PfxFloat epsilon=0.00001f;
	
	PfxMatrix3 mm = PfxMatrix3::identity() - transpose(m) * m;
	return	lengthSqr(mm.getCol0()) < epsilon && 
			lengthSqr(mm.getCol1()) < epsilon && 
			lengthSqr(mm.getCol2()) < epsilon;
}

#ifdef PFX_ENABLE_AVX
template <int X, int Y, int Z, int W>
static SCE_PFX_FORCE_INLINE __m128 pfxShufflePs( __m128 vec0 )
{
#if defined(__AVX__)
	return _mm_permute_ps( vec0, _MM_SHUFFLE( W & 3, Z & 3, Y & 3, X & 3 ) );
#else
	return _mm_shuffle_ps( vec0, vec0, _MM_SHUFFLE( W & 3, Z & 3, Y & 3, X & 3 ) );
#endif
}
#endif

} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_SIMD_UTILS_H
