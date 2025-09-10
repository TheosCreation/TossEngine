/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
*                Copyright (C) 2021 Sony Interactive Entertainment Inc.
*                                                
*/

#ifndef _SCE_PFX_INTERSECT_ALL_CAST_LARGE_TRI_MESH_OBJ_TEMPLATE_H
#define _SCE_PFX_INTERSECT_ALL_CAST_LARGE_TRI_MESH_OBJ_TEMPLATE_H

//#define COLLECT_BVH_NUMBERS

#ifndef _INTERSECT_ALL_CAST_STRUCT_DEFINED
#define _INTERSECT_ALL_CAST_STRUCT_DEFINED

#ifdef COLLECT_BVH_NUMBERS
extern int aveNumBvhs;
extern int aveNumIlnd;
extern int aveNumTris;

#define INCREASE_NUM_BVHS		++aveNumBvhs;
#define INCREASE_NUM_ISLANDS	++aveNumIlnd;
#define INCREASE_NUM_TRIS		++aveNumTris;
#else
#define INCREASE_NUM_BVHS
#define INCREASE_NUM_ISLANDS
#define INCREASE_NUM_TRIS
#endif

struct PfxClosestFacetInfo {
	PfxBool hit;
	PfxUInt32 islandId;
	PfxUInt32 facetId;
	PfxFloat angle;
	PfxFloat tmin;
	PfxFloat squaredDistance;
	PfxVector3 contactPoint;
};

struct EncStack {
	PfxUInt32 nodeId;
	PfxVector3 aabbMin;
	PfxVector3 aabbMax;
};

struct EncStack2 {
	PfxUInt32 nodeId;
	PfxVector3 aabbMin;
	PfxVector3 aabbMax;
	PfxFloat nodeT;
};

struct PfxRayBvInfo {
	PfxVector3 startPos;
	PfxVector3 endPos;
	PfxVector3 direction;
	PfxVector3 oriDirection;
	PfxVector3 radiusVec;
	PfxBvVector3 aabb;
};

template<typename CastInputClass>
using pfxIntersectAllCastTriangleFuncPtr = PfxBool(*)(const PfxTriangle &, const CastInputClass &, PfxClosestFacetInfo &);

static const PfxFloat quantizationInverse = 1.0f / 255.0f;

SCE_PFX_FORCE_INLINE PfxUInt32 pfxGetFacetPointsAndUserData(const PfxLargeTriMeshImpl &largeMesh, PfxUInt32 islandId, PfxUInt32 facetId, PfxVector3 points[3])
{
	PfxUInt32 userData = 0;

	switch (largeMesh.m_type) {
	case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
	{
		const PfxCompressedTriMesh *island = (PfxCompressedTriMesh*)largeMesh.m_islands + islandId;
		PfxUInt32 vertIds[3] = { 0,0,0 };

		PfxUInt32 facet2Id = facetId & 0xFFFF;
		PfxUInt32 facet2LR = facetId >> 16;

		const PfxCompressedFacet2 *facet = (PfxCompressedFacet2*)largeMesh.m_facetBuffer + island->m_facets + facet2Id;
		if (facet2LR == 0) {
			vertIds[0] = facet->m_vertIds[0];
			vertIds[1] = facet->m_vertIds[1];
			vertIds[2] = facet->m_vertIds[2];
			userData = facet->m_userData[0];
		}
		else {
			vertIds[0] = facet->m_vertIds[2];
			vertIds[1] = facet->m_vertIds[3];
			vertIds[2] = facet->m_vertIds[0];
			userData = facet->m_userData[1];
		}

		PfxQuantize3 *qvert1 = (PfxQuantize3*)largeMesh.m_vertexBuffer + island->m_verts + vertIds[0];
		PfxQuantize3 *qvert2 = (PfxQuantize3*)largeMesh.m_vertexBuffer + island->m_verts + vertIds[1];
		PfxQuantize3 *qvert3 = (PfxQuantize3*)largeMesh.m_vertexBuffer + island->m_verts + vertIds[2];
		points[0] = largeMesh.decodePosition(*qvert1);
		points[1] = largeMesh.decodePosition(*qvert2);
		points[2] = largeMesh.decodePosition(*qvert3);
	}
	break;

	case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
	{
		const PfxQuantizedTriMeshBvh *island = (PfxQuantizedTriMeshBvh*)largeMesh.m_islands + islandId;
		const PfxQuantizedFacetBvh &facet = island->m_facets[facetId];
		points[0] = largeMesh.decodePosition(island->m_verts[facet.m_vertIds[0]]);
		points[1] = largeMesh.decodePosition(island->m_verts[facet.m_vertIds[1]]);
		points[2] = largeMesh.decodePosition(island->m_verts[facet.m_vertIds[2]]);
		userData = facet.m_userData;
	}
	break;

	case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
	{
		const PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMesh.m_islands) + islandId;
		const PfxExpandedFacet &facet = island->m_facets[facetId];
		points[0] = island->m_verts[facet.m_vertIds[0]];
		points[1] = island->m_verts[facet.m_vertIds[1]];
		points[2] = island->m_verts[facet.m_vertIds[2]];
		userData = facet.m_userData;
	}
	break;
	}

	return userData;
}

#endif

#ifdef CAST_ENABLE_DISCARD_TRIANGLE
#ifndef _CAST_DISCARD_TRIANGLE_CALLBACK_DEFINED
#define _CAST_DISCARD_TRIANGLE_CALLBACK_DEFINED
template<typename CastDiscardTriangleData>
using pfxIntersectAllCastDiscardTriangleCallbackFuncPtr = PfxBool(*)(const CastDiscardTriangleData &, const PfxTransform3&, void*);
#endif
#endif

