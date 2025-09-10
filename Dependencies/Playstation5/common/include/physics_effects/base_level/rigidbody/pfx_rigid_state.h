/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_RIGID_STATE_H
#define _SCE_PFX_RIGID_STATE_H

#include "../base/pfx_common.h"
#include "../base/pfx_simd_utils.h"
#include "../base/pfx_large_position.h"

namespace sce {
namespace pfxv4 {

/// @brief Motion Type
enum ePfxMotionType {
	kPfxMotionTypeFixed = 0, ///< @brief Fixed in the world
	kPfxMotionTypeActive,	 ///< @brief Active
	kPfxMotionTypeKeyframe,	 ///< @brief Keyframed or animated
	kPfxMotionTypeOneWay,    ///< @brief One way (can be affected but can't push other objects)
	kPfxMotionTypeTrigger,   ///< @brief Trigger
	kPfxMotionTypeCollisionKeyframe, ///< @brief Keyframed but can collide
	kPfxMotionTypeCount      
};

/// @brief Solver quality
enum ePfxSolverQuality {
	kPfxSolverQualityLevel1 = 0,
	kPfxSolverQualityLevel2,
	kPfxSolverQualityLevel3,
	kPfxSolverQualityCount,
};

#define SCE_PFX_MOTION_MASK_DYNAMIC(motion)   ((1<<(motion))&0x0a) // Active,OneWay
#define SCE_PFX_MOTION_MASK_STATIC(motion)    ((1<<(motion))&0x15) // Fixed,Keyframe // [SMS_CHANGE] removed useless bit in the mask
#define SCE_PFX_MOTION_MASK_CAN_SLEEP(motion) ((1<<(motion))&0x2e) // Can sleep

#define SCE_PFX_MOTION_MASK_SLEEPING 0x80 // Is sleeping
#define SCE_PFX_MOTION_MASK_TYPE     0x7f // Motion Type
#define SCE_PFX_MOTION_MASK_ALL      0x7f // Motion Type

#ifdef _MSC_VER
#pragma warning(push)          // [SMS_CHANGE] disable compiler warnings
#pragma warning(disable: 4201) // [SMS_CHANGE] nonstandard extension used : nameless struct/union
#endif

/// @brief Rigid body state
/// @details This class represents the current state of the rigid body. It stores data such as position, velocity, and contact filter.
/// 16-byte alignment is required.
class SCE_PFX_API SCE_PFX_ALIGNED(16) PfxRigidState
{
private:
	union {
		struct {
			PfxUInt8	m_useSleep  : 1;
			PfxUInt8	m_sleeping  : 1;
			PfxUInt8	m_proxyShift : 1;
			PfxUInt8	m_isInitialized : 1;
			PfxUInt8	m_isJointRoot : 1;
			PfxUInt8	m_closestHit : 1;
			PfxUInt8	m_allowSkippingNarrowPhaseDetection : 1;	// [SMS_CHANGE]
			PfxUInt8	m_reserved1 : 1;
		};
		PfxUInt8 m_flags;
	};
	
	PfxUInt8	m_motionType;
	PfxUInt16	m_sleepCount;
	PfxUInt32	m_rigidBodyId;

	PfxUInt32	m_contactFilterSelf;
	PfxUInt32	m_contactFilterTarget;
	PfxUInt16	m_collisionIgnoreGroup[2];
	PfxUInt8	m_solverQuality;
	PfxUInt16	m_islandRootId;
	PfxFloat	m_linearDamping;
	PfxFloat	m_angularDamping;

	PfxFloat	m_maxLinearVelocity;
	PfxFloat	m_maxAngularVelocity;
	
	PfxUInt32	m_userParam[4];

	PfxLargePosition m_position;
	PfxQuat		m_orientation;
	PfxVector3	m_linearVelocity;
	PfxVector3	m_angularVelocity;
	void *m_userData;

public:
	// Internal method
	PfxBool getProxyShiftFlag() const {return m_proxyShift==1;}
	void setProxyShiftFlag(PfxBool shifted) {m_proxyShift = shifted?1:0;}

    PfxBool getIsInitialized() const { return m_isInitialized==1; }
    void setIsInitialized(PfxBool initialized) { m_isInitialized = initialized?1:0;}

	PfxBool getClosestHit() const { return m_closestHit == 1; }
	void setClosestHit(PfxBool hit) { m_closestHit = hit ? 1 : 0; }

