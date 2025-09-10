/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_VEC_INT3_H
#define _SCE_PFX_VEC_INT3_H

#include "pfx_common.h"

namespace sce {
namespace pfxv4 {

#ifdef PFX_ENABLE_AVX // simd

static SCE_PFX_FORCE_INLINE
__m128i pfxMulEpi128(const __m128i &a,const __m128i &b)
{
	__m128i t1 = _mm_mul_epi32(a,b);
	__m128i t2 = _mm_mul_epi32( _mm_srli_si128(a,4), _mm_srli_si128(b,4));
	return _mm_unpacklo_epi32(_mm_shuffle_epi32(t1,_MM_SHUFFLE (0,0,2,0)),_mm_shuffle_epi32(t2,_MM_SHUFFLE (0,0,2,0)));
}

#ifdef _MSC_VER
#pragma warning(push)               // [SMS_CHANGE] disable compiler warnings
#pragma warning(disable: 4267 4201) // [SMS_CHANGE] // nonstandard extension used : nameless struct/union
#endif

struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxVecInt3
{
	union {
		struct {
			PfxInt32 x,y,z,w;
		};
		PfxInt32 elem[4];
		__m128i vi128;
	};
	
	PfxVecInt3() {vi128 = _mm_setzero_si128();}
	PfxVecInt3(__m128i vi128_) {vi128 = vi128_;};
	PfxVecInt3(const PfxVector3 &vec) {vi128 = _mm_cvtps_epi32(sce_vectormath_asm128(vec.get128()));}
	PfxVecInt3(PfxFloat fx,PfxFloat fy,PfxFloat fz) {vi128 = _mm_cvtps_epi32(_mm_set_ps(0,fz,fy,fx));}
	PfxVecInt3(PfxInt32 iv) {vi128 = _mm_set1_epi32(iv);}
	PfxVecInt3(PfxInt32 ix,PfxInt32 iy,PfxInt32 iz) {vi128 = _mm_set_epi32(0,iz,iy,ix);}

	inline PfxVecInt3 &operator =( const PfxVecInt3 &vec);

	inline PfxInt32 get(PfxInt32 i) const {return elem[i];}
	inline PfxInt32 getX() const {return x;}
	inline PfxInt32 getY() const {return y;}
	inline PfxInt32 getZ() const {return z;}
	inline void set(PfxInt32 i,PfxInt32 v) {elem[i] = v;}
	inline void setX(PfxInt32 v) {x = v;}
	inline void setY(PfxInt32 v) {y = v;}
	inline void setZ(PfxInt32 v) {z = v;}

	inline const PfxVecInt3 operator +( const PfxVecInt3 & vec ) const;
	inline const PfxVecInt3 operator -( const PfxVecInt3 & vec ) const;
	inline const PfxVecInt3 operator *( PfxInt32 scalar ) const;
//	inline const PfxVecInt3 operator /( PfxInt32 scalar ) const;

	inline PfxVecInt3 & operator +=( const PfxVecInt3 & vec );
	inline PfxVecInt3 & operator -=( const PfxVecInt3 & vec );
	inline PfxVecInt3 & operator *=( PfxInt32 scalar );
//	inline PfxVecInt3 & operator /=( PfxInt32 scalar );

	inline const PfxVecInt3 operator -() const;

	operator PfxVector3() const
	{
		return PfxVector3(sce_vectormath_asfloat4(_mm_cvtepi32_ps(vi128)));
	}
};

#ifdef _MSC_VER
#pragma warning(pop) // [SMS_CHANGE] disable compiler warnings
#endif

inline PfxVecInt3 &PfxVecInt3::operator =( const PfxVecInt3 &vec)
{
	vi128 = vec.vi128;
	return *this;
}

inline const PfxVecInt3 PfxVecInt3::operator +( const PfxVecInt3 & vec ) const
{
	return PfxVecInt3(_mm_add_epi32(vi128,vec.vi128));
}

inline const PfxVecInt3 PfxVecInt3::operator -( const PfxVecInt3 & vec ) const
{
	return PfxVecInt3(_mm_sub_epi32(vi128,vec.vi128));
}

inline const PfxVecInt3 PfxVecInt3::operator *( PfxInt32 scalar ) const
{
	return PfxVecInt3(pfxMulEpi128(vi128,_mm_set1_epi32(scalar)));
}
/*
inline const PfxVecInt3 PfxVecInt3::operator /( PfxInt32 scalar ) const
{
	return PfxVecInt3(x/scalar, y/scalar, z/scalar);
}
*/
inline PfxVecInt3 &PfxVecInt3::operator +=( const PfxVecInt3 & vec )
{
	*this = *this + vec;
	return *this;
}

inline PfxVecInt3 &PfxVecInt3::operator -=( const PfxVecInt3 & vec )
{
	*this = *this - vec;
	return *this;
}

inline PfxVecInt3 &PfxVecInt3::operator *=( PfxInt32 scalar )
{
	*this = *this * scalar;
	return *this;
}
/*
inline PfxVecInt3 &PfxVecInt3::operator /=( PfxInt32 scalar )
{
	*this = *this / scalar;
	return *this;
}
*/
inline const PfxVecInt3 PfxVecInt3::operator -() const
{
	return PfxVecInt3(_mm_sub_epi32(_mm_setzero_si128(),vi128));
}

inline const PfxVecInt3 operator *( PfxInt32 scalar, const PfxVecInt3 & vec )
{
	return vec * scalar;
}

inline const PfxVecInt3 mulPerElem( const PfxVecInt3 & vec0, const PfxVecInt3 & vec1 )
{
	return PfxVecInt3(pfxMulEpi128(vec0.vi128,vec1.vi128));
}
/*
inline const PfxVecInt3 divPerElem( const PfxVecInt3 & vec0, const PfxVecInt3 & vec1 )
{
	return PfxVecInt3(_mm_div_epi32(vec0.vi128,vec1.vi128));
}
*/
inline const PfxVecInt3 absPerElem( const PfxVecInt3 & vec )
{
	return PfxVecInt3(_mm_abs_epi32(vec.vi128));
}

inline const PfxVecInt3 maxPerElem( const PfxVecInt3 & vec0, const PfxVecInt3 & vec1 )
{
	return PfxVecInt3(_mm_max_epi32(vec0.vi128,vec1.vi128));
}

inline const PfxVecInt3 minPerElem( const PfxVecInt3 & vec0, const PfxVecInt3 & vec1 )
{
	return PfxVecInt3(_mm_min_epi32(vec0.vi128,vec1.vi128));
}

#else // scalar

class SCE_PFX_API SCE_PFX_ALIGNED(16) PfxVecInt3
{
private:
	PfxInt32 x,y,z,w;

public:
	PfxVecInt3() {x=y=z=w=0;}
	PfxVecInt3(const PfxVector3 &vec) {x=(PfxInt32)vec[0];y=(PfxInt32)vec[1];z=(PfxInt32)vec[2];w=0;}
	PfxVecInt3(PfxFloat fx,PfxFloat fy,PfxFloat fz) {x=(PfxInt32)fx;y=(PfxInt32)fy;z=(PfxInt32)fz;w=0;}
	PfxVecInt3(PfxInt32 iv) {x=y=z=iv;w=0;}
	PfxVecInt3(PfxInt32 ix,PfxInt32 iy,PfxInt32 iz) {x=ix;y=iy;z=iz;w=0;}