#ifdef CAST_ENABLE_RIGIDBODY_CACHE
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
template<typename CastInputClass, typename CastOutputClass, typename CastDiscardTriangleData>
class PfxIntersectAllCastCachedIslandBvhPartialObj
#else
template<typename CastInputClass, typename CastOutputClass>
class PfxIntersectAllCastCachedIslandBvhObj
#endif
#else
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
template<typename CastInputClass, typename CastOutputClass, typename CastDiscardTriangleData>
class PfxIntersectAllCastTriMeshBvhPartialObj
#else
template<typename CastInputClass, typename CastOutputClass>
class PfxIntersectAllCastTriMeshBvhObj
#endif
#endif
{
	PfxRayBvInfo rayLocalInfo;
	PfxRayBvInfo rayGlobalInfo;
	PfxClosestFacetInfo closestFacetInfo;
	pfxIntersectAllCastTriangleFuncPtr<CastInputClass> rayTriFuncPtr;

	const PfxLargeTriMeshImpl *largeMesh;
	const PfxTransform3 transform;
	PfxUInt32 ids[4];

	PfxStaticStack<EncStack2> islandStack;
	PfxStaticStack<EncStack2,64> bvhStack;

#ifdef CAST_ENABLE_RIGIDBODY_CACHE
	const PfxUInt32 *cachedIslandIds;
	const PfxUInt32 numCachedIslands;
#endif

#ifdef CAST_ENABLE_DISCARD_TRIANGLE
	pfxIntersectAllCastDiscardTriangleCallbackFuncPtr<CastDiscardTriangleData> discardTriangleCallback;
	void *userDataForDiscardingTriangle;
#endif

void intersectAllCastQuantizedTriMeshBvh(
	const CastInputClass &in,
	const PfxQuantizedTriMeshBvh *mesh, PfxUInt32 islandId, const PfxVector3 &islandAabbMin, const PfxVector3 &islandAabbMax)
{
	{
		PfxFacetBvhNode &encnode = mesh->m_bvhNodes[0];
		
		PfxVector3 quantizedMin(encnode.aabb[0], encnode.aabb[1], encnode.aabb[2]);
		PfxVector3 quantizedMax(encnode.aabb[3], encnode.aabb[4], encnode.aabb[5]);

		EncStack2 encroot;
		encroot.nodeId = 0;
		encroot.aabbMin = islandAabbMin + mulPerElem((islandAabbMax - islandAabbMin), quantizedMin * quantizationInverse);
		encroot.aabbMax = islandAabbMax - mulPerElem((islandAabbMax - islandAabbMin), quantizedMax * quantizationInverse);
		encroot.nodeT = 0.f;

		islandStack.push(encroot);
	}

	PfxDecodedTriMesh decodedMesh;

	PfxClosestFacetInfo tempClosestFacetInfo;
	tempClosestFacetInfo.tmin = closestFacetInfo.tmin;

	INCREASE_NUM_ISLANDS

	while (!islandStack.empty()) {
		EncStack2 info = islandStack.top();
		islandStack.pop();

		PfxFacetBvhNode &encnode = mesh->m_bvhNodes[info.nodeId];

		PfxUInt32 numNextNodes = 0;
		EncStack2 nextNodes[2];

		PfxUInt32 numFacets = 0;
		PfxUInt32 facetsIds[2];

		PfxUInt8 LStatus = encnode.flag & 0x03;
		PfxUInt8 RStatus = (encnode.flag & 0x0C) >> 2;

		if (LStatus == 0) {
			PfxFacetBvhNode &facetNode = mesh->m_bvhNodes[encnode.left];
			PfxVector3 quantizedMin(facetNode.aabb[0], facetNode.aabb[1], facetNode.aabb[2]);
			PfxVector3 quantizedMax(facetNode.aabb[3], facetNode.aabb[4], facetNode.aabb[5]);

			EncStack2 &encnext(nextNodes[numNextNodes]);
			encnext.nodeId = encnode.left;
			encnext.aabbMin = info.aabbMin + mulPerElem((info.aabbMax - info.aabbMin), quantizedMin * quantizationInverse);
			encnext.aabbMax = info.aabbMax - mulPerElem((info.aabbMax - info.aabbMin), quantizedMax * quantizationInverse);
#ifndef CAST_NO_RADIUS
			encnext.nodeT = dot(((encnext.aabbMax + encnext.aabbMin) * 0.5f - rayLocalInfo.startPos), rayLocalInfo.oriDirection);
#endif

			if (pfxTestRayBvByDotProductVector3(rayLocalInfo.startPos, rayLocalInfo.direction, rayLocalInfo.aabb, encnext.aabbMin - rayLocalInfo.radiusVec, encnext.aabbMax + rayLocalInfo.radiusVec))
#ifdef CAST_NO_RADIUS
				islandStack.push(encnext);
#else
				++numNextNodes;
#endif
		}
		else if (LStatus == 1) {
			facetsIds[numFacets++] = encnode.left;
		}

		if (RStatus == 0) {
			PfxFacetBvhNode &facetNode = mesh->m_bvhNodes[encnode.right];
			PfxVector3 quantizedMin(facetNode.aabb[0], facetNode.aabb[1], facetNode.aabb[2]);
			PfxVector3 quantizedMax(facetNode.aabb[3], facetNode.aabb[4], facetNode.aabb[5]);

			EncStack2 &encnext(nextNodes[numNextNodes]);
			encnext.nodeId = encnode.right;
			encnext.aabbMin = info.aabbMin + mulPerElem((info.aabbMax - info.aabbMin), quantizedMin * quantizationInverse);
			encnext.aabbMax = info.aabbMax - mulPerElem((info.aabbMax - info.aabbMin), quantizedMax * quantizationInverse);
#ifndef CAST_NO_RADIUS
			encnext.nodeT = dot(((encnext.aabbMax + encnext.aabbMin) * 0.5f - rayLocalInfo.startPos), rayLocalInfo.oriDirection);
#endif

			if (pfxTestRayBvByDotProductVector3(rayLocalInfo.startPos, rayLocalInfo.direction, rayLocalInfo.aabb, encnext.aabbMin - rayLocalInfo.radiusVec, encnext.aabbMax + rayLocalInfo.radiusVec))
#ifdef CAST_NO_RADIUS
				islandStack.push(encnext);
#else
				++numNextNodes;
#endif
		}
		else if (RStatus == 1) {
			facetsIds[numFacets++] = encnode.right;
		}

#ifndef CAST_NO_RADIUS
		if (numNextNodes == 2) {
			if (nextNodes[0].nodeT > nextNodes[1].nodeT) {
				islandStack.push(nextNodes[0]);
				islandStack.push(nextNodes[1]);
			}
			else {
				islandStack.push(nextNodes[1]);
				islandStack.push(nextNodes[0]);
			}
			continue;
		}
		else if (numNextNodes == 1) {
			islandStack.push(nextNodes[0]);
		}
#endif

		for (PfxUInt32 f = 0; f < numFacets; f++) {
			const PfxQuantizedFacetBvh &facet = mesh->m_facets[facetsIds[f]];

			// デコード
			PfxDecodedFacet decodedFacet;
			const PfxUInt32 vId[3] = { facet.m_vertIds[0],facet.m_vertIds[1],facet.m_vertIds[2] };

			for (int v = 0; v < 3; v++) {
				if (!decodedMesh.isDecodedVertex(vId[v])) {
					decodedMesh.m_verts[vId[v]] = transform.getTranslation() + transform.getUpper3x3() * largeMesh->decodePosition(mesh->m_verts[vId[v]]);
					decodedMesh.m_decodedVertex[vId[v] >> 5] |= 1 << (vId[v] & 0x1f);
				}
			}

			PfxTriangle triangle(
				decodedMesh.m_verts[vId[ids[0]]],
				decodedMesh.m_verts[vId[ids[1]]],
				decodedMesh.m_verts[vId[ids[2]]]);

			PfxVector3 triMax = maxPerElem(maxPerElem(triangle.points[0], triangle.points[1]), triangle.points[2]);
			PfxVector3 triMin = minPerElem(minPerElem(triangle.points[0], triangle.points[1]), triangle.points[2]);
			if (!pfxTestRayBvByDotProductVector3(rayGlobalInfo.startPos, rayGlobalInfo.direction, rayGlobalInfo.aabb, triMin - rayGlobalInfo.radiusVec, triMax + rayGlobalInfo.radiusVec))	continue;

			PfxVector3 normal = triangle.calcNormal();

			if (dot(normal, in.m_startPosition - triangle.points[0]) <= 0.0f) {
				continue;
			}
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
			{
				CastDiscardTriangleData triangleData;
				triangleData.m_vertices[0] = triangle.points[0];
				triangleData.m_vertices[1] = triangle.points[1];
				triangleData.m_vertices[2] = triangle.points[2];
				triangleData.m_normal = normal;
				triangleData.m_userData = facet.m_userData;
				if (!(*discardTriangleCallback)(triangleData, transform, userDataForDiscardingTriangle))
					continue;
			}
#endif
			INCREASE_NUM_TRIS

			tempClosestFacetInfo.squaredDistance = pfxCalcInitialSquaredDistance(in);
			if ((*rayTriFuncPtr)(triangle, in, tempClosestFacetInfo)) {
				if (tempClosestFacetInfo.squaredDistance > closestFacetInfo.squaredDistance)
					continue;

#ifdef CAST_NO_RADIUS
				if ((tempClosestFacetInfo.squaredDistance < closestFacetInfo.squaredDistance && tempClosestFacetInfo.tmin == 0.0f) ||
					(tempClosestFacetInfo.tmin < closestFacetInfo.tmin - SCE_PFX_INTERSECT_COMMON_EPSILON)) {
#else
				PfxFloat angle = dot(normal, in.m_direction);
				if ((tempClosestFacetInfo.squaredDistance < closestFacetInfo.squaredDistance && tempClosestFacetInfo.tmin == 0.0f) ||
					(tempClosestFacetInfo.tmin < closestFacetInfo.tmin - SCE_PFX_INTERSECT_COMMON_EPSILON) ||
					(tempClosestFacetInfo.tmin < closestFacetInfo.tmin + SCE_PFX_INTERSECT_COMMON_EPSILON && angle <= closestFacetInfo.angle)) {
					closestFacetInfo.angle = angle;
#endif
					closestFacetInfo.hit = true;
					closestFacetInfo.facetId = facetsIds[f];
					closestFacetInfo.tmin = tempClosestFacetInfo.tmin;
					closestFacetInfo.contactPoint = tempClosestFacetInfo.contactPoint;
					closestFacetInfo.squaredDistance = tempClosestFacetInfo.squaredDistance;
					closestFacetInfo.islandId = islandId;

					rayLocalInfo.direction = rayLocalInfo.oriDirection * tempClosestFacetInfo.tmin;
					rayLocalInfo.endPos = rayLocalInfo.startPos + rayLocalInfo.direction;
					rayLocalInfo.aabb.vmin = minPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);
					rayLocalInfo.aabb.vmax = maxPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);

					rayGlobalInfo.direction = rayGlobalInfo.oriDirection * tempClosestFacetInfo.tmin;
					rayGlobalInfo.endPos = rayGlobalInfo.startPos + rayGlobalInfo.direction;
					rayGlobalInfo.aabb.vmin = minPerElem(rayGlobalInfo.startPos, rayGlobalInfo.endPos);
					rayGlobalInfo.aabb.vmax = maxPerElem(rayGlobalInfo.startPos, rayGlobalInfo.endPos);
				}
			}
		}
	}
}