	PfxUInt16 getIslandRootId() const { return m_islandRootId; }
	void setIslandRootId(PfxUInt16 islandRootId) { m_islandRootId = islandRootId; }

	static const PfxUInt32 bytesOfRigidState = 132; 

	void save(PfxUInt8 *pout, PfxUInt32 bytes) const;

	void load(const PfxUInt8 *pout, PfxUInt32 bytes);

public:
	/// @brief Reset parameters
	void reset();
	
	/// @brief Get ID of the rigid body
	PfxUInt32	getRigidBodyId() const {return m_rigidBodyId;}

	/// @brief Set ID of the rigid body
	void		setRigidBodyId(PfxUInt32 i) {m_rigidBodyId = i;}

	/// @brief Get self contact filter of the rigid body
	PfxUInt32	getContactFilterSelf() const {return m_contactFilterSelf;}

	/// @brief Set self contact filter of the rigid body
	void		setContactFilterSelf(PfxUInt32 filter) {m_contactFilterSelf = filter;}

	/// @brief Get target contact filter of the rigid body
	PfxUInt32	getContactFilterTarget() const {return m_contactFilterTarget;}

	/// @brief Set target contact filter of the rigid body
	void		setContactFilterTarget(PfxUInt32 filter) {m_contactFilterTarget = filter;}
	
	/// @brief Get motion type of the rigid body
	ePfxMotionType	getMotionType() const {return (ePfxMotionType)m_motionType;}

	/// @brief Set motion type of the rigid body
	void		setMotionType(ePfxMotionType t) { SCE_PFX_ASSERT(t<kPfxMotionTypeCount);m_motionType = (PfxUInt8)t;wakeup();}

	PfxUInt8	getMotionMask() const {return (PfxUInt8)(m_motionType|(m_sleeping<<7));}
	
	/// @brief Return true if the rigid body is asleep
	PfxBool		isAsleep() const {return m_sleeping==1;}
	
	/// @brief Return true if the rigid body is awake
	PfxBool		isAwake() const {return m_sleeping==0;}

	/// @brief Wake up
	void wakeup() {m_sleeping=0;m_sleepCount=0;}
	
	/// @brief Sleep
	void sleep();
	
	/// @brief Check if the rigid body can sleep
	PfxUInt8	getUseSleep() const {return m_useSleep;}

	/// @brief Enable or disable sleep
	void		setUseSleep(PfxUInt8 b) {m_useSleep=b;}

	// [SMS_CHANGE] enables/disables SCE_PFX_ENABLE_SKIP_COLLISION for this body. If enabled on both bodies in a collision pair,
	// if the bodies' broadphase proxies haven't changed and the bodies' speeds are low, skips narrow phase collision detection
	// and reuses the previous frame manifold.
	PfxBool getAllowSkippingNarrowPhaseDetection() const			{ return m_allowSkippingNarrowPhaseDetection == 1; }		// [SMS_CHANGE]
	void	setAllowSkippingNarrowPhaseDetection( PfxBool allow )	{ m_allowSkippingNarrowPhaseDetection = allow ? 1 : 0; }	// [SMS_CHANGE]
	// [SMS_CHANGE] end

	void	 	incrementSleepCount() {if(m_sleepCount<0xffff)m_sleepCount++;}
	void		resetSleepCount() {m_sleepCount=0;}
	void		setSleepCount(const PfxUInt16 count){m_sleepCount = count;}
	PfxUInt16	getSleepCount() const {return m_sleepCount;}

	/// @brief Set for the root of joint chains
	void setAsJointRoot(PfxBool b){ m_isJointRoot = (b ? 1u : 0u); }

	/// @brief Check if this is the root of joint chains
	PfxBool isJointRoot(){ return m_isJointRoot == 1; }
	
	/// @brief Solver quality
	void setSolverQuality(PfxUInt8 q) { if (q < kPfxSolverQualityCount) m_solverQuality = q; }
	PfxUInt8 getSolverQuality() const { return m_solverQuality; }

	/// @brief Get linear damping
	PfxFloat getLinearDamping() const {return m_linearDamping;}

	/// @brief Set linear damping
	void setLinearDamping(PfxFloat damping) {m_linearDamping=damping;}

	/// @brief Get angular damping
	PfxFloat getAngularDamping() const {return m_angularDamping;}

	/// @brief Set angular damping
	void setAngularDamping(PfxFloat damping) {m_angularDamping=damping;}

