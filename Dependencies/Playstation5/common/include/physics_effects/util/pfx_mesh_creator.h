/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_MESH_CREATOR_H
#define _SCE_PFX_MESH_CREATOR_H

#include "../base_level/collision/pfx_convex_mesh.h"
#include "../base_level/collision/pfx_large_tri_mesh.h"

namespace sce {
namespace pfxv4 {

/// @name Mesh creation

/// Mesh creation API

//E Specify these values to a flag parameter
#define SCE_PFX_MESH_FLAG_NORMAL_FLIP					0x01	///< Inverts triangle index sequence
#define SCE_PFX_MESH_FLAG_16BIT_INDEX					0x02	///< Makes triangle index a 16-bit integer
#define SCE_PFX_MESH_FLAG_32BIT_INDEX					0x04	///< Makes triangle index a 32-bit integer
#define SCE_PFX_MESH_FLAG_AUTO_ELIMINATION				0x08	///< Deletes same vertices and triangles with 0 area
#define SCE_PFX_MESH_FLAG_OUTPUT_INFO					0x10	///< Outputs messages when creating a mesh
#define SCE_PFX_MESH_FLAG_OPTIMIZE_BVH_STRUCTURE		0x20	///< Optimizes BVH structure (only effective for BVH)
#define SCE_PFX_MESH_FLAG_FAST_BVH_CONSTRUCTION			0x40	///< Accelerate BVH construction (only effective for BVH)
#define SCE_PFX_MESH_FLAG_FILL_REMOVED_FACETS			0x80	///< Fill holes caused by removed facets

// Structures of a large tri mesh
// If one of them is not specified, the default structure (array) is chosen.
#define SCE_PFX_MESH_FLAG_STRUCTURE_STANDARD_BVH		0x100	///< Compressed BVH structure
#define SCE_PFX_MESH_FLAG_STRUCTURE_COMPRESSED_BVH		0x200	///< High compressed BVH structure

///////////////////////////////////////////////////////////////////////////////
// Convex Mesh

/// @brief Parameters used for convex mesh creation function
/// @details This structure is used to specify parameters when creating a convex mesh with the pfxCreateConvexMesh() function.
/// Flag can be specified by combination of following types.
/// @arg SCE_PFX_MESH_FLAG_NORMAL_FLIP Inverts triangle index sequence
/// @arg SCE_PFX_MESH_FLAG_16BIT_INDEX Makes triangle index a 16-bit integer (unsigned)
/// @arg SCE_PFX_MESH_FLAG_32BIT_INDEX Makes triangle index a 32-bit integer (unsigned)
/// @arg SCE_PFX_MESH_FLAG_AUTO_ELIMINATION Deletes triangles with same vertex and surface area of 0
/// @arg SCE_PFX_MESH_FLAG_FILL_REMOVED_FACETS Fills holes of deleted triangles
struct SCE_PFX_API PfxCreateConvexMeshParam {
	PfxUInt32	flag = SCE_PFX_MESH_FLAG_16BIT_INDEX | SCE_PFX_MESH_FLAG_AUTO_ELIMINATION; ///< @brief Flag (default value: SCE_PFX_MESH_FLAG_16BIT_INDEX|SCE_PFX_MESH_FLAG_AUTO_ELIMINATION)
	SCE_PFX_PADDING4
	PfxFloat*	verts = nullptr;		///< @brief Pointer to vertex buffer 
	PfxUInt32	numVerts = 0;			///< @brief Number of vertices
	SCE_PFX_PADDING4
	void*		triangles = nullptr;	///< @brief Pointer to triangle buffer
	PfxUInt32 *userData = nullptr;		///< @brief User data per each triangle (this is optional)
	PfxUInt32	numTriangles = 0;		///< @brief Number of triangles
	PfxUInt32	vertexStrideBytes = sizeof(PfxFloat) * 3;		///< @brief Number of bytes between vertex data (default value: 12)
	PfxUInt32	triangleStrideBytes = sizeof(PfxUInt16) * 3;	///< @brief Number of bytes between triangle data (default value: 6)
	SCE_PFX_PADDING4
	PfxInt8 name[SCE_PFX_CONVEXMESH_NAME_STR_MAX] = { 'u','n','k','n','w','n' };		///< @brief Name of the mesh used for debugging
};

/// @brief Create convex mesh
/// @details Creates a convex mesh from the vertices and triangle buffers.
/// Whether the input shape is a convex shape is not judged within the function.
/// The buffer for storing the vertices and triangles when creating convex meshes is allocated dynamically. 
/// After use, always release with pfxReleaseConvexMesh().
/// Up to 128 vertices or 64 triangles can be held by a convex mesh. If the designated value is exceeded,
/// the SCE_PFX_ERR_OUT_OF_RANGE error is returned.
/// @param[out] convexMesh Convex mesh
/// @param param Input parameter
/// @return Returns SCE_PFX_OK(0) upon normal termination.
/// @return Returns one of the following error codes (negative value) for errors.
/// @retval SCE_PFX_ERR_INVALID_VALUE The number of vertices or triangles is 0 or NULL has been specified for the pointer
/// @retval SCE_PFX_ERR_OUT_OF_RANGE The number of vertices or triangles exceeds the designated number
/// @retval SCE_PFX_ERR_INVALID_FLAG The flag setting is incorrect
SCE_PFX_API PfxInt32 pfxCreateConvexMesh(PfxConvexMesh &convexMesh,const PfxCreateConvexMeshParam &param);

/// @brief Release convex mesh
/// Releases the buffer allocated with pfxCreateConvexMesh().
/// @param convexMesh Convex mesh
SCE_PFX_API void pfxReleaseConvexMesh(PfxConvexMesh &convexMesh);

///////////////////////////////////////////////////////////////////////////////
// Large Mesh

/// @brief Parameters set by the mesh creator
/// @details This statistics informs you that how much an input mesh is proper. 
struct SCE_PFX_API PfxCreateLargeTriMeshStatistics {
	PfxUInt32 numRemovedTriangles = 0;
	PfxUInt32 numRemovedVertices = 0;
	PfxUInt32 numEdgesSharedByMoreThan2Triangles = 0;
	PfxUInt32 numTotalConvertedVertices = 0;
	PfxUInt32 numTotalConvertedTriangles = 0;
};

/// @brief Parameters used for large mesh creation
/// @details This structure is used to specify the parameters for creating a large mesh with the pfxCreateLargeTriMesh() function.
/// The following combinations of values are input to flag.
/// @arg SCE_PFX_MESH_FLAG_NORMAL_FLIP Inverts triangle index sequence
/// @arg SCE_PFX_MESH_FLAG_16BIT_INDEX Makes triangle index a 16-bit integer (unsigned)
/// @arg SCE_PFX_MESH_FLAG_32BIT_INDEX Makes triangle index a 32-bit integer (unsigned)
/// @arg SCE_PFX_MESH_FLAG_AUTO_ELIMINATION Deletes triangles with same vertex and surface area of 0
/// @arg SCE_PFX_MESH_FLAG_OPTIMIZE_BVH_STRUCTURE Optimizes BVH structure (only effective for BVH)
/// @arg SCE_PFX_MESH_FLAG_STRUCTURE_STANDARD_BVH compressed BVH structure
/// @arg SCE_PFX_MESH_FLAG_STRUCTURE_COMPRESSED_BVH High compressed BVH structure
/// @arg SCE_PFX_MESH_FLAG_OUTPUT_INFO Outputs messages when creating a mesh
/// @arg SCE_PFX_MESH_FLAG_FILL_REMOVED_FACETS Fills holes of deleted triangles
struct SCE_PFX_API PfxCreateLargeTriMeshParam {
	PfxUInt32 flag = SCE_PFX_MESH_FLAG_16BIT_INDEX | 
		SCE_PFX_MESH_FLAG_OPTIMIZE_BVH_STRUCTURE | 
		SCE_PFX_MESH_FLAG_AUTO_ELIMINATION | 
		SCE_PFX_MESH_FLAG_OUTPUT_INFO; ///< @brief Flag
	SCE_PFX_PADDING4
	PfxFloat *verts = nullptr;			///< @brief Pointer to vertex buffer 
	PfxUInt32 numVerts = 0;				///< @brief Number of vertices
	SCE_PFX_PADDING4
	PfxCreateLargeTriMeshStatistics *statistics = nullptr; ///< @brief Information from the mesh conversion process
	void *triangles = nullptr;			///< @brief Pointer to triangle buffer
	PfxUInt32 *userData = nullptr;		///< @brief Pointer to user data buffer (array of 32-bit integer values)
	PfxUInt32 numTriangles = 0;			///< @brief Number of triangles
	PfxUInt32 vertexStrideBytes = sizeof(PfxFloat) * 3;	///< @brief Number of bytes between vertex data (default value: 12)
	PfxUInt32 triangleStrideBytes = sizeof(PfxUInt16) * 3;	///< @brief Number of bytes between triangle data (default value: 6)
	PfxUInt32 numFacetsLimit = 64;		///< @brief Threshold of number of triangles registered to island (default value: 64)
	PfxFloat facetAreaLimit = 0.001f;	///< @brief Facets less than this value are removed. (default value: 0.001f)
	PfxFloat defaultThickness = 0.025f;	///< @brief Default value of triangle thickness (default value: 0.025f)
	PfxInt8 name[SCE_PFX_LARGETRIMESH_NAME_STR_MAX] = { '<','u','n','k','n','o','w','n','>','\0' };	///< @brief Name of the mesh used for debugging
};

/// @brief Create large mesh
///
/// Creates a large mesh from the vertices and triangle buffers.
/// The mesh structure can be efficiently searched hierarchically by dividing the input triangle into
/// multiple groups (islands). The maximum number of islands that can be held by a large mesh is 65536.
/// Up to 128 vertices and 64 triangles can be held by an island. If the designated value is exceeded,
/// the SCE_PFX_ERR_OUT_OF_RANGE error is returned. Arbitrary user data (32-bit integer values) can be
/// assigned to individual triangles. The buffer for storing the islands or additional data during large
/// mesh creation is allocated dynamically. 
/// Following use, be sure to release the allocated buffer with the pfxReleaseLargeTriMesh() function.
/// Although the edge shared by three or more triangles does not result in an error, a warning message
/// will be output, and the third or later triangles will be excluded from the judgment.
/// @param largeMesh Large mesh
/// @param param Input parameter
/// @return Returns SCE_PFX_OK(0) upon normal termination.
/// @return Returns one of the following error codes (negative value) for errors.
/// @retval SCE_PFX_ERR_INVALID_VALUE The number of vertices or triangles is 0 or NULL has been specified for the pointer.
/// @retval SCE_PFX_ERR_OUT_OF_RANGE The number of input parameter vertices or triangles exceeds the designated number.
/// @retval SCE_PFX_ERR_OUT_OF_BUFFER Buffer could not be allocated due to insufficient memory.
/// @retval SCE_PFX_ERR_INVALID_FLAG The flag setting is invalid.
/// @retval SCE_PFX_ERR_OUT_OF_RANGE_VERTEX The number of vertices exceeds the maximum number that can be included in one island.
/// @retval SCE_PFX_ERR_OUT_OF_RANGE_EDGE The number of edges exceeds the maximum number that can be included in one island.
/// @retval SCE_PFX_ERR_OUT_OF_RANGE_FACET The number of triangles exceeds the maximum number that can be included in one island.
/// @retval SCE_PFX_ERR_OUT_OF_RANGE_ISLAND The number of islands exceeds the maximum number that can be included in a large mesh.
/// @retval SCE_PFX_ERR_ZERO_AREA_FACET Detects a triangle whose surface area after compression will be 0
SCE_PFX_API PfxInt32 pfxCreateLargeTriMesh(PfxLargeTriMesh &largeMesh,const PfxCreateLargeTriMeshParam &param);

/// @brief Release large mesh
/// @details Releases the buffer allocated with pfxCreateLargeTriMesh().
/// @param largeMesh Convex mesh
SCE_PFX_API void pfxReleaseLargeTriMesh(PfxLargeTriMesh &largeMesh);


} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_MESH_CREATOR_H
