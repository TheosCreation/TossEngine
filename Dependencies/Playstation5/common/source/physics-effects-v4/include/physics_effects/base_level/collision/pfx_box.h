/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_BOX_H
#define _SCE_PFX_BOX_H

#include "../base/pfx_common.h"

namespace sce{
namespace pfxv4{


/// @brief Box
/// @details This class describes a box shape.
struct SCE_PFX_API PfxBox {
	PfxVector3 m_half; ///< @brief Half extent of the box

	/// @brief Default constructor (do nothing)
	inline PfxBox() {}
	
	/// @brief Construct the box from half extent
	inline PfxBox(const PfxVector3 & half);

	/// @brief Construct the box from x,y,z extent
	inline PfxBox(PfxFloat hx, PfxFloat hy, PfxFloat hz);
	
	/// @brief Set half extent
	inline void set(const PfxVector3 & half);
	
	/// @brief Set x,y,z extent
	inline void set(PfxFloat hx, PfxFloat hy, PfxFloat hz);
};

inline
PfxBox::PfxBox(const PfxVector3 & half)
{
	set(half);
}

inline
PfxBox::PfxBox(PfxFloat hx, PfxFloat hy, PfxFloat hz)
{
	set(hx, hy, hz);
}

inline
void PfxBox::set(const PfxVector3 & half)
{
	SCE_PFX_VALIDATE_VECTOR3(half);
	m_half = half;
}

inline
void PfxBox::set(PfxFloat hx, PfxFloat hy, PfxFloat hz)
{
	SCE_PFX_VALIDATE_FLOAT(hx);
	SCE_PFX_VALIDATE_FLOAT(hy);
	SCE_PFX_VALIDATE_FLOAT(hz);
	m_half = PfxVector3(hx, hy, hz);
}


} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_BOX_H
