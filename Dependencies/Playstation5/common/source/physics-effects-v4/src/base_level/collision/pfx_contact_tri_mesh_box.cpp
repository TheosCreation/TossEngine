/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "pfx_contact_tri_mesh_box.h"
#include "pfx_intersect_common.h"
#include "pfx_mesh_common.h"
#include "pfx_decoded_tri_mesh.h"
#include "pfx_gjk_solver.h"
#include "pfx_gjk_support_func.h"

namespace sce {
namespace pfxv4 {

// checkNormal : skip this axis if it has an opposite direction of a facet normal
inline bool CHECK_SAT(
	const PfxVector3 &axis,PfxFloat AMin,PfxFloat AMax,PfxFloat BMin,PfxFloat BMax,int Type,
	const PfxVector3 &facetNormal, PfxFloat &distMin, PfxVector3 &axisMin, int &feature, bool &invalid)
{
	PfxFloat d1 = AMin - BMax;
	PfxFloat d2 = BMin - AMax;
	PfxFloat checkNormal = dot(-facetNormal, axis);
	if(distMin < d1 /* && checkNormal >= 0.0f */) {
		distMin = d1;
		axisMin = axis;
		feature = Type;
		invalid = checkNormal < 0.0f;
	}
	if(distMin < d2 /* && -checkNormal >= 0.0f */) {
		distMin = d2;
		axisMin = -axis;
		feature = Type;
		invalid = -checkNormal < 0.0f;
	}
	if(d1 > 0.0f || d2 > 0.0f) {
		return false;
	}
	return true;
}

#ifndef ENABLE_MPR_FOR_PRIMITIVES

static SCE_PFX_FORCE_INLINE
void projection(const PfxVector3 &boxHalf,const PfxVector3 &axis,PfxFloat &boxMin,PfxFloat &boxMax)
{
	PfxMatrix3 mtmp(PfxMatrix3::scale(boxHalf));
	PfxVector3 vtmp(absPerElem(mtmp * axis));
	PfxFloat boxHalf_ = sum(vtmp);
	boxMin = - boxHalf_;
	boxMax =   boxHalf_;
}

static bool pfxContactTriangleBox(PfxContactCache &contacts,PfxUInt32 facetId,
							const PfxVector3 &normal,const PfxVector3 &p0,const PfxVector3 &p1,const PfxVector3 &p2,
							const PfxFloat thickness,const PfxVector3 &edgeAngle,PfxUInt32 edgeChk,
							const PfxBox &box)
{
	const PfxFloat epsilon = 0.00001f;
	const PfxVector3 boxHalfLocal = box.m_half;

	// 最も浅い貫通深度とそのときの分離軸
	PfxFloat distMin = -SCE_PFX_FLT_MAX;
	PfxVector3 axisMin(0.0f);
	int feature = 0; // 0:facet A->B 1:facet B->A 2:edge
	bool invalid = false;

	//-------------------------------------------
	// １．分離軸判定

	{
		PfxVector3 facetPnts[6] = {
			p0,p1,p2,p0-thickness*normal,p1-thickness*normal,p2-thickness*normal
		};

		PfxVector3 sideNml[3] = {
			normalize(cross((facetPnts[1] - facetPnts[0]),normal)),
			normalize(cross((facetPnts[2] - facetPnts[1]),normal)),
			normalize(cross((facetPnts[0] - facetPnts[2]),normal)),
		};

		bool insideThickness = false;

		// Trianglesの面 -> Box
		{
			// 分離軸
			const PfxVector3 sepAxis = normal;

			// 分離平面
			//PfxPlane planeA(sepAxis,p0);

			// Boxを分離軸に投影して範囲を取得
			PfxFloat boxMax,boxMin;
			projection(boxHalfLocal,sepAxis,boxMin,boxMax);
			PfxFloat offset = dot(-p0,sepAxis);
			boxMax += offset;
			boxMin += offset;

			// 判定
			if(boxMin > 0.0f || boxMax < -thickness) {
				return false;
			}

			if(thickness > SCE_PFX_THICKNESS_THRESHOLD && boxMax < 0.0f) {
				insideThickness = true;
			}

			if(distMin < boxMin) {
				distMin = boxMin;
				axisMin = -sepAxis;
				feature = 1;
			}
		}

		// Box -> Triangles
		if(!insideThickness) {

		for(int bf=0;bf<3;bf++) {
			// 分離軸
			PfxVector3 sepAxis(0.0f);
			sepAxis[bf] = 1.0f;

			// Trianglesを分離軸に投影して範囲を取得
			PfxFloat triMin,triMax;
			pfxGetProjAxisPnts6(facetPnts,sepAxis,triMin,triMax);

			// Boxを分離軸に投影して範囲を取得
			PfxFloat boxMin = - boxHalfLocal[bf];
			PfxFloat boxMax =   boxHalfLocal[bf];

			if (!CHECK_SAT(sepAxis, triMin, triMax, boxMin, boxMax, 2, normal, distMin, axisMin, feature, invalid)) {
				return false;
			}
		}

		// エッジ Triangles面のエッジ(x3)×Boxのエッジ(x3)
		for(int e=0;e<3;e++) {
			PfxVector3 dir = normalize(facetPnts[(e+1)%3] - facetPnts[e]);

			// 凸以外エッジの判定も、当たらないことを確定するために必要
			//if(((edgeChk>>(e*2))&0x03) != 1) continue;

			for(int i=0;i<3;i++) {
				PfxVector3 boxEdge(0.0f);
				boxEdge[i] = 1.0f;

				// エッジが平行であれば判定しない
				if(pfxIsSameDirection(dir,boxEdge)) continue;

				PfxVector3 sepAxis = normalize(cross(dir,boxEdge));

#if 0
				// 分離軸が面の外側に向くように調整
				if(dot(sepAxis,facetPnts[(e+2)%3] - facetPnts[e]) > 0.0f) {
					sepAxis = -sepAxis;
				}

				// 分離平面
				//PfxPlane planeA(sepAxis,facetPnts[e]);

				// Boxを分離軸に投影して範囲を取得
				PfxFloat boxMax,boxMin;
				projection(boxHalfLocal,boxPos-facetPnts[e],boxOri,sepAxis,boxMin,boxMax);

				// 判定
				if(boxMin > 0.0f) {
					return false;
				}

				if(distMin < boxMin) {
					distMin = boxMin;
					axisMin = -sepAxis;
				}
#else
				// Trianglesを分離軸に投影して範囲を取得
				PfxFloat triMin,triMax;
				pfxGetProjAxisPnts6(facetPnts,sepAxis,triMin,triMax);

				// Boxを分離軸に投影して範囲を取得
				PfxFloat boxMax,boxMin;
				projection(boxHalfLocal,sepAxis,boxMin,boxMax);

				if (!CHECK_SAT(sepAxis, triMin, triMax, boxMin, boxMax, 4, normal, distMin, axisMin, feature, invalid)) {
					return false;
				}
#endif
			}
		}

		} // if(!insideThickness)

		// 面に厚みがある場合の補助的な判定（交差するかしないかだけを判定）
		if(thickness > SCE_PFX_THICKNESS_THRESHOLD) {
			// 厚み側面の法線
			for(int i=0;i<3;i++) {
				// 分離平面
				//PfxPlane planeA(sideNml[i],facetPnts[i]);

				// Boxを分離軸に投影して範囲を取得
				PfxFloat r = dot(boxHalfLocal,absPerElem(sideNml[i]));
				//PfxFloat boxOffset = planeA.onPlane(boxPos);
				PfxFloat boxOffset = dot(-facetPnts[i],sideNml[i]);
				PfxFloat boxMin = boxOffset - r;

				if(boxMin > 0.0f) {
					return false;
				}
			}

			// ２つの厚み側面のなすエッジ3×ボックスのエッジ3
			for(int e=0;e<3;e++) {
				PfxVector3 edgeVec = normalize(cross(sideNml[(e+1)%3],sideNml[e]));
				//PfxVector3 chkNml = sideNml[(e+1)%3] + sideNml[e];

				for(int i=0;i<3;i++) {
					PfxVector3 boxEdge(0.0f);
					boxEdge[i] = 1.0f;

					// エッジが平行であれば判定しない
					if(pfxIsSameDirection(edgeVec,boxEdge)) continue;

					PfxVector3 sepAxis = normalize(cross(edgeVec,boxEdge));

#if 0
					// 分離軸が外側に向くように調整
					if(dot(sepAxis,chkNml) < 0.0f) {
						sepAxis = -sepAxis;
					}

					// 分離平面
					PfxPlane planeA(sepAxis,facetPnts[(e+1)%3]);

					// Boxを分離軸に投影して範囲を取得
					PfxFloat boxMax,boxMin;
					projection(boxHalfLocal,boxPos-facetPnts[(e+1)%3],boxOri,sepAxis,boxMin,boxMax);

					if(boxMin > 0.0f) {
						return false;
					}
#else
					// Trianglesを分離軸に投影して範囲を取得
					PfxFloat triMin,triMax;
					pfxGetProjAxisPnts3(facetPnts,sepAxis,triMin,triMax);

					// Boxを分離軸に投影して範囲を取得
					PfxFloat boxMax,boxMin;
					projection(boxHalfLocal,sepAxis,boxMin,boxMax);

					if(triMax < boxMin || boxMax < triMin) {
						return false;
					}
#endif
				}
			}
		}
	}

	// It returns false, if the closest feature is invalid. (It means a normal direction is opposite to a facet normal)
	if(invalid) return false;

	//-------------------------------------------
	// ２．最近接面の探索

	int faceB=0; // X
	int signB=1; // +

	{
		PfxFloat f,maxf;

		// f = dot(axisMin,boxOri.getCol0());
		// maxf=f;faceB=0;signB=1;
		// f = dot(axisMin,boxOri.getCol1());
		// if(maxf < f) {maxf=f;faceB=1;signB=1;}
		// f = dot(axisMin,boxOri.getCol2());
		// if(maxf < f) {maxf=f;faceB=2;signB=1;}
		// f = dot(axisMin,-boxOri.getCol0());
		// if(maxf < f) {maxf=f;faceB=0;signB=-1;}
		// f = dot(axisMin,-boxOri.getCol1());
		// if(maxf < f) {maxf=f;faceB=1;signB=-1;}
		// f = dot(axisMin,-boxOri.getCol2());
		// if(maxf < f) {maxf=f;faceB=2;signB=-1;}

		f = axisMin[0];
		maxf=f;faceB=0;signB=1;
		f = axisMin[1];
		if(maxf < f) {maxf=f;faceB=1;signB=1;}
		f = axisMin[2];
		if(maxf < f) {maxf=f;faceB=2;signB=1;}
		f = -axisMin[0];
		if(maxf < f) {maxf=f;faceB=0;signB=-1;}
		f = -axisMin[1];
		if(maxf < f) {maxf=f;faceB=1;signB=-1;}
		f = -axisMin[2];
		if(maxf < f) {maxf=f;faceB=2;signB=-1;}
	}

	//-------------------------------------------
	// ３．衝突点の探索

	{
		// 分離軸方向に引き離す(最近接を判定するため、交差回避させる)
		PfxVector3 sepAxis = 1.1f * fabsf(distMin) * axisMin;
		PfxMatrix3 base = PfxMatrix3::identity();

		const PfxVector3 facetPnts[3] = {
			p0 + sepAxis,
			p1 + sepAxis,
			p2 + sepAxis,
		};

		const PfxVector3 boxPnts[4] = {
			(float)signB * boxHalfLocal[faceB] * base[faceB] +
			(float)signB * boxHalfLocal[(faceB+1)%3] * base[(faceB+1)%3] +
			(float)signB * boxHalfLocal[(faceB+2)%3] * base[(faceB+2)%3],
			(float)signB * boxHalfLocal[faceB] * base[faceB] +
			(float)signB * -boxHalfLocal[(faceB+1)%3] * base[(faceB+1)%3] +
			(float)signB * boxHalfLocal[(faceB+2)%3] * base[(faceB+2)%3],
			(float)signB * boxHalfLocal[faceB] * base[faceB] +
			(float)signB * -boxHalfLocal[(faceB+1)%3] * base[(faceB+1)%3] +
			(float)signB * -boxHalfLocal[(faceB+2)%3] * base[(faceB+2)%3],
			(float)signB * boxHalfLocal[faceB] * base[faceB] +
			(float)signB * boxHalfLocal[(faceB+1)%3] * base[(faceB+1)%3] +
			(float)signB * -boxHalfLocal[(faceB+2)%3] * base[(faceB+2)%3],
		};

		//--------------------------------------------------------------------
		// 衝突点の探索

		PfxClosestPoints cp;
		PfxVector3 sA,sB;

		// エッジ間の最短距離と座標値を算出
		if((feature & 4) != 0) {
			for(int i=0;i<3;i++) {
				for(int j=0;j<4;j++) {
					pfxClosestTwoLines(facetPnts[i],facetPnts[(i+1)%3],boxPnts[j],boxPnts[(j+1)%4],sA,sB);
					cp.add(PfxPoint3(sA),PfxPoint3(sB),lengthSqr(sA-sB));
				}
			}
		}

		// Triangleの頂点 -> Boxの面
		if((feature & 2) != 0) {
			pfxClosestPointAABB(PfxVector3(facetPnts[0]), boxHalfLocal,sB);
			cp.add(PfxPoint3(facetPnts[0]),PfxPoint3(sB),lengthSqr(sB-facetPnts[0]));

			pfxClosestPointAABB(facetPnts[1], boxHalfLocal,sB);
			cp.add(PfxPoint3(facetPnts[1]),PfxPoint3(sB),lengthSqr(sB-facetPnts[1]));

			pfxClosestPointAABB(facetPnts[2], boxHalfLocal,sB);
			cp.add(PfxPoint3(facetPnts[2]),PfxPoint3(sB),lengthSqr(sB-facetPnts[2]));
		}

		// Boxの頂点 -> Trianglesの面
		if((feature & 1) != 0) {
			PfxTriangle triangleA(facetPnts[0],facetPnts[1],facetPnts[2]);
			for(int i=0;i<4;i++) {
				pfxClosestPointTriangle(boxPnts[i],triangleA,sA);
				cp.add(PfxPoint3(sA),PfxPoint3(boxPnts[i]),lengthSqr(sA-boxPnts[i]));
			}
		}

		for(int i=0;i<cp.numPoints;i++) {
			if(cp.distSqr[i] < cp.closestDistSqr + epsilon) {
				cp.pA[i] -= sepAxis;

				// 衝突点が平坦なエッジ上であれば法線を変える
				if (pfxPointIsOnTriangleEdge(PfxVector3(cp.pA[i]), edgeChk, p0, p1, p2)) {
					axisMin=-normal;
				}

				PfxSubData subData;
				subData.setFacetId(facetId);
				contacts.addContactPoint(-length(cp.pB[i]-cp.pA[i]),axisMin,cp.pA[i],cp.pB[i],subData,PfxSubData());
			}
		}
	}

	return true;
}

#else // ENABLE_MPR_FOR_PRIMITIVES

bool pfxContactTriangleBox(PfxContactCache &contacts,PfxUInt32 facetId,
							const PfxVector3 &normal,const PfxVector3 &p0,const PfxVector3 &p1,const PfxVector3 &p2,
							const PfxFloat thickness,const PfxVector3 &edgeAngle,PfxUInt32 edgeChk,
							const PfxBox &box)
{
	{
		// 早期チェック + 裏側の誤判定を防ぐ
		PfxVector3 sA(0.0f);
		getSupportVertex(&box, normal, sA, 0.0f);
		PfxFloat projA = dot(sA, normal);
		PfxFloat projB = dot(p0, normal);
		if (projA + thickness < projB) return false;

		// 面法線を分離軸にとる
		getSupportVertex(&box, -normal, sA, 0.0f);
		projA = dot(sA, normal);

		if (projA > projB) return false; // 面より上にある
	}

	// 膨らませる
	float fat = (length(p0 - p1) + length(p1 - p2) + length(p2 - p0)) * 0.033f;
	PfxVector3 facetPnts[4] = {
		p0, p1, p2, (p0 + p1 + p2) / 3.0f - fat * normal,
	};

	PfxPoint3 pA(0.0f), pB(0.0f);
	PfxVector3 nml(0.0f);

	PfxFatTriangle fatTriangle;
	fatTriangle.points[0] = facetPnts[0];
	fatTriangle.points[1] = facetPnts[1];
	fatTriangle.points[2] = facetPnts[2];
	fatTriangle.points[3] = facetPnts[3];

	PfxMpr<PfxFatTriangle, PfxBox> collider(&fatTriangle, &box);

	PfxFloat d;
	PfxVector3 cachedAxis(0.0f);
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	PfxInt32 ret = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, cachedAxis, SCE_PFX_FLT_MAX);
	if (ret != kPfxGjkResultOk || d >= 0.0f) return false;

