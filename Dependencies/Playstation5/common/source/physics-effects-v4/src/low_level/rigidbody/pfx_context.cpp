/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2022 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_context.h"
#include "../../../include/physics_effects/base_level/rigidbody/pfx_rigid_body.h"

namespace sce {
namespace pfxv4 {

extern PfxInt32 gSegmentWidth;
extern PfxFloat gSegmentWidthInv;

PfxInt32 PfxContext::initialize(void *workBuff, PfxUInt32 workBytes, int workerThreads, PfxUInt32 multiThreadFlag, sce::Job::SequenceFactoryInterface *userJobManagerSequenceFactoryInterface)
{
	SCE_PFX_ASSERT(userJobManagerSequenceFactoryInterface != nullptr);

	PfxInt32 ret = SCE_PFX_OK;

	ret = m_singleJobSystem.initialize( workerThreads, userJobManagerSequenceFactoryInterface );
	if( ret != SCE_PFX_OK ) return ret;

	ret = m_multiJobSystem.initialize(workerThreads, userJobManagerSequenceFactoryInterface);
	if(ret != SCE_PFX_OK) return ret;

	m_pool.initialize((PfxUInt8*)workBuff, workBytes);

	setMultiThreadFlag( multiThreadFlag );

	m_enableLargePosition = false;

	return ret;
}

PfxInt32 PfxContext::initialize(void *workBuff, PfxUInt32 workBytes, int workerThreads, PfxUInt32 multiThreadFlag, int affinityMask, int threadPriority)
{
	PfxInt32 ret = SCE_PFX_OK;

	PfxUInt8 *p = (PfxUInt8*)workBuff;

	ret = m_singleJobSystem.initialize( &p, workBytes, workerThreads, affinityMask, threadPriority );
	if( ret != SCE_PFX_OK ) return ret;

	ret = m_multiJobSystem.initialize(&p, workBytes, workerThreads, affinityMask, threadPriority);
	if(ret != SCE_PFX_OK) return ret;

	m_pool.initialize((PfxUInt8*)p, workBytes - ((uintptr_t)p - (uintptr_t)workBuff));

	setMultiThreadFlag( multiThreadFlag );

	m_enableLargePosition = false;

	return ret;
}

void PfxContext::finalize()
{
	m_singleJobSystem.finalize();
	m_multiJobSystem.finalize();
}

void PfxContext::setMultiThreadFlag( PfxUInt32 multiThreadFlag )
{
	SCE_PFX_ASSERT_MSG( m_dispatchedStages == 0, "PFX : The multithread flag can not be changed during simulation" );
	m_broadphaseStage.initialize( ( multiThreadFlag & SCE_PFX_ENABLE_MULTITHREAD_BROADPHASE ) ? ( PfxJobSystem* )&m_multiJobSystem : ( PfxJobSystem* )&m_singleJobSystem, this );
	m_detectCollisionStage.initialize( ( multiThreadFlag & SCE_PFX_STAGE_DETECT_COLLISION ) ? ( PfxJobSystem* )&m_multiJobSystem : ( PfxJobSystem* )&m_singleJobSystem, this );
	m_islandCreationStage.initialize( ( multiThreadFlag & SCE_PFX_STAGE_SOLVE_CONSTRAINTS ) ? ( PfxJobSystem* )&m_multiJobSystem : ( PfxJobSystem* )&m_singleJobSystem, this );
	m_gatherSolverPairsStage.initialize( ( multiThreadFlag & SCE_PFX_STAGE_SOLVE_CONSTRAINTS ) ? ( PfxJobSystem* )&m_multiJobSystem : ( PfxJobSystem* )&m_singleJobSystem, this );
	m_solveConstraintsStage.initialize( ( multiThreadFlag & SCE_PFX_STAGE_SOLVE_CONSTRAINTS ) ? ( PfxJobSystem* )&m_multiJobSystem : ( PfxJobSystem* )&m_singleJobSystem, this );
	m_userCustomStage.initialize( ( multiThreadFlag & SCE_PFX_STAGE_USER_CUSTOM_FUNCTION ) ? ( PfxJobSystem* )&m_multiJobSystem : ( PfxJobSystem* )&m_singleJobSystem, this );
	m_multiThreadFlag = multiThreadFlag;
}

void PfxContext::appendError(eStage stage, PfxInt32 dispatchId, PfxInt32 errorCode)
{
	Error err;
	err.stage = stage;
	err.dispatchId = dispatchId;
	err.errorCode = errorCode;

	if (m_lastDispatchId != dispatchId) {
		PfxUInt32 errId = m_numErrors.fetch_add(1, std::memory_order_acq_rel);
		if (errId < m_maxErrors) {
			m_lastDispatchId = dispatchId;
			m_errors[errId] = err;
		}
	}
}

PfxInt32 PfxContext::getError(int id, eStage &stage, PfxInt32 &dispatchId, PfxInt32 &errorCode)
{
	if(id < m_numErrors) {
		stage = m_errors[id].stage;
		dispatchId = m_errors[id].dispatchId;
		errorCode = m_errors[id].errorCode;
	}
	else {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	return SCE_PFX_OK;
}

PfxInt32 PfxContext::pipelineBegin(
	const PfxLargePosition &worldMin, const PfxLargePosition &worldMax, PfxFloat timeStep,
	PfxRigidState *states, PfxRigidBody *bodies, PfxCollidable *collidables, PfxUInt32 numRigidBodies, PfxUInt32 maxContacts)
{
	if(m_dispatchedStages != 0) return SCE_PFX_ERR_INVALID_CALL;

	m_numErrors = 0;
	m_dispatchCount = 0;
	m_lastDispatchId = -1;
	m_dispatchedStages = kStagePipelineBegin;

	if(m_enablePersistentThread) m_multiJobSystem.enablePersistentThreads();

	PfxInt32 *roots = allocate<PfxInt32>(numRigidBodies);
	if(!roots) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxInt32 ret = SCE_PFX_OK;
	ret = m_broadphaseStage.preparePipelineForUpdateBvh(worldMin, worldMax, timeStep, states, collidables, numRigidBodies);
	if(ret != SCE_PFX_OK) return ret;
	ret = m_broadphaseStage.preparePipelineForFindPairs(maxContacts);
	if(ret != SCE_PFX_OK) return ret;
	ret = m_broadphaseStage.preparePipelineForRefinePairs(states, maxContacts);
	if(ret != SCE_PFX_OK) return ret;
	ret = m_detectCollisionStage.preparePipeline(timeStep, states, collidables);
	if(ret != SCE_PFX_OK) return ret;
	ret = m_islandCreationStage.preparePipeline(roots, states, numRigidBodies);
	if(ret != SCE_PFX_OK) return ret;
	ret = m_gatherSolverPairsStage.preparePipeline(maxContacts);
	if(ret != SCE_PFX_OK) return ret;
	ret = m_solveConstraintsStage.preparePipeline(timeStep, roots, states, bodies, numRigidBodies);
	if(ret != SCE_PFX_OK) return ret;

	return SCE_PFX_OK;
}

PfxInt32 PfxContext::pipelineEnd()
{
	if(m_dispatchedStages == 0) return SCE_PFX_ERR_INVALID_CALL;

	if(m_dispatchedStages & kStageSolveConstraints) {
		m_multiJobSystem.wait();
		m_multiJobSystem.wait();
		m_multiJobSystem.wait();
	}
	else {
		m_multiJobSystem.wait();
	}

	if (m_enablePersistentThread) m_multiJobSystem.disablePersistentThreads();

	clearPool();

	m_dispatchedStages = 0;

	return getNumErrors();
}

PfxInt32 PfxContext::updateProxyContainer(PfxProgressiveBvh &bvh, const PfxLargePosition &worldMin, const PfxLargePosition &worldMax, PfxFloat timeStep,
	PfxRigidState *states, PfxCollidable *collidables, PfxUInt32 numRigidBodies,
	PfxBool fixOutOfWorldBody, PfxBool resetShiftFlag, void(*outOfWorldCallback)(PfxUInt32, void *), void *userDataForOutOfWorldCallback)
{
	if(m_dispatchedStages != 0) return SCE_PFX_ERR_INVALID_CALL;

	m_numErrors = 0;
	m_dispatchCount = 0;
	m_lastDispatchId = -1;

	PfxJobSystem* jobSystem = m_multiThreadFlag & SCE_PFX_STAGE_UPDATE_PROXY_CONTAINER ? ( PfxJobSystem* )&m_multiJobSystem : ( PfxJobSystem* )&m_singleJobSystem;
	m_broadphaseStage.setJobSystem( jobSystem );

	auto preExit = [&]()
	{
		jobSystem->wait();
		if (m_enablePersistentThread) jobSystem->disablePersistentThreads();
		clearPool();
	};

	if (m_enablePersistentThread) jobSystem->enablePersistentThreads();

	PfxInt32 ret = SCE_PFX_OK;
	ret = m_broadphaseStage.preparePipelineForUpdateBvh(worldMin, worldMax, timeStep, states, collidables, numRigidBodies);
	if(ret != SCE_PFX_OK) {preExit();return ret;}
	ret = m_broadphaseStage.updateBvh(bvh, fixOutOfWorldBody, resetShiftFlag, outOfWorldCallback, userDataForOutOfWorldCallback);
	if(ret != SCE_PFX_OK) {preExit();return ret;}

	preExit();

	return ret;
}

PfxInt32 PfxContext::dispatchUpdateProxyContainer(PfxProgressiveBvh &bvh,
	PfxBool fixOutOfWorldBody, PfxBool resetShiftFlag, void(*outOfWorldCallback)(PfxUInt32, void *), void *userDataForOutOfWorldCallback)
{
	if(m_dispatchedStages & kStageSolveConstraints) return SCE_PFX_ERR_INVALID_CALL;
	m_dispatchedStages |= kStageUpdateProxyContainer;
	return m_broadphaseStage.updateBvh(bvh, fixOutOfWorldBody, resetShiftFlag, outOfWorldCallback, userDataForOutOfWorldCallback);
}

PfxInt32 PfxContext::findOverlapPairs(const PfxProgressiveBvh &bvhA, const PfxProgressiveBvh &bvhB, PfxBroadphasePair *outPairs, PfxInt32 *numOutPairs, PfxUInt32 maxContacts)
{
	if(m_dispatchedStages != 0) return SCE_PFX_ERR_INVALID_CALL;

	m_numErrors = 0;
	m_dispatchCount = 0;
	m_lastDispatchId = -1;

	PfxJobSystem* jobSystem = m_multiThreadFlag & SCE_PFX_STAGE_FIND_OVERLAP_PAIRS ? ( PfxJobSystem* )&m_multiJobSystem : ( PfxJobSystem* )&m_singleJobSystem;
	m_broadphaseStage.setJobSystem( jobSystem );

	auto preExit = [&]()
	{
		jobSystem->wait();
		if (m_enablePersistentThread) jobSystem->disablePersistentThreads();
		clearPool();
	};

	if (m_enablePersistentThread) jobSystem->enablePersistentThreads();

	PfxInt32 ret = SCE_PFX_OK;
	ret = m_broadphaseStage.preparePipelineForFindPairs(maxContacts);
	if(ret != SCE_PFX_OK) {preExit();return ret;}
	ret = m_broadphaseStage.findOverlapPairs(bvhA, bvhB, outPairs, numOutPairs);
	if(ret != SCE_PFX_OK) {preExit();return ret;}
	
	preExit();
	
	return ret;
}

PfxInt32 PfxContext::dispatchFindOverlapPairs(const PfxProgressiveBvh &bvhA, const PfxProgressiveBvh &bvhB, PfxBroadphasePair *outPairs, PfxInt32 *numOutPairs)
{
	if(m_dispatchedStages & kStageSolveConstraints) return SCE_PFX_ERR_INVALID_CALL;
	m_dispatchedStages |= kStageFindOverlapPairs;
	return m_broadphaseStage.findOverlapPairs(bvhA, bvhB, outPairs, numOutPairs);
}

PfxInt32 PfxContext::refinePairs(PfxContactComplex &contactComplex, const PfxBroadphasePair *outPairs, const PfxInt32 *numOutPairs, PfxRigidState *states, PfxUInt32 maxContacts, PfxBool dontWakeUp)
{
	if(m_dispatchedStages != 0) return SCE_PFX_ERR_INVALID_CALL;

	m_numErrors = 0;
	m_dispatchCount = 0;
	m_lastDispatchId = -1;

	PfxJobSystem* jobSystem = m_multiThreadFlag & SCE_PFX_STAGE_REFINE_PAIRS ? ( PfxJobSystem* )&m_multiJobSystem : ( PfxJobSystem* )&m_singleJobSystem;
	m_broadphaseStage.setJobSystem( jobSystem );

	auto preExit = [&]()
	{
		jobSystem->wait();
		if (m_enablePersistentThread) jobSystem->disablePersistentThreads();
		clearPool();
	};

	if (m_enablePersistentThread) jobSystem->enablePersistentThreads();

	PfxInt32 ret = SCE_PFX_OK;
	ret = m_broadphaseStage.preparePipelineForRefinePairs(states, maxContacts);
	if(ret != SCE_PFX_OK) {preExit();return ret;}
	ret = m_broadphaseStage.refinePairs(contactComplex, outPairs, numOutPairs, dontWakeUp);
	if(ret != SCE_PFX_OK) {preExit();return ret;}
	
	preExit();
	
	return ret;
}

PfxInt32 PfxContext::dispatchRefinePairs(PfxContactComplex &contactComplex, const PfxBroadphasePair *outPairs, const PfxInt32 *numOutPairs, PfxBool dontWakeUp)
{
	if(m_dispatchedStages & kStageSolveConstraints) return SCE_PFX_ERR_INVALID_CALL;
	m_dispatchedStages |= kStageRefinePairs;
	return m_broadphaseStage.refinePairs(contactComplex, outPairs, numOutPairs, dontWakeUp);
}

PfxInt32 PfxContext::detectCollision(PfxContactComplex &contactComplex, PfxFloat timeStep, PfxRigidState *states, PfxCollidable *collidables) 
{
	if(m_dispatchedStages != 0) return SCE_PFX_ERR_INVALID_CALL;

	m_numErrors = 0;
	m_dispatchCount = 0;
	m_lastDispatchId = -1;

	PfxJobSystem* jobSystem = m_multiThreadFlag & SCE_PFX_STAGE_DETECT_COLLISION ? ( PfxJobSystem* )&m_multiJobSystem : ( PfxJobSystem* )&m_singleJobSystem;
	m_detectCollisionStage.setJobSystem( jobSystem );

	auto preExit = [&]()
	{
		jobSystem->wait();
		if (m_enablePersistentThread) jobSystem->disablePersistentThreads();
		clearPool();
	};

	if (m_enablePersistentThread) jobSystem->enablePersistentThreads();

	PfxInt32 ret = SCE_PFX_OK;
	ret = m_detectCollisionStage.preparePipeline(timeStep, states, collidables);
	if(ret != SCE_PFX_OK) {preExit();return ret;}
	ret = m_detectCollisionStage.dispatchDetectCollision(contactComplex);
	if(ret != SCE_PFX_OK) {preExit();return ret;}
	
	preExit();
	
	return ret;
}

PfxInt32 PfxContext::dispatchDetectCollision(PfxContactComplex &contactComplex)
{
	if(m_dispatchedStages & kStageSolveConstraints) return SCE_PFX_ERR_INVALID_CALL;
	m_dispatchedStages |= kStageDetectCollision;
	return m_detectCollisionStage.dispatchDetectCollision(contactComplex);
}

PfxInt32 PfxContext::solveConstraints(PfxContactComplex &contactComplex, PfxSolveConstraintsStage::SolverParam &param,
	PfxFloat timeStep, PfxRigidState *states, PfxRigidBody *bodies, PfxUInt32 numRigidBodies, PfxUInt32 maxContacts)
{
	if(m_dispatchedStages != 0) return SCE_PFX_ERR_INVALID_CALL;

	m_numErrors = 0;
	m_dispatchCount = 0;
	m_lastDispatchId = -1;

	PfxInt32 ret = SCE_PFX_OK;

	PfxInt32 *roots = allocate<PfxInt32>(numRigidBodies);
	if(!roots) {clearPool();return SCE_PFX_ERR_OUT_OF_BUFFER;}
	
	PfxJobSystem* jobSystem = m_multiThreadFlag & SCE_PFX_STAGE_SOLVE_CONSTRAINTS ? ( PfxJobSystem* )&m_multiJobSystem : ( PfxJobSystem* )&m_singleJobSystem;
	m_islandCreationStage.setJobSystem( jobSystem );
	m_gatherSolverPairsStage.setJobSystem( jobSystem );
	m_solveConstraintsStage.setJobSystem( jobSystem );

	auto preExit = [&]()
	{
		jobSystem->wait();
		if (m_enablePersistentThread) jobSystem->disablePersistentThreads();
		clearPool();
	};

	if (m_enablePersistentThread) jobSystem->enablePersistentThreads();

	ret = m_islandCreationStage.preparePipeline(roots, states, numRigidBodies);
	if(ret != SCE_PFX_OK) {preExit();return ret;}
	ret = m_gatherSolverPairsStage.preparePipeline(maxContacts);
	if(ret != SCE_PFX_OK) {preExit();return ret;}
	ret = m_solveConstraintsStage.preparePipeline(timeStep, roots, states, bodies, numRigidBodies);
	if(ret != SCE_PFX_OK) {preExit();return ret;}
	ret = m_gatherSolverPairsStage.dispatchGatherSolverPairs(contactComplex, param.contactPairs, param.numContactPairsPtr);
	if (ret != SCE_PFX_OK) { preExit(); return ret; }
	ret = m_islandCreationStage.dispatchCreateIslands(param.contactPairs, param.numContactPairsPtr, param.jointPairs, param.numJointPairsPtr);
	if (ret != SCE_PFX_OK) { preExit(); return ret; }
	jobSystem->wait();
	ret = m_solveConstraintsStage.solve(contactComplex, param, true);
	if(ret != SCE_PFX_OK) {preExit();return ret;}
	
	preExit();

	return ret;
}

PfxInt32 PfxContext::dispatchSolveConstraints(PfxContactComplex &contactComplex, PfxSolveConstraintsStage::SolverParam &param)
{
	if(m_dispatchedStages & kStageSolveConstraints) return SCE_PFX_ERR_INVALID_CALL;
	m_dispatchedStages |= kStageSolveConstraints;

	PfxInt32 ret = SCE_PFX_OK;
	ret = m_gatherSolverPairsStage.dispatchGatherSolverPairs(contactComplex, param.contactPairs, param.numContactPairsPtr);
	if(ret != SCE_PFX_OK) {clearPool();return ret;}
	ret = m_islandCreationStage.dispatchCreateIslands(param.contactPairs, param.numContactPairsPtr, param.jointPairs, param.numJointPairsPtr);
	if(ret != SCE_PFX_OK) {clearPool();return ret;}
	ret = m_solveConstraintsStage.solve(contactComplex, param, false);
	if(ret != SCE_PFX_OK) {clearPool();return ret;}
	return ret;
}

PfxInt32 PfxContext::execUserCustomFunction(PfxUInt32 numJobs, UserCustomFunc userFunc, void *userData)
{
	if(m_dispatchedStages != 0) return SCE_PFX_ERR_INVALID_CALL;

	m_numErrors = 0;
	m_dispatchCount = 0;
	m_lastDispatchId = -1;
	
	PfxJobSystem* jobSystem = m_multiThreadFlag & SCE_PFX_STAGE_USER_CUSTOM_FUNCTION ? ( PfxJobSystem* )&m_multiJobSystem : ( PfxJobSystem* )&m_singleJobSystem;
	m_userCustomStage.setJobSystem( jobSystem );

	PfxInt32 ret = m_userCustomStage.execUserCustomAsync(numJobs, userFunc, userData);
	
	clearPool();
	jobSystem->wait();
	
	return ret;
}

PfxInt32 PfxContext::dispatchExecUserCustomFunction(PfxUInt32 numJobs, UserCustomFunc userFunc, void *userData)
{
	if(m_dispatchedStages & kStageSolveConstraints) return SCE_PFX_ERR_INVALID_CALL;
	m_dispatchedStages |= kStageUserCustomFunction;
	return m_userCustomStage.execUserCustomAsync(numJobs, userFunc, userData);
}

PfxInt32 pfxCheckSharedParam(const PfxRigidBodySharedParam &sharedParam)
{
	if(!sharedParam.states || !sharedParam.bodies || !sharedParam.collidables) return SCE_PFX_ERR_INVALID_VALUE;
	if(!SCE_PFX_PTR_IS_ALIGNED16(sharedParam.states) || !SCE_PFX_PTR_IS_ALIGNED16(sharedParam.bodies) || !SCE_PFX_PTR_IS_ALIGNED16(sharedParam.collidables)) return SCE_PFX_ERR_INVALID_ALIGN;
	if(sharedParam.timeStep <= 0.0f) return SCE_PFX_ERR_INVALID_VALUE;
	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
