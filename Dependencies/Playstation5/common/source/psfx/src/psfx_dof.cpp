/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#include "psfx_common.h"
#include "psfx_dof.h"

#include "psfx_details.h"

#include <math.h>

namespace psfx
{
   extern char       dof_compute_coc_header[];
   extern const char dof_compute_coc_text[];
   extern char       dof_max_near_header[];
   extern const char dof_max_near_text[];
   extern char       dof_downscale_header[];
   extern const char dof_downscale_text[];
   extern char       dof_blur_near_header[];
   extern const char dof_blur_near_text[];
   extern char       dof_blur_far_header[];
   extern const char dof_blur_far_text[];
   extern char       dof_blur_near_two_header[];
   extern const char dof_blur_near_two_text[];
   extern char       dof_blur_far_two_header[];
   extern const char dof_blur_far_two_text[];

   extern char      dof_blur_near_half_header[];
   extern const char dof_blur_near_half_text[];
   extern char       dof_blur_far_half_header[];
   extern const char dof_blur_far_half_text[];
   extern char       dof_blur_near_two_half_header[];
   extern const char dof_blur_near_two_half_text[];
   extern char       dof_blur_far_two_half_header[];
   extern const char dof_blur_far_two_half_text[];


   extern char      dof_blur_near_quatr_header[];
   extern const char dof_blur_near_quatr_text[];
   extern char       dof_blur_far_quatr_header[];
   extern const char dof_blur_far_quatr_text[];
   extern char       dof_blur_near_two_quatr_header[];
   extern const char dof_blur_near_two_quatr_text[];
   extern char       dof_blur_far_two_quatr_header[];
   extern const char dof_blur_far_two_quatr_text[];

   extern char       dof_composite_header[];
   extern const char dof_composite_text[];

namespace details
{
 
struct DofCocShaderTextures
{
   sce::Agc::Core::Texture linear_depth;
   sce::Agc::Core::Texture coc;
};

struct DofDownscaleTextures
{
   sce::Agc::Core::Texture input;
   sce::Agc::Core::Texture coc;
};

struct DofDownscaleResults
{
   sce::Agc::Core::Texture out_depth;
   sce::Agc::Core::Texture out_color0;
   sce::Agc::Core::Texture out_color1;
};

struct DofCocTexture
{
   sce::Agc::Core::Texture input;
   sce::Agc::Core::Texture output;
};


struct DofBlurShaderParamFar
{
   sce::Agc::Core::Texture input0;
   sce::Agc::Core::Texture depth;
   sce::Agc::Core::Texture out_color0;
   sce::Agc::Core::Texture out_color1;
};

struct DofBlurShaderParamFarSecond
{
   sce::Agc::Core::Texture input0;
   sce::Agc::Core::Texture input1;
   sce::Agc::Core::Texture depth;
   sce::Agc::Core::Texture out_color0;
   sce::Agc::Core::Texture out_alpha;
};

struct DofBlurShaderParamNear
{
   sce::Agc::Core::Texture input0;
   sce::Agc::Core::Texture depth;
   sce::Agc::Core::Texture coc;
   sce::Agc::Core::Texture out_color0;
   sce::Agc::Core::Texture out_color1;
};

struct DofBlurShaderParamNearSecond
{
   sce::Agc::Core::Texture input0;
   sce::Agc::Core::Texture input1;

   sce::Agc::Core::Texture depth;
   sce::Agc::Core::Texture coc;
   sce::Agc::Core::Texture out_color0;
   sce::Agc::Core::Texture out_alpha;
};

struct DofCompositeTextures
{
   sce::Agc::Core::Texture inputTextNear;
   sce::Agc::Core::Texture inputTextFar;
   sce::Agc::Core::Texture inputTextAlpha;
   sce::Agc::Core::Texture inputTextDepth;

   sce::Agc::Core::Texture outputText;
};

} // details

struct DofContext
{
   DofConfiguration conf;

   sce::Agc::ResourceRegistration::OwnerHandle owner_handle;

