/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_LARGE_TRI_MESH_H
#define _SCE_PFX_LARGE_TRI_MESH_H

#include "../base/pfx_common.h"

#define SCE_PFX_MAX_LARGETRIMESH_ISLANDS 65535
#define SCE_PFX_LARGETRIMESH_NAME_STR_MAX 32

namespace sce {
namespace pfxv4 {

/// @brief Large mesh
struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxLargeTriMesh
{
	PfxUInt8 reserved[176];
};

/// @brief Facet
/// @details It is the structure for storing facet information.
struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxLargeTriMeshFacet
{
	PfxVector3 normal;		///< @brief Normal vector of the facet
	PfxFloat thickness;		///< @brief Thickness of the facet
	PfxUInt32 userData;		///< @brief User data of the facet
	PfxUInt8 vertIds[3];	///< @brief Vertex indices of the facet
	PfxUInt8 edgeType;		///< @brief Edge type
	PfxFloat edgeTilt[3];	///< @brief Edge angle of tilt
};

/// @brief Get the number of islands
/// @details Get the number of islands stored in the large mesh.
/// @param largeMesh Large mesh
/// @return Return the number of islands
SCE_PFX_API PfxUInt32 pfxLargeTriMeshGetNumIslands(const PfxLargeTriMesh &largeMesh);

/// @brief Get AABB of the island
/// @details Get AABB of the island stored in the large mesh.
/// @param largeMesh Large mesh
/// @param islandId Island ID
/// @param[out] aabbMin Minimum position of the island
/// @param[out] aabbMax Maximum position of the island
SCE_PFX_API void pfxLargeTriMeshGetIslandAabb(const PfxLargeTriMesh &largeMesh,PfxUInt32 islandId,PfxVector3 &aabbMin,PfxVector3 &aabbMax);

/// @brief Get the number of vertices of the island
/// @details Get the number of vertices of the island stored in the large mesh.
/// @param largeMesh Large mesh
/// @param islandId Island ID
/// @return Return the number of vertices
SCE_PFX_API PfxUInt32 pfxLargeTriMeshGetNumVerticesInIslands(const PfxLargeTriMesh &largeMesh,PfxUInt32 islandId);

/// @brief Get the vertex of the island
/// @details Get the vertex of the island stored in the large mesh.
/// @param largeMesh Large mesh
/// @param islandId Island ID
/// @param vertexId vertex ID
/// @return Return the vertex
SCE_PFX_API PfxVector3 pfxLargeTriMeshGetVertexInIslands(const PfxLargeTriMesh &largeMesh,PfxUInt32 islandId,PfxUInt32 vertexId);

/// @brief Get the number of facets of the island
/// @details Get the number of facets of the island stored in the large mesh.
/// @param largeMesh Large mesh
/// @param islandId Island ID
/// @return Return the number of facets
SCE_PFX_API PfxUInt32 pfxLargeTriMeshGetNumFacetsInIslands(const PfxLargeTriMesh &largeMesh,PfxUInt32 islandId);

/// @brief Get the facet of the island
/// @details Get the facet of the island stored in the large mesh.
/// @param largeMesh Large mesh
/// @param islandId Island ID
/// @param facetId facet ID
/// @return Return the facet
SCE_PFX_API PfxLargeTriMeshFacet pfxLargeTriMeshGetFacetInIslands(const PfxLargeTriMesh &largeMesh,PfxUInt32 islandId,PfxUInt32 facetId);

/// @brief Set a vertex
/// @details Set a vertex of the island stored in the large mesh.
/// You can set vertices to the uncompressed large mesh only.
/// @param largeMesh Large mesh
/// @param islandId Island ID
/// @param vertexId vertex ID
/// @param vertex vertex coordinate
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxUInt32 pfxLargeTriMeshSetVertexInIslands(PfxLargeTriMesh &largeMesh,PfxUInt32 islandId,PfxUInt32 vertexId,const PfxVector3 &vertex);

/// @brief Rebuild the large mesh
/// @details The large mesh needs to be reconstructed after changing it's vertices.
/// To recalculate a bounding box, call finish() of PfxCollidable which includes the large mesh.
/// @param largeMesh Large mesh
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxUInt32 pfxLargeTriMeshRebuild(PfxLargeTriMesh &largeMesh);

SCE_PFX_API const PfxUInt8 *pfxLargeTriMeshGetName(const PfxLargeTriMesh &largeMesh);

} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_LARGE_TRI_MESH_H
