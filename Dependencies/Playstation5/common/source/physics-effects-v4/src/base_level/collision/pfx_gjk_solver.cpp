/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_intersect_common.h"
#include "pfx_convex_mesh_impl.h"
#include "pfx_gjk_solver.h"
#include "pfx_gjk_support_func.h"

#include "../../../include/physics_effects/base_level/collision/pfx_box.h"
#include "../../../include/physics_effects/base_level/collision/pfx_sphere.h"
#include "../../../include/physics_effects/base_level/collision/pfx_capsule.h"
#include "../../../include/physics_effects/base_level/collision/pfx_cylinder.h"

#include "../../../include/physics_effects/base_level/base/pfx_perf_counter.h"

namespace sce {
namespace pfxv4 {

#define SCE_PFX_GJK_EPSILON				1e-06f
#define SCE_PFX_GJK_MARGIN				0.025f
#define SCE_PFX_GJK_ITERATION_MAX		10
#define SCE_PFX_GJK_SWEEP_EPSILON		1e-08f
#define SCE_PFX_GJK_SWEEP_ITERATION_MAX	16
#define MPR_EPSILON						1e-8f
#define MPR_CLOSE_TO_SURFACE_DISTANCE	1e-4f
#define MPR_DEGENERATE_ANGLE_THRESHOLD	1e-4f
#define MPR_INVALID_NORMAL_LENGTH		1e-3f
#define MPR_RETRY_DEPTH					0.1f
#define MPR_MAX_ITERATION				32

///////////////////////////////////////////////////////////////////////////////
// GJK

template<typename SHAPE_A, typename SHAPE_B>
PfxInt32 PfxGjk<SHAPE_A, SHAPE_B>::closest(PfxFloat &distance, PfxVector3& normal, PfxPoint3 &pointA, PfxPoint3 &pointB,
						const PfxTransform3 & transformA,
						const PfxTransform3 & transformB,
						PfxFloat distanceThreshold)
{
	(void) distanceThreshold;

	SCE_PFX_ASSERT(m_shapeA&&m_shapeB);

	int gjkIterationCount = 0;

	m_simplex.reset();

	// Aローカル座標系に変換
	PfxTransform3 transformAB,transformBA;
	PfxMatrix3 matrixAB,matrixBA;
	PfxVector3 offsetAB,offsetBA;

	// Bローカル->Aローカル変換
	transformAB = inverse(transformA) * transformB;
	matrixAB = transformAB.getUpper3x3();
	offsetAB = transformAB.getTranslation();

	// Aローカル->Bローカル変換
	transformBA = inverse(transformB) * transformA;
	matrixBA = transformBA.getUpper3x3();
	offsetBA = transformBA.getTranslation();

	PfxMatrix3 matrixBA_n = transpose(inverse(matrixBA)); // 法線用回転マトリクス

	PfxVector3 separatingAxis(-offsetAB);
	PfxFloat squaredDistance = lengthSqr(separatingAxis);
	if (SCE_PFX_UNLIKELY(squaredDistance < 0.000001f)) {
		separatingAxis = PfxVector3::xAxis();
		squaredDistance = 1.0f;
	}
	else
		separatingAxis /= sqrtf(squaredDistance);

	PfxFloat delta = 0.0f;

	PfxVector3 latestSeparatingAxis = separatingAxis;

	for(;;) {
		// サポート頂点の取得
		PfxVector3 pInA,qInB;

		PfxFloat distInv = rsqrtf(PfxFloatInVec(squaredDistance));

		getSupportVertex(m_shapeA,(-distInv * separatingAxis),pInA,0.0f);
		getSupportVertex(m_shapeB,normalize(matrixBA_n * distInv * separatingAxis),qInB,0.0f);

		PfxVector3 p = pInA;
		PfxVector3 q = offsetAB + matrixAB * qInB;
		PfxVector3 w = p - q;

		delta = dot(separatingAxis,w);

		// 既に単体に同一頂点が存在している
		if(SCE_PFX_UNLIKELY(m_simplex.inSimplex(w))) {
			break;
		}

		if(gjkIterationCount > 0 && (squaredDistance - delta) < SCE_PFX_GJK_EPSILON) {
			break;
		}

		// 頂点を単体に追加
		m_simplex.addVertex(w,p,q);

		// 原点と単体の最近接点を求め、分離軸を返す
		if(SCE_PFX_UNLIKELY(!m_simplex.closest(separatingAxis))) {
			// zero triangle error
			return kPfxGjkResultInvalid;
		}

		// フルシンプレックスのとき（原点は内部にあり）交差している
		if(m_simplex.fullSimplex()) {
			return kPfxGjkResultIntersect;
		}

		squaredDistance = lengthSqr(separatingAxis);

		// フルシンプレックスでないが原点と接している→交差
		if (squaredDistance < SCE_PFX_GJK_EPSILON * SCE_PFX_GJK_EPSILON) {
			return kPfxGjkResultIntersect;
		}

		// 反復回数が制限に達した。この時点での値を近似値として返す
		if(SCE_PFX_UNLIKELY(gjkIterationCount >= SCE_PFX_GJK_ITERATION_MAX)) {
			break;
		}

		latestSeparatingAxis = separatingAxis;

		gjkIterationCount++;
	}

	PfxVector3 pA(0.0f),pB(0.0f),nA(0.0f);

	m_simplex.getClosestPoints(pA,pB);

	if(lengthSqr(latestSeparatingAxis) < SCE_PFX_GJK_EPSILON * SCE_PFX_GJK_EPSILON) {
		nA = PfxVector3(1.0f,0.0f,0.0f);
	}
	else {
		nA = normalize(latestSeparatingAxis);
	}

	// 全てAローカル座標系
	distance = dot(nA,pA-pB);
	normal = nA;
	pointA = PfxPoint3(pA);
	pointB = transformBA * PfxPoint3(pB);

	SCE_PFX_VALIDATE_VECTOR3(normal);
	SCE_PFX_VALIDATE_FLOAT(distance);

	return kPfxGjkResultOk;
}

template class PfxGjk<PfxSphere, PfxCylinder>;
template class PfxGjk<PfxSphere, PfxConvexMeshImpl>;
template class PfxGjk<PfxCapsule, PfxCylinder>;
template class PfxGjk<PfxCapsule, PfxConvexMeshImpl>;
template class PfxGjk<PfxCylinder, PfxSphere>;
template class PfxGjk<PfxCylinder, PfxCapsule>;
template class PfxGjk<PfxConvexMeshImpl, PfxSphere>;
template class PfxGjk<PfxConvexMeshImpl, PfxCapsule>;

///////////////////////////////////////////////////////////////////////////////
// GJK Sweep


template<typename SHAPE_A, typename SHAPE_B>
PfxInt32 PfxGjkSweep<SHAPE_A, SHAPE_B>::sweep(PfxFloat &lamda, PfxVector3& normal, PfxPoint3 &pointA, PfxPoint3 &pointB,
		const PfxTransform3 & transformA, const PfxVector3 &translationA,
		const PfxTransform3 & transformB, const PfxVector3 &translationB)
{
	SCE_PFX_ASSERT(m_shapeA&&m_shapeB);

	int gjkIterationCount = 0;

	m_simplex.reset();

	// Aローカル座標系に変換
	PfxTransform3 transformAB,transformBA;
	PfxMatrix3 matrixAB,matrixBA;
	PfxVector3 offsetAB,offsetBA;

	// Bローカル->Aローカル変換
	PfxTransform3 transformAInv = inverse(transformA);
	transformAB = transformAInv * transformB;
	matrixAB = transformAB.getUpper3x3();
	offsetAB = transformAB.getTranslation();

	// Aローカル->Bローカル変換
	transformBA = inverse(transformB) * transformA;
	matrixBA = transformBA.getUpper3x3();
	offsetBA = transformBA.getTranslation();

	// 移動ベクトルをワールド->Aローカル変換
	PfxVector3 translation = transformAInv.getUpper3x3() * (translationB - translationA);

	PfxMatrix3 matrixBA_n = transpose(inverse(matrixBA)); // 法線用回転マトリクス

	PfxVector3 separatingAxis = (lengthSqr(offsetAB) < 0.00001f ? PfxVector3(1.f, 0.f, 0.f) : -normalize(offsetAB));
	PfxFloat squaredDistance = SCE_PFX_FLT_MAX;
	PfxFloat distInv = 1.0f;

	PfxFloat offsetLamda = 0.0f;

	PfxVector3 latestP, latestQ;
	PfxVector3 latestSeparatingAxis = separatingAxis;
	bool useLatestResult = false;

	for(;;) {
		// サポート頂点の取得
		PfxVector3 pInA,qInB;

		getSupportVertex(m_shapeA,(-distInv * separatingAxis),pInA,0.0f);
		getSupportVertex(m_shapeB,normalize(matrixBA_n * distInv * separatingAxis),qInB,0.0f);

		PfxVector3 p = pInA;
		PfxVector3 q = offsetAB + matrixAB * qInB;
		PfxVector3 w = p - q;

		// 既に単体に同一頂点が存在している
		if(SCE_PFX_UNLIKELY(m_simplex.inSimplex(w))) {
			break;
		}

		PfxFloat delta = dot(separatingAxis,w);
		if (delta > 0.0f) {
			PfxFloat den = dot(separatingAxis, translation);
			if(den <= 0.0f) {
				return kPfxGjkResultNoHit;
			}

			PfxFloat dlamda = delta / den;

			if (offsetLamda + dlamda >= 1.0f) {
				return kPfxGjkResultNoHit;
			}

			if (dlamda < SCE_PFX_GJK_SWEEP_EPSILON) {
				break;
			}

			offsetLamda += dlamda;

			// Shift the origin to the next position
			offsetAB += dlamda * translation;
			// Also shift q and w
			q += dlamda * translation;
			w = p - q;
			m_simplex.shiftQ(dlamda * translation);
		}

		if (gjkIterationCount > 0 && (squaredDistance - delta) < SCE_PFX_GJK_EPSILON) {
			break;
		}

		// 頂点を単体に追加
		m_simplex.addVertex(w,p,q);

		// 原点と単体の最近接点を求め、分離軸を返す
		if(SCE_PFX_UNLIKELY(!m_simplex.closest(separatingAxis))) {
			// zero triangle error
			return kPfxGjkResultInvalid;
		}

		// フルシンプレックスのとき（原点は内部にあり）交差している
		if(m_simplex.fullSimplex()) {
			if (offsetLamda > 0.0f) {
				useLatestResult = true;
				break;
			}
			else {
				lamda = 0.0f;
				return kPfxGjkResultIntersect;
			}
		}

		squaredDistance = lengthSqr(separatingAxis);

		// 反復回数が制限に達した。この時点での値を近似値として返す
		if (squaredDistance < SCE_PFX_GJK_SWEEP_EPSILON || SCE_PFX_UNLIKELY(gjkIterationCount >= SCE_PFX_GJK_SWEEP_ITERATION_MAX)) {
			break;
		}

		latestSeparatingAxis = separatingAxis;
		m_simplex.getClosestPoints(latestP, latestQ);

		distInv = rsqrtf(PfxFloatInVec(squaredDistance));

		gjkIterationCount++;
	}

	PfxVector3 pA(0.0f),pB(0.0f),nA(0.0f);

	if (!useLatestResult) {
		m_simplex.getClosestPoints(pA, pB);
	}
	else {
		pA = latestP;
		pB = latestQ;
	}

	if(lengthSqr(latestSeparatingAxis) < SCE_PFX_GJK_SWEEP_EPSILON * SCE_PFX_GJK_SWEEP_EPSILON) {
		nA = PfxVector3(1.0f,0.0f,0.0f);
	}
	else {
		nA = normalize(latestSeparatingAxis);
	}

	if (offsetLamda == 0.0f) {
		lamda = offsetLamda;
		normal = nA;
		pointA = PfxPoint3(pA);
		pointB = transformBA * PfxPoint3(pB);
		return kPfxGjkResultIntersect;
	}

	// 全てAローカル座標系
	lamda = offsetLamda;
	normal = nA;
	pointA = PfxPoint3(pA);
	pointB = transformBA * PfxPoint3(pB - offsetLamda * translation);

	SCE_PFX_VALIDATE_VECTOR3(normal);
	SCE_PFX_VALIDATE_FLOAT(lamda);

	return kPfxGjkResultOk;
}

template class PfxGjkSweep<PfxSphere, PfxCapsule>;
template class PfxGjkSweep<PfxSphere, PfxCircle>;
template class PfxGjkSweep<PfxBox, PfxCapsule>;
template class PfxGjkSweep<PfxBox, PfxCircle>;
template class PfxGjkSweep<PfxCapsule, PfxCapsule>;
template class PfxGjkSweep<PfxCylinder, PfxCapsule>;
template class PfxGjkSweep<PfxCapsule, PfxCircle>;
template class PfxGjkSweep<PfxCylinder, PfxCircle>;
template class PfxGjkSweep<PfxConvexMeshImpl, PfxCapsule>;
template class PfxGjkSweep<PfxConvexMeshImpl, PfxCircle>;
template class PfxGjkSweep<PfxConvexMeshImpl, PfxSphere>;
template class PfxGjkSweep<PfxTriangle, PfxCapsule>;
template class PfxGjkSweep<PfxTriangle, PfxCircle>;

///////////////////////////////////////////////////////////////////////////////
// Minkowski Protal Refinement Algorithm

template<typename SHAPE_A, typename SHAPE_B>
PfxInt32 PfxMpr<SHAPE_A, SHAPE_B>::createInitialPortal(Portal &portal, PfxVector3 &cachedAxis)
{
	PfxVector3 s, pInA, qInB, p, q; // Temporal for support function
	PfxUInt32 pId, qId;
	{
		{
			s = normalize(-portal.vtx[0].W);
			pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
			qId = getSupportVertex(m_shapeB, -s, qInB, 0.0);
			p = pInA;
			q = qInB;
			portal.vtx[1].P = pInA;
			portal.vtx[1].Q = qInB;
			portal.vtx[1].W = p - q;
			portal.vtx[1].pId = pId;
			portal.vtx[1].qId = qId;

			if (SCE_PFX_UNLIKELY(dot(portal.vtx[1].W, s) < 0.0f)) {
				cachedAxis = s;
				return kPortalResultInvalidPortal;
			}

			PfxVector3 tmp = cross(portal.vtx[1].W, portal.vtx[0].W);
			if (lengthSqr(tmp) < MPR_EPSILON * MPR_EPSILON) {
				if (1.0f - fabsf(dot(s, PfxVector3::xAxis())) > MPR_EPSILON) {
					tmp = cross(PfxVector3::xAxis(), portal.vtx[0].W);
				}
				else if (1.0f - fabsf(dot(s, PfxVector3::yAxis())) > MPR_EPSILON) {
					tmp = cross(PfxVector3::yAxis(), portal.vtx[0].W);
				}
				else {//if (1.0f - fabsf(dot(s, PfxVector3::zAxis())) > MPR_EPSILON) {
					tmp = cross(PfxVector3::zAxis(), portal.vtx[0].W);
				}
			}

			s = normalize(tmp);

			pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
			qId = getSupportVertex(m_shapeB, -s, qInB, 0.0);
			p = pInA;
			q = qInB;
			portal.vtx[2].P = pInA;
			portal.vtx[2].Q = qInB;
			portal.vtx[2].W = p - q;
			portal.vtx[2].pId = pId;
			portal.vtx[2].qId = qId;

			if (SCE_PFX_UNLIKELY(dot(portal.vtx[2].W, s) < 0.0f)) {
				cachedAxis = s;
				return kPortalResultInvalidPortal;
			}

			s = normalize(cross(portal.vtx[1].W - portal.vtx[0].W, portal.vtx[2].W - portal.vtx[0].W));
			pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
			qId = getSupportVertex(m_shapeB, -s, qInB, 0.0);
			p = pInA;
			q = qInB;
			portal.vtx[3].P = pInA;
			portal.vtx[3].Q = qInB;
			portal.vtx[3].W = p - q;
			portal.vtx[3].pId = pId;
			portal.vtx[3].qId = qId;

			if (SCE_PFX_UNLIKELY(dot(portal.vtx[3].W, s) < 0.0f)) {
				cachedAxis = s;
				return kPortalResultInvalidPortal;
			}
		}

#define SWAP_PORTAL_POINTS(idA,idB) {pfxSwap(portal.vtx[idA], portal.vtx[idB]);}

		//J 原点がポータルの内側にあるかどうかをチェック。なければ再度ポータルを作り直す
		//E Check if the origin is inside of the portal. If the origin is outside of the portal, create new portal again.
		// ここはGJKで置き換え可能
		bool originIsInsidePortal = false;
		int create_count = 0; // Safety net
		do {
			PfxVector3 a = portal.vtx[1].W - portal.vtx[0].W;
			PfxVector3 b = portal.vtx[2].W - portal.vtx[0].W;
			PfxVector3 c = portal.vtx[3].W - portal.vtx[0].W;
			PfxVector3 n1 = normalize(cross(a, c));
			PfxVector3 n2 = normalize(cross(b, a));
			PfxVector3 n3 = normalize(cross(c, b));
			PfxVector3 distFacet = transpose(PfxMatrix3(n1, n2, n3)) * -portal.vtx[0].W;

			if (distFacet[0] > 0.0f) {
				SWAP_PORTAL_POINTS(1, 3)
					s = n1;
				pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
				qId = getSupportVertex(m_shapeB, -s, qInB, 0.0);
				p = pInA;
				q = qInB;
				portal.vtx[2].P = pInA;
				portal.vtx[2].Q = qInB;
				portal.vtx[2].W = p - q;
				portal.vtx[2].pId = pId;
				portal.vtx[2].qId = qId;
			}
			else if (distFacet[1] > 0.0f) {
				SWAP_PORTAL_POINTS(1, 2)
					s = n2;
				pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
				qId = getSupportVertex(m_shapeB, -s, qInB, 0.0);
				p = pInA;
				q = qInB;
				portal.vtx[3].P = pInA;
				portal.vtx[3].Q = qInB;
				portal.vtx[3].W = p - q;
				portal.vtx[3].pId = pId;
				portal.vtx[3].qId = qId;
			}
			else if (distFacet[2] > 0.0f) {
				SWAP_PORTAL_POINTS(2, 3)
					s = n3;
				pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
				qId = getSupportVertex(m_shapeB, -s, qInB, 0.0);
				p = pInA;
				q = qInB;
				portal.vtx[1].P = pInA;
				portal.vtx[1].Q = qInB;
				portal.vtx[1].W = p - q;
				portal.vtx[1].pId = pId;
				portal.vtx[1].qId = qId;
			}
			else {
				originIsInsidePortal = true;
			}

			// Safety net
			if (SCE_PFX_UNLIKELY((create_count++)>32)) {
				return kPortalResultInvalidPortal;
			}
		} while (!originIsInsidePortal);
	}

	return kPortalResultOk;
}

template<typename SHAPE_A, typename SHAPE_B>
PfxInt32 PfxMpr<SHAPE_A, SHAPE_B>::createInitialPortal(Portal &portal, const PfxVector3 &offsetAB, const PfxMatrix3 &matrixAB, const PfxMatrix3 &matrixBA_n, PfxVector3 &cachedAxis)
{
	PfxVector3 s, pInA, qInB, p, q; // Temporal for support function
	PfxUInt32 pId, qId;
	{
		{
			s = normalize(-portal.vtx[0].W);
			pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
			qId = getSupportVertex(m_shapeB, normalize(matrixBA_n * (-s)), qInB, 0.0);
			p = pInA;
			q = offsetAB + matrixAB * qInB;
			portal.vtx[1].P = pInA;
			portal.vtx[1].Q = qInB;
			portal.vtx[1].W = p - q;
			portal.vtx[1].pId = pId;
			portal.vtx[1].qId = qId;
			
			if (SCE_PFX_UNLIKELY(dot(portal.vtx[1].W, s) < 0.0f)) {
				cachedAxis = s;
				return kPortalResultInvalidPortal;
			}

			PfxVector3 tmp = cross(portal.vtx[1].W, portal.vtx[0].W);
			if (lengthSqr(tmp) < MPR_EPSILON * MPR_EPSILON) {
				if (1.0f - fabsf(dot(s, PfxVector3::xAxis())) > MPR_EPSILON) {
					tmp = cross(PfxVector3::xAxis(), portal.vtx[0].W);
				}
				else if (1.0f - fabsf(dot(s, PfxVector3::yAxis())) > MPR_EPSILON) {
					tmp = cross(PfxVector3::yAxis(), portal.vtx[0].W);
				}
				else {//if (1.0f - fabsf(dot(s, PfxVector3::zAxis())) > MPR_EPSILON) {
					tmp = cross(PfxVector3::zAxis(), portal.vtx[0].W);
				}
			}

			s = normalize(tmp);

			pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
			qId = getSupportVertex(m_shapeB, normalize(matrixBA_n * (-s)), qInB, 0.0);
			p = pInA;
			q = offsetAB + matrixAB * qInB;
			portal.vtx[2].P = pInA;
			portal.vtx[2].Q = qInB;
			portal.vtx[2].W = p - q;
			portal.vtx[2].pId = pId;
			portal.vtx[2].qId = qId;

			if (SCE_PFX_UNLIKELY(dot(portal.vtx[2].W, s) < 0.0f)) {
				cachedAxis = s;
				return kPortalResultInvalidPortal;
			}

			s = normalize(cross(portal.vtx[1].W - portal.vtx[0].W, portal.vtx[2].W - portal.vtx[0].W));
			pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
			qId = getSupportVertex(m_shapeB, normalize(matrixBA_n * (-s)), qInB, 0.0);
			p = pInA;
			q = offsetAB + matrixAB * qInB;
			portal.vtx[3].P = pInA;
			portal.vtx[3].Q = qInB;
			portal.vtx[3].W = p - q;
			portal.vtx[3].pId = pId;
			portal.vtx[3].qId = qId;

			if (SCE_PFX_UNLIKELY(dot(portal.vtx[3].W, s) < 0.0f)) {
				cachedAxis = s;
				return kPortalResultInvalidPortal;
			}
		}

#define SWAP_PORTAL_POINTS(idA,idB) {pfxSwap(portal.vtx[idA], portal.vtx[idB]);}

		//J 原点がポータルの内側にあるかどうかをチェック。なければ再度ポータルを作り直す
		//E Check if the origin is inside of the portal. If the origin is outside of the portal, create new portal again.
		// ここはGJKで置き換え可能
		bool originIsInsidePortal = false;
		int create_count = 0; // Safety net
		do {
			PfxVector3 a = portal.vtx[1].W - portal.vtx[0].W;
			PfxVector3 b = portal.vtx[2].W - portal.vtx[0].W;
			PfxVector3 c = portal.vtx[3].W - portal.vtx[0].W;
			PfxVector3 n1 = normalize(cross(a, c));
			PfxVector3 n2 = normalize(cross(b, a));
			PfxVector3 n3 = normalize(cross(c, b));
			PfxVector3 distFacet = transpose(PfxMatrix3(n1, n2, n3)) * -portal.vtx[0].W;
			
			if (distFacet[0] > 0.0f) {
				SWAP_PORTAL_POINTS(1, 3)
					s = n1;
				pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
				qId = getSupportVertex(m_shapeB, normalize(matrixBA_n * (-s)), qInB, 0.0);
				p = pInA;
				q = offsetAB + matrixAB * qInB;
				portal.vtx[2].P = pInA;
				portal.vtx[2].Q = qInB;
				portal.vtx[2].W = p - q;
				portal.vtx[2].pId = pId;
				portal.vtx[2].qId = qId;
			}
			else if (distFacet[1] > 0.0f) {
				SWAP_PORTAL_POINTS(1, 2)
					s = n2;
				pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
				qId = getSupportVertex(m_shapeB, normalize(matrixBA_n * (-s)), qInB, 0.0);
				p = pInA;
				q = offsetAB + matrixAB * qInB;
				portal.vtx[3].P = pInA;
				portal.vtx[3].Q = qInB;
				portal.vtx[3].W = p - q;
				portal.vtx[3].pId = pId;
				portal.vtx[3].qId = qId;
			}
			else if (distFacet[2] > 0.0f) {
				SWAP_PORTAL_POINTS(2, 3)
					s = n3;
				pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
				qId = getSupportVertex(m_shapeB, normalize(matrixBA_n * (-s)), qInB, 0.0);
				p = pInA;
				q = offsetAB + matrixAB * qInB;
				portal.vtx[1].P = pInA;
				portal.vtx[1].Q = qInB;
				portal.vtx[1].W = p - q;
				portal.vtx[1].pId = pId;
				portal.vtx[1].qId = qId;
			}
			else {
				originIsInsidePortal = true;
			}

			// Safety net
			if (SCE_PFX_UNLIKELY((create_count++)>32)) {
				return kPortalResultInvalidPortal;
			}
		} while (!originIsInsidePortal);
	}

	return kPortalResultOk;
}

template<typename SHAPE_A, typename SHAPE_B>
PfxInt32 PfxMpr<SHAPE_A, SHAPE_B>::collide(PfxFloat &distance, PfxVector3& normal, PfxPoint3 &pointA, PfxPoint3 &pointB,PfxUInt32 &featureIdA, PfxUInt32 &featureIdB,
						PfxVector3 &cachedAxis,
						PfxFloat distanceThreshold)
{
	(void) distanceThreshold;

	#ifndef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
	(void) featureIdA;
	(void) featureIdB;
	#endif

	Portal portal;
	PfxVector3 s, pInA, qInB, p, q; // Temporal for support function
	PfxUInt32 pId, qId;

	// early out check
	if (lengthSqr(cachedAxis) > 0.0f) {
		getSupportVertex(m_shapeA, cachedAxis, pInA, 0.0);
		getSupportVertex(m_shapeB, -cachedAxis, qInB, 0.0);
		p = pInA;
		q = qInB;
		PfxVector3 w = p - q;

		if (dot(cachedAxis, -w) > 0.0f) {
			distance = SCE_PFX_FLT_MAX;
			return kPfxGjkResultOk;
		}
	}

	// ポータル原点を作成
	{
		PfxVector3 centerA(0.0f);
		PfxVector3 centerB(0.0f);

		getCenter(m_shapeA,centerA);
		getCenter(m_shapeB,centerB);

		portal.vtx[0].W = centerA - centerB;

		if (SCE_PFX_UNLIKELY(lengthSqr(portal.vtx[0].W) < MPR_EPSILON)) return kPfxGjkResultInvalid;
	}

	PfxFloat distance_ = SCE_PFX_FLT_MAX;

	//J 初期ポータルの作成
	//E Create the initial portal
	PfxInt32 ret = createInitialPortal(portal, cachedAxis);
	if(ret != kPortalResultOk) {
		distance = distance_;
		return kPfxGjkResultInvalid;
	}

	PfxVector3 closestPoint;
	{
		Portal &curPortal = portal;

		int mpr_count = 0;
		do {
			//J サーフェスに近い新たな点を求める
			//E Find new point close to the surface.
			s = normalize(cross(curPortal.vtx[2].W-curPortal.vtx[1].W,curPortal.vtx[3].W-curPortal.vtx[1].W));
			if (SCE_PFX_UNLIKELY(!isfinite((float)s[0]))) {
				distance = distance_;
				return kPfxGjkResultInvalid;
			}

			pId = getSupportVertex(m_shapeA,s,pInA,0.0);
			qId = getSupportVertex(m_shapeB,-s,qInB,0.0);
			p = pInA;
			q = qInB;
			PfxVector3 w = p - q;

			//J 原点とサポート平面の位置関係をチェック
			//E Check if the origin is outside of the support plane.
			if(dot(s,-w) > 0.0f) {
				cachedAxis = s;
				distance = distance_;
				return kPfxGjkResultOk;
			}

			//J ポータルが充分サーフェスに近づいたかどうかを判定
			//E Check distance between the portal and the surface.
			PfxFloat distanceToSurface = dot(s, w - curPortal.vtx[1].W);
			if (distanceToSurface < MPR_CLOSE_TO_SURFACE_DISTANCE) {
				break;
			}

			//J カウントチェック
			//E Check counts
			if((mpr_count++) > MPR_MAX_ITERATION) {
				break;
			}

			//J ポータルのリファイン
			//E Portal refinement
			PfxVector3 cm = w - curPortal.vtx[0].W;
			PfxVector3 n1 = cross(cm, (curPortal.vtx[1].W - curPortal.vtx[0].W));
			PfxVector3 n2 = cross(cm, (curPortal.vtx[2].W - curPortal.vtx[0].W));
			PfxVector3 n3 = cross(cm, (curPortal.vtx[3].W - curPortal.vtx[0].W));
			PfxVector3 dd = transpose(PfxMatrix3(n1, n2, n3)) * -curPortal.vtx[0].W;

			int dst = 2;
			if (lengthSqr(n1) < MPR_EPSILON || (dd[1] > 0.0f && dd[2] <= 0.0f)) {
				dst = 1;
			}
			else if (lengthSqr(n3) < MPR_EPSILON || (dd[0] > 0.0f && dd[1] <= 0.0f)) {
				dst = 3;
			}
			//else if (lengthSqr(n2) < MPR_EPSILON || (dd[2] > 0.0f && dd[0] <= 0.0f)) {
			//	dst = 2;
			//}
			curPortal.vtx[dst].P = pInA;
			curPortal.vtx[dst].Q = qInB;
			curPortal.vtx[dst].W = w;
			curPortal.vtx[dst].pId = pId;
			curPortal.vtx[dst].qId = qId;
		} while(1);

		pfxClosestPointTriangle(PfxVector3::zero(), PfxTriangle(curPortal.vtx[1].W, curPortal.vtx[2].W, curPortal.vtx[3].W), closestPoint);
	}

	PfxVector3 candidateNormal = -closestPoint;
	PfxFloat candidateNormalLength = length(candidateNormal);

	//J ポータルと原点の最近接点が衝突点になる
	//E Calculate the closest point as the contact point between the portal and the origin.
	{
		Portal &closestPortal = portal;

		PfxVector3 e1 = normalize(closestPortal.vtx[2].W - closestPortal.vtx[1].W);
		PfxVector3 e2 = normalize(closestPortal.vtx[3].W - closestPortal.vtx[2].W);
		PfxVector3 e3 = normalize(closestPortal.vtx[1].W - closestPortal.vtx[3].W);

		PfxFloat area = length(cross(closestPortal.vtx[2].W - closestPortal.vtx[1].W, closestPortal.vtx[3].W - closestPortal.vtx[1].W));

		PfxFloat angle1 = dot(-e3, e1);
		PfxFloat angle2 = dot(-e1, e2);
		PfxFloat angle3 = dot(-e2, e3);

		PfxVector3 pA(0.0f), pB(0.0f);

		int portalType = 0; // 0:point 1:edge 2:triangle
		int edge1 = 0, edge2 = 0, edge3 = 0;

		if (area < MPR_EPSILON) {
			portalType = 0;
		}
		else if (fabsf(1.0f - angle1) < MPR_DEGENERATE_ANGLE_THRESHOLD) {
			portalType = 1;
			edge1 = 2;
			edge2 = 1;
			edge3 = 3;
		}
		else if (fabsf(1.0f - angle2) < MPR_DEGENERATE_ANGLE_THRESHOLD) {
			portalType = 1;
			edge1 = 3;
			edge2 = 2;
			edge3 = 1;
		}
		else if (fabsf(1.0f - angle3) < MPR_DEGENERATE_ANGLE_THRESHOLD) {
			portalType = 1;
			edge1 = 1;
			edge2 = 3;
			edge3 = 2;
		}
		else {
			portalType = 2;
		}

		if (portalType == 0) {
			pA = closestPortal.vtx[1].P;
			pB = closestPortal.vtx[1].Q;
			candidateNormal = PfxVector3::zero();
			candidateNormalLength = 0.0f;
			#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
			featureIdA = createFeatureIdVtx(closestPortal.pId, 1);
			featureIdB = createFeatureIdVtx(closestPortal.qId, 1);
			#endif
		}
		else if (portalType == 1) {
			// edge
			PfxVector3 dir1 = closestPortal.vtx[edge1].W - closestPortal.vtx[edge2].W;
			PfxFloat t1 = divf(dot(closestPoint - closestPortal.vtx[edge2].W, dir1), dot(dir1, dir1));
			t1 = SCE_PFX_CLAMP(t1, 0.0f, 1.0f);
			PfxVector3 dir2 = closestPortal.vtx[edge3].W - closestPortal.vtx[edge2].W;
			PfxFloat t2 = divf(dot(closestPoint - closestPortal.vtx[edge2].W, dir2), dot(dir2, dir2));
			t2 = SCE_PFX_CLAMP(t2, 0.0f, 1.0f);

			PfxVector3 P1 = closestPortal.vtx[edge2].P + t1 * (closestPortal.vtx[edge1].P - closestPortal.vtx[edge2].P);
			PfxVector3 Q1 = closestPortal.vtx[edge2].Q + t1 * (closestPortal.vtx[edge1].Q - closestPortal.vtx[edge2].Q);

			PfxVector3 P2 = closestPortal.vtx[edge2].P + t2 * (closestPortal.vtx[edge3].P - closestPortal.vtx[edge2].P);
			PfxVector3 Q2 = closestPortal.vtx[edge2].Q + t2 * (closestPortal.vtx[edge3].Q - closestPortal.vtx[edge2].Q);

			if (lengthSqr(P1 - Q1) < lengthSqr(P2 - Q2)) {
				pA = P1;
				pB = Q1;
			}
			else {
				pA = P2;
				pB = Q2;
				edge1 = edge3;
			}

			if (candidateNormalLength < MPR_INVALID_NORMAL_LENGTH) {
				PfxVector3 S;
				pfxClosestPointLine(closestPortal.vtx[0].W, closestPortal.vtx[edge1].W, closestPortal.vtx[edge2].W - closestPortal.vtx[edge1].W, S);
				candidateNormal = closestPortal.vtx[0].W - S;
			}
			
			#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
			const PfxFloat eps = 0.001f;
			if (fabsf(t1) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, edge2);
				featureIdB = createFeatureIdVtx(closestPortal.qId, edge2);
			}
			else if(fabsf(1.0f - t1) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, edge1);
				featureIdB = createFeatureIdVtx(closestPortal.qId, edge1);
			}
			else {
				featureIdA = createFeatureIdEdge(closestPortal.pId, edge1, edge2);
				featureIdB = createFeatureIdEdge(closestPortal.qId, edge1, edge2);
			}
			#endif
		}
		else if (portalType == 2) {
			// triangle
			PfxVector3 tmp0 = closestPortal.vtx[2].W - closestPortal.vtx[1].W;
			PfxVector3 tmp1 = closestPortal.vtx[3].W - closestPortal.vtx[1].W;
			PfxVector3 tmp2 = closestPoint - closestPortal.vtx[1].W;
			PfxFloat d00 = dot(tmp0,tmp0);
			PfxFloat d01 = dot(tmp0,tmp1);
			PfxFloat d11 = dot(tmp1,tmp1);
			PfxFloat d20 = dot(tmp2,tmp0);
			PfxFloat d21 = dot(tmp2,tmp1);
			PfxFloat denom = d00 * d11 - d01 * d01;
			if(SCE_PFX_UNLIKELY(denom * denom < MPR_EPSILON * MPR_EPSILON)) {
				return kPfxGjkResultInvalid;
			}
			PfxFloat v = (d11 * d20 - d01 * d21) / denom;
			PfxFloat w = (d00 * d21 - d01 * d20) / denom;
			pA = closestPortal.vtx[1].P + v * (closestPortal.vtx[2].P - closestPortal.vtx[1].P) + w * (closestPortal.vtx[3].P - closestPortal.vtx[1].P);
			pB = closestPortal.vtx[1].Q + v * (closestPortal.vtx[2].Q - closestPortal.vtx[1].Q) + w * (closestPortal.vtx[3].Q - closestPortal.vtx[1].Q);

			if (candidateNormalLength < MPR_INVALID_NORMAL_LENGTH) {
				candidateNormal = cross(tmp1, tmp0);
			}

			#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
			const PfxFloat vv = fabsf(v);
			const PfxFloat ww = fabsf(w);
			const PfxFloat eps = 0.001f;
			// edge cases
			if (vv < eps && ww < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, 1);
				featureIdB = createFeatureIdVtx(closestPortal.qId, 1);
			}
			else if (fabsf(1.0f - v) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, 2);
				featureIdB = createFeatureIdVtx(closestPortal.qId, 2);
			}
			else if (fabsf(1.0f - w) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, 3);
				featureIdB = createFeatureIdVtx(closestPortal.qId, 3);
			}
			else if (vv < eps && ww > eps) {
				featureIdA = createFeatureIdEdge(closestPortal.pId, 1, 3);
				featureIdB = createFeatureIdEdge(closestPortal.qId, 1, 3);
			}
			else if (vv > eps && ww < eps) {
				featureIdA = createFeatureIdEdge(closestPortal.pId, 1, 2);
				featureIdB = createFeatureIdEdge(closestPortal.qId, 1, 2);
			}
			else if (fabsf(1.0f - v - w) < eps) {
				featureIdA = createFeatureIdEdge(closestPortal.pId, 2, 3);
				featureIdB = createFeatureIdEdge(closestPortal.qId, 2, 3);
			}
			else {
				featureIdA = createFeatureIdFacet(closestPortal.pId);
				featureIdB = createFeatureIdFacet(closestPortal.qId);
			}
			#endif
		}

		if (SCE_PFX_UNLIKELY(length(candidateNormal) < SCE_PFX_GJK_EPSILON)) {
			candidateNormal = PfxVector3::xAxis();
			distance = 0.0f;
		}
		else {
			candidateNormal = normalize(candidateNormal);
			distance = -candidateNormalLength;
		}

		pointA = PfxPoint3(pA);
		pointB = PfxPoint3(pB);
		normal = candidateNormal;
	}

	SCE_PFX_VALIDATE_VECTOR3(normal);
	SCE_PFX_VALIDATE_FLOAT(distance);

	return kPfxGjkResultOk;
}

