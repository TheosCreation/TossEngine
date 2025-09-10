/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_CONVEX_MESH_H
#define _SCE_PFX_CONVEX_MESH_H

#include "../base/pfx_common.h"

#define SCE_PFX_CONVEXMESH_NAME_STR_MAX 6

namespace sce {
namespace pfxv4 {

/// @name Convex mesh

/// @brief Convex mesh
/// @details This structure expresses convex meshes. To create it, use the pfxCreateConvexMesh() utility function.
/// 16-byte alignment is required.
struct SCE_PFX_API PfxConvexMesh
{
#ifdef SCE_PFX_ENABLE_CUBEMAP_CONVEX_OPTIMIZE
	PfxUInt8 reserved[1232];
#else
	PfxUInt8 reserved[64];
#endif
};

/// @brief Get the number of vertices
/// @details Get the number of vertices stored in the convex mesh.
/// @param convexMesh Convex mesh
/// @return Return the number of vertices
SCE_PFX_API PfxUInt32 pfxConvexMeshGetNumVertices(const PfxConvexMesh &convexMesh);

/// @brief Get the vertex
/// @details Get the vertex of the convex mesh by a vertex ID.
/// @param convexMesh Convex mesh
/// @param vertexId vertex ID
/// @return Return the vertex
SCE_PFX_API PfxVector3 pfxConvexMeshGetVertex(const PfxConvexMesh &convexMesh,PfxUInt32 vertexId);

/// @brief Get the number of indices
/// @details Get the number of indices stored in the convex mesh.
/// @param convexMesh Convex mesh
/// @return Return the number of indices
SCE_PFX_API PfxUInt32 pfxConvexMeshGetNumIndices(const PfxConvexMesh &convexMesh);

/// @brief Get the index
/// @details Get the index of the convex mesh by index ID.
/// @param convexMesh Convex mesh
/// @param indexId index ID
/// @return Return the index
SCE_PFX_API PfxUInt16 pfxConvexMeshGetIndex(const PfxConvexMesh &convexMesh,PfxUInt32 indexId);

/// @brief Check if the convex mesh has user data
/// @details Check if the convex mesh has per triangle user data
/// @param convexMesh Convex mesh
/// @return Return true if the convex mesh has per triangle user data
SCE_PFX_API PfxBool pfxConvexMeshHasPerTriangleUserData(const PfxConvexMesh &convexMesh);

/// @brief Get user data
/// @details Get user data specified by triangle's index
/// @param convexMesh Convex mesh
/// @param triangleId Triangle index
/// @return Return triangle's user data. If the convex mesh doesn't have user data, it returns 0xFFFFFFFF.
SCE_PFX_API PfxUInt32 pfxConvexMeshGetUserDataForTriangle(const PfxConvexMesh &convexMesh,PfxUInt16 triangleId);

SCE_PFX_API const PfxUInt8 *pfxConvexMeshGetName(const PfxConvexMesh &convexMesh);

} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_CONVEX_MESH_H