   details::ComputeShader* dof_compute_coc_cs;
   details::ComputeShader* dof_max_coc_cs;
   details::ComputeShader* dof_circular_separable_near_one_cs;
   details::ComputeShader* dof_circular_separable_far_one_cs;
   details::ComputeShader* dof_circular_separable_near_two_cs;
   details::ComputeShader* dof_circular_separable_far_two_cs;
   details::ComputeShader* dof_circular_separable_near_one_half_cs;
   details::ComputeShader* dof_circular_separable_far_one_half_cs;
   details::ComputeShader* dof_circular_separable_near_two_half_cs;
   details::ComputeShader* dof_circular_separable_far_two_half_cs;
   details::ComputeShader* dof_circular_separable_near_one_quatr_cs;
   details::ComputeShader* dof_circular_separable_far_one_quatr_cs;
   details::ComputeShader* dof_circular_separable_near_two_quatr_cs;
   details::ComputeShader* dof_circular_separable_far_two_quatr_cs;
   details::ComputeShader* dof_composite_cs;
   details::ComputeShader* dof_downscale_cs;

   sce::Agc::Core::Texture coc_full;
   sce::Agc::Core::Texture coc_x4;
   sce::Agc::Core::Texture coc_nearx8;

   sce::Agc::Core::Texture near_x4;
   sce::Agc::Core::Texture far_x4;
   sce::Agc::Core::Texture near_x4_merged;
   sce::Agc::Core::Texture far_x4_merged;
   sce::Agc::Core::Texture alphas;
   sce::Agc::Core::Texture near_0;
   sce::Agc::Core::Texture near_1;
   sce::Agc::Core::Texture far_0;
   sce::Agc::Core::Texture far_1;
};

void dof_configuration_init(DofConfiguration& conf)
{
   details::set_to_zero(conf);
}

int dof_compute_memory_requirement(DofConfiguration const& conf, MemoryRequirement& memory_req)
{
   sce::Agc::SizeAlign mem_size_align { 0, sce::Agc::Alignment::kShaderCode };

   mem_size_align.m_size += sizeof( DofContext );

   sce::Agc::Core::TextureSpec tx_specs;
   tx_specs.init();

   //depth 2f16 floats foreground and backgorund
   details::make_tx_descriptors(conf.max_target_width, conf.max_target_height, 1, { sce::Agc::Core::TypedFormat::k8_8UNorm , sce::Agc::Core::Swizzle::kRG01_R2S24 }, &tx_specs, 1);
   details::tx_memory_usage(&tx_specs, 1, mem_size_align);

   details::make_tx_descriptors(conf.max_target_width / 8, conf.max_target_height / 8, 1, { sce::Agc::Core::TypedFormat::k8UNorm , sce::Agc::Core::Swizzle::kR001_R1S1 }, &tx_specs, 1);
   details::tx_memory_usage(&tx_specs, 1, mem_size_align);

   const uint16_t width_quarter_res = conf.max_target_width / 4;
   const uint16_t height_quarter_res = conf.max_target_height / 4;

   //downscaled Coc max
   details::make_tx_descriptors(width_quarter_res, height_quarter_res, 1, { sce::Agc::Core::TypedFormat::k8_8UNorm , sce::Agc::Core::Swizzle::kRG01_R2S24 }, &tx_specs, 1);
   details::tx_memory_usage(&tx_specs, 1, mem_size_align);

   //2 rgb  (foreground background)
   details::make_tx_descriptors(width_quarter_res, height_quarter_res, 1, { sce::Agc::Core::TypedFormat::k16_16_16_16Float , sce::Agc::Core::Swizzle::kRGBA_R4S4 }, &tx_specs, 1);
   details::tx_memory_usage(&tx_specs, 1, mem_size_align);
   details::tx_memory_usage(&tx_specs, 1, mem_size_align);

   // and the downscaled ones
   details::make_tx_descriptors(width_quarter_res, height_quarter_res, 1, { sce::Agc::Core::TypedFormat::k16_16_16_16Float , sce::Agc::Core::Swizzle::kRGBA_R4S4 }, &tx_specs, 1);
   details::tx_memory_usage(&tx_specs, 1, mem_size_align);
   details::tx_memory_usage(&tx_specs, 1, mem_size_align);

   // thats blur
   details::make_tx_descriptors(width_quarter_res, height_quarter_res, 1, { sce::Agc::Core::TypedFormat::k16_16_16_16Float , sce::Agc::Core::Swizzle::kRGBA_R4S4 }, &tx_specs, 1);
   details::tx_memory_usage(&tx_specs, 1, mem_size_align);
   details::tx_memory_usage(&tx_specs, 1, mem_size_align);

   details::make_tx_descriptors(width_quarter_res, height_quarter_res, 1, { sce::Agc::Core::TypedFormat::k11_11_10Float , sce::Agc::Core::Swizzle::kRGB0_S34 }, &tx_specs, 1);
   details::tx_memory_usage(&tx_specs, 1, mem_size_align);
   details::tx_memory_usage(&tx_specs, 1, mem_size_align);

   details::make_tx_descriptors(conf.max_target_width, conf.max_target_height, 1, { sce::Agc::Core::TypedFormat::k16_16Float , sce::Agc::Core::Swizzle::kRG01_R2S24 }, &tx_specs, 1);
   details::tx_memory_usage(&tx_specs, 1, mem_size_align);

   memory_req.mem_size = mem_size_align;

   return SCE_OK;
}

DofContext* dof_initialize(DofConfiguration const& conf, MemoryPool const& memory_pool)
{
   details::ArenaAllocator mem_arena(memory_pool.mem_ptr);

   DofContext* ctx = mem_arena.alloc<DofContext>();
   memset(ctx, 0, sizeof(*ctx));
   ctx->conf = conf;

   SceError err =  sce::Agc::ResourceRegistration::registerOwner(&ctx->owner_handle, "psfx dof");
   if (err != SCE_OK)
      ctx->owner_handle = sce::Agc::ResourceRegistration::kInvalidOwnerHandle;

   ctx->dof_compute_coc_cs   = details::make_cs(ctx->owner_handle, "psfx dof compute coc", psfx::dof_compute_coc_header, psfx::dof_compute_coc_text);
   ctx->dof_max_coc_cs       = details::make_cs(ctx->owner_handle, "psfx dof max coc", psfx::dof_max_near_header, psfx::dof_max_near_text);
   ctx->dof_downscale_cs     = details::make_cs(ctx->owner_handle, "psfx dof downscale", psfx::dof_downscale_header, psfx::dof_downscale_text);
   ctx->dof_circular_separable_near_one_cs = details::make_cs(ctx->owner_handle, "psfx dof blur near", psfx::dof_blur_near_header, dof_blur_near_text);
   ctx->dof_circular_separable_far_one_cs = details::make_cs(ctx->owner_handle, "psfx dof blur far", psfx::dof_blur_far_header, psfx::dof_blur_far_text);

   ctx->dof_circular_separable_near_two_cs = details::make_cs(ctx->owner_handle, "psfx dof blur near two", psfx::dof_blur_near_two_header, psfx::dof_blur_near_two_text);
   ctx->dof_circular_separable_far_two_cs = details::make_cs(ctx->owner_handle, "psfx dof blur far two", psfx::dof_blur_far_two_header, psfx::dof_blur_far_two_text);


   ctx->dof_circular_separable_near_one_half_cs = details::make_cs(ctx->owner_handle, "psfx dof blur near half rad", psfx::dof_blur_near_half_header, psfx::dof_blur_near_half_text);
   ctx->dof_circular_separable_far_one_half_cs = details::make_cs(ctx->owner_handle, "psfx dof blur far half rad", psfx::dof_blur_far_half_header, psfx::dof_blur_far_half_text);

   ctx->dof_circular_separable_near_two_half_cs = details::make_cs(ctx->owner_handle, "psfx dof blur near two half rad", psfx::dof_blur_near_two_half_header, psfx::dof_blur_near_two_half_text);
   ctx->dof_circular_separable_far_two_half_cs = details::make_cs(ctx->owner_handle, "psfx dof blur far two half rad", psfx::dof_blur_far_two_half_header, psfx::dof_blur_far_two_half_text);



   ctx->dof_circular_separable_near_one_quatr_cs = details::make_cs(ctx->owner_handle, "psfx dof blur near half rad", psfx::dof_blur_near_quatr_header, dof_blur_near_quatr_text);
   ctx->dof_circular_separable_far_one_quatr_cs = details::make_cs(ctx->owner_handle, "psfx dof blur far half rad", psfx::dof_blur_far_quatr_header, psfx::dof_blur_far_quatr_text);

   ctx->dof_circular_separable_near_two_quatr_cs = details::make_cs(ctx->owner_handle, "psfx dof blur near two quater rad", psfx::dof_blur_near_two_quatr_header, psfx::dof_blur_near_two_quatr_text);
   ctx->dof_circular_separable_far_two_quatr_cs = details::make_cs(ctx->owner_handle, "psfx dof blur far two quater rad", psfx::dof_blur_far_two_quatr_header, psfx::dof_blur_far_two_quatr_text);

   ctx->dof_composite_cs = details::make_cs(ctx->owner_handle, "psfx dof composite", psfx::dof_composite_header, psfx::dof_composite_text);

   const uint16_t width_quarter_res  = conf.max_target_width / 4;
   const uint16_t height_quarter_res = conf.max_target_height / 4;

   sce::Agc::Core::TextureSpec tx_specs;

   details::make_tx_descriptors(conf.max_target_width, conf.max_target_height, 1, { sce::Agc::Core::TypedFormat::k8_8UNorm , sce::Agc::Core::Swizzle::kRG01_R2S24 }, &tx_specs, 1);
   details::init_tx_chain(ctx->owner_handle, "psfx CoC", &ctx->coc_full, &tx_specs, 1, mem_arena.arena);

   details::make_tx_descriptors(width_quarter_res, height_quarter_res, 1, { sce::Agc::Core::TypedFormat::k8_8UNorm , sce::Agc::Core::Swizzle::kRG01_R2S24 }, &tx_specs, 1);
   details::init_tx_chain(ctx->owner_handle, "psfx CoC quarter", &ctx->coc_x4, &tx_specs, 1, mem_arena.arena);

   details::make_tx_descriptors(width_quarter_res, height_quarter_res, 1, { sce::Agc::Core::TypedFormat::k16_16_16_16Float , sce::Agc::Core::Swizzle::kRGBA_R4S4 }, &tx_specs, 1);
   details::init_tx_chain(ctx->owner_handle, "psfx dof near Q", &ctx->near_x4, &tx_specs, 1, mem_arena.arena);
   details::init_tx_chain(ctx->owner_handle, "psfx dof far Q", &ctx->far_x4, &tx_specs, 1, mem_arena.arena);
   details::init_tx_chain(ctx->owner_handle, "psfx dof near0", &ctx->near_0, &tx_specs, 1, mem_arena.arena);
   details::init_tx_chain(ctx->owner_handle, "psfx dof near1", &ctx->near_1, &tx_specs, 1, mem_arena.arena);
   details::init_tx_chain(ctx->owner_handle, "psfx dof far0", &ctx->far_0, &tx_specs, 1, mem_arena.arena);
   details::init_tx_chain(ctx->owner_handle, "psfx dof far1", &ctx->far_1, &tx_specs, 1, mem_arena.arena);

   details::make_tx_descriptors(width_quarter_res / 2, height_quarter_res / 2, 1, { sce::Agc::Core::TypedFormat::k8UNorm , sce::Agc::Core::Swizzle::kR001_R1S1 }, &tx_specs, 1);
   details::init_tx_chain(ctx->owner_handle, "psfx Coc near eight", &ctx->coc_nearx8, &tx_specs, 1, mem_arena.arena);
   details::make_tx_descriptors(width_quarter_res, height_quarter_res, 1, { sce::Agc::Core::TypedFormat::k11_11_10Float , sce::Agc::Core::Swizzle::kRGB0_S34 }, &tx_specs, 1);
   details::init_tx_chain(ctx->owner_handle, "psfx dof near Merged", &ctx->near_x4_merged, &tx_specs, 1, mem_arena.arena);
   details::init_tx_chain(ctx->owner_handle, "psfx dof far Merged", &ctx->far_x4_merged, &tx_specs, 1, mem_arena.arena);
   details::make_tx_descriptors(width_quarter_res , height_quarter_res , 1, { sce::Agc::Core::TypedFormat::k16_16Float , sce::Agc::Core::Swizzle::kRG01_R2S24 }, &tx_specs, 1);
   details::init_tx_chain(ctx->owner_handle, "psfx dof Alpha channels", &ctx->alphas, &tx_specs, 1, mem_arena.arena);

   return ctx;
}

static void resize_targets(DofContext* ctx, uint32_t width, uint32_t height)
{
   details::resize_tx(width, height, 1, ctx->coc_full);

   const uint16_t width_quarter_res = width / 4;
   const uint16_t height_quarter_res = height / 4;

   details::resize_tx(width_quarter_res, height_quarter_res, 1, ctx->coc_x4);
   details::resize_tx(width_quarter_res, height_quarter_res, 1, ctx->near_x4);
   details::resize_tx(width_quarter_res, height_quarter_res, 1, ctx->far_x4);
   details::resize_tx(width_quarter_res, height_quarter_res, 1, ctx->near_0);
   details::resize_tx(width_quarter_res, height_quarter_res, 1, ctx->near_1);
   details::resize_tx(width_quarter_res, height_quarter_res, 1, ctx->far_0);
   details::resize_tx(width_quarter_res, height_quarter_res, 1, ctx->far_1);
   details::resize_tx(width_quarter_res, height_quarter_res, 1, ctx->near_x4_merged);
   details::resize_tx(width_quarter_res, height_quarter_res, 1, ctx->far_x4_merged);
   details::resize_tx(width_quarter_res, height_quarter_res, 1, ctx->alphas);

   details::resize_tx(width_quarter_res / 2, height_quarter_res / 2, 1, ctx->coc_nearx8);
}

void dof_terminate(DofContext* ctx)
{
   if (ctx == nullptr)
   {
      SCE_DBG_LOG_ERROR("Trying to terminate a NULL context.");
      return;
   }
   if (ctx->owner_handle != sce::Agc::ResourceRegistration::kInvalidOwnerHandle)
      sce::Agc::ResourceRegistration::unregisterOwnerAndResources(ctx->owner_handle);
}

template <typename T>
void dof_blur_pass(DofContext* ctx, T* dcb, DofRuntimeOption const& options)
{
   details::MarkerAutoScope<T> mark(dcb,"dof blur");
   struct ShaderParamNearVertical
   {
      details::DofBlurShaderParamNear* output;
      float inv_source_size[2];
   };


   float resx = (float)ctx->near_x4.getWidth();
   float resy = (float)ctx->near_x4.getHeight();
   psfx::details::ComputeShader* shaders[4] = { ctx->dof_circular_separable_near_one_cs ,
      ctx->dof_circular_separable_far_one_cs,ctx->dof_circular_separable_near_two_cs,ctx->dof_circular_separable_far_two_cs };

   float resy_16 = 16.0f/resy ;
   float resy_8 = 8.0f/resy ;
   float resy_4 =  4.0f/resy;
   float resx_16 = 16.0f / resx;
   float resx_8 = 8.0f / resx;
   float resx_4 = 4.0f / resx;
   const float blur_ratio_y = 0.03f;
   const float blur_ratio_x = 0.016f;
   if (fabs(resy_16 - blur_ratio_y) < fabs(resy_8 - blur_ratio_y))
   {
      shaders[0] = ctx->dof_circular_separable_near_one_cs;
      shaders[1] = ctx->dof_circular_separable_far_one_cs;


   }

   else if (fabs(resy_16 - blur_ratio_y) > fabs(resy_8 - blur_ratio_y) && fabs(resy_4 - blur_ratio_y) > fabs(resy_8 - blur_ratio_y))
   {
      shaders[0] = ctx->dof_circular_separable_near_one_half_cs;
      shaders[1] = ctx->dof_circular_separable_far_one_half_cs;

   }
   else if (fabs(resy_4 - blur_ratio_y) < fabs(resy_8 - blur_ratio_y))
   {
      shaders[0] = ctx->dof_circular_separable_near_one_quatr_cs;
      shaders[1] = ctx->dof_circular_separable_far_one_quatr_cs;

   }



   if (fabs(resx_16 - blur_ratio_x) < fabs(resx_8 - blur_ratio_x))
   {

      shaders[2] = ctx->dof_circular_separable_near_two_cs;
      shaders[3] = ctx->dof_circular_separable_far_two_cs;

   }

   else if (fabs(resx_16 - blur_ratio_x) > fabs(resx_8 - blur_ratio_x) && fabs(resx_4 - blur_ratio_x) > fabs(resx_8 - blur_ratio_x))
   {

      shaders[2] = ctx->dof_circular_separable_near_two_half_cs;
      shaders[3] = ctx->dof_circular_separable_far_two_half_cs;
   }
   else if (fabs(resx_4 - blur_ratio_x) < fabs(resx_8 - blur_ratio_x))
   {

      shaders[2] = ctx->dof_circular_separable_near_two_quatr_cs;
      shaders[3] = ctx->dof_circular_separable_far_two_quatr_cs;
   }



   const uint32_t pixel_per_wave_dimension = 8;
   const uint32_t wave_x = details::round_div(ctx->near_x4.getWidth(), pixel_per_wave_dimension);
   const uint32_t wave_y = details::round_div(ctx->near_x4.getHeight(), pixel_per_wave_dimension);



   details::DofBlurShaderParamNear* user_datav0 = details::alloc_top_down<details::DofBlurShaderParamNear>(dcb);
   details::DofBlurShaderParamFar* user_datav1 = details::alloc_top_down<details::DofBlurShaderParamFar>(dcb);
   //pass 1 of near
   user_datav0->depth  = ctx->coc_x4;
   user_datav0->coc    = ctx->coc_nearx8;
   user_datav0->input0 = ctx->near_x4;

   ShaderParamNearVertical str_near_vertical;
   str_near_vertical.inv_source_size[0] = 1.f / (float)ctx->near_x4.getWidth();
   str_near_vertical.inv_source_size[1] = 1.f / (float)ctx->near_x4.getHeight();

   user_datav0->out_color0 = ctx->near_0;
   user_datav0->out_color1 = ctx->near_1;
 
   str_near_vertical.output = user_datav0;
   details::set_compute_shader(dcb, shaders[0]);
   details::set_srt( dcb, shaders[0], ( void* ) &str_near_vertical );

   dcb->dispatch(wave_x, wave_y, 1, shaders[0]->m_specials->m_dispatchModifier);

   
   user_datav1->depth  = ctx->coc_x4;
   user_datav1->input0 = ctx->far_x4;

   user_datav1->out_color0 = ctx->far_0;
   user_datav1->out_color1 = ctx->far_1;
 
   struct ShaderParamFarVertical
   {
      details::DofBlurShaderParamFar* output;
      float inv_source_size[2];
   };

   ShaderParamFarVertical str_far_vertical;
   str_far_vertical.output = user_datav1;
   str_far_vertical.inv_source_size[0] = str_near_vertical.inv_source_size[0];
   str_far_vertical.inv_source_size[1] = str_near_vertical.inv_source_size[1];

   details::set_compute_shader( dcb, shaders[1] );
   details::set_srt( dcb, shaders[1], ( void* ) &str_far_vertical );
   
   dcb->dispatch(wave_x, wave_y, 1 , shaders[1]->m_specials->m_dispatchModifier);

   // Synch point between pass one and pass two
   details::cs_sync(dcb);

   //pass two near
   struct ShaderParamNearHorizontal
   {
      details::DofBlurShaderParamNearSecond* output;
      float inv_source_size[2];
   };

   details::DofBlurShaderParamNearSecond* scale_user_datah0 = details::alloc_top_down<details::DofBlurShaderParamNearSecond>(dcb);

   ShaderParamNearHorizontal str_near_horizontal;
   str_near_horizontal.inv_source_size[0] = 1.f / (float)ctx->near_x4.getWidth();
   str_near_horizontal.inv_source_size[1] = 1.f / (float)ctx->near_x4.getHeight();
   str_near_horizontal.output = scale_user_datah0;

   scale_user_datah0->depth = ctx->coc_x4;
   scale_user_datah0->coc = ctx->coc_nearx8;
   scale_user_datah0->input0 = ctx->near_0;
   scale_user_datah0->input1 = ctx->near_1;
   scale_user_datah0->out_color0 = ctx->near_x4_merged;
   scale_user_datah0->out_alpha = ctx->alphas;
 
   details::set_compute_shader( dcb, shaders[2]);
   details::set_srt( dcb, shaders[2], ( void* ) &str_near_horizontal );
   
   dcb->dispatch(wave_x, wave_y, 1, shaders[2]->m_specials->m_dispatchModifier);

   // Need to sync between these steps as they both write the same alpha texture.
   details::cs_sync(dcb);
   ///

   details::DofBlurShaderParamFarSecond* scale_user_datah1 = details::alloc_top_down<details::DofBlurShaderParamFarSecond>(dcb);
   scale_user_datah1->input0 = ctx->far_0;
   scale_user_datah1->input1 = ctx->far_1;
   scale_user_datah1->depth = ctx->coc_x4;
   scale_user_datah1->out_color0 = ctx->far_x4_merged;
   scale_user_datah1->out_alpha = ctx->alphas;
 
   struct ShaderParamFarHorizontal
   {
      details::DofBlurShaderParamFarSecond* output;
      float inv_source_size[2];
   };

   ShaderParamFarHorizontal str_far_horizontal;
   str_far_horizontal.inv_source_size[0] = str_near_horizontal.inv_source_size[0];
   str_far_horizontal.inv_source_size[1] = str_near_horizontal.inv_source_size[1];
   str_far_horizontal.output = scale_user_datah1;
   details::set_compute_shader( dcb, shaders[3]);
   details::set_srt( dcb, shaders[3], ( void* ) &str_far_horizontal );

   dcb->dispatch(wave_x, wave_y, 1, shaders[3]->m_specials->m_dispatchModifier);

   // Synch point
   details::cs_sync(dcb);
}

template <typename T>
void dof_compute_coc(DofContext* ctx, T* dcb, DofRuntimeOption const& options)
{
   struct ShaderParam
   {
      details::DofCocShaderTextures* textures;
      float tx_inv_size[2];
      float near_range[2];
      float near_far_start_end[4]; // near, range near, far, range far
   };

   const float near_plane = options.near_plane_distance;
   const float far_plane = options.far_plane_distance;

   details::MarkerAutoScope<T> marker(dcb, "dof Coc far near");
   details::set_compute_shader( dcb, ctx->dof_compute_coc_cs );

   ShaderParam str;
   details::DofCocShaderTextures* coc_text = details::alloc_top_down<details::DofCocShaderTextures>(dcb);

   str.textures = coc_text;
   str.textures->linear_depth = options.source_linear_depth;
   str.textures->coc = ctx->coc_full;

   str.tx_inv_size[0] = 1.f / options.target_width;
   str.tx_inv_size[1] = 1.f / options.target_height;

   str.near_range[0] = near_plane;
   str.near_range[1] = far_plane - near_plane;
   str.near_far_start_end[0] = options.near_cocMin;
   str.near_far_start_end[1] = 1.f / details::max_of(0.0001f, options.near_cocMax - options.near_cocMin);
   str.near_far_start_end[2] = options.far_cocMin;
   str.near_far_start_end[3] = 1.f / details::max_of(0.0001f, options.far_cocMax - options.far_cocMin);

   details::set_srt( dcb , ctx->dof_compute_coc_cs , ( void* ) &str );
   
   // Each thread reads/writes 2x2 pixels.
   const uint32_t pixel_per_wave_dimension = 8 * 2;
   const uint32_t wave_x = details::round_div(options.target_width, pixel_per_wave_dimension);
   const uint32_t wave_y = details::round_div(options.target_height, pixel_per_wave_dimension);
   dcb->dispatch(wave_x, wave_y, 1, ctx->dof_compute_coc_cs->m_specials->m_dispatchModifier );

   // Synch point
   details::cs_sync(dcb);
}

template <typename T>
void dof_down_scale(DofContext* ctx, T* dcb, DofRuntimeOption const& options)
{
   struct ShaderParam
   {
      details::DofDownscaleTextures* input;
      details::DofDownscaleResults* out_tx;
      float inv_source_size[2];
   };

   details::MarkerAutoScope<T> marker(dcb, "dof DS");

   details::set_compute_shader( dcb, ctx->dof_downscale_cs );
   ShaderParam str;
   details::DofDownscaleTextures* input_text   = details::alloc_top_down<details::DofDownscaleTextures>(dcb);
   details::DofDownscaleResults* results_text = details::alloc_top_down<details::DofDownscaleResults>(dcb);
   str.input = input_text;
   str.input->coc = ctx->coc_full;
   str.input->input = options.source;
   str.out_tx = results_text;
   results_text->out_depth = ctx->coc_x4;
   results_text->out_color0 = ctx->near_x4;
   results_text->out_color1 = ctx->far_x4;

   str.inv_source_size[0] = 1.f / (float)options.target_width;
   str.inv_source_size[1] = 1.f / (float)options.target_height;

   // some arbitrary params
   details::set_srt( dcb, ctx->dof_downscale_cs, ( void* ) &str );
   const uint32_t pixel_per_wave_dimension = 8;
   const uint32_t wave_x = details::round_div(ctx->coc_x4.getWidth(), pixel_per_wave_dimension);
   const uint32_t wave_y = details::round_div(ctx->coc_x4.getHeight(), pixel_per_wave_dimension);
   dcb->dispatch(wave_x, wave_y, 1, ctx->dof_downscale_cs->m_specials->m_dispatchModifier);

   // Synch point
   details::cs_sync(dcb);

   details::MarkerAutoScope<T> marker2(dcb, "dof max coc");

   // now run max coc shader
   struct ShaderParamCoc
   {
      details::DofCocTexture* text;
      float inv_source_size[2];
   };

   details::DofCocTexture* coc_texture = details::alloc_top_down<details::DofCocTexture>(dcb);
   coc_texture->input = ctx->coc_x4;
   coc_texture->output = ctx->coc_nearx8;

   ShaderParamCoc coc;
   coc.text = coc_texture;
   coc.inv_source_size[0] = 1.f / (float)ctx->coc_x4.getWidth();
   coc.inv_source_size[1] = 1.f / (float)ctx->coc_x4.getHeight();
   details::set_compute_shader( dcb, ctx->dof_max_coc_cs );
   details::set_srt( dcb, ctx->dof_max_coc_cs, ( void* ) &coc );

   const uint32_t wave_x2 = details::round_div(ctx->coc_nearx8.getWidth(), pixel_per_wave_dimension);
   const uint32_t wave_y2 = details::round_div(ctx->coc_nearx8.getHeight(), pixel_per_wave_dimension);
   dcb->dispatch(wave_x2, wave_y2, 1, ctx->dof_max_coc_cs->m_specials->m_dispatchModifier);
   details::cs_sync(dcb);
}

template <typename T>
void dof_composite(DofContext* ctx, T* dcb, DofRuntimeOption const& options)
{
   struct DofCompositeParam
   {
      details::DofCompositeTextures *tex;
      float inv_source_size[2];
      float inv_low_res_size[2];
   };

   details::MarkerAutoScope<T> marker(dcb, "dof composite");

   details::set_compute_shader( dcb, ctx->dof_composite_cs );

   details::DofCompositeTextures* user_data = details::alloc_top_down<details::DofCompositeTextures>(dcb);

   user_data->inputTextDepth = ctx->coc_full;
   user_data->inputTextNear  = ctx->near_x4_merged;
   user_data->inputTextFar   = ctx->far_x4_merged;
   user_data->inputTextAlpha = ctx->alphas;
   user_data->outputText = options.source;
   
   DofCompositeParam str_param;
   str_param.tex = user_data;
   str_param.inv_source_size[0] = 1.f / (float)options.target_width;
   str_param.inv_source_size[1] = 1.f / (float)options.target_height;

   str_param.inv_low_res_size[0] = 1.f / (float)ctx->near_x4.getWidth();
   str_param.inv_low_res_size[1] = 1.f / (float)ctx->near_x4.getHeight();

   details::set_srt( dcb, ctx->dof_composite_cs, ( void* ) &str_param );
   const uint32_t wave_x = details::round_div(options.target_width, 4);
   const uint32_t wave_y = details::round_div(options.target_height, 16);
   dcb->dispatch(wave_x, wave_y, 1, ctx->dof_composite_cs->m_specials->m_dispatchModifier);

   // Synch point
   details::cs_sync(dcb);
}

template<typename T>
void dof_apply(DofContext* ctx, T* dcb, DofRuntimeOption const& options)
{
   if (options.target_width != ctx->coc_full.getWidth() ||
       options.target_height != ctx->coc_full.getHeight())
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

   details::MarkerAutoScope<T> marker(dcb, "psfx dof");

   dof_compute_coc(ctx, dcb, options);
   dof_down_scale(ctx, dcb, options);
   dof_blur_pass(ctx, dcb, options);
   dof_composite(ctx, dcb, options);
}

void dof_apply(DofContext* ctx, sce::Agc::DrawCommandBuffer* dcb, DofRuntimeOption const& options)
{
   return dof_apply<sce::Agc::DrawCommandBuffer>(ctx, dcb,options);
}

void dof_apply(DofContext* ctx, sce::Agc::AsyncCommandBuffer* dcb, DofRuntimeOption const& options)
{
   return dof_apply<sce::Agc::AsyncCommandBuffer>(ctx, dcb, options);
}

}
