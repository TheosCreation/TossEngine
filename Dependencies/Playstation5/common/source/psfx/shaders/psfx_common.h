/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

constexpr SamplerState make_point_sampler()
{
   sce::Agc::Core::Sampler sampler_desc = sce::Agc::Core::Sampler();
   sampler_desc.init();
   auto wrap_mode = sce::Agc::Core::Sampler::WrapMode::kClampLastTexel;
   sampler_desc.setWrapMode(wrap_mode, wrap_mode, wrap_mode);
   return SamplerState(sampler_desc);
}

constexpr SamplerState make_bilinear_sampler()
{
   sce::Agc::Core::Sampler sampler_desc = sce::Agc::Core::Sampler();
   sampler_desc.init();
   auto wrap_mode = sce::Agc::Core::Sampler::WrapMode::kClampLastTexel;
   sampler_desc.setWrapMode(wrap_mode, wrap_mode, wrap_mode);
   auto filter_mode = sce::Agc::Core::Sampler::FilterMode::kBilinear;
   sampler_desc.setXyFilterMode(filter_mode, filter_mode);
   return SamplerState(sampler_desc);
}

constexpr SamplerState make_trilinear_sampler()
{
   sce::Agc::Core::Sampler sampler_desc = sce::Agc::Core::Sampler();
   sampler_desc.init();
   auto wrap_mode = sce::Agc::Core::Sampler::WrapMode::kClampLastTexel;
   sampler_desc.setWrapMode(wrap_mode, wrap_mode, wrap_mode);
   auto filter_mode = sce::Agc::Core::Sampler::FilterMode::kBilinear;
   sampler_desc.setXyFilterMode(filter_mode, filter_mode);
   sampler_desc.setMipFilterMode(sce::Agc::Core::Sampler::MipFilterMode::kLinear);
   return SamplerState(sampler_desc);
}

float sqr(float x)
{
   return x * x;
}

float4 sqr(float4 x)
{
   return x * x;
}

float4 encode_linear_z(float4 lin_z, float near, float inv_range)
{
   return sqrt((lin_z - near) * inv_range);
}

float decode_linear_z(float z, float near, float range)
{
   return (sqr(z) * range) + near;
}

float4 decode_linear_z(float4 z, float near, float range)
{
   return (sqr(z) * range) + near;
}

float to_linear_z(const float2 plane_eq, const float z)
{
   const float epsilon_z = 1e-7;
   const float a = plane_eq.x;
   const float b = plane_eq.y;
   // If far is set to infinity the denominator can to zero.

   // Need to handle the division by zero case to avoid infinity that
   // will propagaty further and generate corruption.
   // Based on different z storage (inverse or not) the depth goes
   // to infinity in different direction (-inf or +inf)
   // Doing the check using an if is faster than with the sign operator.

   // b < 0 means that the z is standard, and the denominator is always negative
   // b > 0 means that the z is reversed, and the denominator is always positive
   const float d_neg = min(-epsilon_z, z+a);
   const float d_pos = max(epsilon_z, z+a);
   const float den = CndMask(b < 0, d_neg, d_pos);
   const float lin_z = b * rcp(den);

   return lin_z;
}

float4 to_linear_z(const float2 plane_eq, const float4 z)
{
   const float epsilon_z = 1e-7;
   const float a = plane_eq.x;
   const float b = plane_eq.y;
   // If far is set to infinity the denominator can to zero.

   // Need to handle the division by zero case to avoid infinity that
   // will propagaty further and generate corruption.
   // Based on different z storage (inverse or not) the depth goes
   // to infinity in different direction (-inf or +inf)
   // Doing the check using an if is faster than with the sign operator.

   // b < 0 means that the z is standard, and the denominator is always negative
   // b > 0 means that the z is reversed, and the denominator is always positive
   const float4 d_neg = min(-epsilon_z, z+a);
   const float4 d_pos = max(epsilon_z, z+a);
   const float4 den = CndMask(b < 0, d_neg, d_pos);
   const float4 lin_z = b * rcp(den);

   return lin_z;
}

uint get_lanes_per_wave()
{
   return (__get_wavemode() == __kWaveModeWave64) ? 64 : 32;
}
