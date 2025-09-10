/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../include/physics_effects/util/pfx_world_serializer.h"
#include "pfx_world_binary_serializer.h"

namespace sce {
namespace pfxv4 {

PfxInt32 pfxQueryHeaderFromSerializedWorld(PfxSerializedWorldHeader &worldHeader, const PfxUInt8 *buffer, PfxUInt32 bytes)
{
	PfxWorldBinarySerializer worldSerializer;
	return worldSerializer.queryHeaderFromSerializedWorld(worldHeader, buffer, bytes);
}

PfxInt32 pfxSerializeWorldRead(PfxReadWorldParam &paramForRead, const PfxUInt8 *buffer, PfxUInt32 bytes, PfxUInt8 *workBuffer, PfxUInt32 workBytes, PfxMeshCreationCallbacks &callbacks)
{
	PfxWorldBinarySerializer worldSerializer;
	worldSerializer.setParamForRead(paramForRead);
	PfxInt32 ret = worldSerializer.readWorldBuffer(buffer, bytes, workBuffer, workBytes, callbacks);
	if (ret == SCE_PFX_OK) {
		worldSerializer.getParamForRead(paramForRead);
	}
	return ret;
}

PfxUInt32 pfxQueryWorkBytesToWriteWorldBuffer(const PfxWriteWorldParam &paramForWrite)
{
	PfxWorldBinarySerializer worldSerializer;
	worldSerializer.setBufferForSave(paramForWrite);
	return worldSerializer.queryWorkBytesToSaveWorldBuffer();
}

PfxUInt32 pfxQuerySerializeBytesOfWorldBuffer(const PfxWriteWorldParam &paramForWrite, PfxUInt8 *workBuffer, PfxUInt32 workBytes)
{
	PfxWorldBinarySerializer worldSerializer;
	worldSerializer.setBufferForSave(paramForWrite);
	return worldSerializer.querySerializeBytesOfWorldBuffer(workBuffer, workBytes);
}

PfxInt32 pfxSerializeWorldWrite(const PfxWriteWorldParam &paramForWrite, PfxUInt8 *buffer, PfxUInt32 bytes, PfxUInt8 *workBuffer, PfxUInt32 workBytes)
{
	PfxWorldBinarySerializer worldSerializer;
	worldSerializer.setBufferForSave(paramForWrite);
	return worldSerializer.writeWorldBuffer(buffer, bytes, workBuffer, workBytes);
}

} //namespace pfxv4
} //namespace sce
