/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once


#include <scebase_common.h>
#include <sampleutil/graphics/context.h>
#include <sampleutil/graphics/sprite_utility.h>
#include <sampleutil/graphics/hdr.h>
#include <sampleutil/graphics/reprojection.h>
#if _SCE_TARGET_OS_PROSPERO
#include <sampleutil/graphics/platform_agc/shader_ags.h>
#endif
#if _SCE_TARGET_OS_ORBIS
#include <sampleutil/graphics/platform_gnm/shader_gnm.h>
#endif
