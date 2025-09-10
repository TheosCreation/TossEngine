/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_CYLINDER_H
#define _SCE_PFX_CYLINDER_H

#include "../base/pfx_common.h"

namespace sce {
namespace pfxv4 {


/// @brief Cylinder
struct SCE_PFX_API PfxCylinder
{
	PfxFloat m_halfLen; ///< Half length of the cylinder
	PfxFloat m_radius; ///< Radius of the cylinder
	
	/// @brief Default constructor (do nothing)
	PfxCylinder() {}
	
	/// @brief Construct the cylinder from half length and radius
	PfxCylinder(PfxFloat halfLength, PfxFloat radius);

	/// @brief Set half length and radius
	void set(PfxFloat halfLength, PfxFloat radius);
};

inline
PfxCylinder::PfxCylinder(PfxFloat halfLength, PfxFloat radius)
{
	set(halfLength, radius);
}

inline
void PfxCylinder::set(PfxFloat halfLength, PfxFloat radius)
{
	SCE_PFX_VALIDATE_FLOAT(halfLength);
	SCE_PFX_VALIDATE_FLOAT(radius);
	m_halfLen = halfLength;
	m_radius = radius;
}


} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_CYLINDER_H
