/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _PFX_CONTEXT_H
#define _PFX_CONTEXT_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../../include/physics_effects/low_level/rigidbody/pfx_rigid_body_context.h"
#include "../../../include/physics_effects/low_level/rigidbody/pfx_user_custom_function.h"
#include "../task/pfx_job_system.h"
#include "../broadphase/pfx_broadphase_stage.h"
#include "../collision/pfx_detect_collision_stage.h"
#include "../collision/pfx_contact_complex.h"
#include "../solver/pfx_gather_solver_pairs_stage.h"
#include "../solver/pfx_island_creation_stage.h"
#include "../solver/pfx_solve_constraints_stage.h"
#include "pfx_user_custom_stage.h"

namespace sce {
namespace pfxv4 {

class PoolMemory
{
private:
	PfxUInt8 *m_heap, *m_cur;
	PfxInt32 m_heapBytes, m_maxAlloc;
	
	PfxUInt8 *getCurAddr()
	{
		return m_cur;
	}
	
public:
	enum {ALIGN4=4,ALIGN8=8,ALIGN16=16,ALIGN128=128};
	
	PoolMemory() : m_heap(NULL),m_cur(NULL),m_heapBytes(0) {}
	
	PoolMemory(PfxUInt8 *buf,PfxInt32 bytes)
	{
		m_heap = m_cur = buf;
		m_heapBytes = bytes;
		m_maxAlloc = 0;
	}
	
	void initialize(PfxUInt8 *buf,PfxInt32 bytes)
	{
		m_heap = m_cur = buf;
		m_heapBytes = bytes;
		m_maxAlloc = 0;
	}
	
	PfxInt32 getAllocated()
	{
		return (PfxInt32)(m_cur - m_heap);
	}
	
	PfxInt32 getRest()
	{
		return m_heapBytes-getAllocated();
	}

	PfxInt32 getMaximumAllocatedBytes()
	{
		return m_maxAlloc;
	}

	void *allocate(size_t bytes,PfxInt32 alignment = ALIGN16)
	{
		bytes = SCE_PFX_MAX(bytes, 4); // minimum alloc size = 4 bytes

		uintptr_t p = (uintptr_t)getCurAddr();
		
		uintptr_t tmp = (uintptr_t)alignment - 1;
		
		p = (p+tmp) & ~tmp;
		bytes = (bytes+tmp) & ~tmp;
		
		if((p + bytes) > (uintptr_t)(m_heap + m_heapBytes)) {
			return nullptr;
		}

		m_cur = (PfxUInt8*)(p + bytes); 

		return (void*)p;
	}

	template< typename T >
	T* allocateArray(PfxUInt32 numElements, PfxUInt32 alignment)
	{
		const size_t memoryAmount = numElements * sizeof(T);
		void* memory = allocate(memoryAmount, alignment);
		return reinterpret_cast<T*>(memory);
	}
	
	void clear()
	{
		m_maxAlloc = m_cur - m_heap;
		m_cur = m_heap;
	}
};

class PfxContext {
private:
	///////////////////////////////////////////////////////////////////////////////
	// Configuration

	PfxBool m_enableLargePosition = true;
	PfxBool m_enablePersistentThread = true;

	///////////////////////////////////////////////////////////////////////////////
	// Pool Memory

	PoolMemory m_pool;

	///////////////////////////////////////////////////////////////////////////////
	// Job manager

	PfxUInt32 m_multiThreadFlag;
	PfxSingleJobSystem m_singleJobSystem;
	PfxMultiJobSystem m_multiJobSystem;

	///////////////////////////////////////////////////////////////////////////////
	// Pipeline Stages

	PfxBroadphaseStage m_broadphaseStage;
	PfxDetectCollisionStage m_detectCollisionStage;
	PfxIslandCreationStage m_islandCreationStage;
	PfxGatherSolverPairsStage m_gatherSolverPairsStage;
	PfxSolveConstraintsStage m_solveConstraintsStage;
	PfxUserCustomStage m_userCustomStage;

public:
	PfxInt32 initialize(void *workBuff, PfxUInt32 workBytes, int workerThreads, PfxUInt32 multiThreadFlag, sce::Job::SequenceFactoryInterface *userJobManagerSequenceFactoryInterface = NULL);
	PfxInt32 initialize(void *workBuff, PfxUInt32 workBytes, int workerThreads, PfxUInt32 multiThreadFlag, int affinityMask, int threadPriority);
	void finalize();

	void setEnableLargePosition(PfxBool b) { m_enableLargePosition = b; }
	PfxBool getEnableLargePosition() { return m_enableLargePosition; }

	void setPersistentThread(PfxBool b) { m_enablePersistentThread = b; }
	PfxBool getPersistentThread() {	return m_enablePersistentThread; }

	void setMultiThreadFlag( PfxUInt32 multiThreadFlag );
	PfxUInt32 getMultiThreadFlag() const { return m_multiThreadFlag; }

	///////////////////////////////////////////////////////////////////////////////
	// Memory Pool
	
	template< typename T >
	T* allocate(PfxUInt32 numElements)
	{
		return m_pool.allocateArray<T>(numElements, 16);
	}

	void* allocateBytes(PfxUInt32 bytes)
	{
		return m_pool.allocate(bytes, 16);
	}

	void clearPool() {m_pool.clear();}

