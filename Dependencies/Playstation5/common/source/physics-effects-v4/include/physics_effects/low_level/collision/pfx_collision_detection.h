/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_COLLISION_DETECTION_H_
#define _SCE_PFX_COLLISION_DETECTION_H_

#include "../../base_level/rigidbody/pfx_rigid_state.h"
#include "../../base_level/collision/pfx_collidable.h"
#include "../../base_level/collision/pfx_contact_manifold.h"
#include "../../base_level/solver/pfx_constraint_pair.h"

#include "../rigidbody/pfx_rigid_body_context.h"
#include "../collision/pfx_contact_container.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Detect Collision

/// @brief Detect collisions
/// @details Detect collisions of pairs. Contact results are stored in the contact container.
/// It calls a different collision function according to each shape combination.
/// If a collision is detected, the collision coordinates, the repelling direction and the penetration
/// depth are calculated, and up to four contact points is stored to the contact manifold.
/// @param context Rigid body context
/// @param sharedParam shared parameter
/// @param[in,out] contactContainer contact container
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxDetectCollision(PfxRigidBodyContext &context,PfxRigidBodySharedParam &sharedParam, PfxContactContainer &contactContainer);

/// @brief Non blocking version of pfxDetectCollision()
SCE_PFX_API PfxInt32 pfxDispatchDetectCollision(PfxRigidBodyContext &context,PfxContactContainer &contactContainer);

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_COLLISION_DETECTION_H_ */
