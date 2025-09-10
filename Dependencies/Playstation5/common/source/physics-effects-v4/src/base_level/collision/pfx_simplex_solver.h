/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_SIMPLEX_SOLVER_H
#define _SCE_PFX_SIMPLEX_SOLVER_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"

#define CompareSigns(a,b)	((a > 1e-06f && b > 1e-06f) ? true : ((a < -1e-06f && b < -1e-06f) ? true : false))

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Voronoi Simplex Solver

struct SCE_PFX_ALIGNED(16) PfxBarycentricCoords {
	PfxVector3 closest;

	PfxFloat barycentricCoords[4];

	void reset()
	{
		barycentricCoords[0] = 0.0f;
		barycentricCoords[1] = 0.0f;
		barycentricCoords[2] = 0.0f;
		barycentricCoords[3] = 0.0f;
	}

	bool isValid()
	{
		return   (barycentricCoords[0] >= 0.0f) &&
				 (barycentricCoords[1] >= 0.0f) &&
				 (barycentricCoords[2] >= 0.0f) &&
				 (barycentricCoords[3] >= 0.0f);
	}

	void setBarycentricCoordinates(PfxFloat a,PfxFloat b,PfxFloat c,PfxFloat d)
	{
		barycentricCoords[0] = a;
		barycentricCoords[1] = b;
		barycentricCoords[2] = c;
		barycentricCoords[3] = d;
	}
};

class PfxSimplexSolver {
private:
	 static const int MAX_VERTS = 4;

public:
	int	numVertices;
	SCE_PFX_PADDING(1,12)

	PfxVector3	W[MAX_VERTS];
	PfxVector3	P[MAX_VERTS];
	PfxVector3	Q[MAX_VERTS];

	PfxBarycentricCoords bc;

	inline void	removeVertex(int index);
	inline void	reduceVertices ();
	bool	closestPointTriangleFromOrigin(const PfxVector3 &a,const  PfxVector3 &b,const  PfxVector3 &c,PfxBarycentricCoords& result);
	bool closestPointTetrahedronFromOrigin(const PfxVector3 &a,const  PfxVector3 &b,const  PfxVector3 &c,const  PfxVector3 &d,PfxBarycentricCoords& finalResult);
	bool originOutsideOfPlane(const PfxVector3& a, const PfxVector3& b, const PfxVector3& c, const PfxVector3& d);

public:
	void reset()
	{
		numVertices = 0;
		bc.reset();
	}

	inline void addVertex(const PfxVector3& w_, const PfxVector3& p_, const PfxVector3& q_);

	bool closest(PfxVector3& v)
	{
		// The old implementation of the GJK algorithm
		//return oldClosest(v);

		// The new implementation of the GJK algorithm
		return newClosest(v);
	}

	bool oldClosest(PfxVector3& v);

	bool fullSimplex() const
	{
		return (numVertices == 4);
	}
	
	bool inSimplex(const PfxVector3& w)
	{
		for(int i=0;i<numVertices;i++) {
			if(lengthSqr(W[i] - w) < (1e-04f * 1e-04f))
				return true;
		}
		return false;
	}
	
	void shiftQ(const PfxVector3& shift)
	{
		for(int i=0;i<numVertices;i++) {
			Q[i] += shift;
			W[i] = P[i] - Q[i];
		}
	}
	
	void getClosestPoints(PfxVector3 &p,PfxVector3 &q)
	{
		switch(numVertices) {
			case 1:
			p = P[0];
			q = Q[0];
			break;
			
			case 2:
			p = P[0] * bc.barycentricCoords[0] + P[1] * bc.barycentricCoords[1];
			q = Q[0] * bc.barycentricCoords[0] + Q[1] * bc.barycentricCoords[1];
			break;
			
			case 3:
			p = P[0] * bc.barycentricCoords[0] + P[1] * bc.barycentricCoords[1] + P[2] * bc.barycentricCoords[2]; 
			q = Q[0] * bc.barycentricCoords[0] + Q[1] * bc.barycentricCoords[1] + Q[2] * bc.barycentricCoords[2]; 
			break;
			
			case 4:
			p = P[0] * bc.barycentricCoords[0] + P[1] * bc.barycentricCoords[1] + P[2] * bc.barycentricCoords[2] + P[3] * bc.barycentricCoords[3];
			q = Q[0] * bc.barycentricCoords[0] + Q[1] * bc.barycentricCoords[1] + Q[2] * bc.barycentricCoords[2] + Q[3] * bc.barycentricCoords[3];
			break;
		}
	}

private:
	PfxFloat minAxisLengthSqr;

