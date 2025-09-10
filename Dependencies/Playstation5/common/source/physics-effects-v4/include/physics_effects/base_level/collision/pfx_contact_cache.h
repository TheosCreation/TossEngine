/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CONTACT_CACHE_H
#define _SCE_PFX_CONTACT_CACHE_H

#include "../base/pfx_common.h"
#include "../base/pfx_simd_utils.h"
#include "../collision/pfx_sub_data.h"
#include "pfx_contact_manifold.h"

namespace sce {
namespace pfxv4 {

/*
	衝突結果保持用の軽量コンタクトキャッシュ
*/

///////////////////////////////////////////////////////////////////////////////
// Contact Point

struct SCE_PFX_API PfxCachedContactPoint
{
	PfxFloat m_distance;
	PfxSubData m_subDataA;
	PfxSubData m_subDataB;
#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
	PfxUInt64 m_featureIdA;
	PfxUInt64 m_featureIdB;
#endif
	PfxVector3 m_normal;
	PfxPoint3 m_localPointA;
	PfxPoint3 m_localPointB;

	void reset()
	{
		m_subDataA = m_subDataB = PfxSubData();
		m_distance = SCE_PFX_FLT_MAX;
#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
		m_featureIdA = m_featureIdB = 0;
#endif
	}
};

///////////////////////////////////////////////////////////////////////////////
// Contact Point

class SCE_PFX_API PfxContactCache
{
private:
	PfxUInt32 m_numContactPoints;
	PfxUInt32 m_numClosestPoints;
	PfxCachedContactPoint m_cachedContactPoints[SCE_PFX_MAX_CACHED_CLOSEST_POINTS];
	PfxCachedContactPoint m_cachedClosestPoints[SCE_PFX_MAX_CACHED_CLOSEST_POINTS];
	
	int findNearestContactPoint(const PfxPoint3 &newPointA,const PfxPoint3 &newPointB,const PfxVector3 &newNormal)
	{
		int nearestIdx = -1;
		const PfxFloat samePointDistance = 0.01f;
		PfxFloat minDiff = samePointDistance * samePointDistance;
		for(PfxUInt32 i=0;i<m_numContactPoints;i++) {
			PfxFloat diffA = lengthSqr(m_cachedContactPoints[i].m_localPointA-newPointA);
			PfxFloat diffB = lengthSqr(m_cachedContactPoints[i].m_localPointB-newPointB);
			if(diffA < minDiff && diffB < minDiff) {
				minDiff = SCE_PFX_MAX(diffA,diffB);
				nearestIdx = i;
			}
		}
		return nearestIdx;
	}

#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
	int findContactPointWithSameFeature(PfxUInt64 featureIdA, PfxUInt64 featureIdB)
	{
		for(PfxUInt32 i=0;i<m_numContactPoints;i++) {
			if(m_cachedContactPoints[i].m_featureIdA == featureIdA && m_cachedContactPoints[i].m_featureIdB == featureIdB) {
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

	PfxContactCache() { reset(); }

	void reset();
	
	// For contact points
	void addContactPoint(
		PfxFloat newDistance,
		const PfxVector3 &newNormal, // world normal vector
		const PfxPoint3 &newPointA, // local contact point to the objectA
		const PfxPoint3 &newPointB, // local contact point to the objectB
		PfxUInt64 featureIdA,
		PfxUInt64 featureIdB,
		PfxSubData subDataA,
		PfxSubData subDataB);

	void addContactPoint(
		PfxFloat newDistance,
		const PfxVector3 &newNormal, // world normal vector
		const PfxPoint3 &newPointA, // local contact point to the objectA
		const PfxPoint3 &newPointB, // local contact point to the objectB
		PfxSubData subDataA,
		PfxSubData subDataB)
	{
		addContactPoint(newDistance, newNormal, newPointA, newPointB, 0x80000000, 0x80000000, subDataA, subDataB);
	}
	
	void addContactPoint(const PfxCachedContactPoint &cp);
	
	PfxUInt32 getNumContactPoints() const {return m_numContactPoints;}
	
	PfxCachedContactPoint &getContactPoint(int i) {return m_cachedContactPoints[i];}
	const PfxCachedContactPoint &getContactPoint(int i) const {return m_cachedContactPoints[i];}

	PfxFloat getContactDistance(int i) {return m_cachedContactPoints[i].m_distance;}
	const PfxVector3 &getContactNormal(int i) const {return m_cachedContactPoints[i].m_normal;}
	const PfxPoint3 &getContactLocalPointA(int i) const {return m_cachedContactPoints[i].m_localPointA;}
	const PfxPoint3 &getContactLocalPointB(int i) const {return m_cachedContactPoints[i].m_localPointB;}
	const PfxSubData &getContactSubDataA(int i) const {return m_cachedContactPoints[i].m_subDataA;}
	const PfxSubData &getContactSubDataB(int i) const {return m_cachedContactPoints[i].m_subDataB;}

#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
	PfxUInt64 getFeatureIdA(int i) const { return m_cachedContactPoints[i].m_featureIdA; }
	PfxUInt64 getFeatureIdB(int i) const { return m_cachedContactPoints[i].m_featureIdB; }
#else
	PfxUInt64 getFeatureIdA(int i) const { return 0; }
	PfxUInt64 getFeatureIdB(int i) const { return 0; }
#endif

	// For closest points
	void addClosestPoint(
		PfxFloat newDistance,
		const PfxVector3 &newNormal, // world normal vector
		const PfxPoint3 &newPointA, // local contact point to the objectA
		const PfxPoint3 &newPointB, // local contact point to the objectB
		PfxSubData subDataA,
		PfxSubData subDataB);
	
	int getNumClosestPoints() const {return (int)m_numClosestPoints;}
	
	PfxCachedContactPoint &getClosestPoint(int i) {return m_cachedClosestPoints[i];}
	const PfxCachedContactPoint &getClosestPoint(int i) const {return m_cachedClosestPoints[i];}
	
	PfxFloat getClosestDistance(int i) {return m_cachedClosestPoints[i].m_distance;}
	const PfxVector3 &getClosestNormal(int i) const {return m_cachedClosestPoints[i].m_normal;}
	const PfxPoint3 &getClosestLocalPointA(int i) const {return m_cachedClosestPoints[i].m_localPointA;}
	const PfxPoint3 &getClosestLocalPointB(int i) const {return m_cachedClosestPoints[i].m_localPointB;}
	const PfxSubData &getClosestSubDataA(int i) const {return m_cachedClosestPoints[i].m_subDataA;}
	const PfxSubData &getClosestSubDataB(int i) const {return m_cachedClosestPoints[i].m_subDataB;}
};

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_CONTACT_CACHE_H
