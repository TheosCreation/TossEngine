/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#include <agc.h>

namespace psfx
{

static const sce::Agc::Core::DataFormat k_linear_depth_format = { sce::Agc::Core::TypedFormat::k16UNorm , sce::Agc::Core::Swizzle::kR001_R1S1 };

struct MemoryPool;
struct MemoryRequirement;

struct LinearizeDepthConfiguration
{
};

struct LinearizeDepthRuntimeOption
{
   // Dimensions of the source/destination texture.
   uint32_t target_width;
   uint32_t target_height;

   // Depth texture to linearize.
   sce::Agc::Core::Texture source;

   // Destination linear depth texture
   sce::Agc::Core::Texture destination;

   // Projection parameters.
   bool far_plane_at_infinity = false;
   bool reverse_z = false;
   sce::Agc::CxClipControl::ClipSpace clip_space = sce::Agc::CxClipControl::ClipSpace::kOGL;
   float near_plane_distance;
   float far_plane_distance;
};

struct LinearizeDepthContext;

void linearize_depth_configuration_init(LinearizeDepthConfiguration& conf);

int linearize_depth_compute_memory_requirement(const LinearizeDepthConfiguration& conf, MemoryRequirement& requirement);

LinearizeDepthContext* linearize_depth_initialize(const LinearizeDepthConfiguration& conf, const MemoryPool& memory_pool);
void linearize_depth_terminate(LinearizeDepthContext* ctx);

void linearize_depth(LinearizeDepthContext* ctx, sce::Agc::AsyncCommandBuffer* acb, const LinearizeDepthRuntimeOption& options);
void linearize_depth(LinearizeDepthContext* ctx, sce::Agc::DrawCommandBuffer* dcb, const LinearizeDepthRuntimeOption& options);

}
