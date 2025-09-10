/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_CONTACT_MANIFOLD_H
#define _SCE_PFX_CONTACT_MANIFOLD_H

#include "../base/pfx_common.h"
#include "../base/pfx_simd_utils.h"
#include "pfx_sub_data.h"
#include "../solver/pfx_constraint_row.h"

namespace sce {
namespace pfxv4 {

#define SCE_PFX_CONTACT_SAME_POINT			0.01f
#define SCE_PFX_CONTACT_THRESHOLD_NORMAL	0.01f	// 衝突点の閾値（法線方向）
#define SCE_PFX_CONTACT_THRESHOLD_TANGENT	0.002f	// 衝突点の閾値（平面上）

// The number of cached contact points
#define SCE_PFX_MAX_CACHED_CONTACT_POINTS 4

// The number of cached closest points
// It depends on how many core spheres are inserted into a collidable.
// It should be increased if more core spheres are inserted.
#define SCE_PFX_MAX_CACHED_CLOSEST_POINTS 32

///////////////////////////////////////////////////////////////////////////////
// Contact Point

/// @brief Contact point
struct SCE_PFX_API PfxContactPoint
{
	PfxSubData m_subDataA; ///< Sub data A
	PfxSubData m_subDataB; ///< Sub data B
#ifdef	SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
	PfxUInt64 m_featureIdA; ///< Feature A
	PfxUInt64 m_featureIdB; ///< Feature B
#endif
	PfxFloat m_distance; ///< Distance
	PfxFloat m_localPointA[3]; ///< Position in the local coordinate of A
	PfxFloat m_localPointB[3]; ///< Position in the local coordinate of B
	PfxConstraintRow m_constraintRow[3]; ///< Constraints

	/// @brief Reset parameters
	void reset()
	{
		m_distance = SCE_PFX_FLT_MAX;
		m_subDataA = m_subDataB = PfxSubData();
#ifdef	SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
		m_featureIdA = m_featureIdB = 0;
#endif
		m_constraintRow[0].m_accumImpulse = 0.0f;
		m_constraintRow[1].m_accumImpulse = 0.0f;
		m_constraintRow[2].m_accumImpulse = 0.0f;
	}

	/// @brief Get the normal vector in the world coordinate
	inline PfxVector3 getNormal() const {return m_constraintRow[0].getNormal();}
};

///////////////////////////////////////////////////////////////////////////////
// Closest Point

/// @brief Closest point
struct SCE_PFX_API PfxClosestPoint
{
	PfxSubData m_subDataA; ///< Sub data A
	PfxSubData m_subDataB; ///< Sub data B
	PfxFloat m_distance; ///< Distance
	PfxFloat m_localPointA[3]; ///< Position in the local coordinate of A
	PfxFloat m_localPointB[3]; ///< Position in the local coordinate of B
	PfxConstraintRow m_constraintRow; ///< Constraint

	/// @brief Reset parameters
	void reset()
	{
		m_subDataA = m_subDataB = PfxSubData();
		m_distance = SCE_PFX_FLT_MAX;
		m_constraintRow.m_accumImpulse = 0.0f;
	}

	/// @brief Get the normal vector in the world coordinate
	inline PfxVector3 getNormal() const {return m_constraintRow.getNormal();}
};

///////////////////////////////////////////////////////////////////////////////
// Rolling Friction

struct SCE_PFX_API PfxRollingFriction {
	PfxConstraintRow m_constraint;
	PfxFloat  m_compositeFriction;
};

///////////////////////////////////////////////////////////////////////////////
// Contact Manifold

//J	同一ペアの衝突が続く限り保持されるコンタクト情報
//E PfxContactManifold keeps contact information while two rigid bodies are touching.

/// @brief Contact information
class SCE_PFX_API PfxContactManifold
{
friend class PfxContactManager;
friend class PfxContactHolder;

private:
	PfxUInt16 m_shapeId; // both shapeId A and B are combined
	PfxUInt16 m_duration;
	PfxUInt8 m_numContactPoints;
	PfxUInt8 m_numClosestPoints;
	PfxContactPoint m_contactPoints[SCE_PFX_MAX_CACHED_CONTACT_POINTS];
	PfxClosestPoint *m_closestPoints;
	PfxContactManifold *m_next;

