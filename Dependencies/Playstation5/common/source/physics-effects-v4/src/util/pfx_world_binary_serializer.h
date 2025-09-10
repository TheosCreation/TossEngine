/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_WORLD_BINARY_SERIALIZER_H
#define _SCE_PFX_WORLD_BINARY_SERIALIZER_H

#include "../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../include/physics_effects/base_level/pfx_base_level_include.h"
#include "../../include/physics_effects/util/pfx_world_serializer.h"
#include "pfx_pool_array.h"

///////////////////////////////////////////////////////////////////////////////
// PfxWorldBinarySerializer

namespace sce {
namespace pfxv4 {

class PfxWorldBinarySerializer
{
private:
	// serialize	
	static const PfxUInt32 HEADER_ID = 'P' | 'E' << 8 | 'W' << 16 | 'D' << 24;
	static const PfxUInt8 VERSION_MAJOR = 0;
	static const PfxUInt8 VERSION_MINOR = 2;
	static const PfxUInt32 HEADER_SIZE = 64;

	struct PointerFix {
		PfxUInt32 id = 0;
		PfxUInt32 bytes = 0;
	};

	struct CollidableSerializeInfo {
	    PfxPoolMap<uintptr_t, PointerFix> *pointerTable = nullptr;
		PfxUInt32 shapeCount = 0;
		PfxUInt32 coreSphereCount = 0;
		PfxUInt32 convexMeshCount = 0;
		PfxUInt32 largeMeshCount = 0;
		PfxUInt32 convexMeshBytes = 0;
		PfxUInt32 largeMeshBytes = 0;
	};

	PfxUInt32 getPossiblePointerCount();
	void aggregateCollidableInfo(CollidableSerializeInfo &cinfo);

	PfxUInt32 querySerializeBytesOfRigidStates();
	PfxUInt32 querySerializeBytesOfRigidBodies();
	PfxUInt32 querySerializeBytesOfCollidables(PfxUInt8 *workBuffer, PfxUInt32 workBytes);
	PfxUInt32 querySerializeBytesOfJoints();

	PfxInt32 loadRigidStates(const PfxUInt8 *buffer, PfxUInt32 bytes);
	PfxInt32 loadRigidBodies(const PfxUInt8 *buffer, PfxUInt32 bytes);
	PfxInt32 loadCollidables(const PfxUInt8 *buffer, PfxUInt32 bytes, PfxUInt8 *workBuffer, PfxUInt32 workBytes, PfxMeshCreationCallbacks &callbacks);
	PfxInt32 loadJoints(const PfxUInt8 *buffer, PfxUInt32 bytes);

	PfxInt32 saveRigidStates(PfxUInt8 *buffer, PfxUInt32 bytes);
	PfxInt32 saveRigidBodies(PfxUInt8 *buffer, PfxUInt32 bytes);
	PfxInt32 saveCollidables(PfxUInt8 *buffer, PfxUInt32 bytes, PfxUInt8 *workBuffer, PfxUInt32 workBytes);
	PfxInt32 saveJoints(PfxUInt8 *buffer, PfxUInt32 bytes);

public:
	PfxReadWorldParam m_paramForRead;
	PfxWriteWorldParam m_paramForWrite;

	// load
	void setParamForRead(const PfxReadWorldParam &paramForRead) {m_paramForRead = paramForRead;}
	void getParamForRead(PfxReadWorldParam &paramForRead) { paramForRead = m_paramForRead; }
	PfxInt32 queryHeaderFromSerializedWorld(PfxSerializedWorldHeader &worldHeader, const PfxUInt8 *buffer, PfxUInt32 bytes);
	PfxInt32 readWorldBuffer(const PfxUInt8 *buffer, PfxUInt32 bytes, PfxUInt8 *workBuffer, PfxUInt32 workBytes, PfxMeshCreationCallbacks &callbacks);

	// save
	void setBufferForSave(const PfxWriteWorldParam &paramForWrite) {m_paramForWrite = paramForWrite;}
	PfxUInt32 queryWorkBytesToSaveWorldBuffer();
	PfxUInt32 querySerializeBytesOfWorldBuffer(PfxUInt8 *workBuffer, PfxUInt32 workBytes);
	PfxInt32 writeWorldBuffer(PfxUInt8 *buffer, PfxUInt32 bytes, PfxUInt8 *workBuffer, PfxUInt32 workBytes);
};

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_WORLD_BINARY_SERIALIZER_H
