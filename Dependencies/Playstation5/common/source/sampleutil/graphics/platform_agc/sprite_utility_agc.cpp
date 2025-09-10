/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc. 
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO

#include <memory>
#include <agc/core.h>

#include "sampleutil/graphics/hdr.h"
#include "sampleutil/graphics/platform_agc/shader_ags.h"
#include "sampleutil/sampleutil_error.h"
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/graphics/sprite_utility.h"
#include "sampleutil/graphics/platform_agc/link_libraries_agc.h"


DEFINE_SHADER(sprite_utility_2d_vv);
DEFINE_SHADER(sprite_utility_fill_p);

DEFINE_SHADER(sprite_utility_texture_p);
DEFINE_SHADER(sprite_utility_texture_ycbcr_p);
DEFINE_SHADER(sprite_utility_texture_yuy2_p);

DEFINE_SHADER(sprite_utility_3d_vv);
DEFINE_SHADER(sprite_utility_3d_fill_p);

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
	sce::Agc::Core::Buffer											s_ovalMeshVertexBuffer;

	std::unique_ptr<sce::SampleUtil::Graphics::BoxMesh>				s_cubeMesh;
	std::unique_ptr<sce::SampleUtil::Graphics::SphereMesh>			s_sphereMesh;
	std::unique_ptr<sce::SampleUtil::Graphics::CylinderMesh>		s_cylinderMesh;
	std::unique_ptr<sce::SampleUtil::Graphics::ConeMesh>			s_coneMesh;

	SceError bindShader( sce::Agc::DrawCommandBuffer &dcb, const sce::Agc::Shader &shader)
	{
		if (shader.m_numShRegisters)
		{
			sce::Agc::PacketAddress pa = dcb.setShRegistersIndirect(shader.m_shRegisters, shader.m_numShRegisters);
			SCE_SAMPLE_UTIL_ASSERT_MSG(pa != nullptr, "Unable to write setShRegistersIndirect packet." );
			if (pa == nullptr)
			{
				return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			}
		}
		if (shader.m_numCxRegisters)
		{
			sce::Agc::PacketAddress pa = dcb.setCxRegistersIndirect(shader.m_cxRegisters, shader.m_numCxRegisters);
			SCE_SAMPLE_UTIL_ASSERT_MSG(pa != nullptr, "Unable to write setCxRegistersIndirect packet." );
			if (pa == nullptr)
			{
				return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			}
		}
		return SCE_OK;
	}

	void	setShaderPair(sce::Agc::DrawCommandBuffer &dcb, sce::Agc::Core::StateBuffer &sb, const sce::Agc::Shader &gsShader, const sce::Agc::Shader *psShader, const sce::Agc::UcPrimitiveType::Type &primitiveType)
	{
		sce::Agc::CxShaderLinkage cxLinkage;
		sce::Agc::UcPrimitiveState ucLinkage;

		sce::Agc::Core::linkShaders(&cxLinkage, &ucLinkage, nullptr, &gsShader, psShader, primitiveType);

		bindShader(dcb, gsShader);
		if (psShader != nullptr) {
			bindShader(dcb, *psShader);
		}

		sb.setState(cxLinkage)
			.setState(ucLinkage)
			.setState(sce::Agc::UcIndexOffset().init());
		if (psShader == nullptr) {
			sb.setState(sce::Agc::CxDbShaderControl().init())
				.setState(sce::Agc::CxShaderOutputMask().init());
		}
	}

	sce::Agc::SetRegRangeDirectPacketAddress	bindShaderResourceTable(sce::Agc::DrawCommandBuffer	&dcb, const sce::Agc::Shader &shader, const void	*srtData, uint16_t	&srtSizeInDwords)
	{
		int ret = SCE_OK; (void)ret;
		sce::Agc::RegisterRange location = sce::Agc::Core::getResourceUserDataRange(&shader, sce::Agc::UserDataLayout::DirectResourceType::kShaderResourceTable);
		sce::Agc::SetRegRangeDirectPacketAddress packetAddr;
		ret = sce::Agc::bindUserData(&packetAddr, &dcb, shader.m_type, location.m_start, srtData, location.size());
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		srtSizeInDwords = location.size();

		return packetAddr;
	}
	
	void	setupDrawState(sce::Agc::DrawCommandBuffer	&dcb, sce::Agc::Core::StateBuffer	&sb, const sce::Agc::UcPrimitiveType::Type	primType, const sce::Agc::Shader	&vsShader, const sce::Agc::Shader	*psShader, float eighthWidth = 0.f, float ptSize = 0.f)
	{
		const bool isLine = (eighthWidth > 0.f);
		dcb.setIndexSize(sce::Agc::IndexSize::k16);
		// Set Shaders
		setShaderPair(dcb, sb, vsShader, psShader, primType);

		// Setup RenderStates
		sb
			.setState(isLine ? sce::Agc::CxPrimitiveSetup().init()
				.setPolygonMode(sce::Agc::CxPrimitiveSetup::PolygonMode::kEnable)
				.setFrontPolygonMode(sce::Agc::CxPrimitiveSetup::FrontPolygonMode::kLine)
				.setBackPolygonMode(sce::Agc::CxPrimitiveSetup::BackPolygonMode::kLine) : sce::Agc::CxPrimitiveSetup().init())
			.setState(sce::Agc::CxBlendControl().init()
				.setBlend(s_isAlphaBlendEnabled ? sce::Agc::CxBlendControl::Blend::kEnable : sce::Agc::CxBlendControl::Blend::kDisable)
				.setAlphaBlendFunc(sce::Agc::CxBlendControl::AlphaBlendFunc::kAdd)
				.setAlphaDestMultiplier(sce::Agc::CxBlendControl::AlphaDestMultiplier::kOneMinusSrcAlpha)
				.setAlphaSourceMultiplier(sce::Agc::CxBlendControl::AlphaSourceMultiplier::kOne)
				.setColorBlendFunc(sce::Agc::CxBlendControl::ColorBlendFunc::kAdd)
				.setColorDestMultiplier(sce::Agc::CxBlendControl::ColorDestMultiplier::kOneMinusSrcAlpha)
				.setColorSourceMultiplier(sce::Agc::CxBlendControl::ColorSourceMultiplier::kSrcAlpha))
			.setState(sce::Agc::UcPrimitiveType().init().setType(primType))
			.setState(sce::Agc::CxLineWidth().init().setEighth((uint32_t)eighthWidth))
			.setState(sce::Agc::CxPointSize().init().setHalfWidth(sce::Agc::FixedPointU12_4().setValue(ptSize / 2.f)).setHalfHeight(sce::Agc::FixedPointU12_4().setValue(ptSize / 2.f)));
	}

	sce::Agc::Toolkit::Result	draw2dInternal(
		sce::Agc::DrawCommandBuffer				*dcb,
		sce::Agc::Core::StateBuffer				*sb,
		sce::Agc::Toolkit::RestoreCxState		restoreCxState,
		Vector4_arg								rgba,
		const sce::Agc::Core::Buffer			&vertexBuffer,
		const sce::SampleUtil::Graphics::Vertex	*pVerticesOnCpuMem,
		const void								*pIndices,
		uint32_t								numIndices,
		sce::Agc::UcPrimitiveType::Type			primType,
		sce::Agc::Shader						*pPsShader,
		void									*pUserData,
		const std::array<float,2>				&scale,
		const std::array<float,2>				&offset,
		float									depth,
		float									eighthWidth = 0.f,
		float									ptSize = 0.f,
		const void								*pUserDataOnCpuMem = nullptr,
		uint32_t								userDataSizeInBytes = 0
	)
	{
		int ret = SCE_OK; (void)ret;

		if (dcb == nullptr || sb == nullptr)
		{
			return { SCE_SAMPLE_UTIL_ERROR_NULL_POINTER,
				sce::Agc::Toolkit::Result::StateChange::kNone,
				sce::Agc::Toolkit::Result::ActiveWork::kNone,
				sce::Agc::Toolkit::Result::Caches::kNone };
		}

		// Conditionally save the Cx state.
		if (restoreCxState == sce::Agc::Toolkit::RestoreCxState::kEnable)
		{
			sb->postDraw(); // Ensure we're not appending to a previous indirect state packet.
			dcb->contextStateOp(sce::Agc::ContextStateOperation::kPushState); // Just push, do not clear.
		}

		setupDrawState(*dcb, *sb, primType, *::Shader::sprite_utility_2d_vv, pPsShader, eighthWidth, ptSize);

		// We need to store various shader parameters somewhere the shader can read, in this case the command buffer itself.
		// For compatibility with Core::RingBuffer, we ensure that the value and the draw packet are in the
		// same block segment, so that the memory can't be reclaimed before the draw has finished executing.
		// To do this, we write the whole sequence into a temprary DCB on the stack and then insert it in one go.
		// After that, we still need to patch all the pointers to these parameters, since they will be moved as well.
		uint32_t allocSize = 0;
		if (pVerticesOnCpuMem != nullptr) allocSize += sizeof(sce::SampleUtil::Graphics::Vertex) * vertexBuffer.getNumElements();
		if (pUserDataOnCpuMem != nullptr && pPsShader != nullptr) allocSize += userDataSizeInBytes;
		char *tempDcbMemory = (char *)alloca(allocSize + 256); // Temporary packet storage.
		sce::Agc::DrawCommandBuffer tempDcb;
		tempDcb.init(tempDcbMemory, allocSize + 256);

		// This NOP will hold the all shader parameters.
		auto nopPacket = tempDcb.nop(allocSize / 4 + 1);

		//  bind SRTs with nullptr here. actual content will be patched later
		uint16_t vsSrtSizeInDwords = 0;
		auto setVsSrtPacketAddr = bindShaderResourceTable(tempDcb, *::Shader::sprite_utility_2d_vv, nullptr, vsSrtSizeInDwords);
		sce::Agc::SetRegRangeDirectPacketAddress setPsSrtPacketAddr;
		uint16_t psSrtSizeInDwords = 0;
		if (pPsShader != nullptr) {
			setPsSrtPacketAddr = bindShaderResourceTable(tempDcb, *pPsShader, nullptr, psSrtSizeInDwords);
		}

		// Draw Call
		if (pIndices != nullptr)
		{
			tempDcb.drawIndex(numIndices, pIndices, ::Shader::sprite_utility_2d_vv->m_specials->m_drawModifier);
		} else {
			tempDcb.drawIndexAuto(numIndices, ::Shader::sprite_utility_2d_vv->m_specials->m_drawModifier);
		}

		// At this point, we can insert this buffer into the existing dcb.
		sce::Agc::PacketAddress newBase = dcb->insertPackets(tempDcb.getSubmitPointer(), tempDcb.getSubmitSize());

		// Next, we move the old PacketAddress pointers to their new position.
		ptrdiff_t dcbDiff = newBase - tempDcb.getSubmitPointer();

		nopPacket += dcbDiff;
		setVsSrtPacketAddr += dcbDiff;
		if (pPsShader != nullptr) {
			setPsSrtPacketAddr += dcbDiff;
		}

		// Then get the new address of nop payload and SRTs
		uint32_t	*pNopPayload = nullptr;
		if (allocSize > 0)
		{
			ret = sce::Agc::getNopPayloadAddress(&pNopPayload, nopPacket);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}

		// create SRT content
		struct __attribute__((packed)) VsSrtData
		{
			sce::Agc::Core::Buffer	m_vertexBuffer;
			float					m_scale[2];
			float					m_offset[2];
			float					m_depth;
			uint32_t				__padding__[1];
			void					*m_pUserData;
		} vsSrtData = { vertexBuffer, {scale[0], scale[1]}, {offset[0], offset[1]}, depth, {}, pUserData };

		struct __attribute__((packed)) PsRenderParameters 
		{
			vec_float4	m_rgba;
			int			m_hdr;
			uint32_t	__padding__[1];
			void		*m_pUserData;
		} psRenderParameters = { rgba.get128(), (int)sce::SampleUtil::Graphics::getHdr(), {}, pUserData };

		// Assign nop payload to various render parameters, and fill them
		uint8_t *ptrInNopPayload = (uint8_t *)pNopPayload;
		if (pVerticesOnCpuMem != nullptr)
		{
			// copy vertices to nop payload memory
			memcpy(ptrInNopPayload, pVerticesOnCpuMem, sizeof(sce::SampleUtil::Graphics::Vertex) * vertexBuffer.getNumElements());
			// then patch vertex buffer data address with new pointer
			vsSrtData.m_vertexBuffer.setDataAddress(ptrInNopPayload);
			ptrInNopPayload += sizeof(sce::SampleUtil::Graphics::Vertex) * vertexBuffer.getNumElements();
		}
		if (pUserDataOnCpuMem != nullptr && pPsShader != nullptr)
		{
			// copy user data to nop payload memory
			memcpy(ptrInNopPayload, pUserDataOnCpuMem, userDataSizeInBytes);
			// then patch user data pointer in SRT with new pointer
			vsSrtData.m_pUserData = ptrInNopPayload;
			psRenderParameters.m_pUserData = ptrInNopPayload;
		}

		// patch real SRT by memcpy
		VsSrtData *pVsSrtData = nullptr;
		ret = sce::Agc::getSetRegRangeDirectPayloadAddress((uint32_t **)&pVsSrtData, setVsSrtPacketAddr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		memcpy(pVsSrtData, &vsSrtData, vsSrtSizeInDwords * 4);

		if (pPsShader != nullptr) {
			PsRenderParameters *pPsRenderParameters = nullptr;
			ret = sce::Agc::getSetRegRangeDirectPayloadAddress((uint32_t **)&pPsRenderParameters, setPsSrtPacketAddr);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			memcpy(pPsRenderParameters, &psRenderParameters, psSrtSizeInDwords * 4);
		}

		sb->postDraw();

		if (restoreCxState == sce::Agc::Toolkit::RestoreCxState::kEnable)
		{
			dcb->contextStateOp(sce::Agc::ContextStateOperation::kPopState); // Just push, do not clear.
		}

		// Note that this function by itself does not consider the CB or DB caches dirtied, since the PS might not touch them.
		return { SCE_OK, 
			((pPsShader != nullptr) ? sce::Agc::Toolkit::Result::StateChange::kPsSh : sce::Agc::Toolkit::Result::StateChange::kNone) | 
			sce::Agc::Toolkit::Result::StateChange::kGsSh |
			sce::Agc::Toolkit::Result::StateChange::kDraw,
			(pPsShader != nullptr) ? sce::Agc::Toolkit::Result::ActiveWork::kGsPs : sce::Agc::Toolkit::Result::ActiveWork::kGs,
			((pPsShader != nullptr) ? sce::Agc::Toolkit::Result::Caches::kCbData : sce::Agc::Toolkit::Result::Caches::kNone) |
			sce::Agc::Toolkit::Result::Caches::kGl1 |
			sce::Agc::Toolkit::Result::Caches::kGl2 };
	}

	sce::Agc::Toolkit::Result	drawTextureInternal(
		sce::Agc::DrawCommandBuffer			*dcb,
		sce::Agc::Core::StateBuffer			*sb,
		sce::Agc::Toolkit::RestoreCxState	restoreCxState,
		sce::Agc::Shader					*pPsShader,
		void								*pUserData,
		Vector2_arg							position,
		Vector2_arg							size,
		Vector2_arg							textureOffset,
		Vector2_arg							sizeInTexture,
		Vector4_arg							colorCoeff,
		float								depth,
		void								*pUserDataOnCpuMem,
		uint32_t							userDataSizeInBytes
	)
	{
		std::array<sce::SampleUtil::Graphics::Vertex, 3> vertices;
		vertices[0].m_position	= ToVector3Unaligned(Vector3(position                            , 0.f));
		vertices[1].m_position	= ToVector3Unaligned(Vector3(position + Vector2(size.getX(), 0.f), 0.f));
		vertices[2].m_position	= ToVector3Unaligned(Vector3(position + Vector2(0.f, size.getY()), 0.f));
		vertices[0].m_uv		= ToVector2Unaligned(textureOffset);
		vertices[1].m_uv		= ToVector2Unaligned(textureOffset + Vector2(sizeInTexture.getX(), 0.f));
		vertices[2].m_uv		= ToVector2Unaligned(textureOffset + Vector2(0.f, sizeInTexture.getY()));
		sce::Agc::Core::Buffer	vertexBuffer;
		sce::Agc::Core::initialize(&vertexBuffer, &sce::Agc::Core::BufferSpec().initAsRegularBuffer(nullptr/* dummy data address*/, sizeof(sce::SampleUtil::Graphics::Vertex), vertices.size()));

		const std::array<float, 2> scale = { 2.f, -2.f };
		const std::array<float, 2> offset = { -1.f, 1.f };
		return draw2dInternal(dcb, sb, restoreCxState, colorCoeff, vertexBuffer, vertices.data(), nullptr, vertices.size(), sce::Agc::UcPrimitiveType::Type::kRectList, pPsShader, pUserData, scale, offset, depth, 0.f, 0.f, pUserDataOnCpuMem, userDataSizeInBytes);
	}

	struct __attribute__((packed)) DefaultPsRender3DParameters
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

	struct __attribute__((packed)) VsRenderParameters
	{
		sce::Agc::Core::Buffer	m_vertexBuffer;
		sce::Agc::Core::Buffer	m_modelMatrixBuffer;
		Matrix4Unaligned		m_viewMatrix;
		Matrix4Unaligned		m_projectionMatrix;
	};
	
	void setupDefaultPsRender3DParameters(
		DefaultPsRender3DParameters &outDefaultParameters,
		Vector4_arg colorRGBA,
		Vector3_arg lightPosition = sce::Vectormath::Simd::Aos::Vector3( 0.0f, 0.0f, 0.0f ),
		float ambient = 0.0f,
		Vector3_arg viewPosition = sce::Vectormath::Simd::Aos::Vector3( 0.0f, 0.0f, 0.0f ),
		float shininess = 0.0f )
	{
		memset(&outDefaultParameters, 0xCE, sizeof(DefaultPsRender3DParameters));
		outDefaultParameters.m_rgba				= ToVector4Unaligned(colorRGBA);
		outDefaultParameters.m_hdr				= (int)sce::SampleUtil::Graphics::getHdr();
		outDefaultParameters.__padding__		= 0;
		outDefaultParameters.m_pOption			= nullptr;
		outDefaultParameters.m_lightPosition	= ToVector3Unaligned(lightPosition);
		outDefaultParameters.m_ambient			= ambient;
		outDefaultParameters.m_viewPosition		= ToVector3Unaligned(viewPosition);
		outDefaultParameters.m_shininess		= shininess;
	}

	sce::Agc::Toolkit::Result	draw3DInternal(
		sce::Agc::DrawCommandBuffer				*dcb,
		sce::Agc::Core::StateBuffer				*sb,
		sce::Agc::Toolkit::RestoreCxState		restoreCxState,
		const sce::Agc::Core::Buffer			&vertexBuffer,
		const void								*pIndices,
		uint32_t								numIndices,
		const Matrix4							*modelMatrices,
		uint32_t								numInstances,
		Matrix4_arg								viewMat,
		Matrix4_arg								projMat,
		sce::Agc::UcPrimitiveType::Type			primType,
		const sce::Agc::Shader					*pPsShader,
		const void								*pPsShaderUserData,
		float									eighthWidth = 0.f,
		float									ptSize = 0.f,
		bool									internalAllocateOfmodelMatricesBuffer = true,
		const sce::SampleUtil::Graphics::Vertex	*pVerticesOnCpuMem = nullptr,
		void									*pOptionOnCpuMem = nullptr,
		uint32_t								optionSizeInBytes = 0
	)
	{
		int ret = SCE_OK; (void)ret;

		if (dcb == nullptr || sb == nullptr )
		{
			return { SCE_SAMPLE_UTIL_ERROR_NULL_POINTER,
				sce::Agc::Toolkit::Result::StateChange::kNone,
				sce::Agc::Toolkit::Result::ActiveWork::kNone,
				sce::Agc::Toolkit::Result::Caches::kNone };
		}

		// Conditionally save the Cx state.
		if (restoreCxState == sce::Agc::Toolkit::RestoreCxState::kEnable)
		{
			sb->postDraw(); // Ensure we're not appending to a previous indirect state packet.
			dcb->contextStateOp(sce::Agc::ContextStateOperation::kPushState); // Just push, do not clear.
		}

		setupDrawState(*dcb, *sb, primType, *::Shader::sprite_utility_3d_vv, pPsShader, eighthWidth, ptSize);

		dcb->setNumInstances(std::max(1u, numInstances));

		// We need to store various shader parameters somewhere the shader can read, in this case the command buffer itself.
		// For compatibility with Core::RingBuffer, we ensure that the value and the draw packet are in the
		// same block segment, so that the memory can't be reclaimed before the draw has finished executing.
		// To do this, we write the whole sequence into a temprary DCB on the stack and then insert it in one go.
		// After that, we still need to patch all the pointers to these parameters, since they will be moved as well.
		uint32_t allocSize = sizeof(VsRenderParameters);
		if (internalAllocateOfmodelMatricesBuffer) allocSize += sizeof(Matrix4Unaligned) * numInstances;
		if (pVerticesOnCpuMem != nullptr) allocSize += sizeof(sce::SampleUtil::Graphics::Vertex) * vertexBuffer.getNumElements();
		if (pOptionOnCpuMem != nullptr && pPsShader != nullptr) allocSize += optionSizeInBytes;
		char *tempDcbMemory = (char *)alloca(allocSize + 256); // Temporary packet storage.
		sce::Agc::DrawCommandBuffer tempDcb;
		tempDcb.init(tempDcbMemory, allocSize + 256);

		// This NOP will hold the all shader parameters.
		auto nopPacket = tempDcb.nop(allocSize / 4 + 1);

		//  bind SRTs
		uint16_t vsSrtSizeInDwords = 0; (void)vsSrtSizeInDwords;
		auto setVsSrtPacketAddr = bindShaderResourceTable(tempDcb, *::Shader::sprite_utility_3d_vv, nullptr, vsSrtSizeInDwords);
		uint16_t psSrtSizeInDwords = 0; (void)psSrtSizeInDwords;
		sce::Agc::SetRegRangeDirectPacketAddress	setPsSrtPacketAddr;
		if (pPsShader != nullptr) {
			setPsSrtPacketAddr = bindShaderResourceTable(tempDcb, *pPsShader, pPsShaderUserData, psSrtSizeInDwords);
		}

		// Draw Call
		if (pIndices != nullptr)
		{
			tempDcb.drawIndex(numIndices, pIndices, ::Shader::sprite_utility_3d_vv->m_specials->m_drawModifier);
		} else {
			tempDcb.drawIndexAuto(numIndices, ::Shader::sprite_utility_3d_vv->m_specials->m_drawModifier);
		}

		// At this point, we can insert this buffer into the existing dcb.
		sce::Agc::PacketAddress newBase = dcb->insertPackets(tempDcb.getSubmitPointer(), tempDcb.getSubmitSize());

		// Next, we move the old PacketAddress pointers to their new position.
		ptrdiff_t dcbDiff = newBase - tempDcb.getSubmitPointer();

		nopPacket += dcbDiff;
		setVsSrtPacketAddr += dcbDiff;
		if (pPsShader != nullptr) {
			setPsSrtPacketAddr += dcbDiff;
		}

		// Then get the new address of nop payload and SRTs
		uint32_t	*pNopPayload = nullptr;
		ret = sce::Agc::getNopPayloadAddress(&pNopPayload, nopPacket);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		struct __attribute__((packed)) VsUserSrtData
		{
			VsRenderParameters *m_vsRenderParameters;
		} *pVsUserSrtData = nullptr;
		ret = sce::Agc::getSetRegRangeDirectPayloadAddress((uint32_t **)&pVsUserSrtData, setVsSrtPacketAddr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		DefaultPsRender3DParameters *pPsRenderParams = nullptr;
		if (pPsShader != nullptr) {
			ret = sce::Agc::getSetRegRangeDirectPayloadAddress((uint32_t **)&pPsRenderParams, setPsSrtPacketAddr);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}

		// Assign nop payload to various render parameters, and fill them
		uint8_t *ptrInNopPayload = (uint8_t *)pNopPayload;
		auto *pVsRenderParameters = (VsRenderParameters *)ptrInNopPayload;
		ptrInNopPayload += sizeof(VsRenderParameters);
		memcpy(pVsUserSrtData, &pVsRenderParameters, sizeof(VsRenderParameters *)); // assign pointer to variable by memcpy to avoid miss alignment UBSAN error
		pVsRenderParameters->m_vertexBuffer		= vertexBuffer;
		pVsRenderParameters->m_viewMatrix		= ToMatrix4Unaligned(viewMat);
		pVsRenderParameters->m_projectionMatrix	= ToMatrix4Unaligned(projMat);

		if (internalAllocateOfmodelMatricesBuffer)
		{
			// copy matrices to nop payload memory
			memcpy(ptrInNopPayload, modelMatrices, sizeof(Matrix4Unaligned) * numInstances);
			// then patch model matrices pointer with new pointer
			modelMatrices = (Matrix4 *)ptrInNopPayload;
			ptrInNopPayload += sizeof(Matrix4Unaligned) * numInstances;
		}
		sce::Agc::Core::Buffer matricesBuffer;
		sce::Agc::Core::initialize(&matricesBuffer, &sce::Agc::Core::BufferSpec().initAsRegularBuffer(modelMatrices, sizeof(Matrix4), numInstances));
		memcpy((char *)pVsRenderParameters + offsetof(VsRenderParameters, m_modelMatrixBuffer), &matricesBuffer, sizeof(matricesBuffer)); // create Agc::Core::Buffer in temp variable, then copy it to final destination to avoid miss align UBSAN error
		if (pVerticesOnCpuMem != nullptr)
		{
			// copy vertices to nop payload memory
			memcpy(ptrInNopPayload, pVerticesOnCpuMem, sizeof(sce::SampleUtil::Graphics::Vertex) * vertexBuffer.getNumElements());
			// then patch vertex buffer data address with new pointer
			pVsRenderParameters->m_vertexBuffer.setDataAddress(ptrInNopPayload);
			ptrInNopPayload += sizeof(sce::SampleUtil::Graphics::Vertex) * vertexBuffer.getNumElements();
		}

		if (pOptionOnCpuMem != nullptr && pPsShader != nullptr)
		{
			// copy option to nop payload memory
			memcpy(ptrInNopPayload, pOptionOnCpuMem, optionSizeInBytes);
			// then patch option in pixel shader SRT with new pointer
			memcpy(((char *)pPsRenderParams) + offsetof(DefaultPsRender3DParameters, m_pOption), &ptrInNopPayload, sizeof(void *)); // assign to pointer variable by memcpy to avoid miss alignment UBSAN error
		}

		if (numInstances > 1)
		{	// restore.
			dcb->setNumInstances(1);
		}

		sb->postDraw();

		if (restoreCxState == sce::Agc::Toolkit::RestoreCxState::kEnable)
		{
			dcb->contextStateOp(sce::Agc::ContextStateOperation::kPopState); // Just push, do not clear.
		}

		// Note that this function by itself does not consider the CB or DB caches dirtied, since the PS might not touch them.
		return { SCE_OK, 
			((pPsShader != nullptr) ? sce::Agc::Toolkit::Result::StateChange::kPsSh : sce::Agc::Toolkit::Result::StateChange::kNone) | 
			sce::Agc::Toolkit::Result::StateChange::kGsSh |
			sce::Agc::Toolkit::Result::StateChange::kDraw,
			(pPsShader != nullptr) ? sce::Agc::Toolkit::Result::ActiveWork::kGsPs : sce::Agc::Toolkit::Result::ActiveWork::kGs,
			((pPsShader != nullptr) ? sce::Agc::Toolkit::Result::Caches::kCbData : sce::Agc::Toolkit::Result::Caches::kNone) |
			sce::Agc::Toolkit::Result::Caches::kGl1 |
			sce::Agc::Toolkit::Result::Caches::kGl2 };
	}
} /* namespace anonymous */

namespace sce {	namespace SampleUtil { namespace Graphics {
namespace SpriteUtil {

sce::Agc::Toolkit::Result	g_result;

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
	CREATE_SHADER(sprite_utility_fill_p);

	CREATE_SHADER(sprite_utility_2d_vv);
	CREATE_SHADER(sprite_utility_texture_p);
	CREATE_SHADER(sprite_utility_texture_ycbcr_p);
	CREATE_SHADER(sprite_utility_texture_yuy2_p);

	CREATE_SHADER(sprite_utility_3d_vv);
	CREATE_SHADER(sprite_utility_3d_fill_p);
	
	// Setup Oval Mesh.
	s_ovalMeshVertices = Memory::Gpu::make_unique<Vertex>(kNumberOfOvalMeshVertices, Agc::Alignment::kBuffer, { Agc::ResourceRegistration::ResourceType::kVertexBufferBaseAddress } , videoMemory, "sce::SampleUtil::SpriteUtil::OvalMeshVertices" );
	s_ovalFillIndices = Memory::Gpu::make_unique<uint16_t>(kNumberOfOvalFillIndices, Agc::Alignment::kBuffer, { Agc::ResourceRegistration::ResourceType::kIndexBufferBaseAddress } , videoMemory, "sce::SampleUtil::SpriteUtil::OvalFillIndices" );
	s_ovalDrawIndices = Memory::Gpu::make_unique<uint16_t>(kNumberOfOvalDrawIndices, Agc::Alignment::kBuffer, { Agc::ResourceRegistration::ResourceType::kIndexBufferBaseAddress } , videoMemory, "sce::SampleUtil::SpriteUtil::OvalDrawIndices" );
	
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

	Agc::Core::initialize(&s_ovalMeshVertexBuffer, &Agc::Core::BufferSpec().initAsRegularBuffer(s_ovalMeshVertices.get(), sizeof(Vertex), kNumberOfOvalMeshVertices));

	// Setup 3D Mesh.
	s_cubeMesh = std::make_unique<SampleUtil::Graphics::BoxMesh>(videoMemory, 1.f, 1.f, 1.f);
	SCE_SAMPLE_UTIL_ASSERT( nullptr != s_cubeMesh.get( ) );

	s_sphereMesh = std::make_unique<SampleUtil::Graphics::SphereMesh>(videoMemory, 0.5f, 32.f, 32.f);
	SCE_SAMPLE_UTIL_ASSERT( nullptr != s_sphereMesh.get( ) );

	s_cylinderMesh = std::make_unique<SampleUtil::Graphics::CylinderMesh>(videoMemory, 0.5f, 0.5f, 64);
	SCE_SAMPLE_UTIL_ASSERT( nullptr != s_cylinderMesh.get( ) );

	s_coneMesh = std::make_unique<SampleUtil::Graphics::ConeMesh>(videoMemory, 0.5f, 1.0f, 32.0f);
	SCE_SAMPLE_UTIL_ASSERT( nullptr != s_coneMesh.get( ) );

	return SCE_OK;
}

int finalize()
{
	s_ovalMeshVertices.reset( nullptr );
	s_ovalFillIndices.reset( nullptr );
	s_ovalDrawIndices.reset( nullptr );

	s_cubeMesh.reset( nullptr );
	s_sphereMesh.reset( nullptr );
	s_cylinderMesh.reset( nullptr );
	s_coneMesh.reset( nullptr );

	return SCE_OK;
}

Agc::Toolkit::Result	fillRect(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Vector2_arg	position,
	Vector2_arg	size,
	Vector4_arg	rgba, 
	float depth,
	Agc::Shader	*fillPs,
	void	*fillUserData)
{
	std::array<Vertex, 3> vertices;
	vertices[0].m_position = ToVector3Unaligned(Vector3(position                            , 0.f));
	vertices[1].m_position = ToVector3Unaligned(Vector3(position + Vector2(size.getX(), 0.f), 0.f));
	vertices[2].m_position = ToVector3Unaligned(Vector3(position + Vector2(0.f, size.getY()), 0.f));
	Agc::Core::Buffer vertexBuffer;
	Agc::Core::initialize(&vertexBuffer, &Agc::Core::BufferSpec().initAsRegularBuffer(nullptr/* dummy data addess*/, sizeof(Vertex), vertices.size()));

	const std::array<float, 2> scale = { 2.f, -2.f };
	const std::array<float, 2> offset = { -1.f, 1.f };
	return draw2dInternal(dcb, sb, restoreCxState, rgba, vertexBuffer, vertices.data(), nullptr, vertices.size(), Agc::UcPrimitiveType::Type::kRectList, fillPs, fillUserData, scale, offset, depth);
}

Agc::Toolkit::Result	drawRect(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Vectormath::Simd::Aos::Vector2_arg	position,
	Vectormath::Simd::Aos::Vector2_arg	size,
	Vectormath::Simd::Aos::Vector4_arg	rgba, 
	float    depth,
	uint32_t eighthWidth )
{
	std::array<Vertex, 8> vertices;
	vertices[0].m_position = ToVector3Unaligned(Vector3(position                                    , 0.f));
	vertices[1].m_position = ToVector3Unaligned(Vector3(position + Vector2(size.getX(), 0.f        ), 0.f));
	vertices[2].m_position = ToVector3Unaligned(Vector3(position + Vector2(size.getX(), 0.f        ), 0.f));
	vertices[3].m_position = ToVector3Unaligned(Vector3(position + Vector2(size.getX(), size.getY()), 0.f));
	vertices[4].m_position = ToVector3Unaligned(Vector3(position + Vector2(size.getX(), size.getY()), 0.f));
	vertices[5].m_position = ToVector3Unaligned(Vector3(position + Vector2(0.f        , size.getY()), 0.f));
	vertices[6].m_position = ToVector3Unaligned(Vector3(position + Vector2(0.f        , size.getY()), 0.f));
	vertices[7].m_position = ToVector3Unaligned(Vector3(position                                    , 0.f));
	Agc::Core::Buffer vertexBuffer;
	Agc::Core::initialize(&vertexBuffer, &Agc::Core::BufferSpec().initAsRegularBuffer(nullptr/* dummy data address */, sizeof(Vertex), vertices.size()));

	const std::array<float, 2> scale = { 2.f, -2.f };
	const std::array<float, 2> offset = { -1.f, 1.f };
	return draw2dInternal(dcb, sb, restoreCxState, rgba, vertexBuffer, vertices.data(), nullptr, vertices.size(), Agc::UcPrimitiveType::Type::kLineList, ::Shader::sprite_utility_fill_p, nullptr, scale, offset, depth, eighthWidth);
}

Agc::Toolkit::Result	fillOval(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Vectormath::Simd::Aos::Vector2_arg	position,
	Vectormath::Simd::Aos::Vector2_arg	size,
	Vectormath::Simd::Aos::Vector4_arg	rgba,
	float	depth,
	Agc::Shader	*fillPs,
	void	*fillUserData)
{
	const std::array<float, 2> scale = { 2.f * size.getX(), -2.f * size.getY() };
	const std::array<float, 2> offset = { position.getX() * 2.f - 1.f, -position.getY() * 2.f + 1.f };
	return draw2dInternal(dcb, sb, restoreCxState, rgba, s_ovalMeshVertexBuffer, nullptr, s_ovalFillIndices.get(), kNumberOfOvalFillIndices, Agc::UcPrimitiveType::Type::kTriList, fillPs, fillUserData, scale, offset, depth);
}

Agc::Toolkit::Result	drawOval(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Vectormath::Simd::Aos::Vector2_arg	position,
	Vectormath::Simd::Aos::Vector2_arg	size,
	Vectormath::Simd::Aos::Vector4_arg	rgba,
	float		depth,
	uint32_t	eighthWidth)
{
	const std::array<float, 2> scale = { 2.f * size.getX(), -2.f * size.getY() };
	const std::array<float, 2> offset = { position.getX() * 2.f - 1.f, -position.getY() * 2.f + 1.f };
	return draw2dInternal(dcb, sb, restoreCxState, rgba, s_ovalMeshVertexBuffer, nullptr, s_ovalDrawIndices.get(), kNumberOfOvalDrawIndices, Agc::UcPrimitiveType::Type::kLineList, ::Shader::sprite_utility_fill_p, nullptr, scale, offset, depth, eighthWidth);
}

Agc::Toolkit::Result	drawLine(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Vectormath::Simd::Aos::Vector2_arg	begin,
	Vectormath::Simd::Aos::Vector2_arg	end,
	Vectormath::Simd::Aos::Vector4_arg	rgba,
	float		depth,
	uint32_t	eighthWidth)
{
	std::array<Vertex, 2> vertices;
	vertices[0].m_position = ToVector3Unaligned(Vector3(begin, 0.f));
	vertices[1].m_position = ToVector3Unaligned(Vector3(end, 0.f));
	Agc::Core::Buffer vertexBuffer;
	Agc::Core::initialize(&vertexBuffer, &Agc::Core::BufferSpec().initAsRegularBuffer(nullptr/* dummy data address */, sizeof(Vertex), vertices.size()));

	const std::array<float, 2> scale = { 2.f, -2.f };
	const std::array<float, 2> offset = { -1.f, 1.f };
	return draw2dInternal(dcb, sb, restoreCxState, rgba, vertexBuffer, vertices.data(), nullptr, vertices.size(), Agc::UcPrimitiveType::Type::kLineList, ::Shader::sprite_utility_fill_p, nullptr, scale, offset, depth, eighthWidth);
}

Agc::Toolkit::Result	drawTexture(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Vectormath::Simd::Aos::Vector2_arg	position,
	Vectormath::Simd::Aos::Vector2_arg	size,
	Vectormath::Simd::Aos::Vector2_arg	textureOffset,
	Vectormath::Simd::Aos::Vector2_arg	sizeInTexture,
	const Agc::Core::Texture	&texture, 
	const Agc::Core::Sampler	&sampler,
	Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
	float	depth
	)
{
	struct PsRenderParameters
	{
		sce::Agc::Core::Sampler	m_sampler;
		sce::Agc::Core::Texture	m_texture;
	} userData = { sampler, texture };
	return	drawTextureInternal(dcb, sb, restoreCxState, ::Shader::sprite_utility_texture_p, nullptr, position, size, textureOffset, sizeInTexture, colorCoeff, depth, &userData, sizeof(userData));
}

Agc::Toolkit::Result	drawTextureYuy2(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Vectormath::Simd::Aos::Vector2_arg	position,
	Vectormath::Simd::Aos::Vector2_arg	size,
	Vectormath::Simd::Aos::Vector2_arg	textureOffset,
	Vectormath::Simd::Aos::Vector2_arg	sizeInTexture,
	const Agc::Core::Texture	&texture, 
	const Agc::Core::Sampler	&sampler,
	Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
	float	depth
)
{
	struct PsRenderParameters
	{
		sce::Agc::Core::Sampler	m_sampler;
		sce::Agc::Core::Texture	m_texture;
	} userData = { sampler, texture };
	return	drawTextureInternal(dcb, sb, restoreCxState, ::Shader::sprite_utility_texture_yuy2_p, nullptr, position, size, textureOffset, sizeInTexture, colorCoeff, depth, &userData, sizeof(userData));
}

Agc::Toolkit::Result	drawTextureYcbcr(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Vectormath::Simd::Aos::Vector2_arg	position,
	Vectormath::Simd::Aos::Vector2_arg	size,
	Vectormath::Simd::Aos::Vector2_arg	textureOffset,
	Vectormath::Simd::Aos::Vector2_arg	sizeInTexture,
	const Agc::Core::Texture	&yTexture,
	const Agc::Core::Sampler	&ySampler,
	const Agc::Core::Texture	&cbcrTexture,
	const Agc::Core::Sampler	&cbcrSampler,
	Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
	float	depth,
	YuvMode	yuvMode,
	uint8_t	bitDepth,
	bool isClampToLastTexel
)
{
	struct PsRenderParameters
	{
		Agc::Core::Sampler	m_ySampler;
		Agc::Core::Texture	m_yTexture;
		Agc::Core::Sampler	m_cbcrSampler;
		Agc::Core::Texture	m_cbcrTexture;
		uint32_t			m_bitDepth;
		uint32_t			m_yuvMode;
		uint32_t			m_isClampToLastTexel;
		Vector2Unaligned	m_yMinUv;
		Vector2Unaligned	m_yMaxUv;
		Vector2Unaligned	m_cbcrMinUv;
		Vector2Unaligned	m_cbcrMaxUv;
	} userData = { ySampler, yTexture, cbcrSampler, cbcrTexture, bitDepth, (uint32_t)yuvMode, isClampToLastTexel, ToVector2Unaligned(textureOffset), ToVector2Unaligned(textureOffset + sizeInTexture), userData.m_yMinUv, userData.m_yMaxUv };
	if (cbcrSampler.getTextureCoordinates() == sce::Agc::Core::Sampler::TextureCoordinates::kUnnormalized)
	{
		userData.m_cbcrMinUv.x *= 0.5f;
		userData.m_cbcrMinUv.y *= 0.5f;
		userData.m_cbcrMaxUv.x *= 0.5f;
		userData.m_cbcrMaxUv.y *= 0.5f;
	}
	return drawTextureInternal(dcb, sb, restoreCxState, ::Shader::sprite_utility_texture_ycbcr_p, nullptr, position, size, textureOffset, sizeInTexture, colorCoeff, depth, &userData, sizeof(userData));
}

Agc::Toolkit::Result	drawPoints(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	float psize,
	const Agc::Core::Texture	&texture,
	const Agc::Core::Sampler	&sampler,
	const Agc::Core::Buffer	&vertices,
	Memory::Gpu::unique_ptr<Index[]> & indices,
	uint32_t numIndices,
	Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
	float	depth
	)
{
	struct PsRenderParameters
	{
		Agc::Core::Sampler	m_sampler;
		Agc::Core::Texture	m_texture;
	} userData = { sampler, texture };

	const std::array<float, 2> scale = { 2.f, -2.f };
	const std::array<float, 2> offset = { -1.f, 1.f };
	return draw2dInternal(dcb, sb, restoreCxState, colorCoeff, vertices, nullptr, indices.get(), numIndices, Agc::UcPrimitiveType::Type::kPointList, ::Shader::sprite_utility_texture_p, nullptr, scale, offset, depth, 0.0f, psize, &userData, sizeof(userData));
}

Agc::Toolkit::Result fillCube(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Matrix4_arg model,
	Matrix4_arg view,
	Matrix4_arg projection,
	Vector3_arg lightPosition,
	Vector4_arg color,
	float ambient,
	float shininess,
	Agc::Shader	*fillPs,
	void	*fillUserData)
{
	DefaultPsRender3DParameters defaultPsRender3DParameters;
	if (fillUserData == nullptr)
	{
		setupDefaultPsRender3DParameters(defaultPsRender3DParameters, color, lightPosition, ambient, -view.getTranslation(), shininess);
		fillUserData = &defaultPsRender3DParameters;
	}

	return	draw3DInternal(dcb, sb, restoreCxState, s_cubeMesh->m_vertexBuffers[0], s_cubeMesh->m_pIndexBuffer, s_cubeMesh->m_indexSizeInBytes / 2, &model, 1, view, projection, Agc::UcPrimitiveType::Type::kTriList, fillPs, fillUserData);
}


Agc::Toolkit::Result drawCube(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Matrix4_arg model,
	Matrix4_arg view,
	Matrix4_arg projection,
	Vector4_arg color,
	uint32_t eighthWidth)
{
	DefaultPsRender3DParameters defaultPsRender3DParameters;
	setupDefaultPsRender3DParameters(defaultPsRender3DParameters, color);

	return	draw3DInternal(dcb, sb, restoreCxState, s_cubeMesh->m_vertexBuffers[0], s_cubeMesh->m_pIndexBuffer, s_cubeMesh->m_indexSizeInBytes / 2, &model, 1, view, projection, Agc::UcPrimitiveType::Type::kTriList, ::Shader::sprite_utility_fill_p, &defaultPsRender3DParameters, eighthWidth);
}

Agc::Toolkit::Result fillSphere(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Matrix4_arg model,
	Matrix4_arg view,
	Matrix4_arg projection,
	Vector3_arg lightPosition,
	Vector4_arg color,
	float ambient,
	float shininess,
	Agc::Shader	*fillPs,
	void	*fillUserData)
{
	DefaultPsRender3DParameters defaultPsRender3DParameters;
	if (fillUserData == nullptr)
	{
		setupDefaultPsRender3DParameters(defaultPsRender3DParameters, color, lightPosition, ambient, -view.getTranslation(), shininess);
		fillUserData = &defaultPsRender3DParameters;
	}

	return	draw3DInternal(dcb, sb, restoreCxState, s_sphereMesh->m_vertexBuffers[0], s_sphereMesh->m_pIndexBuffer, s_sphereMesh->m_indexSizeInBytes / 2, &model, 1, view, projection, Agc::UcPrimitiveType::Type::kTriList, fillPs, fillUserData);
}

Agc::Toolkit::Result drawSphere(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Vectormath::Simd::Aos::Matrix4_arg model,
	Vectormath::Simd::Aos::Matrix4_arg view,
	Vectormath::Simd::Aos::Matrix4_arg projection,
	Vectormath::Simd::Aos::Vector4_arg color,
	uint32_t eighthWidth)
{
	DefaultPsRender3DParameters defaultPsRender3DParameters;
	setupDefaultPsRender3DParameters(defaultPsRender3DParameters, color);

	return	draw3DInternal(dcb, sb, restoreCxState, s_sphereMesh->m_vertexBuffers[0], s_sphereMesh->m_pIndexBuffer, s_sphereMesh->m_indexSizeInBytes / 2, &model, 1, view, projection, Agc::UcPrimitiveType::Type::kTriList, ::Shader::sprite_utility_fill_p, &defaultPsRender3DParameters, eighthWidth);
}

Agc::Toolkit::Result fillCylinder(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Matrix4_arg model,
	Matrix4_arg view,
	Matrix4_arg projection,
	Vector3_arg lightPosition,
	Vector4_arg color,
	float ambient,
	float shininess,
	Agc::Shader	*fillPs,
	void	*fillUserData)
{
	DefaultPsRender3DParameters defaultPsRender3DParameters;
	if (fillUserData == nullptr)
	{
		setupDefaultPsRender3DParameters(defaultPsRender3DParameters, color, lightPosition, ambient, -view.getTranslation(), shininess);
		fillUserData = &defaultPsRender3DParameters;
	}

	return	draw3DInternal(dcb, sb, restoreCxState, s_cylinderMesh->m_vertexBuffers[0], s_cylinderMesh->m_pIndexBuffer, s_cylinderMesh->m_indexSizeInBytes / 2, &model, 1, view, projection, Agc::UcPrimitiveType::Type::kTriList, fillPs, fillUserData);
}

Agc::Toolkit::Result drawCylinder(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Matrix4_arg model,
	Matrix4_arg view,
	Matrix4_arg projection,
	Vector4_arg color,
	uint32_t eighthWidth)
{
	DefaultPsRender3DParameters defaultPsRender3DParameters;
	setupDefaultPsRender3DParameters(defaultPsRender3DParameters, color);

	return	draw3DInternal(dcb, sb, restoreCxState, s_cylinderMesh->m_vertexBuffers[0], s_cylinderMesh->m_pIndexBuffer, s_cylinderMesh->m_indexSizeInBytes / 2, &model, 1, view, projection, Agc::UcPrimitiveType::Type::kTriList, ::Shader::sprite_utility_fill_p, &defaultPsRender3DParameters, eighthWidth);
}

Agc::Toolkit::Result fillCone(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Matrix4_arg model,
	Matrix4_arg view,
	Matrix4_arg projection,
	Vector3_arg lightPosition,
	Vector4_arg color,
	float ambient,
	float shininess,
	Agc::Shader	*fillPs,
	void	*fillUserData)
{
	DefaultPsRender3DParameters defaultPsRender3DParameters;
	if (fillUserData == nullptr)
	{
		setupDefaultPsRender3DParameters(defaultPsRender3DParameters, color, lightPosition, ambient, -view.getTranslation(), shininess);
		fillUserData = &defaultPsRender3DParameters;
	}

	return	draw3DInternal(dcb, sb, restoreCxState, s_coneMesh->m_vertexBuffers[0], s_coneMesh->m_pIndexBuffer, s_coneMesh->m_indexSizeInBytes / 2, &model, 1, view, projection, Agc::UcPrimitiveType::Type::kTriList, fillPs, fillUserData);
}

Agc::Toolkit::Result drawCone(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Matrix4_arg model,
	Matrix4_arg view,
	Matrix4_arg projection,
	Vector4_arg color,
	uint32_t eighthWidth)
{
	DefaultPsRender3DParameters defaultPsRender3DParameters;
	setupDefaultPsRender3DParameters(defaultPsRender3DParameters, color);

	return	draw3DInternal(dcb, sb, restoreCxState, s_coneMesh->m_vertexBuffers[0], s_coneMesh->m_pIndexBuffer, s_coneMesh->m_indexSizeInBytes / 2, &model, 1, view, projection, Agc::UcPrimitiveType::Type::kTriList, ::Shader::sprite_utility_fill_p, &defaultPsRender3DParameters, eighthWidth);
}


Agc::Toolkit::Result	drawLine(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Matrix4_arg view,
	Matrix4_arg projection,
	Vector3_arg begin,
	Vector3_arg end,
	Vector4_arg color,
	uint32_t eighthWidth)
{
	DefaultPsRender3DParameters defaultPsRender3DParameters;
	setupDefaultPsRender3DParameters(defaultPsRender3DParameters, color);

	std::array<Vertex, 2> vertices;
	vertices[0].m_position	= begin;
	vertices[1].m_position	= end;
	Agc::Core::Buffer vertexBuffer;
	Agc::Core::initialize(&vertexBuffer, &Agc::Core::BufferSpec().initAsRegularBuffer(nullptr/* dummy data address */, sizeof(Vertex), vertices.size()));

	Matrix4 model = Matrix4::identity();
	return	draw3DInternal(dcb, sb, restoreCxState, vertexBuffer, nullptr, vertexBuffer.getNumElements(), &model, 1, view, projection, Agc::UcPrimitiveType::Type::kLineList, ::Shader::sprite_utility_fill_p, &defaultPsRender3DParameters, eighthWidth, 0.f, true, vertices.data());
}

Agc::Toolkit::Result	drawTexture(
	Agc::DrawCommandBuffer	*dcb,
	Agc::Core::StateBuffer *sb,
	Agc::Toolkit::RestoreCxState restoreCxState,
	Matrix4_arg model,
	Matrix4_arg view,
	Matrix4_arg projection,
	Vector2_arg	textureOffset,
	Vector2_arg	sizeInTexture,
	const Agc::Core::Texture	&texture,
	const Agc::Core::Sampler	&sampler,
	Vector4_arg	colorCoeff
)
{
	struct UserData
	{
		Agc::Core::Sampler	sampler;
		Agc::Core::Texture	texture;
	} userData = { sampler, texture };

	std::array<Vertex, 6> vertices;
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
	Agc::Core::Buffer vertexBuffer;
	Agc::Core::initialize(&vertexBuffer, &Agc::Core::BufferSpec().initAsRegularBuffer(nullptr/* dummy data address */, sizeof(Vertex), vertices.size()));

	DefaultPsRender3DParameters defaultPsRender3DParameters;
	setupDefaultPsRender3DParameters(defaultPsRender3DParameters, colorCoeff);

	return	draw3DInternal(dcb, sb, restoreCxState, vertexBuffer, nullptr, vertexBuffer.getNumElements(), &model, 1, view, projection, Agc::UcPrimitiveType::Type::kTriList, ::Shader::sprite_utility_texture_p, &defaultPsRender3DParameters, 0.f, 0.f, true, vertices.data(), &userData, sizeof(userData));
}

sce::Agc::Toolkit::Result	renderQuads(
	sce::Agc::DrawCommandBuffer	*dcb,
	sce::Agc::Core::StateBuffer *sb,
	sce::Agc::Toolkit::RestoreCxState restoreCxState,
	sce::Vectormath::Simd::Aos::Matrix4_arg view,
	sce::Vectormath::Simd::Aos::Matrix4_arg projection,
	const sce::Vectormath::Simd::Aos::Matrix4 * models,
	uint32_t numModels,
	const sce::Agc::Shader *psShader,
	const void *psUserData,
	bool isModelsBufferManagedByUser
)
{
	std::array<Vertex, 6> vertices;
	vertices[0].m_position = Vector3Unaligned{ -1, 1, 0 };
	vertices[1].m_position = Vector3Unaligned{ -1,-1, 0 };
	vertices[2].m_position = Vector3Unaligned{  1, 1, 0 };
	vertices[3].m_position = Vector3Unaligned{  1, 1, 0 };
	vertices[4].m_position = Vector3Unaligned{ -1,-1, 0 };
	vertices[5].m_position = Vector3Unaligned{  1,-1, 0 };
	vertices[0].m_uv = Vector2Unaligned{ 0, 0 };
	vertices[1].m_uv = Vector2Unaligned{ 0, 1 };
	vertices[2].m_uv = Vector2Unaligned{ 1, 0 };
	vertices[3].m_uv = Vector2Unaligned{ 1, 0 };
	vertices[4].m_uv = Vector2Unaligned{ 0, 1 };
	vertices[5].m_uv = Vector2Unaligned{ 1, 1 };
	Agc::Core::Buffer vertexBuffer;
	Agc::Core::initialize(&vertexBuffer, &Agc::Core::BufferSpec().initAsRegularBuffer(nullptr/* dummy data address */, sizeof(Vertex), vertices.size()));

	return draw3DInternal(dcb, sb, restoreCxState, vertexBuffer, nullptr, vertexBuffer.getNumElements(), models, numModels, view, projection, Agc::UcPrimitiveType::Type::kTriList, psShader, psUserData, 0.0f, 0.0f, !isModelsBufferManagedByUser, vertices.data());
}

} /* namespace SpriteUtil */

} /* namespace Graphics */ } /* namespace SampleUtil */ } /* namespace sce */

#endif //_SCE_TARGET_OS_PROSPERO
