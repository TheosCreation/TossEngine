/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_SHAPE_H
#define _SCE_PFX_SHAPE_H

#include "../base/pfx_common.h"
#include "../base/pfx_simd_utils.h"
#include "pfx_box.h"
#include "pfx_sphere.h"
#include "pfx_capsule.h"
#include "pfx_cylinder.h"
#include "pfx_core_sphere.h"

namespace sce {
namespace pfxv4 {

struct PfxConvexMesh;
struct PfxLargeTriMesh;

/// @brief Shape type
enum ePfxShapeType
{
	kPfxShapeSphere = 0,	///< Sphere
	kPfxShapeBox,			///< Box
	kPfxShapeCapsule,		///< Capsule
	kPfxShapeCylinder,		///< Cylinder
	kPfxShapeConvexMesh,	///< Convex mesh
	kPfxShapeLargeTriMesh,	///< Large mesh
	kPfxShapeCoreSphere,	///< Core sphere
	kPfxShapeReserved0,
	kPfxShapeReserved1,
	kPfxShapeReserved2,
	kPfxShapeReserved3,
	kPfxShapeReserved4,
	kPfxShapeCount
};

/// @brief Shape

class SCE_PFX_API PfxShape
{
friend class PfxShapeElement;
friend class PfxCollidable;

private:
	union {
		PfxFloat  m_vecDataF[3];
		PfxUInt32 m_vecDataI[3];
		PfxUInt64 m_vecDataPtr[2];
	};
	PfxUInt8 m_type;
	SCE_PFX_PADDING3
	PfxFloat m_scale[3];
	PfxFloat m_offsetPosition[3];
	PfxFloat m_offsetOrientation[4];
	PfxUInt32 m_contactFilterSelf;
	PfxUInt32 m_contactFilterTarget;
	SCE_PFX_PADDING4
	PfxUInt32 m_userData;

public:
	PfxBool isMeshReady() const {return m_vecDataPtr[0] != 0;}

	inline PfxShape(const PfxCoreSphere &coreSphere);

	PfxShape() {}

	static const PfxUInt32 bytesOfShape = 68; 

	void save(PfxUInt8 *pout, PfxUInt32 bytes, PfxUInt32 meshId, PfxUInt32 meshBytes) const;

	void load(const PfxUInt8 *pout, PfxUInt32 bytes);

public:
	/// @brief Reset parameters
	inline void reset();

	/// @brief Initialize the shape as box
	inline void setBox(const PfxBox & box);

	/// @brief Initialize the shape as capsule
	inline void setCapsule(const PfxCapsule &capsule);

	/// @brief Initialize the shape as cylinder
	inline void setCylinder(const PfxCylinder &cylinder);

	/// @brief Initialize the shape as sphere
	inline void setSphere(const PfxSphere &sphere);

	/// @brief Initialize the shape as convex mesh
	inline void setConvexMesh(const PfxConvexMesh *convexMesh,PfxFloat scale = 1.0f);

	/// @brief Initialize the shape as large mesh
	inline void setLargeTriMesh(const PfxLargeTriMesh *largeMesh,PfxFloat scale = 1.0f);

	/// @brief Initialize the shape as core sphere
	inline void setCoreSphere(const PfxVector3 &position, PfxFloat radius);

	/// @brief Set scale of the shape
	inline PfxInt32 setScale(PfxFloat scale);
	inline PfxInt32 setScaleXyz(PfxFloat scaleX,PfxFloat scaleY,PfxFloat scaleZ);
	inline PfxInt32 setScaleXyz(const PfxVector3 &scaleXyz);

	/// @brief Get type of the shape
	inline PfxUInt8			getType() const;

	/// @brief Get box
	inline PfxBox			getBox()const ;

	/// @brief Get capsule
	inline PfxCapsule		getCapsule() const;

	/// @brief Get cylinder
	inline PfxCylinder		getCylinder() const;