	PfxVector3 pointsOnTriangle = PfxVector3(pA);
	PfxVector3 axis = nml;

	// Ignore a contact if a normal direction is opposed to a facet normal
	if (dot(-normal, axis) < 0.0f) return false;

	// 面上の最近接点が凸エッジ上でない場合は法線を変える
	if (pfxPointIsOnTriangleEdge(pointsOnTriangle, edgeChk, p0, p1, p2)) {
		axis = -normal;
	}

	PfxSubData subData;
	subData.setFacetId(facetId);
	contacts.addContactPoint(d, axis, pA, pB, featureIdA, featureIdB, subData, PfxSubData());

	return true;
}

#endif // ENABLE_MPR_FOR_PRIMITIVES

template<>
PfxInt32 pfxContactTriMesh<PfxBox, PfxExpandedTriMesh> (
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA,const PfxExpandedTriMesh *meshA,bool flipTriangle,
	const PfxBox &boxB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold)
{
	(void) largeMeshA;
	(void) distanceThreshold;

	PfxTransform3 transformBA = inverse(transformB);
	PfxMatrix3 rotationBA_n = transpose(inverse(transformBA.getUpper3x3()));

	//-------------------------------------------
	// 判定する面を絞り込む

	PfxUInt8 selFacets[SCE_PFX_NUMMESHFACETS] = {0};
	PfxUInt32 numSelFacets = pfxGatherFacets(meshA,
		transformB.getTranslation(),
		absPerElem(transformB.getUpper3x3()) * boxB.m_half,selFacets);

	if(numSelFacets == 0) {
		return 0;
	}

	PfxContactCache localContacts;

	int vi[3] = {0,1,2};
	int ei[3] = {0,1,2};
	if(flipTriangle) {vi[0]=2;vi[2]=0;ei[0]=1;ei[1]=0;}

	// In a box coordinate
	for(PfxUInt32 f = 0; f < numSelFacets; f++) {
		const PfxExpandedFacet &facet = meshA->m_facets[selFacets[f]];

		PfxVector3 facetNormal = normalize(rotationBA_n * facet.m_normal);

		PfxVector3 facetPntsA[3] = {
			transformBA.getTranslation() + transformBA.getUpper3x3() * meshA->m_verts[facet.m_vertIds[vi[0]]],
			transformBA.getTranslation() + transformBA.getUpper3x3() * meshA->m_verts[facet.m_vertIds[vi[1]]],
			transformBA.getTranslation() + transformBA.getUpper3x3() * meshA->m_verts[facet.m_vertIds[vi[2]]],
		};

		const PfxEdge *edge[3] = {
			&meshA->m_edges[facet.m_edgeIds[ei[0]]],
			&meshA->m_edges[facet.m_edgeIds[ei[1]]],
			&meshA->m_edges[facet.m_edgeIds[ei[2]]],
		};

		PfxUInt32 edgeChk = edge[0]->m_angleType | (edge[1]->m_angleType << 2) | (edge[2]->m_angleType << 4);

		PfxVector3 edgeAngle = 0.5f*SCE_PFX_PI/255.0f*PfxVector3(edge[0]->m_tilt,edge[1]->m_tilt,edge[2]->m_tilt);

		pfxContactTriangleBox(localContacts,selFacets[f],
							facetNormal,facetPntsA[0],facetPntsA[1],facetPntsA[2],
							facet.m_thickness,edgeAngle,edgeChk,boxB.m_half);
	}

	PfxMatrix3 rotationB_n = transpose(inverse(transformB.getUpper3x3()));

	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subData = localContacts.getContactSubDataA(i);

		const PfxExpandedFacet &facet = meshA->m_facets[subData.getFacetId()];

		PfxTriangle triangleA(
			meshA->m_verts[facet.m_vertIds[vi[0]]],
			meshA->m_verts[facet.m_vertIds[vi[1]]],
			meshA->m_verts[facet.m_vertIds[vi[2]]]);

		PfxFloat s=0.0f,t=0.0f;
		pfxCalcBarycentricCoords(PfxVector3(localContacts.getContactLocalPointA(i)),triangleA,s,t);
		subData.setType(PfxSubData::MESH_INFO);
		subData.setFacetLocalS(s);
		subData.setFacetLocalT(t);
		subData.setUserData(facet.m_userData);

		PfxUInt16 islandId = (PfxUInt16)(meshA - (PfxExpandedTriMesh*)largeMeshA->m_islands);
		PfxUInt64 featureIdA = ((PfxUInt64)islandId << 32) | localContacts.getFeatureIdA(i);

		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			rotationB_n * localContacts.getContactNormal(i),
			transformB * localContacts.getContactLocalPointA(i),
			localContacts.getContactLocalPointB(i),
			featureIdA, localContacts.getFeatureIdB(i),
			subData,PfxSubData());
	}

	return contacts.getNumContactPoints();
}

