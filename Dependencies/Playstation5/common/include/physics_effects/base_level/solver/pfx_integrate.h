/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_INTEGRATE_H_
#define _SCE_PFX_INTEGRATE_H_

#include "../base/pfx_common.h"
#include "../rigidbody/pfx_rigid_body.h"
#include "../rigidbody/pfx_rigid_state.h"

namespace sce {
namespace pfxv4 {


/// @brief Integrate velocity
/// @details Integrate velocity and calculate the position after time step.
/// @param[in,out] state Rigid body state
/// @param body Rigid body attribute
/// @param timeStep Time step
SCE_PFX_API void pfxIntegrate(PfxRigidState &state,const PfxRigidBody &body,PfxFloat timeStep);

/// @brief Apply external force
/// @details Calculate velocity from external force and torque applied to the rigid body.
/// @param[in,out] state Rigid body state
/// @param body Rigid body attribute
/// @param extForce Force
/// @param extTorque Torque
/// @param timeStep Time step
SCE_PFX_API void pfxApplyExternalForce(PfxRigidState &state,const PfxRigidBody &body,const PfxVector3 &extForce,const PfxVector3 &extTorque,PfxFloat timeStep);


} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_INTEGRATE_H_ */
