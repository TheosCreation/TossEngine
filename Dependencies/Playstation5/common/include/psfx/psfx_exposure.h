/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#include <agc.h>

// Histogram based average luminance computation


namespace psfx
{

struct MemoryPool;
struct MemoryRequirement;
struct ExposureContext;

struct ExposureConfiguration
{
};

enum class ExposureType
{
   e_Manual,
   e_StandardOutput,
   e_SaturationBased,
   e_SaturationBasedHDR
};
static const uint32_t k_exposure_stat_size = 6;
struct ExposureRuntimeOption
{
   // Source texture size.
   uint32_t source_width;
   uint32_t source_height;

   sce::Agc::Core::Texture source;
   sce::Agc::Core::Buffer destination;
   float pre_exposure = 1.f; // Should this be a pointer?
   float ev_min = -4.f;
   float ev_max = 15.f;
   float adaptation_speed_up   = 10.f;
   float adaptation_speed_down = 10.f;
   float white_point = 100.f;
   float gray_point = 50.f;

   float low_threshold = 0.5;
   float high_threshold = 0.95;
};

// Initialize the configuration structure
void exposure_configuration_init(ExposureConfiguration& conf);

// Fill the memory requirement for the effect
// Return -1 in case of error, SCE_OK otherwise
int exposure_compute_memory_requirement(ExposureConfiguration const& conf, MemoryRequirement& memory_pool);

ExposureContext* exposure_initialize(ExposureConfiguration const& conf, MemoryPool const& memory_pool);
void exposure_terminate(ExposureContext* ctx);

void exposure_apply(ExposureContext* ctx, sce::Agc::DrawCommandBuffer* dcb, ExposureRuntimeOption const& options);
void exposure_apply(ExposureContext* ctx, sce::Agc::AsyncCommandBuffer* dcb, ExposureRuntimeOption const& options);

void exposure_draw_histogram(ExposureContext* ctx, sce::Agc::DrawCommandBuffer* dcb, sce::Agc::Core::Texture const& dest, sce::Agc::Core::Buffer const& exposure_buffer);
void exposure_draw_histogram(ExposureContext* ctx, sce::Agc::AsyncCommandBuffer* dcb, sce::Agc::Core::Texture const& dest, sce::Agc::Core::Buffer const& exposure_buffer);

}