template<>
PfxInt32 pfxContactTriMesh<PfxBox, PfxQuantizedTriMeshBvh> (
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA,const PfxQuantizedTriMeshBvh *meshA,bool flipTriangle,
	const PfxBox &boxB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold)
{
	(void) distanceThreshold;

	PfxTransform3 transformBA = inverse(transformB);
	PfxMatrix3 rotationBA_n = transpose(inverse(transformBA.getUpper3x3()));

	PfxVector3 center = transformB.getTranslation();
	PfxVector3 half = absPerElem(transformB.getUpper3x3()) * boxB.m_half;
	PfxUInt8 selFacets[SCE_PFX_NUMMESHFACETS] = {0};
	PfxUInt32 numSelFacets = pfxGatherFacets(meshA,center - half,center + half,selFacets);

	if(numSelFacets == 0) {
		return 0;
	}

	PfxDecodedTriMesh decodedMesh;
	PfxContactCache localContacts;

	int vi[3] = {0,1,2};
	int ei[3] = {0,1,2};
	if(flipTriangle) {vi[0]=2;vi[2]=0;ei[0]=1;ei[1]=0;}

	for(PfxUInt32 f = 0; f < numSelFacets; f++) {
		//-------------------------------------------
		// 交差した面との分離軸判定(SAT)
		// ※Box座標系 (Bローカル)で判定

		const PfxQuantizedFacetBvh &facet = meshA->m_facets[selFacets[f]];

		// デコード
		PfxDecodedFacet decodedFacet;
		const PfxUInt32 vId[3] = {facet.m_vertIds[vi[0]],facet.m_vertIds[vi[1]],facet.m_vertIds[vi[2]]};

		decodedFacet.m_normal = normalize(rotationBA_n * largeMeshA->decodeNormal(facet.m_normal));
		decodedFacet.m_thickness = largeMeshA->decodeFloat(facet.m_thickness);

		for(int v=0;v<3;v++) {
			if(!decodedMesh.isDecodedVertex(vId[v])) {
				PfxVector3 vert = largeMeshA->decodePosition(meshA->m_verts[vId[v]]);
				decodedMesh.m_verts[vId[v]] = transformBA.getTranslation() + transformBA.getUpper3x3() * vert;
				decodedMesh.m_decodedVertex[vId[v]>>5] |= 1 << (vId[v]&0x1f);
			}
		}

		PfxVector3 facetNormal = decodedFacet.m_normal;

		PfxVector3 facetPntsA[3] = {
			decodedMesh.m_verts[vId[0]],
			decodedMesh.m_verts[vId[1]],
			decodedMesh.m_verts[vId[2]],
		};

		const PfxEdge *edge[3] = {
			&meshA->m_edges[facet.m_edgeIds[ei[0]]],
			&meshA->m_edges[facet.m_edgeIds[ei[1]]],
			&meshA->m_edges[facet.m_edgeIds[ei[2]]],
		};

		PfxUInt32 edgeChk = edge[0]->m_angleType | (edge[1]->m_angleType << 2) | (edge[2]->m_angleType << 4);

		PfxVector3 edgeAngle = 0.5f*SCE_PFX_PI/255.0f*PfxVector3(edge[0]->m_tilt,edge[1]->m_tilt,edge[2]->m_tilt);

		pfxContactTriangleBox(localContacts,selFacets[f],
							facetNormal,facetPntsA[0],facetPntsA[1],facetPntsA[2],
							decodedFacet.m_thickness,edgeAngle,edgeChk,boxB);
	}

	PfxMatrix3 rotationB_n = transpose(inverse(transformB.getUpper3x3()));

	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subData = localContacts.getContactSubDataA(i);

		const PfxQuantizedFacetBvh &facet = meshA->m_facets[subData.getFacetId()];

		PfxTriangle triangleA(
			decodedMesh.m_verts[facet.m_vertIds[vi[0]]],
			decodedMesh.m_verts[facet.m_vertIds[vi[1]]],
			decodedMesh.m_verts[facet.m_vertIds[vi[2]]]);

		PfxFloat s=0.0f,t=0.0f;
		pfxCalcBarycentricCoords(PfxVector3(localContacts.getContactLocalPointA(i)),triangleA,s,t);
		subData.setType(PfxSubData::MESH_INFO);
		subData.setFacetLocalS(s);
		subData.setFacetLocalT(t);
		subData.setUserData(facet.m_userData);

		PfxUInt16 islandId = (PfxUInt16)(meshA - (PfxQuantizedTriMeshBvh*)largeMeshA->m_islands);
		PfxUInt64 featureIdA = ((PfxUInt64)islandId << 32) | localContacts.getFeatureIdA(i);

		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			rotationB_n * localContacts.getContactNormal(i),
			transformB * localContacts.getContactLocalPointA(i),
			localContacts.getContactLocalPointB(i),
			featureIdA, localContacts.getFeatureIdB(i),
			subData,PfxSubData());
	}

	return contacts.getNumContactPoints();
}

