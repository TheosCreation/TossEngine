/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2020 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#ifndef __PSSL__

#include <shader/srt_types.h>
#include <vectormath.h>

using namespace sce::Shader::Srt;
using namespace sce::Shader::Pssl;

struct float4x4
{
   float4 row[4];
};

inline void to_float4x4(sce::Vectormath::Scalar::Aos::Matrix4 const& m, float4x4& out_m)
{
   for (int i = 0; i < 4; ++i)
   {
      out_m.row[i].x = m.getRow(0)[i];
      out_m.row[i].y = m.getRow(1)[i];
      out_m.row[i].z = m.getRow(2)[i];
      out_m.row[i].w = m.getRow(3)[i];
   }
}

struct uint2
{
   uint x, y;
};

#endif
