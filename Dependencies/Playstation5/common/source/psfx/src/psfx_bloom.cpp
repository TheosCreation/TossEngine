/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#include "psfx_common.h"
#include "psfx_bloom.h"

#include "psfx_details.h"


namespace psfx
{
   extern char       bloom_downscale_header[];
   extern const char bloom_downscale_text[];
   extern char       bloom_downscale_threshold_header[];
   extern const char bloom_downscale_threshold_text[];
   extern char       bloom_downscale_threshold_no_flicker_header[];
   extern const char bloom_downscale_threshold_no_flicker_text[];
   extern char       bloom_upscale_header[];
   extern const char bloom_upscale_text[];

struct DownscaleShaderTx
{
   sce::Agc::Core::Texture src;
   sce::Agc::Core::Texture dst;
};

struct UpscaleShaderTx
{
   sce::Agc::Core::Texture high_res;
   sce::Agc::Core::Texture low_res;
   sce::Agc::Core::Texture dst;
};

struct BloomContext
{
   static const int k_max_blur_steps = 5;
   int blur_steps = k_max_blur_steps;
   BloomConfiguration conf;

   sce::Agc::ResourceRegistration::OwnerHandle owner_handle;

   details::ComputeShader* bloom_downscale_threshold_no_flicker_cs;
   details::ComputeShader* bloom_downscale_threshold_cs;
   details::ComputeShader* bloom_downscale_cs;
   details::ComputeShader* bloom_upscale_cs;

