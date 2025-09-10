/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/low_level/rigidbody/pfx_user_custom_function.h"
#include "../rigidbody/pfx_context.h"

namespace sce {
namespace pfxv4 {

PfxInt32 pfxUserCustomFunction(PfxRigidBodyContext &context, PfxUInt32 numJobs, UserCustomFunc func, void *userData)
{
	PfxContext *context_ = (PfxContext*)&context;
	
	PfxInt32 ret = context_->execUserCustomFunction(numJobs, func, userData);
	
	return ret;
}

PfxInt32 pfxDispatchUserCustomFunction(PfxRigidBodyContext &context, PfxUInt32 numJobs, UserCustomFunc func, void *userData)
{
	PfxContext *context_ = (PfxContext*)&context;
	
	PfxInt32 ret = context_->dispatchExecUserCustomFunction(numJobs, func, userData);
	
	return ret;
}


} //namespace pfxv4
} //namespace sce