	inline PfxVecInt3 &operator =( const PfxVecInt3 &vec);

	inline PfxInt32 get(PfxInt32 i) const {return *(&x+i);}
	inline PfxInt32 getX() const {return x;}
	inline PfxInt32 getY() const {return y;}
	inline PfxInt32 getZ() const {return z;}
	inline void set(PfxInt32 i,PfxInt32 v) {*(&x+i) = v;}
	inline void setX(PfxInt32 v) {x = v;}
	inline void setY(PfxInt32 v) {y = v;}
	inline void setZ(PfxInt32 v) {z = v;}

	inline const PfxVecInt3 operator +( const PfxVecInt3 & vec ) const;
	inline const PfxVecInt3 operator -( const PfxVecInt3 & vec ) const;
	inline const PfxVecInt3 operator *( PfxInt32 scalar ) const;
	inline const PfxVecInt3 operator /( PfxInt32 scalar ) const;

	inline PfxVecInt3 & operator +=( const PfxVecInt3 & vec );
	inline PfxVecInt3 & operator -=( const PfxVecInt3 & vec );
	inline PfxVecInt3 & operator *=( PfxInt32 scalar );
	inline PfxVecInt3 & operator /=( PfxInt32 scalar );

	inline const PfxVecInt3 operator -() const;

