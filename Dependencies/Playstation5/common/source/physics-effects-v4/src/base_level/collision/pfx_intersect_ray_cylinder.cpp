/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_intersect_common.h"
#include "pfx_intersect_ray_cylinder.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectRayCylinder(const PfxRayInputInternal &ray,PfxRayOutputInternal &out,const PfxCylinder &cylinder,const PfxTransform3 &transform)
{
	// レイを円柱のローカル座標へ変換
	PfxTransform3 transformCapsule = orthoInverse(transform);
	PfxVector3 startPosL = transformCapsule.getUpper3x3() * ray.m_startPosition + transformCapsule.getTranslation();
	PfxVector3 rayDirL = transformCapsule.getUpper3x3() * ray.m_direction;
	
	PfxFloat radSqr = cylinder.m_radius * cylinder.m_radius;

	// 始点が円柱の内側にあるか判定
	{
		PfxFloat h = startPosL[0];
		if(-cylinder.m_halfLen <= h && h <= cylinder.m_halfLen) {
			PfxVector3 Px(h,0,0);
			PfxFloat sqrLen = lengthSqr(startPosL-Px);
			if(sqrLen <= radSqr) return false;
		}
	}

	// 円柱の胴体との交差判定
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
			
			if(fabsf(cp[0]) <= cylinder.m_halfLen) {
				cp[0] = 0.0f;
				out.m_contactFlag = true;
				out.m_variable = tt;
				out.m_contactNormal = transform.getUpper3x3() * normalize(cp);
				out.m_subData.setType(PfxSubData::SHAPE_INFO);

				// if the cylinder is too small and/or distant then the contact point on YZ might have zero length
				// causing the normal to be NaN
				cp[0] = 0.0f;
				PfxFloatInVec yzSquaredLength = cp.getY()*cp.getY() + cp.getZ()*cp.getZ();
				if( yzSquaredLength > PfxFloatInVec( 0.f ) ) {
					out.m_contactNormal = transform.getUpper3x3() * (cp / sqrtf(yzSquaredLength));
				}
				else {
					out.m_contactNormal = normalize( -ray.m_direction );
				}

				return true;
			}
		}
	} while(0);
	
	// 円柱の両端にある平面との交差判定
	{
		if(fabsf(rayDirL[0]) < 0.00001f) return false;
		
		PfxFloat t1 = ( cylinder.m_halfLen - startPosL[0] ) / rayDirL[0];
		PfxFloat t2 = ( - cylinder.m_halfLen - startPosL[0] ) / rayDirL[0];

		PfxFloat tt = SCE_PFX_MIN(t1,t2);
		
		if(tt < 0.0f || tt > 1.0f) return false;

		PfxVector3 p = startPosL + tt * rayDirL;
		p[0] = 0.0f;

		if(lengthSqr(p) < radSqr && tt < out.m_variable) {
			PfxVector3 cp = startPosL + tt * rayDirL;
			out.m_contactFlag = true;
			out.m_variable = tt;
			out.m_contactNormal = transform.getUpper3x3() * ((cp[0]>0.0f)?PfxVector3(1.0,0.0,0.0):PfxVector3(-1.0,0.0,0.0));
			out.m_subData.setType(PfxSubData::SHAPE_INFO);
			return true;
		}
	}
	
	return false;
}
} //namespace pfxv4
} //namespace sce
