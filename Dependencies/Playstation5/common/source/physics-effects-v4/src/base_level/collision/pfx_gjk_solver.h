/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_GJK_SOLVER_H
#define _SCE_PFX_GJK_SOLVER_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "pfx_simplex_solver.h"

#define ENABLE_MPR_FOR_PRIMITIVES

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Gjk

enum ePfxGjkResult {
	kPfxGjkResultOk = 0,
	kPfxGjkResultIntersect,
	kPfxGjkResultNoHit,
	kPfxGjkResultInvalid,
};

class PfxSimplexSolver;

template<typename SHAPE_A, typename SHAPE_B>
class PfxGjk
{
private:
	const SHAPE_A *m_shapeA;
	const SHAPE_B *m_shapeB;

	PfxSimplexSolver m_simplex;

	bool originInTetrahedron(const PfxVector3& p0,const PfxVector3& p1,const PfxVector3& p2,const PfxVector3& p3)
	{
		PfxVector3 n0 = cross((p1-p0),(p2-p0));
		PfxVector3 n1 = cross((p2-p1),(p3-p1));
		PfxVector3 n2 = cross((p3-p2),(p0-p2));
		PfxVector3 n3 = cross((p0-p3),(p1-p3));

		return
			dot(n0,p0) * dot(n0,p3-p0) <= 0.0f &&
			dot(n1,p1) * dot(n1,p0-p1) <= 0.0f &&
			dot(n2,p2) * dot(n2,p1-p2) <= 0.0f &&
			dot(n3,p3) * dot(n3,p2-p3) <= 0.0f;
	}

	PfxGjk() {}

public:
	PfxGjk(const SHAPE_A *sA, const SHAPE_B *sB) { m_shapeA = sA, m_shapeB = sB; }

	PfxInt32 closest(PfxFloat &distance, PfxVector3& normal, PfxPoint3 &pointA, PfxPoint3 &pointB,
					const PfxTransform3 & transformA,
					const PfxTransform3 & transformB,
					PfxFloat distanceThreshold = SCE_PFX_FLT_MAX);
};

///////////////////////////////////////////////////////////////////////////////
// GJK Sweep Algorithm

template<typename SHAPE_A, typename SHAPE_B>
class PfxGjkSweep
{
private:
	const SHAPE_A *m_shapeA;
	const SHAPE_B *m_shapeB;

	PfxSimplexSolver m_simplex;

	PfxGjkSweep() {}

public:
	PfxGjkSweep(const SHAPE_A *sA, const SHAPE_B *sB) { m_shapeA = sA, m_shapeB = sB; }

	PfxInt32 sweep(PfxFloat &lamda, PfxVector3& normal, PfxPoint3 &pointA, PfxPoint3 &pointB,
		const PfxTransform3 & transformA, const PfxVector3 &translationA,
		const PfxTransform3 & transformB, const PfxVector3 &translationB);
};

///////////////////////////////////////////////////////////////////////////////
// Minkowski Protal Refinement Algorithm

template<typename SHAPE_A, typename SHAPE_B>
class PfxMpr
{
private:
	const SHAPE_A *m_shapeA;
	const SHAPE_B *m_shapeB;

	struct PortalVertex {
		PfxVector3 W;
		PfxVector3 P;
		PfxVector3 Q;
		PfxUInt32 pId;
		PfxUInt32 qId;
	};

	struct Portal {
		PortalVertex vtx[4];
	};

	enum PortalResult {
		kPortalResultOk = 0,
		kPortalResultInvalidPortal,
	};

	PfxVector3 calcCandidatePoint(const PfxVector3 &p1, const PfxVector3 &p2, const PfxVector3 &p3)
	{
		PfxVector3 normal = pfxSafeNormalize(cross(p3 - p1, p2 - p1));
		PfxFloat closestLength = SCE_PFX_MAX(0.01f, dot(p1, normal));
		return -closestLength * normal;
	}

	PfxUInt32 createFeatureIdVtx(PfxUInt32 id[4], int v1)
	{
		return id[v1] == (PfxUInt32)-1 ? 0x80000000 : id[v1];
	};

	PfxUInt32 createFeatureIdEdge(PfxUInt32 id[4], int e1, int e2)
	{
		if (id[e1] == (PfxUInt32)-1) return 0x80000000;
		if (id[e1] > id[e2]) pfxSwap(id[e1], id[e2]);
		return id[e1] | (id[e2] << 8) | (1 << 24);
	};

	PfxUInt32 createFeatureIdFacet(PfxUInt32 id[4])
	{
		if (id[1] == (PfxUInt32)-1) return 0x80000000;
		if (id[1] > id[2]) pfxSwap(id[1], id[2]);
		if (id[2] > id[3]) pfxSwap(id[2], id[3]);
		if (id[1] > id[2]) pfxSwap(id[1], id[2]);
		if (id[1] == id[2] && id[1] == id[3]) {
			return id[1]; // Vertex
		}
		else if (id[1] == id[2]) {
			return id[1] | (id[3] << 8) | (1 << 24); // Edge
		}
		else if (id[2] == id[3]) {
			return id[1] | (id[2] << 8) | (1 << 24); // Edge
		}
		return id[1] | (id[2] << 8) | (id[3] << 16) | (2 << 24); // Facet
	}

	// Both shapes are in the same coordinates
	PfxInt32 createInitialPortal(Portal &portal, PfxVector3 &cachedAxis);

	PfxInt32 createInitialPortal(Portal &portal, const PfxVector3 &offsetAB, const PfxMatrix3 &matrixAB, const PfxMatrix3 &matrixBA_n,PfxVector3 &cachedAxis);

	PfxMpr() {}

public:
	PfxMpr(const SHAPE_A *sA, const SHAPE_B *sB) { m_shapeA = sA, m_shapeB = sB; }

	// Both shapes are in the same coordinates
	PfxInt32 collide(PfxFloat &distance, PfxVector3& normal,
		PfxPoint3 &pointA, PfxPoint3 &pointB,PfxUInt32 &featureIdA, PfxUInt32 &featureIdB,
		PfxVector3 &cachedAxis,
		PfxFloat distanceThreshold = SCE_PFX_FLT_MAX);
	
	PfxInt32 collide(PfxFloat &distance, PfxVector3& normal,
		PfxPoint3 &pointA, PfxPoint3 &pointB,PfxUInt32 &featureIdA, PfxUInt32 &featureIdB,
		const PfxTransform3 & transformA,
		const PfxTransform3 & transformB,
		PfxVector3 &cachedAxis,
		PfxFloat distanceThreshold = SCE_PFX_FLT_MAX);

	PfxInt32 collideRetry(PfxFloat &distance, PfxVector3& normal,
		PfxPoint3 &pointA, PfxPoint3 &pointB,PfxUInt32 &featureIdA, PfxUInt32 &featureIdB,
		PfxVector3 &cachedAxis,
		PfxFloat distanceThreshold = SCE_PFX_FLT_MAX);

	PfxInt32 collideRetry(PfxFloat &distance, PfxVector3& normal,
		PfxPoint3 &pointA, PfxPoint3 &pointB,PfxUInt32 &featureIdA, PfxUInt32 &featureIdB,
		const PfxTransform3 & transformA,
		const PfxTransform3 & transformB,
		PfxVector3 &cachedAxis,
		PfxFloat distanceThreshold = SCE_PFX_FLT_MAX);
};

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_GJK_SOLVER_H
