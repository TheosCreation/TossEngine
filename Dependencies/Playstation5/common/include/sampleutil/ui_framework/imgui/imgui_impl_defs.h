/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2019 Sony Interactive Entertainment Inc.
* 
*/
#pragma once

#include <scebase_common.h>

#ifndef IMGUI_NOT_USE_GRAPHICS_CONTEXT

#if _SCE_TARGET_OS_PROSPERO
#define IMGUI_NOT_USE_GRAPHICS_CONTEXT	0
#else
#define IMGUI_NOT_USE_GRAPHICS_CONTEXT	0
#endif

#endif //ifndef IMGUI_NOT_USE_GRAPHICS_CONTEXT
