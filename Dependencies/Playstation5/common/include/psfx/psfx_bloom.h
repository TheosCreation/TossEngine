/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#include <agc.h>

namespace psfx
{

struct MemoryPool;
struct MemoryRequirement;
struct BloomContext;

struct BloomConfiguration
{
   uint32_t max_target_width;
   uint32_t max_target_height;
   bool dynamic_resolution;
};

struct BloomRuntimeOption
{
   uint16_t target_width;
   uint16_t target_height;
   sce::Agc::Core::Texture source;
   sce::Agc::Core::Texture destination;
   float intensity = 1.f;
   float threshold = 0.f;
   bool anti_fireflies = true;
   float const* exposure = nullptr;
};

void bloom_configuration_init(BloomConfiguration& conf);
int bloom_compute_memory_requirement(BloomConfiguration const& conf, MemoryRequirement& memory_pool);

BloomContext* bloom_initialize(BloomConfiguration const& conf, MemoryPool const& memory_pool);
void bloom_terminate(BloomContext* ctx);
void bloom_apply(BloomContext* ctx, sce::Agc::DrawCommandBuffer* dcb, BloomRuntimeOption const& options);
void bloom_apply(BloomContext* ctx, sce::Agc::AsyncCommandBuffer* dcb, BloomRuntimeOption const& options);

}