template<typename SHAPE_A, typename SHAPE_B>
PfxInt32 PfxMpr<SHAPE_A, SHAPE_B>::collide(PfxFloat &distance, PfxVector3& normal, PfxPoint3 &pointA, PfxPoint3 &pointB,PfxUInt32 &featureIdA, PfxUInt32 &featureIdB,
						const PfxTransform3 & transformA,
						const PfxTransform3 & transformB,
						PfxVector3 &cachedAxis,
						PfxFloat distanceThreshold)
{
	(void) distanceThreshold;

#ifndef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
	(void) featureIdA;
	(void) featureIdB;
#endif

	PfxTransform3 transformAB, transformBA;
	PfxMatrix3 matrixAB, matrixBA;
	PfxVector3 offsetAB, offsetBA;
	PfxUInt32 pId, qId;

	// Bローカル→Aローカルへの変換
	transformAB = inverse(transformA) * transformB;
	matrixAB = transformAB.getUpper3x3();
	offsetAB = transformAB.getTranslation();

	// Aローカル→Bローカルへの変換
	transformBA = inverse(transformB) * transformA;
	matrixBA = transformBA.getUpper3x3();
	offsetBA = transformBA.getTranslation();

	PfxMatrix3 matrixBA_n = transpose(inverse(matrixBA)); // 法線用回転マトリクス

	Portal portal;
	PfxVector3 s, pInA, qInB, p, q; // Temporal for support function

	// early out check
	if (lengthSqr(cachedAxis) > 0.0f) {
		getSupportVertex(m_shapeA, cachedAxis, pInA, 0.0);
		getSupportVertex(m_shapeB, normalize(matrixBA_n * (-cachedAxis)), qInB, 0.0);
		p = pInA;
		q = offsetAB + matrixAB * qInB;
		PfxVector3 w = p - q;

		if (dot(cachedAxis, -w) > 0.0f) {
			distance = SCE_PFX_FLT_MAX;
			return kPfxGjkResultOk;
		}
	}

	// ポータル原点を作成
	{
		PfxVector3 centerA(0.0f);
		PfxVector3 centerB(0.0f);

		getCenter(m_shapeA,centerA);
		getCenter(m_shapeB,centerB);

		portal.vtx[0].W = centerA - offsetAB - matrixAB * centerB;

		if (SCE_PFX_UNLIKELY(lengthSqr(portal.vtx[0].W) < MPR_EPSILON)) return kPfxGjkResultInvalid;
	}

	PfxFloat distance_ = SCE_PFX_FLT_MAX;

	//J 初期ポータルの作成
	//E Create the initial portal
	PfxInt32 ret = createInitialPortal(portal, offsetAB, matrixAB, matrixBA_n, cachedAxis );
	if(ret != kPortalResultOk) {
		distance = distance_;
		return kPfxGjkResultInvalid;
	}

	PfxVector3 closestPoint;
	{
		Portal &curPortal = portal;

		int mpr_count = 0;
		do {
			//J サーフェスに近い新たな点を求める
			//E Find new point close to the surface.
			s = normalize(cross(curPortal.vtx[2].W-curPortal.vtx[1].W,curPortal.vtx[3].W-curPortal.vtx[1].W));
			if (SCE_PFX_UNLIKELY(!isfinite((float)s[0]))) {
				distance = distance_;
				return kPfxGjkResultInvalid;
			}

			pId = getSupportVertex(m_shapeA,s,pInA,0.0);
			qId = getSupportVertex(m_shapeB,normalize(matrixBA_n * (-s)),qInB,0.0);
			p = pInA;
			q = offsetAB + matrixAB * qInB;
			PfxVector3 w = p - q;

			//J 原点とサポート平面の位置関係をチェック
			//E Check if the origin is outside of the support plane.
			if(dot(s,-w) > 0.0f) {
				cachedAxis = s;
				distance = distance_;
				return kPfxGjkResultOk;
			}

			//J ポータルが充分サーフェスに近づいたかどうかを判定
			//E Check distance between the portal and the surface.
			PfxFloat distanceToSurface = dot(s, w - curPortal.vtx[1].W);
			if (distanceToSurface < MPR_CLOSE_TO_SURFACE_DISTANCE) {
				break;
			}

			//J カウントチェック
			//E Check counts
			if((mpr_count++) > MPR_MAX_ITERATION) {
				break;
			}

			//J ポータルのリファイン
			//E Portal refinement
			PfxVector3 cm = w - curPortal.vtx[0].W;
			PfxVector3 n1 = cross(cm, (curPortal.vtx[1].W - curPortal.vtx[0].W));
			PfxVector3 n2 = cross(cm, (curPortal.vtx[2].W - curPortal.vtx[0].W));
			PfxVector3 n3 = cross(cm, (curPortal.vtx[3].W - curPortal.vtx[0].W));
			PfxVector3 dd = transpose(PfxMatrix3(n1, n2, n3)) * -curPortal.vtx[0].W;

			int dst = 2;
			if (lengthSqr(n1) < MPR_EPSILON || (dd[1] > 0.0f && dd[2] <= 0.0f)) {
				dst = 1;
			}
			else if (lengthSqr(n3) < MPR_EPSILON || (dd[0] > 0.0f && dd[1] <= 0.0f)) {
				dst = 3;
			}
			//else if (lengthSqr(n2) < MPR_EPSILON || (dd[2] > 0.0f && dd[0] <= 0.0f)) {
			//	dst = 2;
			//}
			curPortal.vtx[dst].P = pInA;
			curPortal.vtx[dst].Q = qInB;
			curPortal.vtx[dst].W = w;
			curPortal.vtx[dst].pId = pId;
			curPortal.vtx[dst].qId = qId;
		} while(1);

		pfxClosestPointTriangle(PfxVector3::zero(), PfxTriangle(curPortal.vtx[1].W, curPortal.vtx[2].W, curPortal.vtx[3].W), closestPoint);
	}

	PfxVector3 candidateNormal = -closestPoint;
	PfxFloat candidateNormalLength = length(candidateNormal);

	//J ポータルと原点の最近接点が衝突点になる
	//E Calculate the closest point as the contact point between the portal and the origin.
	{
		Portal &closestPortal = portal;

		PfxVector3 e1 = normalize(closestPortal.vtx[2].W - closestPortal.vtx[1].W);
		PfxVector3 e2 = normalize(closestPortal.vtx[3].W - closestPortal.vtx[2].W);
		PfxVector3 e3 = normalize(closestPortal.vtx[1].W - closestPortal.vtx[3].W);

		PfxFloat area = length(cross(closestPortal.vtx[2].W - closestPortal.vtx[1].W, closestPortal.vtx[3].W - closestPortal.vtx[1].W));

		PfxFloat angle1 = dot(-e3, e1);
		PfxFloat angle2 = dot(-e1, e2);
		PfxFloat angle3 = dot(-e2, e3);

		PfxVector3 pA(0.0f), pB(0.0f);

		int portalType = 0; // 0:point 1:edge 2:triangle
		int edge1 = 0, edge2 = 0, edge3 = 0;

		if (area < MPR_EPSILON) {
			portalType = 0;
		}
		else if (fabsf(1.0f - angle1) < MPR_DEGENERATE_ANGLE_THRESHOLD) {
			portalType = 1;
			edge1 = 2;
			edge2 = 1;
			edge3 = 3;
		}
		else if (fabsf(1.0f - angle2) < MPR_DEGENERATE_ANGLE_THRESHOLD) {
			portalType = 1;
			edge1 = 3;
			edge2 = 2;
			edge3 = 1;
		}
		else if (fabsf(1.0f - angle3) < MPR_DEGENERATE_ANGLE_THRESHOLD) {
			portalType = 1;
			edge1 = 1;
			edge2 = 3;
			edge3 = 2;
		}
		else {
			portalType = 2;
		}

		if (portalType == 0) {
			pA = closestPortal.vtx[1].P;
			pB = closestPortal.vtx[1].Q;
			candidateNormal = PfxVector3::zero();
			candidateNormalLength = 0.0f;
			#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
			featureIdA = createFeatureIdVtx(closestPortal.pId, 1);
			featureIdB = createFeatureIdVtx(closestPortal.qId, 1);
			#endif
		}
		else if (portalType == 1) {
			// edge
			PfxVector3 dir1 = closestPortal.vtx[edge1].W - closestPortal.vtx[edge2].W;
			PfxFloat t1 = divf(dot(closestPoint - closestPortal.vtx[edge2].W, dir1), dot(dir1, dir1));
			t1 = SCE_PFX_CLAMP(t1, 0.0f, 1.0f);
			PfxVector3 dir2 = closestPortal.vtx[edge3].W - closestPortal.vtx[edge2].W;
			PfxFloat t2 = divf(dot(closestPoint - closestPortal.vtx[edge2].W, dir2), dot(dir2, dir2));
			t2 = SCE_PFX_CLAMP(t2, 0.0f, 1.0f);

			PfxVector3 P1 = closestPortal.vtx[edge2].P + t1 * (closestPortal.vtx[edge1].P - closestPortal.vtx[edge2].P);
			PfxVector3 Q1 = closestPortal.vtx[edge2].Q + t1 * (closestPortal.vtx[edge1].Q - closestPortal.vtx[edge2].Q);

			PfxVector3 P2 = closestPortal.vtx[edge2].P + t2 * (closestPortal.vtx[edge3].P - closestPortal.vtx[edge2].P);
			PfxVector3 Q2 = closestPortal.vtx[edge2].Q + t2 * (closestPortal.vtx[edge3].Q - closestPortal.vtx[edge2].Q);

			if (lengthSqr(P1 - Q1) < lengthSqr(P2 - Q2)) {
				pA = P1;
				pB = Q1;
			}
			else {
				pA = P2;
				pB = Q2;
				edge1 = edge3;
			}

			if (candidateNormalLength < MPR_INVALID_NORMAL_LENGTH) {
				PfxVector3 S;
				pfxClosestPointLine(closestPortal.vtx[0].W, closestPortal.vtx[edge1].W, closestPortal.vtx[edge2].W - closestPortal.vtx[edge1].W, S);
				candidateNormal = closestPortal.vtx[0].W - S;
			}

			#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
			const PfxFloat eps = 0.001f;
			if (fabsf(t1) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, edge2);
				featureIdB = createFeatureIdVtx(closestPortal.qId, edge2);
			}
			else if(fabsf(1.0f - t1) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, edge1);
				featureIdB = createFeatureIdVtx(closestPortal.qId, edge1);
			}
			else {
				featureIdA = createFeatureIdEdge(closestPortal.pId, edge1, edge2);
				featureIdB = createFeatureIdEdge(closestPortal.qId, edge1, edge2);
			}
			#endif
		}
		else if (portalType == 2) {
			// triangle
			PfxVector3 tmp0 = closestPortal.vtx[2].W - closestPortal.vtx[1].W;
			PfxVector3 tmp1 = closestPortal.vtx[3].W - closestPortal.vtx[1].W;
			PfxVector3 tmp2 = closestPoint - closestPortal.vtx[1].W;
			PfxFloat d00 = dot(tmp0,tmp0);
			PfxFloat d01 = dot(tmp0,tmp1);
			PfxFloat d11 = dot(tmp1,tmp1);
			PfxFloat d20 = dot(tmp2,tmp0);
			PfxFloat d21 = dot(tmp2,tmp1);
			PfxFloat denom = d00 * d11 - d01 * d01;
			if(SCE_PFX_UNLIKELY(denom * denom < MPR_EPSILON * MPR_EPSILON)) {
				return kPfxGjkResultInvalid;
			}
			PfxFloat v = (d11 * d20 - d01 * d21) / denom;
			PfxFloat w = (d00 * d21 - d01 * d20) / denom;
			pA = closestPortal.vtx[1].P + v * (closestPortal.vtx[2].P - closestPortal.vtx[1].P) + w * (closestPortal.vtx[3].P - closestPortal.vtx[1].P);
			pB = closestPortal.vtx[1].Q + v * (closestPortal.vtx[2].Q - closestPortal.vtx[1].Q) + w * (closestPortal.vtx[3].Q - closestPortal.vtx[1].Q);

			if (candidateNormalLength < MPR_INVALID_NORMAL_LENGTH) {
				candidateNormal = cross(tmp1, tmp0);
			}			

			#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
			const PfxFloat vv = fabsf(v);
			const PfxFloat ww = fabsf(w);
			const PfxFloat eps = 0.001f;
			// edge cases
			if (vv < eps && ww < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, 1);
				featureIdB = createFeatureIdVtx(closestPortal.qId, 1);
			}
			else if (fabsf(1.0f - v) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, 2);
				featureIdB = createFeatureIdVtx(closestPortal.qId, 2);
			}
			else if (fabsf(1.0f - w) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, 3);
				featureIdB = createFeatureIdVtx(closestPortal.qId, 3);
			}
			else if (vv < eps && ww > eps) {
				featureIdA = createFeatureIdEdge(closestPortal.pId, 1, 3);
				featureIdB = createFeatureIdEdge(closestPortal.qId, 1, 3);
			}
			else if (vv > eps && ww < eps) {
				featureIdA = createFeatureIdEdge(closestPortal.pId, 1, 2);
				featureIdB = createFeatureIdEdge(closestPortal.qId, 1, 2);
			}
			else if (fabsf(1.0f - v - w) < eps) {
				featureIdA = createFeatureIdEdge(closestPortal.pId, 2, 3);
				featureIdB = createFeatureIdEdge(closestPortal.qId, 2, 3);
			}
			else {
				featureIdA = createFeatureIdFacet(closestPortal.pId);
				featureIdB = createFeatureIdFacet(closestPortal.qId);
			}
			#endif
		}

		if (SCE_PFX_UNLIKELY(length(candidateNormal) < SCE_PFX_GJK_EPSILON)) {
			candidateNormal = PfxVector3::xAxis();
			distance = 0.0f;
		}
		else {
			candidateNormal = normalize(candidateNormal);
			distance = -candidateNormalLength;
		}

		pointA = PfxPoint3(pA);
		pointB = PfxPoint3(pB);
		normal = candidateNormal;
	}

	SCE_PFX_VALIDATE_VECTOR3(normal);
	SCE_PFX_VALIDATE_FLOAT(distance);

	return kPfxGjkResultOk;
}