void intersectAllCastCompressedTriMesh(
	const CastInputClass &in,
	const PfxCompressedTriMesh *mesh, PfxUInt32 islandId, const PfxVector3 &islandAabbMin, const PfxVector3 &islandAabbMax)
{
	{
		PfxFacetBvhNode &facetNode = ((PfxFacetBvhNode*)largeMesh->m_bvhNodeBuffer)[mesh->m_bvhNodes];
		PfxVector3 quantizedMin(facetNode.aabb[0], facetNode.aabb[1], facetNode.aabb[2]);
		PfxVector3 quantizedMax(facetNode.aabb[3], facetNode.aabb[4], facetNode.aabb[5]);

		EncStack2 encroot;
		encroot.nodeId = 0;
		encroot.aabbMin = islandAabbMin + mulPerElem((islandAabbMax - islandAabbMin), quantizedMin * quantizationInverse);
		encroot.aabbMax = islandAabbMax - mulPerElem((islandAabbMax - islandAabbMin), quantizedMax * quantizationInverse);
		encroot.nodeT = 0.f;

		islandStack.push(encroot);
	}

	PfxDecodedTriMesh decodedMesh;

	PfxClosestFacetInfo tempClosestFacetInfo;
	tempClosestFacetInfo.tmin = closestFacetInfo.tmin;
	tempClosestFacetInfo.hit = false;

	INCREASE_NUM_ISLANDS

	while(!islandStack.empty()) {
		EncStack2 info = islandStack.top();
		islandStack.pop();

		PfxFacetBvhNode &encnode = ((PfxFacetBvhNode*)largeMesh->m_bvhNodeBuffer)[mesh->m_bvhNodes + info.nodeId];

		PfxUInt32 numNextNodes = 0;
		EncStack2 nextNodes[2];

		PfxUInt32 numFacets = 0;
		PfxUInt32 facetsIds[2];

		PfxUInt8 LStatus = encnode.flag & 0x03;
		PfxUInt8 RStatus = (encnode.flag & 0x0C) >> 2;

		if (LStatus == 0) {
			PfxFacetBvhNode &facetNode = ((PfxFacetBvhNode*)largeMesh->m_bvhNodeBuffer)[mesh->m_bvhNodes + encnode.left];
			PfxVector3 quantizedMin(facetNode.aabb[0], facetNode.aabb[1], facetNode.aabb[2]);
			PfxVector3 quantizedMax(facetNode.aabb[3], facetNode.aabb[4], facetNode.aabb[5]);

			EncStack2 &encnext(nextNodes[numNextNodes]);
			encnext.nodeId = encnode.left;
			encnext.aabbMin = info.aabbMin + mulPerElem((info.aabbMax - info.aabbMin), quantizedMin * quantizationInverse);
			encnext.aabbMax = info.aabbMax - mulPerElem((info.aabbMax - info.aabbMin), quantizedMax * quantizationInverse);
#ifndef CAST_NO_RADIUS
			encnext.nodeT = dot(((encnext.aabbMax + encnext.aabbMin) * 0.5f - rayLocalInfo.startPos), rayLocalInfo.oriDirection);
#endif

			if (pfxTestRayBvByDotProductVector3(rayLocalInfo.startPos, rayLocalInfo.direction, rayLocalInfo.aabb, encnext.aabbMin - rayLocalInfo.radiusVec, encnext.aabbMax + rayLocalInfo.radiusVec))
#ifdef CAST_NO_RADIUS
				islandStack.push(encnext);
#else
				++numNextNodes;
#endif
		}
		else if (LStatus == 1) {
			facetsIds[numFacets++] = encnode.left;
		}

		if (RStatus == 0) {
			PfxFacetBvhNode &facetNode = ((PfxFacetBvhNode*)largeMesh->m_bvhNodeBuffer)[mesh->m_bvhNodes + encnode.right];
			PfxVector3 quantizedMin(facetNode.aabb[0], facetNode.aabb[1], facetNode.aabb[2]);
			PfxVector3 quantizedMax(facetNode.aabb[3], facetNode.aabb[4], facetNode.aabb[5]);

			EncStack2 &encnext(nextNodes[numNextNodes]);
			encnext.nodeId = encnode.right;
			encnext.aabbMin = info.aabbMin + mulPerElem((info.aabbMax - info.aabbMin), quantizedMin * quantizationInverse);
			encnext.aabbMax = info.aabbMax - mulPerElem((info.aabbMax - info.aabbMin), quantizedMax * quantizationInverse);
#ifndef CAST_NO_RADIUS
			encnext.nodeT = dot(((encnext.aabbMax + encnext.aabbMin) * 0.5f - rayLocalInfo.startPos), rayLocalInfo.oriDirection);
#endif

			if (pfxTestRayBvByDotProductVector3(rayLocalInfo.startPos, rayLocalInfo.direction, rayLocalInfo.aabb, encnext.aabbMin - rayLocalInfo.radiusVec, encnext.aabbMax + rayLocalInfo.radiusVec))
#ifdef CAST_NO_RADIUS
				islandStack.push(encnext);
#else
				++numNextNodes;
#endif
		}
		else if (RStatus == 1) {
			facetsIds[numFacets++] = encnode.right;
		}

#ifndef CAST_NO_RADIUS
		if (numNextNodes == 2) {
			if (nextNodes[0].nodeT > nextNodes[1].nodeT) {
				islandStack.push(nextNodes[0]);
				islandStack.push(nextNodes[1]);
			}
			else {
				islandStack.push(nextNodes[1]);
				islandStack.push(nextNodes[0]);
			}
			continue;
		}
		else if (numNextNodes == 1) {
			islandStack.push(nextNodes[0]);
		}
#endif

		for (PfxUInt32 f = 0; f < numFacets; f++) {
			const PfxCompressedFacet2 &facet = ((PfxCompressedFacet2*)largeMesh->m_facetBuffer)[mesh->m_facets + facetsIds[f]];

			// デコード
			PfxDecodedFacet decodedFacet;
			const PfxUInt32 vId[4] = { facet.m_vertIds[0],facet.m_vertIds[1],facet.m_vertIds[2],facet.m_vertIds[3] };

			for (int v = 0; v < 4; v++) {
				if (!decodedMesh.isDecodedVertex(vId[v])) {
					decodedMesh.m_verts[vId[v]] = transform.getTranslation() + transform.getUpper3x3() * largeMesh->decodePosition(*((PfxQuantize3*)largeMesh->m_vertexBuffer + mesh->m_verts + vId[v]));
					decodedMesh.m_decodedVertex[vId[v] >> 5] |= 1 << (vId[v] & 0x1f);
				}
			}

			if (facet.m_facetInfo & 0x8000) {
				PfxTriangle triangle1(
					decodedMesh.m_verts[vId[ids[0]]],
					decodedMesh.m_verts[vId[ids[1]]],
					decodedMesh.m_verts[vId[ids[2]]]);

				PfxVector3 tri1Max = maxPerElem(maxPerElem(triangle1.points[0], triangle1.points[1]), triangle1.points[2]);
				PfxVector3 tri1Min = minPerElem(minPerElem(triangle1.points[0], triangle1.points[1]), triangle1.points[2]);
				if (pfxTestRayBvByDotProductVector3(rayGlobalInfo.startPos, rayGlobalInfo.direction, rayGlobalInfo.aabb, tri1Min - rayGlobalInfo.radiusVec, tri1Max + rayGlobalInfo.radiusVec))
				{
					PfxVector3 normal1 = triangle1.calcNormal();

					if (dot(normal1, in.m_startPosition - triangle1.points[0]) > 0.0f) {
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
						CastDiscardTriangleData triangleData;
						triangleData.m_vertices[0] = triangle1.points[0];
						triangleData.m_vertices[1] = triangle1.points[1];
						triangleData.m_vertices[2] = triangle1.points[2];
						triangleData.m_normal = normal1;
						triangleData.m_userData = facet.m_userData[0];
						if ((*discardTriangleCallback)(triangleData, transform, userDataForDiscardingTriangle)) {
#endif
							INCREASE_NUM_TRIS

							tempClosestFacetInfo.squaredDistance = pfxCalcInitialSquaredDistance(in);
							if ((*rayTriFuncPtr)(triangle1, in, tempClosestFacetInfo)) {
								if (tempClosestFacetInfo.squaredDistance > closestFacetInfo.squaredDistance)
									continue;
#ifdef CAST_NO_RADIUS
								if ((tempClosestFacetInfo.squaredDistance < closestFacetInfo.squaredDistance && tempClosestFacetInfo.tmin == 0.0f) ||
									(tempClosestFacetInfo.tmin < closestFacetInfo.tmin - SCE_PFX_INTERSECT_COMMON_EPSILON)) {
#else
								PfxFloat angle1 = dot(normal1, in.m_direction);
								if ((tempClosestFacetInfo.squaredDistance < closestFacetInfo.squaredDistance && tempClosestFacetInfo.tmin == 0.0f) ||
									(tempClosestFacetInfo.tmin < closestFacetInfo.tmin - SCE_PFX_INTERSECT_COMMON_EPSILON) ||
									(tempClosestFacetInfo.tmin < closestFacetInfo.tmin + SCE_PFX_INTERSECT_COMMON_EPSILON && angle1 <= closestFacetInfo.angle)) {
									closestFacetInfo.angle = angle1;
#endif
									closestFacetInfo.hit = true;
									closestFacetInfo.facetId = facetsIds[f];
									closestFacetInfo.tmin = tempClosestFacetInfo.tmin;
									closestFacetInfo.contactPoint = tempClosestFacetInfo.contactPoint;
									closestFacetInfo.squaredDistance = tempClosestFacetInfo.squaredDistance;
									closestFacetInfo.islandId = islandId;

									rayLocalInfo.direction = rayLocalInfo.oriDirection * tempClosestFacetInfo.tmin;
									rayLocalInfo.endPos = rayLocalInfo.startPos + rayLocalInfo.direction;
									rayLocalInfo.aabb.vmin = minPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);
									rayLocalInfo.aabb.vmax = maxPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);

									rayGlobalInfo.direction = rayGlobalInfo.oriDirection * tempClosestFacetInfo.tmin;
									rayGlobalInfo.endPos = rayGlobalInfo.startPos + rayGlobalInfo.direction;
									rayGlobalInfo.aabb.vmin = minPerElem(rayGlobalInfo.startPos, rayGlobalInfo.endPos);
									rayGlobalInfo.aabb.vmax = maxPerElem(rayGlobalInfo.startPos, rayGlobalInfo.endPos);
								}
							}
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
						}
#endif
					}
				}

				PfxTriangle triangle2(
					decodedMesh.m_verts[vId[ids[2]]],
					decodedMesh.m_verts[vId[ids[3]]],
					decodedMesh.m_verts[vId[ids[0]]]);

				PfxVector3 tri2Max = maxPerElem(maxPerElem(triangle2.points[0], triangle2.points[1]), triangle2.points[2]);
				PfxVector3 tri2Min = minPerElem(minPerElem(triangle2.points[0], triangle2.points[1]), triangle2.points[2]);
				if (pfxTestRayBvByDotProductVector3(rayGlobalInfo.startPos, rayGlobalInfo.direction, rayGlobalInfo.aabb, tri2Min - rayGlobalInfo.radiusVec, tri2Max + rayGlobalInfo.radiusVec))
				{
					PfxVector3 normal2 = triangle2.calcNormal();

					if (dot(normal2, in.m_startPosition - triangle2.points[0]) > 0.0f) {
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
						CastDiscardTriangleData triangleData;
						triangleData.m_vertices[0] = triangle2.points[0];
						triangleData.m_vertices[1] = triangle2.points[1];
						triangleData.m_vertices[2] = triangle2.points[2];
						triangleData.m_normal = normal2;
						triangleData.m_userData = facet.m_userData[1];
						if ((*discardTriangleCallback)(triangleData, transform, userDataForDiscardingTriangle)) {
#endif
							INCREASE_NUM_TRIS

							tempClosestFacetInfo.squaredDistance = pfxCalcInitialSquaredDistance(in);
							if ((*rayTriFuncPtr)(triangle2, in, tempClosestFacetInfo)) {
								if (tempClosestFacetInfo.squaredDistance > closestFacetInfo.squaredDistance)
									continue;
#ifdef CAST_NO_RADIUS
								if ((tempClosestFacetInfo.squaredDistance < closestFacetInfo.squaredDistance && tempClosestFacetInfo.tmin == 0.0f) ||
									(tempClosestFacetInfo.tmin < closestFacetInfo.tmin - SCE_PFX_INTERSECT_COMMON_EPSILON)) {
#else
								PfxFloat angle2 = dot(normal2, in.m_direction);
								if ((tempClosestFacetInfo.squaredDistance < closestFacetInfo.squaredDistance && tempClosestFacetInfo.tmin == 0.0f) ||
									(tempClosestFacetInfo.tmin < closestFacetInfo.tmin - SCE_PFX_INTERSECT_COMMON_EPSILON) ||
									(tempClosestFacetInfo.tmin < closestFacetInfo.tmin + SCE_PFX_INTERSECT_COMMON_EPSILON && angle2 <= closestFacetInfo.angle)) {
									closestFacetInfo.angle = angle2;
#endif
									closestFacetInfo.hit = true;
									closestFacetInfo.facetId = (1 << 16) | facetsIds[f];
									closestFacetInfo.tmin = tempClosestFacetInfo.tmin;
									closestFacetInfo.contactPoint = tempClosestFacetInfo.contactPoint;
									closestFacetInfo.squaredDistance = tempClosestFacetInfo.squaredDistance;
									closestFacetInfo.islandId = islandId;

									rayLocalInfo.direction = rayLocalInfo.oriDirection * tempClosestFacetInfo.tmin;
									rayLocalInfo.endPos = rayLocalInfo.startPos + rayLocalInfo.direction;
									rayLocalInfo.aabb.vmin = minPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);
									rayLocalInfo.aabb.vmax = maxPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);

									rayGlobalInfo.direction = rayGlobalInfo.oriDirection * tempClosestFacetInfo.tmin;
									rayGlobalInfo.endPos = rayGlobalInfo.startPos + rayGlobalInfo.direction;
									rayGlobalInfo.aabb.vmin = minPerElem(rayGlobalInfo.startPos, rayGlobalInfo.endPos);
									rayGlobalInfo.aabb.vmax = maxPerElem(rayGlobalInfo.startPos, rayGlobalInfo.endPos);
								}
							}
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
						}
#endif
					}
				}
			}
			else {
				PfxTriangle triangle(
					decodedMesh.m_verts[vId[ids[0]]],
					decodedMesh.m_verts[vId[ids[1]]],
					decodedMesh.m_verts[vId[ids[2]]]);
				PfxVector3 triMax = maxPerElem(maxPerElem(triangle.points[0], triangle.points[1]), triangle.points[2]);
				PfxVector3 triMin = minPerElem(minPerElem(triangle.points[0], triangle.points[1]), triangle.points[2]);
				if (!pfxTestRayBvByDotProductVector3(rayGlobalInfo.startPos, rayGlobalInfo.direction, rayGlobalInfo.aabb, triMin - rayGlobalInfo.radiusVec, triMax + rayGlobalInfo.radiusVec))
					continue;

				PfxVector3 normal = triangle.calcNormal();

				if (dot(normal, in.m_startPosition - triangle.points[0]) > 0.0f) {
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
					CastDiscardTriangleData triangleData;
					triangleData.m_vertices[0] = triangle.points[0];
					triangleData.m_vertices[1] = triangle.points[1];
					triangleData.m_vertices[2] = triangle.points[2];
					triangleData.m_normal = normal;
					triangleData.m_userData = facet.m_userData[0];
					if ((*discardTriangleCallback)(triangleData, transform, userDataForDiscardingTriangle)) {
#endif
						INCREASE_NUM_TRIS

						tempClosestFacetInfo.squaredDistance = pfxCalcInitialSquaredDistance(in);
						if ((*rayTriFuncPtr)(triangle, in, tempClosestFacetInfo)) {
							if (tempClosestFacetInfo.squaredDistance > closestFacetInfo.squaredDistance)
								continue;
#ifdef CAST_NO_RADIUS
							if ((tempClosestFacetInfo.squaredDistance < closestFacetInfo.squaredDistance && tempClosestFacetInfo.tmin == 0.0f) ||
								(tempClosestFacetInfo.tmin < closestFacetInfo.tmin - SCE_PFX_INTERSECT_COMMON_EPSILON)) {
#else
							PfxFloat angle = dot(normal, in.m_direction);
							if ((tempClosestFacetInfo.squaredDistance < closestFacetInfo.squaredDistance && tempClosestFacetInfo.tmin == 0.0f) ||
								(tempClosestFacetInfo.tmin < closestFacetInfo.tmin - SCE_PFX_INTERSECT_COMMON_EPSILON) ||
								(tempClosestFacetInfo.tmin < closestFacetInfo.tmin + SCE_PFX_INTERSECT_COMMON_EPSILON && angle <= closestFacetInfo.angle)) {
								closestFacetInfo.angle = angle;
#endif
								closestFacetInfo.hit = true;
								closestFacetInfo.facetId = facetsIds[f];
								closestFacetInfo.tmin = tempClosestFacetInfo.tmin;
								closestFacetInfo.contactPoint = tempClosestFacetInfo.contactPoint;
								closestFacetInfo.squaredDistance = tempClosestFacetInfo.squaredDistance;
								closestFacetInfo.islandId = islandId;

								rayLocalInfo.direction = rayLocalInfo.oriDirection * tempClosestFacetInfo.tmin;
								rayLocalInfo.endPos = rayLocalInfo.startPos + rayLocalInfo.direction;
								rayLocalInfo.aabb.vmin = minPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);
								rayLocalInfo.aabb.vmax = maxPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);

								rayGlobalInfo.direction = rayGlobalInfo.oriDirection * tempClosestFacetInfo.tmin;
								rayGlobalInfo.endPos = rayGlobalInfo.startPos + rayGlobalInfo.direction;
								rayGlobalInfo.aabb.vmin = minPerElem(rayGlobalInfo.startPos, rayGlobalInfo.endPos);
								rayGlobalInfo.aabb.vmax = maxPerElem(rayGlobalInfo.startPos, rayGlobalInfo.endPos);
							}
						}
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
					}
#endif
				}
			}
		}
	}
}

