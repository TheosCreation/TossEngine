/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_SPHERE_H
#define _SCE_PFX_SPHERE_H

#include "../base/pfx_common.h"

namespace sce {
namespace pfxv4 {


/// @brief Sphere
struct SCE_PFX_API PfxSphere {
	PfxFloat m_radius; ///< Radius of the sphere

	/// @brief Default constructor (do nothing)
	PfxSphere() {}
	
	/// @brief Construct the sphere from radius
	PfxSphere( PfxFloat radius );
	
	/// @brief Set radius
	void  set( PfxFloat radius );
};

inline
PfxSphere::PfxSphere( PfxFloat radius )
{
	set(radius);
}

inline
void PfxSphere::set( PfxFloat radius )
{
	SCE_PFX_VALIDATE_FLOAT(radius);
	m_radius = radius;
}


} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_SPHERE_H
