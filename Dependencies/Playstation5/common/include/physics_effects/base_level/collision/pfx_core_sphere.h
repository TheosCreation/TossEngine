/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CORE_SPHERE_H
#define _SCE_PFX_CORE_SPHERE_H

#include "../base/pfx_common.h"
#include "../base/pfx_simd_utils.h"

namespace sce {
namespace pfxv4 {

/// @brief Core Sphere
class SCE_PFX_API PfxCoreSphere
{
friend class PfxShape;

private:
	PfxFloat m_radius;
	PfxFloat m_offsetPosition[3];
	PfxUInt32 m_contactFilterSelf;
	PfxUInt32 m_contactFilterTarget;

public:
	static const PfxUInt32 bytesOfCoreSphere = 24; 

	void save(PfxUInt8 *pout, PfxUInt32 bytes) const;

	void load(const PfxUInt8 *pout, PfxUInt32 bytes);

public:
	inline PfxCoreSphere() { reset(); }
	inline PfxCoreSphere(PfxFloat radius, const PfxVector3 & pos) { reset(); setRadius(radius); setOffsetPosition(pos); }

	inline void reset();

	/// @brief Set radius of a core sphere
	inline void setRadius(PfxFloat radius) {m_radius = radius;}

	/// @brief Get radius of a core sphere
	inline PfxFloat getRadius() const {return m_radius;}

	/// @brief Set an offset translation
	inline void setOffsetPosition(const PfxVector3 & pos) {pfxStoreVector3(pos,m_offsetPosition);}

	/// @brief Get an offset translation
	inline PfxVector3 getOffsetPosition() const {return pfxReadVector3(m_offsetPosition);}

	/// @brief Get self contact filter
	inline PfxUInt32 getContactFilterSelf() const {return m_contactFilterSelf;}

	/// @brief Set self contact filter
	inline void setContactFilterSelf(PfxUInt32 filter) {m_contactFilterSelf = filter;}

	/// @brief Get target contact filter
	inline PfxUInt32 getContactFilterTarget() const {return m_contactFilterTarget;}

	/// @brief Set target contact filter
	inline void setContactFilterTarget(PfxUInt32 filter) {m_contactFilterTarget = filter;}

	/// @brief Get AABB of the shape
	inline void getAabb(PfxVector3 &aabbMin,PfxVector3 &aabbMax) const;
};

inline void PfxCoreSphere::reset()
{
	m_offsetPosition[0] = m_offsetPosition[1] = m_offsetPosition[2] = 0.0f;
	m_contactFilterSelf = m_contactFilterTarget = 0xffffffff;
}

inline void PfxCoreSphere::getAabb(PfxVector3 &aabbMin,PfxVector3 &aabbMax) const
{
	PfxVector3 offsetPosition = getOffsetPosition();
	aabbMin = offsetPosition - PfxVector3(m_radius);
	aabbMax = offsetPosition + PfxVector3(m_radius);
}

} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_CORE_SPHERE_H
