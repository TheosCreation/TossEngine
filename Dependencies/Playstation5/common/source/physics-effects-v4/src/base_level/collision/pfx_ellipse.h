/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_ELLIPSE_H
#define _SCE_PFX_ELLIPSE_H

namespace sce {
namespace pfxv4 {


/// @brief Ellipse
struct SCE_PFX_API PfxEllipse
{
	PfxVector3 m_majorAxis; ///< Major axis of the ellipse (normalized)
	PfxVector3 m_minorAxis; ///< Minor axis of the ellipse (normalized)
	PfxFloat m_majorRadius; ///< Major radius of the ellipse
	PfxFloat m_minorRadius; ///< Minor radius of the ellipse

	/// @brief Default constructor (do nothing)
	PfxEllipse() {}
	
	/// @brief Construct the ellipse from major and minor axes
	PfxEllipse(const PfxVector3 &majorAxis, const PfxVector3 &minorAxis, PfxFloat majorRadius, PfxFloat minorRadius);
	
	/// @brief Set major and minor axes
	void set(const PfxVector3 &majorAxis, const PfxVector3 &minorAxis, PfxFloat majorRadius, PfxFloat minorRadius);
};

inline
PfxEllipse::PfxEllipse(const PfxVector3 &majorAxis, const PfxVector3 &minorAxis, PfxFloat majorRadius, PfxFloat minorRadius)
{
	m_majorAxis = majorAxis;
	m_minorAxis = minorAxis;
	m_majorRadius = majorRadius;
	m_minorRadius = minorRadius;
}

inline
void PfxEllipse::set(const PfxVector3 &majorAxis, const PfxVector3 &minorAxis, PfxFloat majorRadius, PfxFloat minorRadius)
{
	m_majorAxis = majorAxis;
	m_minorAxis = minorAxis;
	m_majorRadius = majorRadius;
	m_minorRadius = minorRadius;
}


} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_ELLIPSE_H
