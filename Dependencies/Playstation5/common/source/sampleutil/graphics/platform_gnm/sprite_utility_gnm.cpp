/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. 
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS

#include <memory>

#include <sampleutil/graphics/platform_gnm/shader_gnm.h>
#include <sampleutil/graphics/sprite_utility.h>
#include <sampleutil/sampleutil_error.h>

#include "sampleutil/graphics.h"

DEFINE_SHADER(sce::Gnmx::VsShader, sprite_utility_2d_vv);
DEFINE_SHADER(sce::Gnmx::PsShader, sprite_utility_fill_p);

DEFINE_SHADER(sce::Gnmx::PsShader, sprite_utility_texture_p);
DEFINE_SHADER(sce::Gnmx::PsShader, sprite_utility_texture_ycbcr_p);
DEFINE_SHADER(sce::Gnmx::PsShader, sprite_utility_texture_yuy2_p);

DEFINE_SHADER(sce::Gnmx::VsShader, sprite_utility_3d_vv);
DEFINE_SHADER(sce::Gnmx::PsShader, sprite_utility_3d_fill_p);

using namespace sce::Vectormath::Simd::Aos;

namespace /* anonymous */
{
	constexpr int kNumberOfOvalMeshVertices		= 37;
	constexpr int kNumberOfOvalFillIndices		= (kNumberOfOvalMeshVertices - 1) * 3;
	constexpr int kNumberOfOvalDrawIndices		= (kNumberOfOvalMeshVertices - 1) * 2;

	bool	s_isAlphaBlendEnabled = true;

	sce::SampleUtil::Memory::Gpu::unique_ptr<sce::SampleUtil::Graphics::Vertex[]>	s_ovalMeshVertices;
	sce::SampleUtil::Memory::Gpu::unique_ptr<uint16_t[]>			s_ovalFillIndices;
	sce::SampleUtil::Memory::Gpu::unique_ptr<uint16_t[]>			s_ovalDrawIndices;
	sce::Gnm::Buffer												s_ovalMeshVertexBuffer;

	std::unique_ptr<sce::SampleUtil::Graphics::BoxMesh>				s_cubeMesh;
	std::unique_ptr<sce::SampleUtil::Graphics::SphereMesh>			s_sphereMesh;
	std::unique_ptr<sce::SampleUtil::Graphics::CylinderMesh>		s_cylinderMesh;
	std::unique_ptr<sce::SampleUtil::Graphics::ConeMesh>			s_coneMesh;

	void	setupDrawState(sce::Gnm::DrawCommandBuffer	&dcb, const sce::Gnm::PrimitiveType	primType, float eighthWidth = 0.f, float ptSize = 0.f)
	{
		const bool isLine = (eighthWidth > 0.f);

		dcb.setIndexSize(sce::Gnm::kIndexSize16, sce::Gnm::kCachePolicyLru);
		// Setup RenderStates
		sce::Gnm::PrimitiveSetup ps;
		ps.init();
		if (isLine)
		{
			ps.setPolygonMode(sce::Gnm::kPrimitiveSetupPolygonModeLine, sce::Gnm::kPrimitiveSetupPolygonModeLine);
		}
		dcb.setPrimitiveSetup(ps);
		sce::Gnm::BlendControl bc;
		bc.init();
		bc.setBlendEnable(s_isAlphaBlendEnabled);
		bc.setAlphaEquation(sce::Gnm::kBlendMultiplierSrcAlpha, sce::Gnm::kBlendFuncAdd, sce::Gnm::kBlendMultiplierOneMinusSrcAlpha);
		bc.setColorEquation(sce::Gnm::kBlendMultiplierSrcAlpha, sce::Gnm::kBlendFuncAdd, sce::Gnm::kBlendMultiplierOneMinusSrcAlpha);
		dcb.setBlendControl(0, bc);
		dcb.setPrimitiveType(primType);
		dcb.setLineWidth((uint32_t)eighthWidth);
		dcb.setPointSize((uint32_t)(ptSize * 8.f), (uint32_t)(ptSize * 8.f));
	}

	void	draw2dInternal(
		sce::Gnm::DrawCommandBuffer	&dcb,
		Vector4_arg					rgba,
		const sce::Gnm::Buffer		&vertexBuffer,
		const void					*pIndices,
		uint32_t					numIndices,
		sce::Gnm::PrimitiveType		primType,
		const sce::Gnmx::PsShader	*pPsShader,
		uint32_t					psSrtSizeInDw,
		const void					*pUserData,
		const std::array<float,2>	&scale,
		const std::array<float,2>	&offset,
		float						depth,
		float						eighthWidth = 0.f,
		float						ptSize = 0.f
	)
	{
		dcb.setVsShader(&SHADER(sprite_utility_2d_vv)->m_vsStageRegisters, 0);
		if (pPsShader != nullptr) {
			dcb.setPsShader(&pPsShader->m_psStageRegisters);
			if (pPsShader->m_numInputSemantics > 0) {
				uint32_t psInputTable[32];
				sce::Gnm::generatePsShaderUsageTable(psInputTable, SHADER(sprite_utility_2d_vv)->getExportSemanticTable(), SHADER(sprite_utility_2d_vv)->m_numExportSemantics, pPsShader->getPixelInputSemanticTable(), pPsShader->m_numInputSemantics);

				dcb.setPsShaderUsage(psInputTable, pPsShader->m_numInputSemantics);
			}
		} else {
			dcb.setPsShader(nullptr);
		}
		setupDrawState(dcb, primType, eighthWidth, ptSize);

		//  BindShaderResourceTable
		struct VsSrtData
		{
			sce::Gnm::Buffer	m_vertexBuffer;
			float				m_scale[2];
			float				m_offset[2];
			float				m_depth;
			uint32_t			__padding__[2];
			const void			*m_pUserData;
		} vsSrtData =
		{
			vertexBuffer, {scale[0], scale[1]}, {offset[0], offset[1]}, depth, {}, pUserData
		};
		dcb.setUserDataRegion(sce::Gnm::ShaderStage::kShaderStageVs, 0, reinterpret_cast<uint32_t*>(&vsSrtData), SIZE_OF_SRT(sprite_utility_2d_vv));

		if (pPsShader != nullptr) {
			struct PsRenderParameters
			{
				vec_float4	m_rgba;
				int			m_hdr;
				uint32_t	__padding__[1];
				const void	*m_pUserData;
			} psRenderParameters = { rgba.get128(), (int)sce::SampleUtil::Graphics::getHdr(), {}, pUserData };
			dcb.setUserDataRegion(sce::Gnm::ShaderStage::kShaderStagePs, 0, reinterpret_cast<uint32_t*>(&psRenderParameters), psSrtSizeInDw);
		}

		// Draw Call
		if (pIndices != nullptr)
		{
			dcb.drawIndex(numIndices, pIndices);
		} else {
			dcb.drawIndexAuto(numIndices);
		}
	}