public:
#ifdef CAST_ENABLE_RIGIDBODY_CACHE
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
	PfxIntersectAllCastCachedIslandBvhPartialObj(
		pfxIntersectAllCastTriangleFuncPtr<CastInputClass> _rayTriFuncPtr,
		const PfxLargeTriMeshImpl *_largeMesh,
		const PfxUInt32 *_cachedIslandIds,
		PfxUInt32 _numCachedIslands,
		const PfxTransform3 &_transform,
		PfxBool _flipTriangle,
		pfxIntersectAllCastDiscardTriangleCallbackFuncPtr<CastDiscardTriangleData> _discardTriangleCallback,
		void *_userDataForDiscardingTriangle) :
		rayTriFuncPtr(_rayTriFuncPtr), largeMesh(_largeMesh), transform(_transform), cachedIslandIds(_cachedIslandIds), numCachedIslands(_numCachedIslands), 
		discardTriangleCallback(_discardTriangleCallback), userDataForDiscardingTriangle(_userDataForDiscardingTriangle)
#else
	PfxIntersectAllCastCachedIslandBvhObj(
		pfxIntersectAllCastTriangleFuncPtr<CastInputClass> _rayTriFuncPtr,
		const PfxLargeTriMeshImpl *_largeMesh,
		const PfxUInt32 *_cachedIslandIds,
		PfxUInt32 _numCachedIslands,
		const PfxTransform3 &_transform,
		PfxBool _flipTriangle) :
		rayTriFuncPtr(_rayTriFuncPtr), largeMesh(_largeMesh), transform(_transform), cachedIslandIds(_cachedIslandIds), numCachedIslands(_numCachedIslands)
