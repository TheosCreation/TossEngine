/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#include "psfx_common.h"
#include "psfx_linearize_depth.h"
#include "psfx_ssao.h"

#include "psfx_details.h"

#include <vectormath.h>
#include <utility>

namespace psfx
{

static const sce::Agc::Core::DataFormat k_intermediate_format = { sce::Agc::Core::TypedFormat::k8UNorm, sce::Agc::Core::Swizzle::kR000_S1 };
  
extern char       ssao_prepare_depth_downscale_header[];
extern const char ssao_prepare_depth_downscale_text[];
extern char       ssao_prepare_depth_downscale_interleave_header[];
extern const char ssao_prepare_depth_downscale_interleave_text[];
extern char       ssao_hbao_header[];
extern const char ssao_hbao_text[];
extern char       ssao_hbao_interleaved_header[];
extern const char ssao_hbao_interleaved_text[];
extern char       ssao_blur_h_header[];
extern const char ssao_blur_h_text[];
extern char       ssao_blur_v_header[];
extern const char ssao_blur_v_text[];
extern char       ssao_upsample_header[];
extern const char ssao_upsample_text[];
extern char       ssao_upsample_coarse_header[];
extern const char ssao_upsample_coarse_text[];
extern char       ssao_upsample_coarse_fine_header[];
extern const char ssao_upsample_coarse_fine_text[];
extern char       ssao_upsample_fine_header[];
extern const char ssao_upsample_fine_text[];

//////////////

namespace details
{

using namespace sce::Shader;

struct SsaoAOTextureParam
{
   sce::Agc::Core::Texture input;
   sce::Agc::Core::Texture output;
};

struct SsaoDownscaleDepthShaderParam
{
   sce::Agc::Core::Texture out_depth;
   sce::Agc::Core::Texture out_interleaved_depth;
};

struct SsaoBlurTxRef
{
   sce::Agc::Core::Texture interleaved_ao;
   sce::Agc::Core::Texture linear_depth;
};

struct SsaoUpscaleShaderTxRef
{
   sce::Agc::Core::Texture coarse_depth;
   sce::Agc::Core::Texture fine_depth;

   sce::Agc::Core::Texture coarse_0_ao;
   sce::Agc::Core::Texture coarse_1_ao;
   sce::Agc::Core::Texture fine_ao;

   sce::Agc::Core::Texture result;
};

/////////////

} // details

struct SsaoContext
{
   SsaoConfiguration conf;

   float* w_table;

   static constexpr uint k_max_steps = 4;
   uint32_t scale_steps;

   sce::Agc::Core::Texture intermediate_ao_tx[k_max_steps];             // R8Unorm
   sce::Agc::Core::Texture intermediate_interleaved_ao_tx[k_max_steps]; // R8Unorm
   sce::Agc::Core::Texture ao_temp_tx[k_max_steps];                     // R8Unorm
   sce::Agc::Core::Texture linear_depth[k_max_steps];                   // R16Unorm linear depth
   sce::Agc::Core::Texture linear_interleaved_depth[k_max_steps];       // R16Unorm x 16 linear depth quarter res

   uint8_t valid_textures[4];

   details::ComputeShader* ssao_hbao_cs;
   details::ComputeShader* ssao_hbao_interleaved_cs;

   details::ComputeShader* ssao_blur_h_cs;
   details::ComputeShader* ssao_blur_v_cs;

   details::ComputeShader* prepare_depth_downscale_cs;
   details::ComputeShader* prepare_depth_downscale_interleave_cs;

   details::ComputeShader* ao_upsample;
   details::ComputeShader* ao_upsample_coarse;
   details::ComputeShader* ao_upsample_fine;
   details::ComputeShader* ao_upsample_coarse_fine;

