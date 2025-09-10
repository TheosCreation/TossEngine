/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_large_position.h"

namespace sce {
namespace pfxv4 {

extern PfxInt32 gSegmentWidth;

#ifdef PFX_ENABLE_AVX

PfxVector3 PfxSegment::convertToVector3() const
{
	__m128 vseg = _mm_cvtepi32_ps(vi128);
	return PfxVector3(sce_vectormath_asfloat4(_mm_mul_ps(vseg,_mm_set1_ps((float)gSegmentWidth))));
}

#else

PfxVector3 PfxSegment::convertToVector3() const
{
	return PfxVector3((PfxFloat)x,(PfxFloat)y,(PfxFloat)z) * (PfxFloat)gSegmentWidth;
}

#endif

} //namespace pfxv4
} //namespace sce