#endif
#else
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
PfxIntersectAllCastTriMeshBvhPartialObj(
	pfxIntersectAllCastTriangleFuncPtr<CastInputClass> _rayTriFuncPtr,
	const PfxLargeTriMeshImpl *_largeMesh,
	const PfxTransform3 &_transform,
	PfxBool _flipTriangle, 
	pfxIntersectAllCastDiscardTriangleCallbackFuncPtr<CastDiscardTriangleData> _discardTriangleCallback,
	void *_userDataForDiscardingTriangle) : 
	rayTriFuncPtr(_rayTriFuncPtr), largeMesh(_largeMesh), transform(_transform),
	discardTriangleCallback(_discardTriangleCallback), userDataForDiscardingTriangle(_userDataForDiscardingTriangle)
#else
PfxIntersectAllCastTriMeshBvhObj(
	pfxIntersectAllCastTriangleFuncPtr<CastInputClass> _rayTriFuncPtr,
	const PfxLargeTriMeshImpl *_largeMesh,
	const PfxTransform3 &_transform,
	PfxBool _flipTriangle) : 
	rayTriFuncPtr(_rayTriFuncPtr), largeMesh(_largeMesh), transform(_transform)
#endif
#endif
{
	if (_flipTriangle) {
		ids[0] = 2;
		ids[1] = 1;
		ids[2] = 0;
		ids[3] = 3;
	}
	else {
		ids[0] = 0;
		ids[1] = 1;
		ids[2] = 2;
		ids[3] = 3;
	}
}

PfxBool intersectAllCastLargeTriMesh(const CastInputClass &in, CastOutputClass &out, PfxFloat radiusLocal)
{
	// レイをラージメッシュのローカル座標へ変換
	PfxTransform3 transformLMesh = inverse(transform);
	rayLocalInfo.startPos = transformLMesh.getUpper3x3() * in.m_startPosition + transformLMesh.getTranslation();
	rayLocalInfo.oriDirection = transformLMesh.getUpper3x3() * in.m_direction;
	rayLocalInfo.direction = rayLocalInfo.oriDirection * out.m_variable;
	rayLocalInfo.endPos = rayLocalInfo.startPos + rayLocalInfo.direction;
	rayLocalInfo.radiusVec = PfxVector3(radiusLocal);
	rayLocalInfo.aabb.vmin = minPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);
	rayLocalInfo.aabb.vmax = maxPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);

	rayGlobalInfo.startPos = in.m_startPosition;
	rayGlobalInfo.oriDirection = in.m_direction;
	rayGlobalInfo.direction = rayGlobalInfo.oriDirection * out.m_variable;
	rayGlobalInfo.endPos = rayGlobalInfo.startPos + rayGlobalInfo.direction;
	rayGlobalInfo.radiusVec = PfxVector3(pfxCalcCastRadius(in));
	rayGlobalInfo.aabb.vmin = minPerElem(rayGlobalInfo.startPos, rayGlobalInfo.endPos);
	rayGlobalInfo.aabb.vmax = maxPerElem(rayGlobalInfo.startPos, rayGlobalInfo.endPos);

	closestFacetInfo.hit = false;
	closestFacetInfo.tmin = out.m_variable;
	closestFacetInfo.angle = 1.0f;
	closestFacetInfo.islandId = 0;
	closestFacetInfo.facetId = 0;
	closestFacetInfo.squaredDistance = SCE_PFX_FLT_MAX;

