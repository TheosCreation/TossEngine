/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once

#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include <memory>
#include <vector>
#include <string>
#include <unordered_set>
#include <fstream>
#include <gnmx.h>
#include <shader/binary.h>
#include <gnmx/shader_parser.h>
#include "sampleutil/graphics/compat.h"
#include "sampleutil/memory.h"
#include "sampleutil/debug/perf.h"


#define DEFINE_SHADER(stage, name) \
extern "C" int _binary_##name##_sb_start; \
std::unique_ptr<sce::SampleUtil::Graphics::Shader<stage>> g_shader_##name = nullptr

#define EXTERN_SHADER(stage, name) \
extern "C" int _binary_##name##_sb_start; \
extern std::unique_ptr<sce::SampleUtil::Graphics::Shader<stage>> g_shader_##name

#define CREATE_SHADER(stage, name, allocator) \
if (!g_shader_##name) g_shader_##name = std::unique_ptr<sce::SampleUtil::Graphics::Shader<stage>>(sce::SampleUtil::Graphics::createShader<stage>(#name, &_binary_##name##_sb_start, allocator))

#define CREATE_LS_HS_SHADER(ls_name, hs_name, allocator) \
if (!g_shader_##ls_name) g_shader_##ls_name = std::unique_ptr<sce::SampleUtil::Graphics::Shader<sce::Gnmx::LsShader>>(sce::SampleUtil::Graphics::createShader<sce::Gnmx::LsShader>(#ls_name, &_binary_##ls_name##_sb_start, allocator)); \
if (!g_shader_##hs_name) g_shader_##hs_name = std::unique_ptr<sce::SampleUtil::Graphics::Shader<sce::Gnmx::HsShader>>(new sce::SampleUtil::Graphics::Shader<sce::Gnmx::HsShader>(#hs_name, &_binary_##hs_name##_sb_start, allocator, g_shader_##ls_name->get()));

#define DESTROY_SHADER(name) \
if (g_shader_##name) g_shader_##name.reset(nullptr)

#define SHADER(name) g_shader_##name->get()
#if SCE_GNMX_ENABLE_GFX_LCUE
#define OFFSETS_TABLE(name) g_shader_##name->getLcueOffsetsTable()
#else // SCE_GNMX_ENABLE_GFX_LCUE
#define OFFSETS_TABLE(name) g_shader_##name->getCueOffsetsTable()
#endif // SCE_GNMX_ENABLE_GFX_LCUE
#define SIZE_OF_SRT(name) g_shader_##name->getSrtSizeInDW()

#define SET_CS_SHADER(gfxc, shader, tgDimensions)								\
do { gfxc.setCsShader(SHADER(shader), OFFSETS_TABLE(shader));					\
	tgDimensions[0] = SHADER(shader)->m_csStageRegisters.m_computeNumThreadX;	\
	tgDimensions[1] = SHADER(shader)->m_csStageRegisters.m_computeNumThreadY;	\
	tgDimensions[2] = SHADER(shader)->m_csStageRegisters.m_computeNumThreadZ;	\
} while(0)

namespace sce { namespace SampleUtil { namespace Graphics {
template<typename T> std::string shaderTypeName();

struct ShaderBase
{
	std::string m_fileName;
	VideoAllocator* m_alloc;
	std::vector<uint8_t> m_shaderHeader;
	Memory::Gpu::unique_ptr<uint8_t[]> m_shaderCode;
	Gnmx::InputResourceOffsets m_lcueOffsetsTable;
	Gnmx::ConstantUpdateEngine::InputParameterCache m_cueOffsetsTable;
	uint32_t m_srtSizeInDW;
	ShaderBase();
	ShaderBase(const std::string& fileName, VideoAllocator* alloc)
		: m_fileName(fileName)
		, m_alloc(alloc)
		, m_srtSizeInDW(0)
	{}
	virtual ~ShaderBase();
	static std::unordered_set<ShaderBase*> m_shaders;
	virtual void parseShader(const void *pBinary) = 0;
	const Gnmx::InputResourceOffsets *getLcueOffsetsTable() const  {	return &m_lcueOffsetsTable; }
	const Gnmx::ConstantUpdateEngine::InputParameterCache *getCueOffsetsTable() const {	return &m_cueOffsetsTable; }
	uint32_t getSrtSizeInDW() const { return m_srtSizeInDW; }
};


template<typename T>
struct Shader : public ShaderBase
{
	T* m_shader;
	void copyGpuCode(const Gnmx::ShaderInfo& info, Memory::Gpu::unique_ptr<uint8_t[]>& data, VideoAllocator* alloc)
	{
		data = Memory::Gpu::make_unique<uint8_t>(info.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes, *alloc, "sce::SampleUtil::Graphics::Shader Binary:" + m_fileName);
		std::memcpy(data.get(), info.m_gpuShaderCode, info.m_gpuShaderCodeSize);
		alloc->registerResource(data.get(), info.m_gpuShaderCodeSize, "sce::SampleUtil::Graphics::Shader Binary:" + m_fileName, { Gnm::kResourceTypeShaderBaseAddress });
	}
public:
	Shader(const std::string& fileName, VideoAllocator* alloc)
		: ShaderBase(fileName, alloc)
	{
		auto data = loadShaderData(m_fileName, m_alloc);
		parseShader(data.data());
	}
	Shader(const std::string &name, const void *pBinary, VideoAllocator* alloc)
		: ShaderBase(name, alloc)
	{
		parseShader(pBinary);
	}
	virtual void parseShader(const void *pBinary) override;
	T* get() { return m_shader; }
	const T* get() const { return m_shader; }
};

std::vector<char> loadShaderData(const std::string& fileName, VideoAllocator* allocator);

template<>
struct Shader<Gnmx::GsShader> : public ShaderBase
{
	Gnmx::GsShader* m_shader;
	void* copyGpuCode(const Gnmx::ShaderInfo& gsInfo, const Gnmx::ShaderInfo& vsInfo, Memory::Gpu::unique_ptr<uint8_t[]>& data, VideoAllocator* alloc)
	{	
		uint32_t alignment = Gnm::kAlignmentOfShaderInBytes;
		uint32_t vsCodeOffset = ((gsInfo.m_gpuShaderCodeSize + alignment - 1)/alignment)*alignment;
		uint32_t totalCodeSize = vsCodeOffset + vsInfo.m_gpuShaderCodeSize;

		data = Memory::Gpu::make_unique<uint8_t>(totalCodeSize, Gnm::kAlignmentOfShaderInBytes, *alloc, "sce::SampleUtil::Graphics::GS Binary:" + m_fileName);
		std::memcpy(data.get(), gsInfo.m_gpuShaderCode, gsInfo.m_gpuShaderCodeSize);
		std::memcpy(static_cast<uint8_t*>(data.get()) + vsCodeOffset, vsInfo.m_gpuShaderCode, vsInfo.m_gpuShaderCodeSize);
		alloc->registerResource(static_cast<uint8_t*>(data.get()) + vsCodeOffset, vsInfo.m_gpuShaderCodeSize, "sce::SampleUtil::Graphics::GS Binary:" + m_fileName, { Gnm::kResourceTypeShaderBaseAddress });
		return static_cast<uint8_t*>(data.get()) + vsCodeOffset;
	}
public:
	Shader(const std::string& fileName, VideoAllocator* alloc)
		: ShaderBase(fileName, alloc)
	{
		auto data = loadShaderData(m_fileName, m_alloc);
		parseShader(data.data());
	}
	Shader(const std::string &name, const void *pBinary, VideoAllocator* alloc)
		: ShaderBase(name, alloc)
	{
		parseShader(pBinary);
	}
	virtual void parseShader(const void *pBinary) override
	{

		Gnmx::ShaderInfo gsInfo;
		Gnmx::ShaderInfo vsInfo;
		Gnmx::parseGsShader(&gsInfo, &vsInfo, pBinary);

		m_shaderHeader.resize(reinterpret_cast<const Gnmx::GsShader*>(gsInfo.m_shaderStruct)->computeSize());
		std::memcpy(m_shaderHeader.data(), gsInfo.m_shaderStruct, m_shaderHeader.size());
		void* vsCode = copyGpuCode(gsInfo, vsInfo, m_shaderCode, m_alloc);
		m_shader = reinterpret_cast<Gnmx::GsShader*>(m_shaderHeader.data());
		m_shader->patchShaderGpuAddresses(m_shaderCode.get(), vsCode);
		Gnmx::generateInputResourceOffsetTable(&m_lcueOffsetsTable, Gnm::kShaderStageGs, m_shader);
		Gnmx::ConstantUpdateEngine::initializeInputsCache(&m_cueOffsetsTable, m_shader->getInputUsageSlotTable(), m_shader->m_common.m_numInputUsageSlots);
		const Gnm::InputUsageSlot *pUsageSlots = m_shader->getInputUsageSlotTable();
		for (int i = 0; i < m_shader->m_common.m_numInputUsageSlots; i++)
		{
			if (pUsageSlots[i].m_usageType == Gnm::kShaderInputUsageImmShaderResourceTable)
			{
				m_srtSizeInDW =  pUsageSlots[i].m_srtSizeInDWordMinusOne + 1;
				break;
			}
		}
	}
	Gnmx::GsShader* get() { return m_shader; }
	const Gnmx::GsShader* get() const { return m_shader; }
};

template<>
struct Shader<Gnmx::VsShader> : ShaderBase
{
	Gnmx::VsShader*									m_shader;
	bool											m_hasFetchShader;
	Memory::Gpu::unique_ptr<uint8_t[]>				m_fetchShaderCode;
	Gnm::FetchShaderInstancingMode const*			m_instancing;
	uint32_t										m_numElementsInInstancingData;
public:
	Shader(const std::string& fileName, VideoAllocator* alloc, bool hasFetchShader = false, Gnm::FetchShaderInstancingMode const* instancing = nullptr, uint32_t numElementsInInstancingData = 0)
		: ShaderBase(fileName, alloc)
		, m_hasFetchShader(hasFetchShader)
		, m_instancing(instancing)
		, m_numElementsInInstancingData(numElementsInInstancingData)
	{
		auto data = loadShaderData(m_fileName, m_alloc);
		parseShader(data.data());
	}
	Shader(const std::string &name, const void *pBinary, VideoAllocator* alloc, bool hasFetchShader = false, Gnm::FetchShaderInstancingMode const* instancing = nullptr, uint32_t numElementsInInstancingData = 0)
		: ShaderBase(name, alloc)
		, m_hasFetchShader(hasFetchShader)
		, m_instancing(instancing)
		, m_numElementsInInstancingData(numElementsInInstancingData)
	{
		parseShader(pBinary);
	}
	virtual void parseShader(const void *pBinary) override
	{
		Gnmx::ShaderInfo info;
		Gnmx::parseShader(&info, pBinary);
		
		m_shaderHeader.resize(reinterpret_cast<const Gnmx::VsShader*>(info.m_shaderStruct)->computeSize());
		std::memcpy(m_shaderHeader.data(), info.m_shaderStruct, m_shaderHeader.size());
		m_shaderCode = Memory::Gpu::make_unique<uint8_t>(info.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes, *m_alloc, "sce::SampleUtil::Graphics::VS Binary:" + m_fileName);
		std::memcpy(m_shaderCode.get(), info.m_gpuShaderCode, info.m_gpuShaderCodeSize);
		m_alloc->registerResource(m_shaderCode.get(), info.m_gpuShaderCodeSize, "sce::SampleUtil::Graphics::VS Binary:" + m_fileName, { Gnm::kResourceTypeShaderBaseAddress });
		m_shader = reinterpret_cast<Gnmx::VsShader*>(m_shaderHeader.data());
		m_shader->patchShaderGpuAddress(m_shaderCode.get());

		auto fetchShaderSize = Gnmx::computeVsFetchShaderSize(info.m_vsShader);
		m_fetchShaderCode = Memory::Gpu::make_unique<uint8_t>(fetchShaderSize, Gnm::kAlignmentOfFetchShaderInBytes, *m_alloc, "sce::SampleUtil::Graphics::fetchShader:" + m_fileName);
		uint32_t shaderModifier;
		Gnmx::generateVsFetchShader(m_fetchShaderCode.get(), &shaderModifier, m_shader, m_instancing, m_numElementsInInstancingData);
		m_alloc->registerResource(m_fetchShaderCode.get(), fetchShaderSize, "sce::SampleUtil::Graphics::fetchShader:" + m_fileName, { Gnm::kResourceTypeFetchShaderBaseAddress });
		m_shader->applyFetchShaderModifier(shaderModifier);
		Gnmx::generateInputResourceOffsetTable(&m_lcueOffsetsTable, Gnm::kShaderStageVs, m_shader);
		Gnmx::ConstantUpdateEngine::initializeInputsCache(&m_cueOffsetsTable, m_shader->getInputUsageSlotTable(), m_shader->m_common.m_numInputUsageSlots);
		const Gnm::InputUsageSlot *pUsageSlots = m_shader->getInputUsageSlotTable();
		for (int i = 0; i < m_shader->m_common.m_numInputUsageSlots; i++)
		{
			if (pUsageSlots[i].m_usageType == Gnm::kShaderInputUsageImmShaderResourceTable)
			{
				m_srtSizeInDW =  pUsageSlots[i].m_srtSizeInDWordMinusOne + 1;
				break;
			}
		}
	}

	Gnmx::VsShader* get() { return m_shader; }
	const Gnmx::VsShader* get() const { return m_shader; }
	void* getFetchShader() const { return m_fetchShaderCode.get(); }
};

template<>
struct Shader<Gnmx::EsShader> : ShaderBase
{
	Gnmx::EsShader*							m_shader;
	bool									m_hasFetchShader;
	Memory::Gpu::unique_ptr<uint8_t[]>		m_fetchShaderCode;
	Gnm::FetchShaderInstancingMode const*	m_instancing;
	uint32_t								m_numElementsInInstancingData;
public:
	Shader(const std::string& fileName, VideoAllocator* alloc, bool hasFetchShader = false, Gnm::FetchShaderInstancingMode const* instancing = nullptr, uint32_t numElementsInInstancingData = 0)
		: ShaderBase(fileName, alloc)
		, m_hasFetchShader(hasFetchShader)
		, m_instancing(instancing)
		, m_numElementsInInstancingData(numElementsInInstancingData)
	{
		auto data = loadShaderData(m_fileName, m_alloc);
		parseShader(data.data());
	}
	Shader(const std::string &name, const void *pBinary, VideoAllocator* alloc, bool hasFetchShader = false, Gnm::FetchShaderInstancingMode const* instancing = nullptr, uint32_t numElementsInInstancingData = 0)
		: ShaderBase(name, alloc)
		, m_hasFetchShader(hasFetchShader)
		, m_instancing(instancing)
		, m_numElementsInInstancingData(numElementsInInstancingData)
	{
		parseShader(pBinary);
	}
	virtual void parseShader(const void *pBinary) override
	{
		Gnmx::ShaderInfo info;
		Gnmx::parseShader(&info, pBinary);
		
		m_shaderHeader.resize(reinterpret_cast<const Gnmx::EsShader*>(info.m_shaderStruct)->computeSize());
		std::memcpy(m_shaderHeader.data(), info.m_shaderStruct, m_shaderHeader.size());
		m_shaderCode = Memory::Gpu::make_unique<uint8_t>(info.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes, *m_alloc, "sce::SampleUtil::Graphics::ES Binary:" + m_fileName);
		std::memcpy(m_shaderCode.get(), info.m_gpuShaderCode, info.m_gpuShaderCodeSize);
		m_alloc->registerResource(m_shaderCode.get(), info.m_gpuShaderCodeSize, "sce::SampleUtil::Graphics::ES Binary:" + m_fileName, { Gnm::kResourceTypeShaderBaseAddress });
		m_shader = reinterpret_cast<Gnmx::EsShader*>(m_shaderHeader.data());
		m_shader->patchShaderGpuAddress(m_shaderCode.get());

		auto fetchShaderSize = Gnmx::computeEsFetchShaderSize(info.m_esShader);
		m_fetchShaderCode = Memory::Gpu::make_unique<uint8_t>(fetchShaderSize, Gnm::kAlignmentOfFetchShaderInBytes, *m_alloc, "sce::SampleUtil::Graphics::fetchShader:" + m_fileName);
		uint32_t shaderModifier;
		Gnmx::generateEsFetchShader(m_fetchShaderCode.get(), &shaderModifier, m_shader, m_instancing, m_numElementsInInstancingData);
		m_alloc->registerResource(m_fetchShaderCode.get(), fetchShaderSize, "sce::SampleUtil::Graphics::fetchShader:" + m_fileName, { Gnm::kResourceTypeFetchShaderBaseAddress });
		m_shader->applyFetchShaderModifier(shaderModifier);
		Gnmx::generateInputResourceOffsetTable(&m_lcueOffsetsTable, Gnm::kShaderStageEs, m_shader);
		Gnmx::ConstantUpdateEngine::initializeInputsCache(&m_cueOffsetsTable, m_shader->getInputUsageSlotTable(), m_shader->m_common.m_numInputUsageSlots);
		const Gnm::InputUsageSlot *pUsageSlots = m_shader->getInputUsageSlotTable();
		for (int i = 0; i < m_shader->m_common.m_numInputUsageSlots; i++)
		{
			if (pUsageSlots[i].m_usageType == Gnm::kShaderInputUsageImmShaderResourceTable)
			{
				m_srtSizeInDW =  pUsageSlots[i].m_srtSizeInDWordMinusOne + 1;
				break;
			}
		}
	}

	Gnmx::EsShader* get() { return m_shader; }
	const Gnmx::EsShader* get() const { return m_shader; }
	void* getFetchShader() const { return m_fetchShaderCode.get(); }
};

template<>
struct Shader<Gnmx::LsShader> : ShaderBase
{
	Gnmx::LsShader*							m_shader;
	bool									m_hasFetchShader;
	Memory::Gpu::unique_ptr<uint8_t[]>		m_fetchShaderCode;
	Gnm::FetchShaderInstancingMode const*	m_instancing;
	uint32_t								m_numElementsInInstancingData;
public:
	Shader(const std::string& fileName, VideoAllocator* alloc, bool hasFetchShader = false, Gnm::FetchShaderInstancingMode const* instancing = nullptr, uint32_t numElementsInInstancingData = 0)
		: ShaderBase(fileName, alloc)
		, m_hasFetchShader(hasFetchShader)
		, m_instancing(instancing)
		, m_numElementsInInstancingData(numElementsInInstancingData)
	{
		auto data = loadShaderData(m_fileName, m_alloc);
		parseShader(data.data());
	}
	Shader(const std::string &name, const void *pBinary, VideoAllocator* alloc, bool hasFetchShader = false, Gnm::FetchShaderInstancingMode const* instancing = nullptr, uint32_t numElementsInInstancingData = 0)
		: ShaderBase(name, alloc)
		, m_hasFetchShader(hasFetchShader)
		, m_instancing(instancing)
		, m_numElementsInInstancingData(numElementsInInstancingData)
	{
		parseShader(pBinary);
	}
	virtual void parseShader(const void *pBinary) override
	{
		Gnmx::ShaderInfo info;
		Gnmx::parseShader(&info, pBinary);
		
		m_shaderHeader.resize(reinterpret_cast<const Gnmx::LsShader*>(info.m_shaderStruct)->computeSize());
		std::memcpy(m_shaderHeader.data(), info.m_shaderStruct, m_shaderHeader.size());
		m_shaderCode = Memory::Gpu::make_unique<uint8_t>(info.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes, *m_alloc, "sce::SampleUtil::Graphics::LS Binary:" + m_fileName);
		std::memcpy(m_shaderCode.get(), info.m_gpuShaderCode, info.m_gpuShaderCodeSize);
		m_alloc->registerResource(m_shaderCode.get(), info.m_gpuShaderCodeSize, "sce::SampleUtil::Graphics::LS Binary:" + m_fileName, { Gnm::kResourceTypeShaderBaseAddress });
		m_shader = reinterpret_cast<Gnmx::LsShader*>(m_shaderHeader.data());
		m_shader->patchShaderGpuAddress(m_shaderCode.get());
		auto fetchShaderSize = Gnmx::computeLsFetchShaderSize(info.m_lsShader);
		m_fetchShaderCode = Memory::Gpu::make_unique<uint8_t>(fetchShaderSize, Gnm::kAlignmentOfFetchShaderInBytes, *m_alloc, "sce::SampleUtil::Graphics::fetchShader" + m_fileName);
		uint32_t shaderModifier;
		Gnmx::generateLsFetchShader(m_fetchShaderCode.get(), &shaderModifier, m_shader, m_instancing, m_numElementsInInstancingData);
		m_alloc->registerResource(m_fetchShaderCode.get(), fetchShaderSize, "sce::SampleUtil::Graphics::fetchShader:" + m_fileName, { Gnm::kResourceTypeFetchShaderBaseAddress });
		m_shader->applyFetchShaderModifier(shaderModifier);
		Gnmx::generateInputResourceOffsetTable(&m_lcueOffsetsTable, Gnm::kShaderStageLs, m_shader);
		Gnmx::ConstantUpdateEngine::initializeInputsCache(&m_cueOffsetsTable, m_shader->getInputUsageSlotTable(), m_shader->m_common.m_numInputUsageSlots);
		const Gnm::InputUsageSlot *pUsageSlots = m_shader->getInputUsageSlotTable();
		for (int i = 0; i < m_shader->m_common.m_numInputUsageSlots; i++)
		{
			if (pUsageSlots[i].m_usageType == Gnm::kShaderInputUsageImmShaderResourceTable)
			{
				m_srtSizeInDW =  pUsageSlots[i].m_srtSizeInDWordMinusOne + 1;
				break;
			}
		}
	}

	Gnmx::LsShader* get() { return m_shader; }
	const Gnmx::LsShader* get() const { return m_shader; }
	void* getFetchShader() const { return m_fetchShaderCode.get(); }
};

template<>
struct Shader<Gnmx::HsShader> : public ShaderBase
{
	Gnmx::HsShader													*m_shader;
	uint32_t														m_tgPatchCount;
	uint32_t														m_vgtPrimCount;
	Memory::Gpu::unique_ptr<Gnm::TessellationDataConstantBuffer>	m_tessConstants;

	void computeTessellationParam(Gnmx::LsShader	*pLsShader, VideoAllocator* alloc)
	{
		uint32_t minLdsSize		= Gnm::computeLdsUsagePerPatchInBytesPerThreadGroup(&m_shader->m_hullStateConstants, pLsShader->m_lsStride);
		uint32_t minVgprCount	= m_shader->getNumVgprs();
		uint32_t ldsSize		= std::max<uint32_t>(minLdsSize, 4096);
		uint32_t vgprCount		= std::max<uint32_t>(minVgprCount, 16);
		Gnmx::computeVgtPrimitiveAndPatchCounts(&m_vgtPrimCount, &m_tgPatchCount, vgprCount, ldsSize, pLsShader, m_shader);
		Gnm::TessellationDataConstantBuffer	tessConstants;
		tessConstants.init(&m_shader->m_hullStateConstants, pLsShader->m_lsStride, m_tgPatchCount, 32.f);
		m_tessConstants = Memory::Gpu::make_unique<Gnm::TessellationDataConstantBuffer>(Gnm::kAlignmentOfBufferInBytes, std::vector<sce::SampleUtil::Graphics::Compat::ResourceType>({ Gnm::kResourceTypeBufferBaseAddress }), *alloc, "sce::SampleUtil::Graphics::TessConstants:" + m_fileName);
		memcpy(m_tessConstants.get(), &tessConstants, sizeof(tessConstants));
	}

	void copyGpuCode(const Gnmx::ShaderInfo& info, Memory::Gpu::unique_ptr<uint8_t[]>& data, VideoAllocator* alloc)
	{
		data = Memory::Gpu::make_unique<uint8_t>(info.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes, *alloc, "sce::SampleUtil::Graphics::Shader Binary:" + m_fileName);
		std::memcpy(data.get(), info.m_gpuShaderCode, info.m_gpuShaderCodeSize);
		alloc->registerResource(data.get(), info.m_gpuShaderCodeSize, "sce::SampleUtil::Graphics::Shader Binary:" + m_fileName, { Gnm::kResourceTypeShaderBaseAddress });
	}
public:
	Shader(const std::string& fileName, VideoAllocator* alloc, Gnmx::LsShader *pLsShader)
		: ShaderBase(fileName, alloc)
	{
		auto data = loadShaderData(m_fileName, m_alloc);
		parseShader(data.data());
		computeTessellationParam(pLsShader, m_alloc);
	}
	Shader(const std::string &name, const void *pBinary, VideoAllocator* alloc, Gnmx::LsShader *pLsShader)
		: ShaderBase(name, alloc)
	{
		parseShader(pBinary);
		computeTessellationParam(pLsShader, m_alloc);
	}
	virtual void parseShader(const void *pBinary) override
	{
		Gnmx::ShaderInfo info;
		Gnmx::parseShader(&info, pBinary);
		
		m_shaderHeader.resize(reinterpret_cast<const Gnmx::HsShader*>(info.m_shaderStruct)->computeSize());
		std::memcpy(m_shaderHeader.data(), info.m_shaderStruct, m_shaderHeader.size());
		m_shaderCode = Memory::Gpu::make_unique<uint8_t>(info.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes, { Gnm::kResourceTypeShaderBaseAddress }, *m_alloc, "sce::SampleUtil::Graphics::HS Binary:" + m_fileName);
		std::memcpy(m_shaderCode.get(), info.m_gpuShaderCode, info.m_gpuShaderCodeSize);
		m_alloc->registerResource(m_shaderCode.get(), info.m_gpuShaderCodeSize, "sce::SampleUtil::Graphics::HS Binary:" + m_fileName, { Gnm::kResourceTypeShaderBaseAddress });
		m_shader = reinterpret_cast<Gnmx::HsShader*>(m_shaderHeader.data());
		m_shader->patchShaderGpuAddress(m_shaderCode.get());
		Gnmx::generateInputResourceOffsetTable(&m_lcueOffsetsTable, Gnm::kShaderStageHs, m_shader);
		Gnmx::ConstantUpdateEngine::initializeInputsCache(&m_cueOffsetsTable, m_shader->getInputUsageSlotTable(), m_shader->m_common.m_numInputUsageSlots);
		const Gnm::InputUsageSlot *pUsageSlots = m_shader->getInputUsageSlotTable();
		for (int i = 0; i < m_shader->m_common.m_numInputUsageSlots; i++)
		{
			if (pUsageSlots[i].m_usageType == Gnm::kShaderInputUsageImmShaderResourceTable)
			{
				m_srtSizeInDW =  pUsageSlots[i].m_srtSizeInDWordMinusOne + 1;
				break;
			}
		}
	}
	Gnmx::HsShader* get() { return m_shader; }
	const Gnmx::HsShader* get() const { return m_shader; }
};

template<typename T>
Shader<T>* loadAndCreateShader(const std::string& fileName, VideoAllocator* allocator)
{
	return new Shader<T>(fileName, allocator);
}

template<typename T>
Shader<T>* createShader(const std::string& name, const void *pBinary, VideoAllocator* allocator)
{
	return new Shader<T>(name, pBinary, allocator);
}

template<typename T>
void _generateInputOffsetsCache(
	Gnmx::InputResourceOffsets *pLcueOffsetsTable,
	Gnmx::ConstantUpdateEngine::InputParameterCache *pCueOffsetsTable,
	const T *pShader);

template<typename T>
void Shader<T>::parseShader(const void *pBinary)
{
	Gnmx::ShaderInfo info;
	Gnmx::parseShader(&info, pBinary);

	m_shaderHeader.resize(reinterpret_cast<const T*>(info.m_shaderStruct)->computeSize());
	std::memcpy(m_shaderHeader.data(), info.m_shaderStruct, m_shaderHeader.size());
	copyGpuCode(info, m_shaderCode, m_alloc);
	m_shader = reinterpret_cast<T*>(m_shaderHeader.data());
	m_shader->patchShaderGpuAddress(m_shaderCode.get());
	_generateInputOffsetsCache<T>(&m_lcueOffsetsTable, &m_cueOffsetsTable, m_shader);
	const Gnm::InputUsageSlot *pUsageSlots = m_shader->getInputUsageSlotTable();
	for (int i = 0; i < m_shader->m_common.m_numInputUsageSlots; i++)
	{
		if (pUsageSlots[i].m_usageType == Gnm::kShaderInputUsageImmShaderResourceTable)
		{
			m_srtSizeInDW =  pUsageSlots[i].m_srtSizeInDWordMinusOne + 1;
			break;
		}
	}
}
} } } // namespace sce::SampleUtil::Graphics
#endif