   sce::Agc::Core::Texture blur_tx[k_max_blur_steps];
   sce::Agc::Core::Texture intermediate_blur_tx[k_max_blur_steps];
};

static const sce::Agc::Core::DataFormat k_tx_format = { sce::Agc::Core::TypedFormat::k11_11_10Float, sce::Agc::Core::Swizzle::kRGB0_S34 };

void bloom_configuration_init(BloomConfiguration& conf)
{
   details::set_to_zero(conf);
}

int bloom_compute_memory_requirement(BloomConfiguration const& conf, MemoryRequirement& memory_req)
{
   if ((conf.max_target_width * conf.max_target_height) == 0)
   {
      SCE_DBG_LOG_ERROR("zero area render target, aborting.");
      return -1;
   }
 
   sce::Agc::SizeAlign gpu_size_align { 0, sce::Agc::Alignment::kShaderCode };
   gpu_size_align.m_size = sizeof(BloomContext);

   sce::Agc::Core::TextureSpec tx_specs[BloomContext::k_max_blur_steps];

   details::make_tx_descriptors(conf.max_target_width / 2, conf.max_target_height / 2, 1, k_tx_format, tx_specs, BloomContext::k_max_blur_steps);
   details::tx_memory_usage(tx_specs, BloomContext::k_max_blur_steps, gpu_size_align);

   // Intermediate targets share memory (see bloom_initialize()).
   details::make_tx_descriptors(conf.max_target_width, conf.max_target_height, 1, k_tx_format, tx_specs, 2);
   details::tx_memory_usage(tx_specs, 2, gpu_size_align);

   memory_req.mem_size = gpu_size_align;

   return SCE_OK;
}

BloomContext* bloom_initialize(BloomConfiguration const& conf, MemoryPool const& memory_pool)
{
   details::ArenaAllocator mem_arena(memory_pool.mem_ptr);

   BloomContext* ctx = mem_arena.alloc<BloomContext>();
   memset(ctx, 0, sizeof(BloomContext));
   ctx->conf = conf;

   SceError err = sce::Agc::ResourceRegistration::registerOwner(&ctx->owner_handle, "psfx bloom");
   if (err != SCE_OK)
      ctx->owner_handle = sce::Agc::ResourceRegistration::kInvalidOwnerHandle;

   ctx->bloom_downscale_threshold_cs = details::make_cs(ctx->owner_handle, "psfx bloom downscale threshold", psfx::bloom_downscale_threshold_header, psfx::bloom_downscale_threshold_text);
   ctx->bloom_downscale_threshold_no_flicker_cs = details::make_cs(ctx->owner_handle, "psfx bloom downscale threshold no flicker", psfx::bloom_downscale_threshold_no_flicker_header, psfx::bloom_downscale_threshold_no_flicker_text);
   ctx->bloom_downscale_cs = details::make_cs(ctx->owner_handle, "psfx bloom downscale", psfx::bloom_downscale_header, psfx::bloom_downscale_text);
   ctx->bloom_upscale_cs   = details::make_cs(ctx->owner_handle, "psfx bloom upsample", psfx::bloom_upscale_header, psfx::bloom_upscale_text);

   sce::Agc::Core::TextureSpec tx_specs[BloomContext::k_max_blur_steps];
   details::make_tx_descriptors(conf.max_target_width / 2, conf.max_target_height / 2, 1, k_tx_format, tx_specs, BloomContext::k_max_blur_steps);
   details::init_tx_chain(ctx->owner_handle, "psfx downsampled", ctx->blur_tx, tx_specs, BloomContext::k_max_blur_steps, mem_arena.arena);

   details::make_tx_descriptors(conf.max_target_width, conf.max_target_height, 1, k_tx_format, tx_specs, BloomContext::k_max_blur_steps);
   {
      // intermediate texture share memory
      void* tx_memory[2] = {nullptr, nullptr};
      char const* tx_name[2] = {"bloom intermediate_0", "bloom intermediate_1"};
      for (int step = 0; step < 2; step++)
      {
         tx_specs[step].m_allowNullptr = true;
         sce::Agc::Core::initialize(&ctx->intermediate_blur_tx[step] , &tx_specs[step]);
         auto mem_size = sce::Agc::Core::getSize(&tx_specs[step]);
         tx_memory[step] = mem_arena.alloc(mem_size.m_size, mem_size.m_align);
         ctx->intermediate_blur_tx[step].setDataAddress(tx_memory[step]);
         if (ctx->owner_handle != sce::Agc::ResourceRegistration::kInvalidOwnerHandle)
         {
            sce::Agc::ResourceRegistration::ResourceHandle handle;
            sce::Agc::ResourceRegistration::registerResource(&handle, ctx->owner_handle, tx_memory[step], mem_size.m_size, tx_name[step], sce::Agc::ResourceRegistration::ResourceType::kTextureBaseAddress, 0);
         }
      }
      for (int step = 2; step < BloomContext::k_max_blur_steps; step++)
      {
         tx_specs[step].m_allowNullptr = true;
         sce::Agc::Core::initialize(&ctx->intermediate_blur_tx[step], &tx_specs[step]);
         ctx->intermediate_blur_tx[step].setDataAddress(tx_memory[step % 2]);
      }
   }

   return ctx;
}

static void resize_targets(BloomContext* ctx, uint32_t width, uint32_t height)
{
   details::resize_tx_chain(width / 2, height / 2, ctx->blur_tx, BloomContext::k_max_blur_steps);
   details::resize_tx_chain(width, height, ctx->intermediate_blur_tx, BloomContext::k_max_blur_steps);
}

void bloom_terminate(BloomContext* ctx)
{
   if (ctx == nullptr)
   {
      SCE_DBG_LOG_ERROR("Trying to terminate a NULL context.");
      return;
   }
   if (ctx->owner_handle != sce::Agc::ResourceRegistration::kInvalidOwnerHandle)
      sce::Agc::ResourceRegistration::unregisterOwnerAndResources(ctx->owner_handle);
}

struct StepScaling
{
   uint32_t src_w, src_h;
   uint32_t dst_w, dst_h;
};

template <typename T>
static void down_scale(BloomContext* ctx, T* dcb, BloomRuntimeOption const& options, StepScaling scaling[])
{
   details::MarkerAutoScope<T> marker(dcb, "downscale");

   DownscaleShaderTx* downscale_params_tx = details::alloc_top_down<DownscaleShaderTx>(dcb, ctx->blur_steps);

   struct ShaderParams
   {
      DownscaleShaderTx* tx;
      sce::Agc::Core::Buffer exposure;
      float tx_scale_x;
      float tx_scale_y;
      float intensity;
      float threshold;
   };

   details::ComputeShader* set_shader = nullptr;
   details::ComputeShader* current_shader = nullptr;
   if (options.anti_fireflies)
   {
      current_shader = ctx->bloom_downscale_threshold_no_flicker_cs;
   }
   else
   {
      current_shader = ctx->bloom_downscale_threshold_cs;
   }

   ShaderParams downscale_params;
   downscale_params_tx->src = options.source;
   downscale_params.intensity = options.intensity;
   downscale_params.threshold = options.threshold;
   sce::Agc::Core::BufferSpec downscale_params_specs;
   downscale_params_specs.m_format = { sce::Agc::Core::TypedFormat::k32Float, sce::Agc::Core::Swizzle::kR100_S1 };
   downscale_params_specs.initAsDataBuffer( options.exposure, { sce::Agc::Core::TypedFormat::k32Float, sce::Agc::Core::Swizzle::kR100_S1 }, 1 );
   sce::Agc::Core::initialize( &downscale_params.exposure, &downscale_params_specs );

   scaling[0].src_w = options.source.getWidth();
   scaling[0].src_h = options.source.getHeight();

   for (uint32_t pass = 0; pass < ctx->blur_steps; pass++)
   {
      if (set_shader != current_shader)
      {
         details::set_compute_shader( dcb, current_shader );
         
         set_shader = current_shader;
      }
      scaling[pass].dst_w = details::max_of(1U, scaling[pass].src_w >> 1);
      scaling[pass].dst_h = details::max_of(1U, scaling[pass].src_h >> 1);
      scaling[pass].dst_h&1  ?  : scaling[pass].dst_h++;
      const uint32_t k_pixel_per_wave_dimension = 8;
      const uint32_t wave_x = details::round_div(scaling[pass].dst_w, k_pixel_per_wave_dimension);
      const uint32_t wave_y = details::round_div(scaling[pass].dst_h, k_pixel_per_wave_dimension);

      downscale_params_tx->dst = ctx->blur_tx[pass];

      downscale_params.tx = downscale_params_tx;

      // Assuming the first image is fullscreen
      // We can consider a sub space
      float ratio_x = (float)scaling[pass].src_w / (float)downscale_params_tx->src.getWidth();
      float ratio_y = (float)scaling[pass].src_h / (float)downscale_params_tx->src.getHeight();
      downscale_params.tx_scale_x = ratio_x / (float)scaling[pass].dst_w;
      downscale_params.tx_scale_y = ratio_y / (float)scaling[pass].dst_h;
      details::set_srt( dcb, current_shader, ( void* ) &downscale_params );

      dcb->dispatch(wave_x, wave_y, 1, current_shader->m_specials->m_dispatchModifier);

      details::cs_sync(dcb);

      if (pass != ctx->blur_steps - 1)
      {
         sce::Agc::Core::Texture temp = downscale_params_tx->dst;
         downscale_params_tx++;
         downscale_params_tx->src = temp;
         scaling[pass + 1].src_w = scaling[pass].dst_w;
         scaling[pass + 1].src_h = scaling[pass].dst_h;
      }
      downscale_params.intensity = 1.f;
      downscale_params.threshold = 0.f;
      current_shader = ctx->bloom_downscale_cs;
   }
}

template <typename T>
static void upscale_scale(BloomContext* ctx, T* dcb, BloomRuntimeOption const& options, StepScaling scaling[])
{
   details::MarkerAutoScope<T> marker(dcb, "upscale");

   details::set_compute_shader( dcb, ctx->bloom_upscale_cs );

   UpscaleShaderTx* upscale_params_tx = details::alloc_top_down<UpscaleShaderTx>(dcb, ctx->blur_steps);

   struct UpscaleShaderParams
   {
      UpscaleShaderTx* tx;
      float tx_scale_x;
      float tx_scale_y;
      float weight_0;
      float weight_1;
   };

   UpscaleShaderParams params;

   upscale_params_tx->low_res = ctx->blur_tx[ctx->blur_steps - 1];

   params.weight_0 = 0.5f;
   params.weight_1 = 0.5f;
   for (uint32_t pass = 0; pass < ctx->blur_steps; pass++)
   {
      if (pass != ctx->blur_steps - 1)
      {
         upscale_params_tx->high_res = ctx->blur_tx[ctx->blur_steps - 2 - pass];
         upscale_params_tx->dst = ctx->intermediate_blur_tx[ctx->blur_steps - 1 - pass];
      }
      else
      {
         params.weight_0 = 1.0f;
         params.weight_1 = 0.5f;
         // This naively assume that source and destination are the same size.
         upscale_params_tx->high_res = options.source;
         upscale_params_tx->dst = options.destination;
      }
      const uint32_t dst_width  = scaling[ctx->blur_steps - pass - 1].src_w;
      const uint32_t dst_height = scaling[ctx->blur_steps - pass - 1].src_h;

      const uint32_t k_pixel_per_wave_dimension = 8;
      const uint32_t wave_x = details::round_div(dst_width,  k_pixel_per_wave_dimension);
      const uint32_t wave_y = details::round_div(dst_height, k_pixel_per_wave_dimension);

      params.tx = upscale_params_tx;
      params.tx_scale_x = 0.5f / static_cast<float>(upscale_params_tx->low_res.getWidth());
      params.tx_scale_y = 0.5f / static_cast<float>(upscale_params_tx->low_res.getHeight());

      details::set_srt( dcb, ctx->bloom_upscale_cs, ( void* ) &params );
      dcb->dispatch( wave_x, wave_y, 1 , ctx->bloom_upscale_cs->m_specials->m_dispatchModifier);

      details::cs_sync(dcb);

      if (pass != ctx->blur_steps - 1)
      {
         sce::Agc::Core::Texture old_dst = upscale_params_tx->dst;
         upscale_params_tx++;
         upscale_params_tx->low_res = old_dst;
      }
   }
}

template<typename T>
void bloom_apply(BloomContext* ctx, T* dcb, BloomRuntimeOption const& options)
{
   ctx->blur_steps = BloomContext::k_max_blur_steps;
   if (ctx->conf.dynamic_resolution)
   {
      float res_ratio = (float)options.target_height / (float)ctx->conf.max_target_height;
      int new_step = ctx->k_max_blur_steps * res_ratio;
      new_step = details::max_of<float>(2, new_step);
      new_step = details::min_of<float >(ctx->k_max_blur_steps, new_step);
      ctx->blur_steps = new_step;
   }


   if (options.target_width != ctx->intermediate_blur_tx[0].getWidth() ||
       options.target_height != ctx->intermediate_blur_tx[0].getHeight())
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

   details::MarkerAutoScope<T> marker(dcb, "psfx bloom");

   StepScaling rt_sizes[BloomContext::k_max_blur_steps];
   down_scale(ctx, dcb, options, rt_sizes);
   upscale_scale(ctx, dcb, options, rt_sizes);
}

void bloom_apply(BloomContext* ctx, sce::Agc::DrawCommandBuffer* dcb, BloomRuntimeOption const& options)
{
   return bloom_apply<sce::Agc::DrawCommandBuffer>(ctx, dcb, options);
}

void bloom_apply(BloomContext* ctx, sce::Agc::AsyncCommandBuffer* dcb, BloomRuntimeOption const& options)
{
   return bloom_apply<sce::Agc::AsyncCommandBuffer>(ctx, dcb, options);
}

}
