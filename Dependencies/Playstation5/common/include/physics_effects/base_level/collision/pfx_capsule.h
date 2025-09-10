/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_CAPSULE_H
#define _SCE_PFX_CAPSULE_H

#include "../base/pfx_common.h"

namespace sce {
namespace pfxv4 {


/// @brief Capsule
struct SCE_PFX_API PfxCapsule
{
	PfxFloat m_halfLen; ///< Half length of the capsule
	PfxFloat m_radius; ///< Radius of the capsule

	/// @brief Default constructor (do nothing)
	PfxCapsule() {}
	
	/// @brief Construct the capsule from half length and radius
	PfxCapsule(PfxFloat halfLength, PfxFloat radius);
	
	/// @brief Set half length and radius
	void set(PfxFloat halfLength, PfxFloat radius);
};

inline
PfxCapsule::PfxCapsule(PfxFloat halfLength, PfxFloat radius)
{
	set(halfLength, radius);
}

inline
void PfxCapsule::set(PfxFloat halfLength, PfxFloat radius)
{
	SCE_PFX_VALIDATE_FLOAT(halfLength);
	SCE_PFX_VALIDATE_FLOAT(radius);
	m_halfLen = halfLength;
	m_radius = radius;
}


} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_CAPSULE_H
