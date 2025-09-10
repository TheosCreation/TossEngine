/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2020 Sony Interactive Entertainment Inc.
* 
*/

#ifndef SSAO_GENERATE_INTERLEAVED
#define SSAO_GENERATE_INTERLEAVED 0
#endif

#include "psfx_common.h"

// #define MIN_MAX_DOWNSAMPLE
// #define TEST

struct OutTextures
{
   RW_Texture2D<float> out_depth;
   RW_Texture2D_Array<float> out_interleaved_depth;
};

struct ShaderParam
{
   Texture2D<float> input;
   OutTextures* out_tx;
   float2 tx_inv_size;
   float2 near_Inv_range;
};

ShaderParam srt_data : S_SRT_DATA;

float4 out_depth(float4 lin_z)
{
   float4 z = (lin_z - srt_data.near_Inv_range.x) * srt_data.near_Inv_range.y;
   return sqrt(z);
}


float find_best_z(float4 z0, bool odd)
{
#ifdef MIN_MAX_DOWNSAMPLE
   if (odd)
      return min(min3(z0.x, z0.y, z0.z), z0.w);
   else
      return max(max3(z0.x, z0.y, z0.z), z0.w);
#else
   PSSL_UNUSED(odd);
   return min(min3(z0.x, z0.y, z0.z), z0.w);
#endif
}

static const SamplerState point_sampler = make_point_sampler();

static const uint k_wavefront_size = 8;
[CxxSymbol(SYMBOL_NAME)]
[NUM_THREADS(k_wavefront_size, k_wavefront_size, 1)]
void main(uint2 thread_id : S_DISPATCH_THREAD_ID)
{
   static const uint2 i_offsets[4] = { uint2(0, 1), uint2(1, 1), uint2(1, 0), uint2(0, 0) };
   const uint2 start_point = thread_id*2;

   float4 surrouning[4];
   [unroll]
   for(int i = 0; i < 4; ++i)
   {
      const int2 load_coords = (start_point + i_offsets[i])*2;
      const float2 gather_uv = (load_coords + float2(1.0)) * srt_data.tx_inv_size;
      surrouning[i] = srt_data.input.Gather(point_sampler, gather_uv);
   }
   float4 z_samples;
   z_samples.x = find_best_z(surrouning[0], true);
   z_samples.y = find_best_z(surrouning[1], false);
   z_samples.z = find_best_z(surrouning[2], true);
   z_samples.w = find_best_z(surrouning[3], false);

   const float4 store_value = z_samples;

   [unroll]
   for(int i = 0; i < 4; ++i)
   {
      const uint2 out_pixel = start_point + i_offsets[i];
      srt_data.out_tx->out_depth[out_pixel] = store_value[i];
#if SSAO_GENERATE_INTERLEAVED
      const uint2 interleaved_coord = BitFieldExtract(out_pixel, uint2(2), uint2(30));
      const uint2 layer = BitFieldExtract(out_pixel, uint2(0), uint2(2));
      const uint layer_idx = layer.y * 4 + layer.x;
      srt_data.out_tx->out_interleaved_depth[int3(interleaved_coord, layer_idx)] = store_value[i];
#endif
   }
}
