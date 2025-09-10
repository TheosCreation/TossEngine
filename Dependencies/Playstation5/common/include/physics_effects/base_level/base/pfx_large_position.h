/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_LARGE_POSITION_H
#define _SCE_PFX_LARGE_POSITION_H

#include "pfx_common.h"
#include "pfx_simd_utils.h"

namespace sce {
namespace pfxv4 {

#ifdef PFX_ENABLE_AVX // simd

#ifdef _MSC_VER
#pragma warning(push)          // [SMS_CHANGE] disable compiler warnings
#pragma warning(disable: 4201) // [SMS_CHANGE] nonstandard extension used : nameless struct/union
#endif

struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxSegment {
	union {
		struct {
			PfxInt32 x,y,z,w;
		};
		__m128i vi128;
		PfxInt32 xyzw[4];
	};
	
	PfxSegment() {vi128 = _mm_setzero_si128();}
	PfxSegment(const PfxSegment &segment) {vi128 = segment.vi128;}
	PfxSegment(PfxInt32 x_,PfxInt32 y_,PfxInt32 z_) {vi128 = _mm_set_epi32(0,z_,y_,x_);}
	PfxSegment(const __m128i &vec) {vi128 = vec;}

	PfxSegment operator +( const PfxSegment & seg ) const
	{
		return PfxSegment(_mm_add_epi32(vi128,seg.vi128));
	}

	PfxSegment operator -( const PfxSegment & seg ) const
	{
		return PfxSegment(_mm_sub_epi32(vi128,seg.vi128));
	}

	PfxSegment operator *( PfxInt32 scalar ) const
	{
		return PfxSegment(pfxMulEpi128(vi128,_mm_set1_epi32(scalar)));
	}
	
	PfxSegment halve() const
	{
		return PfxSegment(_mm_srai_epi32(vi128,1));
	}

	PfxVector3 castToVector3() const
	{
		__m128 vseg = _mm_cvtepi32_ps(vi128);
		return PfxVector3(sce_vectormath_asfloat4(vseg));
	}

	PfxVector3 convertToVector3() const;
};

#ifdef _MSC_VER
#pragma warning(pop)  // [SMS_CHANGE] disable compiler warnings
#endif

inline PfxBool operator ==(const PfxSegment &segA,const PfxSegment &segB)
{
	return _mm_movemask_epi8(_mm_cmpeq_epi32(segA.vi128,segB.vi128)) == 0xffff;
}

inline PfxBool operator !=(const PfxSegment &segA,const PfxSegment &segB)
{
	return !(segA == segB);
}

inline PfxSegment maxPerElem(const PfxSegment &segA,const PfxSegment &segB)
{
	return PfxSegment(_mm_max_epi32(segA.vi128,segB.vi128));
}

inline PfxSegment minPerElem(const PfxSegment &segA,const PfxSegment &segB)
{
	return PfxSegment(_mm_min_epi32(segA.vi128,segB.vi128));
}

struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxLargePosition
{
	PfxSegment segment;
	PfxVector3 offset;
	
	PfxLargePosition() : segment() {}
	PfxLargePosition(PfxFloat scalar) : segment(),offset(scalar) {}
	PfxLargePosition(const PfxSegment &segment_,const PfxVector3 &offset_) : segment(segment_),offset(offset_) {}
	PfxLargePosition(const PfxLargePosition &lpos) : segment(lpos.segment),offset(lpos.offset) {}
	PfxLargePosition(const PfxVector3 &offset_) : segment(),offset(offset_) {}
	
	void changeSegment(const PfxSegment &segment_)
	{
		offset = offset + (segment - segment_).convertToVector3();
		segment = segment_;
	}

	PfxLargePosition operator +( const PfxLargePosition & lpos ) const
	{
		return PfxLargePosition(segment + lpos.segment,offset + lpos.offset);
	}

	PfxLargePosition operator -( const PfxLargePosition & lpos ) const
	{
		return PfxLargePosition(segment - lpos.segment,offset - lpos.offset);
	}
	
	PfxLargePosition operator +( const PfxVector3 & vec ) const
	{
		return PfxLargePosition(segment,offset + vec);
	}

	PfxLargePosition operator -( const PfxVector3 & vec ) const
	{
		return PfxLargePosition(segment,offset - vec);
	}

	PfxVector3 convertToVector3() const
	{
		return segment.convertToVector3() + offset;
	}
};

inline PfxLargePosition maxPerElem(const PfxLargePosition &lposA,const PfxLargePosition &lposB)
{
	extern PfxFloat gSegmentWidthInv;

	__m128 vsegA = _mm_cvtepi32_ps(lposA.segment.vi128);
	__m128 vsegB = _mm_cvtepi32_ps(lposB.segment.vi128);
	__m128 voffA = sce_vectormath_asm128(lposA.offset.get128());
	__m128 voffB = sce_vectormath_asm128(lposB.offset.get128());
	
	union {__m128i vi;__m128 vf;} mask;
	mask.vf = _mm_cmpgt_ps(_mm_sub_ps(vsegA,vsegB),_mm_mul_ps(_mm_sub_ps(voffB,voffA),_mm_set1_ps(gSegmentWidthInv)));
	__m128i result_seg = _mm_blendv_epi8(lposB.segment.vi128,lposA.segment.vi128,mask.vi);
	__m128 result_off = _mm_blendv_ps(voffB,voffA,mask.vf);
	
	return PfxLargePosition(PfxSegment(result_seg),PfxVector3(sce_vectormath_asfloat4(result_off)));
}

inline PfxLargePosition minPerElem(const PfxLargePosition &lposA,const PfxLargePosition &lposB)
{
	extern PfxFloat gSegmentWidthInv;

	__m128 vsegA = _mm_cvtepi32_ps(lposA.segment.vi128);
	__m128 vsegB = _mm_cvtepi32_ps(lposB.segment.vi128);
	__m128 voffA = sce_vectormath_asm128(lposA.offset.get128());
	__m128 voffB = sce_vectormath_asm128(lposB.offset.get128());
	
	union {__m128i vi;__m128 vf;} mask;
	mask.vf = _mm_cmplt_ps(_mm_sub_ps(vsegA,vsegB),_mm_mul_ps(_mm_sub_ps(voffB,voffA),_mm_set1_ps(gSegmentWidthInv)));
	__m128i result_seg = _mm_blendv_epi8(lposB.segment.vi128,lposA.segment.vi128,mask.vi);
	__m128 result_off = _mm_blendv_ps(voffB,voffA,mask.vf);
	
	return PfxLargePosition(PfxSegment(result_seg),PfxVector3(sce_vectormath_asfloat4(result_off)));
}

#else // scalar

/// @brief Segment
/// @details It represents a segment position.
struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxSegment{
	union {
		struct {
			PfxInt32 x; ///< x
			PfxInt32 y; ///< y
			PfxInt32 z; ///< z
			PfxInt32 w; ///< w
		};
		PfxInt32 xyzw[4];
	};
	
	/// 
	/// @brief Default constructor
	/// @details Construct a segment setting 0 to all elements.
	PfxSegment() : x(0),y(0),z(0),w(0) {}

	/// 
	/// @brief Copy constructor
	/// @details Copy a segment.
	/// @param segment Segment
	PfxSegment(const PfxSegment &segment) : x(segment.x),y(segment.y),z(segment.z),w(segment.w) {}

	/// 
	/// @brief Construct a segment from x,y and z elements
	/// @details Construct a segment from x,y and z elements.
	/// @param x_ x value
	/// @param y_ y value
	/// @param z_ z value
	PfxSegment(PfxInt32 x_,PfxInt32 y_,PfxInt32 z_) : x(x_),y(y_),z(z_),w(0) {}
	
	/// 
	/// @brief Add two segments
	/// @details Add two segments.
	/// @param seg Segment
	/// @return Sum of segments
	PfxSegment operator +( const PfxSegment & seg ) const
	{
		return PfxSegment(x+seg.x,y+seg.y,z+seg.z);
	}

	/// 
	/// @brief Subtract segments
	/// @details Subtract two segments.
	/// @param seg Segment
	/// @return Difference of segments
	PfxSegment operator -( const PfxSegment & seg ) const
	{
		return PfxSegment(x-seg.x,y-seg.y,z-seg.z);
	}

	/// 
	/// @brief Multiply segments
	/// @details Multiply segment by a scalar.
	/// @param scalar Scalar
	/// @return Product of a segment and scalar
	PfxSegment operator *( PfxInt32 scalar ) const
	{
		return PfxSegment(x*scalar,y*scalar,z*scalar);
	}
	
	/// 
	/// @brief Halve segment
	/// @details Halve segment.
	/// @return Halved segment
	PfxSegment halve() const
	{
		return PfxSegment(x/2,y/2,z/2);
	}

	/// 
	/// @brief Cast to PfxVector3.
	/// @return Return cast to PfxVector3
	PfxVector3 castToVector3() const
	{
		return PfxVector3((PfxFloat)x,(PfxFloat)y,(PfxFloat)z);
	}

	/// 
	/// @brief Convert to PfxVector3.
	/// @details Be careful to use this method, it may causes an accuracy drop.
	/// @return Return PfxVector3 converted from a segment
	PfxVector3 convertToVector3() const;
};

/// 
/// @brief Equal operator of two segments
/// @details Check if two segments are equal.
/// @param segA Segment
/// @param segB Segment
/// @retval true two segments are equal
/// @retval false two segment aren't equal
inline PfxBool operator ==(const PfxSegment &segA,const PfxSegment &segB)
{
	return segA.x == segB.x && segA.y == segB.y && segA.z == segB.z;
}

/// 
/// @brief Not equal operator of two segments
/// @details Check if two segments are not equal.
/// @param segA Segment
/// @param segB Segment
/// @retval true two segments aren't equal
/// @retval false two segment are equal
inline PfxBool operator !=(const PfxSegment &segA,const PfxSegment &segB)
{
	return !(segA == segB);
}

/// 
/// @brief Maximum of two segments per element
/// @details Create a segment in which each element is the maximum of specified segments.
/// @param segA Segment
/// @param segB Segment
/// @return A segment in which each element is the maximum of specified segments.
inline PfxSegment maxPerElem(const PfxSegment &segA,const PfxSegment &segB)
{
	return PfxSegment(SCE_PFX_MAX(segA.x,segB.x),SCE_PFX_MAX(segA.y,segB.y),SCE_PFX_MAX(segA.z,segB.z));
}

/// 
/// @brief Minimum of two segments per element
/// @details Create a segment in which each element is the minimum of specified segments.
/// @param segA Segment
/// @param segB Segment
/// @return A segment in which each element is the minimum of specified segments.
inline PfxSegment minPerElem(const PfxSegment &segA,const PfxSegment &segB)
{
	return PfxSegment(SCE_PFX_MIN(segA.x,segB.x),SCE_PFX_MIN(segA.y,segB.y),SCE_PFX_MIN(segA.z,segB.z));
}

/// 
/// @brief Large position
/// @details It represents a large position.
struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxLargePosition
{
	PfxSegment segment;	///< @brief Segment position
	PfxVector3 offset;	///< @brief Offset from a segment position
	
	/// 
	/// @brief Default constructor
	/// @details Construct a large position.
	PfxLargePosition() : segment() {}
	
	/// 
	/// @brief Construct a large position from a scalar value
	/// @details Construct a large position setting a specified value to a offset elements.
	/// @param scalar Scalar
	PfxLargePosition(PfxFloat scalar) : segment(),offset(scalar) {}
	
	/// 
	/// @brief Construct a large position from a segment and offset
	/// @details Construct a large position from the specified segment and offset.
	/// @param segment_ Segment
	/// @param offset_ Offset
	PfxLargePosition(const PfxSegment &segment_,const PfxVector3 &offset_) : segment(segment_),offset(offset_) {}
	
	/// 
	/// @brief Construct a large position from an offset
	/// @details Construct a large position from the specified offset. Set 0 to all elements of a segment
	/// @param offset_ Offset
	PfxLargePosition(const PfxVector3 &offset_) : segment(),offset(offset_) {}
	
	/// 
	/// @brief Copy a large position
	/// @details Construct a large position copying the specified large position.
	/// @param lpos Large position
	PfxLargePosition(const PfxLargePosition &lpos) : segment(lpos.segment),offset(lpos.offset) {}
	
	/// 
	/// @brief Change a segment
	/// @details Change a segment and an offset position is recalculated. Do not change actual position.
	/// @param segment_ Segment
	void changeSegment(const PfxSegment &segment_)
	{
		offset = offset + (segment - segment_).convertToVector3();
		segment = segment_;
	}
	
	/// 
	/// @brief Add two large positions
	/// @details Add two large positions.
	/// @param lpos Large position
	/// @return Sum of large positions
	PfxLargePosition operator +( const PfxLargePosition & lpos ) const
	{
		return PfxLargePosition(segment + lpos.segment,offset + lpos.offset);
	}

	/// 
	/// @brief Subtract two large positions
	/// @details Subtract two large positions.
	/// @param lpos Large position
	/// @return Difference of large positions
	PfxLargePosition operator -( const PfxLargePosition & lpos ) const
	{
		return PfxLargePosition(segment - lpos.segment,offset - lpos.offset);
	}
	
	/// 
	/// @brief Add a vector
	/// @details Add a vector to a large position.
	/// @param vec Vector
	/// @return Sum of large positions
	PfxLargePosition operator +( const PfxVector3 & vec ) const
	{
		return PfxLargePosition(segment,offset + vec);
	}

	/// 
	/// @brief Subtract a vector
	/// @details Subtract a vector from a large position.
	/// @param vec Vector
	/// @return Difference of a large position and vector
	PfxLargePosition operator -( const PfxVector3 & vec ) const
	{
		return PfxLargePosition(segment,offset - vec);
	}
	
	/// 
	/// @brief Convert to PfxVector3.
	/// @details Be careful to use this method, it may causes an accuracy drop.
	/// @return Return PfxVector3 converted from a large position
	PfxVector3 convertToVector3() const
	{
		return segment.convertToVector3() + offset;
	}
};

/// 
/// @brief Maximum of two large positions per element
/// @details Create a large position in which each element is the maximum of specified large positions.
/// @param lposA Large position
/// @param lposB Large position
/// @return A large position in which each element is the maximum of specified large positions.
inline PfxLargePosition maxPerElem(const PfxLargePosition &lposA,const PfxLargePosition &lposB)
{
	PfxLargePosition result = lposB;
	extern PfxFloat gSegmentWidthInv;

	PfxSegment segmentDiff = lposA.segment - lposB.segment;
	PfxVector3 offsetDiff = lposB.offset - lposA.offset;
	if((PfxFloat)segmentDiff.x > offsetDiff[0] * gSegmentWidthInv) {
		result.segment.x = lposA.segment.x;
		result.offset[0] = lposA.offset[0];
	}
	if((PfxFloat)segmentDiff.y > offsetDiff[1] * gSegmentWidthInv) {
		result.segment.y = lposA.segment.y;
		result.offset[1] = lposA.offset[1];
	}
	if((PfxFloat)segmentDiff.z > offsetDiff[2] * gSegmentWidthInv) {
		result.segment.z = lposA.segment.z;
		result.offset[2] = lposA.offset[2];
	}
	return result;
}

/// 
/// @brief Minimum of two large positions per element
/// @details Create a large position in which each element is the Minimum of specified large positions.
/// @param lposA Large position
/// @param lposB Large position
/// @return A large position in which each element is the Minimum of specified large positions.
inline PfxLargePosition minPerElem(const PfxLargePosition &lposA,const PfxLargePosition &lposB)
{
	PfxLargePosition result = lposB;
	extern PfxFloat gSegmentWidthInv;
	
	PfxSegment segmentDiff = lposA.segment - lposB.segment;
	PfxVector3 offsetDiff = lposB.offset - lposA.offset;
	if((PfxFloat)segmentDiff.x < offsetDiff[0] * gSegmentWidthInv) {
		result.segment.x = lposA.segment.x;
		result.offset[0] = lposA.offset[0];
	}
	if((PfxFloat)segmentDiff.y < offsetDiff[1] * gSegmentWidthInv) {
		result.segment.y = lposA.segment.y;
		result.offset[1] = lposA.offset[1];
	}
	if((PfxFloat)segmentDiff.z < offsetDiff[2] * gSegmentWidthInv) {
		result.segment.z = lposA.segment.z;
		result.offset[2] = lposA.offset[2];
	}
	return result;
}

#endif // scalar

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_LARGE_POSITION_H