	/// @brief Get maximum linear velocity
	PfxFloat getMaxLinearVelocity() const {return m_maxLinearVelocity;}

	/// @brief Set maximum linear velocity (Velocity will be clamped if it exceeds this value.)
	void setMaxLinearVelocity(PfxFloat maxvelocity){m_maxLinearVelocity = maxvelocity;}

	/// @brief Get maximum angular velocity
	PfxFloat getMaxAngularVelocity() const {return m_maxAngularVelocity;}

	/// @brief Set maximum angular velocity (Velocity will be clamped if it exceeds this value.)
	void setMaxAngularVelocity(PfxFloat maxvelocity){m_maxAngularVelocity = maxvelocity;}

	/// @brief Get offset position of the rigid body
	PfxVector3 getPosition() const {return m_position.offset;}

	/// @brief Set offset position of the rigid body
	void setPosition(const PfxVector3 &offset) { SCE_PFX_VALIDATE_VECTOR3(offset); m_position.offset = offset; }

	/// @brief Get segment position of the rigid body
	PfxSegment getSegment() const {return m_position.segment;}

	/// @brief Set segment position of the rigid body
	void setSegment(const PfxSegment &segment) {m_position.segment = segment;}

	/// @brief Get large position of the rigid body
	PfxLargePosition getLargePosition() const {return m_position;}

	/// @brief Set large position of the rigid body
	void setLargePosition(const PfxLargePosition &lpos) { SCE_PFX_VALIDATE_VECTOR3(lpos.offset); m_position = lpos;}

	/// @brief Get orientation of the rigid body
	PfxQuat    getOrientation() const {return m_orientation;}

	/// @brief Set orientation of the rigid body
	void setOrientation(const PfxQuat &rot) { SCE_PFX_VALIDATE_QUAT(rot);  m_orientation = rot; }

	/// @brief Get linear velocity of the rigid body
	PfxVector3 getLinearVelocity() const {return m_linearVelocity;}

	/// @brief Set linear velocity of the rigid body
	void setLinearVelocity(const PfxVector3 &vel) { SCE_PFX_VALIDATE_VECTOR3(vel); m_linearVelocity = vel;}

	/// @brief Get angular velocity of the rigid body
	PfxVector3 getAngularVelocity() const {return m_angularVelocity;}

	/// @brief Set angular velocity of the rigid body
	void setAngularVelocity(const PfxVector3 &vel) { SCE_PFX_VALIDATE_VECTOR3(vel); m_angularVelocity = vel;}

	/// @brief Change segment position and recalculate offset position
	void changeSegment(const PfxSegment &segment) {m_position.changeSegment(segment);}

	/// @brief Get user parameter
	PfxUInt32 getUserParam(int i) const {return m_userParam[i%4];}

	/// @brief Reset internal flags (call before the simulation pipeline)
	void resetFlags() {m_proxyShift = 0;}

	/// @brief Set user parameter
	void setUserParam(int i,PfxUInt32 param) {m_userParam[i%4]=param;}

	/// @brief Get user pointer
	void *getUserData() const {return m_userData;}

	/// @brief Set user pointer
	void setUserData(void *ptr) {m_userData = ptr;}

	/// @brief Calculate velocity to move the rigid body to the specified position
	void movePosition(const PfxLargePosition &pos,PfxFloat timeStep);
	
	/// @brief Calculate velocity to move the rigid body to the specified orientation
	void moveOrientation(const PfxQuat &rot,PfxFloat timeStep);

	/// @brief Change position to the nearest segment position
	void updateSegment();

	/// @brief Set collision ignore group. The special group PfxUInt16(-1) is used to disable the functionality on a collidable instance.
    PfxUInt16 getCollisionIgnoreGroup(int i) const
	{
		SCE_PFX_ASSERT(i<2);
		return m_collisionIgnoreGroup[i];
	}
	
	/// @brief Get collision ignore group. The special group PfxUInt16(-1) is used to disable the functionality on a collidable instance.
	void setCollisionIgnoreGroup(int i,const PfxUInt16 group)
	{
		SCE_PFX_ASSERT(i<2);
		m_collisionIgnoreGroup[i] = group;
	}

	void check();
};

#ifdef _MSC_VER
#pragma warning(pop) // [SMS_CHANGE] disable compiler warnings
#endif

} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_RIGID_STATE_H

