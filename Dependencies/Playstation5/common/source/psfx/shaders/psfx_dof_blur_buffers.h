/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2020 Sony Interactive Entertainment Inc.
* 
*/

#include "psfx_common.h"
#include "psfx_dof_filters.h"

#pragma argument(realtypes)
DofBlurParams srt : S_SRT_DATA;
[CxxSymbol(SYMBOL_NAME)]
[NUM_THREADS(8, 8, 1)]
void main(ushort2 offset : S_GROUP_ID, ushort2 thread_id : S_GROUP_THREAD_ID)
{
   const SamplerState bilinear_sampler = make_bilinear_sampler();
   ushort2 text_loc = offset * 8 + thread_id;

   half2 uv = ((half2)text_loc + half2(0.5, 0.5)) * (half2)srt.tx_scale;
#if NEAR_PASS
   half coc = srt.textures->max_coc.SampleLOD(bilinear_sampler, uv, 0).x;
#elif FAR_PASS
   half coc = srt.textures->coc.SampleLOD(bilinear_sampler, uv, 0).y;
#endif
   [branch]
   if (coc > 0.0f)
   {
      filter_component(text_loc, srt, bilinear_sampler, coc);
   }
   else
   {
      fill_with_zero(text_loc, srt);
   }
}
