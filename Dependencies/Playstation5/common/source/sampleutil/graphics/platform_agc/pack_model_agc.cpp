/* SIE CONFIDENTIAL
 PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2025 Sony Interactive Entertainment Inc.
 * 
 */
#ifndef SCE_SAMPLE_UTIL_DISABLE_MODEL_RENDER
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/core.h>
#include <sampleutil/debug/perf.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/graphics.h>
#include <sampleutil/graphics/pack_model.h>
#include <sampleutil/graphics/platform_agc/link_libraries_agc.h>
#include <sampleutil/graphics/default_pack_model_shaders.h>

using namespace sce::Vectormath::Simd::Aos;

namespace {

struct VertexBufferSpec
{
	std::vector<sce::Agc::Core::VertexAttribute>			*m_pAttributes;
	sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags	m_flags;
};

struct ConstVertexBufferSpec
{
	const std::vector<sce::Agc::Core::VertexAttribute>		*m_pAttributes;
	sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags	m_flags;
};

int	getValidVertexAttributes(VertexBufferSpec	&validVertexBufferSpec, const ConstVertexBufferSpec	&meshVertexBufferSpec)
{
	int	vert_i = 0;
	validVertexBufferSpec.m_pAttributes->clear();

	// position
	validVertexBufferSpec.m_pAttributes->push_back((*meshVertexBufferSpec.m_pAttributes)[vert_i]);
	++vert_i;

	// normal
	if (validVertexBufferSpec.m_flags.m_hasNormal && meshVertexBufferSpec.m_flags.m_hasNormal) {
		SCE_SAMPLE_UTIL_ASSERT(meshVertexBufferSpec.m_pAttributes->size() > vert_i);
		if (meshVertexBufferSpec.m_pAttributes->size() == vert_i) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		validVertexBufferSpec.m_pAttributes->push_back((*meshVertexBufferSpec.m_pAttributes)[vert_i]);
	}
	vert_i += meshVertexBufferSpec.m_flags.m_hasNormal;

	// tangent
	if (validVertexBufferSpec.m_flags.m_hasTangent && meshVertexBufferSpec.m_flags.m_hasTangent) {
		SCE_SAMPLE_UTIL_ASSERT(meshVertexBufferSpec.m_pAttributes->size() > vert_i);
		if (meshVertexBufferSpec.m_pAttributes->size() == vert_i) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		validVertexBufferSpec.m_pAttributes->push_back((*meshVertexBufferSpec.m_pAttributes)[vert_i]);
	}
	vert_i += meshVertexBufferSpec.m_flags.m_hasTangent;

	// color
	if (validVertexBufferSpec.m_flags.m_hasColor && meshVertexBufferSpec.m_flags.m_hasColor) {
		SCE_SAMPLE_UTIL_ASSERT(meshVertexBufferSpec.m_pAttributes->size() > vert_i);
		if (meshVertexBufferSpec.m_pAttributes->size() == vert_i) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		validVertexBufferSpec.m_pAttributes->push_back((*meshVertexBufferSpec.m_pAttributes)[vert_i]);
	}
	vert_i += meshVertexBufferSpec.m_flags.m_hasColor;

	// uv
	const uint32_t	numUvs = (validVertexBufferSpec.m_flags.m_uvCount < meshVertexBufferSpec.m_flags.m_uvCount) ? validVertexBufferSpec.m_flags.m_uvCount : meshVertexBufferSpec.m_flags.m_uvCount;
	for (int i = 0; i < numUvs; i++) {
		SCE_SAMPLE_UTIL_ASSERT(meshVertexBufferSpec.m_pAttributes->size() > vert_i + i);
		if (meshVertexBufferSpec.m_pAttributes->size() == vert_i + i) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		validVertexBufferSpec.m_pAttributes->push_back((*meshVertexBufferSpec.m_pAttributes)[vert_i + i]);
	}
	vert_i += meshVertexBufferSpec.m_flags.m_uvCount;

	// bones
	if (validVertexBufferSpec.m_flags.m_hasSkinning && meshVertexBufferSpec.m_flags.m_hasSkinning) {
		SCE_SAMPLE_UTIL_ASSERT(meshVertexBufferSpec.m_pAttributes->size() > vert_i + 1);
		if (meshVertexBufferSpec.m_pAttributes->size() <= vert_i + 1) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		validVertexBufferSpec.m_pAttributes->push_back((*meshVertexBufferSpec.m_pAttributes)[vert_i]); // bone indices
		validVertexBufferSpec.m_pAttributes->push_back((*meshVertexBufferSpec.m_pAttributes)[vert_i + 1]); // bone weights
	}

	return	SCE_OK;
}

} // anonymous namespace

