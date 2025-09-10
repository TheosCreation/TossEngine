/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_contact_cache.h"
#include "pfx_intersect_common.h"

namespace sce {
	namespace pfxv4 {

void PfxContactCache::reset()
{
	cachedAxis = PfxVector3::zero();
	m_numContactPoints = m_numClosestPoints = 0;
}

static inline PfxFloat calcArea4Points(const PfxVector3 &p0,const PfxVector3 &p1,const PfxVector3 &p2,const PfxVector3 &p3)
{
	PfxVector3 a[3],b[3];
	a[0] = p0 - p1;
	a[1] = p0 - p2;
	a[2] = p0 - p3;
	b[0] = p2 - p3;
	b[1] = p1 - p3;
	b[2] = p1 - p2;
#if 1
	PfxMatrix3 xyzA(a[0], a[1], a[2]);
	PfxMatrix3 xyzB(b[0], b[1], b[2]);
	xyzA = transpose(xyzA);
	xyzB = transpose(xyzB);
	PfxVector3 tmpX = mulPerElem(xyzA[1],xyzB[2])-mulPerElem(xyzA[2],xyzB[1]);
	PfxVector3 tmpY = mulPerElem(xyzA[2],xyzB[0])-mulPerElem(xyzA[0],xyzB[2]);
	PfxVector3 tmpZ = mulPerElem(xyzA[0],xyzB[1])-mulPerElem(xyzA[1],xyzB[0]);
#else
	PfxVector3 Ax(a[0].getX(),a[1].getX(),a[2].getX());
	PfxVector3 Ay(a[0].getY(),a[1].getY(),a[2].getY());
	PfxVector3 Az(a[0].getZ(),a[1].getZ(),a[2].getZ());
	PfxVector3 Bx(b[0].getX(),b[1].getX(),b[2].getX());
	PfxVector3 By(b[0].getY(),b[1].getY(),b[2].getY());
	PfxVector3 Bz(b[0].getZ(),b[1].getZ(),b[2].getZ());
	PfxVector3 tmpX = mulPerElem(Ay,Bz)-mulPerElem(Az,By);
	PfxVector3 tmpY = mulPerElem(Az,Bx)-mulPerElem(Ax,Bz);
	PfxVector3 tmpZ = mulPerElem(Ax,By)-mulPerElem(Ay,Bx);
#endif
	PfxVector3 area = mulPerElem(tmpX,tmpX) + mulPerElem(tmpY,tmpY) + mulPerElem(tmpZ,tmpZ);
	return maxElem(area);
}

int PfxContactCache::sort4ContactPoints(const PfxPoint3 &newCP,PfxFloat newDistance)
{
	int maxPenetrationIndex = -1;
	PfxFloat maxPenetration = newDistance;

	// 最も深い衝突点は排除対象からはずす
	for(PfxUInt32 i=0;i<m_numContactPoints;i++) {
		if(m_cachedContactPoints[i].m_distance < maxPenetration) {
			maxPenetrationIndex = i;
			maxPenetration = m_cachedContactPoints[i].m_distance;
		}
	}

	PfxFloat res[4] = {0.0f};

	// 各点を除いたときの衝突点が作る面積のうち、最も大きくなるものを選択
	PfxVector3 newp(newCP);
	PfxVector3 p[4];
	p[0] = (PfxVector3)m_cachedContactPoints[0].m_localPointA;
	p[1] = (PfxVector3)m_cachedContactPoints[1].m_localPointA;
	p[2] = (PfxVector3)m_cachedContactPoints[2].m_localPointA;
	p[3] = (PfxVector3)m_cachedContactPoints[3].m_localPointA;

	if(maxPenetrationIndex != 0) {
		res[0] = calcArea4Points(newp,p[1],p[2],p[3]);
	}

	if(maxPenetrationIndex != 1) {
		res[1] = calcArea4Points(newp,p[0],p[2],p[3]);
	}

	if(maxPenetrationIndex != 2) {
		res[2] = calcArea4Points(newp,p[0],p[1],p[3]);
	}

	if(maxPenetrationIndex != 3) {
		res[3] = calcArea4Points(newp,p[0],p[1],p[2]);
	}

	int maxIndex = 0;
	PfxFloat maxVal = res[0];

	if (res[1] > maxVal) {
		maxIndex = 1;
		maxVal = res[1];
	}

	if (res[2] > maxVal) {
		maxIndex = 2;
		maxVal = res[2];
	}

	if (res[3] > maxVal) {
		maxIndex = 3;
		maxVal = res[3];
	}

	return maxIndex;
}

int PfxContactCache::sort5ContactPoints(const PfxPoint3 &newCP, PfxFloat newDistance)
{
	int maxPenetrationIndex = -1;
	PfxFloat maxPenetration = newDistance;

	// 最も深い衝突点は排除対象からはずす
	for (PfxUInt32 i = 0; i<m_numContactPoints; i++) {
		if (m_cachedContactPoints[i].m_distance < maxPenetration) {
			maxPenetrationIndex = i;
			maxPenetration = m_cachedContactPoints[i].m_distance;
		}
	}

	PfxFloat res[5] = { 0.0f };

	// 各点を除いたときの衝突点が作る面積のうち、最も大きくなるものを選択
	PfxVector3 newp(newCP);
	PfxVector3 p[4];
	p[0] = (PfxVector3)m_cachedContactPoints[0].m_localPointA;
	p[1] = (PfxVector3)m_cachedContactPoints[1].m_localPointA;
	p[2] = (PfxVector3)m_cachedContactPoints[2].m_localPointA;
	p[3] = (PfxVector3)m_cachedContactPoints[3].m_localPointA;

	if (maxPenetrationIndex != 0) {
		res[0] = calcArea4Points(newp, p[1], p[2], p[3]);
	}

	if (maxPenetrationIndex != 1) {
		res[1] = calcArea4Points(newp, p[0], p[2], p[3]);
	}

	if (maxPenetrationIndex != 2) {
		res[2] = calcArea4Points(newp, p[0], p[1], p[3]);
	}

	if (maxPenetrationIndex != 3) {
		res[3] = calcArea4Points(newp, p[0], p[1], p[2]);
	}

	res[4] = calcArea4Points(p[0], p[1], p[2], p[3]);

	int maxIndex = 0;
	PfxFloat maxVal = res[0];

	if (res[1] > maxVal) {
		maxIndex = 1;
		maxVal = res[1];
	}

	if (res[2] > maxVal) {
		maxIndex = 2;
		maxVal = res[2];
	}

	if (res[3] > maxVal) {
		maxIndex = 3;
		maxVal = res[3];
	}

	if (res[4] > maxVal) {
		maxIndex = -1;
		maxVal = res[4];
	}

	return maxIndex;
}

void PfxContactCache::addContactPoint(
		PfxFloat newDistance,
		const PfxVector3 &newNormal, // world coords
		const PfxPoint3 &newPointA, // local to the objectA
		const PfxPoint3 &newPointB, // local to the objectB
		PfxUInt64 featureIdA,
		PfxUInt64 featureIdB,
		PfxSubData subDataA,
		PfxSubData subDataB)
{
//#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
//	int id = -1;
//	if(((featureIdA & 0x80000000) | (featureIdB & 0x80000000)) == 0) {
//		id = findContactPointWithSameFeature(featureIdA, featureIdB);
//	}
//	else {
//		id = findNearestContactPoint(newPointA,newPointB,newNormal);
//	}
//#else
	int id = findNearestContactPoint(newPointA, newPointB, newNormal);
//#endif

	PfxVector3 newNormal_ = newNormal;
	PfxFloat newDistance_ = newDistance;

	if(id < 0 && m_numContactPoints < SCE_PFX_MAX_CACHED_CONTACT_POINTS) {
		// 衝突点を新規追加
		id = m_numContactPoints++;
		m_cachedContactPoints[id].reset();
	}
	else if(id < 0){
		// ソート
		id = sort5ContactPoints(newPointA,newDistance);
		if (id < 0) return;
		m_cachedContactPoints[id].reset();
	}
	else {
		// 同一頂点
		PfxFloat dotNml = dot(newNormal, m_cachedContactPoints[id].m_normal);
		dotNml *= dotNml;
		if (dotNml < 0.9f) {
			newNormal_ = normalize(newNormal + m_cachedContactPoints[id].m_normal);
			newDistance_ = dot(newNormal, newNormal_) * newDistance;
		}
	}

	m_cachedContactPoints[id].m_distance = newDistance_;
	m_cachedContactPoints[id].m_subDataA = subDataA;
	m_cachedContactPoints[id].m_subDataB = subDataB;
#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
	m_cachedContactPoints[id].m_featureIdA = featureIdA;
	m_cachedContactPoints[id].m_featureIdB = featureIdB;
#endif
	m_cachedContactPoints[id].m_normal = newNormal_;
	m_cachedContactPoints[id].m_localPointA = newPointA;
	m_cachedContactPoints[id].m_localPointB = newPointB;
}

void PfxContactCache::addContactPoint(const PfxCachedContactPoint &cp)
{
	PfxPoint3 pA = cp.m_localPointA;
	PfxPoint3 pB = cp.m_localPointB;

	// PfxContactCacheは同一形状ペアをキャッシュしないので、Featureは違う
	// 距離判定で近いものをまとめるようにする
//#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
//	int id = -1;
//	if(((cp.m_featureIdA & 0x80000000) | (cp.m_featureIdB & 0x80000000)) == 0) {
//		id = findContactPointWithSameFeature(cp.m_featureIdA, cp.m_featureIdB);
//	}
//	else {
//		id = findNearestContactPoint(pA,pB,cp.m_normal);
//	}
//#else
	int id = findNearestContactPoint(pA, pB, cp.m_normal);
//#endif

	if(id >= 0) {
		if(m_cachedContactPoints[id].m_distance > cp.m_distance) {
			// 同一点を発見、衝突点情報を更新
			PfxFloat dotNml = dot(cp.m_normal, m_cachedContactPoints[id].m_normal);
			dotNml *= dotNml;
			if (dotNml > 0.9f) {
				m_cachedContactPoints[id].m_normal = cp.m_normal;
				m_cachedContactPoints[id].m_distance = cp.m_distance;
			}
			else {
				m_cachedContactPoints[id].m_normal = normalize(cp.m_normal + m_cachedContactPoints[id].m_normal);
				m_cachedContactPoints[id].m_distance = dot(cp.m_normal, m_cachedContactPoints[id].m_normal) * cp.m_distance;
			}
			m_cachedContactPoints[id].m_localPointA = pA;
			m_cachedContactPoints[id].m_localPointB = pB;
		}
	}
	else if(m_numContactPoints < SCE_PFX_MAX_CACHED_CONTACT_POINTS) {
		// 衝突点を新規追加
		m_cachedContactPoints[m_numContactPoints++] = cp;
	}
	else {
		// ソート
		id = sort5ContactPoints(pA,cp.m_distance);
		if (id < 0) return;

		// コンタクトポイント入れ替え
		m_cachedContactPoints[id] = cp;
	}
}

void PfxContactCache::addClosestPoint(
		PfxFloat newDistance,
		const PfxVector3 &newNormal, // world coords
		const PfxPoint3 &newPointA, // local to the objectA
		const PfxPoint3 &newPointB, // local to the objectB
		PfxSubData subDataA,
		PfxSubData subDataB)
{
	int replaceId = -1;
	PfxUInt8 newPriorityA = subDataA.getCcdPriority();
	PfxUInt8 newPriorityB = subDataB.getCcdPriority();
	PfxUInt8 newPriority = SCE_PFX_MAX(newPriorityA, newPriorityB);
	bool isNewHit = (newPriorityA & 0x08) || (newPriorityB & 0x08);
	{
		int lowestPriority = 0xff;
		int lowestId = -1;
		PfxFloat minDiff = SCE_PFX_CONTACT_SAME_POINT * SCE_PFX_CONTACT_SAME_POINT;
		for(PfxUInt32 i=0;i<m_numClosestPoints;i++) {
			PfxFloat diffA = lengthSqr(m_cachedClosestPoints[i].m_localPointA - newPointA);
			PfxFloat diffB = lengthSqr(m_cachedClosestPoints[i].m_localPointB - newPointB);
			PfxFloat angle = dot(m_cachedClosestPoints[i].m_normal, newNormal);
			PfxUInt8 priorityA = m_cachedClosestPoints[i].m_subDataA.getCcdPriority();
			PfxUInt8 priorityB = m_cachedClosestPoints[i].m_subDataB.getCcdPriority();
			PfxUInt8 priority = SCE_PFX_MAX(priorityA, priorityB);
			//bool isHit = (priorityA & 0x08) || (priorityB & 0x08);

			if(diffA < minDiff && diffB < minDiff) {
				if (isNewHit) {
					replaceId = i;
					break;
				}
				else {
					return;
				}
			}
			
			auto isSameCoreSphere = [&](const PfxSubData &newSubData, const PfxSubData &cachedSubData)
			{
				PfxUInt8 newPrio = newSubData.getCcdPriority();
				return (newPrio & 0x10) && (newSubData.getShapeId() == cachedSubData.getShapeId());
			};

			if ((isSameCoreSphere(subDataA, m_cachedClosestPoints[i].m_subDataA) || isSameCoreSphere(subDataB, m_cachedClosestPoints[i].m_subDataB)) && angle > 0.85f) {
				if (newDistance < m_cachedClosestPoints[i].m_distance) {
					if (priority <= newPriority) {
						replaceId = i;
						break;
					}
					else {
						return;
					}
				}
			}

			if (priority < lowestPriority) {
				lowestPriority = priority;
				lowestId = i;
			}
		}

		if(replaceId < 0) {
			if (m_numClosestPoints < SCE_PFX_MAX_CACHED_CLOSEST_POINTS) {
				replaceId = m_numClosestPoints++;
			}
			else if(lowestPriority < newPriority) {
				replaceId = lowestId;
			}
		}
	}

	if(replaceId >= 0) {
		m_cachedClosestPoints[replaceId].reset();
		m_cachedClosestPoints[replaceId].m_distance = newDistance;
		m_cachedClosestPoints[replaceId].m_subDataA = subDataA;
		m_cachedClosestPoints[replaceId].m_subDataB = subDataB;
		m_cachedClosestPoints[replaceId].m_normal = newNormal;
		m_cachedClosestPoints[replaceId].m_localPointA = newPointA;
		m_cachedClosestPoints[replaceId].m_localPointB = newPointB;
	}
}

} //namespace pfxv4
} //namespace sce
