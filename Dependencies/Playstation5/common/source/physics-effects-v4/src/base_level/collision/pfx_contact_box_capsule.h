/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CONTACT_BOX_CAPSULE_H
#define _SCE_PFX_CONTACT_BOX_CAPSULE_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"

namespace sce {
namespace pfxv4 {

PfxFloat pfxContactBoxCapsule(
	PfxVector3 &normal,PfxPoint3 &pointA,PfxPoint3 &pointB,
	void *shapeA,const PfxTransform3 &transformA,
	void *shapeB,const PfxTransform3 &transformB,
	PfxFloat distanceThreshold = SCE_PFX_FLT_MAX);

} //namespace pfxv4
} //namespace sce

#endif //  _SCE_PFX_CONTACT_BOX_CAPSULE_H
