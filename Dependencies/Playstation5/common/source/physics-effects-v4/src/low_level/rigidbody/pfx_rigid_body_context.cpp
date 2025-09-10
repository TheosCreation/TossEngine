/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/low_level/rigidbody/pfx_rigid_body_context.h"
#include "pfx_context.h"

#define SCE_PFX_DEFAULT_SEGMENT_WIDTH 1000

namespace sce {
namespace pfxv4 {

PfxInt32 pfxRigidBodyContextInit(PfxRigidBodyContext &context,const PfxRigidBodyContextParam &param,void *workBuff,PfxInt32 workBytes)
{
	SCE_PFX_ASSERT(sizeof(PfxContext) <= sizeof(PfxRigidBodyContext));
	
	PfxContext *context_ = new(&context) PfxContext;

	if(param.userJobManagerSequenceFactoryInterface) {
		PfxInt32 ret = context_->initialize(workBuff, workBytes, param.numWorkerThreads, param.multiThreadFlag, (sce::Job::SequenceFactoryInterface*)param.userJobManagerSequenceFactoryInterface);
		if(ret != SCE_PFX_OK) return ret;
	}
	else {
		PfxInt32 ret = context_->initialize(workBuff, workBytes, param.numWorkerThreads, param.multiThreadFlag, param.affinityMask, param.threadPriority);
		if(ret != SCE_PFX_OK) return ret;
	}
	
	extern PfxInt32 gSegmentWidth;
	extern PfxFloat gSegmentWidthInv;

	if(param.segmentWidth == 0) {
		gSegmentWidth = SCE_PFX_DEFAULT_SEGMENT_WIDTH;
		context_->setEnableLargePosition(false);
	}
	else {
		gSegmentWidth = param.segmentWidth;
		context_->setEnableLargePosition(true);
	}

	gSegmentWidthInv = 1.0f/(PfxFloat)gSegmentWidth;

	context_->setPersistentThread(param.enablePersistentThreads);

	return SCE_PFX_OK;
}

PfxInt32 pfxRigidBodyContextTerm(PfxRigidBodyContext &context)
{
	PfxContext *context_ = (PfxContext*)&context;
	
	context_->finalize();
	
	return SCE_PFX_OK;
}

void pfxRigidBodyContextResetSegmentWidth(PfxRigidBodyContext &context,PfxInt32 segmentWidth)
{
	PfxContext *context_ = (PfxContext*)&context;

	extern PfxInt32 gSegmentWidth;
	extern PfxFloat gSegmentWidthInv;

	if(segmentWidth == 0) {
		gSegmentWidth = SCE_PFX_DEFAULT_SEGMENT_WIDTH;
		context_->setEnableLargePosition(false);
	}
	else {
		gSegmentWidth = segmentWidth;
		context_->setEnableLargePosition(true);
	}
	
	gSegmentWidthInv = 1.0f/(PfxFloat)gSegmentWidth;
}

PfxUInt32 pfxRigidBodyContextGetMultiThreadFlag( const PfxRigidBodyContext &context )
{
	const PfxContext *context_ = ( const PfxContext* )&context;
	return context_->getMultiThreadFlag();
}

void pfxRigidBodyContextSetMultiThreadFlag( PfxRigidBodyContext &context, PfxUInt32 multiThreadFlag )
{
	PfxContext *context_ = ( PfxContext* )&context;
	context_->setMultiThreadFlag( multiThreadFlag );
}

PfxInt32 pfxRigidBodyPipelineBegin(PfxRigidBodyContext &context, PfxRigidBodySharedParam &sharedParam)
{
	PfxInt32 ret = pfxCheckSharedParam(sharedParam);
	if (ret != SCE_PFX_OK) return ret;

	return ((PfxContext*)&context)->pipelineBegin(
		sharedParam.worldMin, sharedParam.worldMax, sharedParam.timeStep,
		sharedParam.states, sharedParam.bodies, sharedParam.collidables,
		sharedParam.numRigidBodies, sharedParam.maxContacts);
}

SCE_PFX_API PfxInt32 pfxRigidBodyPipelineEnd(PfxRigidBodyContext &context)
{
	return ((PfxContext*)&context)->pipelineEnd();
}

PfxInt32 pfxRigidBodyContextAllocatedBytes(PfxRigidBodyContext &context)
{
	return ((PfxContext*)&context)->getAllocatedPoolBytes();
}

PfxInt32 pfxRigidBodyContextGetNumErrors(PfxRigidBodyContext &context)
{
	return ((PfxContext*)&context)->getNumErrors();
}

PfxInt32 pfxRigidBodyContextGetError(PfxRigidBodyContext &context, PfxInt32 errorId, PfxInt32 &stage, PfxInt32 &dispatchId, PfxInt32 &errorCode)
{
	PfxContext::eStage stage_;
	PfxInt32 ret = ((PfxContext*)&context)->getError(errorId, stage_, dispatchId, errorCode);
	if(ret == SCE_PFX_OK) {
		switch(stage_) {
			default:
			stage = 0;
			break;

			case PfxContext::kStageUpdateProxyContainer:
			stage = SCE_PFX_STAGE_UPDATE_PROXY_CONTAINER;
			break;

			case PfxContext::kStageFindOverlapPairs:
			stage = SCE_PFX_STAGE_FIND_OVERLAP_PAIRS;
			break;

			case PfxContext::kStageRefinePairs:
			stage = SCE_PFX_STAGE_REFINE_PAIRS;
			break;

			case PfxContext::kStageDetectCollision:
			stage = SCE_PFX_STAGE_DETECT_COLLISION;
			break;

			case PfxContext::kStageSolveConstraints:
			stage = SCE_PFX_STAGE_SOLVE_CONSTRAINTS;
			break;

			case PfxContext::kStageUserCustomFunction:
			stage = SCE_PFX_STAGE_USER_CUSTOM_FUNCTION;
			break;
		}
	}

	return ret;
}

} //namespace pfxv4
} //namespace sce
