/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include "platform_agc/render_target_agc.h"
#endif
#if _SCE_TARGET_OS_ORBIS
#include "platform_gnm/render_target_gnm.h"
#endif