#ifndef CAST_ENABLE_RIGIDBODY_CACHE
	{
		{
			PfxVector3 largeMeshAabbMin = largeMesh->m_offset - largeMesh->m_half;
			PfxVector3 largeMeshAabbMax = largeMesh->m_offset + largeMesh->m_half;
			if (!pfxTestRayBvByDotProductVector3(rayLocalInfo.startPos, rayLocalInfo.direction, rayLocalInfo.aabb, largeMeshAabbMin - rayLocalInfo.radiusVec, largeMeshAabbMax + rayLocalInfo.radiusVec))	return false;

			PfxIslandBvhNode &encnode = largeMesh->m_bvhNodes[0];
			PfxVector3 quantizedMin(encnode.aabb[0], encnode.aabb[1], encnode.aabb[2]);
			PfxVector3 quantizedMax(encnode.aabb[3], encnode.aabb[4], encnode.aabb[5]);

			EncStack2 encroot;
			encroot.nodeId = 0;
			encroot.aabbMin = largeMeshAabbMin + mulPerElem((largeMeshAabbMax - largeMeshAabbMin), quantizedMin * quantizationInverse);
			encroot.aabbMax = largeMeshAabbMax - mulPerElem((largeMeshAabbMax - largeMeshAabbMin), quantizedMax * quantizationInverse);
			encroot.nodeT = 0.f;

			bvhStack.push(encroot);
		}

		while (!bvhStack.empty()) {
			EncStack2 info = bvhStack.top();
			bvhStack.pop();

			PfxIslandBvhNode &encnode = largeMesh->m_bvhNodes[info.nodeId];

			INCREASE_NUM_BVHS

			PfxUInt32 numNextNodes = 0;
			EncStack2 nextNodes[2];

			PfxUInt32 numSelIslands = 0;
			PfxUInt32 selIslands[2];

			PfxUInt8 LStatus = encnode.flag & 0x03;
			PfxUInt8 RStatus = (encnode.flag & 0x0C) >> 2;

			if (LStatus == 0) {
				PfxIslandBvhNode &islandNode = largeMesh->m_bvhNodes[encnode.left];
				PfxVector3 quantizedMin(islandNode.aabb[0], islandNode.aabb[1], islandNode.aabb[2]);
				PfxVector3 quantizedMax(islandNode.aabb[3], islandNode.aabb[4], islandNode.aabb[5]);

				EncStack2 &encnext(nextNodes[numNextNodes]);
				encnext.nodeId = encnode.left;
				encnext.aabbMin = info.aabbMin + mulPerElem((info.aabbMax - info.aabbMin), quantizedMin * quantizationInverse);
				encnext.aabbMax = info.aabbMax - mulPerElem((info.aabbMax - info.aabbMin), quantizedMax * quantizationInverse);
				encnext.nodeT = dot(((encnext.aabbMax + encnext.aabbMin) * 0.5f - rayLocalInfo.startPos), rayLocalInfo.oriDirection);
//				encnext.nodeT = lengthSqr((encnext.aabbMax + encnext.aabbMin) * 0.5f - rayLocalInfo.startPos);

				if (pfxTestRayBvByDotProductVector3(rayLocalInfo.startPos, rayLocalInfo.direction, rayLocalInfo.aabb, encnext.aabbMin - rayLocalInfo.radiusVec, encnext.aabbMax + rayLocalInfo.radiusVec))
					++numNextNodes;
			}
			else if (LStatus == 1) {
				selIslands[numSelIslands++] = encnode.left;
			}

			if (RStatus == 0) {
				PfxIslandBvhNode &islandNode = largeMesh->m_bvhNodes[encnode.right];
				PfxVector3 quantizedMin(islandNode.aabb[0], islandNode.aabb[1], islandNode.aabb[2]);
				PfxVector3 quantizedMax(islandNode.aabb[3], islandNode.aabb[4], islandNode.aabb[5]);

				EncStack2 &encnext(nextNodes[numNextNodes]);
				encnext.nodeId = encnode.right;
				encnext.aabbMin = info.aabbMin + mulPerElem((info.aabbMax - info.aabbMin), quantizedMin * quantizationInverse);
				encnext.aabbMax = info.aabbMax - mulPerElem((info.aabbMax - info.aabbMin), quantizedMax * quantizationInverse);
				encnext.nodeT = dot(((encnext.aabbMax + encnext.aabbMin) * 0.5f - rayLocalInfo.startPos), rayLocalInfo.oriDirection);
//				encnext.nodeT = lengthSqr((encnext.aabbMax + encnext.aabbMin) * 0.5f - rayLocalInfo.startPos);

				if (pfxTestRayBvByDotProductVector3(rayLocalInfo.startPos, rayLocalInfo.direction, rayLocalInfo.aabb, encnext.aabbMin - rayLocalInfo.radiusVec, encnext.aabbMax + rayLocalInfo.radiusVec))
					++numNextNodes;
			}
			else if (RStatus == 1) {
				selIslands[numSelIslands++] = encnode.right;
			}

			if (numNextNodes == 2) {
				if (nextNodes[0].nodeT > nextNodes[1].nodeT) {
					bvhStack.push(nextNodes[0]);
					bvhStack.push(nextNodes[1]);
				}
				else {
					bvhStack.push(nextNodes[1]);
					bvhStack.push(nextNodes[0]);
				}
				continue;
			}
			else if (numNextNodes == 1) {
				bvhStack.push(nextNodes[0]);
			}

			for (PfxUInt32 i = 0; i < numSelIslands; i++) {
				PfxUInt32 islandId = selIslands[i];
				//	now, check whether this one intersects the island
				if (largeMesh->m_type == SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION) {
					PfxCompressedTriMesh *island = (PfxCompressedTriMesh*)largeMesh->m_islands + islandId;
					PfxVector3 islandAabbMin = pfxReadVector3(island->m_aabbMin);
					PfxVector3 islandAabbMax = pfxReadVector3(island->m_aabbMax);

					if (pfxTestRayBvByDotProductVector3(rayLocalInfo.startPos, rayLocalInfo.direction, rayLocalInfo.aabb, islandAabbMin - rayLocalInfo.radiusVec, islandAabbMax + rayLocalInfo.radiusVec))
						intersectAllCastCompressedTriMesh(in, island, islandId, islandAabbMin, islandAabbMax);
				}
				else {
					PfxQuantizedTriMeshBvh *island = (PfxQuantizedTriMeshBvh*)largeMesh->m_islands + islandId;
					PfxVector3 islandAabbMin = pfxReadVector3(island->m_aabbMin);
					PfxVector3 islandAabbMax = pfxReadVector3(island->m_aabbMax);

					if (pfxTestRayBvByDotProductVector3(rayLocalInfo.startPos, rayLocalInfo.direction, rayLocalInfo.aabb, islandAabbMin - rayLocalInfo.radiusVec, islandAabbMax + rayLocalInfo.radiusVec))
						intersectAllCastQuantizedTriMeshBvh(in, island, islandId, islandAabbMin, islandAabbMax);
				}
			}
		}
	}
#else
	for (PfxUInt32 i = 0; i < numCachedIslands; ++i) {
		PfxUInt32 islandId = cachedIslandIds[i];

		//	now, check whether this one intersects the island
		if (largeMesh->m_type == SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION) {
			PfxCompressedTriMesh *island = (PfxCompressedTriMesh*)largeMesh->m_islands + islandId;
			PfxVector3 islandAabbMin = pfxReadVector3(island->m_aabbMin);
			PfxVector3 islandAabbMax = pfxReadVector3(island->m_aabbMax);
			if (pfxTestRayBvByDotProductVector3(rayLocalInfo.startPos, rayLocalInfo.direction, rayLocalInfo.aabb, islandAabbMin - rayLocalInfo.radiusVec, islandAabbMax + rayLocalInfo.radiusVec))
				intersectAllCastCompressedTriMesh(in, island, islandId, islandAabbMin, islandAabbMax);
		}
		else {
			PfxQuantizedTriMeshBvh *island = (PfxQuantizedTriMeshBvh*)largeMesh->m_islands + islandId;
			PfxVector3 islandAabbMin = pfxReadVector3(island->m_aabbMin);
			PfxVector3 islandAabbMax = pfxReadVector3(island->m_aabbMax);

			if (pfxTestRayBvByDotProductVector3(rayLocalInfo.startPos, rayLocalInfo.direction, rayLocalInfo.aabb, islandAabbMin - rayLocalInfo.radiusVec, islandAabbMax + rayLocalInfo.radiusVec))
				intersectAllCastQuantizedTriMeshBvh(in, island, islandId, islandAabbMin, islandAabbMax);
		}
	}
