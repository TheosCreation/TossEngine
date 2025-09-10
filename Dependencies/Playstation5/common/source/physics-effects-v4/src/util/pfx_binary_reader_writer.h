/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_BINARY_READER_WRITER_H
#define _SCE_PFX_BINARY_READER_WRITER_H

#include "../../include/physics_effects/base_level/base/pfx_common.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Binary Reader

void readInt8(const PfxUInt8 **buff,PfxInt8 &value);

void readInt8Array(const PfxUInt8 **buff,PfxInt8 *value,PfxUInt32 num);

void readUInt8(const PfxUInt8 **buff,PfxUInt8 &value);

void readUInt8Array(const PfxUInt8 **buff,PfxUInt8 *value,PfxUInt32 num);

void readInt16(const PfxUInt8 **buff,PfxInt16 &value);

void readInt16Array(const PfxUInt8 **buff,PfxInt16 *value,PfxUInt32 num);

void readUInt16(const PfxUInt8 **buff,PfxUInt16 &value);

void readUInt16Array(const PfxUInt8 **buff,PfxUInt16 *value,PfxUInt32 num);

void readInt32(const PfxUInt8 **buff,PfxInt32 &value);

void readInt32Array(const PfxUInt8 **buff,PfxInt32 *value,PfxUInt32 num);

void readUInt32(const PfxUInt8 **buff,PfxUInt32 &value);

void readUInt32Array(const PfxUInt8 **buff,PfxUInt32 *value,PfxUInt32 num);

void readInt64(const PfxUInt8 **buff,PfxInt64 &value);

void readInt64Array(const PfxUInt8 **buff,PfxInt64 *value,PfxUInt32 num);

void readUInt64(const PfxUInt8 **buff,PfxUInt64 &value);

void readUInt64Array(const PfxUInt8 **buff,PfxUInt64 *value,PfxUInt32 num);

void readFloat32(const PfxUInt8 **buff,PfxFloat &value);

void readFloat32Array(const PfxUInt8 **buff,PfxFloat *value,PfxUInt32 num);

///////////////////////////////////////////////////////////////////////////////
// Binary Writer

void writeInt8(PfxUInt8 **buff,PfxInt8 value);

void writeInt8Array(PfxUInt8 **buff,const PfxInt8 *value,PfxUInt32 num);

void writeUInt8(PfxUInt8 **buff,PfxUInt8 value);

void writeUInt8Array(PfxUInt8 **buff,const PfxUInt8 *value,PfxUInt32 num);

void writeInt16(PfxUInt8 **buff,PfxInt16 value);

void writeInt16Array(PfxUInt8 **buff,const PfxInt16 *value,PfxUInt32 num);

void writeUInt16(PfxUInt8 **buff,PfxUInt16 value);

void writeUInt16Array(PfxUInt8 **buff,const PfxUInt16 *value,PfxUInt32 num);

void writeInt32(PfxUInt8 **buff,PfxInt32 value);

void writeInt32Array(PfxUInt8 **buff,const PfxInt32 *value,PfxUInt32 num);

void writeUInt32(PfxUInt8 **buff,PfxUInt32 value);

void writeUInt32Array(PfxUInt8 **buff,const PfxUInt32 *value,PfxUInt32 num);

void writeInt64(PfxUInt8 **buff,PfxInt64 value);

void writeInt64Array(PfxUInt8 **buff,const PfxInt64 *value,PfxUInt32 num);

void writeUInt64(PfxUInt8 **buff,PfxUInt64 value);

void writeUInt64Array(PfxUInt8 **buff,const PfxUInt64 *value,PfxUInt32 num);

void writeFloat32(PfxUInt8 **buff,PfxFloat value);

void writeFloat32Array(PfxUInt8 **buff,const PfxFloat *value,PfxUInt32 num);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_BINARY_READER_WRITER_H