	PfxInt32 getMaximumAllocatedBytes() {return m_pool.getMaximumAllocatedBytes();}

	///////////////////////////////////////////////////////////////////////////////
	// Pipeline

	enum eStage {
		kStageUpdateProxyContainer		 = 0x01, // SCE_PFX_STAGE_UPDATE_PROXY_CONTAINER	
		kStageFindOverlapPairs			 = 0x02, // SCE_PFX_STAGE_FIND_OVERLAP_PAIRS		
		kStageRefinePairs				 = 0x04, // SCE_PFX_STAGE_REFINE_PAIRS				
		kStageDetectCollision			 = 0x08, // SCE_PFX_STAGE_DETECT_COLLISION			
		kStageSolveConstraints			 = 0x10, // SCE_PFX_STAGE_SOLVE_CONSTRAINTS			
		kStageUserCustomFunction		 = 0x20, // SCE_PFX_STAGE_USER_CUSTOM_FUNCTION		
		kStagePipelineBegin				 = 0x80000000
	};

	PfxUInt32 m_dispatchedStages = 0;

private:
	struct Error {
		PfxInt32 errorCode;
		PfxInt32 dispatchId;
		eStage stage;
	};

	const static PfxInt32 m_maxErrors = 32;
	PfxInt32 m_dispatchCount = 0;
	std::atomic<PfxInt32> m_lastDispatchId;
	std::atomic<PfxInt32> m_numErrors;
	Error m_errors[m_maxErrors];

public:

	PfxUInt32 getAllocatedPoolBytes() { return m_pool.getAllocated(); }
	PfxUInt32 getRestPoolBytes() { return m_pool.getRest(); }

	PfxUInt32 getDispatchId() {return m_dispatchCount++;}

	PfxUInt32 getNumErrors() {return SCE_PFX_MIN((PfxInt32)m_numErrors, m_maxErrors);}
	void appendError(eStage stage, PfxInt32 dispatchId, PfxInt32 errorCode);
	PfxInt32 getError(int id, eStage &stage, PfxInt32 &dispatchId, PfxInt32 &errorCode);

	// Non-Blocking APIs

	PfxInt32 pipelineBegin(const PfxLargePosition &worldMin, const PfxLargePosition &worldMax, PfxFloat timeStep,
		PfxRigidState *states, PfxRigidBody *bodies, PfxCollidable *collidables, PfxUInt32 numRigidBodies, PfxUInt32 maxContacts);

	PfxInt32 pipelineEnd();

	PfxInt32 dispatchUpdateProxyContainer(PfxProgressiveBvh &bvh,
		PfxBool fixOutOfWorldBody, PfxBool resetShiftFlag, void(*outOfWorldCallback)(PfxUInt32, void *), void *userDataForOutOfWorldCallback);

	PfxInt32 dispatchFindOverlapPairs(const PfxProgressiveBvh &bvhA, const PfxProgressiveBvh &bvhB, PfxBroadphasePair *outPairs, PfxInt32 *numOutPairs);

	PfxInt32 dispatchRefinePairs(PfxContactComplex &contactComplex, const PfxBroadphasePair *outPairs, const PfxInt32 *numOutPairs, PfxBool dontWakeUp);

	PfxInt32 dispatchDetectCollision(PfxContactComplex &contactComplex);

	PfxInt32 dispatchSolveConstraints(PfxContactComplex &contactComplex, PfxSolveConstraintsStage::SolverParam &param);

	PfxInt32 dispatchExecUserCustomFunction(PfxUInt32 numJobs, UserCustomFunc userFunc, void *userData);

	// Blocking APIs

	PfxInt32 updateProxyContainer(PfxProgressiveBvh &bvh, const PfxLargePosition &worldMin, const PfxLargePosition &worldMax,
		PfxFloat timeStep, PfxRigidState *states, PfxCollidable *collidables, PfxUInt32 numRigidBodies, 
		PfxBool fixOutOfWorldBody, PfxBool resetShiftFlag, void(*outOfWorldCallback)(PfxUInt32, void *), void *userDataForOutOfWorldCallback);

	PfxInt32 findOverlapPairs(const PfxProgressiveBvh &bvhA, const PfxProgressiveBvh &bvhB, PfxBroadphasePair *outPairs, PfxInt32 *numOutPairs, PfxUInt32 maxContacts);

	PfxInt32 refinePairs(PfxContactComplex &contactComplex, const PfxBroadphasePair *outPairs, const PfxInt32 *numOutPairs, PfxRigidState *states, PfxUInt32 maxContacts, PfxBool dontWakeUp);

	PfxInt32 detectCollision(PfxContactComplex &contactComplex, PfxFloat timeStep, PfxRigidState *states, PfxCollidable *collidables);

	PfxInt32 solveConstraints(PfxContactComplex &contactComplex, PfxSolveConstraintsStage::SolverParam &param,
		PfxFloat timeStep, PfxRigidState *states, PfxRigidBody *bodies, PfxUInt32 numRigidBodies, PfxUInt32 maxContacts);

	PfxInt32 execUserCustomFunction(PfxUInt32 numJobs, UserCustomFunc userFunc, void *userData);
};

PfxInt32 pfxCheckSharedParam(const PfxRigidBodySharedParam &sharedParam);

} //namespace pfxv4
} //namespace sce

#endif // _PFX_CONTEXT_H

