/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_intersect_common.h"
#include "pfx_intersect_ray_box.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectRayBox(const PfxRayInputInternal &ray,PfxRayOutputInternal &out,const PfxBox &box,const PfxTransform3 &transform)
{
	// レイをBoxのローカル座標へ変換
	PfxTransform3 transformBox = orthoInverse(transform);
	PfxVector3 rayStartPosition = transformBox.getUpper3x3() * ray.m_startPosition + transformBox.getTranslation();
	PfxVector3 rayDirection = transformBox.getUpper3x3() * ray.m_direction;
	
	// 交差判定
	PfxFloat tmpVariable=0.0f;
	PfxVector3 tmpNormal(0.0f);
	if(pfxIntersectRayAABB(rayStartPosition,rayDirection,PfxVector3::zero(),box.m_half,tmpVariable,tmpNormal)) {
		if(tmpVariable > 0.0f && tmpVariable < out.m_variable) {
			out.m_contactFlag = true;
			out.m_variable = tmpVariable;
			out.m_contactNormal = transform.getUpper3x3() * tmpNormal;
			out.m_subData.setType(PfxSubData::SHAPE_INFO);
			return true;
		}
	}
	
	return false;
}
} //namespace pfxv4
} //namespace sce
