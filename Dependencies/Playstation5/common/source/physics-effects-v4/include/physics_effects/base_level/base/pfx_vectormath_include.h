/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_VECTORMATH_INCLUDE_H
#define _SCE_PFX_VECTORMATH_INCLUDE_H

#include <vectormath.h>

#if defined(_WIN32)
	#define SCE_PFX_USE_SIMD_VECTORMATH
	#define PFX_ENABLE_AVX
#elif defined(__ORBIS__) || defined(__PROSPERO__)// [SMS_CHANGE] add Prospero support
	#define SCE_PFX_USE_SIMD_VECTORMATH
	#define PFX_ENABLE_AVX
#endif

#ifdef SCE_PFX_USE_SIMD_VECTORMATH
	namespace sce {
	namespace pfxv4 {
	typedef sce::Vectormath::Simd::Aos::Point3     PfxPoint3;
	typedef sce::Vectormath::Simd::Aos::Vector3    PfxVector3;
	typedef sce::Vectormath::Simd::Aos::Vector4    PfxVector4;
	typedef sce::Vectormath::Simd::Aos::Quat       PfxQuat;
	typedef sce::Vectormath::Simd::Aos::Matrix3    PfxMatrix3;
	typedef sce::Vectormath::Simd::Aos::Matrix4    PfxMatrix4;
	typedef sce::Vectormath::Simd::Aos::Transform3 PfxTransform3;
	typedef sce::Vectormath::Simd::floatInVec		PfxFloatInVec;
	typedef sce::Vectormath::Simd::boolInVec		PfxBoolInVec;
	} //namespace pfxv4
	} //namespace sce
#else
	namespace sce {
	namespace pfxv4 {
	typedef sce::Vectormath::Scalar::Aos::Point3     PfxPoint3;
	typedef sce::Vectormath::Scalar::Aos::Vector3    PfxVector3;
	typedef sce::Vectormath::Scalar::Aos::Vector4    PfxVector4;
	typedef sce::Vectormath::Scalar::Aos::Quat       PfxQuat;
	typedef sce::Vectormath::Scalar::Aos::Matrix3    PfxMatrix3;
	typedef sce::Vectormath::Scalar::Aos::Matrix4    PfxMatrix4;
	typedef sce::Vectormath::Scalar::Aos::Transform3 PfxTransform3;
	typedef sce::Vectormath::Scalar::floatInVec		PfxFloatInVec;
	typedef sce::Vectormath::Scalar::boolInVec		PfxBoolInVec;
	} //namespace pfxv4
	} //namespace sce
#endif

#endif // _SCE_PFX_VECTORMATH_INCLUDE_H
