/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_user_custom_stage.h"
#include "pfx_context.h"
#include "../task/pfx_job_system.h"

namespace sce {
namespace pfxv4 {

void PfxUserCustomStage::job_userCustom(void *data, int uid)
{
	JobArg *arg = (JobArg*)data;

	PfxContext *context = (PfxContext*)arg->stage->getRigidBodyContext();

	if(arg->userFunc) {
		PfxInt32 ret = arg->userFunc(uid, arg->userData);
		if(ret != 0) {
			context->appendError(PfxContext::kStageUserCustomFunction, arg->dispatchId, ret);
		}
	}
}

PfxInt32 PfxUserCustomStage::execUserCustomAsync(PfxUInt32 numJobs, PfxInt32 (*func)(PfxInt32 jobId, void *userData), void *userData)
{
	PfxInt32 ret = SCE_OK;
	PfxInt32 dispatchId = getRigidBodyContext()->getDispatchId();

	if(numJobs > 0) {
		JobArg *jobArgs = getRigidBodyContext()->allocate<JobArg>(1);
		if (!jobArgs) { return SCE_PFX_ERR_OUT_OF_BUFFER; }

		JobArg &arg = *jobArgs;
		arg.stage = this;
		arg.dispatchId = dispatchId;
		arg.numJobs = numJobs;
		arg.userFunc = func;
		arg.userData = userData;

		ret = getJobSystem()->dispatch(job_userCustom, &arg, numJobs, "user custom");
		if (ret != SCE_OK) return ret;

		ret = getJobSystem()->barrier();
		if (ret != SCE_OK) return ret;
	}
	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
