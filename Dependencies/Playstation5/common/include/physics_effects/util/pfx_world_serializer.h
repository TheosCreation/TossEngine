/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_WORLD_SERIALIZER_H
#define _SCE_PFX_WORLD_SERIALIZER_H

#include "../base_level/pfx_base_level_include.h"

namespace sce {
namespace pfxv4 {

/// @name World serializer

/// World binary serialize API

typedef void* (*PfxRestoreConvexMeshFunc)(const PfxUInt8* meshBuff, PfxUInt32 meshBytes, void *userData);
typedef void* (*PfxRestoreLargeTriMeshFunc)(const PfxUInt8* meshBuff, PfxUInt32 meshBytes, void *userData);

/// @brief Parameters used for specify callbacks to restore meshes
/// @details pfxSerializeWorldRead() calls proper callbacks when it finds meshes in a serialied data.
/// pfxRestoreConvexMesh() or pfxRestoreLargeTriMesh() can be used to restore meshes.
struct SCE_PFX_API PfxMeshCreationCallbacks {
	PfxRestoreConvexMeshFunc restoreConvexMesh;		///< @brief Callback to restore a convex mesh
	PfxRestoreLargeTriMeshFunc restoreLargeTriMesh;	///< @brief Callback to restore a large tri mesh
	void *userDataForRestoreConvexMesh = nullptr;	///< @brief User data transferred to restoreConvexMesh
	void *userDataForRestoreLargeTriMesh = nullptr;	///< @brief User data transferred to restoreLargeTriMesh
};

/// @brief Header information of a world serialized buffer
/// @details This structure can be used to query counters and the size of a work buffer in a serialized buffer.
struct SCE_PFX_API PfxSerializedWorldHeader {
	PfxUInt32 rigidBodyCount = 0;	///< @brief The number of rigid bodies
	PfxUInt32 jointCount = 0;		///< @brief The number of joints
	PfxUInt32 shapeCount = 0;		///< @brief The number of shapes
	PfxUInt32 coreSphereCount = 0;	///< @brief The number of core spheres
	PfxUInt32 convexMeshCount = 0;	///< @brief The number of convex meshes
	PfxUInt32 largeMeshCount = 0;	///< @brief The number of large tri meshes
	PfxUInt32 workBytes = 0;		///< @brief The size of a work buffer used to load a serialized buffer
};

/// @brief Parameters used to restore a world from a serialized buffer
/// @details Pointers to user arrays and the maximum size of each array should be specified before calling load functions.
/// Actual number of each array is set by pfxSerializeWorldWrite() after restoring a world is completed.
struct SCE_PFX_API PfxReadWorldParam {
	PfxRigidState *states = nullptr;		///< @brief Pointer to the array of rigid body states
	PfxRigidBody  *bodies = nullptr;		///< @brief Pointer to the array of rigid bodies
	PfxCollidable *collidables = nullptr;	///< @brief Pointer to the array of collidables
	PfxShape *shapes = nullptr;				///< @brief Pointer to the array of shapes
	PfxCoreSphere *coreSpheres = nullptr;	///< @brief Pointer to the array of core spheres
	PfxJoint *joints = nullptr;				///< @brief Pointer to the array of joints
	PfxConstraintPair *jointPairs = nullptr;///< @brief Pointer to the array of joint pairs
	PfxUInt32 maxJoints = 0;				///< @brief Maximum number of joints
	PfxUInt32 maxRigidBodies = 0;			///< @brief Maximum number of rigid bodies
	PfxUInt32 maxShapes = 0;				///< @brief Maximum number of shapes
	PfxUInt32 maxCoreSpheres = 0;			///< @brief Maximum number of core spheres
	PfxUInt32 numRigidBodies = 0;			///< @brief Number of rigid bodies
	PfxUInt32 numShapes = 0;				///< @brief Number of shapes
	PfxUInt32 numCoreSpheres = 0;			///< @brief Number of core spheres
	PfxUInt32 numJoints = 0;				///< @brief Number of joints
};

/// @brief Parameters used to serialize a world to a buffer
/// @details Pointers to user arrays and the number of each array should be specified before calling save functions.
struct SCE_PFX_API PfxWriteWorldParam {
	PfxRigidState *states = nullptr;		///< @brief Pointer to the array of rigid body states
	PfxRigidBody  *bodies = nullptr;		///< @brief Pointer to the array of rigid bodies
	PfxCollidable *collidables = nullptr;	///< @brief Pointer to the array of collidables
	PfxJoint *joints = nullptr;				///< @brief Pointer to the array of joints
	PfxConstraintPair *jointPairs = nullptr;///< @brief Pointer to the array of joint pairs
	PfxUInt32 numRigidBodies = 0;			///< @brief Number of rigid bodies
	PfxUInt32 numShapes = 0;				///< @brief Number of shapes
	PfxUInt32 numCoreSpheres = 0;			///< @brief Number of core spheres
	PfxUInt32 numJoints = 0;				///< @brief Number of joints
};

/// @brief Query header information from a serialized buffer
/// @details You can get counters of arrays stored in a buffer to allocate enough memory to store these arrays.
/// @param[out] worldHeader Header information of a world serialized buffer
/// @param buffer Serialized buffer
/// @param bytes Size of a serialized buffer
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxQueryHeaderFromSerializedWorld(PfxSerializedWorldHeader &worldHeader, const PfxUInt8 *buffer, PfxUInt32 bytes);

/// @brief Restore a world from a serialized buffer
/// @details Each parameter in paramForRead should be set before loading a serialized buffer.
/// @param[in,out] paramForRead Parameters used to restore a world
/// @param[in] buffer Serialized buffer
/// @param bytes Size of a serialized buffer
/// @param workBuffer Work buffer
/// @param workBytes Size of a work buffer
/// @param callbacks Callbacks called to restore meshes
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxSerializeWorldRead(PfxReadWorldParam &paramForRead, const PfxUInt8 *buffer, PfxUInt32 bytes, PfxUInt8 *workBuffer, PfxUInt32 workBytes, PfxMeshCreationCallbacks &callbacks);

/// @brief Query the size of work buffer needed to serialize
/// @details Work buffer is used to store temporary data when serializing a world.
/// @param[in] paramForWrite Parameters used to seralize a world
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxUInt32 pfxQueryWorkBytesToWriteWorldBuffer(const PfxWriteWorldParam &paramForWrite);

/// @brief Query the size of buffer needed to store a serialized world
/// @details Buffer is used to store a serialized world.
/// @param[in] paramForWrite Parameters used to seralize a world
/// @param workBuffer Work buffer
/// @param workBytes Size of a work buffer
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxUInt32 pfxQuerySerializeBytesOfWorldBuffer(const PfxWriteWorldParam &paramForWrite, PfxUInt8 *workBuffer, PfxUInt32 workBytes);

/// @brief Serialize a world to a buffer
/// @details Gather arrays and set their pointers to paramForWrite and call this function to serialize a world.
/// Proper buffers need to be prepared before calling this function.
/// @param[in] paramForWrite Parameters used to seralize a world
/// @param[out] buffer Buffer used to serialize a world
/// @param bytes Size of a buffer
/// @param workBuffer Work buffer
/// @param workBytes Size of a work buffer
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxSerializeWorldWrite(const PfxWriteWorldParam &paramForWrite, PfxUInt8 *buffer, PfxUInt32 bytes, PfxUInt8 *workBuffer, PfxUInt32 workBytes);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_MESH_SERIALIZER_H
