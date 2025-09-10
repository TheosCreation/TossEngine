/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_intersect_common.h"
#include "pfx_intersect_ray_capsule.h"

namespace sce {
namespace pfxv4 {

// [SMS_CHANGE] START
static PfxVector3 pfxGenerateSafeNormal( const PfxVector3& contactPointOffset, const PfxVector3& localRayDirection ) {
	// If the capsule radius is very small compared to the length of the ray it is possible that floating point
	// precision is not enough and the resulting contact point could be (0,0,0)
	const PfxFloatInVec centerToHitPointSquaredLength = dot( contactPointOffset, contactPointOffset );
	PfxVector3 localNormal;
	if( centerToHitPointSquaredLength > PfxFloatInVec( 0.00001f ) )
	{
		localNormal = contactPointOffset / sqrtf( centerToHitPointSquaredLength );
	}
	else
	{
		// If we get here than the radius is so small compared to the length of the ray that the capsule can be thought as 
		// having the size of a point. In this case the normal can only be the opposite of the ray direction
		localNormal = -normalize( localRayDirection );
	}

	return localNormal;
}
// [SMS_CHANGE] END

PfxBool pfxIntersectRayCapsule(const PfxRayInputInternal &ray,PfxRayOutputInternal &out,const PfxCapsule &capsule,const PfxTransform3 &transform)
{
	// レイをCapsuleのローカル座標へ変換
	PfxTransform3 transformCapsule = orthoInverse(transform);
	PfxVector3 startPosL = transformCapsule.getUpper3x3() * ray.m_startPosition + transformCapsule.getTranslation();
	PfxVector3 rayDirL = transformCapsule.getUpper3x3() * ray.m_direction;
	
	PfxFloat radSqr = capsule.m_radius * capsule.m_radius;

	// 始点がカプセルの内側にあるか判定
	{
		PfxFloat h = startPosL[0];
		h = SCE_PFX_CLAMP(h,-capsule.m_halfLen,capsule.m_halfLen);
		PfxVector3 Px(h,0,0);
		PfxFloat sqrLen = lengthSqr(startPosL-Px);
		if(sqrLen <= radSqr) return false;
	}

	// カプセルの胴体との交差判定
	do {
		PfxVector3 P(startPosL);
		PfxVector3 D(rayDirL);
		
		P[0] = 0.0f;
		D[0] = 0.0f;
		
		PfxFloat a = dot(D,D);
		PfxFloat b = dot(P,D);
		PfxFloat c = dot(P,P) - radSqr;
		
		PfxFloat d = b * b - a * c;
		
		if(d < 0.0f) return false; // レイは逸れている
		if(fabsf(a) < 0.00001f) break; // レイがX軸に平行
		
		PfxFloat tt = ( -b - sqrtf(d) ) / a;
		
		if(tt < 0.0f || tt > 1.0f) break;
		
		if(tt < out.m_variable) {
			PfxVector3 cp = startPosL + tt * rayDirL;
			
			if(fabsf(cp[0]) <= capsule.m_halfLen) {
				cp[0] = 0.0f;
				out.m_contactFlag = true;
				out.m_variable = tt;
				const PfxVector3 localNormal = pfxGenerateSafeNormal( cp, rayDirL ); // [SMS_CHANGE]
				out.m_contactNormal = transform.getUpper3x3() * localNormal; // [SMS_CHANGE]
				out.m_subData.setType(PfxSubData::SHAPE_INFO);
				return true;
			}
		}
	} while(0);
	
	// カプセルの両端にある球体との交差判定
	PfxFloat a = dot(rayDirL,rayDirL);
	
	const PfxVector3 capP[2] = {PfxVector3(capsule.m_halfLen,0.0f,0.0f),PfxVector3(-capsule.m_halfLen,0.0f,0.0f)};
	PfxFloat tmin = 999.0f;
	int imin = 0;

	for(int i=0;i<2;i++) {
		PfxVector3 v = startPosL - capP[i];

		PfxFloat b = dot(v,rayDirL);
		PfxFloat c = dot(v,v) - radSqr;

		PfxFloat d = b * b - a * c;
		
		if(d < 0.0f) continue;
		
		PfxFloat tt = ( -b - sqrtf(d) ) / a;
		
		if(tt < 0.0f || tt > 1.0f) continue;
		
		if(tt < tmin) {
			imin = i;
			tmin = tt;
		}
	}
	
	if(tmin < out.m_variable) {
		PfxVector3 cp = startPosL + tmin * rayDirL;
		out.m_contactFlag = true;
		out.m_variable = tmin;
		const PfxVector3 localNormal = pfxGenerateSafeNormal( cp-capP[imin], rayDirL );// [SMS_CHANGE]
		out.m_contactNormal = transform.getUpper3x3() * localNormal;// [SMS_CHANGE]
		out.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	
	return false;
}
} //namespace pfxv4
} //namespace sce
