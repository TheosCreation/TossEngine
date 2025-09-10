/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_COLLIDABLE_H
#define _SCE_PFX_COLLIDABLE_H

#include "pfx_shape.h"
#include "pfx_core_sphere.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Collidable Object

/// @brief Collidable
/// @details This is a class that expresses the shape of a rigid body.
/// PfxCollidable functions as a container that stores several PfxShape expressing a single shape.
/// 16-byte alignment is required.
class SCE_PFX_API SCE_PFX_ALIGNED(16) PfxCollidable
{
private:
	PfxShape *m_shapes;
	PfxCoreSphere *m_coreSpheres;
	PfxUInt8 m_numShapes;
	PfxUInt8 m_maxShapes;
	PfxUInt8 m_numCoreSpheres;
	PfxUInt8 m_maxCoreSpheres;
	PfxFloat m_center[3];	// AABB center (Local)
	PfxFloat m_half[3];		// AABB half (Local)
	PfxBool m_useCcd;

public:
    static const PfxUInt32 bytesOfCollidable = 44;

	void save(PfxUInt8 *pout, PfxUInt32 bytes, PfxUInt32 shapeId, PfxUInt32 coreSphereId) const;

	void load(const PfxUInt8 *pout, PfxUInt32 bytes, PfxShape *shapeBasePtr, PfxCoreSphere *coreSphereBasePtr);

public:
	/// @brief Reset parameters
	inline void reset();

	/// @brief
	void finish();

	/// @brief calculate an AABB in the specified orientation
	void calcAabb(const PfxQuat &offsetOrientation, PfxVector3 &center, PfxVector3 &extent) const;

	/// @brief Set a shape
	inline PfxUInt32 setShapeBuffer(PfxShape *baseAddr,  PfxUInt32 num);
	inline PfxUInt32 setCoreSphereBuffer(PfxCoreSphere *baseAddr,  PfxUInt32 num);
	
	inline PfxUInt32 addShape(const PfxShape &shape);
	inline PfxUInt32 addCoreSphere(const PfxCoreSphere &coreSphere);

	/// @brief Get the number of shapes
	inline PfxUInt32 getNumShapes() const {return m_numShapes;}
	inline PfxUInt32 getNumCoreSpheres() const {return m_numCoreSpheres;}

	/// @brief Get a shape
	inline PfxShape& getShape(int i) {return m_shapes[i];}
	inline PfxCoreSphere& getCoreSphere(int i) {return m_coreSpheres[i];}

	inline const PfxShape& getShape(int i) const {return m_shapes[i];}
	inline const PfxCoreSphere& getCoreSphere(int i) const {return m_coreSpheres[i];}

	inline void setCenter(const PfxVector3 &center);
	inline void setHalf(const PfxVector3 &half);

	/// @brief Get half extent of AABB
	inline PfxVector3 getHalf() const;

	/// @brief Get center of AABB
	inline PfxVector3 getCenter() const;

	/// @brief Check if it should be handled as continuous
	PfxBool isContinuous() const {return m_useCcd;}

	/// @brief Enable continuous collision for this collidable
	/// Enabling this can be used to expand AABB by velocities. works as discrete collision detection if there is no core spheres.
	void enableCcd() { m_useCcd = true; }

	/// @brief Disable continuous collision for this collidable
	void disableCcd() {m_useCcd = false;}
};

} // namespace pfxv4
} // namespace sce

#include "pfx_collidable_implementation.h"

#endif // _SCE_PFX_COLLIDABLE_H