	void	drawTextureInternal(
		sce::Gnm::DrawCommandBuffer			&dcb,
		const sce::Gnmx::PsShader			*pPsShader,
		uint32_t							psSrtSizeInDw,
		const void							*pUserData,
		Vector2_arg							position,
		Vector2_arg							size,
		Vector2_arg							textureOffset,
		Vector2_arg							sizeInTexture,
		Vector4_arg							colorCoeff,
		float								depth
	)
	{
		void *verticesBuffer = dcb.allocateFromCommandBuffer(sizeof(sce::SampleUtil::Graphics::Vertex) * 3, sce::Gnm::kEmbeddedDataAlignment16);
		auto *vertices = new (verticesBuffer) std::array <sce::SampleUtil::Graphics::Vertex, 3>();
		(*vertices)[0].m_position	= ToVector3Unaligned(Vector3(position                            , 0.f));
		(*vertices)[1].m_position	= ToVector3Unaligned(Vector3(position + Vector2(size.getX(), 0.f), 0.f));
		(*vertices)[2].m_position	= ToVector3Unaligned(Vector3(position + Vector2(0.f, size.getY()), 0.f));
		(*vertices)[0].m_uv			= ToVector2Unaligned(textureOffset);
		(*vertices)[1].m_uv			= ToVector2Unaligned(textureOffset + Vector2(sizeInTexture.getX(), 0.f));
		(*vertices)[2].m_uv			= ToVector2Unaligned(textureOffset + Vector2(0.f, sizeInTexture.getY()));
		sce::Gnm::Buffer	vertexBuffer;
		vertexBuffer.initAsRegularBuffer(vertices->data(), sizeof(sce::SampleUtil::Graphics::Vertex), vertices->size());

		const std::array<float, 2> scale = { 2.f, -2.f };
		const std::array<float, 2> offset = { -1.f, 1.f };
		draw2dInternal(dcb, colorCoeff, vertexBuffer, nullptr, vertices->size(), sce::Gnm::kPrimitiveTypeRectList, pPsShader, psSrtSizeInDw, pUserData, scale, offset, depth);
	}

	struct DefaultPsRender3DParameters
	{
		Vector4Unaligned	m_rgba;
		int					m_hdr;
		int					__padding__;
		const void			*m_pOption;
		Vector3Unaligned	m_lightPosition;
		float				m_ambient;
		Vector3Unaligned	m_viewPosition;
		float				m_shininess;
	};

	void setupDefaultPsRender3DParameters(
		sce::Gnm::DrawCommandBuffer &allocator,
		DefaultPsRender3DParameters *&outDefaultParameters,
		const void *option,
		Vector4_arg colorRGBA,
		Vector3_arg lightPosition = Vector3( 0.0f, 0.0f, 0.0f ),
		float ambient = 0.0f,
		Vector3_arg viewPosition = Vector3( 0.0f, 0.0f, 0.0f ),
		float shininess = 0.0f )
	{
		outDefaultParameters = reinterpret_cast<DefaultPsRender3DParameters *>(allocator.allocateFromCommandBuffer(sizeof(DefaultPsRender3DParameters), sce::Gnm::EmbeddedDataAlignment::kEmbeddedDataAlignment16));
		memset(outDefaultParameters, 0xCE, sizeof(DefaultPsRender3DParameters));
		outDefaultParameters->m_rgba				= ToVector4Unaligned(colorRGBA);
		outDefaultParameters->m_hdr					= (int)sce::SampleUtil::Graphics::getHdr();
		outDefaultParameters->__padding__			= 0;
		outDefaultParameters->m_pOption				= option;
		outDefaultParameters->m_lightPosition		= ToVector3Unaligned(lightPosition);
		outDefaultParameters->m_ambient				= ambient;
		outDefaultParameters->m_viewPosition		= ToVector3Unaligned(viewPosition);
		outDefaultParameters->m_shininess			= shininess;
	}

	uint32_t getSizeOfDefaultPsRender3DParametersInDwords( )
	{
		return sizeof(DefaultPsRender3DParameters) / sizeof(uint32_t);
	}
	
