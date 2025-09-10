/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_binary_reader_writer.h"

namespace sce {
namespace pfxv4 {

union I8 {
	PfxInt8 signedValue;
	PfxUInt8 unsignedValue;
	PfxUInt8 elem;
};

union I16 {
	PfxInt16 signedValue;
	PfxUInt16 unsignedValue;
	PfxUInt8 elem[2];
};

union I32 {
	PfxInt32 signedValue;
	PfxUInt32 unsignedValue;
	PfxUInt8 elem[4];
};

union I64 {
	PfxInt64 signedValue;
	PfxUInt64 unsignedValue;
	PfxUInt8 elem[8];
};

union F32 {
	PfxFloat value;
	PfxUInt8 elem[4];
};

void readInt8(const PfxUInt8 **buff,PfxInt8 &value)
{
	I8 in;
	in.elem = **buff;
	(*buff)++;
	value = in.signedValue;
}

void readInt8Array(const PfxUInt8 **buff,PfxInt8 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		readInt8(buff,value[n]);
	}
}

void readUInt8(const PfxUInt8 **buff,PfxUInt8 &value)
{
	I8 in;
	in.elem = **buff;
	(*buff)++;
	value = in.unsignedValue;
}

void readUInt8Array(const PfxUInt8 **buff,PfxUInt8 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		readUInt8(buff,value[n]);
	}
}

void readInt16(const PfxUInt8 **buff,PfxInt16 &value)
{
	I16 in;
	in.elem[0] = **buff; (*buff)++;
	in.elem[1] = **buff; (*buff)++;
	value = in.signedValue;
}

void readInt16Array(const PfxUInt8 **buff,PfxInt16 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		readInt16(buff,value[n]);
	}
}

void readUInt16(const PfxUInt8 **buff,PfxUInt16 &value)
{
	I16 in;
	in.elem[0] = **buff; (*buff)++;
	in.elem[1] = **buff; (*buff)++;
	value = in.unsignedValue;
}

void readUInt16Array(const PfxUInt8 **buff,PfxUInt16 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		readUInt16(buff,value[n]);
	}
}

void readInt32(const PfxUInt8 **buff,PfxInt32 &value)
{
	I32 in;
	in.elem[0] = **buff; (*buff)++;
	in.elem[1] = **buff; (*buff)++;
	in.elem[2] = **buff; (*buff)++;
	in.elem[3] = **buff; (*buff)++;
	value = in.signedValue;
}

void readInt32Array(const PfxUInt8 **buff,PfxInt32 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		readInt32(buff,value[n]);
	}
}

void readUInt32(const PfxUInt8 **buff,PfxUInt32 &value)
{
	I32 in;
	in.elem[0] = **buff; (*buff)++;
	in.elem[1] = **buff; (*buff)++;
	in.elem[2] = **buff; (*buff)++;
	in.elem[3] = **buff; (*buff)++;
	value = in.unsignedValue;
}

void readUInt32Array(const PfxUInt8 **buff,PfxUInt32 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		readUInt32(buff,value[n]);
	}
}

void readInt64(const PfxUInt8 **buff,PfxInt64 &value)
{
	I64 in;
	in.elem[0] = **buff; (*buff)++;
	in.elem[1] = **buff; (*buff)++;
	in.elem[2] = **buff; (*buff)++;
	in.elem[3] = **buff; (*buff)++;
	in.elem[4] = **buff; (*buff)++;
	in.elem[5] = **buff; (*buff)++;
	in.elem[6] = **buff; (*buff)++;
	in.elem[7] = **buff; (*buff)++;
	value = in.signedValue;
}

void readInt64Array(const PfxUInt8 **buff,PfxInt64 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		readInt64(buff,value[n]);
	}
}

void readUInt64(const PfxUInt8 **buff,PfxUInt64 &value)
{
	I64 in;
	in.elem[0] = **buff; (*buff)++;
	in.elem[1] = **buff; (*buff)++;
	in.elem[2] = **buff; (*buff)++;
	in.elem[3] = **buff; (*buff)++;
	in.elem[4] = **buff; (*buff)++;
	in.elem[5] = **buff; (*buff)++;
	in.elem[6] = **buff; (*buff)++;
	in.elem[7] = **buff; (*buff)++;
	value = in.unsignedValue;
}

void readUInt64Array(const PfxUInt8 **buff,PfxUInt64 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		readUInt64(buff,value[n]);
	}
}

void readFloat32(const PfxUInt8 **buff,PfxFloat &value)
{
	F32 in;
	in.elem[0] = **buff; (*buff)++;
	in.elem[1] = **buff; (*buff)++;
	in.elem[2] = **buff; (*buff)++;
	in.elem[3] = **buff; (*buff)++;
	value = in.value;
}

