/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#include <agc.h>
#include <vectormath.h>

namespace psfx
{

struct MemoryPool;
struct MemoryRequirement;

struct SsrConfiguration
{
   // Maximum dimensions of the source texture region. Actual dimensions are specified each frame
   // and can be dynamically adjusted.
   uint32_t max_target_width;
   uint32_t max_target_height;

   // Maximum mip count for rough reflections (see SsrRuntimeOption::mip_count). This controls
   // memory allocation size, but the actual mip level used can be dynamically adjusted each
   // frame. If 0, the maximum will be set automatically.
   uint32_t max_mip_count;
};

struct SsrRuntimeOption
{
   // Source/destination texture size.
   uint32_t target_width;
   uint32_t target_height;

   // Scene colour texture.
   sce::Agc::Core::Texture source_colour;

   // Scene depth texture.
   sce::Agc::Core::Texture source_depth;

   // Scene world-space normals (RGB, scaled/biased to [0,1] range) and roughness (A) texture.
   // Roughness can also be used as a mask for reflective surfaces that should not have SSR
   // applied to them, by setting roughness to 1 in this texture even though the real roughness
   // values are less than 1.
   // TODO: Investigate whether we can give more flexibility to the user in how these are supplied.
   sce::Agc::Core::Texture source_normal_roughness;

   // Destination reflection texture (3 channels, e.g. 11_11_10Float). At each pixel this will
   // contain the incoming radiance to that pixel in the scene due to reflections.
   //
   // It is recommended to enable DCC (read/write compatibility) for this target, as it will allow
   // optimization of clearing non-reflective areas and also likely improve performance of writes
   // to reflective areas.
   sce::Agc::Core::Texture dest_reflection;

   // Destination mask texture (1 channel, e.g. 8UNorm). The value in at each pixel will indicate
   // whether a valid SSR result was determined for that pixel (1 = valid, 0 = invalid). Values
   // between 0 and 1 are used to blend with a fallback solution such as a cubemap, e.g.
   // lerp(cube_result, ssr_result, mask).
   //
   // It is recommended to enable DCC (read/write compatibility) for this target, as it will allow
   // optimization of clearing non-reflective areas.
   sce::Agc::Core::Texture dest_mask;

   // Maximum number of ray tracing iterations. This affects the maximum distance for reflections,
   // but also performance (higher maximum is potentially slower). Note that maximum distance and
   // trace time do not increase linearly with this value due to the hierarchical tracing
   // algorithm used.
   uint max_iterations;

   // Roughness cutoff. Reflections won't be calculated for pixels with roughness greater than or
   // equal to this.
   float roughness_cutoff;

   // Edge fade threshold. SSR results fade out as the ray hit position gets close to the screen
   // edge, the distance over which this occurs is specified by this value (in the range 0-1).
   float edge_fade_threshold;

   // Object thickness value. Hits further than this behind the (linear) depth buffer value will
   // be rejected.
   float object_thickness;

   // Mip level count/scale for rough reflections. Rough reflections are implemented by generating
   // a mip chain from the results of the screen-space ray tracing, where each mip level is
   // downscaled and Gaussian blurred from the previous level, and then sampling (trilinear) from
   // a mip level based on roughness when generating the final output reflections. Mip level is
   // calculated from roughness as: clamp(mip_count * roughness * mip_scale, 0, mip_count).
   //
   // When using dynamic resolution scaling, these parameters can be tuned so that reflections look
   // similar at different resolutions (see tutorial code for example).
   uint32_t mip_count;
   float mip_scale;

   // Parameters for contact hardening on rough reflections. This is a faked effect (not physically
   // accurate) used to improve appearance of reflections on contact by avoiding leaking from other
   // surfaces or miss results). See psfx_ssr_trace_c.pssl for details.
   float contact_max_distance; // Maximum distance the effect applies over, 0.0f disables it.
   float contact_max_mip_bias; // Maximum mip level bias.

   // View/projection parameters.
   sce::Vectormath::Scalar::Aos::Matrix4 view_matrix;
   sce::Vectormath::Scalar::Aos::Matrix4 projection_matrix;
   sce::Agc::CxClipControl::ClipSpace clip_space = sce::Agc::CxClipControl::ClipSpace::kOGL;
   bool reverse_z = false;
};

struct SsrReprojectRuntimeOption
{
   // Source texture size.
   uint32_t source_width;
   uint32_t source_height;

   // Destination texture size. May be different from the source to support dynamic resolution
   // switching across frames.
   uint32_t dest_width;
   uint32_t dest_height;

   // Generated SSR reflection/mask textures from the previous frame.
   sce::Agc::Core::Texture source_reflection;
   sce::Agc::Core::Texture source_mask;

   // Destination reprojected reflection/mask textures.
   sce::Agc::Core::Texture dest_reflection;
   sce::Agc::Core::Texture dest_mask;

   // Velocity buffer for the current frame.
   sce::Agc::Core::Texture velocity;
};

struct SsrContext;

void ssr_configuration_init(SsrConfiguration& conf);

int ssr_compute_memory_requirement(const SsrConfiguration& conf, MemoryRequirement& requirement);

SsrContext* ssr_initialize(const SsrConfiguration& conf, const MemoryPool& memory_pool);
void ssr_terminate(SsrContext* ctx);

void ssr_generate(SsrContext* ctx, sce::Agc::AsyncCommandBuffer* acb, const SsrRuntimeOption& options);
void ssr_generate(SsrContext* ctx, sce::Agc::DrawCommandBuffer* dcb, const SsrRuntimeOption& options);

void ssr_reproject(SsrContext* ctx, sce::Agc::AsyncCommandBuffer* acb, const SsrReprojectRuntimeOption& options);
void ssr_reproject(SsrContext* ctx, sce::Agc::DrawCommandBuffer* dcb, const SsrReprojectRuntimeOption& options);

}
