/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc.
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS

#include "sampleutil/graphics/platform_gnm/shader_gnm.h"

namespace sce { namespace SampleUtil { namespace Graphics {

template<> std::string shaderTypeName<Gnmx::VsShader>(){ return "Gnmx::VsShader"; }
template<> std::string shaderTypeName<Gnmx::PsShader>(){ return "Gnmx::PsShader"; }
template<> std::string shaderTypeName<Gnmx::EsShader>(){ return "Gnmx::EsShader"; }
template<> std::string shaderTypeName<Gnmx::GsShader>(){ return "Gnmx::GsShader"; }
template<> std::string shaderTypeName<Gnmx::LsShader>(){ return "Gnmx::LsShader"; }
template<> std::string shaderTypeName<Gnmx::HsShader>(){ return "Gnmx::HsShader"; }
template<> std::string shaderTypeName<Gnmx::CsShader>(){ return "Gnmx::CsShader"; }

template<> void _generateInputOffsetsCache<Gnmx::CsShader>(
	Gnmx::InputResourceOffsets *pLcueOffsetsTable,
	Gnmx::ConstantUpdateEngine::InputParameterCache *pCueOffsetsTable,
	const Gnmx::CsShader *pShader)
{
	Gnmx::generateInputResourceOffsetTable(pLcueOffsetsTable, Gnm::kShaderStageCs, pShader);
	Gnmx::ConstantUpdateEngine::initializeInputsCache(pCueOffsetsTable, pShader->getInputUsageSlotTable(), pShader->m_common.m_numInputUsageSlots);
}

template<> void _generateInputOffsetsCache<Gnmx::PsShader>(
	Gnmx::InputResourceOffsets *pLcueOffsetsTable,
	Gnmx::ConstantUpdateEngine::InputParameterCache *pCueOffsetsTable,
	const Gnmx::PsShader *pShader)
{
	Gnmx::generateInputResourceOffsetTable(pLcueOffsetsTable, Gnm::kShaderStagePs, pShader);
	Gnmx::ConstantUpdateEngine::initializeInputsCache(pCueOffsetsTable, pShader->getInputUsageSlotTable(), pShader->m_common.m_numInputUsageSlots);
}

template<> void _generateInputOffsetsCache<Gnmx::HsShader>(
	Gnmx::InputResourceOffsets *pLcueOffsetsTable,
	Gnmx::ConstantUpdateEngine::InputParameterCache *pCueOffsetsTable,
	const Gnmx::HsShader *pShader)
{
	Gnmx::generateInputResourceOffsetTable(pLcueOffsetsTable, Gnm::kShaderStageHs, pShader);
	Gnmx::ConstantUpdateEngine::initializeInputsCache(pCueOffsetsTable, pShader->getInputUsageSlotTable(), pShader->m_common.m_numInputUsageSlots);
}

std::unordered_set<ShaderBase*> ShaderBase::m_shaders;

ShaderBase::~ShaderBase()
{
	m_shaders.erase(this);
}

ShaderBase::ShaderBase()
{
	m_shaders.insert(this);
}

std::vector<char> loadShaderData( const std::string& fileName, VideoAllocator* allocator )
{
	std::ifstream ifs(fileName, std::ios_base::binary);
	if(!ifs.good())
	{
		return {};
	}
	ifs.seekg(0, std::ios_base::end);
	size_t fileSize = ifs.tellg();
	ifs.seekg(0, std::ios_base::beg);

	std::vector<char> data(fileSize);
	ifs.read(data.data(), fileSize);
	ifs.close();

	return data;
}

} } } // namespace SampleUtil::Graphics

#endif // _SCE_TARGET_OS_ORBIS