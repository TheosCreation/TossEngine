/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_RIGID_BODY_H
#define _SCE_PFX_RIGID_BODY_H

#include "../base/pfx_common.h"

namespace sce {
namespace pfxv4 {


#define SCE_PFX_FIX_ROTATION_X 0x01 ///< @brief Fix rotation of local axis X
#define SCE_PFX_FIX_ROTATION_Y 0x02 ///< @brief Fix rotation of local axis Y
#define SCE_PFX_FIX_ROTATION_Z 0x04 ///< @brief Fix rotation of local axis Z

/// @brief Rigid body attribute
class SCE_PFX_API SCE_PFX_ALIGNED(16) PfxRigidBody
{
private:
	// Rigid Body constants
	PfxMatrix3     m_inertia;     // Inertia matrix in body's coords
	PfxFloat       m_mass;        // Rigid Body mass
	PfxFloat       m_restitution; // Coefficient of restitution
	PfxFloat       m_friction;    // Coefficient of friction
	PfxFloat       m_rollingFriction; // Coefficient of rotational friction
	PfxUInt32      m_arrangeInertia;

public:
	static const PfxUInt32 bytesOfRigidBody = 56; 

	void save(PfxUInt8 *pout, PfxUInt32 bytes) const;

	void load(const PfxUInt8 *pout, PfxUInt32 bytes);

public:
	/// @brief Reset parameters
	inline void reset();
	
	/// @brief Limit the rotation behavior
	inline	void arrangeInertia(PfxUInt32 mask) {m_arrangeInertia = mask;}
	PfxUInt32 getArrangeInertia() const { return m_arrangeInertia; }

	/// @brief Get mass of the rigid body
	PfxFloat       getMass() const {return m_mass;};

	/// @brief Get inverse mass
	PfxFloat    getMassInv() const;

	/// @brief Set mass of the rigid body
	void           setMass(PfxFloat mass) { SCE_PFX_VALIDATE_FLOAT(mass);  m_mass = mass; }

	/// @brief Get inertia tensor of the rigid body
	const PfxMatrix3& getInertia() const {return m_inertia;}

	/// @brief Get inverse of inertia tensor
	PfxMatrix3	getInertiaInv() const;

	/// @brief Set inertia tensor of the rigid body
	void              setInertia(const PfxMatrix3 & inertia) 	
	{	SCE_PFX_VALIDATE_MATRIX3(inertia);
		m_inertia = inertia;
	}

	/// @brief Get a coefficient of restitution of the rigid body
	PfxFloat       getRestitution() const {return m_restitution;}

	/// @brief Set a coefficient of restitution of the rigid body
	void           setRestitution(PfxFloat restitution) { SCE_PFX_VALIDATE_FLOAT(restitution); m_restitution = restitution; }

	/// @brief Get a coefficient of friction of the rigid body
	PfxFloat       getFriction() const {return m_friction;}

	/// @brief Set a coefficient of friction of the rigid body
	void           setFriction(PfxFloat friction) { SCE_PFX_VALIDATE_FLOAT(friction); m_friction = friction;}

	/// @brief Get a coefficient of rotational friction of the rigid body
	PfxFloat       getRollingFriction() const {return m_rollingFriction;}

	/// @brief Set a coefficient of rotational friction of the rigid body
	void           setRollingFriction(PfxFloat friction) { SCE_PFX_VALIDATE_FLOAT(friction); m_rollingFriction = friction;}
};

inline
void PfxRigidBody::reset()
{
	m_inertia = PfxMatrix3::identity();
	m_mass = 0.0f;
	m_restitution = 0.2f;
	m_friction = 0.6f;
	m_rollingFriction = 0.0f;
	m_arrangeInertia = 0;
}


} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_RIGID_BODY_H
