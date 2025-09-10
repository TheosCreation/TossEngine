/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#include "psfx_common.h"
#include "psfx_linearize_depth.h"

#include "psfx_details.h"

#include "../shaders/psfx_linearize_depth.h"

namespace psfx
{

struct LinearizeDepthContext
{
   LinearizeDepthConfiguration conf;

   sce::Agc::ResourceRegistration::OwnerHandle owner_handle;

   sce::Agc::Shader* cs;

};

extern char       linearize_depth_header[];
extern const char linearize_depth_text[];

void linearize_depth_configuration_init(LinearizeDepthConfiguration& conf)
{
   memset(&conf, 0, sizeof(conf));
}

int linearize_depth_compute_memory_requirement(const LinearizeDepthConfiguration& conf, MemoryRequirement& requirement)
{
   sce::Agc::SizeAlign& mem_size = requirement.mem_size;

   mem_size = { 0, 8 };
   mem_size.m_size += sizeof(LinearizeDepthContext);

   return 0;
}

LinearizeDepthContext* linearize_depth_initialize(const LinearizeDepthConfiguration& conf, const MemoryPool& memory_pool)
{
   if (!memory_pool.mem_ptr)
   {
      SCE_DBG_LOG_ERROR("mem_ptr can't be NULL.");
      return nullptr;
   }

   details::ArenaAllocator mem_arena(memory_pool.mem_ptr);

   LinearizeDepthContext* ctx = mem_arena.alloc<LinearizeDepthContext>(sce::Agc::Alignment::kBuffer);
   memset(ctx, 0, sizeof(LinearizeDepthContext));
   ctx->conf = conf;

   SceError err = sce::Agc::ResourceRegistration::registerOwner(&ctx->owner_handle, "psfx linearize depth");
   if (err != SCE_OK)
      ctx->owner_handle = sce::Agc::ResourceRegistration::kInvalidOwnerHandle;

   ctx->cs = details::make_cs(ctx->owner_handle, "psfx linearize depth", linearize_depth_header, linearize_depth_text);

   return ctx;
}

void linearize_depth_terminate(LinearizeDepthContext* ctx)
{
   if (ctx == nullptr)
   {
      SCE_DBG_LOG_ERROR("Trying to terminate a NULL context.");
      return;
   }

   if (ctx->owner_handle != sce::Agc::ResourceRegistration::kInvalidOwnerHandle)
      sce::Agc::ResourceRegistration::unregisterOwnerAndResources(ctx->owner_handle);
}

template<typename T>
void linearize_depth(LinearizeDepthContext* ctx, T* cb, const LinearizeDepthRuntimeOption& options)
{
   if (options.target_width * options.target_height == 0)
   {
      SCE_DBG_LOG_ERROR("The target dimensions are zero.");
      return;
   }

   details::MarkerAutoScope<T> marker(cb, "psfx depth linearize");

   LinearizeDepthSrt* srt = details::alloc_top_down<LinearizeDepthSrt>(cb);

   srt->depth             = options.source;
   srt->linear_depth      = options.destination;
   srt->inv_source_size.x = 1.0f / static_cast<float>(options.target_width);
   srt->inv_source_size.y = 1.0f / static_cast<float>(options.target_height);

   const float near_plane = options.near_plane_distance;
   const float far_plane  = options.far_plane_distance;
   const auto coeff       = details::compute_coefficent_for_linear_depth(near_plane, far_plane, options.clip_space, options.far_plane_at_infinity, options.reverse_z);

   srt->plane_eq.x = coeff.first;
   srt->plane_eq.y = coeff.second;
   srt->near       = near_plane;
   srt->inv_range  = 1.f / (far_plane - near_plane);

   details::set_compute_shader(cb, ctx->cs);
   details::set_srt(cb, ctx->cs, &srt);

   cb->dispatch(
      details::round_div(options.target_width, k_linearize_depth_block_size),
      details::round_div(options.target_height, k_linearize_depth_block_size),
      1,
      ctx->cs->m_specials->m_dispatchModifier);

   details::cs_sync(cb);
}

void linearize_depth(LinearizeDepthContext* ctx, sce::Agc::AsyncCommandBuffer* acb, const LinearizeDepthRuntimeOption& options)
{
   return linearize_depth<sce::Agc::AsyncCommandBuffer>(ctx, acb, options);
}

void linearize_depth(LinearizeDepthContext* ctx, sce::Agc::DrawCommandBuffer* dcb, const LinearizeDepthRuntimeOption& options)
{
   return linearize_depth<sce::Agc::DrawCommandBuffer>(ctx, dcb, options);
}

}