/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_LINE_SEGMENT_H
#define _SCE_PFX_LINE_SEGMENT_H

namespace sce {
namespace pfxv4 {


/// @brief Line segment
struct SCE_PFX_API PfxLineSegment
{
	PfxVector3 m_point1; ///< Point 1 of the line segment
	PfxVector3 m_point2; ///< Point 2 of the ellipse

	/// @brief Default constructor (do nothing)
	PfxLineSegment() {}
	
	/// @brief Construct the line segment from points 1 and 2
	PfxLineSegment(const PfxVector3 &point1, const PfxVector3 &point2);
	
	/// @brief Set points 1 and 2
	void set(const PfxVector3 &point1, const PfxVector3 &point2);
};

inline
PfxLineSegment::PfxLineSegment(const PfxVector3 &point1, const PfxVector3 &point2)
{
	m_point1 = point1;
	m_point2 = point2;
}

inline
void PfxLineSegment::set(const PfxVector3 &point1, const PfxVector3 &point2)
{
	m_point1 = point1;
	m_point2 = point2;
}


} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_ELLIPSE_H
