/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#include "psfx_common.h"
#include "psfx_exposure.h"

#include "psfx_details.h"

#include <cmath>

// TODO:
// can we speed the atomic writing using GDS instead of a buffer?

namespace psfx
{
   extern char       exposure_shader_count_header[];
   extern const char exposure_shader_count_text[];
   extern char       exposure_shader_count_MS_header[];
   extern const char exposure_shader_count_MS_text[];
   extern char       exposure_shader_gather_header[];
   extern const char exposure_shader_gather_text[];
   extern char       set_uint_header[];
   extern const char set_uint_text[];
   extern char       exposure_shader_draw_header[];
   extern const char exposure_shader_draw_text[];
 
static const uint32_t k_bucket_count = 128;
static const uint32_t k_pixel_per_wave_dimension = 16;

struct HistogramConstantBuffer
{
   sce::Agc::Core::Texture source;
   sce::Agc::Core::Buffer buckets;
};

struct DrawConstantBuffer
{
   sce::Agc::Core::Buffer buckets;
   sce::Agc::Core::Buffer exposure_stats;
};

struct ExposureContext
{
   sce::Agc::ResourceRegistration::OwnerHandle owner_handle;

   details::ComputeShader* clear_cs;
   details::ComputeShader* count_cs;
   details::ComputeShader* count_ms_cs;
   details::ComputeShader* gather_cs;

   details::ComputeShader* draw_cs;