   sce::Agc::ResourceRegistration::OwnerHandle owner_handle;
};

void ssao_configuration_init(SsaoConfiguration& conf)
{
   memset(&conf, 0, sizeof(SsaoConfiguration));
}

int ssao_compute_memory_requirement(SsaoConfiguration const& conf, MemoryRequirement& memory_pool)
{
   if (conf.max_target_width * conf.max_target_height == 0)
   {
      SCE_DBG_LOG_ERROR("The target dimensions are zero.");
      return -1;
   }

   sce::Agc::SizeAlign mem_size_align { 0, 8 };
   mem_size_align.m_size += details::align_to(sizeof( SsaoContext), sce::Agc::Alignment::kBuffer );
 
   const uint32_t intermediate_steps = details::min_of(SsaoContext::k_max_steps, conf.downsample_steps);
   const uint32_t half_res_w = conf.max_target_width / 2;
   const uint32_t half_res_h = conf.max_target_height / 2;
   {  // Intermediate AO result textures
      sce::Agc::Core::TextureSpec tx_specs[SsaoContext::k_max_steps];


      details::make_tx_descriptors(half_res_w, half_res_h, 1, k_linear_depth_format, tx_specs, intermediate_steps);
      details::tx_memory_usage(tx_specs, intermediate_steps, mem_size_align);

      if (conf.use_interleaved)
      {
         details::tx_memory_usage(tx_specs, intermediate_steps, mem_size_align); // intermediate result
      }
      details::tx_memory_usage(tx_specs, intermediate_steps, mem_size_align); // temp
   }
   {  // Downscale depth
      sce::Agc::Core::TextureSpec depth_specs[SsaoContext::k_max_steps];

      details::make_tx_descriptors(half_res_w, half_res_h, 1, k_linear_depth_format, depth_specs, intermediate_steps);
      details::tx_memory_usage(depth_specs, intermediate_steps, mem_size_align);

      if (conf.use_interleaved)
      {
         details::make_tx_descriptors(half_res_w / 4, half_res_h / 4, 16, k_linear_depth_format, depth_specs, intermediate_steps);
         details::tx_memory_usage(depth_specs, intermediate_steps, mem_size_align);
      }
   }
   {  // Constant buffers
      mem_size_align.m_size += details::align_to(sizeof(float) * 12, sce::Agc::Alignment::kBuffer);
   }
   memory_pool.mem_size = mem_size_align;

   return SCE_OK;
}

SsaoContext* ssao_initialize(SsaoConfiguration const& conf, MemoryPool const& memory_pool)
{
   if (memory_pool.mem_ptr == nullptr)
   {
      SCE_DBG_LOG_ERROR("mem_ptr can't be NULL.");
      return nullptr;
   }

   details::ArenaAllocator mem_arena(memory_pool.mem_ptr);
   SsaoContext* ctx = mem_arena.alloc<SsaoContext>( sce::Agc::Alignment::kBuffer );
   memset(ctx, 0, sizeof(SsaoContext));
   ctx->conf = conf;

   SceError err =  sce::Agc::ResourceRegistration::registerOwner(&ctx->owner_handle, "psfx ssao");
   if (err != SCE_OK)
      ctx->owner_handle = sce::Agc::ResourceRegistration::kInvalidOwnerHandle;

   ctx->scale_steps = details::min_of(SsaoContext::k_max_steps, conf.downsample_steps);

   ctx->ssao_hbao_cs             = details::make_cs(ctx->owner_handle, "psfx ssao hbao", psfx::ssao_hbao_header, psfx::ssao_hbao_text);
   ctx->ssao_hbao_interleaved_cs = details::make_cs(ctx->owner_handle, "psfx ssao hbao interleaved", psfx::ssao_hbao_interleaved_header, psfx::ssao_hbao_interleaved_text);

   ctx->ssao_blur_h_cs  = details::make_cs(ctx->owner_handle, "psfx ssao blur h", psfx::ssao_blur_h_header, psfx::ssao_blur_h_text);
   ctx->ssao_blur_v_cs  = details::make_cs(ctx->owner_handle, "psfx ssao blur v", psfx::ssao_blur_v_header, psfx::ssao_blur_v_text);

   ctx->prepare_depth_downscale_cs            = details::make_cs(ctx->owner_handle, "psfx ssao depth downscale", psfx::ssao_prepare_depth_downscale_header, psfx::ssao_prepare_depth_downscale_text);
   ctx->prepare_depth_downscale_interleave_cs = details::make_cs(ctx->owner_handle, "psfx ssao depth downscale interleaved", psfx::ssao_prepare_depth_downscale_interleave_header, psfx::ssao_prepare_depth_downscale_interleave_text);

   ctx->ao_upsample             = details::make_cs(ctx->owner_handle, "psfx ssao upsample",             psfx::ssao_upsample_header, psfx::ssao_upsample_text);
   ctx->ao_upsample_coarse      = details::make_cs(ctx->owner_handle, "psfx ssao upsample coarse", psfx::ssao_upsample_coarse_header, psfx::ssao_upsample_coarse_text);
   ctx->ao_upsample_fine        = details::make_cs(ctx->owner_handle, "psfx ssao upsample fine", psfx::ssao_upsample_fine_header, psfx::ssao_upsample_fine_text);
   ctx->ao_upsample_coarse_fine = details::make_cs(ctx->owner_handle, "psfx ssao upsample coarse fine", psfx::ssao_upsample_coarse_fine_header, psfx::ssao_upsample_coarse_fine_text);

   //// Texture
   uint32_t half_res_w = conf.max_target_width / 2;
   uint32_t half_res_h = conf.max_target_height / 2;
   sce::Agc::Core::TextureSpec tx_specs[SsaoContext::k_max_steps];

   {  // intermediate textures
      details::make_tx_descriptors(half_res_w, half_res_h, 1, k_intermediate_format, tx_specs, ctx->scale_steps);
      details::init_tx_chain(ctx->owner_handle, "Ao", ctx->intermediate_ao_tx, tx_specs, ctx->scale_steps, mem_arena.arena);
      ctx->valid_textures[BufferSemantic::k_ao] = ctx->scale_steps;
      if (conf.use_interleaved)
      {
         details::init_tx_chain(ctx->owner_handle, "Ao interleaved", ctx->intermediate_interleaved_ao_tx, tx_specs, ctx->scale_steps, mem_arena.arena);
         ctx->valid_textures[BufferSemantic::k_ao_interleaved] = ctx->scale_steps;
      }
      details::init_tx_chain(ctx->owner_handle, "ao temp", ctx->ao_temp_tx, tx_specs, ctx->scale_steps, mem_arena.arena);
   }

   {  // Downscale depth
      details::make_tx_descriptors(half_res_w, half_res_h, 1, k_linear_depth_format, tx_specs, ctx->scale_steps);
      details::init_tx_chain(ctx->owner_handle, "linear depth", ctx->linear_depth, tx_specs, ctx->scale_steps, mem_arena.arena);
      ctx->valid_textures[BufferSemantic::k_linear_depth] = ctx->scale_steps;
      if (conf.use_interleaved)
      {

         details::make_tx_descriptors(half_res_w / 4, half_res_h / 4, 16, k_linear_depth_format, tx_specs, ctx->scale_steps);
         details::init_tx_chain(ctx->owner_handle, "Linear depth interleaved", ctx->linear_interleaved_depth, tx_specs, ctx->scale_steps, mem_arena.arena);
         ctx->valid_textures[BufferSemantic::k_linear_interleaved_depth] = ctx->scale_steps;
      }
   }

   {  // Shader params
      ctx->w_table = mem_arena.alloc<float>(sce::Agc::Alignment::kBuffer, 12);

      struct float3 { float x, y, z; };
      // z represent how many sample with that weight we compute together
      // check psfx_ssao_hbao_c.pssl for details.
      float3 s_pos[] = {
         // These call horizon2
         { 0.2f, 0.0f, 2.f },
         { 0.4f, 0.0f, 2.f },
         { 0.6f, 0.0f, 2.f },
         { 0.8f, 0.0f, 2.f },
         { 0.2f, 0.2f, 2.f },
         { 0.3f, 0.4f, 2.f },
         { 0.6f, 0.6f, 2.f },
         // These use horizon4
         { 0.2f, 0.4f, 4.f },
         { 0.2f, 0.6f, 4.f },
         { 0.2f, 0.8f, 4.f },
         { 0.4f, 0.6f, 4.f },
         { 0.4f, 0.8f, 4.f }
      };
      float w_table_sum = 0.f;
      for (int i = 0; i < 12; ++i)
      {
         ctx->w_table[i] = sqrt(1.f - (s_pos[i].x * s_pos[i].x + s_pos[i].y * s_pos[i].y) );
         w_table_sum += ctx->w_table[i];
      }
      for (int i = 0; i < 12; ++i)
      {
         ctx->w_table[i] /= (w_table_sum * s_pos[i].z);
      }
   }

   return ctx;
}

static void resize_targets(SsaoContext* ctx, uint32_t width, uint32_t height)
{
   uint32_t half_res_w = width / 2;
   uint32_t half_res_h = height / 2;



   {  // intermediate textures
      details::resize_tx_chain(half_res_w, half_res_h, ctx->intermediate_ao_tx, ctx->scale_steps);
      
      if (ctx->conf.use_interleaved)
      {
         details::resize_tx_chain(half_res_w, half_res_h, ctx->intermediate_interleaved_ao_tx, ctx->scale_steps);
      }

      details::resize_tx_chain(half_res_w, half_res_h, ctx->ao_temp_tx, ctx->scale_steps);
   }

   {  // Downscale depth
      details::resize_tx_chain(half_res_w, half_res_h, ctx->linear_depth, ctx->scale_steps);

      if (ctx->conf.use_interleaved)
      {
         details::resize_tx_chain(half_res_w / 4, half_res_h / 4, ctx->linear_interleaved_depth, ctx->scale_steps);
      }
   }
}

void ssao_terminate(SsaoContext* ctx)
{
   if (ctx == nullptr)
   {
      SCE_DBG_LOG_ERROR("Trying to terminate a NULL context.");
      return;
   }

   if (ctx->owner_handle != sce::Agc::ResourceRegistration::kInvalidOwnerHandle)
      sce::Agc::ResourceRegistration::unregisterOwnerAndResources(ctx->owner_handle);
}

sce::Agc::Core::Texture* ssao_get_buffer(SsaoContext* ctx, BufferSemantic semantic, uint32_t index)
{
   //if (index > SsaoContext::k_max_steps)
   if (index >= ctx->valid_textures[semantic])
      return nullptr;
   switch(semantic)
   {
   case BufferSemantic::k_linear_depth:
      return ctx->linear_depth + index;
   case BufferSemantic::k_linear_interleaved_depth:
      return ctx->linear_interleaved_depth + index;
   case BufferSemantic::k_ao:
      return ctx->intermediate_ao_tx + index;
   case BufferSemantic::k_ao_interleaved:
      return ctx->intermediate_interleaved_ao_tx + index;
   case BufferSemantic::k_nothing:
      return nullptr;
   }
   return nullptr;
}

details::ComputeShader* get_depth_shader(SsaoContext* ctx, bool interleaved)
{
   if (interleaved)
   {
      return ctx->prepare_depth_downscale_interleave_cs;
   }
   else
   {
      return ctx->prepare_depth_downscale_cs;
   }
}

struct ViewSize
{
   uint32_t w, h;
};

template<typename T>
static void ssao_prepare_depth(SsaoContext* ctx, T* dcb, SsaoRuntimeOption const& options, ViewSize* view_size)
{
   struct ShaderParam
   {
      sce::Agc::Core::Texture    input;
      details::SsaoDownscaleDepthShaderParam* out_tx;
      float inv_source_size[2];
      float near_Inv_range[2];
   };

   details::MarkerAutoScope<T> marker(dcb, "prepare depth");

   const uint32_t depth_downscale_steps = ctx->scale_steps;
   const bool write_interleaved = ctx->conf.use_interleaved;
   details::SsaoDownscaleDepthShaderParam* scale_user_data = details::alloc_top_down<details::SsaoDownscaleDepthShaderParam>(dcb, depth_downscale_steps);

   sce::Agc::Core::Texture input_tx = options.source_linear_depth;
   uint32_t src_width  = input_tx.getWidth();
   uint32_t src_height = input_tx.getHeight();

   const float near_plane = options.near_plane_distance;
   const float far_plane  = options.far_plane_distance;

   ShaderParam str;
   str.near_Inv_range[0] = near_plane;
   str.near_Inv_range[1] = 1.f / (far_plane - near_plane);

   details::ComputeShader* step_cs = get_depth_shader(ctx, write_interleaved);
   details::set_compute_shader( dcb, step_cs );

   for (int step = 0; step < depth_downscale_steps; ++step)
   {
      uint32_t active_area_width  = details::max_of(1u, (src_width + 1) >> 1);
      uint32_t active_area_height = details::max_of(1u, (src_height + 1) >> 1);

      str.input  = input_tx;
      str.out_tx = scale_user_data;

      scale_user_data->out_depth = ctx->linear_depth[step];
 
      view_size[step].w = active_area_width;
      view_size[step].h = active_area_height;
      if (write_interleaved)
      {
         scale_user_data->out_interleaved_depth = ctx->linear_interleaved_depth[step];
      }

      str.inv_source_size[0] = 1.f / (float)src_width;
      str.inv_source_size[1] = 1.f / (float)src_height;
      details::set_srt( dcb, step_cs, ( void* ) &str );
      const uint32_t pixel_per_wave_dimension = 8 * 2;
      const uint32_t wave_x = details::round_div(active_area_width, pixel_per_wave_dimension);
      const uint32_t wave_y = details::round_div(active_area_height, pixel_per_wave_dimension);
      dcb->dispatch(wave_x, wave_y, 1, step_cs->m_specials->m_dispatchModifier);

      // Sync-point
      //TODO: Split dispatch in two with two sync-points to reduce GPU bubbles
      details::cs_sync(dcb);

      input_tx = scale_user_data->out_depth;
      scale_user_data++;
      src_width = active_area_width;
      src_height = active_area_height;
   }
}

template <typename T>
static void ssao_compute_hbao(SsaoContext* ctx, T* dcb, SsaoRuntimeOption const& options, ViewSize const* view_size)
{
   struct ShaderParams
   {
      details::SsaoAOTextureParam* textures;
      sce::Agc::Core::Buffer w_table;
      float tx_inv_size[2];
      float inv_diameter;
      float reject_falloff;
      float distance_scale;
   };


   details::MarkerAutoScope<T> marker(dcb, "compute hbao");

   const uint32_t steps = ctx->scale_steps;
   const uint32_t ssao_steps = ctx->conf.use_interleaved ? steps * 2 : steps;
   details::SsaoAOTextureParam* user_data_tx = details::alloc_top_down<details::SsaoAOTextureParam>(dcb, ssao_steps);

   ShaderParams shader_params;
   const float distance_range = options.far_plane_distance - options.near_plane_distance;

   const float k_sphere_diameter_px = 10.f;
   sce::Agc::Core::BufferSpec spec;
   spec.initAsDataBuffer( ctx->w_table, { sce::Agc::Core::TypedFormat::k32Float, sce::Agc::Core::Swizzle::kR100_S1 }, 12 );
   sce::Agc::Core::initialize( &shader_params.w_table, &spec );
   
   const float falloff_param = -1.f / (1.f + options.reject_falloff);
   if (ctx->conf.use_interleaved)
   {
      details::MarkerAutoScope<T> interleaved_marker(dcb, "interleaved hbao");

      details::set_compute_shader( dcb, ctx->ssao_hbao_interleaved_cs );
      for (uint32_t step = 0; step < steps; ++step)
      {
         user_data_tx->input  = ctx->linear_interleaved_depth[step];
         user_data_tx->output = ctx->intermediate_interleaved_ao_tx[step];
         
         shader_params.textures = user_data_tx;

         const uint32_t quater_res_w = ctx->linear_interleaved_depth[step].getWidth();//details::max_of(view_size[step].w / 4, 1u);
         const uint32_t quater_res_h = ctx->linear_interleaved_depth[step].getHeight();//details::max_of(view_size[step].h / 4, 1u);

         float w = static_cast<float>(quater_res_w);
         float h = static_cast<float>(quater_res_h);
         shader_params.tx_inv_size[0] = 1.f / w;
         shader_params.tx_inv_size[1] = 1.f / h;
         const float pixel_size = 2.f * options.fov_x / w;
         shader_params.inv_diameter = 1.f / (pixel_size * k_sphere_diameter_px);
         shader_params.reject_falloff = falloff_param;
         shader_params.distance_scale = distance_range;

         details::set_srt( dcb, ctx->ssao_hbao_interleaved_cs , ( void* ) &shader_params );

         const uint32_t pixel_per_wave_dimension = 8;
         const uint32_t wave_x = details::round_div(quater_res_w, pixel_per_wave_dimension);
         const uint32_t wave_y = details::round_div(quater_res_h, pixel_per_wave_dimension);
         dcb->dispatch(wave_x, wave_y, 16, ctx->ssao_hbao_interleaved_cs->m_specials->m_dispatchModifier);
         ++user_data_tx;
      }
   }

   details::MarkerAutoScope<T> hbao_marker(dcb, "hbao");

   details::set_compute_shader( dcb, ctx->ssao_hbao_cs );
   for (uint32_t step = 0; step < steps; ++step)
   {
      user_data_tx->input  = ctx->linear_depth[step];
      user_data_tx->output = ctx->intermediate_ao_tx[step];
      shader_params.textures = user_data_tx;
      float w = static_cast<float>(view_size[step].w);
      float h = static_cast<float>(view_size[step].h);
      shader_params.tx_inv_size[0] = 1.f / w;
      shader_params.tx_inv_size[1] = 1.f / h;
      const float pixel_size = 2.f * options.fov_x / w;
      shader_params.inv_diameter = 1.f / (pixel_size * k_sphere_diameter_px);
      shader_params.reject_falloff = falloff_param;
      shader_params.distance_scale = distance_range;

      details::set_srt( dcb, ctx->ssao_hbao_cs , ( void* ) &shader_params );
      
      const uint32_t pixel_per_wave_dimension = 8;
      const uint32_t wave_x = details::round_div(view_size[step].w,  pixel_per_wave_dimension);
      const uint32_t wave_y = details::round_div(view_size[step].h, pixel_per_wave_dimension);
      dcb->dispatch(wave_x, wave_y, 1, ctx->ssao_hbao_cs->m_specials->m_dispatchModifier);
      ++user_data_tx;
   }

   details::cs_sync(dcb);
}

template<typename T>
static void ssao_blur_interleaved(SsaoContext* ctx, T* dcb, SsaoRuntimeOption const& options, ViewSize const* view_size)
{
   details::MarkerAutoScope<T> marker(dcb, "blur");

   const uint32_t steps = ctx->scale_steps;
   details::SsaoBlurTxRef* blur_user_data = details::alloc_top_down<details::SsaoBlurTxRef>(dcb, steps * 2);

   struct ShaderParam
   {
      sce::Agc::Core::Texture out_tx;
      details::SsaoBlurTxRef* tx_ref;
      float inv_source_size[2];
      float depth_scale;
   };

   float depth_scale = powf(10.f, options.log_blur_scale) * (options.far_plane_distance - options.near_plane_distance);
   details::SyncResult synch_point[SsaoContext::k_max_steps];
   // Horizontal blur
   details::set_compute_shader( dcb, ctx->ssao_blur_h_cs );
   for (uint32_t step = 0; step < steps; ++step)
   {
      blur_user_data->linear_depth   = ctx->linear_depth[step];
      blur_user_data->interleaved_ao = ctx->intermediate_interleaved_ao_tx[step];

      ShaderParam param;
      param.tx_ref = blur_user_data;

      param.inv_source_size[0] = 1.f / static_cast<float>(view_size[step].w);
      param.inv_source_size[1] = 1.f / static_cast<float>(view_size[step].h);
      param.depth_scale = depth_scale / static_cast<float>(1 << step);

      param.out_tx = ctx->ao_temp_tx[step];
      details::set_srt( dcb, ctx->ssao_blur_h_cs, (void*)&param );
      const uint32_t pixel_per_wave_dimension = 8;
      const uint32_t wave_x = details::round_div(view_size[step].w, pixel_per_wave_dimension);
      const uint32_t wave_y = details::round_div(view_size[step].h, pixel_per_wave_dimension);
      dcb->dispatch(wave_x, wave_y, 1, ctx->ssao_blur_h_cs->m_specials->m_dispatchModifier);

      synch_point[step] = details::write_cs_sync(dcb);
      blur_user_data++;
   }
   // Vertical blur
   details::set_compute_shader( dcb, ctx->ssao_blur_v_cs );
   for (uint32_t step = 0; step < steps; ++step)
   {
      blur_user_data->linear_depth   = ctx->linear_depth[step];
      blur_user_data->interleaved_ao = ctx->ao_temp_tx[step];

      ShaderParam param;
      param.tx_ref = blur_user_data;

      param.inv_source_size[0] = 1.f / static_cast<float>(view_size[step].w);
      param.inv_source_size[1] = 1.f / static_cast<float>(view_size[step].h);

      param.depth_scale = depth_scale / static_cast<float>(1 << step);

      param.out_tx = ctx->intermediate_interleaved_ao_tx[step];

      details::set_srt( dcb, ctx->ssao_blur_v_cs, (void*)&param );

      const uint32_t pixel_per_wave_dimension = 8;
      const uint32_t wave_x = details::round_div(view_size[step].w, pixel_per_wave_dimension);
      const uint32_t wave_y = details::round_div(view_size[step].h, pixel_per_wave_dimension);

      details::wait_cs_sync(dcb, synch_point[step]);
      dcb->dispatch(wave_x, wave_y, 1, ctx->ssao_blur_v_cs->m_specials->m_dispatchModifier);
      blur_user_data++;
   }
}

template<typename T>
static void upsample(SsaoContext* ctx, T* dcb, SsaoRuntimeOption const& options, ViewSize const* view_sizes)
{
   details::MarkerAutoScope<T> marker(dcb, "upsample");

   details::cs_sync(dcb);
   uint32_t step_count = ctx->scale_steps;
   details::SsaoUpscaleShaderTxRef* tx_ref = details::alloc_top_down<details::SsaoUpscaleShaderTxRef>(dcb, step_count);

   struct ShaderParam
   {
      details::SsaoUpscaleShaderTxRef* tx_ref;
      float inv_coarse_resolution[2];
      float inv_fine_resolution[2];
      float upscale_threshold;
   } shader_params;
   shader_params.upscale_threshold = options.upscale_threshold;
   details::ComputeShader* selected_shader = nullptr;
   if (ctx->conf.use_interleaved)
   {
      tx_ref->coarse_0_ao = ctx->intermediate_interleaved_ao_tx[step_count - 1];
      details::set_compute_shader( dcb, ctx->ao_upsample_coarse_fine );
      selected_shader = ctx->ao_upsample_coarse_fine;
   }
   else
   {
      tx_ref->coarse_0_ao = ctx->intermediate_ao_tx[step_count - 1];
      details::set_compute_shader( dcb, ctx->ao_upsample_fine );
      selected_shader = ctx->ao_upsample_fine;
   }

   for (int step_idx = step_count - 1; step_idx > 0; --step_idx)
   {
      tx_ref->coarse_depth   = ctx->linear_depth[step_idx];
      tx_ref->fine_depth     = ctx->linear_depth[step_idx - 1];
      if (ctx->conf.use_interleaved)
      {
         tx_ref->coarse_1_ao = ctx->intermediate_ao_tx[step_idx];
         tx_ref->fine_ao = ctx->intermediate_interleaved_ao_tx[step_idx - 1];
      }
      else
      {
         tx_ref->fine_ao = ctx->intermediate_ao_tx[step_idx - 1];
      }
      uint32_t view_w = view_sizes[step_idx - 1].w;
      uint32_t view_h = view_sizes[step_idx - 1].h;

      tx_ref->result = ctx->ao_temp_tx[step_idx - 1];

      shader_params.tx_ref = tx_ref;
      shader_params.inv_coarse_resolution[0] = 1.0f / (float)tx_ref->coarse_depth.getWidth();
      shader_params.inv_coarse_resolution[1] = 1.0f / (float)tx_ref->coarse_depth.getHeight();
      shader_params.inv_fine_resolution[0]   = 1.0f / (float)tx_ref->fine_depth.getWidth();
      shader_params.inv_fine_resolution[1]   = 1.0f / (float)tx_ref->fine_depth.getHeight();
      details::set_srt( dcb, selected_shader, ( void* ) &shader_params );
      dcb->dispatch(details::round_div(view_w + 2, 16), details::round_div(view_h + 2, 16), 1, selected_shader->m_specials->m_dispatchModifier);

      tx_ref++;
      tx_ref->coarse_0_ao      = ctx->ao_temp_tx[step_idx - 1];

      details::cs_sync(dcb);
   }
   // Step 0
   if (ctx->conf.use_interleaved)
   {
      details::set_compute_shader( dcb, ctx->ao_upsample_coarse );
      selected_shader = ctx->ao_upsample_coarse;
   }
   else
   {
      details::set_compute_shader( dcb, ctx->ao_upsample );
      selected_shader = ctx->ao_upsample;
   }
   tx_ref->coarse_depth   = ctx->linear_depth[0];
   tx_ref->fine_depth     = options.source_linear_depth;
   if (ctx->conf.use_interleaved)
   {
      tx_ref->coarse_1_ao = ctx->intermediate_ao_tx[0];
   }

   tx_ref->result = options.destination;

   shader_params.tx_ref = tx_ref;
   shader_params.inv_coarse_resolution[0]  = 1.0f / (float)tx_ref->coarse_depth.getWidth();
   shader_params.inv_coarse_resolution[1]  = 1.0f / (float)tx_ref->coarse_depth.getHeight();
   shader_params.inv_fine_resolution[0]    = 1.0f / (float)tx_ref->fine_depth.getWidth();
   shader_params.inv_fine_resolution[1]    = 1.0f / (float)tx_ref->fine_depth.getHeight();
   shader_params.upscale_threshold = options.upscale_threshold;

   details::set_srt( dcb, selected_shader, ( void* ) &shader_params );
   dcb->dispatch(details::round_div(view_sizes[0].w * 2 + 2, 16), details::round_div(view_sizes[0].h * 2 + 2, 16), 1, selected_shader->m_specials->m_dispatchModifier);

   details::cs_sync(dcb);
}

template<typename T> 
void ssao_apply(SsaoContext* ctx, T* dcb, SsaoRuntimeOption const& options, volatile sce::Agc::Label* finished)
{

   ctx->scale_steps = ctx->conf.downsample_steps;
   if (!ctx->conf.use_interleaved && ctx->conf.dynamic_resolution)
   {
      float res_ratio = (float)options.target_height / (float)ctx->conf.max_target_height;
      int new_step = details::round(ctx->conf.downsample_steps * res_ratio);
      new_step = details::max_of<float>(1, new_step);
      new_step = details::min_of<float>(ctx->k_max_steps, new_step);
      ctx->scale_steps = new_step;
   }


   uint32_t half_res_w = options.target_width / 2;
   uint32_t half_res_h = options.target_height / 2;
   if (half_res_w != ctx->intermediate_ao_tx[0].getWidth() ||
       half_res_h != ctx->intermediate_ao_tx[0].getHeight())
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

   details::MarkerAutoScope<T> marker(dcb, "psfx ssao");

   ViewSize views[SsaoContext::k_max_steps];
   ssao_prepare_depth(ctx, dcb, options, views);
   ssao_compute_hbao(ctx, dcb, options, views);
   if (ctx->conf.use_interleaved)
   {
      ssao_blur_interleaved(ctx, dcb, options, views);
   }
   upsample(ctx, dcb, options, views);

   if (finished)
   {
      details::write_label(dcb, finished);
   }
}

void ssao_apply(SsaoContext* ctx, sce::Agc::DrawCommandBuffer* dcb, SsaoRuntimeOption const& options, volatile sce::Agc::Label* finished)
{
   return ssao_apply<sce::Agc::DrawCommandBuffer>(ctx, dcb, options, finished);
}

void ssao_apply(SsaoContext* ctx, sce::Agc::AsyncCommandBuffer* dcb, SsaoRuntimeOption const& options, volatile sce::Agc::Label* finished)
{
   return ssao_apply<sce::Agc::AsyncCommandBuffer>(ctx, dcb, options, finished);
}

}
