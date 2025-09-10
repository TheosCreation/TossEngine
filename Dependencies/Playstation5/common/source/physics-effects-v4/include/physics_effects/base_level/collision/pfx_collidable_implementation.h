/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_COLLIDABLE_IMPLEMENTATION_H
#define _SCE_PFX_COLLIDABLE_IMPLEMENTATION_H

namespace sce {
namespace pfxv4 {

inline
void PfxCollidable::reset()
{
	m_shapes = nullptr;
	m_coreSpheres = nullptr;
	m_numShapes = 0;
	m_maxShapes = 0;
	m_numCoreSpheres = 0;
	m_maxCoreSpheres = 0;
	m_center[0] = 0.0f;
	m_center[1] = 0.0f;
	m_center[2] = 0.0f;
	m_half[0] = 0.0f;
	m_half[1] = 0.0f;
	m_half[2] = 0.0f;
	m_useCcd = false;
}

inline PfxUInt32 PfxCollidable::setShapeBuffer(PfxShape *baseAddr,  PfxUInt32 num)
{
	if (num > 255) return SCE_PFX_ERR_OUT_OF_RANGE;
	m_shapes = baseAddr;
	m_numShapes = 0;
	m_maxShapes = (PfxUInt8)num;
	return SCE_PFX_OK;
}

inline PfxUInt32 PfxCollidable::setCoreSphereBuffer(PfxCoreSphere *baseAddr,  PfxUInt32 num)
{
	if (num > 255) return SCE_PFX_ERR_OUT_OF_RANGE;
	m_coreSpheres = baseAddr;
	m_numCoreSpheres = 0;
	m_maxCoreSpheres = (PfxUInt8)num;
	return SCE_PFX_OK;
}

inline PfxUInt32 PfxCollidable::addShape(const PfxShape &shape)
{
	if (m_numShapes == m_maxShapes) return SCE_PFX_ERR_OUT_OF_RANGE;

	m_shapes[m_numShapes++] = shape;

	return SCE_PFX_OK;
}

inline PfxUInt32 PfxCollidable::addCoreSphere(const PfxCoreSphere &coreSphere)
{
	if (m_numCoreSpheres == m_maxCoreSpheres) return SCE_PFX_ERR_OUT_OF_RANGE;

	m_coreSpheres[m_numCoreSpheres++] = coreSphere;

	return SCE_PFX_OK;
}

inline
void PfxCollidable::setCenter(const PfxVector3 &center)
{
	pfxStoreVector3(center,m_center);
}

inline
void PfxCollidable::setHalf(const PfxVector3 &half)
{
	pfxStoreVector3(half,m_half);
}

inline
PfxVector3 PfxCollidable::getHalf() const
{
	return pfxReadVector3(m_half);
}

inline
PfxVector3 PfxCollidable::getCenter() const
{
	return pfxReadVector3(m_center);
}

} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_COLLIDABLE_IMPLEMENTATION_H
