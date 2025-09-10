/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_intersect_common.h"
#include "../../../include/physics_effects/base_level/collision/pfx_sphere.h"
#include "pfx_intersect_moving_sphere_sphere.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectMovingSphereSphere(const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,const PfxSphere &sphere,const PfxTransform3 &transform)
{
	PfxVector3 spherePos = transform.getTranslation();
	PfxVector3 startPos = sphereIn.m_startPosition;
	PfxVector3 rayDir = sphereIn.m_direction;
	PfxFloat rayRadius = sphereIn.m_radius;
	
	PfxFloat tmpVariable=0.0f;
	if(pfxIntersectRaySphere(startPos,rayDir,spherePos,rayRadius+sphere.m_radius,tmpVariable) && tmpVariable < sphereOut.m_variable) {
		PfxVector3 stopPoint = startPos + tmpVariable * rayDir;

		PfxVector3 normal = stopPoint - spherePos;
		PfxFloat distance = length(normal);

		if (distance < SCE_PFX_INTERSECT_COMMON_EPSILON)
			normal = PfxVector3::xAxis();
		else
			normal = normal / distance;

		if (tmpVariable > 0.0f) {
			sphereOut.m_contactPoint = stopPoint - rayRadius * normal;
			sphereOut.m_contactNormal = normal;
			sphereOut.m_contactFlag = true;
			sphereOut.m_variable = tmpVariable;
			sphereOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		}
		else {
			PfxVector3 contactPoint = spherePos + sphere.m_radius * normal;
			PfxFloat cpDist = length(contactPoint - startPos);
			if (cpDist > rayRadius) {
				contactPoint = startPos + rayRadius * normal;
			}

			if (dot((startPos - contactPoint), normal) < 0.0f) {
				normal = -normal;
			}

			sphereOut.m_contactPoint = contactPoint;
			sphereOut.m_contactNormal = normal;
			sphereOut.m_contactFlag = true;
			sphereOut.m_variable = tmpVariable;
			sphereOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		}

		return true;
	}

	return false;
}

} //namespace pfxv4
} //namespace sce
