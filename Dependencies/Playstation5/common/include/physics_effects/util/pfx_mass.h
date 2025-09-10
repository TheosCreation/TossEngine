/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


///////////////////////////////////////////////////////////////////////////////
// Mass , Inertia tensor calculation

#ifndef _SCE_PFX_MASS_H
#define _SCE_PFX_MASS_H

#include "../base_level/base/pfx_common.h"

namespace sce {
namespace pfxv4 {

/// @name Mass

/// @brief Calculate mass of the box
/// @param density Density
/// @param halfExtent Half extent of the box
/// @return Return mass.
SCE_PFX_API PfxFloat pfxCalcMassBox(PfxFloat density,const PfxVector3 &halfExtent);

/// @brief Calculate inertia tensor of the box
/// @param halfExtent Half extent of the box
/// @param mass Mass
/// @return Return inertia tensor.
SCE_PFX_API PfxMatrix3 pfxCalcInertiaBox(const PfxVector3 &halfExtent,PfxFloat mass);

/// @brief Calculate mass of the sphere
/// @param density Density
/// @param radius Radius of the sphere
/// @return Return mass.
SCE_PFX_API PfxFloat pfxCalcMassSphere(PfxFloat density,PfxFloat radius);

/// @brief Calculate inertia tensor of the sphere
/// @param radius Radius of the sphere
/// @param mass Mass
/// @return Return inertia tensor.
SCE_PFX_API PfxMatrix3 pfxCalcInertiaSphere(PfxFloat radius,PfxFloat mass);

/// @brief Calculate mass of the cylinder
/// @param density Density
/// @param halfLength Half length of the cylinder
/// @param radius Radius of the cylinder
/// @return Return mass.
SCE_PFX_API PfxFloat pfxCalcMassCylinder(PfxFloat density,PfxFloat halfLength,PfxFloat radius);

/// @brief Calculate inertia tensor of the cylinder parallel to the X-axis
/// @param halfLength Half length of the cylinder
/// @param radius Radius of the cylinder
/// @param mass Mass
/// @return Return inertia tensor.
SCE_PFX_API PfxMatrix3 pfxCalcInertiaCylinderX(PfxFloat halfLength,PfxFloat radius,PfxFloat mass);

/// @brief Calculate inertia tensor of the cylinder parallel to the Y-axis
/// @param halfLength Half length of the cylinder
/// @param radius Radius of the cylinder
/// @param mass Mass
/// @return Return inertia tensor.
SCE_PFX_API PfxMatrix3 pfxCalcInertiaCylinderY(PfxFloat halfLength,PfxFloat radius,PfxFloat mass);

/// @brief Calculate inertia tensor of the cylinder parallel to the Z-axis
/// @param halfLength Half length of the cylinder
/// @param radius Radius of the cylinder
/// @param mass Mass
/// @return Return inertia tensor.
SCE_PFX_API PfxMatrix3 pfxCalcInertiaCylinderZ(PfxFloat halfLength,PfxFloat radius,PfxFloat mass);


/// @brief Calculate mass of the capsule
/// @param density Density
/// @param halfLength Half length of the capsule
/// @param radius Radius of the capsule
/// @return Return mass.
SCE_PFX_API PfxFloat pfxCalcMassCapsule(PfxFloat density, PfxFloat halfLength, PfxFloat radius);

/// @brief Calculate inertia tensor of the capsule parallel to the X-axis
/// @param halfLength Half length of the capsule
/// @param radius Radius of the capsule
/// @param mass Mass
/// @return Return inertia tensor.
SCE_PFX_API PfxMatrix3 pfxCalcInertiaCapsuleX(PfxFloat halfLength, PfxFloat radius, PfxFloat mass);

/// @brief Calculate inertia tensor of the capsule parallel to the Y-axis
/// @param halfLength Half length of the capsule
/// @param radius Radius of the capsule
/// @param mass Mass
/// @return Return inertia tensor.
SCE_PFX_API PfxMatrix3 pfxCalcInertiaCapsuleY(PfxFloat halfLength, PfxFloat radius, PfxFloat mass);

/// @brief Calculate inertia tensor of the capsule parallel to the Z-axis
/// @param halfLength Half length of the capsule
/// @param radius Radius of the capsule
/// @param mass Mass
/// @return Return inertia tensor.
SCE_PFX_API PfxMatrix3 pfxCalcInertiaCapsuleZ(PfxFloat halfLength, PfxFloat radius, PfxFloat mass);

///////////////////////////////////////////////////////////////////////////////
/// Mass convertion

/// @brief Transfer inertia tensor
/// @param mass Mass
/// @param inertia Inertia tensor
/// @param translation Translation vector
/// @return Return translated inertia tensor.
SCE_PFX_API PfxMatrix3 pfxMassTranslate(PfxFloat mass,const PfxMatrix3 &inertia,const PfxVector3 &translation);

/// @brief Rotate inertia tensor
/// @param inertia Inertia tensor
/// @param rotate Rotation vector
/// @return Return rotated inertia tensor.
SCE_PFX_API PfxMatrix3 pfxMassRotate(const PfxMatrix3 &inertia,const PfxMatrix3 &rotate);


} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_MASS_H
