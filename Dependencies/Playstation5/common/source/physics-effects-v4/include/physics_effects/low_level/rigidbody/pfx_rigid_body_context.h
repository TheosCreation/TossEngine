/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_RIGID_BODY_CONTEXT_H_
#define _SCE_PFX_RIGID_BODY_CONTEXT_H_

#include "../../base_level/base/pfx_common.h"
#include "../../base_level/base/pfx_large_position.h"
#include "../../base_level/rigidbody/pfx_rigid_state.h"
#include "../../base_level/rigidbody/pfx_rigid_body.h"
#include "../../base_level/collision/pfx_collidable.h"

#include <job_common.h>

#if defined(__ORBIS__) || defined(__PROSPERO__)
	#include <kernel.h>
#endif

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Simulation Stages

#define SCE_PFX_STAGE_UPDATE_PROXY_CONTAINER	0x01
#define SCE_PFX_STAGE_FIND_OVERLAP_PAIRS		0x02
#define SCE_PFX_STAGE_REFINE_PAIRS				0x04
#define SCE_PFX_STAGE_DETECT_COLLISION			0x08
#define SCE_PFX_STAGE_SOLVE_CONSTRAINTS			0x10
#define SCE_PFX_STAGE_USER_CUSTOM_FUNCTION		0x20

///////////////////////////////////////////////////////////////////////////////
// Multithread flag

#define SCE_PFX_ENABLE_MULTITHREAD_BROADPHASE	(\
	SCE_PFX_STAGE_UPDATE_PROXY_CONTAINER|\
	SCE_PFX_STAGE_FIND_OVERLAP_PAIRS|\
	SCE_PFX_STAGE_REFINE_PAIRS)

#define SCE_PFX_ENABLE_MULTITHREAD_ALL			(\
	SCE_PFX_ENABLE_MULTITHREAD_BROADPHASE|\
	SCE_PFX_STAGE_DETECT_COLLISION|\
	SCE_PFX_STAGE_SOLVE_CONSTRAINTS|\
	SCE_PFX_STAGE_USER_CUSTOM_FUNCTION)

///////////////////////////////////////////////////////////////////////////////
// Rigid Body Context

/// @brief Rigid body context
/// @details It maintains information about overall rigid body simulation.
struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxRigidBodyContext{
	PfxUInt8 reserved[3904];
};

/// @brief Input parameter for creating the rigid body context
/// @details Segment position always becomes 0, if 0 is set to segmentWidth.(Don't use large position)
/// At least 1 is set to numWorkerThreads
struct SCE_PFX_API PfxRigidBodyContextParam {
	PfxUInt32 segmentWidth = 0;		///< @brief Segment width
	PfxUInt32 numWorkerThreads = 1; ///< @brief Number of worker threads
#if defined(__ORBIS__) || defined(__PROSPERO__)
		PfxInt32 affinityMask = SCE_KERNEL_CPUMASK_USER_ALL;
		PfxInt32 threadPriority = SCE_KERNEL_PRIO_FIFO_DEFAULT + 1;
#else
		PfxInt32 affinityMask = 0x000000FF;///< @brief CPU Affinity mask
		PfxInt32 threadPriority = 0;///< @brief Thread priority
#endif
	PfxUInt32 multiThreadFlag = SCE_PFX_ENABLE_MULTITHREAD_ALL;
	PfxBool enablePersistentThreads = true; ///< @brief Worker threads aren't suspended if it is true
	sce::Job::SequenceFactoryInterface *userJobManagerSequenceFactoryInterface = nullptr;	///< @brief Parameter of the user job manager
};

/// @brief Initialize the rigid body context.
/// @details At least one worker thread is created, or an external job sequence can be specified via PfxRigidBodyContextParam.
/// Work buffer is used a temporary buffer in the whole simulation pipeline.
/// If APIs return SCE_PFX_ERR_OUT_OF_BUFFER, more buffer would be needed to run a pipeline.
/// @param[in,out] context Rigid body context
/// @param param Input parameter
/// @param workBuff Work buffer
/// @param workBytes Bytes of the work buffer
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxRigidBodyContextInit(PfxRigidBodyContext &context,const PfxRigidBodyContextParam &param,void *workBuff,PfxInt32 workBytes);

