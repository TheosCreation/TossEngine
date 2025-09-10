/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_sphere.h"
#include "pfx_intersect_common.h"
#include "pfx_intersect_ray_sphere.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectRaySphere(const PfxRayInputInternal &ray,PfxRayOutputInternal &out,const PfxSphere &sphere,const PfxTransform3 &transform)
{
	PfxVector3 v = ray.m_startPosition - transform.getTranslation();

	PfxFloat a = dot(ray.m_direction,ray.m_direction);
	PfxFloat b = dot(v,ray.m_direction);
	PfxFloat c = dot(v,v) - sphere.m_radius * sphere.m_radius;

	if(c < 0.0f) return false;

	PfxFloat d = b * b - a * c;
	
	if(d < 0.0f || fabsf(a) < 0.00001f) return false;
	
	PfxFloat tt = ( -b - sqrtf(d) ) / a;
	
	if(tt < 0.0f || tt > 1.0f) return false;
	
	if(tt < out.m_variable) {
		out.m_contactFlag = true;
		out.m_variable = tt;
		out.m_subData.setType(PfxSubData::SHAPE_INFO);

        // If the sphere radius is very small compared to the length of the ray it is possible that floating point precision is not enough.
        const PfxVector3 sphereToHitPoint( ray.m_startPosition + tt * ray.m_direction - transform.getTranslation() );
        const PfxFloatInVec sphereToHitPointSquaredLength = dot( sphereToHitPoint, sphereToHitPoint );
        if( sphereToHitPointSquaredLength > PfxFloatInVec( 0.00001f ) )
        {
            out.m_contactNormal = sphereToHitPoint / sqrtf( sphereToHitPointSquaredLength );
        }
        else
		{
            // If we get here than the sphere radius is so small compared to the length of the ray that the sphere can be thought as 
            // having the size of a point. In this case the normal can only be the opposite of the ray direction
            out.m_contactNormal = ray.m_direction / (-a);
        }

		return true;
	}
	
	return false;
}

} //namespace pfxv4
} //namespace sce
