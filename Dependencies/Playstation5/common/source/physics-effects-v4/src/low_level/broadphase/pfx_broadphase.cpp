/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/low_level/broadphase/pfx_broadphase.h"
#include "../rigidbody/pfx_context.h"
#include "../collision/pfx_contact_complex.h"

namespace sce {
namespace pfxv4 {

PfxInt32 pfxCheckParamOfUpdateBroadphaseProxies(const PfxUpdateBroadphaseProxiesParam &param)
{
	if(!param.proxyContainer) return SCE_PFX_ERR_INVALID_VALUE;
	return SCE_PFX_OK;
}

PfxInt32 pfxCheckParamOfFindOverlapPairs(const PfxFindOverlapPairsParam &param)
{
	if(!param.proxyContainerA || !param.proxyContainerB) return SCE_PFX_ERR_INVALID_VALUE;
	if(!param.pairs || !param.numPairsPtr) return SCE_PFX_ERR_INVALID_VALUE;
	return SCE_PFX_OK;
}

PfxInt32 pfxCheckParamOfRefinePairs(const PfxRefinePairsParam &param)
{
	if(!param.contactContainer) return SCE_PFX_ERR_INVALID_VALUE;
	return SCE_PFX_OK;
}

PfxInt32 pfxUpdateBroadphaseProxies(PfxRigidBodyContext &context, PfxRigidBodySharedParam &sharedParam, PfxUpdateBroadphaseProxiesParam &param)
{
	PfxContext *context_ = (PfxContext*)&context;

	PfxInt32 ret = pfxCheckSharedParam(sharedParam);
	if(ret != SCE_PFX_OK) return ret;

	ret = pfxCheckParamOfUpdateBroadphaseProxies(param);
	if(ret != SCE_PFX_OK) return ret;

	if (((PfxProgressiveBvh*)param.proxyContainer)->empty()) {
		return SCE_PFX_OK;
	}

	ret = context_->updateProxyContainer(
		*(PfxProgressiveBvh*)param.proxyContainer,
		sharedParam.worldMin, 
		sharedParam.worldMax, 
		sharedParam.timeStep, 
		sharedParam.states, 
		sharedParam.collidables, 
		sharedParam.numRigidBodies,
		param.fixOutOfWorldBody,
		param.resetShiftFlag,
		param.outOfWorldCallback,
		param.userDataForOutOfWorldCallback);
	
	return ret;
}

PfxInt32 pfxDispatchUpdateBroadphaseProxies(PfxRigidBodyContext &context, PfxUpdateBroadphaseProxiesParam &param)
{
	PfxContext *context_ = (PfxContext*)&context;
	
	PfxInt32 ret = pfxCheckParamOfUpdateBroadphaseProxies(param);
	if(ret != SCE_PFX_OK) return ret;

	if (((PfxProgressiveBvh*)&param.proxyContainer)->empty()) {
		return SCE_PFX_OK;
	}

	ret = context_->dispatchUpdateProxyContainer(
		*(PfxProgressiveBvh*)param.proxyContainer,
		param.fixOutOfWorldBody,
		param.resetShiftFlag,
		param.outOfWorldCallback,
		param.userDataForOutOfWorldCallback);
	
	return ret;
}

PfxInt32 pfxFindOverlapPairs(PfxRigidBodyContext &context, PfxRigidBodySharedParam &sharedParam, PfxFindOverlapPairsParam &param)
{
	PfxContext *context_ = (PfxContext*)&context;
	
	PfxInt32 ret = pfxCheckSharedParam(sharedParam);
	if(ret != SCE_PFX_OK) return ret;

	ret = pfxCheckParamOfFindOverlapPairs(param);
	if(ret != SCE_PFX_OK) return ret;
	
	ret = context_->findOverlapPairs(
		*(PfxProgressiveBvh*)param.proxyContainerA,
		*(PfxProgressiveBvh*)param.proxyContainerB,
		param.pairs,
		param.numPairsPtr,
		sharedParam.maxContacts);

	return ret;
}

PfxInt32 pfxDispatchFindOverlapPairs(PfxRigidBodyContext &context, PfxFindOverlapPairsParam &param)
{
	PfxContext *context_ = (PfxContext*)&context;
	
	PfxInt32 ret = pfxCheckParamOfFindOverlapPairs(param);
	if(ret != SCE_PFX_OK) return ret;
	
	ret = context_->dispatchFindOverlapPairs(
		*(PfxProgressiveBvh*)param.proxyContainerA,
		*(PfxProgressiveBvh*)param.proxyContainerB,
		param.pairs,
		param.numPairsPtr);

	return ret;
}

PfxInt32 pfxRefinePairs(PfxRigidBodyContext &context, PfxRigidBodySharedParam &sharedParam, PfxRefinePairsParam &param)
{
	PfxContext *context_ = (PfxContext*)&context;

	PfxInt32 ret = pfxCheckSharedParam(sharedParam);
	if(ret != SCE_PFX_OK) return ret;

	ret = pfxCheckParamOfRefinePairs(param);
	if(ret != SCE_PFX_OK) return ret;

	ret = context_->refinePairs(
		*(PfxContactComplex*)param.contactContainer,
		param.pairs, 
		param.numPairsPtr, 
		sharedParam.states,
		sharedParam.maxContacts,
		param.dontWakeUp);

	return ret;
}

PfxInt32 pfxDispatchRefinePairs(PfxRigidBodyContext &context, PfxRefinePairsParam &param)
{
	PfxContext *context_ = (PfxContext*)&context;

	PfxInt32 ret = pfxCheckParamOfRefinePairs(param);
	if(ret != SCE_PFX_OK) return ret;

	ret = context_->dispatchRefinePairs(
		*(PfxContactComplex*)param.contactContainer,
		param.pairs, 
		param.numPairsPtr, 
		param.dontWakeUp);

	return ret;
}

} //namespace pfxv4
} //namespace sce
