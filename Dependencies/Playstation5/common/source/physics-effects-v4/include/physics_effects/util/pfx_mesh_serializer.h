/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_MESH_SERIALIZER_H
#define _SCE_PFX_MESH_SERIALIZER_H

#include "../base_level/collision/pfx_convex_mesh.h"
#include "../base_level/collision/pfx_large_tri_mesh.h"

namespace sce {
namespace pfxv4 {

/// @name Mesh serializer

/// Mesh binary serialize API

/// @brief Serialize the convex mesh
/// @details This function serialize a convex mesh into the specified buffer. pfxQuerySerializedBytesOfConvexMesh() 
/// needs to be used to calculate the size of buffer
/// @param convexMesh Convex mesh
/// @param[out] meshBuff Serialized binary data
/// @param[out] meshBytes Size of binary data
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxSerializeConvexMesh(const PfxConvexMesh &convexMesh,PfxUInt8 *meshBuff,PfxUInt32 meshBytes);

/// @brief Restore the convex mesh
/// @details This function restores the convex mesh from the binary data serialized by pfxSerializeConvexMesh().
/// The size of buffer to store restored data is obtained by pfxQueryBytesToRestoreConvexMesh().
/// @param[out] convexMesh Convex mesh
/// @param[out] outBuff Used to store mesh data
/// @param[out] outBytes Size of outBuff
/// @param[in] meshBuff Serialized binary data  (16 bytes align)
/// @param[in] meshBytes Size of binary data
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxRestoreConvexMesh(PfxConvexMesh &convexMesh,PfxUInt8 *outBuff,PfxUInt32 outBytes, const PfxUInt8 *meshBuff,PfxUInt32 meshBytes);

/// @brief Serialize the large mesh
/// @details This function serialize a convex mesh into the specified buffer. pfxQuerySerializedBytesOfLargeTriMesh() 
/// needs to be used to calculate the size of buffer
/// @param largeMesh Large mesh
/// @param[out] meshBuff Serialized binary data
/// @param[out] meshBytes Size of binary data
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxSerializeLargeTriMesh(const PfxLargeTriMesh &largeMesh,PfxUInt8 *meshBuff,PfxUInt32 meshBytes);

/// @brief Restore the large mesh from binary data
/// @details This function restores the large mesh from the binary data serialized by pfxSerializeLargeTriMesh().
/// The size of buffer to store restored data is obtained by pfxQueryBytesToRestoreLargeTriMesh().
/// @param largeMesh Large mesh
/// @param[out] outBuff Used to store mesh data
/// @param[out] outBytes Size of outBuff
/// @param[in] meshBuff Serialized binary data  (16 bytes align)
/// @param[in] meshBytes Size of binary data
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxRestoreLargeTriMesh(PfxLargeTriMesh &largeMesh,PfxUInt8 *outBuff,PfxUInt32 outBytes, const PfxUInt8 *meshBuff,PfxUInt32 meshBytes);

SCE_PFX_API PfxInt32 pfxQueryBytesToRestoreConvexMesh(const PfxUInt8 *meshBuff,PfxUInt32 meshBytes);

SCE_PFX_API PfxUInt32 pfxQuerySerializedBytesOfConvexMesh(const PfxConvexMesh &convexMesh);

SCE_PFX_API PfxInt32 pfxQueryBytesToRestoreLargeTriMesh(const PfxUInt8 *meshBuff,PfxUInt32 meshBytes);

SCE_PFX_API PfxUInt32 pfxQuerySerializedBytesOfLargeTriMesh(const PfxLargeTriMesh &largeMesh);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_MESH_SERIALIZER_H
