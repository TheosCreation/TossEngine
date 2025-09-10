/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_COMPOUND_CONTAINER_H_
#define _SCE_PFX_COMPOUND_CONTAINER_H_

#include "../../base_level/base/pfx_common.h"
#include "../../base_level/rigidbody/pfx_rigid_body.h"
#include "../../base_level/rigidbody/pfx_rigid_state.h"

namespace sce {
namespace pfxv4 {

#define SCE_PFX_USE_ORIGIN_CENTER_OF_MASS	   0x01

struct SCE_PFX_ALIGNED(16) PfxCompoundContainer {
	PfxUInt8 reserved[64];
};

struct SCE_PFX_API PfxCompoundContainerInitParam {
	PfxUInt32 flags = 0;	///< @brief Configuration of a conpound container
	PfxUInt32 maxStoredRigidBodies = 0; ///< @brief The maximum number of rigid bodies stored in a compound container
	PfxUInt32 maxPairs = 0; ///< @brief The maximum number of pairs cached during solver calculation
};

SCE_PFX_API PfxUInt32 pfxCompoundContainerQueryMem(const PfxCompoundContainerInitParam &param);

SCE_PFX_API PfxInt32 pfxCompoundContainerInit(PfxCompoundContainer &compoundContainer, const PfxCompoundContainerInitParam &param, void *workBuff, PfxUInt32 workBytes);

SCE_PFX_API PfxInt32 pfxCompoundContainerTerm(PfxCompoundContainer &compoundContainer);

SCE_PFX_API PfxInt32 pfxCompoundContainerReset(PfxCompoundContainer &compoundContainer);

SCE_PFX_API PfxUInt32 pfxCompoundContainerGetNumRigidBodies(PfxCompoundContainer &compoundContainer);

SCE_PFX_API PfxUInt32 pfxCompoundContainerGetRigidBodyId(PfxCompoundContainer &compoundContainer, PfxUInt32 childId);

SCE_PFX_API PfxUInt32 pfxCompoundContainerFindRigidBodyId(PfxCompoundContainer &compoundContainer, PfxUInt32 rigidBodyId);

SCE_PFX_API PfxUInt32 pfxCompoundContainerGetOriginBody(PfxCompoundContainer &compoundContainer);

SCE_PFX_API PfxInt32 pfxCompoundContainerSetOriginBody(PfxCompoundContainer &compoundContainer, PfxUInt32 rigidBodyId);

SCE_PFX_API PfxInt32 pfxCompoundContainerAttachBody(PfxCompoundContainer &compoundContainer, PfxUInt32 rigidBodyId);

SCE_PFX_API PfxInt32 pfxCompoundContainerDetachBody(PfxCompoundContainer &compoundContainer, PfxUInt32 rigidBodyId);

SCE_PFX_API PfxInt32 pfxCompoundContainerFinish(PfxCompoundContainer &compoundContainer, PfxRigidState *states, PfxRigidBody *bodies, PfxUInt32 numRigidBodies);

SCE_PFX_API PfxInt32 pfxCompoundContainerAttachBodyImmediately(PfxCompoundContainer &compoundContainer, PfxUInt32 rigidBodyId, PfxRigidState *states, PfxRigidBody *bodies, PfxUInt32 numRigidBodies);

SCE_PFX_API PfxInt32 pfxCompoundContainerDetachBodyImmediately(PfxCompoundContainer &compoundContainer, PfxUInt32 rigidBodyId, PfxRigidState *states, PfxRigidBody *bodies, PfxUInt32 numRigidBodies);

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_COMPOUND_CONTAINER_H_ */
