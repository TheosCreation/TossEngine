/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#ifndef __PSSL__
#include <shader/srt_types.h>

using namespace sce::Shader::Srt;
using namespace sce::Shader::Pssl;

using int2 = int[2];
#endif

static const unsigned int k_motion_blur_tile_size = 40;
static const unsigned int k_tile_range_group_size = 8;
static const unsigned int k_neighbour_range_group_size = 8;
static const unsigned int k_blur_group_size = 8;

// We need an exact number of thread groups per tile for the indirect dispatch logic to work.
static_assert(
   (k_motion_blur_tile_size % k_blur_group_size) == 0,
   "Tile size must be a multiple of blur shader group size");

static const unsigned int k_blur_groups_per_tile_dim = k_motion_blur_tile_size / k_blur_group_size;
static const unsigned int k_blur_groups_per_tile = k_blur_groups_per_tile_dim * k_blur_groups_per_tile_dim;

// Tile classification. We have a fast path for where velocity is similar across a whole tile.
enum TileType
{
   k_tile_type_regular,
   k_tile_type_fast,

   k_tile_type_count
};

struct TileRangeHSrt
{
   Texture2D<float2> velocity;
   RW_Texture2D<float4> tile_range_h;
   float2 inv_source_size;
};

struct TileRangeVSrt
{
   Texture2D<float4> tile_range_h;
   RW_Texture2D<float4> tile_range;
};

struct NeighbourRangeSrt
{
   Texture2D<float4> tile_range;
   RW_Texture2D<float2> neighbour_max;
   RW_RegularBuffer<unsigned int> tile_buffers[k_tile_type_count];
   RW_RegularBuffer<unsigned int> indirect_buffers[k_tile_type_count];
   int2 tex_max;
   float2 velocity_scale;
};

struct BlurSrt
{
   Texture2D<float3> colour;
   Texture2D<float2> velocity;
   Texture2D<float> linear_depth;
   Texture2D<float2> neighbour_max;
   RW_Texture2D<float3> output;

   RegularBuffer<unsigned int> tile_buffer;

   int2 source_max;
   float2 velocity_scale;
   float max_blur;
   float z_near;
   float z_range;
};

struct CopySrt
{
   Texture2D<float3> source;
   RW_Texture2D<float4> dest;

   RegularBuffer<unsigned int> tile_buffer;
};