template<>
PfxInt32 pfxContactTriMesh<PfxBox, PfxCompressedTriMesh> (
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA,const PfxCompressedTriMesh *meshA,bool flipTriangle,
	const PfxBox &boxB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold)
{
	(void) distanceThreshold;

	PfxTransform3 transformBA = inverse(transformB);

	PfxVector3 center = transformB.getTranslation();
	PfxVector3 half = absPerElem(transformB.getUpper3x3()) * boxB.m_half;
	PfxUInt8 selFacets[SCE_PFX_NUMMESHFACETS] = {0};
	PfxUInt32 numSelFacets = pfxGatherFacets(meshA,(PfxFacetBvhNode*)largeMeshA->m_bvhNodeBuffer,center - half,center + half,selFacets);
	if(numSelFacets == 0) return 0;

	PfxDecodedTriMesh decodedMesh;
	PfxContactCache localContacts;

	int vi[4] = {0,1,2,3};
	if(flipTriangle) {vi[0]=2;vi[2]=0;}

	for(PfxUInt32 f = 0; f < numSelFacets; f++) {
		//-------------------------------------------
		// 交差した面との分離軸判定(SAT)
		// ※Box座標系 (Bローカル)で判定

		const PfxCompressedFacet2 &facet = ((PfxCompressedFacet2*)largeMeshA->m_facetBuffer)[meshA->m_facets + selFacets[f]];

		const PfxUInt32 vId[4] = {facet.m_vertIds[vi[0]],facet.m_vertIds[vi[1]],facet.m_vertIds[vi[2]],facet.m_vertIds[vi[3]]};

		for(int v=0;v<4;v++) {
			if(!decodedMesh.isDecodedVertex(vId[v])) {
				PfxVector3 vert = largeMeshA->decodePosition(*((PfxQuantize3*)largeMeshA->m_vertexBuffer + meshA->m_verts + vId[v]));
				decodedMesh.m_verts[vId[v]] = transformBA.getTranslation() + transformBA.getUpper3x3() * vert;
				decodedMesh.m_decodedVertex[vId[v]>>5] |= 1 << (vId[v]&0x1f);
			}
		}

		PfxVector3 facetPntsA[4] = {
			decodedMesh.m_verts[vId[0]],
			decodedMesh.m_verts[vId[1]],
			decodedMesh.m_verts[vId[2]],
			decodedMesh.m_verts[vId[3]],
		};

		if(facet.m_facetInfo & 0x8000) {
			// double facets
			PfxVector3 facetNormal0 = normalize(cross(facetPntsA[1]-facetPntsA[0],facetPntsA[2]-facetPntsA[0]));
			PfxVector3 facetNormal1 = normalize(cross(facetPntsA[3]-facetPntsA[2],facetPntsA[0]-facetPntsA[2]));

			PfxUInt32 edgeChk0 = facet.m_edgeInfo & 0x3f;
			PfxUInt32 edgeChk1 = facet.m_edgeInfo>>4;

			if(flipTriangle) {
				edgeChk0 = ((edgeChk0>>2)&0x0c) | ((edgeChk0<<2)&0x30) | (edgeChk0&0x03);
				edgeChk1 = ((edgeChk1>>2)&0x0c) | ((edgeChk1<<2)&0x30) | (edgeChk1&0x03);
			}

			PfxVector3 edgeAngle(0.0f);

			PfxUInt8 facetId0 = selFacets[f];
			PfxUInt8 facetId1 = selFacets[f] | 0x80;

			pfxContactTriangleBox(localContacts,facetId0,
								facetNormal0,facetPntsA[0],facetPntsA[1],facetPntsA[2],
								largeMeshA->m_defaultThickness,edgeAngle,edgeChk0,boxB);

			pfxContactTriangleBox(localContacts,facetId1,
								facetNormal1,facetPntsA[0],facetPntsA[2],facetPntsA[3],
								largeMeshA->m_defaultThickness,edgeAngle,edgeChk1,boxB);
		}
		else {
			// signle facet
			PfxVector3 facetNormal = normalize(cross(facetPntsA[1]-facetPntsA[0],facetPntsA[2]-facetPntsA[0]));

			PfxUInt32 edgeChk = facet.m_edgeInfo & 0x3f;

			if(flipTriangle) {
				edgeChk = ((edgeChk>>2)&0x0c) | ((edgeChk<<2)&0x30) | (edgeChk&0x03);
			}

			PfxVector3 edgeAngle(0.0f);

			pfxContactTriangleBox(localContacts,selFacets[f],
								facetNormal,facetPntsA[0],facetPntsA[1],facetPntsA[2],
								largeMeshA->m_defaultThickness,edgeAngle,edgeChk,boxB);
		}
	}

	PfxMatrix3 rotationB_n = transpose(inverse(transformB.getUpper3x3()));

	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subData = localContacts.getContactSubDataA(i);

		PfxUInt8 facetId = subData.getFacetId();

		const PfxCompressedFacet2 &facet = ((PfxCompressedFacet2*)largeMeshA->m_facetBuffer)[meshA->m_facets + (facetId&0x7f)];

		PfxVector3 p[3];
		PfxUInt32 userData;

		if(facetId&0x80) {
			// facet 1
			p[0] = decodedMesh.m_verts[facet.m_vertIds[vi[0]]];
			p[1] = decodedMesh.m_verts[facet.m_vertIds[vi[2]]];
			p[2] = decodedMesh.m_verts[facet.m_vertIds[vi[3]]];
			userData = facet.m_userData[1];
			facetId = (facet.m_facetInfo&0xFF)+1; // Calculate the actual facet index
		}
		else {
			// facet 0
			p[0] = decodedMesh.m_verts[facet.m_vertIds[vi[0]]];
			p[1] = decodedMesh.m_verts[facet.m_vertIds[vi[1]]];
			p[2] = decodedMesh.m_verts[facet.m_vertIds[vi[2]]];
			userData = facet.m_userData[0];
			facetId = (facet.m_facetInfo&0xFF); // Calculate the actual facet index
		}

		PfxTriangle triangleA(p[0],p[1],p[2]);

		PfxFloat s=0.0f,t=0.0f;
		pfxCalcBarycentricCoords(PfxVector3(localContacts.getContactLocalPointA(i)),triangleA,s,t);
		subData.setType(PfxSubData::MESH_INFO);
		subData.setFacetLocalS(s);
		subData.setFacetLocalT(t);
		subData.setUserData(userData);
		subData.setFacetId(facetId);

		PfxUInt16 islandId = (PfxUInt16)(meshA - (PfxCompressedTriMesh*)largeMeshA->m_islands);
		PfxUInt64 featureIdA = ((PfxUInt64)islandId << 32) | localContacts.getFeatureIdA(i);

		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			rotationB_n * localContacts.getContactNormal(i),
			transformB * localContacts.getContactLocalPointA(i),
			localContacts.getContactLocalPointB(i),
			featureIdA, localContacts.getFeatureIdB(i),
			subData,PfxSubData());
	}

	return contacts.getNumContactPoints();
}

} //namespace pfxv4
} //namespace sce
