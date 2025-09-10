/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_SHAPE_IMPLEMENTATION_H
#define _SCE_PFX_SHAPE_IMPLEMENTATION_H

namespace sce {
namespace pfxv4 {

inline
PfxShape::PfxShape(const PfxCoreSphere &coreSphere)
{
	m_type = kPfxShapeCoreSphere;
	m_vecDataF[0] = coreSphere.m_radius;
	m_vecDataF[1] = 0.0f;
	m_vecDataF[2] = 0.0f;
	m_scale[0] = m_scale[1] = m_scale[2] = 1.0f;
	m_offsetPosition[0] = coreSphere.m_offsetPosition[0];
	m_offsetPosition[1] = coreSphere.m_offsetPosition[1];
	m_offsetPosition[2] = coreSphere.m_offsetPosition[2];
	m_offsetOrientation[0] = m_offsetOrientation[1] = m_offsetOrientation[2]= 0.0f;
	m_offsetOrientation[3] = 1.0f;
	m_contactFilterSelf = coreSphere.m_contactFilterSelf;
	m_contactFilterTarget = coreSphere.m_contactFilterTarget;
}

inline
void PfxShape::reset()
{
	m_type = kPfxShapeSphere;
	m_scale[0] = m_scale[1] = m_scale[2] = 1.0f;
	m_offsetPosition[0] = m_offsetPosition[1] = m_offsetPosition[2]= 0.0f;
	m_offsetOrientation[0] = m_offsetOrientation[1] = m_offsetOrientation[2]= 0.0f;
	m_offsetOrientation[3] = 1.0f;
	m_contactFilterSelf = m_contactFilterTarget = 0xffffffff;
}

inline
void PfxShape::setBox(const PfxBox & box)
{
	SCE_PFX_ALWAYS_ASSERT( box.m_half[ 0 ] >= 0.0f );
	SCE_PFX_ALWAYS_ASSERT( box.m_half[ 1 ] >= 0.0f );
	SCE_PFX_ALWAYS_ASSERT( box.m_half[ 2 ] >= 0.0f );
	m_vecDataF[0] = box.m_half[0];
	m_vecDataF[1] = box.m_half[1];
	m_vecDataF[2] = box.m_half[2];
	m_type = kPfxShapeBox;
}

inline
void PfxShape::setCapsule(const PfxCapsule &capsule)
{
	SCE_PFX_ALWAYS_ASSERT( capsule.m_halfLen >= 0.0f );
	SCE_PFX_ALWAYS_ASSERT( capsule.m_radius >= 0.0f );
	m_vecDataF[0] = capsule.m_halfLen;
	m_vecDataF[1] = capsule.m_radius;
	m_vecDataF[2] = 0.0f;
	m_type = kPfxShapeCapsule;
}

inline
void PfxShape::setCylinder(const PfxCylinder &cylinder)
{
	SCE_PFX_ALWAYS_ASSERT( cylinder.m_halfLen >= 0.0f );
	SCE_PFX_ALWAYS_ASSERT( cylinder.m_radius >= 0.0f );
	m_vecDataF[0] = cylinder.m_halfLen;
	m_vecDataF[1] = cylinder.m_radius;
	m_vecDataF[2] = 0.0f;
	m_type = kPfxShapeCylinder;
}

inline
void PfxShape::setSphere(const PfxSphere &sphere)
{
	SCE_PFX_ALWAYS_ASSERT( sphere.m_radius >= 0.0f );
	m_vecDataF[0] = sphere.m_radius;
	m_vecDataF[1] = 0.0f;
	m_vecDataF[2] = 0.0f;
	m_type = kPfxShapeSphere;
}

inline
void PfxShape::setConvexMesh(const PfxConvexMesh *convexMesh,PfxFloat scale)
{
	SCE_PFX_ASSERT(scale>0.0f);
	m_vecDataPtr[0] = (uintptr_t)convexMesh;
	m_vecDataPtr[1] = 0;
	m_type = kPfxShapeConvexMesh;
	setScale(scale);
}

inline
void PfxShape::setLargeTriMesh(const PfxLargeTriMesh *largeMesh,PfxFloat scale)
{
	SCE_PFX_ASSERT(scale>0.0f);
	m_vecDataPtr[0] = (uintptr_t)largeMesh;
	m_vecDataPtr[1] = 0;
	m_type = kPfxShapeLargeTriMesh;
	setScale(scale);
}

inline
void PfxShape::setCoreSphere(const PfxVector3 &position, PfxFloat radius)
{
	SCE_PFX_ALWAYS_ASSERT(radius > 0.0f);
	m_vecDataF[0] = radius;
	m_vecDataF[1] = 0.0f;
	m_vecDataF[2] = 0.0f;
	m_type = kPfxShapeCoreSphere;
	setOffsetOrientation(PfxQuat::identity());
	setOffsetPosition(position);
}

inline
void PfxShape::setOffsetTransform(const PfxTransform3 & xfrm)
{
	setOffsetOrientation(PfxQuat(xfrm.getUpper3x3()));
	setOffsetPosition(xfrm.getTranslation());
}

inline
PfxUInt8 PfxShape::getType() const
{
	return m_type;
}

inline
PfxBox PfxShape::getBox() const
{
	SCE_PFX_ASSERT(m_type==kPfxShapeBox);
	return PfxBox(m_vecDataF[0],m_vecDataF[1],m_vecDataF[2]);
}

inline
PfxCapsule PfxShape::getCapsule() const
{
	SCE_PFX_ASSERT(m_type==kPfxShapeCapsule);
	return PfxCapsule(m_vecDataF[0], m_vecDataF[1]);
}

inline
PfxCylinder PfxShape::getCylinder() const
{
	SCE_PFX_ASSERT(m_type==kPfxShapeCylinder);
	return PfxCylinder(m_vecDataF[0], m_vecDataF[1]);
}

inline
PfxSphere PfxShape::getSphere() const
{
	SCE_PFX_ASSERT(m_type==kPfxShapeSphere || m_type==kPfxShapeCoreSphere);
	return PfxSphere(m_vecDataF[0]);
}

inline
const PfxConvexMesh *PfxShape::getConvexMesh() const
{
	SCE_PFX_ASSERT(m_type==kPfxShapeConvexMesh);
	SCE_PFX_ASSERT(m_vecDataPtr[0]);
	return (PfxConvexMesh*)(uintptr_t)m_vecDataPtr[0];
}

inline
const PfxLargeTriMesh *PfxShape::getLargeTriMesh() const
{
	SCE_PFX_ASSERT(m_type==kPfxShapeLargeTriMesh);
	SCE_PFX_ASSERT(m_vecDataPtr[0]);
	return (PfxLargeTriMesh*)(uintptr_t)m_vecDataPtr[0];
}

inline
PfxCoreSphere PfxShape::getCoreSphere() const
{
	SCE_PFX_ALWAYS_ASSERT(m_type == kPfxShapeCoreSphere);
	return PfxCoreSphere(m_vecDataF[0], getOffsetPosition());
}

inline
PfxInt32 PfxShape::setScale(PfxFloat scale)
{
	return setScaleXyz(PfxVector3(scale));
}

inline
PfxInt32 PfxShape::setScaleXyz(PfxFloat scaleX,PfxFloat scaleY,PfxFloat scaleZ)
{
	return setScaleXyz(PfxVector3(scaleX,scaleY,scaleZ));
}

inline
PfxInt32 PfxShape::setScaleXyz(const PfxVector3 &scaleXyz)
{
	PfxVector3 chk = absPerElem(scaleXyz);
	if(chk[0] < 0.00001f || chk[0] < 0.00001f || chk[0] < 0.00001f) return SCE_PFX_ERR_INVALID_VALUE;
	if (m_type == kPfxShapeConvexMesh || m_type == kPfxShapeLargeTriMesh) {
		pfxStoreVector3(scaleXyz, m_scale); // convex mesh , large mesh
	}
	else {
		m_scale[0] = m_scale[1] = m_scale[2] = scaleXyz[0]; // every primitive only accepts uniform scaling
	}
	return SCE_PFX_OK;
}

inline
PfxTransform3 PfxShape::getOffsetTransform() const
{
	return PfxTransform3(getOffsetOrientation(),getOffsetPosition());
}

} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_SHAPE_IMPLEMENTATION_H