	/// @brief Get sphere
	inline PfxSphere		getSphere() const;

	/// @brief Get a pointer to the convex mesh
	inline const PfxConvexMesh*   getConvexMesh() const;

	/// @brief Get a pointer to the large mesh
	inline const PfxLargeTriMesh* getLargeTriMesh() const;

	/// @brief Get core sphere
	inline PfxCoreSphere getCoreSphere() const;

	/// @brief Get user shape
	inline const void* getUserShape() const;

	/// @brief Get scale of the shape
	PfxFloat getScale() const {return m_scale[0];}
	void getScaleXyz(PfxFloat &scaleX,PfxFloat &scaleY,PfxFloat &scaleZ) const
		{scaleX = m_scale[0];scaleY = m_scale[1];scaleZ = m_scale[2];}
	PfxVector3 getScaleXyz() const {return pfxReadVector3(m_scale);}

	/// @brief Set an offset transform
	inline void setOffsetTransform(const PfxTransform3 & xfrm);

	/// @brief Set an offset translation
	inline void setOffsetPosition(const PfxVector3 & pos) {pfxStoreVector3(pos,m_offsetPosition);}

	/// @brief Set an offset orientation
	inline void setOffsetOrientation(const PfxQuat & rot) {pfxStoreQuat(rot,m_offsetOrientation);}

	/// @brief Get an offset transform
	inline PfxTransform3	getOffsetTransform() const;

	/// @brief Get an offset translation
	inline PfxVector3		getOffsetPosition() const {return pfxReadVector3(m_offsetPosition);}

	/// @brief Get an offset orientation
	inline PfxQuat			getOffsetOrientation() const {return pfxReadQuat(m_offsetOrientation);}

	/// @brief Set data as PfxFloat
	inline void setDataFloat(int i,PfxFloat v) {m_vecDataF[i]=v;}

	/// @brief Set data as PfxUInt32
	inline void setDataInteger(int i,PfxUInt32 v) {m_vecDataI[i]=v;}

	/// @brief Set data as PfxUInt64
	inline void setDataPtr64(int i,PfxUInt64 v) {m_vecDataPtr[i]=v;}

	/// @brief Get data as PfxFloat
	inline PfxFloat getDataFloat(int i) const {return m_vecDataF[i];}

	/// @brief Get data as PfxUInt32
	inline PfxUInt32 getDataInteger(int i) const {return m_vecDataI[i];}

	/// @brief Get data as PfxUInt64
	inline PfxUInt64 getDataPtr64(int i) const {return m_vecDataPtr[i];}

	/// @brief Get self contact filter
	PfxUInt32	getContactFilterSelf() const {return m_contactFilterSelf;}

	/// @brief Set self contact filter
	void		setContactFilterSelf(PfxUInt32 filter) {m_contactFilterSelf = filter;}

	/// @brief Get target contact filter
	PfxUInt32	getContactFilterTarget() const {return m_contactFilterTarget;}

	/// @brief Set target contact filter
	void		setContactFilterTarget(PfxUInt32 filter) {m_contactFilterTarget = filter;}

	/// @brief Get user data
	PfxUInt32	getUserData() const {return m_userData;}

	/// @brief Set user data
	void		setUserData(PfxUInt32 data) {m_userData = data;}

	/// @brief Get AABB of the shape
	void		getAabb(PfxVector3 &aabbMin,PfxVector3 &aabbMax) const;
};

// Ability to override the GetShapeAabb call.
typedef void (*PfxFuncGetShapeAabb)(const PfxShape &shape,PfxVector3 &aabbMin,PfxVector3 &aabbMax);

int pfxSetGetShapeAabbFunc(PfxUInt8 shapeType,PfxFuncGetShapeAabb func);

} // namespace pfxv4
} // namespace sce

#include "pfx_shape_implementation.h"

#endif // _SCE_PFX_SHAPE_H
