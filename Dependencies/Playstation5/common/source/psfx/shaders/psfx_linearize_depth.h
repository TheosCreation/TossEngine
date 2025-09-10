/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2020 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#ifndef __PSSL__
#include <shader/srt_types.h>

using namespace sce::Shader::Srt;
using namespace sce::Shader::Pssl;
#endif

// Each lane processes a 2x2 block.
static const unsigned int k_linearize_depth_group_size = 8;
static const unsigned int k_linearize_depth_block_size = k_linearize_depth_group_size * 2;

struct LinearizeDepthSrt
{
   Texture2D<float> depth;
   RW_Texture2D<float> linear_depth;
   float2 plane_eq;
   float2 inv_source_size;
   float near;
   float inv_range;
};
