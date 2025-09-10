/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#include "psfx_common.h"
#include "psfx_motion_blur.h"

#include "../shaders/psfx_motion_blur.h"

#include "psfx_details.h"

namespace psfx
{

enum MotionBlurTarget
{
   k_target_tile_range_h,
   k_target_tile_range,
   k_target_neighbour_max,
   k_target_output,

   k_target_count
};

static const char* const k_tile_type_names[k_tile_type_count] =
{
   "regular",
   "fast",
};

struct MotionBlurContext
{
   MotionBlurConfiguration conf;

   sce::Agc::ResourceRegistration::OwnerHandle owner_handle;

   sce::Agc::Shader* cs_copy;
   sce::Agc::Shader* cs_tile_range_h;
   sce::Agc::Shader* cs_tile_range_v;
   sce::Agc::Shader* cs_neighbour_range;
   sce::Agc::Shader* cs_blur[k_tile_type_count];

   sce::Agc::Core::Texture targets[k_target_count];

   sce::Agc::Core::Buffer tile_buffers[k_tile_type_count];
   sce::Agc::Core::Buffer indirect_buffers[k_tile_type_count];
};

extern char       motion_blur_copy_header[];
extern const char motion_blur_copy_text[];
extern char       motion_blur_tile_range_h_header[];
extern const char motion_blur_tile_range_h_text[];
extern char       motion_blur_tile_range_v_header[];
extern const char motion_blur_tile_range_v_text[];
extern char       motion_blur_neighbour_range_header[];
extern const char motion_blur_neighbour_range_text[];
extern char       motion_blur_header[];
extern const char motion_blur_text[];
extern char       motion_blur_fast_header[];
extern const char motion_blur_fast_text[];

// Whether to enable DCC for the intermediate output.
static const bool k_output_dcc = true;

static const sce::Agc::Core::DataFormat k_output_format        = { sce::Agc::Core::TypedFormat::k11_11_10Float, sce::Agc::Core::Swizzle::kRGB1_R3S34 };
static const sce::Agc::Core::DataFormat k_tile_range_format    = { sce::Agc::Core::TypedFormat::k16_16_16_16SNorm, sce::Agc::Core::Swizzle::kRGBA_R4S4 };
static const sce::Agc::Core::DataFormat k_neighbour_max_format = { sce::Agc::Core::TypedFormat::k16_16SNorm, sce::Agc::Core::Swizzle::kRG01_R2S24 };

static void make_tx_descriptors(const MotionBlurConfiguration& conf, sce::Agc::Core::TextureSpec* tx_specs)
{
   const uint32_t tile_range_h_width  = details::round_div(conf.max_target_width, k_motion_blur_tile_size);
   const uint32_t tile_range_h_height = conf.max_target_height;
   details::make_tx_descriptors(tile_range_h_width, tile_range_h_height, 1, k_tile_range_format, &tx_specs[k_target_tile_range_h], 1);

   const uint32_t tile_range_width  = tile_range_h_width;
   const uint32_t tile_range_height = details::round_div(conf.max_target_height, k_motion_blur_tile_size);
   details::make_tx_descriptors(tile_range_width, tile_range_height, 1, k_tile_range_format, &tx_specs[k_target_tile_range], 1);
   details::make_tx_descriptors(tile_range_width, tile_range_height, 1, k_neighbour_max_format, &tx_specs[k_target_neighbour_max], 1);

   details::make_tx_descriptors(conf.max_target_width, conf.max_target_height, 1, k_output_format, &tx_specs[k_target_output], 1, k_output_dcc);
}

static void resize_targets(MotionBlurContext* ctx, uint32_t width, uint32_t height)
{
   const uint32_t tile_range_h_width  = details::round_div(width, k_motion_blur_tile_size);
   const uint32_t tile_range_h_height = height;
   details::resize_tx(tile_range_h_width, tile_range_h_height, 1, ctx->targets[k_target_tile_range_h]);

   const uint32_t tile_range_width  = tile_range_h_width;
   const uint32_t tile_range_height = details::round_div(height, k_motion_blur_tile_size);
   details::resize_tx(tile_range_width, tile_range_height, 1, ctx->targets[k_target_tile_range]);
   details::resize_tx(tile_range_width, tile_range_height, 1, ctx->targets[k_target_neighbour_max]);

   details::resize_tx(width, height, 1, ctx->targets[k_target_output]);

   const uint32_t tile_buffer_count = details::round_div(width, k_motion_blur_tile_size) * details::round_div(height, k_motion_blur_tile_size);

   for (uint32_t i = 0; i < k_tile_type_count; i++)
   {
      sce::Agc::Core::BufferSpec spec;
      spec.initAsRegularBuffer(ctx->tile_buffers[i].getDataAddress(), sizeof(uint32_t), tile_buffer_count);
      sce::Agc::Core::initialize(&ctx->tile_buffers[i], &spec);
   }
}

void motion_blur_configuration_init(MotionBlurConfiguration& conf)
{
   memset(&conf, 0, sizeof(conf));
}

int motion_blur_compute_memory_requirement(const MotionBlurConfiguration& conf, MemoryRequirement& requirement)
{
   if (conf.max_target_width * conf.max_target_height == 0)
   {
      SCE_DBG_LOG_ERROR("The target dimensions are zero.");
      return -1;
   }

   sce::Agc::SizeAlign& mem_size = requirement.mem_size;

   mem_size = { 0, 8 };
   mem_size.m_size += details::align_to(sizeof(MotionBlurContext), sce::Agc::Alignment::kBuffer);

   sce::Agc::Core::TextureSpec tx_specs[k_target_count];
   make_tx_descriptors(conf, tx_specs);
   details::tx_memory_usage(tx_specs, k_target_count, mem_size);

   const uint32_t tile_buffer_count = details::round_div(conf.max_target_width, k_motion_blur_tile_size) * details::round_div(conf.max_target_height, k_motion_blur_tile_size);
   const uint32_t tile_buffer_size  = tile_buffer_count * sizeof(uint32_t);

   for (uint32_t i = 0; i < k_tile_type_count; i++)
   {
      mem_size.m_size  = details::align_to(mem_size.m_size, sce::Agc::Alignment::kBuffer);
      mem_size.m_size += tile_buffer_size;

      mem_size.m_size  = details::align_to(mem_size.m_size, sce::Agc::Alignment::kIndirectArgs);
      mem_size.m_size += sizeof(sce::Agc::DispatchIndirectArgs);
   }

   return SCE_OK;
}

MotionBlurContext* motion_blur_initialize(const MotionBlurConfiguration& conf, const MemoryPool& memory_pool)
{
   if (!memory_pool.mem_ptr)
   {
      SCE_DBG_LOG_ERROR("mem_ptr can't be NULL.");
      return nullptr;
   }

   details::ArenaAllocator mem_arena(memory_pool.mem_ptr);

   MotionBlurContext* ctx = mem_arena.alloc<MotionBlurContext>(sce::Agc::Alignment::kBuffer);
   memset(ctx, 0, sizeof(MotionBlurContext));
   ctx->conf = conf;

   SceError err = sce::Agc::ResourceRegistration::registerOwner(&ctx->owner_handle, "psfx motion blur");
   if (err != SCE_OK)
      ctx->owner_handle = sce::Agc::ResourceRegistration::kInvalidOwnerHandle;

   ctx->cs_copy                      = details::make_cs(ctx->owner_handle, "psfx mb copy", motion_blur_copy_header, motion_blur_copy_text);
   ctx->cs_tile_range_h              = details::make_cs(ctx->owner_handle, "psfx mb tile range horizontal", motion_blur_tile_range_h_header, motion_blur_tile_range_h_text);
   ctx->cs_tile_range_v              = details::make_cs(ctx->owner_handle, "psfx mb tile range vertical", motion_blur_tile_range_v_header, motion_blur_tile_range_v_text);
   ctx->cs_neighbour_range           = details::make_cs(ctx->owner_handle, "psfx mb neighbour range", motion_blur_neighbour_range_header, motion_blur_neighbour_range_text);
   ctx->cs_blur[k_tile_type_regular] = details::make_cs(ctx->owner_handle, "psfx mb blur", motion_blur_header, motion_blur_text);
   ctx->cs_blur[k_tile_type_fast]    = details::make_cs(ctx->owner_handle, "psfx mb blur fast", motion_blur_fast_header, motion_blur_fast_text);

   sce::Agc::Core::TextureSpec tx_specs[k_target_count];
   make_tx_descriptors(conf, tx_specs);

   details::init_tx_chain(ctx->owner_handle, "psfx mb tile range horizontal", &ctx->targets[k_target_tile_range_h], &tx_specs[k_target_tile_range_h], 1, mem_arena.arena);
   details::init_tx_chain(ctx->owner_handle, "psfx mb tile range", &ctx->targets[k_target_tile_range], &tx_specs[k_target_tile_range], 1, mem_arena.arena);
   details::init_tx_chain(ctx->owner_handle, "psfx mb neighbour max", &ctx->targets[k_target_neighbour_max], &tx_specs[k_target_neighbour_max], 1, mem_arena.arena);
   details::init_tx_chain(ctx->owner_handle, "psfx mb output", &ctx->targets[k_target_output], &tx_specs[k_target_output], 1, mem_arena.arena);

   const uint32_t tile_buffer_count = details::round_div(conf.max_target_width, k_motion_blur_tile_size) * details::round_div(conf.max_target_height, k_motion_blur_tile_size);
   const uint32_t tile_buffer_size  = tile_buffer_count * sizeof(uint32_t);

   for (uint32_t i = 0; i < k_tile_type_count; i++)
   {
      void* const tile_buffer     = mem_arena.alloc(tile_buffer_size, sce::Agc::Alignment::kBuffer);
      void* const indirect_buffer = mem_arena.alloc(sizeof(sce::Agc::DispatchIndirectArgs), sce::Agc::Alignment::kAsyncDispatchIndirectArgs);

      sce::Agc::Core::BufferSpec spec;

      spec.initAsRegularBuffer(tile_buffer, sizeof(uint32_t), tile_buffer_count);
      sce::Agc::Core::initialize(&ctx->tile_buffers[i], &spec);

      spec.initAsRegularBuffer(indirect_buffer, sizeof(uint32_t), sizeof(sce::Agc::DispatchIndirectArgs) / sizeof(uint32_t));
      sce::Agc::Core::initialize(&ctx->indirect_buffers[i], &spec);
   }

   return ctx;
}

void motion_blur_terminate(MotionBlurContext* ctx)
{
   if (ctx == nullptr)
   {
      SCE_DBG_LOG_ERROR("Trying to terminate a NULL context.");
      return;
   }

   if (ctx->owner_handle != sce::Agc::ResourceRegistration::kInvalidOwnerHandle)
      sce::Agc::ResourceRegistration::unregisterOwnerAndResources(ctx->owner_handle);
}

template<typename T> struct WriteDataDst {};
template<> struct WriteDataDst<sce::Agc::DrawCommandBuffer>  { static constexpr auto k_dst = sce::Agc::WriteDataDst::kGl2Me; };
template<> struct WriteDataDst<sce::Agc::AsyncCommandBuffer> { static constexpr auto k_dst = sce::Agc::AsyncWriteDataDst::kGl2; };

template<typename T>
void motion_blur_apply(MotionBlurContext* ctx, T* dcb, const MotionBlurRuntimeOption& options)
{
   if (options.delta_time == 0.0f)
   {
      // No time passed -> no motion blur.
      return;
   }

   const bool resized =
      options.target_width != ctx->targets[k_target_output].getWidth() ||
      options.target_height != ctx->targets[k_target_output].getHeight();

   if (resized)
   {
      if (options.target_width * options.target_height == 0)
      {
         SCE_DBG_LOG_ERROR("The target dimensions are zero.");
         return;
      }
      else if (options.target_width > ctx->conf.max_target_width || options.target_height > ctx->conf.max_target_height)
      {
         SCE_DBG_LOG_ERROR("Target dimensions are greater than the context maximum.");
         return;
      }

      resize_targets(ctx, options.target_width, options.target_height);
   }

   details::MarkerAutoScope<T> marker(dcb, "psfx motion blur");

   const uint32_t tile_range_h_width  = details::round_div(options.target_width, k_motion_blur_tile_size);
   const uint32_t tile_range_h_height = options.target_height;
   const uint32_t tile_range_width    = tile_range_h_width;
   const uint32_t tile_range_height   = details::round_div(options.target_height, k_motion_blur_tile_size);

   const float exposure_time_scale = options.exposure_time / options.delta_time;
   const float2 velocity_scale(
      static_cast<float>(options.target_width) * exposure_time_scale * 0.5f,
      static_cast<float>(options.target_height) * exposure_time_scale * 0.5f);

   // Scale the maximum blur radius based on the current resolution. This means that the maximum
   // is the same fraction of the screen regardless of resolution, and therefore prevents the
   // possibility that we blur more at lower resolutions.
   const float max_blur =
      static_cast<float>(k_motion_blur_tile_size) *
      (static_cast<float>(options.target_width) / static_cast<float>(ctx->conf.max_target_width));

   // Initialise the indirect dispatch buffers.
   const sce::Agc::DispatchIndirectArgs indirect_init = { 0, 1, 1 };
   for (uint32_t i = 0; i < k_tile_type_count; i++)
   {
      dcb->writeData(
         WriteDataDst<T>::k_dst,
         sce::Agc::CachePolicy::kLru,
         reinterpret_cast<uint64_t>(ctx->indirect_buffers[i].getDataAddress()),
         &indirect_init,
         sizeof(indirect_init) / sizeof(uint32_t));
   }

   // Calculate per-tile maximum/minimum (magnitude) velocity. This is done in two passes,
   // horizontally and then vertically.
   {
      details::MarkerAutoScope<T> marker_tile_range(dcb, "tile range");

      {
         details::MarkerAutoScope<T> marker_tile_range_h(dcb, "tile range horizontal");

         TileRangeHSrt* srt = details::alloc_top_down<TileRangeHSrt>(dcb);
         srt->velocity          = options.source_velocity;
         srt->tile_range_h      = ctx->targets[k_target_tile_range_h];
         srt->inv_source_size.x = 1.0f / static_cast<float>(options.target_width);
         srt->inv_source_size.y = 1.0f / static_cast<float>(options.target_height);

         details::set_compute_shader(dcb, ctx->cs_tile_range_h);
         details::set_srt(dcb, ctx->cs_tile_range_h, &srt);

         dcb->dispatch(
            details::round_div(tile_range_h_width, k_tile_range_group_size),
            details::round_div(tile_range_h_height, k_tile_range_group_size),
            1,
            ctx->cs_tile_range_h->m_specials->m_dispatchModifier);

         details::cs_sync(dcb);
      }

      {
         details::MarkerAutoScope<T> marker_tile_range_v(dcb, "tile range vertical");

         TileRangeVSrt srt;
         srt.tile_range_h = ctx->targets[k_target_tile_range_h];
         srt.tile_range   = ctx->targets[k_target_tile_range];

         details::set_compute_shader(dcb, ctx->cs_tile_range_v);
         details::set_srt(dcb, ctx->cs_tile_range_v, &srt);

         dcb->dispatch(
            details::round_div(tile_range_width, k_tile_range_group_size),
            details::round_div(tile_range_height, k_tile_range_group_size),
            1,
            ctx->cs_tile_range_v->m_specials->m_dispatchModifier);

         details::cs_sync(dcb);
      }
   }

   // For each tile, calculate maximum/minimum velocity in 3x3 tile neighbourhood, and classify
   // tiles into unaffected, fast path or regular path.
   {
      details::MarkerAutoScope<T> marker_neighbour_range(dcb, "neighbour range");

      NeighbourRangeSrt* srt = details::alloc_top_down<NeighbourRangeSrt>(dcb);
      srt->tile_range     = ctx->targets[k_target_tile_range];
      srt->neighbour_max  = ctx->targets[k_target_neighbour_max];
      srt->tex_max[0]     = tile_range_width - 1;
      srt->tex_max[1]     = tile_range_height - 1;
      srt->velocity_scale = velocity_scale;

      for (uint32_t i = 0; i < k_tile_type_count; i++)
      {
         srt->tile_buffers[i]     = ctx->tile_buffers[i];
         srt->indirect_buffers[i] = ctx->indirect_buffers[i];
      }

      details::set_compute_shader(dcb, ctx->cs_neighbour_range);
      details::set_srt(dcb, ctx->cs_neighbour_range, &srt);

      dcb->dispatch(
         details::round_div(tile_range_width, k_neighbour_range_group_size),
         details::round_div(tile_range_height, k_neighbour_range_group_size),
         1,
         ctx->cs_neighbour_range->m_specials->m_dispatchModifier);

      details::cs_sync(dcb);

      // Ensure PFP sees the written indirect dispatch arguments.
      details::stall_pfp(dcb);
   }

   if (k_output_dcc && resized)
   {
      // If we resized the targets we need to clear the output texture if DCC is enabled. Otherwise
      // corruption can occur due to stale metadata mismatching the target contents.
      details::clear_texture(dcb, ctx->targets[k_target_output]);
      details::cs_sync(dcb);
   }

   // Main blur pass.
   //
   // We sample from the specified destination texture, and output to an intermediate texture
   // (can't read/write same texture as blurred results for one pixel may be sampled for
   // others rather than the original values).
   //
   // This uses indirect dispatches generated in the neighbour range pass, only affected tiles
   // are written to the output buffer (bear this in mind if viewing the intermediate texture,
   // unmodified tiles will contain data from previous frames, or be uninitialised).
   details::SyncResult blur_sync[k_tile_type_count];

   {
      details::MarkerAutoScope<T> marker_blur(dcb, "blur");

      for (uint32_t i = 0; i < k_tile_type_count; i++)
      {
         details::MarkerAutoScope<T> marker_type(dcb, k_tile_type_names[i]);

         BlurSrt* srt = details::alloc_top_down<BlurSrt>(dcb);
         srt->colour         = options.destination;
         srt->velocity       = options.source_velocity;
         srt->linear_depth   = options.source_linear_depth;
         srt->neighbour_max  = ctx->targets[k_target_neighbour_max];
         srt->output         = ctx->targets[k_target_output];
         srt->tile_buffer    = ctx->tile_buffers[i];
         srt->source_max[0]  = options.target_width - 1;
         srt->source_max[1]  = options.target_height - 1;
         srt->max_blur       = max_blur;
         srt->z_near         = options.near_plane_distance;
         srt->z_range        = options.far_plane_distance - options.near_plane_distance;
         srt->velocity_scale = velocity_scale;

         details::set_compute_shader(dcb, ctx->cs_blur[i]);
         details::set_srt(dcb, ctx->cs_blur[i], &srt);

         details::dispatch_indirect(
            dcb,
            reinterpret_cast<sce::Agc::DispatchIndirectArgs*>(ctx->indirect_buffers[i].getDataAddress()),
            ctx->cs_blur[i]->m_specials->m_dispatchModifier);

         // Each tile will only be affected by 1 dispatch so we can let them overlap execution.
         // The later copy dispatch for each tile type only depends on the blur dispatch for
         // that type having completed. We can use a split barrier per tile type to synchronise
         // here, rather than draining compute between blur and copy - this keeps the GPU busy
         // all the way through from the start of blur to the end of copy (as long as there is
         // a mix of tiles of both tile types).
         blur_sync[i] = details::write_cs_sync(dcb);
      }
   }

   // Copy back modified tiles from the intermediate output to the main texture. We reuse the
   // same indirect dispatch lists as the blur pass here.
   {
      details::MarkerAutoScope<T> marker_copy(dcb, "output");

      details::set_compute_shader(dcb, ctx->cs_copy);

      for (uint32_t i = 0; i < k_tile_type_count; i++)
      {
         details::MarkerAutoScope<T> marker_type(dcb, k_tile_type_names[i]);

         details::wait_cs_sync(dcb, blur_sync[i]);

         CopySrt* srt = details::alloc_top_down<CopySrt>(dcb);
         srt->source      = ctx->targets[k_target_output];
         srt->dest        = options.destination;
         srt->tile_buffer = ctx->tile_buffers[i];

         details::set_srt(dcb, ctx->cs_copy, &srt);

         details::dispatch_indirect(
            dcb,
            reinterpret_cast<sce::Agc::DispatchIndirectArgs*>(ctx->indirect_buffers[i].getDataAddress()),
            ctx->cs_copy->m_specials->m_dispatchModifier);
      }

      details::cs_sync(dcb);
   }
}

void motion_blur_apply(MotionBlurContext* ctx, sce::Agc::AsyncCommandBuffer* acb, const MotionBlurRuntimeOption& options)
{
   return motion_blur_apply<sce::Agc::AsyncCommandBuffer>(ctx, acb, options);
}

void motion_blur_apply(MotionBlurContext* ctx, sce::Agc::DrawCommandBuffer* dcb, const MotionBlurRuntimeOption& options)
{
   return motion_blur_apply<sce::Agc::DrawCommandBuffer>(ctx, dcb, options);
}

}