namespace sce {	namespace SampleUtil { namespace Graphics {

void	PackModelDrawContext::beginDraw(sce::Agc::DrawCommandBuffer	&dcb, sce::Agc::Core::RingBuffer	*rb, Mesh::VertexAttributeFlags	validVertexAttributes)
{
	// clear states
	m_binder.init(&dcb, &dcb, rb);
	for (int type = 0; type < (int)sce::Agc::ShaderType::kLegacyCount; type++)
	{
		m_ppBoundShaders[type] = nullptr;
	}
	m_activeStages				= 0;
	m_forceCullFace				= (sce::Agc::CxPrimitiveSetup::CullFace)-1;
	m_prevFrontFace				= (sce::Agc::CxPrimitiveSetup::FrontFace)-1;
	m_prevCullFace				= (sce::Agc::CxPrimitiveSetup::CullFace)-1;
	m_pPrevPsShader				= (sce::Agc::Shader*) - 1;
	m_prevIsWireframe			= false;
	m_renderMode				= RenderMode::kNormal;
	m_isGlobalTableSet			= false;
	m_indexElementSize			= 0;
	m_primitiveType				= sce::Agc::UcPrimitiveType::Type::kNone;
	m_numInstances				= 0;
	m_validVertexAttributeFlags	= validVertexAttributes;

	// set gpu states
	dcb.setIndexSize(sce::Agc::IndexSize::k16);
}

void	PackModelDrawContext::endDraw(sce::Agc::DrawCommandBuffer &dcb)
{
	dcb.setNumInstances(1);
	dcb.setIndexSize(sce::Agc::IndexSize::k16);
	m_binder.reset();
	m_binder.fini();
}

int	PackModelDrawContext::setLsHsShader(sce::Agc::Core::StateBuffer	&sb, const sce::Agc::Shader	*pLs, const sce::Agc::Shader	*pHs, const sce::Agc::Shader	*pFusedShader)
{
	SCE_SAMPLE_UTIL_ASSERT(pLs != nullptr && pHs != nullptr && pFusedShader != nullptr);
	if (pLs == nullptr || pHs == nullptr || pFusedShader == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	m_ppBoundShaders[(int)sce::Agc::ShaderType::kHsFront] = pLs;
	m_ppBoundShaders[(int)sce::Agc::ShaderType::kHsBack] = pHs;
	m_ppBoundShaders[(int)sce::Agc::ShaderType::kHs] = pFusedShader;
	m_activeStages |= (1u << (int)sce::Agc::ShaderType::kHsFront) | (1u << (int)sce::Agc::ShaderType::kHsBack);
	m_activeStages &=~(1u << (int)sce::Agc::ShaderType::kHs);
	sb.setShader(pFusedShader);

	return SCE_OK;
}

int	PackModelDrawContext::setEsGsShader(sce::Agc::Core::StateBuffer	&sb, const sce::Agc::Shader	*pEs, const sce::Agc::Shader	*pGs, const sce::Agc::Shader	*pFusedShader)
{
	SCE_SAMPLE_UTIL_ASSERT(pEs != nullptr && pGs != nullptr && pFusedShader != nullptr);
	if (pEs == nullptr || pGs == nullptr || pFusedShader == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	m_ppBoundShaders[(int)sce::Agc::ShaderType::kGsFront] = pEs;
	m_ppBoundShaders[(int)sce::Agc::ShaderType::kGsBack] = pGs;
	m_ppBoundShaders[(int)sce::Agc::ShaderType::kGs] = pFusedShader;
	m_activeStages |= (1u << (int)sce::Agc::ShaderType::kGsFront) | (1u << (int)sce::Agc::ShaderType::kGsBack);
	m_activeStages &=~(1u << (int)sce::Agc::ShaderType::kGs);
	sb.setShader(pFusedShader);

	return SCE_OK;
}

int	PackModelDrawContext::setVsShader(sce::Agc::Core::StateBuffer	&sb, const sce::Agc::Shader	*pVs)
{
	SCE_SAMPLE_UTIL_ASSERT(pVs != nullptr);
	if (pVs == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	m_ppBoundShaders[(int)sce::Agc::ShaderType::kGsFront] = nullptr;
	m_ppBoundShaders[(int)sce::Agc::ShaderType::kGsBack] = nullptr;
	m_ppBoundShaders[(int)sce::Agc::ShaderType::kGs] = pVs;
	m_activeStages &= ~((1u << (int)sce::Agc::ShaderType::kGsFront) | (1u << (int)sce::Agc::ShaderType::kGsBack));
	m_activeStages |= (1u << (int)sce::Agc::ShaderType::kGs);
	sb.setShader(pVs);

	return SCE_OK;
}

sce::Agc::Toolkit::Result	PackModelDrawContext::drawMesh(sce::Agc::DrawCommandBuffer	&dcb, Agc::Core::StateBuffer	&sb, const std::array<const sce::Agc::Shader *, 2>	&psShaders, const void	*pCustomUserData, bool	isWireframe,
															const PackModel	&packModel, const PackModelInstanceUserData	*pPackModelInstanceUserData, uint32_t	numPackModelInstances, uint32_t	meshIndex, uint32_t	instanceIndex, bool	useInstancing,
															const void	*pIndexBuffer, uint32_t	indexElementSizeInBytes, uint32_t	indexCount)
{
	int ret = SCE_OK; (void)ret;

	sce::Agc::Toolkit::Result result = {};

	const Mesh					&mesh					= packModel.m_meshes[meshIndex];
	const RegularBuffer<MeshInstanceUserData>	meshInstanceUserDataBuffer = packModel.m_gpuInstanceUserDataArray[meshIndex];
	const MeshInstanceUserData	&meshInstanceUserData	= reinterpret_cast<MeshInstanceUserData *>(meshInstanceUserDataBuffer.getDataAddress())[instanceIndex];
	const uint32_t				numMeshInstances		= useInstancing ? meshInstanceUserDataBuffer.getNumElements() : 1;
	const uint32_t				numInstances			= numPackModelInstances * numMeshInstances;
	const ShaderMaterial		&material				= packModel.m_materialsMemory[meshInstanceUserData.m_materialIndex];

	if (m_numInstances != numInstances) {
		m_numInstances = numInstances;
		dcb.setNumInstances(m_numInstances);
	}

	if (m_indexElementSize != indexElementSizeInBytes) {
		m_indexElementSize = indexElementSizeInBytes;
		dcb.setIndexSize((m_indexElementSize == 1) ? sce::Agc::IndexSize::k8 : (m_indexElementSize == 2) ? sce::Agc::IndexSize::k16 : sce::Agc::IndexSize::k32);
	}

	if (m_primitiveType != mesh.m_primType) {
		m_primitiveType = mesh.m_primType;
		sb.setState(sce::Agc::UcPrimitiveType().init().setType(m_primitiveType));
	}

	sce::Agc::CxPrimitiveSetup ps;
	ps.init()
		.setFrontFace(!m_isMirrorImage ? sce::Agc::CxPrimitiveSetup::FrontFace::kCcw : sce::Agc::CxPrimitiveSetup::FrontFace::kCw)
		.setCullFace((m_forceCullFace != (sce::Agc::CxPrimitiveSetup::CullFace)-1) ? m_forceCullFace :
						material.m_isTwoSided ? sce::Agc::CxPrimitiveSetup::CullFace::kNone : sce::Agc::CxPrimitiveSetup::CullFace::kBack)
		.setFrontPolygonOffset(isWireframe ? sce::Agc::CxPrimitiveSetup::FrontPolygonOffset::kEnable : sce::Agc::CxPrimitiveSetup::FrontPolygonOffset::kDisable)
		.setBackPolygonOffset(isWireframe ? sce::Agc::CxPrimitiveSetup::BackPolygonOffset::kEnable : sce::Agc::CxPrimitiveSetup::BackPolygonOffset::kDisable);
	if (m_prevFrontFace != ps.getFrontFace() || m_prevCullFace != ps.getCullFace() || m_prevIsWireframe != isWireframe) {
		sb.setState(ps);
		if (m_prevIsWireframe != isWireframe) {
			sb.setState(sce::Agc::CxPolygonOffsetFront().init().setOffset(isWireframe ? -100.f : 0.f))
				.setState(sce::Agc::CxPolygonOffsetBack().init().setOffset(isWireframe ? -100.f : 0.f));
		}
	}
	m_prevFrontFace = ps.getFrontFace(); m_prevCullFace = ps.getCullFace(); m_prevIsWireframe = isWireframe;

	const sce::Agc::Shader *pPsShader = psShaders[material.m_enableAlphaTest ? 1 : 0];
	if (!isWireframe) {
		if (pPsShader != m_pPrevPsShader) {
			sce::Agc::CxShaderLinkage cxLinkage;
			sce::Agc::UcPrimitiveState ucLinkage;
			sce::Agc::Core::linkShaders(&cxLinkage, &ucLinkage, m_ppBoundShaders[(int)sce::Agc::ShaderType::kHs], m_ppBoundShaders[(int)sce::Agc::ShaderType::kGs], pPsShader, m_primitiveType);
			sb.setState(cxLinkage)
				.setState(ucLinkage)
				.setShader(pPsShader)
				.setState(sce::Agc::UcIndexOffset().init());

			result.m_activeWork |= sce::Agc::Toolkit::Result::ActiveWork::kGs;
			result.m_changes |= sce::Agc::Toolkit::Result::StateChange::kDraw;
			result.m_dirtyCaches |= sce::Agc::Toolkit::Result::Caches::kDbData | sce::Agc::Toolkit::Result::Caches::kGl1 | sce::Agc::Toolkit::Result::Caches::kGl2;
			if (pPsShader == nullptr) {
				sb.setState(sce::Agc::CxDbShaderControl().init())
					.setState(sce::Agc::CxShaderOutputMask().init());
			} else {
				result.m_activeWork |= sce::Agc::Toolkit::Result::ActiveWork::kPs;
				result.m_changes |= sce::Agc::Toolkit::Result::StateChange::kPsSh;
				result.m_dirtyCaches |= sce::Agc::Toolkit::Result::Caches::kCbData | sce::Agc::Toolkit::Result::Caches::kGl1 | sce::Agc::Toolkit::Result::Caches::kGl2;
			}
			sce::Agc::ShaderType frontMostGsStage = sce::Agc::ShaderType::kGs;
			if (m_ppBoundShaders[(int)sce::Agc::ShaderType::kHsFront] != nullptr && m_ppBoundShaders[(int)sce::Agc::ShaderType::kHsBack] != nullptr) {
				if (m_ppBoundShaders[(int)sce::Agc::ShaderType::kGsFront] != nullptr && m_ppBoundShaders[(int)sce::Agc::ShaderType::kGsBack] != nullptr) {
					m_binder.setLsHsEsGsPsShaders(m_ppBoundShaders[(int)sce::Agc::ShaderType::kHsFront], m_ppBoundShaders[(int)sce::Agc::ShaderType::kHsBack], m_ppBoundShaders[(int)sce::Agc::ShaderType::kGsFront], m_ppBoundShaders[(int)sce::Agc::ShaderType::kGsBack], pPsShader);
					frontMostGsStage = sce::Agc::ShaderType::kGsFront;
				} else {
					m_binder.setLsHsVsPsShaders(m_ppBoundShaders[(int)sce::Agc::ShaderType::kHsFront], m_ppBoundShaders[(int)sce::Agc::ShaderType::kHsBack], m_ppBoundShaders[(int)sce::Agc::ShaderType::kGs], pPsShader);
				}
				result.m_changes |= sce::Agc::Toolkit::Result::StateChange::kHsSh | sce::Agc::Toolkit::Result::StateChange::kGsSh | sce::Agc::Toolkit::Result::StateChange::kPsSh;
			} else if (m_ppBoundShaders[(int)sce::Agc::ShaderType::kGsFront] != nullptr && m_ppBoundShaders[(int)sce::Agc::ShaderType::kGsBack] != nullptr) {
				m_binder.setEsGsPsShaders(m_ppBoundShaders[(int)sce::Agc::ShaderType::kGsFront], m_ppBoundShaders[(int)sce::Agc::ShaderType::kGsBack], pPsShader);
				frontMostGsStage = sce::Agc::ShaderType::kGsFront;
				result.m_changes |= sce::Agc::Toolkit::Result::StateChange::kGsSh | sce::Agc::Toolkit::Result::StateChange::kPsSh;
			} else {
				m_binder.setVsPsShaders(m_ppBoundShaders[(int)sce::Agc::ShaderType::kGs], pPsShader);
				result.m_changes |= sce::Agc::Toolkit::Result::StateChange::kGsSh | sce::Agc::Toolkit::Result::StateChange::kPsSh;
			}
			m_binder.getStage(frontMostGsStage).setGsFlags(sce::Agc::GsFlags::kCullOffScreen | sce::Agc::GsFlags::kOglClipSpace | sce::Agc::GsFlags::kCullBack);
		}
		m_pPrevPsShader = pPsShader;
	}

	// pixel shader resource setup
	if (!isWireframe && pPsShader != nullptr) {
		struct PsUserData
		{
			RegularBuffer<ShaderMaterial>		m_materialBuffer;
			RegularBuffer<MeshInstanceUserData>	m_meshInstanceBuffer;
			const void							*m_pCustomUserData;
		} psUserData =
		{
			packModel.m_materialBuffer,
			packModel.m_gpuInstanceUserDataArray[meshIndex],
			pCustomUserData
		};
		const uint32_t	srtSizeInDw = sce::Agc::Core::getResourceUserDataRange(pPsShader, sce::Agc::UserDataLayout::DirectResourceType::kShaderResourceTable).size();
		m_binder.getStage(sce::Agc::ShaderType::kPs).setUserSrtBuffer(&psUserData, srtSizeInDw);
		resolveShaderMaterial(packModel.m_materialsMemory.get()[meshInstanceUserData.m_materialIndex], *packModel.m_pTextureLibrary);
	}

	m_binder.beginPatch(m_activeStages);
	{
#ifdef _DEBUG
		Debug::ScopedPerfOf<sce::Agc::DrawCommandBuffer> perf(&dcb, mesh.m_name);
#endif
		// geometry shaders user data setup
		struct GeomUserData
		{
			RegularBuffer<PackModelInstanceUserData>	m_packModelInstanceBuffer;
			RegularBuffer<MeshInstanceUserData>			m_meshInstanceBuffer;
			const void *m_pCustomUserData;
			uint										m_instanceBaseIndex;
			uint										m_numMeshInstances;
		}	geomUserData;
		ret = sce::Agc::Core::initialize(&geomUserData.m_packModelInstanceBuffer, &sce::Agc::Core::BufferSpec().initAsRegularBuffer(pPackModelInstanceUserData, sizeof(PackModelInstanceUserData), numPackModelInstances));
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		geomUserData.m_meshInstanceBuffer		= packModel.m_gpuInstanceUserDataArray[meshIndex];
		geomUserData.m_pCustomUserData			= pCustomUserData;
		geomUserData.m_instanceBaseIndex		= instanceIndex;
		geomUserData.m_numMeshInstances			= numMeshInstances;

		// set user data
		for (int type = 0; type < (int)Agc::ShaderType::kLegacyCount; type++) {
			if (m_ppBoundShaders[type] != nullptr &&
				(type != (int)Agc::ShaderType::kGs || m_ppBoundShaders[(int)Agc::ShaderType::kGsFront] == nullptr) &&
				(type != (int)Agc::ShaderType::kHs || m_ppBoundShaders[(int)Agc::ShaderType::kHsFront] == nullptr))
			{
				const uint32_t	srtSizeInDw = sce::Agc::Core::getResourceUserDataRange(m_ppBoundShaders[type], sce::Agc::UserDataLayout::DirectResourceType::kShaderResourceTable).size();
				m_binder.getStage((sce::Agc::ShaderType)type).setUserSrtBuffer(&geomUserData, srtSizeInDw);
			}
		}

		// set internal global table for tessellation
		if (!m_isGlobalTableSet && m_ppBoundShaders[(int)sce::Agc::ShaderType::kHsFront] != nullptr && m_ppBoundShaders[(int)sce::Agc::ShaderType::kHsBack] != nullptr) {
			m_binder.getStage(sce::Agc::ShaderType::kHsBack).setInternalGlobalTable(m_pGlobalTablePtr);
			m_binder.getStage(sce::Agc::ShaderType::kGs).setInternalGlobalTable(m_pGlobalTablePtr);
			m_isGlobalTableSet = true;
		}

		thread_local static std::vector<sce::Agc::Core::VertexAttribute>	validVertexAttributes;
		thread_local static uint64_t										prevVertexAttrsHash[4] = { 0ul, 0ul, 0ul, 0ul };
		if (prevVertexAttrsHash[0] != mesh.m_vertexAttrsHash[0] || prevVertexAttrsHash[1] != mesh.m_vertexAttrsHash[1] || prevVertexAttrsHash[2] != mesh.m_vertexAttrsHash[2] || prevVertexAttrsHash[3] != mesh.m_vertexAttrsHash[3]) {
			// get valid vertex attributes specfied by valid vertex attribute flags
			VertexBufferSpec			validVertexBufferSpec	= { &validVertexAttributes, m_validVertexAttributeFlags };
			const ConstVertexBufferSpec	meshVertexBufferSpec	= { &mesh.m_vertexAttributes, mesh.m_vertexAttributeFlags };
			ret = getValidVertexAttributes(validVertexBufferSpec, meshVertexBufferSpec);
			if (ret != SCE_OK) {
				result.m_errorCode = ret;
				return	result;
			}
			prevVertexAttrsHash[0] = mesh.m_vertexAttrsHash[0]; prevVertexAttrsHash[1] = mesh.m_vertexAttrsHash[1]; prevVertexAttrsHash[2] = mesh.m_vertexAttrsHash[2]; prevVertexAttrsHash[3] = mesh.m_vertexAttrsHash[3];
		}
		// set vertex buffers
		m_binder.getStage(m_binder.getFrontmostStage())
			.setVertexAttributes(0, validVertexAttributes.size(), validVertexAttributes.data())
			.setVertexBuffers(0, mesh.m_vertexBuffers.size(), mesh.m_vertexBuffers.data());
		if (!useInstancing) {
			// reserved dcb to prevent draw packet and PackModelInstanceUserData from existing in separate segments
			const uint32_t reserveSizeInBytes = sce::Agc::DrawCommandBuffer::drawIndexSize() + sizeof(PackModelInstanceUserData) + 16;
			dcb.reserveSpaceInBytes(reserveSizeInBytes);
		}
		sce::Agc::SinglePacketAddress drawPacket = dcb.drawIndex(indexCount, pIndexBuffer, m_binder.getStage(m_binder.getFrontmostStage()).getShader()->m_specials->m_drawModifier);
		if ((uint32_t)m_renderMode & (uint32_t)RenderMode::kConditional) {
			sce::Agc::setPacketPredication(drawPacket, sce::Agc::Predication::kSkipWhenPredicated);
		}
		if (!useInstancing) {
			// In case of non-instancing mode, we allocate and copy PackModelInstanceUserData to GPU accessible memory.
			// Here we allocate memory from TwoSidedAllocator(or draw command buffer), so we need to allocate memory 'after draw', and patch V# data address with it to prevent SRT user data backward reference which could cause problem when using with Agc::Core::RingBuffer.
			auto *pPackModelInstanceGpuUserData = reinterpret_cast<PackModelInstanceUserData *>(dcb.allocateTopDown(sizeof(PackModelInstanceUserData), sce::Agc::Alignment::kBuffer));
			memcpy(pPackModelInstanceGpuUserData, pPackModelInstanceUserData, sizeof(PackModelInstanceUserData));
			m_binder.patch(offsetof(GeomUserData, m_packModelInstanceBuffer), pPackModelInstanceGpuUserData, true);
		}
	}
	m_binder.endPatch();

	sb.postDraw();
	m_binder.postDraw();

	return	result;
}

sce::Agc::Toolkit::Result	PackModelDrawContext::draw(sce::Agc::DrawCommandBuffer	&dcb, Agc::Core::StateBuffer	&sb, sce::Agc::Toolkit::RestoreCxState	restoreCxState,
														PackModelInstance	&packModelInstance, const std::array<const sce::Agc::Shader *, 2>	&psShaders, const void	*pCustomUserData, bool	useInstancing, bool	isWireframe)
{
	int ret = SCE_OK; (void)ret;

	sce::Agc::Toolkit::Result result = {};

	if (!packModelInstance.m_isVisible) {
		return result;
	}

	SCE_SAMPLE_UTIL_ASSERT(packModelInstance.m_pPackModel != nullptr);
	if (packModelInstance.m_pPackModel == nullptr) {
		result.m_errorCode = SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		return result;
	}

	// Conditionally save the Cx state.
	if (restoreCxState == sce::Agc::Toolkit::RestoreCxState::kEnable) {
		sb.postDraw(); // Ensure we're not appending to a previous indirect state packet.
		dcb.contextStateOp(Agc::ContextStateOperation::kPushState); // Just push, do not clear.
	}

#ifdef _DEBUG
	Debug::ScopedPerfOf<sce::Agc::DrawCommandBuffer> perf(&dcb, packModelInstance.m_name);
#endif

	PackModel	&packModel = *packModelInstance.m_pPackModel;
	packModel.resolve();

	PackModelInstanceUserData	packModelInstanceUserData; // Later this is copied to memory allocated by TwoSidedAllocator, and V# data address in SRT is patched with it.
	packModelInstanceUserData.m_transform	= packModelInstance.m_modelMatrix;
	sce::Agc::Core::BufferSpec	skinningMatrixBufferArraySpec;
	skinningMatrixBufferArraySpec.initAsRegularBuffer(packModelInstance.m_pSkinningMatrixBuffers, sizeof(sce::Agc::Core::Buffer), packModelInstance.m_animInstances.size());
	ret = sce::Agc::Core::initialize(&packModelInstanceUserData.m_skinningMatricesArray, &skinningMatrixBufferArraySpec);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	if ((uint32_t)m_renderMode & (uint32_t)RenderMode::kOcclusionQuery) {
		SCE_SAMPLE_UTIL_ASSERT(packModelInstance.m_occlusionQueryResults.get() != nullptr);
		if (packModelInstance.m_occlusionQueryResults.get() == nullptr) {
			result.m_errorCode = SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
			return result;
		}
		// clear occlusion query result
		dcb.reserveSpaceInDwords(packModelInstance.m_drawOrder.size() * sizeof(sce::Agc::OcclusionQueryResults) + 0x1c/*=dmaData*/);
		void *pZeroBuffer = dcb.allocateTopDown(packModelInstance.m_drawOrder.size() * sizeof(sce::Agc::OcclusionQueryResults), 4);
		memset(pZeroBuffer, 0, packModelInstance.m_drawOrder.size() * sizeof(sce::Agc::OcclusionQueryResults));
		dcb.dmaData(sce::Agc::CpEngine::kMe, sce::Agc::DmaDataDst::kGl2, sce::Agc::CachePolicy::kLru, (uint64_t)packModelInstance.m_occlusionQueryResults.get(),
			sce::Agc::DmaDataSrc::kGl2, sce::Agc::CachePolicy::kLru, (uint64_t)pZeroBuffer, packModelInstance.m_drawOrder.size() * sizeof(sce::Agc::OcclusionQueryResults),
			sce::Agc::DmaDataWaitForPreviousDmas::kDisable, sce::Agc::DmaDataWriteConfirm::kEnable, sce::Agc::DmaDataBlockCpEngine::kEnable);
	}

	for (uint32_t i = 0; i < packModel.m_instances.size(); ++i)
	{
		const int	instance_i = (packModelInstance.m_drawOrder.size() > 0) ? packModelInstance.m_drawOrder[i] : i;
		const MeshInstance &meshInstance = packModel.m_instances[instance_i];

		if (!meshInstance.m_isVisible) {
			continue;
		}
#ifdef _DEBUG
		Debug::ScopedPerfOf<sce::Agc::DrawCommandBuffer> perf(&dcb, meshInstance.m_name);
#endif
		SCE_SAMPLE_UTIL_ASSERT(((uint32_t)m_renderMode & ((uint32_t)RenderMode::kOcclusionQuery | (uint32_t)RenderMode::kConditional)) != ((uint32_t)RenderMode::kOcclusionQuery | (uint32_t)RenderMode::kConditional));
		if ((uint32_t)m_renderMode & (uint32_t)RenderMode::kOcclusionQuery) {
			SCE_SAMPLE_UTIL_ASSERT(packModelInstance.m_occlusionQueryResults.get() != nullptr);
			if (packModelInstance.m_occlusionQueryResults.get() == nullptr) {
				result.m_errorCode = SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
				return result;
			}
			dcb.beginOcclusionQuery(sce::Agc::OcclusionQueryClear::kDisable, &packModelInstance.m_occlusionQueryResults.get()[instance_i]);
		}

		if ((uint32_t)m_renderMode & (uint32_t)RenderMode::kConditional) {
			SCE_SAMPLE_UTIL_ASSERT(packModelInstance.m_occlusionQueryResults.get() != nullptr);
			if (packModelInstance.m_occlusionQueryResults.get() == nullptr) {
				result.m_errorCode = SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
				return result;
			}
			dcb.setZPassPredicationEnable(sce::Agc::PredicationZPassCondition::kSkipIfNotVisible,
				sce::Agc::PredicationZPassWaitOp::kDoNotPredicateIfQueryResultsNotReady,
				&packModelInstance.m_occlusionQueryResults.get()[instance_i]
			);
		}

		result |= drawMesh(dcb, sb, psShaders, pCustomUserData, isWireframe, packModel, &packModelInstanceUserData, 1, meshInstance.m_meshIndex, meshInstance.m_instanceIndex, false,
							packModelInstance.getIndexBuffer(meshInstance.m_meshIndex), packModelInstance.getIndexElementSizeInBytes(meshInstance.m_meshIndex), packModelInstance.getIndexCount(meshInstance.m_meshIndex));

		if ((uint32_t)m_renderMode & (uint32_t)RenderMode::kConditional) {
			dcb.setPredicationDisable();
		}
		if ((uint32_t)m_renderMode & (uint32_t)RenderMode::kOcclusionQuery) {
			dcb.endOcclusionQuery(&packModelInstance.m_occlusionQueryResults.get()[instance_i]);
		}
	}

	if (restoreCxState == sce::Agc::Toolkit::RestoreCxState::kEnable) {
		dcb.contextStateOp(sce::Agc::ContextStateOperation::kPopState);
	}

	return	result;
}

sce::Agc::Toolkit::Result	PackModelDrawContext::drawInstanced(sce::Agc::DrawCommandBuffer &dcb, Agc::Core::StateBuffer &sb, sce::Agc::Toolkit::RestoreCxState	restoreCxState,
																PackModel &packModel, const PackModelInstanceUserData	*pPackModelInstanceUserData, uint32_t	numInstances, const std::array<const sce::Agc::Shader *, 2> &psShaders, const void *pCustomUserData, bool	useInstancing, bool	isWireframe)
{
	int ret = SCE_OK; (void)ret;

	sce::Agc::Toolkit::Result result = {};

	SCE_SAMPLE_UTIL_ASSERT_MSG(((uint32_t)m_renderMode & ((uint32_t)RenderMode::kOcclusionQuery | (uint32_t)RenderMode::kConditional)) == 0u, "Conditional rendering won't work for instanced draw");

	// Conditionally save the Cx state.
	if (restoreCxState == sce::Agc::Toolkit::RestoreCxState::kEnable) {
		sb.postDraw(); // Ensure we're not appending to a previous indirect state packet.
		dcb.contextStateOp(Agc::ContextStateOperation::kPushState); // Just push, do not clear.
	}

#ifdef _DEBUG
	Debug::ScopedPerfOf<sce::Agc::DrawCommandBuffer> perf(&dcb, packModel.m_name);
#endif

	packModel.resolve();

	// prim setup
	sce::Agc::CxPrimitiveSetup ps;
	ps.init();
	ps.setFrontFace(!m_isMirrorImage ? sce::Agc::CxPrimitiveSetup::FrontFace::kCcw : sce::Agc::CxPrimitiveSetup::FrontFace::kCw);

	const bool isForceCullMode = (m_forceCullFace != (sce::Agc::CxPrimitiveSetup::CullFace)-1);
	ps.setCullFace(isForceCullMode ? m_forceCullFace : sce::Agc::CxPrimitiveSetup::CullFace::kBack);
	if (m_prevFrontFace != ps.getFrontFace() || m_prevCullFace != ps.getCullFace() || m_prevIsWireframe != isWireframe) {
		sb.setState(ps);
	}
	m_prevFrontFace = ps.getFrontFace(); m_prevCullFace = ps.getCullFace(); m_prevIsWireframe = isWireframe;

	if (isWireframe) {
		ps.setFrontPolygonOffset(sce::Agc::CxPrimitiveSetup::FrontPolygonOffset::kEnable).setBackPolygonOffset(sce::Agc::CxPrimitiveSetup::BackPolygonOffset::kEnable);
		sb.setState(sce::Agc::CxPolygonOffsetFront().init().setOffset(-100.f));
		sb.setState(sce::Agc::CxPolygonOffsetBack().init().setOffset(-100.f));
	}

	for (uint32_t meshIdx = 0; meshIdx < packModel.m_meshes.size(); meshIdx++) {
		const auto		&mesh		= packModel.m_meshes[meshIdx];
		const uint32_t	indexCount	= mesh.m_indexSizeInBytes / mesh.m_indexElementSizeInBytes;
		result	|= drawMesh(dcb, sb, psShaders, pCustomUserData, isWireframe, packModel, pPackModelInstanceUserData, numInstances, meshIdx, 0, true,
							mesh.m_pIndexBuffer, mesh.m_indexElementSizeInBytes, indexCount);
	}

	if (restoreCxState == sce::Agc::Toolkit::RestoreCxState::kEnable) {
		dcb.contextStateOp(sce::Agc::ContextStateOperation::kPopState);
	}

	return	result;
}

void	initializePackModelDraw()
{
	initializeDefaultPackModelShaders();
}

void	finalizePackModelDraw()
{
}

sce::Agc::Toolkit::Result	drawPackModelInternal(sce::Agc::DrawCommandBuffer	*dcb, sce::Agc::Core::StateBuffer	*sb, sce::Agc::Toolkit::RestoreCxState	restoreCxState, sce::Agc::Core::RingBuffer	*rb,
													PackModel	&packModel, SceLibcMspace	edgeAnimMemoryAlloc, VideoRingAllocator	&videoRingMemoryAlloc, const float	*pAnimationTimesInSec,
													const Matrix4	*pModelMatrices, uint32_t	numInstances, Matrix4_arg	viewMatrix, Matrix4_arg	projectionMatrix, Vector3_arg	lightPosition, float	ambient, float	shininess,
													const sce::Agc::Shader	*vs, const sce::Agc::Shader	*fillPs, const void	*fillUserData/* = nullptr*/,
													const std::vector<JointTransformDesignation>	*pAuxTransforms, bool	drawSkeleton)
{
	sce::Agc::Toolkit::Result	result = {};

	SCE_SAMPLE_UTIL_ASSERT(dcb != nullptr);
	SCE_SAMPLE_UTIL_ASSERT(sb != nullptr);
	if (dcb == nullptr || sb == nullptr) {
		result.m_errorCode	= SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
		return	result;
	}

	PackModelInstanceUserData	*pInstanceUserData = (PackModelInstanceUserData*)videoRingMemoryAlloc.allocate(sizeof(PackModelInstanceUserData) * numInstances, 16);
	const bool	hasAnimation	= (packModel.m_animations.size() > 0) && (edgeAnimMemoryAlloc != nullptr);
	std::vector<SkeletalAnimationInstance>	animInstances;
	if (hasAnimation) {
		for (auto& animation : packModel.m_animations) {
			animInstances.emplace_back(animation, edgeAnimMemoryAlloc, packModel.m_name);
		}
	}
	for (int i = 0; i < numInstances; i++) {
		pInstanceUserData[i].m_transform	= ToMatrix4x3Unaligned(Transform3(pModelMatrices[i]));
		if (hasAnimation) {
			// Update animation
			sce::Agc::Core::Buffer	*pSkinningMatricesBuffers	= reinterpret_cast<sce::Agc::Core::Buffer *>(videoRingMemoryAlloc.allocate(sizeof(sce::Agc::Core::Buffer) * packModel.m_animations.size(), sce::Agc::Alignment::kBuffer));
			sce::Agc::Core::initializeRegularBuffer(&pInstanceUserData[i].m_skinningMatricesArray, pSkinningMatricesBuffers,
													sizeof(sce::Agc::Core::Buffer), packModel.m_animations.size());
			for (int j = 0; j < packModel.m_animations.size(); j++) {
				auto	&animInstance	= animInstances[j];
				if (animInstance.m_pSkeletalAnimation != nullptr) {
					animInstance.evaluateSkinningMatrices(pAnimationTimesInSec[i], videoRingMemoryAlloc, (pAuxTransforms != nullptr) ? pAuxTransforms[i] : std::vector<JointTransformDesignation>{});
				} else {
					animInstance.setIdentitiesToSkinningMatrices(1, videoRingMemoryAlloc); // set one identity to avoid nullptr skinning matrix buffer
				}
				pSkinningMatricesBuffers[j]	= animInstance.m_skinningMatrixBuffer;
			}
		}
	}

	// Prepare SRT data
	struct SrtData
	{
		Matrix4Unaligned	m_viewProjectionMatrix;
		Vector3Unaligned	m_cameraPosition;
		Vector3Unaligned	m_lightPosition;
		float				m_ambient;
		float				m_shininess;
		int					m_hdr;
		const void			*m_fillUserData;
	}	*srtData	= reinterpret_cast<SrtData *>(videoRingMemoryAlloc.allocate(sizeof(SrtData), 16));
	srtData->m_viewProjectionMatrix	= projectionMatrix * viewMatrix;
	srtData->m_cameraPosition		= -viewMatrix.getTranslation();
	srtData->m_lightPosition		= lightPosition;
	srtData->m_ambient				= ambient;
	srtData->m_shininess			= shininess;
	srtData->m_hdr 					= (int)getHdr();
	srtData->m_fillUserData			= fillUserData;

	if (vs == nullptr) {
		// search for vertex shader matching with pack vertex attributes
		vs	= PackModel::getDefaultVertexShader(packModel, sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags(true, true, true, 2, hasAnimation));
		SCE_SAMPLE_UTIL_ASSERT(vs != nullptr);
		if (vs == nullptr) {
			result.m_errorCode	= SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			return	result;
		}
	}

	if (fillPs == nullptr) {
		// search for pixel shader matching with pack material
		fillPs	= PackModel::getDefaultPixelShader(packModel);
		SCE_SAMPLE_UTIL_ASSERT(fillPs != nullptr);
		if (fillPs == nullptr) {
			result.m_errorCode	= SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			return	result;
		}
	}

	// Draw
	PackModelDrawContext	drawContext;
	drawContext.beginDraw(*dcb, rb, sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags(true, true, true, 1, true));
	drawContext.setVsShader(*sb, vs);
	result	= drawContext.drawInstanced(*dcb, *sb, restoreCxState, packModel, pInstanceUserData, numInstances, { fillPs, fillPs }, srtData);
	drawContext.endDraw(*dcb);

	if (drawSkeleton && packModel.m_animations.size() > 0 && edgeAnimMemoryAlloc != nullptr) {
		sb->setState(sce::Agc::CxDepthStencilControl().init().setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable));
		sb->postDraw();
		for (int i = 0; i < numInstances; i++) {
			for (int j = 0; j < packModel.m_animations.size(); j++) {
				animInstances[j].drawJoints(*dcb, sce::Vectormath::Simd::Aos::Transform3(pModelMatrices[i]), sce::Vectormath::Simd::Aos::Transform3(viewMatrix), projectionMatrix);
			}
		}
		sb->setState(sce::Agc::CxDepthStencilControl().init().setDepth(sce::Agc::CxDepthStencilControl::Depth::kEnable).setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kLessEqual));
		sb->postDraw();
	}

	return	result;
}

} } } // namespace sce::SampleUtil::Graphics
#endif // _SCE_TARGET_OS_PROSPERO
#endif