	// New implementation of calculating barycentric coordinate for 1-simplex
	void SCE_PFX_FORCE_INLINE S1D(const PfxUInt32 vi[], PfxUInt32 vo[], PfxFloat lambda[], PfxVector3 &v)
	{
		PfxVector3 dir(W[vi[1]] - W[vi[0]]);
		PfxFloat t = -(dot(W[vi[0]], dir) / dot(dir, dir));

		if (t <= 0.f) {
			PfxFloat lenSqr = lengthSqr(W[vi[0]]);
			if (lenSqr < minAxisLengthSqr) {
				minAxisLengthSqr = lenSqr;
				lambda[0] = 1.f;
				vo[0] = vi[0];
				v = W[vi[0]];
				numVertices = 1u;
			}
		}

		else if (t >= 1.f) {
			PfxFloat lenSqr = lengthSqr(W[vi[1]]);
			if (lenSqr < minAxisLengthSqr) {
				minAxisLengthSqr = lenSqr;
				lambda[0] = 1.f;
				vo[0] = vi[1];
				v = W[vi[1]];
				numVertices = 1u;
			}
		}

		else {
			PfxFloat tLambda[2] = { 1.f - t, t };
			PfxVector3 tV = W[vi[0]] + t * (W[vi[1]] - W[vi[0]]);
			PfxFloat lenSqr = lengthSqr(tV);
			if (lenSqr < minAxisLengthSqr)
			{
				lambda[0] = tLambda[0];
				lambda[1] = tLambda[1];

				vo[0] = vi[0];
				vo[1] = vi[1];

				v = tV;
				numVertices = 2u;
			}
		}
	}
	// New implementation of calculating barycentric coordinate for 2-simplex
	void SCE_PFX_FORCE_INLINE S2D(const PfxUInt32 vi[], PfxUInt32 vo[], PfxFloat lambda[], PfxVector3 &v)
	{
		PfxVector3 n(cross(W[vi[1]] - W[vi[0]], W[vi[2]] - W[vi[0]]));
		PfxVector3 po(n * ((dot(W[vi[0]], n) / dot(n, n))));

#ifdef PFX_ENABLE_AVX
		vec_float4 vec[ 3 ][ 3 ] = {
			{
				sce_vectormath_mul( sce_vectormath_shuffle2t<1,2,5,6>( W[ vi[ 1 ] ].get128(), po.get128() ), sce_vectormath_shuffle2t<2,1,6,5>( W[ vi[ 2 ] ].get128(), W[ vi[ 0 ] ].get128() ) ),
				sce_vectormath_mul( sce_vectormath_shuffle2t<1,2,5,6>( W[ vi[ 2 ] ].get128(), po.get128() ), sce_vectormath_shuffle2t<2,1,6,5>( W[ vi[ 0 ] ].get128(), W[ vi[ 1 ] ].get128() ) ),
				sce_vectormath_mul( sce_vectormath_shuffle2t<1,2,5,6>( W[ vi[ 0 ] ].get128(), po.get128() ), sce_vectormath_shuffle2t<2,1,6,5>( W[ vi[ 1 ] ].get128(), W[ vi[ 2 ] ].get128() ) )
			},
			{
				sce_vectormath_mul( sce_vectormath_shuffle2t<0,2,4,6>( W[ vi[ 1 ] ].get128(), po.get128() ), sce_vectormath_shuffle2t<2,0,6,4>( W[ vi[ 2 ] ].get128(), W[ vi[ 0 ] ].get128() ) ),
				sce_vectormath_mul( sce_vectormath_shuffle2t<0,2,4,6>( W[ vi[ 2 ] ].get128(), po.get128() ), sce_vectormath_shuffle2t<2,0,6,4>( W[ vi[ 0 ] ].get128(), W[ vi[ 1 ] ].get128() ) ),
				sce_vectormath_mul( sce_vectormath_shuffle2t<0,2,4,6>( W[ vi[ 0 ] ].get128(), po.get128() ), sce_vectormath_shuffle2t<2,0,6,4>( W[ vi[ 1 ] ].get128(), W[ vi[ 2 ] ].get128() ) )
			},
			{
				sce_vectormath_mul( sce_vectormath_shuffle2t<0,1,4,5>( W[ vi[ 1 ] ].get128(), po.get128() ), sce_vectormath_shuffle2t<1,0,5,4>( W[ vi[ 2 ] ].get128(), W[ vi[ 0 ] ].get128() ) ),
				sce_vectormath_mul( sce_vectormath_shuffle2t<0,1,4,5>( W[ vi[ 2 ] ].get128(), po.get128() ), sce_vectormath_shuffle2t<1,0,5,4>( W[ vi[ 0 ] ].get128(), W[ vi[ 1 ] ].get128() ) ),
				sce_vectormath_mul( sce_vectormath_shuffle2t<0,1,4,5>( W[ vi[ 0 ] ].get128(), po.get128() ), sce_vectormath_shuffle2t<1,0,5,4>( W[ vi[ 1 ] ].get128(), W[ vi[ 2 ] ].get128() ) )
			}
		};

		static const PfxUInt32 flagIndex[ 8 ] = {
			0u, // 0000
			0u, // 0001
			1u, // 0010
			0u, // 0011
			2u, // 0100
			0u, // 0101
			1u, // 0110
			0u, // 0111
		};

		vec_float4 vecSums1( sce_vectormath_add( sce_vectormath_add( sce_vectormath_shuffle2t<0, 1, 4, 5>( vec[ 0 ][ 0 ], vec[ 1 ][ 0 ] ), sce_vectormath_shuffle2t<0, 1, 4, 5>( vec[ 0 ][ 1 ], vec[ 1 ][ 1 ] ) ), sce_vectormath_shuffle2t<0, 1, 4, 5>( vec[ 0 ][ 2 ], vec[ 1 ][ 2 ] ) ) );
		vec_float4 vecSums2( sce_vectormath_add( sce_vectormath_add( vec[ 2 ][ 0 ], vec[ 2 ][ 1 ] ), vec[ 2 ][ 2 ] ) );
		PfxVector4 vecMu( sce_vectormath_sub( sce_vectormath_shuffle2t<0, 2, 4, 0>( vecSums1, vecSums2 ), sce_vectormath_shuffle2t<1, 3, 5, 1>( vecSums1, vecSums2 ) ) );

		vec_float4 vecAbsMu = sce_vectormath_abs( vecMu.get128() );
		vec_float4 vecAbsMaxMu( sce_vectormath_max( sce_vectormath_max( vecAbsMu, sce_vectormath_shuffle1t<1, 2, 0, 3>( vecAbsMu ) ), sce_vectormath_shuffle1t<2, 0, 1, 3>( vecAbsMu ) ) );
		PfxUInt32 maxVecInd( flagIndex[ _mm_movemask_ps( _mm_cmpeq_ps( sce_vectormath_asm128( vecAbsMu ), sce_vectormath_asm128( vecAbsMaxMu ) ) ) - 8 ] );
		PfxFloat maxMu = vecMu[ maxVecInd ];

		PfxVector3 CV( sce_vectormath_add( sce_vectormath_add(
			sce_vectormath_sub( sce_vectormath_shuffle1t<0, 3, 2, 1>( vec[ maxVecInd ][ 0 ] ), sce_vectormath_shuffle1t<1, 2, 3, 0>( vec[ maxVecInd ][ 0 ] ) ),
			sce_vectormath_sub( sce_vectormath_shuffle1t<2, 0, 3, 1>( vec[ maxVecInd ][ 1 ] ), sce_vectormath_shuffle1t<3, 1, 2, 0>( vec[ maxVecInd ][ 1 ] ) ) ),
			sce_vectormath_sub( sce_vectormath_shuffle1t<3, 2, 0, 1>( vec[ maxVecInd ][ 2 ] ), sce_vectormath_shuffle1t<2, 3, 1, 0>( vec[ maxVecInd ][ 2 ] ) ) ) );
#else
		// Find out which of the xy-, yz-, and xz- plane is the safest
		PfxFloat curSign = 1.f;
		PfxFloat maxMu = 0.f;
		PfxUInt32 maxJ, curK = 1u, curL = 2u;
		for (PfxUInt32 i = 0u; i < 3u; ++i) {
			PfxFloat mu = curSign * (
				W[vi[0]].getElem(curK) * W[vi[1]].getElem(curL) + W[vi[1]].getElem(curK) * W[vi[2]].getElem(curL) + W[vi[2]].getElem(curK) * W[vi[0]].getElem(curL) -
				W[vi[0]].getElem(curK) * W[vi[2]].getElem(curL) - W[vi[1]].getElem(curK) * W[vi[0]].getElem(curL) - W[vi[2]].getElem(curK) * W[vi[1]].getElem(curL));

			if (fabsf(mu) > fabsf(maxMu)) {
				maxMu = mu;
				maxJ = i;
			}

			curSign *= -1.f;
			curK = curL;
			curL = i;
		}
		PfxUInt32 maxX = ((maxJ == 0u) ? 1u : 0u);
		PfxUInt32 maxY = ((maxJ == 2u) ? 1u : 2u);

		// Compute the determinant and cofactors
		PfxVector3 CV;
		CV.setX(po.getElem(maxX) * W[vi[1]].getElem(maxY) + W[vi[1]].getElem(maxX) * W[vi[2]].getElem(maxY) + W[vi[2]].getElem(maxX) * po.getElem(maxY)
			  - po.getElem(maxX) * W[vi[2]].getElem(maxY) - W[vi[1]].getElem(maxX) * po.getElem(maxY) - W[vi[2]].getElem(maxX) * W[vi[1]].getElem(maxY));
		CV.setY(W[vi[0]].getElem(maxX) * po.getElem(maxY) + po.getElem(maxX) * W[vi[2]].getElem(maxY) + W[vi[2]].getElem(maxX) * W[vi[0]].getElem(maxY)
			  - W[vi[0]].getElem(maxX) * W[vi[2]].getElem(maxY) - po.getElem(maxX) * W[vi[0]].getElem(maxY) - W[vi[2]].getElem(maxX) * po.getElem(maxY));
		CV.setZ(W[vi[0]].getElem(maxX) * W[vi[1]].getElem(maxY) + W[vi[1]].getElem(maxX) * po.getElem(maxY) + po.getElem(maxX) * W[vi[0]].getElem(maxY)
			  - W[vi[0]].getElem(maxX) * po.getElem(maxY) - W[vi[1]].getElem(maxX) * W[vi[0]].getElem(maxY) - po.getElem(maxX) * W[vi[1]].getElem(maxY));
#endif

		PfxFloat C[3] = { CV.getX(), CV.getY(), CV.getZ() };

		// Compare the signs
		PfxBool sameSigns = true;

		if (!CompareSigns(maxMu, C[0])) {
			const PfxUInt32 indices[2] = { vi[1], vi[2] };
			sameSigns = false;
			S1D(indices, vo, lambda, v);
		}

		if (!CompareSigns(maxMu, C[1])) {
			const PfxUInt32 indices[2] = { vi[0], vi[2] };
			sameSigns = false;
			S1D(indices, vo, lambda, v);
		}

		if (!CompareSigns(maxMu, C[2])) {
			const PfxUInt32 indices[2] = { vi[0], vi[1] };
			sameSigns = false;
			S1D(indices, vo, lambda, v);
		}

		// Change the vertex indices
		if (sameSigns) {
			PfxFloat invMaxMu = 1.f / maxMu;
			PfxVector3 tV = (C[0] *= invMaxMu) * W[vi[0]] + (C[1] *= invMaxMu) * W[vi[1]] + (C[2] *= invMaxMu) * W[vi[2]];

			PfxFloat lenSqr = lengthSqr(tV);
			if (lenSqr < minAxisLengthSqr)
			{
				minAxisLengthSqr = lenSqr;
				for (PfxUInt32 i = 0u; i < 3u; ++i) {
					lambda[i] = C[i];
					vo[i] = vi[i];
				}

				v = tV;
				numVertices = 3u;
			}
		}
	}