	int findNearestContactPoint(const PfxPoint3 &newPointA,const PfxPoint3 &newPointB,const PfxVector3 &newNormal)
	{
		int nearestIdx = -1;
		PfxFloat minDiff = SCE_PFX_CONTACT_SAME_POINT * SCE_PFX_CONTACT_SAME_POINT;
		for(int i=0;i<m_numContactPoints;i++) {
			PfxFloat diffA = lengthSqr(pfxReadVector3(m_contactPoints[i].m_localPointA) - PfxVector3(newPointA));
			PfxFloat diffB = lengthSqr(pfxReadVector3(m_contactPoints[i].m_localPointB) - PfxVector3(newPointB));
			if(diffA < minDiff && diffB < minDiff) {
				minDiff = SCE_PFX_MAX(diffA,diffB);
				nearestIdx = i;
			}
		}
		return nearestIdx;
	}

#ifdef	SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
	int findContactPointWithSameFeature(PfxUInt64 featureIdA, PfxUInt64 featureIdB)
	{
		for(PfxUInt32 i=0;i<m_numContactPoints;i++) {
			if(m_contactPoints->cp[i].m_featureIdA == featureIdA && m_contactPoints->cp[i].m_featureIdB == featureIdB) {
				return i;
			}
		}
		return -1;
	}
#endif

	int sort4ContactPoints(const PfxPoint3 &newPoint,PfxFloat newDistance);
	int sort5ContactPoints(const PfxPoint3 &newPoint, PfxFloat newDistance);

public:
	PfxVector3 cachedAxis;

	// Internal method
	void setClosestPointBuffer(PfxClosestPoint *buff,PfxUInt32 num)
	{
		m_closestPoints = buff;
		m_numClosestPoints = 0;
	}

public:
	PfxUInt8 getShapeIdA() {return m_shapeId & 0xFF;}
	PfxUInt8 getShapeIdB() {return m_shapeId >> 8;}

	// @brief Copy parameters
	void copy(const PfxContactManifold &src)
	{
		m_shapeId = src.m_shapeId;
		m_duration = src.m_duration;
		m_numContactPoints = src.m_numContactPoints;
		m_numClosestPoints = src.m_numClosestPoints;
		m_contactPoints[0] = src.m_contactPoints[0];
		m_contactPoints[1] = src.m_contactPoints[1];
		m_contactPoints[2] = src.m_contactPoints[2];
		m_contactPoints[3] = src.m_contactPoints[3];
	}

	/// @brief Reset parameters
	void reset(PfxUInt8 shapeIdA, PfxUInt8 shapeIdB)
	{
		cachedAxis = PfxVector3::zero();
		m_numContactPoints = 0;
		m_duration = 0;
		m_shapeId = ((PfxUInt16)shapeIdB << 8) | (PfxUInt16)shapeIdA;
		m_numClosestPoints = 0;
		m_closestPoints = nullptr;
		m_next = nullptr;
	}

	/// @brief Add a contact point
	void addContactPoint(
		PfxFloat newDistance,
		const PfxVector3 &newNormal, // world coords
		const PfxPoint3 &newPointA, // local coords to the objectA
		const PfxPoint3 &newPointB, // local coords to the objectB
		PfxUInt64 featureIdA,
		PfxUInt64 featureIdB,
		PfxSubData subDataA,
		PfxSubData subDataB);

	void addContactPoint(
		PfxFloat newDistance,
		const PfxVector3 &newNormal, // world coords
		const PfxPoint3 &newPointA, // local coords to the objectA
		const PfxPoint3 &newPointB, // local coords to the objectB
		PfxSubData subDataA,
		PfxSubData subDataB)
	{
		addContactPoint(newDistance, newNormal, newPointA, newPointB, 0x80000000, 0x80000000, subDataA, subDataB);
	}

	/// @brief Add a contact point
	void addContactPoint(const PfxContactPoint &cp);

	/// @brief Remove a contact point
	void removeContactPoint(int i)
	{
		SCE_PFX_ASSERT(i>=0&&i<m_numContactPoints);
		pfxSwap(m_contactPoints[i], m_contactPoints[m_numContactPoints - 1]);
		m_numContactPoints--;
	}

