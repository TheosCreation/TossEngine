/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/low_level/collision/pfx_collision_detection.h"
#include "../rigidbody/pfx_context.h"
#include "pfx_detect_collision_stage.h"
#include "pfx_contact_complex.h"

namespace sce {
namespace pfxv4 {

PfxInt32 pfxDetectCollision(PfxRigidBodyContext &context,PfxRigidBodySharedParam &sharedParam, PfxContactContainer &contactContainer)
{
	PfxContext *context_ = (PfxContext*)&context;

	PfxInt32 ret = pfxCheckSharedParam(sharedParam);
	if(ret != SCE_PFX_OK) return ret;

	ret = context_->detectCollision(
		*(PfxContactComplex*)&contactContainer,
		sharedParam.timeStep, 
		sharedParam.states, 
		sharedParam.collidables);
	
	return ret;
}

PfxInt32 pfxDispatchDetectCollision(PfxRigidBodyContext &context,PfxContactContainer &contactContainer)
{
	PfxContext *context_ = (PfxContext*)&context;

	PfxInt32 ret = context_->dispatchDetectCollision(*(PfxContactComplex*)&contactContainer);
	
	return ret;
}


} //namespace pfxv4
} //namespace sce
