/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc.
 * 
 */
#pragma once

#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include <sampleutil/graphics/graphics_memory.h>
#endif

namespace sce {	namespace SampleUtil { namespace Graphics {

#if _SCE_TARGET_OS_PROSPERO
void	initializeDefaultPackModelShaders();
#endif
#if _SCE_TARGET_OS_ORBIS
void	initializeDefaultPackModelShaders(VideoAllocator	&allocator);
void	finalizeDefaultPackModelShaders();
#endif

}}} // namespace sce::SampleUtil::Graphics

