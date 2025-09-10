/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_CIRCLE_H
#define _SCE_PFX_CIRCLE_H

#include "../base/pfx_common.h"

namespace sce {
namespace pfxv4 {


/// @brief Circle
struct SCE_PFX_API PfxCircle
{
	PfxFloat m_radius; ///< Radius of the circle
	PfxVector3 m_normal; ///< Normal of the circle

	/// @brief Default constructor (do nothing)
	PfxCircle() {}
	
	/// @brief Construct the circle from radius and normal vector
	PfxCircle(PfxFloat radius, const PfxVector3 &normal);
	
	/// @brief Set radius and normal vector
	void set(PfxFloat radius, const PfxVector3 &normal);
};

inline
PfxCircle::PfxCircle(PfxFloat radius, const PfxVector3 &normal)
{
	m_radius = radius;
	m_normal = normal;
}

inline
void PfxCircle::set(PfxFloat radius, const PfxVector3 &normal)
{
	m_radius = radius;
	m_normal = normal;
}


} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_CIRCLE_H
