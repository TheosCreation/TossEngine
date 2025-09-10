/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#include "psfx_common.h"
#include "psfx_linearize_depth.h"
#include "psfx_exposure.h"
#include "psfx_bloom.h"
#include "psfx_ssao.h"
#include "psfx_ssr.h"
#include "psfx_dof.h"
#include "psfx_motion_blur.h"

namespace psfx
{

struct Context
{
   LinearizeDepthContext* linearize_depth;
   BloomContext* bloom;
   SsaoContext* ssao;
   DofContext* dof;
   ExposureContext* exposure;
   MotionBlurContext* motion_blur;
   SsrContext* ssr;
};

}
