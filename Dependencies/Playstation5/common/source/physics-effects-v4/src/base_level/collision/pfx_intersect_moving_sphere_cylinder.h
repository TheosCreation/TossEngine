/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_INTERSECT_MOVING_SPHERE_CYLINDER_H
#define _SCE_PFX_INTERSECT_MOVING_SPHERE_CYLINDER_H

#include "../../../include/physics_effects/base_level/collision/pfx_ray.h"
#include "../../../include/physics_effects/base_level/collision/pfx_cylinder.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectMovingSphereCylinder(const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,const PfxCylinder &cylinder,const PfxTransform3 &transform);

} //namespace pfxv4
} //namespace sce
 
#endif // _SCE_PFX_INTERSECT_MOVING_SPHERE_CYLINDER_H