void readFloat32Array(const PfxUInt8 **buff,PfxFloat *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		readFloat32(buff,value[n]);
	}
}

void writeInt8(PfxUInt8 **buff,PfxInt8 value)
{
	I8 out;
	out.signedValue = value;
	**buff = out.elem; (*buff)++;
}

void writeInt8Array(PfxUInt8 **buff,const PfxInt8 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		writeInt8(buff,value[n]);
	}
}

void writeUInt8(PfxUInt8 **buff,PfxUInt8 value)
{
	I8 out;
	out.unsignedValue = value;
	**buff = out.elem; (*buff)++;
}

void writeUInt8Array(PfxUInt8 **buff,const PfxUInt8 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		writeUInt8(buff,value[n]);
	}
}

void writeInt16(PfxUInt8 **buff,PfxInt16 value)
{
	I16 out;
	out.signedValue = value;
	**buff = out.elem[0]; (*buff)++;
	**buff = out.elem[1]; (*buff)++;
}

void writeInt16Array(PfxUInt8 **buff,const PfxInt16 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		writeInt16(buff,value[n]);
	}
}

void writeUInt16(PfxUInt8 **buff,PfxUInt16 value)
{
	I16 out;
	out.unsignedValue = value;
	**buff = out.elem[0]; (*buff)++;
	**buff = out.elem[1]; (*buff)++;
}

void writeUInt16Array(PfxUInt8 **buff,const PfxUInt16 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		writeUInt16(buff,value[n]);
	}
}

void writeInt32(PfxUInt8 **buff,PfxInt32 value)
{
	I32 out;
	out.signedValue = value;
	**buff = out.elem[0]; (*buff)++;
	**buff = out.elem[1]; (*buff)++;
	**buff = out.elem[2]; (*buff)++;
	**buff = out.elem[3]; (*buff)++;
}

void writeInt32Array(PfxUInt8 **buff,const PfxInt32 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		writeInt32(buff,value[n]);
	}
}

void writeUInt32(PfxUInt8 **buff,PfxUInt32 value)
{
	I32 out;
	out.unsignedValue = value;
	**buff = out.elem[0]; (*buff)++;
	**buff = out.elem[1]; (*buff)++;
	**buff = out.elem[2]; (*buff)++;
	**buff = out.elem[3]; (*buff)++;
}

void writeUInt32Array(PfxUInt8 **buff,const PfxUInt32 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		writeUInt32(buff,value[n]);
	}
}

void writeInt64(PfxUInt8 **buff,PfxInt64 value)
{
	I64 out;
	out.signedValue = value;
	**buff = out.elem[0]; (*buff)++;
	**buff = out.elem[1]; (*buff)++;
	**buff = out.elem[2]; (*buff)++;
	**buff = out.elem[3]; (*buff)++;
	**buff = out.elem[4]; (*buff)++;
	**buff = out.elem[5]; (*buff)++;
	**buff = out.elem[6]; (*buff)++;
	**buff = out.elem[7]; (*buff)++;
}

void writeInt64Array(PfxUInt8 **buff,const PfxInt64 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		writeInt64(buff,value[n]);
	}
}

void writeUInt64(PfxUInt8 **buff,PfxUInt64 value)
{
	I64 out;
	out.unsignedValue = value;
	**buff = out.elem[0]; (*buff)++;
	**buff = out.elem[1]; (*buff)++;
	**buff = out.elem[2]; (*buff)++;
	**buff = out.elem[3]; (*buff)++;
	**buff = out.elem[4]; (*buff)++;
	**buff = out.elem[5]; (*buff)++;
	**buff = out.elem[6]; (*buff)++;
	**buff = out.elem[7]; (*buff)++;
}

void writeUInt64Array(PfxUInt8 **buff,const PfxUInt64 *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		writeUInt64(buff,value[n]);
	}
}

void writeFloat32(PfxUInt8 **buff,PfxFloat value)
{
	F32 out;
	out.value = value;
	**buff = out.elem[0]; (*buff)++;
	**buff = out.elem[1]; (*buff)++;
	**buff = out.elem[2]; (*buff)++;
	**buff = out.elem[3]; (*buff)++;
}

void writeFloat32Array(PfxUInt8 **buff,const PfxFloat *value,PfxUInt32 num)
{
	for(PfxUInt32 n=0;n<num;n++) {
		writeFloat32(buff,value[n]);
	}
}

} //namespace pfxv4
} //namespace sce