template<typename SHAPE_A, typename SHAPE_B>
PfxInt32 PfxMpr<SHAPE_A, SHAPE_B>::collideRetry(PfxFloat &distance, PfxVector3& normal, PfxPoint3 &pointA, PfxPoint3 &pointB,PfxUInt32 &featureIdA, PfxUInt32 &featureIdB,
						PfxVector3 &cachedAxis,
						PfxFloat distanceThreshold)
{
	(void)distanceThreshold;

#ifndef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
	(void) featureIdA;
	(void) featureIdB;
#endif

	Portal portal[4];
	PfxVector3 closestPoints[4];
	PfxFloat portalDistance[4] = {
		SCE_PFX_FLT_MAX, SCE_PFX_FLT_MAX, SCE_PFX_FLT_MAX, SCE_PFX_FLT_MAX
	};

	PfxVector3 s, pInA, qInB, p, q; // Temporal for support function
	PfxUInt32 pId, qId;

	// early out check
	if (lengthSqr(cachedAxis) > 0.0f) {
		getSupportVertex(m_shapeA, cachedAxis, pInA, 0.0);
		getSupportVertex(m_shapeB, -cachedAxis, qInB, 0.0);
		p = pInA;
		q = qInB;
		PfxVector3 w = p - q;

		if (dot(cachedAxis, -w) > 0.0f) {
			distance = SCE_PFX_FLT_MAX;
			return kPfxGjkResultOk;
		}
	}

	for (int retry = 0; retry<4; retry++) {
		Portal &curPortal = portal[retry];

		// ポータル原点を作成
		if (retry == 0) {
			PfxVector3 centerA(0.0f);
			PfxVector3 centerB(0.0f);

			getCenter(m_shapeA, centerA);
			getCenter(m_shapeB, centerB);

			curPortal.vtx[0].W = centerA - centerB;

			if (SCE_PFX_UNLIKELY(lengthSqr(curPortal.vtx[0].W) < MPR_EPSILON)) return kPfxGjkResultInvalid;
		}
		else {
			const int ids[] = { 1, 2, 2, 3, 3, 1 };
			portal[retry].vtx[0].W = calcCandidatePoint(portal[0].vtx[0].W, portal[0].vtx[ids[2 * (retry - 1)]].W, portal[0].vtx[ids[2 * (retry - 1) + 1]].W);
		}

		PfxFloat distance_ = SCE_PFX_FLT_MAX;

		//J 初期ポータルの作成
		//E Create the initial portal
		PfxInt32 ret = createInitialPortal(curPortal, cachedAxis);
		if (ret != kPortalResultOk) {
			if (retry == 0) {
				distance = distance_;
				return kPfxGjkResultInvalid;
			}
			else {
				break;
			}
		}

		{
			int mpr_count = 0;
			bool skip = false;
			do {
				//J サーフェスに近い新たな点を求める
				//E Find new point close to the surface.
				s = normalize(cross(curPortal.vtx[2].W - curPortal.vtx[1].W, curPortal.vtx[3].W - curPortal.vtx[1].W));
				if (SCE_PFX_UNLIKELY(!isfinite((float)s[0]))) {
					if (retry == 0) {
						distance = distance_;
						return kPfxGjkResultInvalid;
					}
					else {
						skip = true;
						break;
					}
				}

				pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
				qId = getSupportVertex(m_shapeB, -s, qInB, 0.0);
				p = pInA;
				q = qInB;
				PfxVector3 w = p - q;

				//J 原点とサポート平面の位置関係をチェック
				//E Check if the origin is outside of the support plane.
				if (dot(s, -w) > 0.0f) {
					if (retry == 0) {
						cachedAxis = s;
						distance = distance_;
						return kPfxGjkResultOk;
					}
					else {
						skip = true;
						break;
					}
				}

				//J ポータルが充分サーフェスに近づいたかどうかを判定
				//E Check distance between the portal and the surface.
				PfxFloat distanceToSurface = dot(s, w - curPortal.vtx[1].W);
				if (distanceToSurface < MPR_CLOSE_TO_SURFACE_DISTANCE) {
					break;
				}

				//J カウントチェック
				//E Check counts
				if ((mpr_count++) > MPR_MAX_ITERATION) {
					break;
				}

				//J ポータルのリファイン
				//E Portal refinement
				PfxVector3 cm = w - curPortal.vtx[0].W;
				PfxVector3 n1 = cross(cm, (curPortal.vtx[1].W - curPortal.vtx[0].W));
				PfxVector3 n2 = cross(cm, (curPortal.vtx[2].W - curPortal.vtx[0].W));
				PfxVector3 n3 = cross(cm, (curPortal.vtx[3].W - curPortal.vtx[0].W));
				// dd[0] = dot(-curPortal.vtx[0].W, n1)
				// dd[1] = dot(-curPortal.vtx[0].W, n2)
				// dd[2] = dot(-curPortal.vtx[0].W, n3)
				PfxVector3 dd = transpose(PfxMatrix3(n1, n2, n3)) * -curPortal.vtx[0].W;

				int dst = 2;
				if (lengthSqr(n1) < MPR_EPSILON || (dd[1] > 0.0f && dd[2] <= 0.0f)) {
					dst = 1;
				}
				else if (lengthSqr(n3) < MPR_EPSILON || (dd[0] > 0.0f && dd[1] <= 0.0f)) {
					dst = 3;
				}
				//else if (lengthSqr(n2) < MPR_EPSILON || (dd[2] > 0.0f && dd[0] <= 0.0f)) {
				//	dst = 2;
				//}
				curPortal.vtx[dst].P = pInA;
				curPortal.vtx[dst].Q = qInB;
				curPortal.vtx[dst].W = w;
				curPortal.vtx[dst].pId = pId;
				curPortal.vtx[dst].qId = qId;
			} while (1);

			if (skip) continue;

			pfxClosestPointTriangle(PfxVector3::zero(), PfxTriangle(curPortal.vtx[1].W, curPortal.vtx[2].W, curPortal.vtx[3].W), closestPoints[retry]);

			portalDistance[retry] = lengthSqr(closestPoints[retry]);

			// There is a possiblity to detect better contact points. So retry portal refinement again by using the new origin.
			if (portalDistance[retry] < MPR_RETRY_DEPTH * MPR_RETRY_DEPTH) {
				break;
			}
		}
	}

	// Choose the best candidate for the contact
	int closestId = 0;
	PfxFloat closestDistance = portalDistance[0];
	for (int i = 1; i<4; i++) {
		if (portalDistance[i] < closestDistance) {
			closestDistance = portalDistance[i];
			closestId = i;
		}
	}
	PfxVector3 candidateNormal = -closestPoints[closestId];
	PfxFloat candidateNormalLength = length(candidateNormal);

	//J ポータルと原点の最近接点が衝突点になる
	//E Calculate the closest point as the contact point between the portal and the origin.
	{
		Portal &closestPortal = portal[closestId];
		PfxVector3 &closestPoint = closestPoints[closestId];

		PfxVector3 e1 = normalize(closestPortal.vtx[2].W - closestPortal.vtx[1].W);
		PfxVector3 e2 = normalize(closestPortal.vtx[3].W - closestPortal.vtx[2].W);
		PfxVector3 e3 = normalize(closestPortal.vtx[1].W - closestPortal.vtx[3].W);

		PfxFloat area = length(cross(closestPortal.vtx[2].W - closestPortal.vtx[1].W, closestPortal.vtx[3].W - closestPortal.vtx[1].W));

		PfxFloat angle1 = dot(-e3, e1);
		PfxFloat angle2 = dot(-e1, e2);
		PfxFloat angle3 = dot(-e2, e3);

		PfxVector3 pA(0.0f), pB(0.0f);

		int portalType = 0; // 0:point 1:edge 2:triangle
		int edge1 = 0, edge2 = 0, edge3 = 0;

		if (area < MPR_EPSILON) {
			portalType = 0;
		}
		else if (fabsf(1.0f - angle1) < MPR_DEGENERATE_ANGLE_THRESHOLD) {
			portalType = 1;
			edge1 = 2;
			edge2 = 1;
			edge3 = 3;
		}
		else if (fabsf(1.0f - angle2) < MPR_DEGENERATE_ANGLE_THRESHOLD) {
			portalType = 1;
			edge1 = 3;
			edge2 = 2;
			edge3 = 1;
		}
		else if (fabsf(1.0f - angle3) < MPR_DEGENERATE_ANGLE_THRESHOLD) {
			portalType = 1;
			edge1 = 1;
			edge2 = 3;
			edge3 = 2;
		}
		else {
			portalType = 2;
		}

		if (portalType == 0) {
			pA = closestPortal.vtx[1].P;
			pB = closestPortal.vtx[1].Q;
			candidateNormal = PfxVector3::zero();
			candidateNormalLength = 0.0f;
			#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
			featureIdA = createFeatureIdVtx(closestPortal.pId, 1);
			featureIdB = createFeatureIdVtx(closestPortal.qId, 1);
			#endif
		}
		else if (portalType == 1) {
			// edge
			PfxVector3 dir1 = closestPortal.vtx[edge1].W - closestPortal.vtx[edge2].W;
			PfxFloat t1 = divf(dot(closestPoint - closestPortal.vtx[edge2].W, dir1), dot(dir1, dir1));
			t1 = SCE_PFX_CLAMP(t1, 0.0f, 1.0f);
			PfxVector3 dir2 = closestPortal.vtx[edge3].W - closestPortal.vtx[edge2].W;
			PfxFloat t2 = divf(dot(closestPoint - closestPortal.vtx[edge2].W, dir2), dot(dir2, dir2));
			t2 = SCE_PFX_CLAMP(t2, 0.0f, 1.0f);

			PfxVector3 P1 = closestPortal.vtx[edge2].P + t1 * (closestPortal.vtx[edge1].P - closestPortal.vtx[edge2].P);
			PfxVector3 Q1 = closestPortal.vtx[edge2].Q + t1 * (closestPortal.vtx[edge1].Q - closestPortal.vtx[edge2].Q);

			PfxVector3 P2 = closestPortal.vtx[edge2].P + t2 * (closestPortal.vtx[edge3].P - closestPortal.vtx[edge2].P);
			PfxVector3 Q2 = closestPortal.vtx[edge2].Q + t2 * (closestPortal.vtx[edge3].Q - closestPortal.vtx[edge2].Q);

			if (lengthSqr(P1 - Q1) < lengthSqr(P2 - Q2)) {
				pA = P1;
				pB = Q1;
			}
			else {
				pA = P2;
				pB = Q2;
				edge1 = edge3;
			}

			if (candidateNormalLength < MPR_INVALID_NORMAL_LENGTH) {
				PfxVector3 S;
				pfxClosestPointLine(closestPortal.vtx[0].W, closestPortal.vtx[edge1].W, closestPortal.vtx[edge2].W - closestPortal.vtx[edge1].W, S);
				candidateNormal = closestPortal.vtx[0].W - S;
			}

			#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
			const PfxFloat eps = 0.001f;
			if (fabsf(t1) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, edge2);
				featureIdB = createFeatureIdVtx(closestPortal.qId, edge2);
			}
			else if(fabsf(1.0f - t1) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, edge1);
				featureIdB = createFeatureIdVtx(closestPortal.qId, edge1);
			}
			else {
				featureIdA = createFeatureIdEdge(closestPortal.pId, edge1, edge2);
				featureIdB = createFeatureIdEdge(closestPortal.qId, edge1, edge2);
			}
			#endif
		}
		else if (portalType == 2) {
			// triangle
			PfxVector3 tmp0 = closestPortal.vtx[2].W - closestPortal.vtx[1].W;
			PfxVector3 tmp1 = closestPortal.vtx[3].W - closestPortal.vtx[1].W;
			PfxVector3 tmp2 = closestPoint - closestPortal.vtx[1].W;
			PfxFloat d00 = dot(tmp0, tmp0);
			PfxFloat d01 = dot(tmp0, tmp1);
			PfxFloat d11 = dot(tmp1, tmp1);
			PfxFloat d20 = dot(tmp2, tmp0);
			PfxFloat d21 = dot(tmp2, tmp1);
			PfxFloat denom = d00 * d11 - d01 * d01;
			if (SCE_PFX_UNLIKELY(denom * denom < MPR_EPSILON * MPR_EPSILON)) {
				return kPfxGjkResultInvalid;
			}
			PfxFloat v = (d11 * d20 - d01 * d21) / denom;
			PfxFloat w = (d00 * d21 - d01 * d20) / denom;
			pA = closestPortal.vtx[1].P + v * (closestPortal.vtx[2].P - closestPortal.vtx[1].P) + w * (closestPortal.vtx[3].P - closestPortal.vtx[1].P);
			pB = closestPortal.vtx[1].Q + v * (closestPortal.vtx[2].Q - closestPortal.vtx[1].Q) + w * (closestPortal.vtx[3].Q - closestPortal.vtx[1].Q);

			if (candidateNormalLength < MPR_INVALID_NORMAL_LENGTH) {
				candidateNormal = cross(tmp1, tmp0);
			}

			#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
			const PfxFloat vv = fabsf(v);
			const PfxFloat ww = fabsf(w);
			const PfxFloat eps = 0.001f;
			// edge cases
			if (vv < eps && ww < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, 1);
				featureIdB = createFeatureIdVtx(closestPortal.qId, 1);
			}
			else if (fabsf(1.0f - v) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, 2);
				featureIdB = createFeatureIdVtx(closestPortal.qId, 2);
			}
			else if (fabsf(1.0f - w) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, 3);
				featureIdB = createFeatureIdVtx(closestPortal.qId, 3);
			}
			else if (vv < eps && ww > eps) {
				featureIdA = createFeatureIdEdge(closestPortal.pId, 1, 3);
				featureIdB = createFeatureIdEdge(closestPortal.qId, 1, 3);
			}
			else if (vv > eps && ww < eps) {
				featureIdA = createFeatureIdEdge(closestPortal.pId, 1, 2);
				featureIdB = createFeatureIdEdge(closestPortal.qId, 1, 2);
			}
			else if (fabsf(1.0f - v - w) < eps) {
				featureIdA = createFeatureIdEdge(closestPortal.pId, 2, 3);
				featureIdB = createFeatureIdEdge(closestPortal.qId, 2, 3);
			}
			else {
				featureIdA = createFeatureIdFacet(closestPortal.pId);
				featureIdB = createFeatureIdFacet(closestPortal.qId);
			}
			#endif
		}

		if (SCE_PFX_UNLIKELY(length(candidateNormal) < SCE_PFX_GJK_EPSILON)) {
			candidateNormal = PfxVector3::xAxis();
			distance = 0.0f;
		}
		else {
			candidateNormal = normalize(candidateNormal);
			distance = -candidateNormalLength;
		}

		pointA = PfxPoint3(pA);
		pointB = PfxPoint3(pB);
		normal = candidateNormal;
	}

	SCE_PFX_VALIDATE_VECTOR3(normal);
	SCE_PFX_VALIDATE_FLOAT(distance);

	return kPfxGjkResultOk;
}