#endif

	if (closestFacetInfo.hit) {
		PfxVector3 points[3];
		PfxUInt32 userData;
		userData = pfxGetFacetPointsAndUserData(*largeMesh, closestFacetInfo.islandId, closestFacetInfo.facetId, points);

		PfxTriangle triangle(
			transform.getTranslation() + transform.getUpper3x3() * points[ids[0]],
			transform.getTranslation() + transform.getUpper3x3() * points[ids[1]],
			transform.getTranslation() + transform.getUpper3x3() * points[ids[2]]);

		PfxVector3 stopPoint = in.m_startPosition + closestFacetInfo.tmin * in.m_direction;
		PfxVector3 contactPoint;

		pfxClosestPointTriangle(stopPoint, triangle, contactPoint);

		out.m_contactFlag = true;
		out.m_variable = closestFacetInfo.tmin;
		out.m_contactPoint = contactPoint;
		out.m_contactNormal = stopPoint - contactPoint;
		PfxFloat distanceSqr = lengthSqr(out.m_contactNormal);
		if (distanceSqr < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON ||
			radiusLocal == 0.0f ) // always use the triangle normal for line tests
		{
			out.m_contactNormal = triangle.calcNormal();
		}
		else {
			out.m_contactNormal = out.m_contactNormal / sqrtf(distanceSqr);
		}


		PfxFloat s = 0.0f, t = 0.0f;
		pfxCalcBarycentricCoords(contactPoint, triangle, s, t);
		out.m_subData.setType(PfxSubData::MESH_INFO);
		out.m_subData.setFacetLocalS(s);
		out.m_subData.setFacetLocalT(t);
		if (largeMesh->m_type == SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION) {
			const PfxCompressedTriMesh *island = (PfxCompressedTriMesh*)largeMesh->m_islands + closestFacetInfo.islandId;
			const PfxCompressedFacet2 &facet = ((PfxCompressedFacet2*)largeMesh->m_facetBuffer)[island->m_facets + (closestFacetInfo.facetId & 0xFFFF)];
			if (closestFacetInfo.facetId & 0x10000) {
				out.m_subData.setFacetId((facet.m_facetInfo & 0xFF) + 1);
			}
			else {
				out.m_subData.setFacetId(facet.m_facetInfo & 0xFF);
			}
		}
		else {
			out.m_subData.setFacetId(closestFacetInfo.facetId);
		}
		out.m_subData.setIslandId(closestFacetInfo.islandId);
		out.m_subData.setUserData(userData);
	}

	return closestFacetInfo.hit;
}
};

#ifdef CAST_ENABLE_RIGIDBODY_CACHE
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
template<typename CastInputClass, typename CastOutputClass, typename CastDiscardTriangleData>
class PfxIntersectAllCastCachedIslandArrayPartialObj
#else
template<typename CastInputClass, typename CastOutputClass>
class PfxIntersectAllCastCachedIslandArrayObj
#endif
#else
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
template<typename CastInputClass, typename CastOutputClass, typename CastDiscardTriangleData>
class PfxIntersectAllCastTriMeshArrayPartialObj
#else
template<typename CastInputClass, typename CastOutputClass>
class PfxIntersectAllCastTriMeshArrayObj
#endif
#endif
{
	PfxRayBvInfo rayLocalInfo;
	PfxClosestFacetInfo closestFacetInfo;
	pfxIntersectAllCastTriangleFuncPtr<CastInputClass> rayTriFuncPtr;

	const PfxLargeTriMeshImpl *largeMesh;
	const PfxTransform3 transform;
	PfxUInt32 ids[3];

	PfxStaticStack<EncStack2> islandStack;
	PfxStaticStack<EncStack2, 64> bvhStack;

#ifdef CAST_ENABLE_RIGIDBODY_CACHE
	const PfxUInt32 *cachedIslandIds;
	const PfxUInt32 numCachedIslands;
#endif

#ifdef CAST_ENABLE_DISCARD_TRIANGLE
	pfxIntersectAllCastDiscardTriangleCallbackFuncPtr<CastDiscardTriangleData> discardTriangleCallback;
	void *userDataForDiscardingTriangle;
#endif

void intersectAllCastExpandedTriMeshArray(const CastInputClass &in, const PfxExpandedTriMesh *mesh, PfxUInt32 islandId)
{
	PfxClosestFacetInfo tempClosestFacetInfo;
	tempClosestFacetInfo.tmin = closestFacetInfo.tmin;

	for (PfxUInt32 f = 0; f < mesh->m_numFacets; f++) {
		const PfxExpandedFacet &facet = mesh->m_facets[f];
		PfxVector3 center = pfxReadVector3((PfxFloat*)&facet.m_center);
		PfxVector3 half = pfxReadVector3((PfxFloat*)&facet.m_half) + rayLocalInfo.radiusVec;

		if (!pfxTestRayBvByDotProductVector3(rayLocalInfo.startPos, rayLocalInfo.direction, rayLocalInfo.aabb, center - half, center + half))	continue;

		PfxTriangle triangle(
			transform.getTranslation() + transform.getUpper3x3() * mesh->m_verts[facet.m_vertIds[ids[0]]],
			transform.getTranslation() + transform.getUpper3x3() * mesh->m_verts[facet.m_vertIds[ids[1]]],
			transform.getTranslation() + transform.getUpper3x3() * mesh->m_verts[facet.m_vertIds[ids[2]]]);

		PfxVector3 normal = triangle.calcNormal();
		if (dot(normal, in.m_startPosition - triangle.points[0]) <= 0.0f) {
			continue;
		}

#ifdef CAST_ENABLE_DISCARD_TRIANGLE
		{
			CastDiscardTriangleData triangleData;
			triangleData.m_vertices[0] = triangle.points[0];
			triangleData.m_vertices[1] = triangle.points[1];
			triangleData.m_vertices[2] = triangle.points[2];
			triangleData.m_normal = normal;
			triangleData.m_userData = facet.m_userData;
			if (!(*discardTriangleCallback)(triangleData, transform, userDataForDiscardingTriangle))
				continue;
		}
#endif
		INCREASE_NUM_TRIS

		tempClosestFacetInfo.squaredDistance = pfxCalcInitialSquaredDistance(in);
		if ((*rayTriFuncPtr)(triangle, in, tempClosestFacetInfo)) {
			if (tempClosestFacetInfo.squaredDistance > closestFacetInfo.squaredDistance)
				continue;

#ifdef CAST_NO_RADIUS
			if ((tempClosestFacetInfo.squaredDistance < closestFacetInfo.squaredDistance && tempClosestFacetInfo.tmin == 0.0f) ||
				(tempClosestFacetInfo.tmin < closestFacetInfo.tmin - SCE_PFX_INTERSECT_COMMON_EPSILON)) {
#else
			PfxFloat angle = dot(normal, in.m_direction);
			if ((tempClosestFacetInfo.squaredDistance < closestFacetInfo.squaredDistance && tempClosestFacetInfo.tmin == 0.0f) ||
				// if it happens earlier just accept the contact
				(tempClosestFacetInfo.tmin < closestFacetInfo.tmin - SCE_PFX_INTERSECT_COMMON_EPSILON) ||
				// if it happens approximately at the same time only accept if it has a more incident angle
				(tempClosestFacetInfo.tmin < closestFacetInfo.tmin + SCE_PFX_INTERSECT_COMMON_EPSILON && angle <= closestFacetInfo.angle)) {
				closestFacetInfo.angle = angle;
#endif
				closestFacetInfo.hit = true;
				closestFacetInfo.facetId = f;
				closestFacetInfo.tmin = tempClosestFacetInfo.tmin;
				closestFacetInfo.contactPoint = tempClosestFacetInfo.contactPoint;
				closestFacetInfo.squaredDistance = tempClosestFacetInfo.squaredDistance;
				closestFacetInfo.islandId = islandId;

				rayLocalInfo.direction = rayLocalInfo.oriDirection * tempClosestFacetInfo.tmin;
				rayLocalInfo.endPos = rayLocalInfo.startPos + rayLocalInfo.direction;
				rayLocalInfo.aabb.vmin = minPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);
				rayLocalInfo.aabb.vmax = maxPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);
			}
		}
	}
}

