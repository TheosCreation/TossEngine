/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#include "psfx_types.h"

static const uint k_classify_tile_size = 16;
static const uint k_classify_group_size = k_classify_tile_size / 2;

enum TileType
{
   k_tile_type_ssr,
   k_tile_type_clear,

   k_tile_type_count
};

struct ClassifySrtBuffers
{
   RW_RegularBuffer<uint> tile_buffer;
   RW_RegularBuffer<uint> indirect_buffer;
   RW_RegularBuffer<uint> need_ssr;
};

struct ClassifySrt
{
   Texture2D<float4> normal_roughness; // 8
   ClassifySrtBuffers* buffers[k_tile_type_count]; // 12
   float2 inv_target_size; // 14
   float roughness_cutoff; // 15
};

static const uint k_depth_group_size = 8;

struct DepthDownscaleSrt
{
   Texture2D<float> depth;
   RW_Texture2D<float> dest_depth;
   float2 inv_source_size;
   uint2 source_size;
   int source_level;
   float depth_scale;
};

static const uint k_trace_group_size = 8;

static_assert(
   (k_classify_tile_size % k_trace_group_size) == 0,
   "Tile size must be a multiple of trace shader group size");

static const unsigned int k_trace_groups_per_tile_dim = k_classify_tile_size / k_trace_group_size;
static const unsigned int k_trace_groups_per_tile = k_trace_groups_per_tile_dim * k_trace_groups_per_tile_dim;

enum TraceFlags : uint
{
   k_trace_clip_space_dx = 1 << 0,
};

struct TraceSrtIndirect
{
   Texture2D<float> depth;
   Texture2D<float3> colour;
   Texture2D<float4> normal_roughness;
   RW_Texture2D<float3> dest_reflection;
   RW_Texture2D<float> dest_mask;
   RW_Texture2D<float> dest_mip_level;
   float4x4 view_matrix;
   float4x4 projection_matrix;
   float4x4 inv_projection_matrix;
   float2 inv_target_size;
   float2 target_size;
   uint max_iterations;
   float edge_fade_threshold;
   float object_thickness;
   float reflection_mip_count;
   float reflection_mip_scale;
   float contact_max_distance;
   float contact_max_mip_bias;
};

struct TraceSrtInline
{
   // This is used from the main trace loop and it is therefore beneficial to force it to be kept
   // in SGPRs which avoids a load in the loop.
   Texture2D<float> depth_chain; // 8

   RegularBuffer<uint> tile_buffer; // 12
   TraceSrtIndirect* indirect; // 14
   float roughness_cutoff; // 15
   uint flags; // 16
};

struct ClearSrt
{
   RegularBuffer<uint> tile_buffer;
   RW_Texture2D<float3> dest_reflection;
   RW_Texture2D<float> dest_mask;
};

static const uint k_blur_group_size = 8;

#ifndef __PSSL__
using half3 = float3;
using half = float;
#endif

struct BlurSrt
{
   Texture2D<half3> reflection;
   Texture2D<half> mask;
   RW_Texture2D<half3> dest_reflection;
   RW_Texture2D<half> dest_mask;
   float2 inv_dest_size;
   float source_level;
};

struct OutputSrt
{
   RegularBuffer<uint> tile_buffer;
   Texture2D<float3> reflection;
   Texture2D<float> mask;
   Texture2D<float> mip_level;
   RW_Texture2D<float3> dest_reflection;
   RW_Texture2D<float> dest_mask;
   float2 inv_target_size;
   uint mip_count;
};

static const uint k_reproject_group_size = 8;

struct ReprojectSrt
{
   Texture2D<float3> reflection;
   Texture2D<float> mask;
   Texture2D<float2> velocity;
   RW_Texture2D<float3> dest_reflection;
   RW_Texture2D<float> dest_mask;
   float2 inv_dest_size;
};
