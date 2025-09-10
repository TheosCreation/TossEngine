/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_USER_CUSTOM_FUNCTION_H_
#define _SCE_PFX_USER_CUSTOM_FUNCTION_H_

#include "../rigidbody/pfx_rigid_body_context.h"

namespace sce {
namespace pfxv4 {

/// @brief Callback for a user custom function
/// @details It should return 0, if there is no errors.
typedef PfxInt32(*UserCustomFunc)(PfxInt32 jobId, void *userData);

/// @brief An user specified function can be called in a pipeline.
/// @details An user specified function is executed in a pipeline in dispatched order.
/// Specified number of jobs are dispatched simultaneously and each job calls an user function with its index.
/// @param[in,out] context Rigid body context
/// @param numJobs Number of jobs
/// @param func User function called in a pipeline
/// @param userData User data transferred to the callback
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxUserCustomFunction(PfxRigidBodyContext &context, PfxUInt32 numJobs, UserCustomFunc func, void *userData);

/// @brief Non blocking version of pfxUserCustomFunction()
SCE_PFX_API PfxInt32 pfxDispatchUserCustomFunction(PfxRigidBodyContext &context, PfxUInt32 numJobs, UserCustomFunc func, void *userData);

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_USER_CUSTOM_FUNCTION_H_ */