	int	draw3DInternal(
		sce::Gnm::DrawCommandBuffer			*dcb,
		const sce::Gnm::Buffer				&vertexBuffer,
		const void							*pIndices,
		uint32_t							numIndices,
		const Matrix4						*modelMatrices,
		uint32_t							numInstances,
		Matrix4_arg							viewMat,
		Matrix4_arg							projMat,
		sce::Gnm::PrimitiveType				primType,
		const sce::Gnmx::PsShader			*pPsShader,
		const void							*pPsShaderUserData,
		uint32_t							psShaderUserDataSizeInDword,
		float								eighthWidth = 0.f,
		float								ptSize = 0.f,
		bool								internalAllocateOfmodelMatricesBuffer = true
	)
	{
		if (dcb == nullptr || (pPsShader != nullptr && pPsShaderUserData == nullptr))
		{
			return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
		}

		dcb->setVsShader(&SHADER(sprite_utility_3d_vv)->m_vsStageRegisters, 0);
		if (pPsShader != nullptr) {
			dcb->setPsShader(&pPsShader->m_psStageRegisters);
			if (pPsShader->m_numInputSemantics > 0) {
				uint32_t psInputTable[32];
				sce::Gnm::generatePsShaderUsageTable(psInputTable, SHADER(sprite_utility_3d_vv)->getExportSemanticTable(), SHADER(sprite_utility_3d_vv)->m_numExportSemantics, pPsShader->getPixelInputSemanticTable(), pPsShader->m_numInputSemantics);

				dcb->setPsShaderUsage(psInputTable, pPsShader->m_numInputSemantics);
			}
		} else {
			dcb->setPsShader(nullptr);
		}
		setupDrawState(*dcb, primType, eighthWidth, ptSize);

		//  BindShaderResourceTable
		struct VsRenderParameters
		{
			sce::Gnm::Buffer	m_vertexBuffer;
			sce::Gnm::Buffer	m_modelMatrixBuffer;
			Matrix4Unaligned	m_viewMatrix;
			Matrix4Unaligned	m_projectionMatrix;
		};
	
		struct VsUserSrtData
		{
			VsRenderParameters *m_vsRenderParameters;
		} vsUserSrtData;


		vsUserSrtData.m_vsRenderParameters = reinterpret_cast<VsRenderParameters *>(dcb->allocateFromCommandBuffer(sizeof(VsRenderParameters), sce::Gnm::kEmbeddedDataAlignment16));

		vsUserSrtData.m_vsRenderParameters->m_vertexBuffer					= vertexBuffer;

		if (internalAllocateOfmodelMatricesBuffer == true)
		{
			Matrix4Unaligned *tempModelMatrices = reinterpret_cast<Matrix4Unaligned *>(dcb->allocateFromCommandBuffer(sizeof(Matrix4Unaligned) * numInstances, sce::Gnm::kEmbeddedDataAlignment16));
			SCE_SAMPLE_UTIL_ASSERT(nullptr != tempModelMatrices);
			memcpy(tempModelMatrices, modelMatrices, sizeof(Matrix4Unaligned) * numInstances);
			vsUserSrtData.m_vsRenderParameters->m_modelMatrixBuffer.initAsRegularBuffer(tempModelMatrices, sizeof(tempModelMatrices[0]), numInstances);
		} else {
			vsUserSrtData.m_vsRenderParameters->m_modelMatrixBuffer.initAsRegularBuffer((void*)modelMatrices, sizeof(modelMatrices[0]), numInstances);
		}

		vsUserSrtData.m_vsRenderParameters->m_viewMatrix					= ToMatrix4Unaligned(viewMat);
		vsUserSrtData.m_vsRenderParameters->m_projectionMatrix				= ToMatrix4Unaligned(projMat);

		dcb->setUserDataRegion(sce::Gnm::ShaderStage::kShaderStageVs, 0, (uint32_t*)&vsUserSrtData, SIZE_OF_SRT(sprite_utility_3d_vv));
		if (pPsShader != nullptr) {
			dcb->setUserDataRegion(sce::Gnm::ShaderStage::kShaderStagePs, 0, (uint32_t *)pPsShaderUserData, psShaderUserDataSizeInDword);
		}

		// Draw Call
		if (numInstances > 1)
		{
			dcb->setNumInstances(numInstances);
		}
		else
		{
			dcb->setNumInstances(1);
		}

		if (pIndices != nullptr)
		{
			dcb->drawIndex(numIndices, pIndices);
		} else {
			dcb->drawIndexAuto(numIndices);
		}

		if (numInstances > 1)
		{
			dcb->setNumInstances(1);
		}
		return SCE_OK;
	}
} /* namespace anonymous */