	/// @brief Get the number of contact points
	PfxUInt32 getNumContactPoints() const {return (PfxUInt32)m_numContactPoints;}

	/// @brief Get a contact point
	PfxContactPoint &getContactPoint(int i) {return m_contactPoints[i];}

	/// @brief Get a contact point
	const PfxContactPoint &getContactPoint(int i) const {return m_contactPoints[i];}

	/// @brief Refresh contact points
	void refresh()
	{
		if (m_numContactPoints > 0 && m_duration < 65535) m_duration++;
		if (m_numContactPoints == 0) m_duration = 0;
		m_numClosestPoints = 0;
		m_closestPoints = NULL;
	}

	void refresh(const PfxVector3 &pA,const PfxQuat &qA,const PfxVector3 &pB,const PfxQuat &qB);

	/// @brief Merge contact information
	void merge(const PfxContactManifold &contact);

	/// @brief Get duration count
	PfxUInt16 getDuration() const {return m_duration;}

	/// @brief Add a closest point
	void addClosestPoint(
		PfxFloat newDistance,
		const PfxVector3 &newNormal,
		const PfxPoint3 &newPointA,
		const PfxPoint3 &newPointB,
		PfxSubData subDataA,
		PfxSubData subDataB);

	/// @brief Get the number of closest points
	PfxUInt32 getNumClosestPoints() const {return m_numClosestPoints;}

	/// @brief Get a closest point
	PfxClosestPoint &getClosestPoint(PfxUInt32 i)
	{
		SCE_PFX_ASSERT(i<m_numClosestPoints);
		return m_closestPoints[i];
	}

	/// @brief Get a closest point
	const PfxClosestPoint &getClosestPoint(PfxUInt32 i) const
	{
		SCE_PFX_ASSERT(i<m_numClosestPoints);
		return m_closestPoints[i];
	}

	/// @brief Clear closest points
	void clearClosestPoints()
	{
		m_numClosestPoints = 0;
	}

	PfxBool isEmpty() const
	{
		return m_numClosestPoints == 0 && m_numContactPoints == 0;
	}
};


///////////////////////////////////////////////////////////////////////////////
// Contact

class SCE_PFX_API PfxContactHolder
{
friend class PfxContactManager;

private:
	PfxUInt32 m_userData;
	PfxUInt16 m_duration;
	PfxUInt16 m_rigidBodyIdA;
	PfxUInt16 m_rigidBodyIdB;
	PfxFloat  m_compositeFriction;
	PfxFloat  m_compositeRestitution;
	PfxRollingFriction m_rollingFriction;
	PfxContactManifold *m_contactManifolds;
	bool m_narrowPhaseDetctionSkipped; // [SMS_CHANGE] report if narrow-phase detection was skipped due to SCE_PFX_ENABLE_SKIP_COLLISION

public:
	// Internal method
	PfxFloat getCompositeRollingFriction() const { return m_rollingFriction.m_compositeFriction; }
	void setCompositeRollingFriction(PfxFloat f) { m_rollingFriction.m_compositeFriction = f; }

	PfxFloat getCompositeFriction() const { return m_compositeFriction; }
	void setCompositeFriction(PfxFloat f) { m_compositeFriction = f; }

	PfxFloat getCompositeRestitution() const { return m_compositeRestitution; }
	void setCompositeRestitution(PfxFloat f) { m_compositeRestitution = f; }

	void setNarrowPhaseDetctionSkipped(bool skipped) {m_narrowPhaseDetctionSkipped = skipped;} // [SMS_CHANGE]

public:
	/// @brief Copy parameters
	void copy(const PfxContactHolder &src)
	{
		m_userData = src.m_userData;
		m_duration = src.m_duration;
		m_rigidBodyIdA = src.m_rigidBodyIdA;
		m_rigidBodyIdB = src.m_rigidBodyIdB;
		m_compositeFriction = src.m_compositeFriction;
		m_compositeRestitution = src.m_compositeRestitution;
		m_rollingFriction = src.m_rollingFriction;
		m_narrowPhaseDetctionSkipped = src.m_narrowPhaseDetctionSkipped;
	}