   sce::Agc::Core::Buffer buckets_data_buffer;
   sce::Agc::Core::Buffer buckets_byte_buffer;
};

void exposure_configuration_init(ExposureConfiguration& conf)
{
   memset(&conf, 0, sizeof(ExposureConfiguration));
}

int exposure_compute_memory_requirement(ExposureConfiguration const& conf, MemoryRequirement& memory_req)
{
   sce::Agc::SizeAlign mem_size_align { 0, sce::Agc::Alignment::kShaderCode };
   mem_size_align.m_size += sizeof( ExposureContext );

   mem_size_align.m_size = details::align_to(mem_size_align.m_size, sce::Agc::Alignment::kBuffer);

   uint32_t buffer_size = k_bucket_count * sizeof(uint32_t);
   mem_size_align.m_size += buffer_size;

   memory_req.mem_size = mem_size_align;

   return SCE_OK;
}

ExposureContext* exposure_initialize(ExposureConfiguration const& conf, MemoryPool const& memory_pool)
{
   if (memory_pool.mem_ptr == nullptr)
   {
      SCE_DBG_LOG_ERROR("mem_ptr can't be NULL.");
      return nullptr;
   }

   details::ArenaAllocator mem_arena(memory_pool.mem_ptr);

   ExposureContext* ctx = mem_arena.alloc<ExposureContext>();

   SceError err = sce::Agc::ResourceRegistration::registerOwner(&ctx->owner_handle, "psfx exposure");
   if (err != SCE_OK)
      ctx->owner_handle = sce::Agc::ResourceRegistration::kInvalidOwnerHandle;

   ctx->clear_cs    = details::make_cs(ctx->owner_handle, "psfx exposure clear", psfx::set_uint_header, psfx::set_uint_text);
   ctx->count_cs    = details::make_cs(ctx->owner_handle, "psfx exposure count", psfx::exposure_shader_count_header, psfx::exposure_shader_count_text);
   ctx->count_ms_cs = details::make_cs(ctx->owner_handle, "psfx exposure count ms", psfx::exposure_shader_count_MS_header, psfx::exposure_shader_count_MS_text);
   ctx->gather_cs   = details::make_cs(ctx->owner_handle, "psfx exposure gather", psfx::exposure_shader_gather_header, psfx::exposure_shader_gather_text);
   ctx->draw_cs     = details::make_cs(ctx->owner_handle, "psfx exposure draw", psfx::exposure_shader_draw_header, psfx::exposure_shader_draw_text);

   uint32_t buffer_size = sizeof(uint32_t) * k_bucket_count;

   ctx->buckets_byte_buffer.init();
   sce::Agc::Core::BufferSpec spec;
   spec.initAsByteBuffer( mem_arena.alloc( buffer_size, sce::Agc::Alignment::kBuffer ), buffer_size );
   sce::Agc::Core::initialize( &ctx->buckets_byte_buffer, &spec );

   spec.initAsDataBuffer( ctx->buckets_byte_buffer.getDataAddress(), { sce::Agc::Core::TypedFormat::k32UInt , sce::Agc::Core::Swizzle::kR000_S1 } ,k_bucket_count );
   sce::Agc::Core::initialize( &ctx->buckets_data_buffer, &spec );

   return ctx;
}

void exposure_terminate(ExposureContext* ctx)
{
   if (ctx == nullptr)
   {
      SCE_DBG_LOG_ERROR("Trying to terminate a NULL context.");
      return;
   }
   if (ctx->owner_handle != sce::Agc::ResourceRegistration::kInvalidOwnerHandle)
      sce::Agc::ResourceRegistration::unregisterOwnerAndResources(ctx->owner_handle);
}

#if 0
TODO Add the SOS and SDS and support for physical camera parameters
float getSaturationBasedExposure(float aperture,
                                 float shutterSpeed,
                                 float iso)
{
   float l_max = (7800.0f / 65.0f) * Sqr(aperture) / (iso * shutterSpeed);
   return 1.0f / l_max;
}

/*
* Get an exposure using the Standard Output Sensitivity method.
* Accepts an additional parameter of the target middle grey.
*/
float getStandardOutputBasedExposure(float aperture,
                                     float shutterSpeed,
                                     float iso,
                                     float middleGrey = 0.18f)
{
   float l_avg = (1000.0f / 65.0f) * Sqr(aperture) / (iso * shutterSpeed);
   return middleGrey / l_avg;
}
#endif

template<typename T>
void exposure_apply(ExposureContext* ctx, T* dcb, ExposureRuntimeOption const& options)
{
   details::MarkerAutoScope<T> marker(dcb, "psfx exposure");

   HistogramConstantBuffer* constant_buffer = details::alloc_top_down<HistogramConstantBuffer>(dcb);

   // TODO: Put these in the runtime options
   const float pre_exposure = options.pre_exposure;
   const float ev_min = options.ev_min;
   const float ev_max = details::max_of(ev_min, options.ev_max);
   const float adaptation_speed_up = options.adaptation_speed_up;
   const float adaptation_speed_down = options.adaptation_speed_down;

   constant_buffer->source  = options.source;
   constant_buffer->buckets = ctx->buckets_byte_buffer;

   struct ClearParams
   {
      sce::Agc::Core::Buffer dest;
      uint32_t value;
      uint32_t size_minus_one;
   } clear_param;
   clear_param.dest = ctx->buckets_data_buffer;
   clear_param.value = 0;
   clear_param.size_minus_one = 127;

   details::set_compute_shader( dcb , ctx->clear_cs );
   details::set_srt( dcb, ctx->clear_cs, ( void* ) &clear_param );
   dcb->dispatch(details::round_div(k_bucket_count, 64), 1, 1, ctx->clear_cs->m_specials->m_dispatchModifier);

   details::cs_sync(dcb);

   struct ShaderParams
   {
      HistogramConstantBuffer* scale_user_data;
      float tx_inv_size[2];
      float ev_min;
      float ev_max;
      float ev_adjust;
      uint32_t offset_x;
      uint32_t offset_y;
   } shader_params;

   const uint32_t wave_x = options.source_width /  k_pixel_per_wave_dimension;
   const uint32_t wave_y = options.source_height / k_pixel_per_wave_dimension;

   shader_params.tx_inv_size[0] = 1.f / static_cast<float>(options.source_width);
   shader_params.tx_inv_size[1] = 1.f / static_cast<float>(options.source_height);
   shader_params.scale_user_data = constant_buffer;
   shader_params.ev_min = ev_min;
   shader_params.ev_max = ev_max;
   shader_params.ev_adjust = log2(options.white_point * pre_exposure);
   // Align the buffer as multiple of 32.
   // Discard the border as usual don't contribute to the average of the image
   // Pro: the shader is simpler because it doesn't have to check the border of the render target
   // Cons: each wavefront will read a partial tile
   uint32_t off_x = (options.source_width  - (wave_x * k_pixel_per_wave_dimension)) / 2;
   uint32_t off_y = (options.source_height - (wave_y * k_pixel_per_wave_dimension)) / 2;
   shader_params.offset_x = off_x;
   shader_params.offset_y = off_y;

   if (options.source.getNumFragments() == sce::Agc::Core::Texture::NumFragments::k1)
   {
      details::set_compute_shader( dcb, ctx->count_cs );
      details::set_srt( dcb, ctx->count_cs, ( void* ) &shader_params );
      dcb->dispatch( wave_x, wave_y, 1 , ctx->count_cs->m_specials->m_dispatchModifier);

   }
   else
   {
      details::set_compute_shader( dcb, ctx->count_ms_cs );
      details::set_srt( dcb, ctx->count_ms_cs, ( void* ) &shader_params );
      dcb->dispatch( wave_x, wave_y, 1, ctx->count_ms_cs->m_specials->m_dispatchModifier );
   }

   details::cs_sync(dcb);

   struct GatherParams
   {
      sce::Agc::Core::Buffer buckets;
      sce::Agc::Core::Buffer exposure;
      float ev_min;
      float ev_range;
      float ev_target;
      float up_speed;
      float down_speed;
      float low_threshold;
      float hi_threshold;
   } gather_param;

   gather_param.buckets = ctx->buckets_data_buffer;
   gather_param.exposure = options.destination;

   gather_param.ev_min = ev_min;
   gather_param.ev_range = details::max_of(ev_max - ev_min, 0.f);
   if (gather_param.ev_range == 0.f)
   {
      gather_param.ev_range = 1.f;
   }
   const float ISO_speed = 100;
   const float K = 12.5f; // reflected light meter calibration constant (Canon and Nikon use 12.5)
   gather_param.ev_target = log2(options.gray_point * ISO_speed / K);
   gather_param.up_speed = 1.f / (1.f + adaptation_speed_up);
   gather_param.down_speed = 1.f / (1.f + adaptation_speed_down);

   gather_param.low_threshold   = options.low_threshold;
   gather_param.hi_threshold = options.high_threshold;

   details::set_compute_shader( dcb, ctx->gather_cs );
   details::set_srt( dcb, ctx->gather_cs, ( void* ) &gather_param );
   dcb->dispatch( 1, 1, 1, ctx->gather_cs->m_specials->m_dispatchModifier );

   details::cs_sync(dcb);
}

void exposure_apply(ExposureContext* ctx, sce::Agc::DrawCommandBuffer* dcb, ExposureRuntimeOption const& options)
{
   return exposure_apply<sce::Agc::DrawCommandBuffer>(ctx,dcb,options);
}

void exposure_apply(ExposureContext* ctx, sce::Agc::AsyncCommandBuffer* dcb, ExposureRuntimeOption const& options)
{
   return exposure_apply<sce::Agc::AsyncCommandBuffer>(ctx, dcb, options);
}

template<typename T>
void exposure_draw_histogram(ExposureContext* ctx, T* dcb, sce::Agc::Core::Texture const& dest, sce::Agc::Core::Buffer const& exposure_buffer)
{
   details::MarkerAutoScope<T> marker(dcb, "Exposure Histogram");

   struct DrawParams
   {
      sce::Agc::Core::Texture dest;
      DrawConstantBuffer* buffers;
      uint32_t x, y, w, h;
      float scale;
      float alpha;
   } draw_param;

   DrawConstantBuffer* constant_buffer = details::alloc_top_down<DrawConstantBuffer>(dcb);

   draw_param.dest = dest;
   draw_param.buffers = constant_buffer;
   constant_buffer->buckets = ctx->buckets_data_buffer;
   constant_buffer->exposure_stats = exposure_buffer;

   const uint32_t height = dest.getHeight();
   const uint32_t border = 0;
   const uint32_t h_width = 320;
   const uint32_t h_height = 240;
   draw_param.x = border;
   draw_param.y = height - h_height - border;
   draw_param.w = h_width;
   draw_param.h = h_height;
   draw_param.scale = 6.f;
   draw_param.alpha = 1.f;

   details::set_compute_shader( dcb, ctx->draw_cs );
   details::set_srt( dcb, ctx->draw_cs, ( void* ) &draw_param );

   const uint32_t wave_x = details::round_div(draw_param.w,  8);
   const uint32_t wave_y = details::round_div(draw_param.h, 8);
   dcb->dispatch( wave_x, wave_y, 1, ctx->draw_cs->m_specials->m_dispatchModifier);
}

void exposure_draw_histogram(ExposureContext* ctx, sce::Agc::DrawCommandBuffer* dcb, sce::Agc::Core::Texture const& dest, sce::Agc::Core::Buffer const& exposure_buffer)
{
   return exposure_draw_histogram<sce::Agc::DrawCommandBuffer>(ctx, dcb, dest, exposure_buffer);
}

void exposure_draw_histogram(ExposureContext* ctx, sce::Agc::AsyncCommandBuffer* dcb, sce::Agc::Core::Texture const& dest, sce::Agc::Core::Buffer const& exposure_buffer)
{
   return exposure_draw_histogram<sce::Agc::AsyncCommandBuffer>(ctx, dcb, dest,exposure_buffer);
}

}
