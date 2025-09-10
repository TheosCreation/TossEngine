/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

//
// References:
// [Uludag14] Hi-Z Screen-Space Cone-Traced Reflections
//     GPU Pro 5: Advanced Rendering Techniques (https://www.routledge.com/GPU-Pro-5-Advanced-Rendering-Techniques-1st-Edition/Engel/p/book/9781482208634)
// [Bitsquid17] Notes on Screen Space HiZ Tracing
//     http://bitsquid.blogspot.com/2017/08/notes-on-screen-space-hiz-tracing.html
//
// TODO:
//  - Support tracing at reduced resolution and upsampling results to full resolution.
//  - Support reflection rays facing toward camera.
//  - Support tracing rays behind surfaces.
//

#include "psfx_common.h"
#include "psfx_ssr.h"

#include "psfx_details.h"

#include "../shaders/psfx_ssr.h"

namespace psfx
{

enum SsrTarget
{
   // Minimum depth mip chain.
   k_target_depth_chain,

   // Output mip level target.
   k_target_mip_level,

   // Reflection/mask mip chains.
   k_target_reflection_chain,
   k_target_mask_chain,

   // Temporary targets for blurring.
   k_target_temp_reflection,
   k_target_temp_mask,

   k_target_count
};

struct SsrContext
{
   SsrConfiguration conf;

   sce::Agc::ResourceRegistration::OwnerHandle owner_handle;

   sce::Agc::Shader* cs_blur_h;
   sce::Agc::Shader* cs_blur_v;
   sce::Agc::Shader* cs_classify;
   sce::Agc::Shader* cs_clear;
   sce::Agc::Shader* cs_depth_downscale;
   sce::Agc::Shader* cs_depth_downscale_gather;
   sce::Agc::Shader* cs_output;
   sce::Agc::Shader* cs_reproject;
   sce::Agc::Shader* cs_trace;
   sce::Agc::Shader* cs_trace_reverse_z;

   sce::Agc::Core::Texture targets[k_target_count];

