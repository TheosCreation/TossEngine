/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

/*separable circual dof stuff*/
#ifdef __PSSL__

#if NEAR_PASS
struct DofBlurTexturesPassVertical
{
   Texture2D<half4>    inputText1;
   Texture2D<half>     max_coc;
   Texture2D<half2>    coc;
   RW_Texture2D<half4> output_RG;
   RW_Texture2D<half4> output_BA;
};

struct DofBlurTexturesPassHorizontal
{
   Texture2D<half4>    input_RG;
   Texture2D<half4>    input_BA;
   Texture2D<half>     max_coc;
   Texture2D<half2>    coc;
   RW_Texture2D<half3> output_tx;
   RW_Texture2D<half2> output_alfa;
};

#else

struct DofBlurTexturesPassVertical
{
   Texture2D<half4>    inputText1;
   Texture2D<half2>    coc;
   RW_Texture2D<half4> output_RG;
   RW_Texture2D<half4> output_BA;

};
struct DofBlurTexturesPassHorizontal
{
   Texture2D<half4>    input_RG;
   Texture2D<half4>    input_BA;
   Texture2D<half2>    coc;
   RW_Texture2D<half3> output_tx;
   RW_Texture2D<half2> output_alfa;
};
#endif

struct DofBlurParams
{
#if FIRST_PASS
   DofBlurTexturesPassVertical *textures;
#else
   DofBlurTexturesPassHorizontal *textures;
#endif
   float2 tx_scale;
};

// multiplication of two complex numbers
constexpr half2 multiply_complex(half2 p, half2 q)
{
   return half2(p.x*q.x - p.y*q.y, p.x*q.y + p.y*q.x);
}

/*precomputed stuff*/
#if RAD_FULL
static const short KERNEL_RADIUS = 16;
#elif RAD_HALF
static const short KERNEL_RADIUS = 8;
#elif RAD_QUATR
static const short KERNEL_RADIUS = 4;
#endif
static const half2 Kernel_Weights = half2(0.767583, 1.862321);

static constexpr half a0 = -0.862325;
static constexpr half b0 = 1.624835;

constexpr float2 get_kernel(const float x, const float a, const float b)
{
   const float x2 = x * x;
   return exp(x2 * a) * float2(cos(x2 * b), sin(x2 * b));
}

constexpr float get_accum(const float a, const float b, const float2 weight)
{
   float accum = 0.0f;
   [unroll]
   for (int i = -KERNEL_RADIUS; i <= KERNEL_RADIUS; ++i)
   {
      [unroll]
      for (int j = -KERNEL_RADIUS; j <= KERNEL_RADIUS; ++j)
      {
         const float2 ii = get_kernel((float)i / (float)KERNEL_RADIUS, a, b);
         const float2 jj = get_kernel((float)j / (float)KERNEL_RADIUS, a, b);
         accum += (half)dot(weight,(float2) multiply_complex((half2)ii, (half2)jj));
      }
   }
   return accum;
}

static const half normal_kernel = 1.0h /(half) sqrt(get_accum(a0, b0, Kernel_Weights));

void filter_component(ushort2 pixel_coord, DofBlurParams param, SamplerState sam, half fsize)
{
   const half2 step_val = (half2)param.tx_scale;

#if RAD_FULL
   constexpr half2 delta_step = half2(1.0f / (3840.0f / 4.0f), 1.0 / (2160.0f / 4.0f)) ;
#elif RAD_HALF
   constexpr half2 delta_step = half2(1.0f / (1920.0f / 4.0f), 1.0 / (1080.0f / 4.0f)) ;
#elif RAD_QUATR
   constexpr half2 delta_step = half2(1.0f / (1280.0f / 4.0f), 1.0 / (720.0f / 4.0f)) ;
#endif

   const half2 uv = (half2)(pixel_coord + half2(0.5f)) * step_val;
   const half2 delta = (half2)delta_step * fsize;
   half2 accumR = { 0.0h, 0.0h };
   half2 accumG = { 0.0h, 0.0h };
   half2 accumB = { 0.0h, 0.0h };
   half2 accumA = { 0.0h, 0.0h };

   [unroll]
   for (short i = -KERNEL_RADIUS; i <= KERNEL_RADIUS; ++i)
   {

#if FIRST_PASS
      const half2 uv_offset = half2(0.0f, half(i)) * delta;
#else
      const half2 uv_offset = half2(half(i), 0.0f) * delta;
#endif
      const half2 sample_loc = uv + uv_offset;
#if FAR_PASS
      half coc = param.textures->coc.SampleLOD(sam, sample_loc, 0).y;
#else
      half coc = param.textures->coc.SampleLOD(sam, sample_loc, 0).x;
#endif
      coc = 1.0;
      const half2 c0 =(half2) get_kernel((half)i / (half)KERNEL_RADIUS, a0, b0) * normal_kernel * coc;

#if FIRST_PASS
      half4 fetch = half4(param.textures->inputText1.SampleLOD(sam, sample_loc, 0));
      accumR += fetch.x * c0;
      accumG += fetch.y * c0;

      accumB += fetch.z * c0;
      accumA += fetch.w * c0;
   }

   param.textures->output_RG[pixel_coord] = half4(accumR, accumG);
   param.textures->output_BA[pixel_coord] = half4(accumB, accumA);
#else // NEAR_PASS
      half4 fetch1 = param.textures->input_RG.SampleLOD(sam, sample_loc, 0);
      half4 fetch2 = param.textures->input_BA.SampleLOD(sam, sample_loc, 0);
      accumR += multiply_complex((fetch1.xy), c0); // Red
      accumG += multiply_complex((fetch1.zw), c0); // Green

      accumB += multiply_complex((fetch2.xy), c0); // Blue
      accumA += multiply_complex((fetch2.zw), c0); // Alpha
}

   half r = dot(accumR, Kernel_Weights);
   half g = dot(accumG, Kernel_Weights);
   half b = dot(accumB, Kernel_Weights);
   half a = dot(accumA, Kernel_Weights);
   param.textures->output_tx[pixel_coord] = half3(r, g, b);
#if NEAR_PASS
   param.textures->output_alfa[pixel_coord].x = a;
#elif FAR_PASS
   param.textures->output_alfa[pixel_coord].y = a;
#endif
#endif
}

void fill_with_zero(ushort2 xy, DofBlurParams param)
{
#if FIRST_PASS
   param.textures->output_RG[xy] = half4(0, 0, 0, 0);
   param.textures->output_BA[xy] = half4(0, 0, 0, 0);
#else
   param.textures->output_tx[xy] = half3(0, 0, 0);
#if NEAR_PASS
   param.textures->output_alfa[xy].x = 0;
#elif FAR_PASS
   param.textures->output_alfa[xy].y = 0;
#endif
#endif
}

#endif //#ifdef __PSSL__