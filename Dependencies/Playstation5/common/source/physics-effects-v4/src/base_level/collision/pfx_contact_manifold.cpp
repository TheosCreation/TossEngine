/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_contact_manifold.h"

namespace sce {
namespace pfxv4 {

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

int PfxContactManifold::sort4ContactPoints(const PfxPoint3 &newCP,PfxFloat newDistance)
{
	int maxPenetrationIndex = -1;
	PfxFloat maxPenetration = newDistance;

	// 最も深い衝突点は排除対象からはずす
	for(int i=0;i<m_numContactPoints;i++) {
		if(m_contactPoints[i].m_distance < maxPenetration) {
			maxPenetrationIndex = i;
			maxPenetration = m_contactPoints[i].m_distance;
		}
	}

	PfxFloat res[4] = {0.0f};

	// 各点を除いたときの衝突点が作る面積のうち、最も大きくなるものを選択
	PfxVector3 newp(newCP);
	PfxVector3 p[4];
	p[0] = pfxReadVector3(m_contactPoints[0].m_localPointA);
	p[1] = pfxReadVector3(m_contactPoints[1].m_localPointA);
	p[2] = pfxReadVector3(m_contactPoints[2].m_localPointA);
	p[3] = pfxReadVector3(m_contactPoints[3].m_localPointA);

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

int PfxContactManifold::sort5ContactPoints(const PfxPoint3 &newCP, PfxFloat newDistance)
{
	int maxPenetrationIndex = -1;
	PfxFloat maxPenetration = newDistance;

	// 最も深い衝突点は排除対象からはずす
	for (int i = 0; i<m_numContactPoints; i++) {
		if (m_contactPoints[i].m_distance < maxPenetration) {
			maxPenetrationIndex = i;
			maxPenetration = m_contactPoints[i].m_distance;
		}
	}

	PfxFloat res[5] = { 0.0f };

	// 各点を除いたときの衝突点が作る面積のうち、最も大きくなるものを選択
	PfxVector3 newp(newCP);
	PfxVector3 p[4];
	p[0] = pfxReadVector3(m_contactPoints[0].m_localPointA);
	p[1] = pfxReadVector3(m_contactPoints[1].m_localPointA);
	p[2] = pfxReadVector3(m_contactPoints[2].m_localPointA);
	p[3] = pfxReadVector3(m_contactPoints[3].m_localPointA);

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

void PfxContactManifold::addContactPoint(
		PfxFloat newDistance,
		const PfxVector3 &newNormal, // world coords
		const PfxPoint3 &newPointA, // local to the objectA
		const PfxPoint3 &newPointB, // local to the objectB
		PfxUInt64 featureIdA,
		PfxUInt64 featureIdB,
		PfxSubData subDataA,
		PfxSubData subDataB)
{
#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
	int id = -1;
	if(((featureIdA & 0x80000000) | (featureIdB & 0x80000000)) == 0) {
		id = findContactPointWithSameFeature(featureIdA, featureIdB);
	}
	else {
		id = findNearestContactPoint(newPointA,newPointB,newNormal);
	}
#else
	int id = findNearestContactPoint(newPointA, newPointB, newNormal);
#endif

	PfxVector3 newNormal_ = newNormal;
	PfxFloat newDistance_ = newDistance;

	if(id < 0 && m_numContactPoints < SCE_PFX_MAX_CACHED_CONTACT_POINTS) {
		// 衝突点を新規追加
		id = m_numContactPoints++;
		m_contactPoints[id].reset();
	}
	else if(id < 0) {
		// ソート
		id = sort5ContactPoints(newPointA,newDistance);
		if (id < 0) return;
		m_contactPoints[id].reset();
	}
	else {
		PfxVector3 oldNormal(m_contactPoints[id].m_constraintRow[0].getNormal());
		PfxFloat dotNml = dot(newNormal,oldNormal);
		dotNml *= dotNml;
		if(dotNml < 0.9f) {
			newNormal_ = normalize(newNormal + oldNormal);
			newDistance_ = dot(newNormal, newNormal_) * newDistance;
		}
	}

	m_contactPoints[id].m_distance = newDistance_;
	m_contactPoints[id].m_subDataA = subDataA;
	m_contactPoints[id].m_subDataB = subDataB;
#ifdef	SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
	m_contactPoints[id].m_featureIdA = featureIdA;
	m_contactPoints[id].m_featureIdB = featureIdB;
#endif
	pfxStorePoint3(newPointA,m_contactPoints[id].m_localPointA);
	pfxStorePoint3(newPointB,m_contactPoints[id].m_localPointB);
	m_contactPoints[id].m_constraintRow[0].setNormal(newNormal_);
}

void PfxContactManifold::addContactPoint(const PfxContactPoint &cp)
{
	PfxPoint3 pA = pfxReadPoint3(cp.m_localPointA);
	PfxPoint3 pB = pfxReadPoint3(cp.m_localPointB);

#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
	int id = -1;
	if(((cp.m_featureIdA & 0x80000000) | (cp.m_featureIdB & 0x80000000)) == 0) {
		id = findContactPointWithSameFeature(cp.m_featureIdA, cp.m_featureIdB);
	}
	else {
		id = findNearestContactPoint(pA,pB,cp.m_constraintRow[0].getNormal());
	}
#else
	int id = findNearestContactPoint(pA, pB, cp.m_constraintRow[0].getNormal());
#endif

	if(id >= 0) {
#if 1
		PfxVector3 nml1(m_contactPoints[id].m_constraintRow[0].getNormal());
		PfxVector3 nml2(cp.m_constraintRow[0].getNormal());
		PfxFloat dotNml = dot(nml1,nml2);
		dotNml *= dotNml;
		if(dotNml > 0.99f ) {
			// 同一点を発見、蓄積された情報を継続
			m_contactPoints[id].m_distance = cp.m_distance;
			m_contactPoints[id].m_localPointA[0] = cp.m_localPointA[0];
			m_contactPoints[id].m_localPointA[1] = cp.m_localPointA[1];
			m_contactPoints[id].m_localPointA[2] = cp.m_localPointA[2];
			m_contactPoints[id].m_localPointB[0] = cp.m_localPointB[0];
			m_contactPoints[id].m_localPointB[1] = cp.m_localPointB[1];
			m_contactPoints[id].m_localPointB[2] = cp.m_localPointB[2];
			m_contactPoints[id].m_constraintRow[0].copyNormal(cp.m_constraintRow[0]);
		}
		else {
			// 同一点ではあるが法線が違うため更新
			PfxVector3 newNml = normalize(nml1 + nml2);
			m_contactPoints[id] = cp;
			m_contactPoints[id].m_distance = dot(nml2, newNml) * cp.m_distance;
			m_contactPoints[id].m_constraintRow[0].setNormal(newNml);
		}
#else
		if(m_contactPoints[id]->m_distance > cp.m_distance) {
			// 同一点を発見、衝突点情報を更新
			m_contactPoints[id]->m_distance = cp.m_distance;
			m_contactPoints[id]->m_localPointA[0] = cp.m_localPointA[0];
			m_contactPoints[id]->m_localPointA[1] = cp.m_localPointA[1];
			m_contactPoints[id]->m_localPointA[2] = cp.m_localPointA[2];
			m_contactPoints[id]->m_localPointB[0] = cp.m_localPointB[0];
			m_contactPoints[id]->m_localPointB[1] = cp.m_localPointB[1];
			m_contactPoints[id]->m_localPointB[2] = cp.m_localPointB[2];
			m_contactPoints[id]->m_constraintRow[0].copyNormal(cp.m_constraintRow[0]);
		}
#endif
	}
	else if(m_numContactPoints < SCE_PFX_MAX_CACHED_CONTACT_POINTS) {
		// 衝突点を新規追加
		m_contactPoints[m_numContactPoints++] = cp;
	}
	else {
		// ソート
		id = sort5ContactPoints(pA,cp.m_distance);
		if (id < 0) return;

		// コンタクトポイント入れ替え
		m_contactPoints[id] = cp;
	}
}

void PfxContactManifold::merge(const PfxContactManifold &contact)
{
	for(PfxUInt32 i=0;i<contact.getNumContactPoints();i++) {
		addContactPoint(contact.getContactPoint(i));
	}
}

void PfxContactManifold::refresh(const PfxVector3 &pA,const PfxQuat &qA,const PfxVector3 &pB,const PfxQuat &qB)
{
	// 衝突点の更新
	// 両衝突点間の距離が閾値（CONTACT_THRESHOLD）を超えたら消去
	for(int i=0;i<(int)m_numContactPoints;i++) {
		PfxVector3 normal = m_contactPoints[i].m_constraintRow[0].getNormal();
		PfxVector3 cpA = pA + rotate(qA,pfxReadVector3(m_contactPoints[i].m_localPointA));
		PfxVector3 cpB = pB + rotate(qB,pfxReadVector3(m_contactPoints[i].m_localPointB));

		// 貫通深度がプラスに転じたかどうかをチェック
		PfxFloat distance = dot(normal,cpA - cpB);
		if(distance > SCE_PFX_CONTACT_THRESHOLD_NORMAL) {
			removeContactPoint(i);
			i--;
			continue;
		}
		m_contactPoints[i].m_distance = distance;

		// 深度方向を除去して両点の距離をチェック
		cpA = cpA - m_contactPoints[i].m_distance * normal;
		PfxFloat distanceAB = lengthSqr(cpA - cpB);
		if(distanceAB > SCE_PFX_CONTACT_THRESHOLD_TANGENT) {
			removeContactPoint(i);
			i--;
			continue;
		}
	}
	if(m_numContactPoints > 0 && m_duration < 65535) m_duration++;
	if(m_numContactPoints == 0) m_duration = 0;

	// 最近接点のクリア
	m_numClosestPoints = 0;
	m_closestPoints = NULL;
}

void PfxContactManifold::addClosestPoint(
		PfxFloat newDistance,
		const PfxVector3 &newNormal,
		const PfxPoint3 &newPointA,
		const PfxPoint3 &newPointB,
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
			PfxFloat diffA = lengthSqr(pfxReadPoint3(m_closestPoints[i].m_localPointA) - newPointA);
			PfxFloat diffB = lengthSqr(pfxReadPoint3(m_closestPoints[i].m_localPointB) - newPointB);
			PfxFloat angle = dot(m_closestPoints[i].getNormal(), newNormal);
			PfxUInt8 priorityA = m_closestPoints[i].m_subDataA.getCcdPriority();
			PfxUInt8 priorityB = m_closestPoints[i].m_subDataB.getCcdPriority();
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

			if ((isSameCoreSphere(subDataA, m_closestPoints[i].m_subDataA) || isSameCoreSphere(subDataB, m_closestPoints[i].m_subDataB)) && angle > 0.85f) {
				if (newDistance < m_closestPoints[i].m_distance) {
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
		m_closestPoints[replaceId].m_subDataA = subDataA;
		m_closestPoints[replaceId].m_subDataB = subDataB;
		m_closestPoints[replaceId].m_distance = newDistance;
		m_closestPoints[replaceId].m_constraintRow.m_accumImpulse = 0.0f;
		m_closestPoints[replaceId].m_constraintRow.setNormal(newNormal);
		pfxStorePoint3(newPointA, m_closestPoints[replaceId].m_localPointA);
		pfxStorePoint3(newPointB, m_closestPoints[replaceId].m_localPointB);
	}
}

PfxContactManifold *PfxContactHolder::findContactManifold(PfxUInt8 shapeIdA, PfxUInt8 shapeIdB)
{
	PfxUInt16 shapeId = ((PfxUInt16)shapeIdB << 8) | (PfxUInt16)shapeIdA;
	PfxContactManifold *cm = m_contactManifolds;
	while(cm) {
		if(cm->m_shapeId == shapeId) {
			return cm;
		}
		cm = cm->m_next;
	}
	return nullptr;
}

} //namespace pfxv4
} //namespace sce