	operator PfxVector3() const
	{
		return PfxVector3((PfxFloat)x,(PfxFloat)y,(PfxFloat)z);
	}
};

inline PfxVecInt3 &PfxVecInt3::operator =( const PfxVecInt3 &vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}

inline const PfxVecInt3 PfxVecInt3::operator +( const PfxVecInt3 & vec ) const
{
	return PfxVecInt3(x+vec.x, y+vec.y, z+vec.z);
}

inline const PfxVecInt3 PfxVecInt3::operator -( const PfxVecInt3 & vec ) const
{
	return PfxVecInt3(x-vec.x, y-vec.y, z-vec.z);
}

inline const PfxVecInt3 PfxVecInt3::operator *( PfxInt32 scalar ) const
{
	return PfxVecInt3(x*scalar, y*scalar, z*scalar);
}

inline const PfxVecInt3 PfxVecInt3::operator /( PfxInt32 scalar ) const
{
	return PfxVecInt3(x/scalar, y/scalar, z/scalar);
}

inline PfxVecInt3 &PfxVecInt3::operator +=( const PfxVecInt3 & vec )
{
	*this = *this + vec;
	return *this;
}

inline PfxVecInt3 &PfxVecInt3::operator -=( const PfxVecInt3 & vec )
{
	*this = *this - vec;
	return *this;
}

inline PfxVecInt3 &PfxVecInt3::operator *=( PfxInt32 scalar )
{
	*this = *this * scalar;
	return *this;
}

inline PfxVecInt3 &PfxVecInt3::operator /=( PfxInt32 scalar )
{
	*this = *this / scalar;
	return *this;
}

inline const PfxVecInt3 PfxVecInt3::operator -() const
{
	return PfxVecInt3(-x,-y,-z);
}

inline const PfxVecInt3 operator *( PfxInt32 scalar, const PfxVecInt3 & vec )
{
	return vec * scalar;
}

inline const PfxVecInt3 mulPerElem( const PfxVecInt3 & vec0, const PfxVecInt3 & vec1 )
{
	return PfxVecInt3(vec0.getX()*vec1.getX(), vec0.getY()*vec1.getY(), vec0.getZ()*vec1.getZ());
}

inline const PfxVecInt3 divPerElem( const PfxVecInt3 & vec0, const PfxVecInt3 & vec1 )
{
	return PfxVecInt3(vec0.getX()/vec1.getX(), vec0.getY()/vec1.getY(), vec0.getZ()/vec1.getZ());
}

inline const PfxVecInt3 absPerElem( const PfxVecInt3 & vec )
{
	return PfxVecInt3(abs(vec.getX()), abs(vec.getY()), abs(vec.getZ()));
}

inline const PfxVecInt3 maxPerElem( const PfxVecInt3 & vec0, const PfxVecInt3 & vec1 )
{
	return PfxVecInt3(
		(vec0.getX() > vec1.getX())? vec0.getX() : vec1.getX(),
		(vec0.getY() > vec1.getY())? vec0.getY() : vec1.getY(),
		(vec0.getZ() > vec1.getZ())? vec0.getZ() : vec1.getZ()
	);
}

inline const PfxVecInt3 minPerElem( const PfxVecInt3 & vec0, const PfxVecInt3 & vec1 )
{
	return PfxVecInt3(
		(vec0.getX() < vec1.getX())? vec0.getX() : vec1.getX(),
		(vec0.getY() < vec1.getY())? vec0.getY() : vec1.getY(),
		(vec0.getZ() < vec1.getZ())? vec0.getZ() : vec1.getZ()
	);
}

#endif // scalar

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_VEC_INT3_H