template<typename SHAPE_A, typename SHAPE_B>
PfxInt32 PfxMpr<SHAPE_A, SHAPE_B>::collideRetry(PfxFloat &distance, PfxVector3& normal, PfxPoint3 &pointA, PfxPoint3 &pointB, PfxUInt32 &featureIdA, PfxUInt32 &featureIdB,
	const PfxTransform3 & transformA,
	const PfxTransform3 & transformB,
	PfxVector3 &cachedAxis,
	PfxFloat distanceThreshold)
{
	(void)distanceThreshold;

#ifndef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
	(void) featureIdA;
	(void) featureIdB;
#endif

	PfxTransform3 transformAB, transformBA;
	PfxMatrix3 matrixAB, matrixBA;
	PfxVector3 offsetAB, offsetBA;

	// Bローカル→Aローカルへの変換
	transformAB = inverse(transformA) * transformB;
	matrixAB = transformAB.getUpper3x3();
	offsetAB = transformAB.getTranslation();

	// Aローカル→Bローカルへの変換
	transformBA = inverse(transformB) * transformA;
	matrixBA = transformBA.getUpper3x3();
	offsetBA = transformBA.getTranslation();

	PfxMatrix3 matrixBA_n = transpose(inverse(matrixBA)); // 法線用回転マトリクス

	Portal portal[4];
	PfxVector3 closestPoints[4];
	PfxFloat portalDistance[4] = {
		SCE_PFX_FLT_MAX, SCE_PFX_FLT_MAX, SCE_PFX_FLT_MAX, SCE_PFX_FLT_MAX
	};

	PfxVector3 s, pInA, qInB, p, q; // Temporal for support function
	PfxUInt32 pId, qId;

	// early out check
	if (lengthSqr(cachedAxis) > 0.0f) {
		getSupportVertex(m_shapeA, cachedAxis, pInA, 0.0);
		getSupportVertex(m_shapeB, normalize(matrixBA_n * (-cachedAxis)), qInB, 0.0);
		p = pInA;
		q = offsetAB + matrixAB * qInB;
		PfxVector3 w = p - q;

		if (dot(cachedAxis, -w) > 0.0f) {
			distance = SCE_PFX_FLT_MAX;
			return kPfxGjkResultOk;
		}
	}

	for (int retry = 0; retry<4; retry++) {
		Portal &curPortal = portal[retry];

		// ポータル原点を作成
		if (retry == 0) {
			PfxVector3 centerA(0.0f);
			PfxVector3 centerB(0.0f);

			getCenter(m_shapeA, centerA);
			getCenter(m_shapeB, centerB);

			curPortal.vtx[0].W = centerA - offsetAB - matrixAB * centerB;

			if (SCE_PFX_UNLIKELY(lengthSqr(curPortal.vtx[0].W) < MPR_EPSILON)) return kPfxGjkResultInvalid;
		}
		else {
			const int ids[] = { 1, 2, 2, 3, 3, 1 };
			portal[retry].vtx[0].W = calcCandidatePoint(portal[0].vtx[0].W, portal[0].vtx[ids[2 * (retry - 1)]].W, portal[0].vtx[ids[2 * (retry - 1) + 1]].W);
		}

		PfxFloat distance_ = SCE_PFX_FLT_MAX;

		//J 初期ポータルの作成
		//E Create the initial portal
		PfxInt32 ret = createInitialPortal(curPortal, offsetAB, matrixAB, matrixBA_n, cachedAxis);
		if (ret != kPortalResultOk) {
			if (retry == 0) {
				distance = distance_;
				return kPfxGjkResultInvalid;
			}
			else {
				break;
			}
		}

		{
			int mpr_count = 0;
			bool skip = false;
			do {
				//J サーフェスに近い新たな点を求める
				//E Find new point close to the surface.
				s = normalize(cross(curPortal.vtx[2].W - curPortal.vtx[1].W, curPortal.vtx[3].W - curPortal.vtx[1].W));
				if (SCE_PFX_UNLIKELY(!isfinite((float)s[0]))) {
					if (retry == 0) {
						distance = distance_;
						return kPfxGjkResultInvalid;
					}
					else {
						skip = true;
						break;
					}
				}

				pId = getSupportVertex(m_shapeA, s, pInA, 0.0);
				qId = getSupportVertex(m_shapeB, normalize(matrixBA_n * (-s)), qInB, 0.0);
				p = pInA;
				q = offsetAB + matrixAB * qInB;
				PfxVector3 w = p - q;

				//J 原点とサポート平面の位置関係をチェック
				//E Check if the origin is outside of the support plane.
				if (dot(s, -w) > 0.0f) {
					if (retry == 0) {
						cachedAxis = s;
						distance = distance_;
						return kPfxGjkResultOk;
					}
					else {
						skip = true;
						break;
					}
				}

				//J ポータルが充分サーフェスに近づいたかどうかを判定
				//E Check distance between the portal and the surface.
				PfxFloat distanceToSurface = dot(s, w - curPortal.vtx[1].W);
				if (distanceToSurface < MPR_CLOSE_TO_SURFACE_DISTANCE) {
					break;
				}

				//J カウントチェック
				//E Check counts
				if ((mpr_count++) > MPR_MAX_ITERATION) {
					break;
				}

				//J ポータルのリファイン
				//E Portal refinement
				PfxVector3 cm = w - curPortal.vtx[0].W;
				PfxVector3 n1 = cross(cm, (curPortal.vtx[1].W - curPortal.vtx[0].W));
				PfxVector3 n2 = cross(cm, (curPortal.vtx[2].W - curPortal.vtx[0].W));
				PfxVector3 n3 = cross(cm, (curPortal.vtx[3].W - curPortal.vtx[0].W));
				PfxVector3 dd = transpose(PfxMatrix3(n1, n2, n3)) * -curPortal.vtx[0].W;

				int dst = 2;
				if (lengthSqr(n1) < MPR_EPSILON || (dd[1] > 0.0f && dd[2] <= 0.0f)) {
					dst = 1;
				}
				else if (lengthSqr(n3) < MPR_EPSILON || (dd[0] > 0.0f && dd[1] <= 0.0f)) {
					dst = 3;
				}
				//else if (lengthSqr(n2) < MPR_EPSILON || (dd[2] > 0.0f && dd[0] <= 0.0f)) {
				//	dst = 2;
				//}
				curPortal.vtx[dst].P = pInA;
				curPortal.vtx[dst].Q = qInB;
				curPortal.vtx[dst].W = w;
				curPortal.vtx[dst].pId = pId;
				curPortal.vtx[dst].qId = qId;
			} while (1);

			if (skip) continue;

			pfxClosestPointTriangle(PfxVector3::zero(), PfxTriangle(curPortal.vtx[1].W, curPortal.vtx[2].W, curPortal.vtx[3].W), closestPoints[retry]);

			portalDistance[retry] = lengthSqr(closestPoints[retry]);

			// There is a possiblity to detect better contact points. So retry portal refinement again by using the new origin.
			if (portalDistance[retry] < MPR_RETRY_DEPTH * MPR_RETRY_DEPTH) {
				break;
			}
		}
	}

	// Choose the best candidate for the contact
	int closestId = 0;
	PfxFloat closestDistance = portalDistance[0];
	for (int i = 1; i<4; i++) {
		if (portalDistance[i] < closestDistance) {
			closestDistance = portalDistance[i];
			closestId = i;
		}
	}
	PfxVector3 candidateNormal = -closestPoints[closestId];
	PfxFloat candidateNormalLength = length(candidateNormal);

	//J ポータルと原点の最近接点が衝突点になる
	//E Calculate the closest point as the contact point between the portal and the origin.
	{
		Portal &closestPortal = portal[closestId];
		PfxVector3 &closestPoint = closestPoints[closestId];

		PfxVector3 e1 = normalize(closestPortal.vtx[2].W - closestPortal.vtx[1].W);
		PfxVector3 e2 = normalize(closestPortal.vtx[3].W - closestPortal.vtx[2].W);
		PfxVector3 e3 = normalize(closestPortal.vtx[1].W - closestPortal.vtx[3].W);

		PfxFloat area = length(cross(closestPortal.vtx[2].W - closestPortal.vtx[1].W, closestPortal.vtx[3].W - closestPortal.vtx[1].W));

		PfxFloat angle1 = dot(-e3, e1);
		PfxFloat angle2 = dot(-e1, e2);
		PfxFloat angle3 = dot(-e2, e3);

		PfxVector3 pA(0.0f), pB(0.0f);

		int portalType = 0; // 0:point 1:edge 2:triangle
		int edge1 = 0, edge2 = 0, edge3 = 0;

		if (area < MPR_EPSILON) {
			portalType = 0;
		}
		else if (fabsf(1.0f - angle1) < MPR_DEGENERATE_ANGLE_THRESHOLD) {
			portalType = 1;
			edge1 = 2;
			edge2 = 1;
			edge3 = 3;
		}
		else if (fabsf(1.0f - angle2) < MPR_DEGENERATE_ANGLE_THRESHOLD) {
			portalType = 1;
			edge1 = 3;
			edge2 = 2;
			edge3 = 1;
		}
		else if (fabsf(1.0f - angle3) < MPR_DEGENERATE_ANGLE_THRESHOLD) {
			portalType = 1;
			edge1 = 1;
			edge2 = 3;
			edge3 = 2;
		}
		else {
			portalType = 2;
		}

		if (portalType == 0) {
			pA = closestPortal.vtx[1].P;
			pB = closestPortal.vtx[1].Q;
			candidateNormal = PfxVector3::zero();
			candidateNormalLength = 0.0f;
			#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
			featureIdA = createFeatureIdVtx(closestPortal.pId, 1);
			featureIdB = createFeatureIdVtx(closestPortal.qId, 1);
			#endif
		}
		else if (portalType == 1) {
			// edge
			PfxVector3 dir1 = closestPortal.vtx[edge1].W - closestPortal.vtx[edge2].W;
			PfxFloat t1 = divf(dot(closestPoint - closestPortal.vtx[edge2].W, dir1), dot(dir1, dir1));
			t1 = SCE_PFX_CLAMP(t1, 0.0f, 1.0f);
			PfxVector3 dir2 = closestPortal.vtx[edge3].W - closestPortal.vtx[edge2].W;
			PfxFloat t2 = divf(dot(closestPoint - closestPortal.vtx[edge2].W, dir2), dot(dir2, dir2));
			t2 = SCE_PFX_CLAMP(t2, 0.0f, 1.0f);

			PfxVector3 P1 = closestPortal.vtx[edge2].P + t1 * (closestPortal.vtx[edge1].P - closestPortal.vtx[edge2].P);
			PfxVector3 Q1 = closestPortal.vtx[edge2].Q + t1 * (closestPortal.vtx[edge1].Q - closestPortal.vtx[edge2].Q);

			PfxVector3 P2 = closestPortal.vtx[edge2].P + t2 * (closestPortal.vtx[edge3].P - closestPortal.vtx[edge2].P);
			PfxVector3 Q2 = closestPortal.vtx[edge2].Q + t2 * (closestPortal.vtx[edge3].Q - closestPortal.vtx[edge2].Q);

			if (lengthSqr(P1 - Q1) < lengthSqr(P2 - Q2)) {
				pA = P1;
				pB = Q1;
			}
			else {
				pA = P2;
				pB = Q2;
				edge1 = edge3;
			}

			if (candidateNormalLength < MPR_INVALID_NORMAL_LENGTH) {
				PfxVector3 S;
				pfxClosestPointLine(closestPortal.vtx[0].W, closestPortal.vtx[edge1].W, closestPortal.vtx[edge2].W - closestPortal.vtx[edge1].W, S);
				candidateNormal = closestPortal.vtx[0].W - S;
			}

			#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
			const PfxFloat eps = 0.001f;
			if (fabsf(t1) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, edge2);
				featureIdB = createFeatureIdVtx(closestPortal.qId, edge2);
			}
			else if(fabsf(1.0f - t1) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, edge1);
				featureIdB = createFeatureIdVtx(closestPortal.qId, edge1);
			}
			else {
				featureIdA = createFeatureIdEdge(closestPortal.pId, edge1, edge2);
				featureIdB = createFeatureIdEdge(closestPortal.qId, edge1, edge2);
			}
			#endif
		}
		else if (portalType == 2) {
			// triangle
			PfxVector3 tmp0 = closestPortal.vtx[2].W - closestPortal.vtx[1].W;
			PfxVector3 tmp1 = closestPortal.vtx[3].W - closestPortal.vtx[1].W;
			PfxVector3 tmp2 = closestPoint - closestPortal.vtx[1].W;
			PfxFloat d00 = dot(tmp0, tmp0);
			PfxFloat d01 = dot(tmp0, tmp1);
			PfxFloat d11 = dot(tmp1, tmp1);
			PfxFloat d20 = dot(tmp2, tmp0);
			PfxFloat d21 = dot(tmp2, tmp1);
			PfxFloat denom = d00 * d11 - d01 * d01;
			if (SCE_PFX_UNLIKELY(denom * denom < MPR_EPSILON * MPR_EPSILON)) {
				return kPfxGjkResultInvalid;
			}
			PfxFloat v = (d11 * d20 - d01 * d21) / denom;
			PfxFloat w = (d00 * d21 - d01 * d20) / denom;
			pA = closestPortal.vtx[1].P + v * (closestPortal.vtx[2].P - closestPortal.vtx[1].P) + w * (closestPortal.vtx[3].P - closestPortal.vtx[1].P);
			pB = closestPortal.vtx[1].Q + v * (closestPortal.vtx[2].Q - closestPortal.vtx[1].Q) + w * (closestPortal.vtx[3].Q - closestPortal.vtx[1].Q);

			if (candidateNormalLength < MPR_INVALID_NORMAL_LENGTH) {
				candidateNormal = cross(tmp1, tmp0);
			}

			#ifdef SCE_PFX_ENABLE_FEATURE_BASED_CONTACT_CACHE
			const PfxFloat vv = fabsf(v);
			const PfxFloat ww = fabsf(w);
			const PfxFloat eps = 0.001f;
			// edge cases
			if (vv < eps && ww < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, 1);
				featureIdB = createFeatureIdVtx(closestPortal.qId, 1);
			}
			else if (fabsf(1.0f - v) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, 2);
				featureIdB = createFeatureIdVtx(closestPortal.qId, 2);
			}
			else if (fabsf(1.0f - w) < eps) {
				featureIdA = createFeatureIdVtx(closestPortal.pId, 3);
				featureIdB = createFeatureIdVtx(closestPortal.qId, 3);
			}
			else if (vv < eps && ww > eps) {
				featureIdA = createFeatureIdEdge(closestPortal.pId, 1, 3);
				featureIdB = createFeatureIdEdge(closestPortal.qId, 1, 3);
			}
			else if (vv > eps && ww < eps) {
				featureIdA = createFeatureIdEdge(closestPortal.pId, 1, 2);
				featureIdB = createFeatureIdEdge(closestPortal.qId, 1, 2);
			}
			else if (fabsf(1.0f - v - w) < eps) {
				featureIdA = createFeatureIdEdge(closestPortal.pId, 2, 3);
				featureIdB = createFeatureIdEdge(closestPortal.qId, 2, 3);
			}
			else {
				featureIdA = createFeatureIdFacet(closestPortal.pId);
				featureIdB = createFeatureIdFacet(closestPortal.qId);
			}
			#endif
		}

		if (SCE_PFX_UNLIKELY(length(candidateNormal) < SCE_PFX_GJK_EPSILON)) {
			candidateNormal = PfxVector3::xAxis();
			distance = 0.0f;
		}
		else {
			candidateNormal = normalize(candidateNormal);
			distance = -candidateNormalLength;
		}

		pointA = PfxPoint3(pA);
		pointB = PfxPoint3(pB);
		normal = candidateNormal;
	}

	SCE_PFX_VALIDATE_VECTOR3(normal);
	SCE_PFX_VALIDATE_FLOAT(distance);

	return kPfxGjkResultOk;
}