	// New implementation of calculating barycentric coordinate for 3-simplex
	void SCE_PFX_FORCE_INLINE S3D(PfxUInt32 vo[], PfxFloat lambda[], PfxVector3 &v)
	{
		// Compute the determinant and cofactors
		PfxFloat C[4], detM;
		// C[0] = determinant(PfxMatrix3(W[1], W[2], W[3]));
		// C[1] = -determinant(PfxMatrix3(W[0], W[2], W[3]));
		// C[2] = determinant(PfxMatrix3(W[0], W[1], W[3]));
		// C[3] = -determinant(PfxMatrix3(W[0], W[1], W[2]));

		detM = C[0] = (dot(W[3], cross(W[1], W[2])));
		detM += C[1] = (dot(W[3], cross(W[2], W[0])));
		detM += C[2] = (dot(W[3], cross(W[0], W[1])));
		detM += C[3] = (dot(W[2], cross(W[1], W[0])));

		// Compare the signs
		PfxBool sameSigns = true;
		static const PfxUInt32 indices[4][3] = { { 1u, 2u, 3u },{ 0u, 2u, 3u },{ 0u, 1u, 3u },{ 0u, 1u, 2u } };

		for (PfxUInt32 i = 0u; i < 4u; ++i) {
			if (!CompareSigns(detM, C[i])) {
				sameSigns = false;
				S2D(indices[i], vo, lambda, v);
			}
		}

		// Change the set of vertices
		if (sameSigns) {
			PfxFloat invDetM = 1.f / detM;
			PfxVector3 tV = (C[0] *= invDetM) * W[0] + (C[1] *= invDetM) * W[1] + (C[2] *= invDetM) * W[2] + (C[3] *= invDetM) * W[3];

			PfxFloat lenSqr = lengthSqr(tV);
			if (lenSqr < minAxisLengthSqr) {
				minAxisLengthSqr = lenSqr;
				for (PfxUInt32 i = 0u; i < 4u; ++i) {
					lambda[i] = C[i];
					vo[i] = i;
				}
				v = tV;
				numVertices = 4u;
			}
		}
	}