   sce::Agc::Core::Buffer tile_buffers[k_tile_type_count];
};

extern char       ssr_blur_h_header[];
extern const char ssr_blur_h_text[];
extern char       ssr_blur_v_header[];
extern const char ssr_blur_v_text[];
extern char       ssr_classify_header[];
extern const char ssr_classify_text[];
extern char       ssr_clear_header[];
extern const char ssr_clear_text[];
extern char       ssr_depth_downscale_header[];
extern const char ssr_depth_downscale_text[];
extern char       ssr_depth_downscale_gather_header[];
extern const char ssr_depth_downscale_gather_text[];
extern char       ssr_output_header[];
extern const char ssr_output_text[];
extern char       ssr_reproject_header[];
extern const char ssr_reproject_text[];
extern char       ssr_trace_header[];
extern const char ssr_trace_text[];
extern char       ssr_trace_reverse_z_header[];
extern const char ssr_trace_reverse_z_text[];

static const sce::Agc::Core::DataFormat k_depth_format      = { sce::Agc::Core::TypedFormat::k32Float, sce::Agc::Core::Swizzle::kR001_R1S1 };
static const sce::Agc::Core::DataFormat k_reflection_format = { sce::Agc::Core::TypedFormat::k11_11_10Float, sce::Agc::Core::Swizzle::kRGB1_R3S34 };
static const sce::Agc::Core::DataFormat k_mask_format       = { sce::Agc::Core::TypedFormat::k8UNorm, sce::Agc::Core::Swizzle::kR001_R1S1 };
static const sce::Agc::Core::DataFormat k_mip_level_format  = { sce::Agc::Core::TypedFormat::k8UNorm, sce::Agc::Core::Swizzle::kR001_R1S1 };

static const uint32_t k_min_reflection_mip_size = 8;

static uint32_t calc_reflection_mip_count(const SsrConfiguration& conf, uint32_t width, uint32_t height)
{
   uint32_t mips   = 1;
   while (width > k_min_reflection_mip_size && height > k_min_reflection_mip_size)
   {
      width >>= 1;
      height >>= 1;
      mips++;
   }

   return (conf.max_mip_count == 0) ? mips : std::min(mips, conf.max_mip_count);
}

static void make_tx_descriptors(const SsrConfiguration& conf, sce::Agc::Core::TextureSpec* tx_specs)
{
   // Depth needs a full mip chain, excluding the top mip as we sample the original texture.
   uint32_t depth_width = conf.max_target_width >> 1;
   uint32_t depth_height = conf.max_target_height >> 1;
   details::make_tx_mip_descriptors(depth_width, depth_height, 0, 1, k_depth_format, tx_specs[k_target_depth_chain]);

   details::make_tx_descriptors(conf.max_target_width, conf.max_target_height, 1, k_mip_level_format, &tx_specs[k_target_mip_level], 1);

   // Reflection/mask need a mip chain but we limit the number of mips as there's not a lot of
   // point to having the smallest mips.
   //
   // We enable DCC for these targets to enable fast clears, see ssr_generate(). It also gives
   // some performance improvement for reads/writes to the reflection colour target.
   //
   // The mask target is k8UNorm, and generally DCC is not efficient for 1 byte per pixel formats.
   // For our usage, it doesn't cause any significant performance change to reads/writes on this
   // target, however it is still an overall improvement due to the fast clear in most cases
   // (except where most/all of the screen tiles need SSR calculated rather than just clearing).
   uint32_t reflection_mip_count = calc_reflection_mip_count(conf, conf.max_target_width, conf.max_target_height);
   details::make_tx_mip_descriptors(conf.max_target_width, conf.max_target_height, reflection_mip_count, 1, k_reflection_format, tx_specs[k_target_reflection_chain], true);
   details::make_tx_mip_descriptors(conf.max_target_width, conf.max_target_height, reflection_mip_count, 1, k_mask_format, tx_specs[k_target_mask_chain], true);

   // These are created as large enough for mip level 1, and we overlay the smaller mips onto the
   // same memory.
   uint32_t temp_width  = conf.max_target_width >> 1;
   uint32_t temp_height = conf.max_target_height >> 1;
   details::make_tx_descriptors(temp_width, temp_height, 1, k_reflection_format, &tx_specs[k_target_temp_reflection], 1, true);
   details::make_tx_descriptors(temp_width, temp_height, 1, k_mask_format, &tx_specs[k_target_temp_mask], 1);
}

static void resize_targets(SsrContext* ctx, uint32_t width, uint32_t height)
{
   uint32_t depth_width = width >> 1;
   uint32_t depth_height = height >> 1;
   details::resize_tx(depth_width, depth_height, 0, ctx->targets[k_target_depth_chain]);

   details::resize_tx(width, height, 1, ctx->targets[k_target_mip_level]);

   uint32_t reflection_mip_count = calc_reflection_mip_count(ctx->conf, width, height);
   details::resize_tx(width, height, reflection_mip_count, ctx->targets[k_target_reflection_chain]);
   details::resize_tx(width, height, reflection_mip_count, ctx->targets[k_target_mask_chain]);

   uint32_t temp_width = width >> 1;
   uint32_t temp_height = height >> 1;
   details::resize_tx(temp_width, temp_height, 1, ctx->targets[k_target_temp_reflection]);
   details::resize_tx(temp_width, temp_height, 1, ctx->targets[k_target_temp_mask]);

   uint32_t tile_buffer_count = details::round_div(width, k_classify_tile_size) * details::round_div(height, k_classify_tile_size);

   for (uint32_t i = 0; i < k_tile_type_count; i++)
   {
      sce::Agc::Core::BufferSpec spec;
      spec.initAsRegularBuffer(ctx->tile_buffers[i].getDataAddress(), sizeof(uint32_t), tile_buffer_count);
      sce::Agc::Core::initialize(&ctx->tile_buffers[i], &spec);
   }
}

void ssr_configuration_init(SsrConfiguration& conf)
{
   memset(&conf, 0, sizeof(conf));
}

int ssr_compute_memory_requirement(const SsrConfiguration& conf, MemoryRequirement& requirement)
{
   if (conf.max_target_width * conf.max_target_height == 0)
   {
      SCE_DBG_LOG_ERROR("The target dimensions are zero.");
      return -1;
   }

   sce::Agc::SizeAlign& mem_size = requirement.mem_size;

   mem_size = { 0, 8 };
   mem_size.m_size += sizeof(SsrContext);

   sce::Agc::Core::TextureSpec tx_specs[k_target_count];
   make_tx_descriptors(conf, tx_specs);
   details::tx_memory_usage(tx_specs, k_target_count, mem_size);

   uint32_t tile_buffer_count = details::round_div(conf.max_target_width, k_classify_tile_size) * details::round_div(conf.max_target_height, k_classify_tile_size);
   uint32_t tile_buffer_size  = tile_buffer_count * sizeof(uint32_t);

   for (uint32_t i = 0; i < k_tile_type_count; i++)
   {
      mem_size.m_size = details::align_to(mem_size.m_size, sce::Agc::Alignment::kBuffer);
      mem_size.m_size += tile_buffer_size;
   }

   return SCE_OK;
}

SsrContext* ssr_initialize(const SsrConfiguration& conf, const MemoryPool& memory_pool)
{
   if (!memory_pool.mem_ptr)
   {
      SCE_DBG_LOG_ERROR("mem_ptr can't be NULL.");
      return nullptr;
   }

   details::ArenaAllocator mem_arena(memory_pool.mem_ptr);

   SsrContext* ctx = mem_arena.alloc<SsrContext>(sce::Agc::Alignment::kBuffer);
   memset(ctx, 0, sizeof(SsrContext));
   ctx->conf = conf;

   SceError err = sce::Agc::ResourceRegistration::registerOwner(&ctx->owner_handle, "psfx ssr");
   if (err != SCE_OK)
      ctx->owner_handle = sce::Agc::ResourceRegistration::kInvalidOwnerHandle;

   ctx->cs_blur_h                 = details::make_cs(ctx->owner_handle, "psfx ssr blur h", ssr_blur_h_header, ssr_blur_h_text);
   ctx->cs_blur_v                 = details::make_cs(ctx->owner_handle, "psfx ssr blur v", ssr_blur_v_header, ssr_blur_v_text);
   ctx->cs_classify               = details::make_cs(ctx->owner_handle, "psfx ssr classify", ssr_classify_header, ssr_classify_text);
   ctx->cs_clear                  = details::make_cs(ctx->owner_handle, "psfx ssr clear", ssr_clear_header, ssr_clear_text);
   ctx->cs_depth_downscale        = details::make_cs(ctx->owner_handle, "psfx ssr depth downscale", ssr_depth_downscale_header, ssr_depth_downscale_text);
   ctx->cs_depth_downscale_gather = details::make_cs(ctx->owner_handle, "psfx ssr depth downscale gather", ssr_depth_downscale_gather_header, ssr_depth_downscale_gather_text);
   ctx->cs_output                 = details::make_cs(ctx->owner_handle, "psfx ssr output", ssr_output_header, ssr_output_text);
   ctx->cs_reproject              = details::make_cs(ctx->owner_handle, "psfx ssr reproject", ssr_reproject_header, ssr_reproject_text);
   ctx->cs_trace                  = details::make_cs(ctx->owner_handle, "psfx ssr trace", ssr_trace_header, ssr_trace_text);
   ctx->cs_trace_reverse_z        = details::make_cs(ctx->owner_handle, "psfx ssr trace reverse z", ssr_trace_reverse_z_header, ssr_trace_reverse_z_text);

   sce::Agc::Core::TextureSpec tx_specs[k_target_count];
   make_tx_descriptors(conf, tx_specs);

   auto init_tx = [&] (SsrTarget index, const char* name) { details::init_tx_chain(ctx->owner_handle, name, &ctx->targets[index], &tx_specs[index], 1, mem_arena.arena); };

   init_tx(k_target_depth_chain, "psfx ssr depth chain");
   init_tx(k_target_mip_level, "psfx ssr mip level");
   init_tx(k_target_reflection_chain, "psfx ssr reflection chain");
   init_tx(k_target_mask_chain, "psfx ssr mask chain");
   init_tx(k_target_temp_reflection, "psfx ssr temp reflection");
   init_tx(k_target_temp_mask, "psfx ssr temp mask");

   uint32_t tile_buffer_count = details::round_div(conf.max_target_width, k_classify_tile_size) * details::round_div(conf.max_target_height, k_classify_tile_size);
   uint32_t tile_buffer_size  = tile_buffer_count * sizeof(uint32_t);

   for (uint32_t i = 0; i < k_tile_type_count; i++)
   {
      void* tile_buffer = mem_arena.alloc(tile_buffer_size, sce::Agc::Alignment::kBuffer);
      
      sce::Agc::Core::BufferSpec spec;
      spec.initAsRegularBuffer(tile_buffer, sizeof(uint32_t), tile_buffer_count);
      sce::Agc::Core::initialize(&ctx->tile_buffers[i], &spec);
   }

   return ctx;
}

void ssr_terminate(SsrContext* ctx)
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
void ssr_generate(SsrContext* ctx, T* cb, const SsrRuntimeOption& options)
{
   bool resized =
      options.target_width != ctx->targets[k_target_mip_level].getWidth() ||
      options.target_height != ctx->targets[k_target_mip_level].getHeight();

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

   details::MarkerAutoScope<T> marker(cb, "psfx ssr generate");

   if (resized)
   {
      // If we resized the targets we need to clear the ones using DCC, otherwise corruption can
      // occur due to stale metadata mismatching the target contents.
      details::clear_texture(cb, ctx->targets[k_target_reflection_chain]);
      details::clear_texture(cb, ctx->targets[k_target_mask_chain]);
      details::clear_texture(cb, ctx->targets[k_target_temp_reflection]);
      details::cs_sync(cb);
   }

   // To start, classify tiles based on roughness into tiles which need SSR and those that do not.
   // This produces an indirect dispatch list for each type of tile.
   //
   // This also produces a flag which indicates whether there are any tiles using SSR at all. This
   // is used to predicate various subsequent steps, which we use to avoid doing most of the work
   // when SSR is not needed for a frame.
   sce::Agc::DispatchIndirectArgs* indirect_mem[k_tile_type_count];
   sce::Agc::Core::Buffer indirect_buffers[k_tile_type_count];
   for (uint32_t i = 0; i < k_tile_type_count; i++)
   {
      indirect_mem[i] = details::alloc_top_down<sce::Agc::DispatchIndirectArgs>(cb, 1, sce::Agc::Alignment::kAsyncDispatchIndirectArgs);

      indirect_mem[i]->m_dimX = 0;
      indirect_mem[i]->m_dimY = 1;
      indirect_mem[i]->m_dimZ = 1;

      sce::Agc::Core::BufferSpec indirect_spec;
      indirect_spec.initAsRegularBuffer(indirect_mem[i], sizeof(uint32_t), sizeof(sce::Agc::DispatchIndirectArgs) / sizeof(uint32_t));
      sce::Agc::Core::initialize(&indirect_buffers[i], &indirect_spec);
   }

   uint32_t* need_ssr_mem = details::alloc_top_down<uint32_t>(cb);
   need_ssr_mem[0] = 0;

   sce::Agc::Core::Buffer need_ssr_buffer;
   sce::Agc::Core::BufferSpec need_ssr_spec;
   need_ssr_spec.initAsRegularBuffer(need_ssr_mem, sizeof(uint32_t), 1);
   sce::Agc::Core::initialize(&need_ssr_buffer, &need_ssr_spec);

   {
      details::MarkerAutoScope<T> marker_classify(cb, "classify");

      ClassifySrt srt;
      srt.normal_roughness  = options.source_normal_roughness;
      srt.inv_target_size.x = 1.0f / static_cast<float>(options.target_width);
      srt.inv_target_size.y = 1.0f / static_cast<float>(options.target_height);
      srt.roughness_cutoff  = options.roughness_cutoff;

      for (uint32_t i = 0; i < k_tile_type_count; i++)
      {
         srt.buffers[i] = details::alloc_top_down<ClassifySrtBuffers>(cb);
         srt.buffers[i]->tile_buffer     = ctx->tile_buffers[i];
         srt.buffers[i]->indirect_buffer = indirect_buffers[i];
         srt.buffers[i]->need_ssr        = need_ssr_buffer;
      }

      details::set_compute_shader(cb, ctx->cs_classify);
      details::set_srt(cb, ctx->cs_classify, &srt);

      // Group size is half tile size since each thread reads 2x2 pixels.
      cb->dispatch(
         details::round_div(options.target_width, k_classify_tile_size),
         details::round_div(options.target_height, k_classify_tile_size),
         1,
         ctx->cs_classify->m_specials->m_dispatchModifier);

      details::cs_sync(cb);

      // Ensure PFP sees the written indirect dispatch arguments.
      details::stall_pfp(cb);
   }

   // Generate our hierarchical depth buffer.
   {
      details::MarkerAutoScope<T> marker_depth(cb, "depth chain");

      uint32_t source_width = options.target_width;
      uint32_t source_height = options.target_height;

      sce::Agc::Core::Texture dest_depth = ctx->targets[k_target_depth_chain];

      // To save us having to copy the original depth into the top mip of our chain, the trace
      // shader special cases access to level 0 to access the original texture, and the chain
      // texture effectively starts with level 1. We don't allocate the top mip in it so level
      // indices are shifted by 1.
      sce::Agc::Shader* curr_shader = nullptr;
      for (uint32_t level = 0; level < ctx->targets[k_target_depth_chain].getNumMipLevels(); level++)
      {
         dest_depth.setMipLevelRange(level, level);

         DepthDownscaleSrt* srt = details::alloc_top_down<DepthDownscaleSrt>(cb);
         srt->depth             = (level > 0) ? ctx->targets[k_target_depth_chain] : options.source_depth;
         srt->dest_depth        = dest_depth;
         srt->inv_source_size.x = 1.0f / static_cast<float>(source_width);
         srt->inv_source_size.y = 1.0f / static_cast<float>(source_height);
         srt->source_size.x     = source_width;
         srt->source_size.y     = source_height;
         srt->source_level      = (level == 0) ? 0 : level - 1;
         srt->depth_scale       = (options.reverse_z) ? -1.0f : 1.0f;

         // See shader source for details.
         sce::Agc::Shader* shader = (source_width & 1 || source_height & 1)
            ? ctx->cs_depth_downscale
            : ctx->cs_depth_downscale_gather;

         if (shader != curr_shader)
         {
            details::set_compute_shader(cb, shader);
            curr_shader = shader;
         }

         details::set_srt(cb, shader, &srt);

         // We want to dispatch with the destination mip size.
         if (source_width > 1)  source_width >>= 1;
         if (source_height > 1) source_height >>= 1;

         // Each thread reads 2x2 pixels and reduces to 1 pixel. This is predicated on whether we
         // need SSR since the depth chain is otherwise unneeded.
         details::cond_exec(cb, need_ssr_mem, [&](T* cond_cb)
            {
               cond_cb->dispatch(
                  details::round_div(source_width, k_depth_group_size),
                  details::round_div(source_height, k_depth_group_size),
                  1,
                  shader->m_specials->m_dispatchModifier);

               details::cs_sync(cond_cb);
            });
      }
   }

   sce::Agc::Core::Texture dest_reflection = ctx->targets[k_target_reflection_chain];
   sce::Agc::Core::Texture dest_mask       = ctx->targets[k_target_mask_chain];

   unsigned int reflection_mip_count = std::max(1u, std::min(dest_reflection.getNumMipLevels(), options.mip_count));

   dest_reflection.setMipLevelRange(0, 0);
   dest_mask.setMipLevelRange(0, 0);

   // If we are using SSR at all in this frame, we must clear the non-SSR tiles in the intermediate
   // mip chain, so that the downsample/blur produces correct results.
   //
   // When DCC is enabled on the targets (it is by default, support for having it disabled is here
   // for testing purposes), it is fastest for us to do a DCC fast clear on the whole targets as
   // this just requires initializing the metadata.
   //
   // Otherwise, we do an indirect dispatch to write zeros only to the non-SSR tiles. This is done
   // after dispatching the SSR tiles, as that dispatch drains out fairly slowly, so the clear can
   // start as this happens and get better utilization.
   //
   // These have already been cleared if we resized textures.
   bool can_fast_clear_chain =
      dest_reflection.getTextureMetadataType() == sce::Agc::Core::Texture::TextureMetadataType::kTextureMetadataTypeDcc &&
      dest_mask.getTextureMetadataType() == sce::Agc::Core::Texture::TextureMetadataType::kTextureMetadataTypeDcc;
   if (can_fast_clear_chain && !resized)
   {
      details::cond_exec(cb, need_ssr_mem, [&](T* cond_cb)
         {
            details::clear_texture(cond_cb, dest_reflection);
            details::clear_texture(cond_cb, dest_mask);
            details::cs_sync(cond_cb);
         });
   }

   // Trace rays for SSR tiles.
   {
      details::MarkerAutoScope<T> marker_trace(cb, "trace");

      TraceSrtInline srt;
      srt.depth_chain      = ctx->targets[k_target_depth_chain];
      srt.tile_buffer      = ctx->tile_buffers[k_tile_type_ssr];
      srt.roughness_cutoff = options.roughness_cutoff;
      srt.flags            = options.clip_space == sce::Agc::CxClipControl::ClipSpace::kDX ? k_trace_clip_space_dx : 0;

      srt.indirect = details::alloc_top_down<TraceSrtIndirect>(cb);

      srt.indirect->colour               = options.source_colour;
      srt.indirect->depth                = options.source_depth;
      srt.indirect->normal_roughness     = options.source_normal_roughness;
      srt.indirect->dest_reflection      = dest_reflection;
      srt.indirect->dest_mask            = dest_mask;
      srt.indirect->dest_mip_level       = ctx->targets[k_target_mip_level];
      srt.indirect->inv_target_size.x    = 1.0f / static_cast<float>(options.target_width);
      srt.indirect->inv_target_size.y    = 1.0f / static_cast<float>(options.target_height);
      srt.indirect->target_size.x        = static_cast<float>(options.target_width);
      srt.indirect->target_size.y        = static_cast<float>(options.target_height);
      srt.indirect->max_iterations       = options.max_iterations;
      srt.indirect->edge_fade_threshold  = options.edge_fade_threshold;
      srt.indirect->object_thickness     = options.object_thickness;
      srt.indirect->reflection_mip_count = reflection_mip_count;
      srt.indirect->reflection_mip_scale = options.mip_scale;
      srt.indirect->contact_max_distance = options.contact_max_distance;
      srt.indirect->contact_max_mip_bias = options.contact_max_mip_bias;

      to_float4x4(options.view_matrix, srt.indirect->view_matrix);
      to_float4x4(options.projection_matrix, srt.indirect->projection_matrix);
      to_float4x4(inverse(options.projection_matrix), srt.indirect->inv_projection_matrix);

      sce::Agc::Shader* shader = (options.reverse_z) ? ctx->cs_trace_reverse_z : ctx->cs_trace;

      details::set_compute_shader(cb, shader);
      details::set_srt(cb, shader, &srt);

      details::dispatch_indirect(
         cb,
         indirect_mem[k_tile_type_ssr],
         shader->m_specials->m_dispatchModifier);
   }

   // See comment earlier regarding clearing.
   if (!can_fast_clear_chain && !resized)
   {
      details::MarkerAutoScope<T> marker_clear(cb, "clear intermediate");

      ClearSrt* srt = details::alloc_top_down<ClearSrt>(cb);
      srt->tile_buffer     = ctx->tile_buffers[k_tile_type_clear];
      srt->dest_reflection = dest_reflection;
      srt->dest_mask       = dest_mask;

      details::set_compute_shader(cb, ctx->cs_clear);
      details::set_srt(cb, ctx->cs_clear, &srt);

      details::cond_exec(cb, need_ssr_mem, [&](T* cond_cb)
         {
            details::dispatch_indirect(
               cond_cb,
               indirect_mem[k_tile_type_clear],
               ctx->cs_clear->m_specials->m_dispatchModifier);
         });
   }

   // Blur depends on completion of both clear and SSR tiles, sync.
   details::cs_sync(cb);

   // Generate blurred reflection/mask mip chain.
   {
      details::MarkerAutoScope<T> marker_colour(cb, "reflection chain");

      uint32_t dest_width = options.target_width;
      uint32_t dest_height = options.target_height;

      // Temporary textures for horizontal blur step. We allocated enough for mip level 1, and
      // overlay the smaller mips into the same space.
      sce::Agc::Core::TextureSpec temp_reflection_spec;
      sce::Agc::Core::translate(&temp_reflection_spec, &ctx->targets[k_target_temp_reflection]);
      sce::Agc::Core::TextureSpec temp_mask_spec;
      sce::Agc::Core::translate(&temp_mask_spec, &ctx->targets[k_target_temp_mask]);

      for (uint32_t level = 1; level < reflection_mip_count; level++)
      {
         dest_reflection.setMipLevelRange(level, level);
         dest_mask.setMipLevelRange(level, level);

         if (dest_width > 1)  dest_width >>= 1;
         if (dest_height > 1) dest_height >>= 1;

         // Temporary target for horizontal blur.
         temp_reflection_spec.setWidth(dest_width);
         temp_reflection_spec.setHeight(dest_height);
         sce::Agc::Core::Texture temp_reflection;
         sce::Agc::Core::initialize(&temp_reflection, &temp_reflection_spec);
         temp_mask_spec.setWidth(dest_width);
         temp_mask_spec.setHeight(dest_height);
         sce::Agc::Core::Texture temp_mask;
         sce::Agc::Core::initialize(&temp_mask, &temp_mask_spec);

         // Downscale and horizontal blur to temporary target.
         {
            BlurSrt* srt = details::alloc_top_down<BlurSrt>(cb);
            srt->reflection      = ctx->targets[k_target_reflection_chain];
            srt->mask            = ctx->targets[k_target_mask_chain];
            srt->dest_reflection = temp_reflection;
            srt->dest_mask       = temp_mask;
            srt->inv_dest_size.x = 1.0f / static_cast<float>(dest_width);
            srt->inv_dest_size.y = 1.0f / static_cast<float>(dest_height);
            srt->source_level    = level - 1;

            details::set_compute_shader(cb, ctx->cs_blur_h);
            details::set_srt(cb, ctx->cs_blur_h, &srt);

            // Not needed with no reflection.
            details::cond_exec(cb, need_ssr_mem, [&](T* cond_cb)
               {
                  cond_cb->dispatch(
                     details::round_div(dest_width, k_blur_group_size),
                     details::round_div(dest_height, k_blur_group_size),
                     1,
                     ctx->cs_blur_h->m_specials->m_dispatchModifier);

                  details::cs_sync(cond_cb);
               });
         }

         // Vertical blur.
         {
            BlurSrt* srt = details::alloc_top_down<BlurSrt>(cb);
            srt->reflection      = temp_reflection;
            srt->mask            = temp_mask;
            srt->dest_reflection = dest_reflection;
            srt->dest_mask       = dest_mask;
            srt->inv_dest_size.x = 1.0f / static_cast<float>(dest_width);
            srt->inv_dest_size.y = 1.0f / static_cast<float>(dest_height);
            srt->source_level     = 0; // Using temporary target with only 1 mip.

            details::set_compute_shader(cb, ctx->cs_blur_v);
            details::set_srt(cb, ctx->cs_blur_v, &srt);

            // Not needed with no reflection.
            details::cond_exec(cb, need_ssr_mem, [&](T* cond_cb)
               {
                  cond_cb->dispatch(
                     details::round_div(dest_width, k_blur_group_size),
                     details::round_div(dest_height, k_blur_group_size),
                     1,
                     ctx->cs_blur_v->m_specials->m_dispatchModifier);

                  details::cs_sync(cond_cb);
               });
         }
      }
   }

   // We must clear non-SSR tiles in the final output. As with the intermediate target clear
   // earlier, if DCC is enabled for the targets, a DCC fast clear is the best option.
   //
   // If not, we do another indirect dispatch to clear only the non-SSR tiles. Again,
   // dispatching the clear tiles after the SSR tiles achieves better overlap/utilization.
   bool can_fast_clear_output =
      options.dest_reflection.getTextureMetadataType() == sce::Agc::Core::Texture::TextureMetadataType::kTextureMetadataTypeDcc &&
      options.dest_mask.getTextureMetadataType() == sce::Agc::Core::Texture::TextureMetadataType::kTextureMetadataTypeDcc;
   if (can_fast_clear_output)
   {
      details::clear_texture(cb, options.dest_reflection, sce::Agc::Core::Encoder::encode({ 0 }));
      details::clear_texture(cb, options.dest_mask, sce::Agc::Core::Encoder::encode({ 0 }));
      details::cs_sync(cb);
   }

   // Generate final output reflections in SSR tiles.
   {
      details::MarkerAutoScope<T> marker_output(cb, "output");

      // Ensure mip level is clamped to the requested range when sampling.
      dest_reflection.setMipLevelRange(0, reflection_mip_count);
      dest_mask.setMipLevelRange(0, reflection_mip_count);

      OutputSrt* srt = details::alloc_top_down<OutputSrt>(cb);
      srt->tile_buffer       = ctx->tile_buffers[k_tile_type_ssr];
      srt->reflection        = dest_reflection;
      srt->mask              = dest_mask;
      srt->mip_level         = ctx->targets[k_target_mip_level];
      srt->dest_reflection   = options.dest_reflection;
      srt->dest_mask         = options.dest_mask;
      srt->inv_target_size.x = 1.0f / static_cast<float>(options.target_width);
      srt->inv_target_size.y = 1.0f / static_cast<float>(options.target_height);
      srt->mip_count         = reflection_mip_count;

      details::set_compute_shader(cb, ctx->cs_output);
      details::set_srt(cb, ctx->cs_output, &srt);

      details::dispatch_indirect(
         cb,
         indirect_mem[k_tile_type_ssr],
         ctx->cs_output->m_specials->m_dispatchModifier);
   }

   if (!can_fast_clear_output)
   {
      details::MarkerAutoScope<T> marker_clear(cb, "clear output");

      ClearSrt* srt = details::alloc_top_down<ClearSrt>(cb);
      srt->tile_buffer     = ctx->tile_buffers[k_tile_type_clear];
      srt->dest_reflection = options.dest_reflection;
      srt->dest_mask       = options.dest_mask;

      details::set_compute_shader(cb, ctx->cs_clear);
      details::set_srt(cb, ctx->cs_clear, &srt);

      details::dispatch_indirect(
         cb,
         indirect_mem[k_tile_type_clear],
         ctx->cs_clear->m_specials->m_dispatchModifier);
   }

   details::cs_sync(cb);
}

void ssr_generate(SsrContext* ctx, sce::Agc::AsyncCommandBuffer* acb, const SsrRuntimeOption& options)
{
   return ssr_generate<sce::Agc::AsyncCommandBuffer>(ctx, acb, options);
}

void ssr_generate(SsrContext* ctx, sce::Agc::DrawCommandBuffer* dcb, const SsrRuntimeOption& options)
{
   return ssr_generate<sce::Agc::DrawCommandBuffer>(ctx, dcb, options);
}

template<typename T>
void ssr_reproject(SsrContext* ctx, T* cb, const SsrReprojectRuntimeOption& options)
{
   details::MarkerAutoScope<T> marker_reproject(cb, "psfx ssr reproject");

   ReprojectSrt* srt = details::alloc_top_down<ReprojectSrt>(cb);
   srt->reflection      = options.source_reflection;
   srt->mask            = options.source_mask;
   srt->velocity        = options.velocity;
   srt->dest_reflection = options.dest_reflection;
   srt->dest_mask       = options.dest_mask;
   srt->inv_dest_size.x = 1.0f / static_cast<float>(options.dest_width);
   srt->inv_dest_size.y = 1.0f / static_cast<float>(options.dest_height);

   details::set_compute_shader(cb, ctx->cs_reproject);
   details::set_srt(cb, ctx->cs_reproject, &srt);

   cb->dispatch(
      details::round_div(options.dest_width, k_reproject_group_size),
      details::round_div(options.dest_height, k_reproject_group_size),
      1,
      ctx->cs_reproject->m_specials->m_dispatchModifier);

   details::cs_sync(cb);
}

void ssr_reproject(SsrContext* ctx, sce::Agc::AsyncCommandBuffer* acb, const SsrReprojectRuntimeOption& options)
{
   return ssr_reproject<sce::Agc::AsyncCommandBuffer>(ctx, acb, options);
}

void ssr_reproject(SsrContext* ctx, sce::Agc::DrawCommandBuffer* dcb, const SsrReprojectRuntimeOption& options)
{
   return ssr_reproject<sce::Agc::DrawCommandBuffer>(ctx, dcb, options);
}

}