template class PfxMpr<PfxBox, PfxBox>;
template class PfxMpr<PfxCylinder, PfxSphere>;
template class PfxMpr<PfxCylinder, PfxBox>;
template class PfxMpr<PfxCylinder, PfxCapsule>;
template class PfxMpr<PfxCylinder, PfxCylinder>;
template class PfxMpr<PfxCylinder, PfxConvexMeshImpl>;
template class PfxMpr<PfxSphere, PfxCylinder>;
template class PfxMpr<PfxBox, PfxCylinder>;
template class PfxMpr<PfxCapsule, PfxCylinder>;
template class PfxMpr<PfxConvexMeshImpl, PfxSphere>;
template class PfxMpr<PfxConvexMeshImpl, PfxCapsule>;
template class PfxMpr<PfxConvexMeshImpl, PfxCylinder>;
template class PfxMpr<PfxConvexMeshImpl, PfxConvexMeshImpl>;
template class PfxMpr<PfxSphere, PfxConvexMeshImpl>;
template class PfxMpr<PfxConvexMeshImpl, PfxBox>;
template class PfxMpr<PfxBox, PfxConvexMeshImpl>;
template class PfxMpr<PfxCapsule, PfxConvexMeshImpl>;
template class PfxMpr<PfxFatTriangle, PfxBox>;
template class PfxMpr<PfxFatTriangle, PfxCapsule>;
template class PfxMpr<PfxFatTriangle, PfxCylinder>;
template class PfxMpr<PfxFatTriangle, PfxConvexMeshImpl>;

} //namespace pfxv4
} //namespace sce