	/// @brief Reset parameters
	void reset(PfxUInt16 rigidBodyIdA, PfxUInt16 rigidBodyIdB)
	{
		m_userData = 0;
		m_duration = 0;
		m_rigidBodyIdA = rigidBodyIdA;
		m_rigidBodyIdB = rigidBodyIdB;
		m_contactManifolds = nullptr;
		m_compositeFriction = -1.0f;
		m_compositeRestitution = -1.0f;
		m_rollingFriction.m_compositeFriction = -1.0f;
		m_rollingFriction.m_constraint.m_accumImpulse = 0.0f;
		m_narrowPhaseDetctionSkipped = false; // [SMS_CHANGE]
	}

	/// @brief Refresh contact points
	inline void refresh();
	inline void refresh(const PfxVector3 &pA, const PfxQuat &qA, const PfxVector3 &pB, const PfxQuat &qB);

	/// @brief Get duration count
	PfxUInt16 getDuration() const { return m_duration; }

	/// @brief Get rigid body A ID
	PfxUInt32 getRigidBodyIdA() const { return m_rigidBodyIdA; }

	/// @brief Get rigid body B ID
	PfxUInt32 getRigidBodyIdB() const { return m_rigidBodyIdB; }

	inline PfxUInt32 getTotalContactAndClosestPoints() const;
	inline PfxUInt32 getTotalContactPoints() const;
	inline PfxUInt32 getTotalClosestPoints() const;

	PfxBool isEmpty() const
	{
		return !m_contactManifolds->m_next && m_contactManifolds->isEmpty();
	}

	/// @brief Get a rolling friction constraint
	PfxConstraintRow &getRollingFrictionConstraint()
	{
		return m_rollingFriction.m_constraint;
	}

	/// @brief Get a rolling friction constraint
	const PfxConstraintRow &getRollingFrictionConstraint() const
	{
		return m_rollingFriction.m_constraint;
	}

	PfxContactManifold *findContactManifold(PfxUInt8 shapeIdA, PfxUInt8 shapeIdB);

	PfxContactManifold *findFirstContactManifold() { return m_contactManifolds; }
	const PfxContactManifold *findFirstContactManifold() const { return m_contactManifolds; }

	PfxContactManifold *findNextContactManifold(const PfxContactManifold *contactManifold) { return contactManifold->m_next;}
	const PfxContactManifold *findNextContactManifold(const PfxContactManifold *contactManifold) const { return contactManifold->m_next; }

	bool getNarrowPhaseDetctionSkipped() const { return m_narrowPhaseDetctionSkipped; } // [SMS_CHANGE]
};

inline void PfxContactHolder::refresh(const PfxVector3 &pA, const PfxQuat &qA, const PfxVector3 &pB, const PfxQuat &qB)
{
	if (m_duration < 0xff) m_duration++;
	PfxContactManifold *cm = m_contactManifolds;
	while (cm) {
		cm->refresh(pA, qA, pB, qB);
		cm = cm->m_next;
	}
}

inline void PfxContactHolder::refresh()
{
	if (m_duration < 0xff) m_duration++;
	PfxContactManifold *cm = m_contactManifolds;
	while (cm) {
		cm->refresh();
		cm = cm->m_next;
	}
}

inline PfxUInt32 PfxContactHolder::getTotalContactAndClosestPoints() const
{
	PfxUInt32 total = 0;
	PfxContactManifold *cm = m_contactManifolds;
	total += cm->getNumClosestPoints();
	
	while(cm) {
		total += cm->getNumContactPoints();
		cm = cm->m_next;
	}
	return total;
}

inline PfxUInt32 PfxContactHolder::getTotalContactPoints() const
{
	PfxUInt32 total = 0;
	PfxContactManifold *cm = m_contactManifolds;

	while(cm) {
		total += cm->getNumContactPoints();
		cm = cm->m_next;
	}
	return total;
}

inline PfxUInt32 PfxContactHolder::getTotalClosestPoints() const
{
	return m_contactManifolds->getNumClosestPoints();;
}

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_CONTACT_MANIFOLD_H