	// The new implementation of the GJK algorithm
	bool SCE_PFX_FORCE_INLINE newClosest(PfxVector3& v)
	{
		static const PfxUInt32 indices[4] = { 0u, 1u, 2u, 3u };
		PfxUInt32 oIndices[4], tNumVertices = numVertices;

		bc.reset();
		minAxisLengthSqr = SCE_PFX_FLT_MAX;

		switch (numVertices) {
		case 0:
			return false;

		case 1:
		{
			bc.setBarycentricCoordinates(1.f, 0.f, 0.f, 0.f);
			v = W[0];
			return true;
		}
		break;

		case 2:
		{
			S1D(indices, oIndices, bc.barycentricCoords, v);
			return true;
		}

		case 3:
		{
			S2D(indices, oIndices, bc.barycentricCoords, v);
		}
		break;

		case 4:
		{
			S3D(oIndices, bc.barycentricCoords, v);
		}
		break;
		}

		if (tNumVertices != numVertices) {
			for (PfxUInt32 i = 0u; i < (PfxUInt32)numVertices; ++i) {
				W[i] = W[oIndices[i]];
				P[i] = P[oIndices[i]];
				Q[i] = Q[oIndices[i]];
			}
		}

		return true;
	}
};

inline
void	PfxSimplexSolver::removeVertex(int index)
{
	SCE_PFX_ASSERT(numVertices>0);
	numVertices--;
	W[index] = W[numVertices];
	P[index] = P[numVertices];
	Q[index] = Q[numVertices];
	bc.barycentricCoords[index] = bc.barycentricCoords[numVertices];
}

inline
void	PfxSimplexSolver::reduceVertices ()
{
	if ((numVertices >= 4) && (bc.barycentricCoords[3] == 0.0f))
		removeVertex(3);

	if ((numVertices >= 3) && (bc.barycentricCoords[2] == 0.0f))
		removeVertex(2);

	if ((numVertices >= 2) && (bc.barycentricCoords[1] == 0.0f))
		removeVertex(1);
	
	if ((numVertices >= 1) && (bc.barycentricCoords[0] == 0.0f))
		removeVertex(0);
}

inline
void PfxSimplexSolver::addVertex(const PfxVector3& w, const PfxVector3& p, const PfxVector3& q)
{
	W[numVertices] = w;
	P[numVertices] = p;
	Q[numVertices] = q;
	numVertices++;
}

inline
bool PfxSimplexSolver::originOutsideOfPlane(const PfxVector3& a, const PfxVector3& b, const PfxVector3& c, const PfxVector3& d)
{
	PfxVector3 normal = cross((b-a),(c-a));

	PfxFloat signp = dot(-a,normal);
	PfxFloat signd = dot((d - a),normal);

	return signp * signd < 0.0f;
}


} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_SIMPLEX_SOLVER_H
