/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#ifndef SCE_SAMPLE_UTIL_DISABLE_MODEL_RENDER
#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include <sampleutil/debug/perf.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/graphics.h>
#include <sampleutil/graphics/pack_model.h>
#include <sampleutil/graphics/default_pack_model_shaders.h>

using namespace sce::Vectormath::Simd::Aos;

#if SCE_GNMX_ENABLE_GFX_LCUE
#define CUE(gfxc) (gfxc).m_lwcue
#else
#define CUE(gfxc) (gfxc).m_cue
#endif

namespace
{

	struct VertexBufferSpec
	{
		std::vector<sce::Gnm::Buffer> *m_pVertexBuffers;
		sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags	m_flags;
	};

	struct ConstVertexBufferSpec
	{
		const std::vector<sce::Gnm::Buffer> *m_pVertexBuffers;
		sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags	m_flags;
	};

	int	getValidVertexBuffers(VertexBufferSpec &validVertexBufferSpec, const ConstVertexBufferSpec &meshVertexBufferSpec)
	{
		int	vert_i = 0;
		validVertexBufferSpec.m_pVertexBuffers->clear();

		// position
		validVertexBufferSpec.m_pVertexBuffers->push_back((*meshVertexBufferSpec.m_pVertexBuffers)[vert_i]);
		++vert_i;

		// normal
		if (validVertexBufferSpec.m_flags.m_hasNormal && meshVertexBufferSpec.m_flags.m_hasNormal) {
			SCE_SAMPLE_UTIL_ASSERT(meshVertexBufferSpec.m_pVertexBuffers->size() > vert_i);
			if (meshVertexBufferSpec.m_pVertexBuffers->size() == vert_i) {
				return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			validVertexBufferSpec.m_pVertexBuffers->push_back((*meshVertexBufferSpec.m_pVertexBuffers)[vert_i]);
		}
		vert_i += meshVertexBufferSpec.m_flags.m_hasNormal;

		// tangent
		if (validVertexBufferSpec.m_flags.m_hasTangent && meshVertexBufferSpec.m_flags.m_hasTangent) {
			SCE_SAMPLE_UTIL_ASSERT(meshVertexBufferSpec.m_pVertexBuffers->size() > vert_i);
			if (meshVertexBufferSpec.m_pVertexBuffers->size() == vert_i) {
				return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			validVertexBufferSpec.m_pVertexBuffers->push_back((*meshVertexBufferSpec.m_pVertexBuffers)[vert_i]);
		}
		vert_i += meshVertexBufferSpec.m_flags.m_hasTangent;

		// color
		if (validVertexBufferSpec.m_flags.m_hasColor && meshVertexBufferSpec.m_flags.m_hasColor) {
			SCE_SAMPLE_UTIL_ASSERT(meshVertexBufferSpec.m_pVertexBuffers->size() > vert_i);
			if (meshVertexBufferSpec.m_pVertexBuffers->size() == vert_i) {
				return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			validVertexBufferSpec.m_pVertexBuffers->push_back((*meshVertexBufferSpec.m_pVertexBuffers)[vert_i]);
		}
		vert_i += meshVertexBufferSpec.m_flags.m_hasColor;

		// uv
		const uint32_t	numUvs = (validVertexBufferSpec.m_flags.m_uvCount < meshVertexBufferSpec.m_flags.m_uvCount) ? validVertexBufferSpec.m_flags.m_uvCount : meshVertexBufferSpec.m_flags.m_uvCount;
		for (int i = 0; i < numUvs; i++) {
			SCE_SAMPLE_UTIL_ASSERT(meshVertexBufferSpec.m_pVertexBuffers->size() > vert_i + i);
			if (meshVertexBufferSpec.m_pVertexBuffers->size() == vert_i + i) {
				return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			validVertexBufferSpec.m_pVertexBuffers->push_back((*meshVertexBufferSpec.m_pVertexBuffers)[vert_i + i]);
		}
		vert_i += meshVertexBufferSpec.m_flags.m_uvCount;

		// bones
		if (validVertexBufferSpec.m_flags.m_hasSkinning && meshVertexBufferSpec.m_flags.m_hasSkinning) {
			SCE_SAMPLE_UTIL_ASSERT(meshVertexBufferSpec.m_pVertexBuffers->size() > vert_i + 1);
			if (meshVertexBufferSpec.m_pVertexBuffers->size() <= vert_i + 1) {
				return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			validVertexBufferSpec.m_pVertexBuffers->push_back((*meshVertexBufferSpec.m_pVertexBuffers)[vert_i]); // bone indices
			validVertexBufferSpec.m_pVertexBuffers->push_back((*meshVertexBufferSpec.m_pVertexBuffers)[vert_i + 1]); // bone weights
		}

		return	SCE_OK;
	}

} // anonymous namespace

namespace sce {	namespace SampleUtil { namespace Graphics {

void	PackModelDrawContext::beginDraw(Gnmx::GnmxGfxContext	&gfxc, Mesh::VertexAttributeFlags	validVertexAttributes)
{
	// clear states
	m_forceCullFace				= (sce::Gnm::PrimitiveSetupCullFaceMode)-1;
	m_prevFrontFace				= (sce::Gnm::PrimitiveSetupFrontFace)-1;
	m_prevCullFace				= (sce::Gnm::PrimitiveSetupCullFaceMode)-1;
	m_pPrevPsShader				= (Shader<sce::Gnmx::PsShader>*) - 1;
	m_prevIsWireframe			= false;
	m_renderMode				= RenderMode::kNormal;
	m_indexElementSize			= 0;
	m_primitiveType				= sce::Gnm::kPrimitiveTypeNone;
	m_numInstances				= 0;
	m_validVertexAttributeFlags	= validVertexAttributes;

	// set gpu states
	gfxc.setIndexSize(sce::Gnm::kIndexSize16);
}

void	PackModelDrawContext::endDraw(sce::Gnmx::GnmxGfxContext	&gfxc)
{
	gfxc.setIndexSize(sce::Gnm::kIndexSize16);
	gfxc.setNumInstances(1);
}

int	PackModelDrawContext::drawMesh(sce::Gnmx::GnmxGfxContext	&gfxc, const std::array<const Shader<sce::Gnmx::PsShader> *, 2>	&psShaders, const void	*pCustomUserData, bool	isWireframe,
										const PackModel	&packModel, PackModelInstanceUserData	*pPackModelInstanceUserData, uint32_t	numPackModelInstances, uint32_t	meshIndex, uint32_t	instanceIndex, bool	useInstancing, const void	*pIndexBuffer, uint32_t	indexElementSizeInBytes, uint32_t	indexCount)
{
	const Mesh									&mesh						= packModel.m_meshes[meshIndex];
	const RegularBuffer< MeshInstanceUserData>	meshInstanceUserDataBuffer	= packModel.m_gpuInstanceUserDataArray[meshIndex];
	const MeshInstanceUserData					&meshInstanceUserData		= reinterpret_cast<MeshInstanceUserData *>(meshInstanceUserDataBuffer.getBaseAddress())[instanceIndex];
	const uint32_t								numMeshInstances			= useInstancing ? meshInstanceUserDataBuffer.getNumElements() : 1;
	const uint32_t								numInstances 				= numPackModelInstances * numMeshInstances;
	const ShaderMaterial						&material					= packModel.m_materialsMemory.get()[meshInstanceUserData.m_materialIndex];

	if (m_numInstances != numInstances) {
		m_numInstances = numInstances;
		gfxc.setNumInstances(m_numInstances);
	}

	if (m_indexElementSize != indexElementSizeInBytes) {
		m_indexElementSize = indexElementSizeInBytes;
		gfxc.setIndexSize((m_indexElementSize == 2) ? sce::Gnm::kIndexSize16 : sce::Gnm::kIndexSize32);
	}

	if (m_primitiveType != mesh.m_primType) {
		m_primitiveType = mesh.m_primType;
		gfxc.setPrimitiveType(m_primitiveType);
	}

	// prim setup
	sce::Gnm::PrimitiveSetup ps;
	ps.init();
	ps.setFrontFace(!m_isMirrorImage ? sce::Gnm::kPrimitiveSetupFrontFaceCcw : sce::Gnm::kPrimitiveSetupFrontFaceCw);
	ps.setCullFace((m_forceCullFace != (sce::Gnm::PrimitiveSetupCullFaceMode)-1) ? m_forceCullFace :
					material.m_isTwoSided ? sce::Gnm::kPrimitiveSetupCullFaceNone : sce::Gnm::kPrimitiveSetupCullFaceBack);
	ps.setPolygonOffsetEnable(isWireframe ? sce::Gnm::kPrimitiveSetupPolygonOffsetEnable : sce::Gnm::kPrimitiveSetupPolygonOffsetDisable,
							isWireframe ? sce::Gnm::kPrimitiveSetupPolygonOffsetEnable : sce::Gnm::kPrimitiveSetupPolygonOffsetDisable);
	if (m_prevFrontFace != ps.getFrontFace() || m_prevCullFace != ps.getCullFace() || m_prevIsWireframe != isWireframe) {
		gfxc.setPrimitiveSetup(ps);
		if (m_prevIsWireframe != isWireframe) {
			const float	scale			= isWireframe ? -100.f : 0.f;
			const float	polygonOffset	= isWireframe ? -500.f : 0.f;
			gfxc.setPolygonOffsetFront(scale, polygonOffset);
			gfxc.setPolygonOffsetBack(scale, polygonOffset);
		}
	}
	m_prevFrontFace = ps.getFrontFace(); m_prevCullFace = ps.getCullFace(); m_prevIsWireframe = isWireframe;

	const Shader<sce::Gnmx::PsShader>	*pPsShader = psShaders[material.m_enableAlphaTest ? 1 : 0];
	if (!isWireframe) {
		if (pPsShader != m_pPrevPsShader) {
#if SCE_GNMX_ENABLE_GFX_LCUE
			gfxc.setPsShader(pPsShader ? pPsShader->get() : nullptr, pPsShader ? pPsShader->getLcueOffsetsTable() : nullptr);
#else
			gfxc.setPsShader(pPsShader ? pPsShader->get() : nullptr, pPsShader ? pPsShader->getCueOffsetsTable() : nullptr);
#endif
		}
		m_pPrevPsShader = pPsShader;
	}

	// geometry shaders user data setup
	struct GeomUserData
	{
		RegularBuffer<PackModelInstanceUserData>	m_packModelInstanceBuffer;
		RegularBuffer<MeshInstanceUserData>			m_meshInstanceBuffer;
		const void									*m_pCustomUserData;
		uint										m_instanceBaseIndex;
		uint										m_numMeshInstances;
	} geomUserData;
	geomUserData.m_packModelInstanceBuffer.initAsRegularBuffer(pPackModelInstanceUserData, sizeof(PackModelInstanceUserData), numPackModelInstances);
	geomUserData.m_meshInstanceBuffer	= packModel.m_gpuInstanceUserDataArray[meshIndex];
	geomUserData.m_pCustomUserData		= pCustomUserData;
	geomUserData.m_instanceBaseIndex	= instanceIndex;
	geomUserData.m_numMeshInstances		= numMeshInstances;

	const auto activeShaderStages = CUE(gfxc).getActiveShaderStages();
	sce::Gnm::ShaderStage	frontmostStage;
	switch (activeShaderStages) {
	case sce::Gnm::kActiveShaderStagesVsPs:
		frontmostStage = sce::Gnm::kShaderStageVs;
		gfxc.setUserSrtBuffer(sce::Gnm::kShaderStageVs, &geomUserData, sizeof(geomUserData) / 4);
		break;
	case sce::Gnm::kActiveShaderStagesEsGsVsPs:
		frontmostStage = sce::Gnm::kShaderStageEs;
		gfxc.setUserSrtBuffer(sce::Gnm::kShaderStageEs, &geomUserData, sizeof(geomUserData) / 4);
		gfxc.setUserSrtBuffer(sce::Gnm::kShaderStageGs, &geomUserData, sizeof(geomUserData) / 4);
		gfxc.setUserSrtBuffer(sce::Gnm::kShaderStageVs, &geomUserData, sizeof(geomUserData) / 4);
		break;
	case sce::Gnm::kActiveShaderStagesLsHsVsPs:
	case sce::Gnm::kActiveShaderStagesOffChipLsHsVsPs:
		frontmostStage = sce::Gnm::kShaderStageLs;
		gfxc.setUserSrtBuffer(sce::Gnm::kShaderStageLs, &geomUserData, sizeof(geomUserData) / 4);
		gfxc.setUserSrtBuffer(sce::Gnm::kShaderStageHs, &geomUserData, sizeof(geomUserData) / 4);
		gfxc.setUserSrtBuffer(sce::Gnm::kShaderStageVs, &geomUserData, sizeof(geomUserData) / 4);
	case sce::Gnm::kActiveShaderStagesLsHsEsGsVsPs:
	case sce::Gnm::kActiveShaderStagesOffChipLsHsEsGsVsPs:
		frontmostStage = sce::Gnm::kShaderStageLs;
		gfxc.setUserSrtBuffer(sce::Gnm::kShaderStageLs, &geomUserData, sizeof(geomUserData) / 4);
		gfxc.setUserSrtBuffer(sce::Gnm::kShaderStageHs, &geomUserData, sizeof(geomUserData) / 4);
		gfxc.setUserSrtBuffer(sce::Gnm::kShaderStageEs, &geomUserData, sizeof(geomUserData) / 4);
		gfxc.setUserSrtBuffer(sce::Gnm::kShaderStageGs, &geomUserData, sizeof(geomUserData) / 4);
		gfxc.setUserSrtBuffer(sce::Gnm::kShaderStageVs, &geomUserData, sizeof(geomUserData) / 4);
	}
	// get valid vertex attributes specfied by valid vertex attribute flags
	std::vector<sce::Gnm::Buffer>	validVertexBuffers;
	VertexBufferSpec			validVertexBufferSpec = { &validVertexBuffers, m_validVertexAttributeFlags };
	const ConstVertexBufferSpec	meshVertexBufferSpec = { &mesh.m_vertexBuffers, mesh.m_vertexAttributeFlags };
	int ret = getValidVertexBuffers(validVertexBufferSpec, meshVertexBufferSpec);
	if (ret != SCE_OK) {
		return	ret;
	}
	// set vertex buffers
	gfxc.setVertexBuffers(frontmostStage, 0, validVertexBuffers.size(), validVertexBuffers.data());

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
		SCE_SAMPLE_UTIL_ASSERT(sizeof(PsUserData) / 4 >= pPsShader->m_srtSizeInDW);
		gfxc.setUserSrtBuffer(sce::Gnm::kShaderStagePs, &psUserData, pPsShader->m_srtSizeInDW);
		resolveShaderMaterial(packModel.m_materialsMemory.get()[meshInstanceUserData.m_materialIndex], *packModel.m_pTextureLibrary);
	}

	gfxc.drawIndex(indexCount, pIndexBuffer, 0, 0, sce::Gnm::kDrawModifierDefault);

	return	SCE_OK;
}

int	PackModelDrawContext::draw(sce::Gnmx::GnmxGfxContext	&gfxc, PackModelInstance	&packModelInstance, const std::array<const Shader<sce::Gnmx::PsShader> *, 2>	&psShaders, const void	*pCustomUserData, bool	useInstancing, bool	isWireframe)
{
	if (!packModelInstance.m_isVisible)
	{
		return SCE_OK;
	}

	SCE_SAMPLE_UTIL_ASSERT(packModelInstance.m_pPackModel != nullptr);
	if (packModelInstance.m_pPackModel == nullptr)
	{
		return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
	}

#ifdef _DEBUG
	Debug::ScopedPerfOf<sce::Gnmx::GnmxGfxContext>	perf(&gfxc, packModelInstance.m_name);
#endif

	const PackModel	&packModel = *packModelInstance.m_pPackModel;

	auto *pPackModelInstanceUserData = reinterpret_cast<PackModelInstanceUserData *>(gfxc.allocateFromCommandBuffer(sizeof(PackModelInstanceUserData), sce::Gnm::kEmbeddedDataAlignment4));
	pPackModelInstanceUserData->m_transform = packModelInstance.m_modelMatrix;
	pPackModelInstanceUserData->m_skinningMatricesArray.initAsRegularBuffer(packModelInstance.m_animInstances.size() ? packModelInstance.m_pSkinningMatrixBuffers : nullptr, sizeof(sce::Gnm::Buffer), packModelInstance.m_animInstances.size());

	if ((uint32_t)m_renderMode & (uint32_t)RenderMode::kOcclusionQuery)
	{
		SCE_SAMPLE_UTIL_ASSERT(packModelInstance.m_occlusionQueryResults.get() != nullptr);
		if (packModelInstance.m_occlusionQueryResults.get() == nullptr)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}
		// clear occlusion query results
		void *pZeroBuffer = gfxc.allocateFromCommandBuffer(sizeof(sce::Gnm::OcclusionQueryResults) * packModelInstance.m_drawOrder.size(), sce::Gnm::kEmbeddedDataAlignment4);
		memset(pZeroBuffer, 0, sizeof(Gnm::OcclusionQueryResults) * packModelInstance.m_drawOrder.size());
		gfxc.m_dcb.dmaData(sce::Gnm::kDmaDataDstMemory, (uint64_t)packModelInstance.m_occlusionQueryResults.get(), sce::Gnm::kDmaDataSrcMemory, (uint64_t)pZeroBuffer, sizeof(sce::Gnm::OcclusionQueryResults) * packModelInstance.m_drawOrder.size(), sce::Gnm::kDmaDataBlockingEnable);
	}

	for (uint32_t i = 0; i < packModel.m_instances.size(); ++i) {
		const int	instance_i = (packModelInstance.m_drawOrder.size() > 0) ? packModelInstance.m_drawOrder[i] : i;
		const MeshInstance	&meshInstance = packModel.m_instances[instance_i];

		if (!meshInstance.m_isVisible) {
			continue;
		}
#ifdef _DEBUG
		Debug::ScopedPerfOf<sce::Gnmx::GnmxGfxContext>	perf(&gfxc, meshInstance.m_name);
#endif
		SCE_SAMPLE_UTIL_ASSERT(((uint32_t)m_renderMode & ((uint32_t)RenderMode::kOcclusionQuery | (uint32_t)RenderMode::kConditional)) != ((uint32_t)RenderMode::kOcclusionQuery | (uint32_t)RenderMode::kConditional));
		if (((uint32_t)m_renderMode & ((uint32_t)RenderMode::kOcclusionQuery | (uint32_t)RenderMode::kConditional)) == ((uint32_t)RenderMode::kOcclusionQuery | (uint32_t)RenderMode::kConditional)) {
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		if ((uint32_t)m_renderMode & (uint32_t)RenderMode::kOcclusionQuery)
		{
			gfxc.writeOcclusionQuery(sce::Gnm::kOcclusionQueryOpBeginWithoutClear, &packModelInstance.m_occlusionQueryResults.get()[instance_i]);
		}

		if ((uint32_t)m_renderMode & (uint32_t)RenderMode::kConditional)
		{
			SCE_SAMPLE_UTIL_ASSERT(packModelInstance.m_occlusionQueryResults.get());
			if (packModelInstance.m_occlusionQueryResults.get() == nullptr) {
				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			gfxc.setZPassPredicationEnable(&packModelInstance.m_occlusionQueryResults.get()[instance_i], sce::Gnm::kPredicationZPassHintWait, sce::Gnm::kPredicationZPassActionDrawIfVisible);
			gfxc.setZPassPredicationDisable();
		}

		const void		*pIndexBuffer			= packModelInstance.getIndexBuffer(meshInstance.m_meshIndex);
		const uint32_t	indexElementSizeInBytes	= packModelInstance.getIndexElementSizeInBytes(meshInstance.m_meshIndex);
		uint32_t		indexCount				= packModelInstance.getIndexCount(meshInstance.m_meshIndex);
		int ret = drawMesh(gfxc, psShaders, pCustomUserData, isWireframe, packModel, pPackModelInstanceUserData, 1, meshInstance.m_meshIndex, meshInstance.m_instanceIndex, false, pIndexBuffer, indexElementSizeInBytes, indexCount);
		if (ret != SCE_OK) {
			return ret;
		}

		if ((uint32_t)m_renderMode & (uint32_t)RenderMode::kConditional)
		{
			gfxc.setZPassPredicationDisable();
		}
		if ((uint32_t)m_renderMode & (uint32_t)RenderMode::kOcclusionQuery)
		{
			gfxc.writeOcclusionQuery(sce::Gnm::kOcclusionQueryOpEnd, &packModelInstance.m_occlusionQueryResults.get()[instance_i]);
		}
	}

	return	SCE_OK;
}

void	initializePackModelDraw(sce::SampleUtil::Graphics::VideoAllocator	*allocator)
{
	initializeDefaultPackModelShaders(*allocator);
}

void	finalizePackModelDraw()
{
	finalizeDefaultPackModelShaders();
}

int	drawPackModelInternal(sce::Gnmx::GnmxGfxContext	*gfxc, PackModel	&packModel,	SceLibcMspace	edgeAnimMemoryAlloc, VideoRingAllocator	&videoRingMemoryAlloc, float	animationTimeInSec,
						Matrix4_arg	modelMatrix, Matrix4_arg	viewMatrix, Matrix4_arg	projectionMatrix, Vector3_arg	lightPosition, float	ambient, float	shininess,
						const Shader<sce::Gnmx::VsShader>	*vs, const Shader<sce::Gnmx::PsShader>	*fillPs, const void	*fillUserData,
						const std::vector<JointTransformDesignation>	&auxTransforms,	bool	drawSkeleton)
{
	SCE_SAMPLE_UTIL_ASSERT(gfxc != nullptr);
	if (gfxc == nullptr)
	{
		return	SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}

	PackModelInstance	*packModelInstance = new PackModelInstance(packModel, edgeAnimMemoryAlloc, nullptr, Transform3(modelMatrix));
	SCE_SAMPLE_UTIL_ASSERT(packModelInstance != nullptr);
	if (packModelInstance == nullptr)
	{
		return	SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
	}

	if (packModel.m_animations.size() > 0 && edgeAnimMemoryAlloc != nullptr) {
		// Update animation
		packModelInstance->m_pSkinningMatrixBuffers = reinterpret_cast<sce::Gnm::Buffer *>(videoRingMemoryAlloc.allocate(sizeof(sce::Gnm::Buffer) * packModel.m_animations.size(), sce::Gnm::kAlignmentOfBufferInBytes));
		for (int i = 0; i < packModelInstance->m_animInstances.size(); i++) {
			packModelInstance->m_animInstances[i].evaluateSkinningMatrices(animationTimeInSec, videoRingMemoryAlloc, auxTransforms);
			packModelInstance->m_pSkinningMatrixBuffers[i] = packModelInstance->m_animInstances[i].m_skinningMatrixBuffer;
		}
	}

	struct SrtData
	{
		Matrix4Unaligned	m_viewProjectionMatrix;
		Vector3Unaligned	m_cameraPosition;
		Vector3Unaligned	m_lightPosition;
		float				m_ambient;
		float				m_shininess;
		int					m_hdr;
		const void			*m_pUserData;
	};
	SrtData *srtData = (SrtData*)videoRingMemoryAlloc.allocate(sizeof(SrtData), 16);
	srtData->m_viewProjectionMatrix	= projectionMatrix * viewMatrix;
	srtData->m_cameraPosition		= -viewMatrix.getTranslation();
	srtData->m_lightPosition		= lightPosition;
	srtData->m_ambient				= ambient;
	srtData->m_shininess			= shininess;
	srtData->m_hdr					= (int)getHdr();
	srtData->m_pUserData			= fillUserData;

	if (vs == nullptr) {
		// search for vertex shader matching with pack vertex attributes
		vs = PackModel::getDefaultVertexShader(packModel);
		SCE_SAMPLE_UTIL_ASSERT(vs != nullptr);
		if (vs == nullptr) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
	}

	if (fillPs == nullptr) {
		// search for pixel shader matching with pack material
		fillPs = PackModel::getDefaultPixelShader(packModel);
		SCE_SAMPLE_UTIL_ASSERT(fillPs != nullptr);
		if (fillPs == nullptr) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
	}

	PackModelDrawContext	drawContext;
	drawContext.beginDraw(*gfxc);
#if SCE_GNMX_ENABLE_GFX_LCUE
	gfxc->setVsShader(vs->get(), 0, vs->getFetchShader(), vs->getLcueOffsetsTable());
#else
	gfxc->setVsShader(vs->get(), 0, vs->getFetchShader(), vs->getCueOffsetsTable());
#endif
	int ret = drawContext.draw(*gfxc, *packModelInstance, { fillPs, fillPs }, srtData);
	drawContext.endDraw(*gfxc);

	if (drawSkeleton && packModel.m_animations.size() > 0 && edgeAnimMemoryAlloc != nullptr) {
		sce::Gnm::DepthStencilControl	dsc;
		dsc.init();
		dsc.setDepthEnable(false);
		gfxc->setDepthStencilControl(dsc);
		for (int i = 0; i < packModelInstance->m_animInstances.size(); i++) {
			packModelInstance->m_animInstances[i].drawJoints(gfxc->m_dcb, Transform3(modelMatrix), Transform3(viewMatrix), projectionMatrix);
		}
		dsc.setDepthEnable(true);
		dsc.setDepthControl(sce::Gnm::kDepthControlZWriteEnable, sce::Gnm::kCompareFuncLessEqual);
		gfxc->setDepthStencilControl(dsc);
	}

	delete packModelInstance;

	return	ret;
}

} } } // namespace sce::SampleUtil::Graphics
#endif // _SCE_TARGET_OS_ORBIS
#endif