/// @brief Finish the rigid body context
/// @details Stops worker threads and terminates the rigid body context.
/// @param[in,out] context Rigid body context
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxRigidBodyContextTerm(PfxRigidBodyContext &context);

/// @brief Reset distance between segments
/// @details Change a distance of the segment width.
/// If 0 is specified to segmentWidth, default value (=1000) is set.
/// @param[in,out] context Rigid body context
/// @param[in] segmentWidth Distance between segments
SCE_PFX_API void pfxRigidBodyContextResetSegmentWidth(PfxRigidBodyContext &context,PfxInt32 segmentWidth);

/// @brief Get the multi threading flag
/// @param[in] context Rigid body context
/// @return Return the multi threading flag
SCE_PFX_API PfxUInt32 pfxRigidBodyContextGetMultiThreadFlag( const PfxRigidBodyContext &context );

/// @brief Change the multi threading flag
/// @details Define simulation stages which should be processed in parallel
/// @param[in,out] context Rigid body context
/// @param[in] multiThreadFlag Stages processed in parallel
SCE_PFX_API void pfxRigidBodyContextSetMultiThreadFlag( PfxRigidBodyContext &context, PfxUInt32 multiThreadFlag );

/// @brief Input parameter storing shared information
struct SCE_PFX_API PfxRigidBodySharedParam {
	PfxFloat timeStep = 0.016f;						///< @brief Time step
	PfxUInt32 numRigidBodies = 0;					///< @brief The number of rigid bodies
	PfxUInt32 maxContacts = 0;						///< @brief The maximum number of contacts
	PfxLargePosition worldMin = PfxLargePosition();	///< @brief Minimum position of the world
	PfxLargePosition worldMax = PfxLargePosition();	///< @brief Maximum position of the world
	PfxRigidState *states = nullptr;				///< @brief Array of the rigid body states
	PfxRigidBody *bodies = nullptr;					///< @brief Array of the rigid bodies
	PfxCollidable *collidables = nullptr;			///< @brief Array of the collidables
};

/// @brief Specify the beginning of a pipeline
/// @details This function is used to specify the beginning of a pipeline in a non-blocking mode.
/// pfxDispatchXXX functions , which run with a non-blocking manner, can be called in between 
/// pfxRigidBodyPipelineBegin() and pfxRigidBodyPipelineEnd().
/// @param[in,out] context Rigid body context
/// @param[in,out] sharedParam Shared parameter
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxRigidBodyPipelineBegin(PfxRigidBodyContext &context, PfxRigidBodySharedParam &sharedParam);

/// @brief Specify the end of a pipeline
/// @details This function is used to specify the end of a pipeline in a non-blocking mode.
/// @param[in,out] context Rigid body context
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxRigidBodyPipelineEnd(PfxRigidBodyContext &context);

/// @brief Get the allocated bytes
/// @details This function is used to get the allocated bytes of a work buffer consumed by dispatch functions.
/// @param context Rigid body context
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxRigidBodyContextAllocatedBytes(PfxRigidBodyContext &context);

/// @brief Get the number of errors
/// @details This function returns the number of errors which occurs during running the pipeline.
/// @param context Rigid body context
/// @return Return the number of errors
SCE_PFX_API PfxInt32 pfxRigidBodyContextGetNumErrors(PfxRigidBodyContext &context);

/// @brief Get the information of an error
/// @details This function returns the information of an error which occurs during running the pipeline.
/// @param context Rigid body context
/// @param id Error index
/// @param stage Simulation stage in a pipeline where an error occurs
/// @param dispatchId Dispatch index
/// @param errorCode Error code
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxRigidBodyContextGetError(PfxRigidBodyContext &context, PfxInt32 id, PfxInt32 &stage, PfxInt32 &dispatchId, PfxInt32 &errorCode);

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_RIGID_BODY_CONTEXT_H_ */
