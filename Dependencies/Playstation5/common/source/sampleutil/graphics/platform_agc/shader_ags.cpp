/* SIE CONFIDENTIAL
 PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc.
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include "sampleutil/graphics/graphics_memory.h"
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/graphics/platform_agc/shader_ags.h"

namespace
{
#ifdef _DEBUG
	sce::Agc::ResourceRegistration::OwnerHandle		s_shaderBinaryResource = sce::Agc::ResourceRegistration::kInvalidOwnerHandle;
#endif
}

namespace sce { namespace SampleUtil { namespace Graphics {


	int	registerShaderBinary(const Agc::Shader	*pShader, const char *pName)
	{
		int ret = SCE_OK;
#ifndef _DEBUG
		(void)pShader;
		(void)pName;
#else
		if (g_isResourceRegistrationInitialized)
		{
			if (s_shaderBinaryResource == Agc::ResourceRegistration::kInvalidOwnerHandle)
			{
				ret = Agc::ResourceRegistration::registerOwner(&s_shaderBinaryResource, "shader binary");
				SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
				if (ret != SCE_OK)
				{
					return ret;
				}
			}
			Agc::ResourceRegistration::ResourceHandle rh;
			ret = Agc::ResourceRegistration::registerResource(&rh, s_shaderBinaryResource, (const void *)pShader->m_code, pShader->m_shaderSize, pName, Agc::ResourceRegistration::ResourceType::kShaderBaseAddress, 0);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
		}
#endif
		return ret;
	}

}}} // namespace sce::SampleUtil::Graphics
#endif