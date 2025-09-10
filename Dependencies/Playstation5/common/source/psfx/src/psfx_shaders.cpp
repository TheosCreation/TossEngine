/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2020 Sony Interactive Entertainment Inc.
* 
*/

#include "psfx_details.h"
#include <shader/shader_reflection.h>

namespace psfx
{

namespace details
{

uint32_t shader_code_size(sce::Agc::Shader const& common)
{
   return common.m_shaderSize + common.m_embeddedConstantBufferSizeInDQW* 16;
}


sce::Agc::Shader* make_cs(sce::Agc::ResourceRegistration::OwnerHandle owner, const char* resource_name, char* header, const char* text)
{
   sce::Agc::Shader* agc_shader = (sce::Agc::Shader*)header;
   
   if (!agc_shader->m_code)
   {
      SceError ret = sce::Agc::createShader(&agc_shader, header, text);
      assert(ret == SCE_OK);
      (void)ret;
   }
   if (owner != sce::Agc::ResourceRegistration::kInvalidOwnerHandle)
   {
      sce::Agc::ResourceRegistration::ResourceHandle handle;
     SceError ret = sce::Agc::ResourceRegistration::registerResource(&handle, owner, const_cast<void*>(agc_shader->m_code), agc_shader->m_shaderSize, resource_name, sce::Agc::ResourceRegistration::ResourceType::kShaderBaseAddress, 0);
      assert(ret == SCE_OK);
      (void)ret;
   }
    
   return agc_shader;
}

}

}