public:
#ifdef CAST_ENABLE_RIGIDBODY_CACHE
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
	PfxIntersectAllCastCachedIslandArrayPartialObj(
		pfxIntersectAllCastTriangleFuncPtr<CastInputClass> _rayTriFuncPtr,
		const PfxLargeTriMeshImpl *_largeMesh,
		const PfxUInt32 *_cachedIslandIds,
		PfxUInt32 _numCachedIslands,
		const PfxTransform3 &_transform,
		PfxBool _flipTriangle,
		pfxIntersectAllCastDiscardTriangleCallbackFuncPtr<CastDiscardTriangleData> _discardTriangleCallback,
		void *_userDataForDiscardingTriangle) :
		rayTriFuncPtr(_rayTriFuncPtr), largeMesh(_largeMesh), transform(_transform), cachedIslandIds(_cachedIslandIds), numCachedIslands(_numCachedIslands),
		discardTriangleCallback(_discardTriangleCallback), userDataForDiscardingTriangle(_userDataForDiscardingTriangle)
#else
	PfxIntersectAllCastCachedIslandArrayObj(
		pfxIntersectAllCastTriangleFuncPtr<CastInputClass> _rayTriFuncPtr,
		const PfxLargeTriMeshImpl *_largeMesh,
		const PfxUInt32 *_cachedIslandIds,
		PfxUInt32 _numCachedIslands,
		const PfxTransform3 &_transform,
		PfxBool _flipTriangle) :
		rayTriFuncPtr(_rayTriFuncPtr), largeMesh(_largeMesh), transform(_transform), cachedIslandIds(_cachedIslandIds), numCachedIslands(_numCachedIslands)
#endif
#else
#ifdef CAST_ENABLE_DISCARD_TRIANGLE
PfxIntersectAllCastTriMeshArrayPartialObj(
	pfxIntersectAllCastTriangleFuncPtr<CastInputClass> _rayTriFuncPtr,
	const PfxLargeTriMeshImpl *_largeMesh,
	const PfxTransform3 &_transform,
	PfxBool _flipTriangle, 
	pfxIntersectAllCastDiscardTriangleCallbackFuncPtr<CastDiscardTriangleData> _discardTriangleCallback,
	void *_userDataForDiscardingTriangle) : 
	rayTriFuncPtr(_rayTriFuncPtr), largeMesh(_largeMesh), transform(_transform),
	discardTriangleCallback(_discardTriangleCallback), userDataForDiscardingTriangle(_userDataForDiscardingTriangle)
#else
PfxIntersectAllCastTriMeshArrayObj(
	pfxIntersectAllCastTriangleFuncPtr<CastInputClass> _rayTriFuncPtr,
	const PfxLargeTriMeshImpl *_largeMesh,
	const PfxTransform3 &_transform,
	PfxBool _flipTriangle) : 
	rayTriFuncPtr(_rayTriFuncPtr), largeMesh(_largeMesh), transform(_transform)
#endif
#endif
{
	if (_flipTriangle) {
		ids[0] = 2;
		ids[1] = 1;
		ids[2] = 0;
	}
	else {
		ids[0] = 0;
		ids[1] = 1;
		ids[2] = 2;
	}
}

PfxBool intersectAllCastLargeTriMesh(const CastInputClass &in, CastOutputClass &out, PfxFloat radiusLocal)
{
	// レイをラージメッシュのローカル座標へ変換
	PfxTransform3 transformLMesh = inverse(transform);
	rayLocalInfo.startPos = transformLMesh.getUpper3x3() * in.m_startPosition + transformLMesh.getTranslation();
	rayLocalInfo.oriDirection = transformLMesh.getUpper3x3() * in.m_direction;
	rayLocalInfo.direction = rayLocalInfo.oriDirection * out.m_variable;
	rayLocalInfo.endPos = rayLocalInfo.startPos + rayLocalInfo.direction;
	rayLocalInfo.radiusVec = PfxVector3(radiusLocal);
	rayLocalInfo.aabb.vmin = minPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);
	rayLocalInfo.aabb.vmax = maxPerElem(rayLocalInfo.startPos, rayLocalInfo.endPos);

	closestFacetInfo.hit = false;
	closestFacetInfo.tmin = out.m_variable;
	closestFacetInfo.angle = 1.0f;
	closestFacetInfo.islandId = 0;
	closestFacetInfo.facetId = 0;
	closestFacetInfo.squaredDistance = SCE_PFX_FLT_MAX;

	PfxVecInt3 spr, snr, epr, enr, aabbMinL, aabbMaxL;
	spr = largeMesh->getLocalPosition(rayLocalInfo.startPos + rayLocalInfo.radiusVec);
	snr = largeMesh->getLocalPosition(rayLocalInfo.startPos - rayLocalInfo.radiusVec);
	epr = largeMesh->getLocalPosition(rayLocalInfo.endPos + rayLocalInfo.radiusVec);
	enr = largeMesh->getLocalPosition(rayLocalInfo.endPos - rayLocalInfo.radiusVec);

	aabbMinL = minPerElem(minPerElem(minPerElem(spr, snr), epr), enr);
	aabbMaxL = maxPerElem(maxPerElem(maxPerElem(spr, snr), epr), enr);

	PfxUInt32 numIslands = largeMesh->m_numIslands;
	for (PfxUInt32 i = 0; i<numIslands; i++) {
		PfxUInt32 islandId = i;

		PfxAabb16 aabbB = largeMesh->m_aabbList[islandId];
		if (aabbMaxL.getX() < pfxGetXMin(aabbB) || aabbMinL.getX() > pfxGetXMax(aabbB)) continue;
		if (aabbMaxL.getY() < pfxGetYMin(aabbB) || aabbMinL.getY() > pfxGetYMax(aabbB)) continue;
		if (aabbMaxL.getZ() < pfxGetZMin(aabbB) || aabbMinL.getZ() > pfxGetZMax(aabbB)) continue;

		INCREASE_NUM_BVHS

		PfxVector3 aabbMin = largeMesh->getWorldPosition(PfxVecInt3((PfxFloat)pfxGetXMin(aabbB), (PfxFloat)pfxGetYMin(aabbB), (PfxFloat)pfxGetZMin(aabbB)));
		PfxVector3 aabbMax = largeMesh->getWorldPosition(PfxVecInt3((PfxFloat)pfxGetXMax(aabbB), (PfxFloat)pfxGetYMax(aabbB), (PfxFloat)pfxGetZMax(aabbB)));

		if (!pfxTestRayBvByDotProductVector3(rayLocalInfo.startPos, rayLocalInfo.direction, rayLocalInfo.aabb, aabbMin - rayLocalInfo.radiusVec, aabbMax + rayLocalInfo.radiusVec))	continue;

		INCREASE_NUM_ISLANDS

		// アイランドとの交差チェック
		PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largeMesh->m_islands) + islandId;

		intersectAllCastExpandedTriMeshArray(in, island, islandId);
	}

	if (closestFacetInfo.hit) {
		PfxVector3 points[3];
		PfxUInt32 userData;
		userData = pfxGetFacetPointsAndUserData(*largeMesh, closestFacetInfo.islandId, closestFacetInfo.facetId, points);

		PfxTriangle triangle(
			transform.getTranslation() + transform.getUpper3x3() * points[ids[0]],
			transform.getTranslation() + transform.getUpper3x3() * points[ids[1]],
			transform.getTranslation() + transform.getUpper3x3() * points[ids[2]]);

		PfxVector3 stopPoint = in.m_startPosition + closestFacetInfo.tmin * in.m_direction;

		out.m_contactFlag = true;
		out.m_variable = closestFacetInfo.tmin;
		out.m_contactPoint = closestFacetInfo.contactPoint;
		out.m_contactNormal = (stopPoint - closestFacetInfo.contactPoint);
		PfxFloat distanceSqr = lengthSqr(out.m_contactNormal);
		if (distanceSqr < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON || 
			radiusLocal == 0.0f ) // always use the triangle normal for line tests
		{
			out.m_contactNormal = triangle.calcNormal();
		}
		else {
			out.m_contactNormal = out.m_contactNormal / sqrtf(distanceSqr);
		}

		PfxFloat s = 0.0f, t = 0.0f;
		pfxCalcBarycentricCoords(closestFacetInfo.contactPoint, triangle, s, t);
		out.m_subData.setType(PfxSubData::MESH_INFO);
		out.m_subData.setFacetLocalS(s);
		out.m_subData.setFacetLocalT(t);
		out.m_subData.setFacetId(closestFacetInfo.facetId);
		out.m_subData.setIslandId(closestFacetInfo.islandId);
		out.m_subData.setUserData(userData);
	}
	return closestFacetInfo.hit;
}
};

#endif