namespace sce {	namespace SampleUtil { namespace Graphics {
namespace SpriteUtil {

void	setAlphaBlendEnable(bool	enable)
{
	s_isAlphaBlendEnabled = enable;
}

bool	getAlphaBlendEnable()
{
	return s_isAlphaBlendEnabled;
}

int	initialize(SampleUtil::Graphics::VideoAllocator	&videoMemory)
{
	// Initialize Shaders.
	CREATE_SHADER(sce::Gnmx::PsShader, sprite_utility_fill_p, &videoMemory);

	CREATE_SHADER(sce::Gnmx::VsShader, sprite_utility_2d_vv, &videoMemory);
	CREATE_SHADER(sce::Gnmx::PsShader, sprite_utility_texture_p, &videoMemory);
	CREATE_SHADER(sce::Gnmx::PsShader, sprite_utility_texture_ycbcr_p, &videoMemory);
	CREATE_SHADER(sce::Gnmx::PsShader, sprite_utility_texture_yuy2_p, &videoMemory);

	CREATE_SHADER(sce::Gnmx::VsShader, sprite_utility_3d_vv, &videoMemory);
	CREATE_SHADER(sce::Gnmx::PsShader, sprite_utility_3d_fill_p, &videoMemory);

	// Setup Oval Mesh.
	s_ovalMeshVertices = Memory::Gpu::make_unique<Vertex>(kNumberOfOvalMeshVertices, sce::Gnm::kAlignmentOfBufferInBytes, { sce::Gnm::kResourceTypeVertexBufferBaseAddress } , videoMemory, "sce::SampleUtil::SpriteUtil::OvalMeshVertices" );
	s_ovalFillIndices = Memory::Gpu::make_unique<uint16_t>(kNumberOfOvalFillIndices, sce::Gnm::kAlignmentOfBufferInBytes, { sce::Gnm::kResourceTypeIndexBufferBaseAddress } , videoMemory, "sce::SampleUtil::SpriteUtil::OvalFillIndices" );
	s_ovalDrawIndices = Memory::Gpu::make_unique<uint16_t>(kNumberOfOvalDrawIndices, sce::Gnm::kAlignmentOfBufferInBytes, { sce::Gnm::kResourceTypeIndexBufferBaseAddress } , videoMemory, "sce::SampleUtil::SpriteUtil::OvalDrawIndices" );
	
	for (uint32_t i = 0 ; i < kNumberOfOvalMeshVertices - 1; i++)
	{
		const float rad = static_cast<float>(i * 2.f * M_PI / static_cast<float>(kNumberOfOvalMeshVertices - 1));

		s_ovalMeshVertices[i].m_position = ToVector3Unaligned(Vector3(0.5f, 0.5f, 0.f) + (Vector3(0.5f * cos(rad), 0.5f * sin(rad), 0.f)));
		s_ovalMeshVertices[i].m_uv = Vector2Unaligned{ s_ovalMeshVertices[i].m_position.x, 1.f - s_ovalMeshVertices[i].m_position.y };
		s_ovalFillIndices[3 * i + 0] = (i + 0) % (kNumberOfOvalMeshVertices - 1);
		s_ovalFillIndices[3 * i + 1] = (i + 1) % (kNumberOfOvalMeshVertices - 1);
		s_ovalFillIndices[3 * i + 2] = kNumberOfOvalMeshVertices - 1;

		s_ovalDrawIndices[2 * i + 0] = (i + 0) % (kNumberOfOvalMeshVertices - 1);
		s_ovalDrawIndices[2 * i + 1] = (i + 1) % (kNumberOfOvalMeshVertices - 1);
	}
	s_ovalMeshVertices[kNumberOfOvalMeshVertices - 1].m_position = Vector3Unaligned{ 0.5f, 0.5f, 0.f };
	s_ovalMeshVertices[kNumberOfOvalMeshVertices - 1].m_uv = Vector2Unaligned{ 0.5f, 0.5f, };

	s_ovalMeshVertexBuffer.initAsRegularBuffer(s_ovalMeshVertices.get(), sizeof(Vertex), kNumberOfOvalMeshVertices);

	// Setup 3D Mesh.
	s_cubeMesh = std::make_unique<SampleUtil::Graphics::BoxMesh>(videoMemory, 1.f, 1.f, 1.f);
	SCE_SAMPLE_UTIL_ASSERT(s_cubeMesh.get( ) != nullptr);

	s_sphereMesh = std::make_unique<SampleUtil::Graphics::SphereMesh>(videoMemory, 0.5f, 32.f, 32.f);
	SCE_SAMPLE_UTIL_ASSERT(s_sphereMesh.get() != nullptr);

	s_cylinderMesh = std::make_unique<SampleUtil::Graphics::CylinderMesh>(videoMemory, 0.5f, 0.5f, 64);
	SCE_SAMPLE_UTIL_ASSERT(s_cylinderMesh.get() != nullptr);

	s_coneMesh = std::make_unique<SampleUtil::Graphics::ConeMesh>(videoMemory, 0.5f, 1.0f, 32.0f);
	SCE_SAMPLE_UTIL_ASSERT(s_coneMesh.get() != nullptr);

	return SCE_OK;
}

int finalize()
{
	DESTROY_SHADER(sprite_utility_fill_p);
	DESTROY_SHADER(sprite_utility_2d_vv);
	DESTROY_SHADER(sprite_utility_texture_p);
	DESTROY_SHADER(sprite_utility_texture_ycbcr_p);
	DESTROY_SHADER(sprite_utility_texture_yuy2_p);
	DESTROY_SHADER(sprite_utility_3d_vv);
	DESTROY_SHADER(sprite_utility_3d_fill_p);

	s_ovalMeshVertices.reset(nullptr);
	s_ovalFillIndices.reset(nullptr);
	s_ovalDrawIndices.reset(nullptr);

	s_cubeMesh.reset(nullptr);
	s_sphereMesh.reset(nullptr);
	s_cylinderMesh.reset(nullptr);
	s_coneMesh.reset(nullptr);

	return SCE_OK;
}

int	fillRect(
	sce::Gnm::DrawCommandBuffer *dcb,
	Vector2_arg	position, Vector2_arg	size, Vector4_arg	rgba, float	depth,
	const sce::Gnmx::PsShader	*fillPs, uint32_t	psSrtSizeInDw, const void	*fillUserData)
{
	SCE_SAMPLE_UTIL_ASSERT(dcb != nullptr);
	if (dcb == nullptr)
	{
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	void *verticesBuffer = dcb->allocateFromCommandBuffer(sizeof(Vertex) * 3, sce::Gnm::kEmbeddedDataAlignment16);
	auto *vertices = new (verticesBuffer) std::array<Vertex,3>();

	(*vertices)[0].m_position = ToVector3Unaligned(Vector3(position                            , 0.f));
	(*vertices)[1].m_position = ToVector3Unaligned(Vector3(position + Vector2(size.getX(), 0.f), 0.f));
	(*vertices)[2].m_position = ToVector3Unaligned(Vector3(position + Vector2(0.f, size.getY()), 0.f));
	sce::Gnm::Buffer vertexBuffer;
	vertexBuffer.initAsRegularBuffer(vertices->data(), sizeof(Vertex), vertices->size());

	const std::array<float, 2> scale = { 2.f, -2.f };
	const std::array<float, 2> offset = { -1.f, 1.f };
	draw2dInternal(*dcb, rgba, vertexBuffer, nullptr, vertices->size(), sce::Gnm::kPrimitiveTypeRectList, fillPs, psSrtSizeInDw, fillUserData, scale, offset, depth);

	return SCE_OK;
}

int	drawRect(
	sce::Gnm::DrawCommandBuffer	*dcb,
	Vector2_arg	position, Vector2_arg	size, Vector4_arg	rgba, float	depth, uint32_t	eighthWidth)
{
	SCE_SAMPLE_UTIL_ASSERT(dcb != nullptr);
	if (dcb == nullptr)
	{
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	void *verticesBuffer = dcb->allocateFromCommandBuffer(sizeof(Vertex) * 8, sce::Gnm::kEmbeddedDataAlignment16);
	auto *vertices = new (verticesBuffer) std::array<Vertex, 8>();

	(*vertices)[0].m_position = ToVector3Unaligned(Vector3(position                                    , 0.f));
	(*vertices)[1].m_position = ToVector3Unaligned(Vector3(position + Vector2(size.getX(), 0.f        ), 0.f));
	(*vertices)[2].m_position = ToVector3Unaligned(Vector3(position + Vector2(size.getX(), 0.f        ), 0.f));
	(*vertices)[3].m_position = ToVector3Unaligned(Vector3(position + Vector2(size.getX(), size.getY()), 0.f));
	(*vertices)[4].m_position = ToVector3Unaligned(Vector3(position + Vector2(size.getX(), size.getY()), 0.f));
	(*vertices)[5].m_position = ToVector3Unaligned(Vector3(position + Vector2(0.f        , size.getY()), 0.f));
	(*vertices)[6].m_position = ToVector3Unaligned(Vector3(position + Vector2(0.f        , size.getY()), 0.f));
	(*vertices)[7].m_position = ToVector3Unaligned(Vector3(position                                    , 0.f));
	sce::Gnm::Buffer vertexBuffer;
	vertexBuffer.initAsRegularBuffer(vertices->data(), sizeof(Vertex), vertices->size());

	const std::array<float, 2> scale = { 2.f, -2.f };
	const std::array<float, 2> offset = { -1.f, 1.f };
	draw2dInternal(*dcb, rgba, vertexBuffer, nullptr, vertices->size(), sce::Gnm::kPrimitiveTypeLineList, SHADER(sprite_utility_fill_p), SIZE_OF_SRT(sprite_utility_fill_p), nullptr, scale, offset, depth, eighthWidth);

	return SCE_OK;
}

int	fillOval(
	sce::Gnm::DrawCommandBuffer	*dcb,
	Vector2_arg	position, Vector2_arg	size, Vector4_arg	rgba, float	depth,
	const sce::Gnmx::PsShader	*fillPs, uint32_t	psSrtSizeInDw, const void	*fillUserData)
{
	const std::array<float, 2> scale = { 2.f * size.getX(), -2.f * size.getY() };
	const std::array<float, 2> offset = { position.getX() * 2.f - 1.f, -position.getY() * 2.f + 1.f };
	draw2dInternal(*dcb, rgba, s_ovalMeshVertexBuffer, s_ovalFillIndices.get(), kNumberOfOvalFillIndices, sce::Gnm::kPrimitiveTypeTriList, fillPs, psSrtSizeInDw, fillUserData, scale, offset, depth);

	return SCE_OK;
}

int	drawOval(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Vector2_arg	position, Vector2_arg	size, Vector4_arg	rgba, float		depth, uint32_t	eighthWidth)
{
	SCE_SAMPLE_UTIL_ASSERT(dcb != nullptr);
	if (dcb == nullptr)
	{
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	const std::array<float, 2> scale = { 2.f * size.getX(), -2.f * size.getY() };
	const std::array<float, 2> offset = { position.getX() * 2.f - 1.f, -position.getY() * 2.f + 1.f };
	draw2dInternal(*dcb, rgba, s_ovalMeshVertexBuffer, s_ovalDrawIndices.get(), kNumberOfOvalDrawIndices, sce::Gnm::kPrimitiveTypeLineList, SHADER(sprite_utility_fill_p), SIZE_OF_SRT(sprite_utility_fill_p), nullptr, scale, offset, depth, eighthWidth);

	return SCE_OK;
}

int	drawLine(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Vector2_arg	begin, Vector2_arg	end, Vector4_arg	rgba, float		depth, uint32_t	eighthWidth)
{
	SCE_SAMPLE_UTIL_ASSERT(dcb != nullptr);
	if (dcb == nullptr)
	{
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	void *buffer = dcb->allocateFromCommandBuffer(sizeof(Vertex) * 2, sce::Gnm::kEmbeddedDataAlignment16);
	auto *vertices = new (buffer) std::array<Vertex, 2>();

	(*vertices)[0].m_position = ToVector3Unaligned(Vector3(begin, 0.f));
	(*vertices)[1].m_position = ToVector3Unaligned(Vector3(end, 0.f));
	sce::Gnm::Buffer vertexBuffer;
	vertexBuffer.initAsRegularBuffer(vertices->data(), sizeof(Vertex), vertices->size());

	const std::array<float, 2> scale = { 2.f, -2.f };
	const std::array<float, 2> offset = { -1.f, 1.f };
	draw2dInternal(*dcb, rgba, vertexBuffer, nullptr, vertices->size(), sce::Gnm::kPrimitiveTypeLineList, SHADER(sprite_utility_fill_p), SIZE_OF_SRT(sprite_utility_fill_p), nullptr, scale, offset, depth, eighthWidth);

	return SCE_OK;
}

int	drawTexture(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Vector2_arg	position, Vector2_arg	size, Vector2_arg	textureOffset, Vector2_arg	sizeInTexture,
	const sce::Gnm::Texture	&texture, const sce::Gnm::Sampler	&sampler,
	Vector4_arg	colorCoeff,	float	depth
	)
{
	SCE_SAMPLE_UTIL_ASSERT(dcb != nullptr);
	if (dcb == nullptr)
	{
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	struct PsRenderParameters
	{
		sce::Gnm::Sampler	m_sampler;
		sce::Gnm::Texture	m_texture;
	} *userData = reinterpret_cast <PsRenderParameters *>(dcb->allocateFromCommandBuffer(sizeof(PsRenderParameters) , sce::Gnm::kEmbeddedDataAlignment16));
	userData->m_sampler = sampler;
	userData->m_texture = texture;
	drawTextureInternal(*dcb, SHADER(sprite_utility_texture_p), SIZE_OF_SRT(sprite_utility_texture_p), userData, position, size, textureOffset, sizeInTexture, colorCoeff, depth);

	return SCE_OK;
}

int	drawTextureYuy2(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Vector2_arg	position, Vector2_arg	size, Vector2_arg	textureOffset, Vector2_arg	sizeInTexture,
	const sce::Gnm::Texture	&texture, const sce::Gnm::Sampler	&sampler,
	Vector4_arg	colorCoeff, float	depth
)
{
	SCE_SAMPLE_UTIL_ASSERT(dcb != nullptr);
	if (dcb == nullptr)
	{
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	struct PsRenderParameters
	{
		sce::Gnm::Sampler	m_sampler;
		sce::Gnm::Texture	m_texture;
	} *userData = reinterpret_cast<PsRenderParameters *>(dcb->allocateFromCommandBuffer(sizeof(PsRenderParameters), sce::Gnm::kEmbeddedDataAlignment16));
	userData->m_sampler = sampler;
	userData->m_texture = texture;
	drawTextureInternal(*dcb, SHADER(sprite_utility_texture_yuy2_p), SIZE_OF_SRT(sprite_utility_texture_yuy2_p), userData, position, size, textureOffset, sizeInTexture, colorCoeff, depth);

	return SCE_OK;
}

int	drawTextureYcbcr(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Vector2_arg	position, Vector2_arg	size, Vector2_arg	textureOffset, Vector2_arg	sizeInTexture,
	const sce::Gnm::Texture	&yTexture, const sce::Gnm::Sampler	&ySampler, const sce::Gnm::Texture	&cbcrTexture, const sce::Gnm::Sampler	&cbcrSampler,
	Vector4_arg	colorCoeff, float	depth, YuvMode	yuvMode, uint8_t	bitDepth
)
{
	SCE_SAMPLE_UTIL_ASSERT(dcb != nullptr);
	if (dcb == nullptr)
	{
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	struct PsRenderParameters
	{
		sce::Gnm::Sampler	m_ySampler;
		sce::Gnm::Texture	m_yTexture;
		sce::Gnm::Sampler	m_cbcrSampler;
		sce::Gnm::Texture	m_cbcrTexture;
		uint32_t		m_bitDepth;
		uint32_t		m_yuvMode;
	} *userData = reinterpret_cast<PsRenderParameters *>(dcb->allocateFromCommandBuffer(sizeof(PsRenderParameters), sce::Gnm::kEmbeddedDataAlignment16));
	userData->m_ySampler	= ySampler;
	userData->m_yTexture	= yTexture;
	userData->m_cbcrSampler	= cbcrSampler;
	userData->m_cbcrTexture	= cbcrTexture;
	userData->m_bitDepth	= bitDepth;
	userData->m_yuvMode		= (uint32_t)yuvMode;
	drawTextureInternal(*dcb, SHADER(sprite_utility_texture_ycbcr_p), SIZE_OF_SRT(sprite_utility_texture_ycbcr_p), userData, position, size, textureOffset, sizeInTexture, colorCoeff, depth);
	return SCE_OK;
}

int	drawPoints(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	float psize,
	const sce::Gnm::Texture	&texture, const sce::Gnm::Sampler	&sampler, const sce::Gnm::Buffer	&vertices,
	Memory::Gpu::unique_ptr<Index[]>	&indices, uint32_t	numIndices,
	Vector4_arg	colorCoeff, float	depth
	)
{
	SCE_SAMPLE_UTIL_ASSERT(dcb != nullptr);
	if (dcb == nullptr)
	{
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	struct PsRenderParameters
	{
		sce::Gnm::Sampler	m_sampler;
		sce::Gnm::Texture	m_texture;
	} *userData = reinterpret_cast <PsRenderParameters *>(dcb->allocateFromCommandBuffer(sizeof(PsRenderParameters) , sce::Gnm::kEmbeddedDataAlignment16));
	userData->m_sampler = sampler;
	userData->m_texture = texture;

	const std::array<float, 2> scale = { 2.f, -2.f };
	const std::array<float, 2> offset = { -1.f, 1.f };
	draw2dInternal(*dcb, colorCoeff, vertices, indices.get(), numIndices, sce::Gnm::kPrimitiveTypePointList, SHADER(sprite_utility_texture_p), SIZE_OF_SRT(sprite_utility_texture_p), userData, scale, offset, depth, 0.0f, psize);

	return SCE_OK;
}

int fillCube(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Matrix4_arg	model, Matrix4_arg	view, Matrix4_arg	projection, Vector3_arg	lightPosition, Vector4_arg	color, float	ambient, float	shininess,
	const sce::Gnmx::PsShader	*fillPs, uint32_t	psSrtSizeInDw, const void	*fillUserData)
{
	if (nullptr == fillUserData)
	{
		DefaultPsRender3DParameters * defaultPsRender3DParameters = nullptr;
		setupDefaultPsRender3DParameters(*dcb, defaultPsRender3DParameters, nullptr, color, lightPosition, ambient, -view.getTranslation( ), shininess);
		fillUserData = defaultPsRender3DParameters;
		psSrtSizeInDw = getSizeOfDefaultPsRender3DParametersInDwords( );
	}

	return	draw3DInternal(dcb, s_cubeMesh->m_vertexBuffers[0], s_cubeMesh->m_pIndexBuffer, s_cubeMesh->m_indexSizeInBytes / sizeof(sce::SampleUtil::Graphics::Index), &model, 1, view, projection, sce::Gnm::PrimitiveType::kPrimitiveTypeTriList, fillPs, fillUserData, psSrtSizeInDw);
}


int drawCube(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Matrix4_arg	model, Matrix4_arg	view, Matrix4_arg	projection, Vector4_arg	color, uint32_t	eighthWidth)
{
	DefaultPsRender3DParameters * defaultPsRender3DParameters = nullptr;
	setupDefaultPsRender3DParameters(*dcb, defaultPsRender3DParameters, nullptr, color);
	const uint32_t SizeOfDefaultPsRender3DParametersInDwords = SIZE_OF_SRT(sprite_utility_fill_p);

	return	draw3DInternal(dcb, s_cubeMesh->m_vertexBuffers[0], s_cubeMesh->m_pIndexBuffer, s_cubeMesh->m_indexSizeInBytes / sizeof(sce::SampleUtil::Graphics::Index), &model, 1, view, projection, sce::Gnm::PrimitiveType::kPrimitiveTypeTriList, SHADER(sprite_utility_fill_p), defaultPsRender3DParameters, SizeOfDefaultPsRender3DParametersInDwords, eighthWidth);
}

int fillSphere(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Matrix4_arg	model, Matrix4_arg	view, Matrix4_arg	projection, Vector3_arg	lightPosition, Vector4_arg	color, float ambient, float	shininess,
	const sce::Gnmx::PsShader	*fillPs, uint32_t	psSrtSizeInDw, const void	*fillUserData)
{
	if (nullptr == fillUserData)
	{
		DefaultPsRender3DParameters * defaultPsRender3DParameters = nullptr;
		setupDefaultPsRender3DParameters(*dcb, defaultPsRender3DParameters, nullptr, color, lightPosition, ambient, -view.getTranslation( ), shininess);
		fillUserData = defaultPsRender3DParameters;
		psSrtSizeInDw = getSizeOfDefaultPsRender3DParametersInDwords( );
	}
	
	return draw3DInternal(dcb, s_sphereMesh->m_vertexBuffers[0], s_sphereMesh->m_pIndexBuffer, s_sphereMesh->m_indexSizeInBytes / sizeof(sce::SampleUtil::Graphics::Index), &model, 1, view, projection, sce::Gnm::PrimitiveType::kPrimitiveTypeTriList, fillPs, fillUserData, psSrtSizeInDw);;
}

int drawSphere(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Matrix4_arg	model, Matrix4_arg view, Matrix4_arg	projection, Vector4_arg	color, uint32_t	eighthWidth)
{
	DefaultPsRender3DParameters * defaultPsRender3DParameters = nullptr;
	setupDefaultPsRender3DParameters(*dcb, defaultPsRender3DParameters, nullptr, color);
	const uint32_t SizeOfDefaultPsRender3DParametersInDwords = SIZE_OF_SRT(sprite_utility_fill_p);

	return draw3DInternal(dcb, s_sphereMesh->m_vertexBuffers[0], s_sphereMesh->m_pIndexBuffer, s_sphereMesh->m_indexSizeInBytes / sizeof(sce::SampleUtil::Graphics::Index), &model, 1, view, projection, sce::Gnm::PrimitiveType::kPrimitiveTypeTriList, SHADER(sprite_utility_fill_p), defaultPsRender3DParameters, SizeOfDefaultPsRender3DParametersInDwords, eighthWidth);
}

int fillCylinder(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Matrix4_arg	model, Matrix4_arg	view, Matrix4_arg	projection, Vector3_arg	lightPosition, Vector4_arg	color, float	ambient, float	shininess,
	const sce::Gnmx::PsShader	*fillPs, uint32_t	psSrtSizeInDw, const void	*fillUserData)
{
	if (nullptr == fillUserData)
	{
		DefaultPsRender3DParameters * defaultPsRender3DParameters = nullptr;
		setupDefaultPsRender3DParameters(*dcb, defaultPsRender3DParameters, nullptr, color, lightPosition, ambient, -view.getTranslation( ), shininess);
		fillUserData = defaultPsRender3DParameters;
		psSrtSizeInDw = getSizeOfDefaultPsRender3DParametersInDwords( );
	}

	return draw3DInternal(dcb, s_cylinderMesh->m_vertexBuffers[0], s_cylinderMesh->m_pIndexBuffer, s_cylinderMesh->m_indexSizeInBytes / sizeof(sce::SampleUtil::Graphics::Index), &model, 1, view, projection, sce::Gnm::PrimitiveType::kPrimitiveTypeTriList, fillPs, fillUserData, psSrtSizeInDw);
}

int drawCylinder(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Matrix4_arg	model, Matrix4_arg	view, Matrix4_arg	projection, Vector4_arg	color, uint32_t	eighthWidth)
{
	DefaultPsRender3DParameters * defaultPsRender3DParameters = nullptr;
	setupDefaultPsRender3DParameters(*dcb, defaultPsRender3DParameters, nullptr, color);
	const uint32_t SizeOfDefaultPsRender3DParametersInDwords = SIZE_OF_SRT(sprite_utility_fill_p);

	return draw3DInternal(dcb, s_cylinderMesh->m_vertexBuffers[0], s_cylinderMesh->m_pIndexBuffer, s_cylinderMesh->m_indexSizeInBytes / sizeof(sce::SampleUtil::Graphics::Index), &model, 1, view, projection, sce::Gnm::PrimitiveType::kPrimitiveTypeTriList, SHADER(sprite_utility_fill_p), defaultPsRender3DParameters, SizeOfDefaultPsRender3DParametersInDwords, eighthWidth);
}

int fillCone(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Matrix4_arg	model, Matrix4_arg	view, Matrix4_arg	projection, Vector3_arg	lightPosition, Vector4_arg	color, float	ambient, float	shininess,
	const sce::Gnmx::PsShader	*fillPs, uint32_t	psSrtSizeInDw, const void	*fillUserData)
{
	if (nullptr == fillUserData)
	{
		DefaultPsRender3DParameters * defaultPsRender3DParameters = nullptr;
		setupDefaultPsRender3DParameters(*dcb, defaultPsRender3DParameters, nullptr, color, lightPosition, ambient, -view.getTranslation( ), shininess);
		fillUserData = defaultPsRender3DParameters;
		psSrtSizeInDw = getSizeOfDefaultPsRender3DParametersInDwords( );
	}
	
	return draw3DInternal(dcb, s_coneMesh->m_vertexBuffers[0], s_coneMesh->m_pIndexBuffer, s_coneMesh->m_indexSizeInBytes / sizeof(sce::SampleUtil::Graphics::Index), &model, 1, view, projection, sce::Gnm::PrimitiveType::kPrimitiveTypeTriList, fillPs, fillUserData, psSrtSizeInDw);
}

int drawCone(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Matrix4_arg	model, Matrix4_arg	view, Matrix4_arg	projection, Vector4_arg	color, uint32_t	eighthWidth)
{
	DefaultPsRender3DParameters * defaultPsRender3DParameters = nullptr;
	setupDefaultPsRender3DParameters(*dcb, defaultPsRender3DParameters, nullptr, color);
	const uint32_t SizeOfDefaultPsRender3DParametersInDwords = SIZE_OF_SRT(sprite_utility_fill_p);

	return draw3DInternal(dcb, s_coneMesh->m_vertexBuffers[0], s_coneMesh->m_pIndexBuffer, s_coneMesh->m_indexSizeInBytes / sizeof(sce::SampleUtil::Graphics::Index), &model, 1, view, projection, sce::Gnm::PrimitiveType::kPrimitiveTypeTriList, SHADER(sprite_utility_fill_p), defaultPsRender3DParameters, SizeOfDefaultPsRender3DParametersInDwords, eighthWidth);
}


int	drawLine(
	sce::Gnm::DrawCommandBuffer	*dcb, 
	Matrix4_arg	view, Matrix4_arg	projection, Vector3_arg	begin, Vector3_arg	end, Vector4_arg	color, uint32_t	eighthWidth)
{
	DefaultPsRender3DParameters * defaultPsRender3DParameters = nullptr;
	setupDefaultPsRender3DParameters(*dcb, defaultPsRender3DParameters, nullptr, color);
	const uint32_t SizeOfDefaultPsRender3DParametersInDwords = SIZE_OF_SRT(sprite_utility_fill_p);

	auto vertices = reinterpret_cast<Vertex *>(dcb->allocateFromCommandBuffer(sizeof(Vertex) * 2, sce::Gnm::kEmbeddedDataAlignment16));
	SCE_SAMPLE_UTIL_ASSERT(vertices != nullptr);
	vertices[0].m_position	= begin;
	vertices[1].m_position	= end;
	sce::Gnm::Buffer	vertexBuffer;
	vertexBuffer.initAsRegularBuffer(vertices, sizeof(Vertex), 2);
	
	Matrix4 model = Matrix4::identity();
	return 	draw3DInternal(dcb, vertexBuffer, nullptr, vertexBuffer.getNumElements(), &model, 1, view, projection, sce::Gnm::PrimitiveType::kPrimitiveTypeLineList, SHADER(sprite_utility_fill_p), defaultPsRender3DParameters, SizeOfDefaultPsRender3DParametersInDwords, eighthWidth);
}

int	drawTexture(
	sce::Gnm::DrawCommandBuffer	*dcb,
	Matrix4_arg	model, Matrix4_arg	view, Matrix4_arg	projection, Vector2_arg	textureOffset, Vector2_arg	sizeInTexture,
	const sce::Gnm::Texture	&texture, const sce::Gnm::Sampler	&sampler,
	Vector4_arg	colorCoeff
)
{
	struct UserData
	{
		sce::Gnm::Sampler	sampler;
		sce::Gnm::Texture	texture;
	} *userData = reinterpret_cast<UserData *>(dcb->allocateFromCommandBuffer(sizeof(UserData), sce::Gnm::EmbeddedDataAlignment::kEmbeddedDataAlignment16));
	userData->sampler = sampler;
	userData->texture = texture;

	auto vertices = reinterpret_cast<Vertex *>(dcb->allocateFromCommandBuffer(sizeof(Vertex) * 6, sce::Gnm::EmbeddedDataAlignment::kEmbeddedDataAlignment16));
	SCE_SAMPLE_UTIL_ASSERT(vertices != nullptr);
	vertices[0].m_position = Vector3Unaligned{ -1, 1, 0 };
	vertices[1].m_position = Vector3Unaligned{ -1,-1, 0 };
	vertices[2].m_position = Vector3Unaligned{  1, 1, 0 };
	vertices[3].m_position = Vector3Unaligned{  1, 1, 0 };
	vertices[4].m_position = Vector3Unaligned{ -1,-1, 0 };
	vertices[5].m_position = Vector3Unaligned{  1,-1, 0 };
	vertices[0].m_uv = Vector2Unaligned{ textureOffset.getX()                       , textureOffset.getY() };
	vertices[1].m_uv = Vector2Unaligned{ textureOffset.getX()                       , textureOffset.getY() + sizeInTexture.getY() };
	vertices[2].m_uv = Vector2Unaligned{ textureOffset.getX() + sizeInTexture.getX(), textureOffset.getY() };
	vertices[3].m_uv = Vector2Unaligned{ textureOffset.getX() + sizeInTexture.getX(), textureOffset.getY() };
	vertices[4].m_uv = Vector2Unaligned{ textureOffset.getX()                       , textureOffset.getY() + sizeInTexture.getY() };
	vertices[5].m_uv = Vector2Unaligned{ textureOffset.getX() + sizeInTexture.getX(), textureOffset.getY() + sizeInTexture.getY() };
	sce::Gnm::Buffer vertexBuffer;
	vertexBuffer.initAsRegularBuffer(vertices, sizeof(Vertex), 6);

	DefaultPsRender3DParameters *defaultPsRender3DParameters;
	setupDefaultPsRender3DParameters(*dcb, defaultPsRender3DParameters, userData, colorCoeff);
	const uint32_t sizeOfDefaultPsRender3DParametersInDwords = SIZE_OF_SRT(sprite_utility_texture_p);

	return	draw3DInternal(dcb, vertexBuffer, nullptr, vertexBuffer.getNumElements(), &model, 1, view, projection, sce::Gnm::kPrimitiveTypeTriList, SHADER(sprite_utility_texture_p), defaultPsRender3DParameters, sizeOfDefaultPsRender3DParametersInDwords);
}

int renderQuads(
	sce::Gnm::DrawCommandBuffer	*dcb,
	Matrix4_arg	view, Matrix4_arg	projection, const Matrix4	*models, uint32_t	numModels,
	const sce::Gnmx::PsShader	*psShader, const void	*psUserData, uint32_t	psUserDataSizeInDw,
	bool	isModelsBufferManagedByUser
)
{
	auto vertices = reinterpret_cast<Vertex *>(dcb->allocateFromCommandBuffer(sizeof(Vertex) * 4, sce::Gnm::EmbeddedDataAlignment::kEmbeddedDataAlignment16));
	SCE_SAMPLE_UTIL_ASSERT(vertices != nullptr);
	vertices[0].m_position = Vector3Unaligned{ -1, 1, 0 };
	vertices[1].m_position = Vector3Unaligned{ -1,-1, 0 };
	vertices[2].m_position = Vector3Unaligned{  1,-1, 0 };
	vertices[3].m_position = Vector3Unaligned{  1, 1, 0 };
	vertices[0].m_uv = Vector2Unaligned{ 0, 0 };
	vertices[1].m_uv = Vector2Unaligned{ 0, 1 };
	vertices[2].m_uv = Vector2Unaligned{ 1, 1 };
	vertices[3].m_uv = Vector2Unaligned{ 1, 0 };
	sce::Gnm::Buffer vertexBuffer;
	vertexBuffer.initAsRegularBuffer(vertices, sizeof(Vertex), 4);
	return draw3DInternal(dcb, vertexBuffer, nullptr, vertexBuffer.getNumElements(), models, numModels, view, projection, sce::Gnm::PrimitiveType::kPrimitiveTypeQuadList, psShader, psUserData, psUserDataSizeInDw, 0.0f, 0.0f, !isModelsBufferManagedByUser);
}

} /* namespace SpriteUtil */

} /* namespace Graphics */ } /* namespace SampleUtil */ } /* namespace sce */

#endif //_SCE_TARGET_OS_PROSPERO
