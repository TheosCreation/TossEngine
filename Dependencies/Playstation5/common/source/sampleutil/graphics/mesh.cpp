/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#include <math.h>
#include <float.h>
#include <algorithm>
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/core.h>
#include "sampleutil/graphics/platform_agc/link_libraries_agc.h"
#include "sampleutil/graphics/serialize.h"
#include "sampleutil/graphics/deserialize.h"
#include "pack/agc_utils.h"
#endif
#if _SCE_TARGET_OS_ORBIS
#include <gnm/buffer.h>
#endif
#include "pack/pack_file.h"
#include <vectormath/cpp/vectormath_aos.h>
#include "sampleutil/debug/perf.h"
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/graphics/mesh.h"

using namespace sce::Vectormath::Simd::Aos;

namespace
{
	size_t	alignSize(size_t	size, size_t	align)
	{
		return ((size + align - 1ul) / align) * align;
	}

	void	initializeMesh(sce::SampleUtil::Graphics::Mesh	&mesh, const std::string	&name, sce::SampleUtil::Memory::AllocatorBase	&allocator, uint32_t vertexCount, uint32_t indexCount, bool	hasSkinning, sce::SampleUtil::Graphics::WindingOrder frontFace, sce::SampleUtil::Graphics::PrimitiveType primType)
	{
		int ret = SCE_OK; (void)ret;

		using namespace sce::SampleUtil;

		mesh.m_name				= name;
		mesh.m_frontFace		= frontFace;
#if _SCE_TARGET_OS_PROSPERO
		mesh.m_primType			= (primType == sce::SampleUtil::Graphics::PrimitiveType::kTriList) ? sce::Agc::UcPrimitiveType::Type::kTriList : sce::Agc::UcPrimitiveType::Type::kPatch;
#endif
#if _SCE_TARGET_OS_ORBIS
		mesh.m_primType			= (primType == sce::SampleUtil::Graphics::PrimitiveType::kTriList) ? sce::Gnm::kPrimitiveTypeTriList : sce::Gnm::kPrimitiveTypePatch;
#endif

		mesh.m_indexElementSizeInBytes = sizeof(sce::SampleUtil::Graphics::Index);
		const uint32_t is32bitIndex = (indexCount & sce::SampleUtil::Graphics::Mesh::IndexCountMask::kIndexSize32bit);
		if (is32bitIndex) {
			mesh.m_indexElementSizeInBytes = 4;
		}
		mesh.m_indexSizeInBytes = (indexCount & sce::SampleUtil::Graphics::Mesh::IndexCountMask::kCount) * (is32bitIndex ? 4 : 2);

		// compute gpu memory size
		size_t	gpuMemSize = 0;
		gpuMemSize += sizeof(Graphics::Vertex) * vertexCount;
		gpuMemSize = alignSize(gpuMemSize, 16);
		gpuMemSize += mesh.m_indexSizeInBytes;
		if (hasSkinning) {
			gpuMemSize = alignSize(gpuMemSize, 16);
			gpuMemSize += sizeof(Graphics::SkinVertex) * vertexCount;
		}

		// allocate aggregated gpu memory
		mesh.m_gpuMemory = Memory::Gpu::make_unique<uint8_t>(gpuMemSize, 16, allocator);
		uint8_t *bytes = mesh.m_gpuMemory.get();

		void	*pVerticesMem = bytes; bytes += sizeof(Graphics::Vertex) * vertexCount;
#if _SCE_TARGET_OS_PROSPERO
		allocator.registerResource(pVerticesMem, sizeof(Graphics::Vertex) * vertexCount, std::string("Vertices:") + name, { sce::Agc::ResourceRegistration::ResourceType::kVertexBufferBaseAddress });
#endif
#if _SCE_TARGET_OS_ORBIS
		allocator.registerResource(pVerticesMem, sizeof(Graphics::Vertex) * vertexCount, std::string("Vertices:") + name, { sce::Gnm::kResourceTypeVertexBufferBaseAddress });
#endif
		mesh.m_vertexBuffers.emplace_back();
#if _SCE_TARGET_OS_PROSPERO
		ret = sce::Agc::Core::initialize(&mesh.m_vertexBuffers.back(), &sce::Agc::Core::BufferSpec().initAsRegularBuffer(pVerticesMem, sizeof(Graphics::Vertex), vertexCount));
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
#endif
#if _SCE_TARGET_OS_ORBIS
		mesh.m_vertexBuffers.back().initAsRegularBuffer(pVerticesMem, sizeof(Graphics::Vertex), vertexCount);
#endif

		bytes = (uint8_t *)(((uintptr_t)bytes + 0xful) & ~0xful);
		mesh.m_pIndexBuffer = bytes; bytes += mesh.m_indexSizeInBytes;
#if _SCE_TARGET_OS_PROSPERO
		allocator.registerResource(mesh.m_pIndexBuffer, mesh.m_indexSizeInBytes, std::string("Indices:") + name, { sce::Agc::ResourceRegistration::ResourceType::kIndexBufferBaseAddress });
#endif
#if _SCE_TARGET_OS_ORBIS
		allocator.registerResource(mesh.m_pIndexBuffer, mesh.m_indexSizeInBytes, std::string("Indices:") + name, { sce::Gnm::kResourceTypeIndexBufferBaseAddress });
#endif

		if (hasSkinning)
		{
			bytes = (uint8_t *)(((uintptr_t)bytes + 0xful) & ~0xful);
			void *pSkinVerticesMem = bytes;
#if _SCE_TARGET_OS_PROSPERO
			allocator.registerResource(pSkinVerticesMem, sizeof(Graphics::SkinVertex) * vertexCount, std::string("SkinVertices:") + name, { sce::Agc::ResourceRegistration::ResourceType::kVertexBufferBaseAddress });
#endif
#if _SCE_TARGET_OS_ORBIS
			allocator.registerResource(pSkinVerticesMem, sizeof(Graphics::SkinVertex) * vertexCount, std::string("SkinVertices:") + name, { sce::Gnm::kResourceTypeVertexBufferBaseAddress });
#endif
			mesh.m_vertexBuffers.emplace_back();
#if _SCE_TARGET_OS_PROSPERO
			ret = sce::Agc::Core::initialize(&mesh.m_vertexBuffers.back(), &sce::Agc::Core::BufferSpec().initAsRegularBuffer(pSkinVerticesMem, sizeof(Graphics::SkinVertex), vertexCount));
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
#endif
#if _SCE_TARGET_OS_ORBIS
			mesh.m_vertexBuffers.back().initAsRegularBuffer(pSkinVerticesMem, sizeof(Graphics::SkinVertex), vertexCount);
#endif
		}
	}

	uint32_t	getChannelCount(PackFile::VertexAttribFormat	format)
	{
		using namespace PackFile;
		switch (format) {
		case e_float32:
		case e_s_int32:
			return	1;
		case e_float16_2:
		case e_float32_2:
		case e_s_int16_2:
		case e_s_int32_2:
		case e_u_int16_2:
		case e_u_int32_2:
		case e_sn_int16_2:
		case e_un_int16_2:
		case e_u_int32:
			return	2;
		case e_float32_3:
		case e_s_int32_3:
		case e_u_int32_3:
		case e_float11_11_10:
		case e_sn_int11_11_10:
		case e_un_int11_11_10:
			return	3;
		case e_float16_4:
		case e_float32_4:
		case e_s_int8_4:
		case e_s_int16_4:
		case e_s_int32_4:
		case e_u_int8_4:
		case e_u_int16_4:
		case e_u_int32_4:
		case e_sn_int8_4:
		case e_sn_int16_4:
		case e_un_int8_4:
		case e_un_int16_4:
		case e_sn_int10_10_10_2:
		case e_un_int10_10_10_2:
			return	4;
		case e_none:
			return	0;
		}
		return	0;
	}

	sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags	getAttributeFlag(std::vector<PackFile::VertexAttribute> const	&attributes)
	{
		sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags	meshFlag = {};
		for (auto &attr : attributes) {
			switch (attr.semantic) {
			default:
			case PackFile::VertexSemantic::e_vx_position:
				break;
			case PackFile::VertexSemantic::e_vx_normal:
				meshFlag.m_hasNormal	= 1;
				break;
			case PackFile::VertexSemantic::e_vx_tangent:
				meshFlag.m_hasTangent	= 1;
				break;
			case PackFile::VertexSemantic::e_vx_color:
				meshFlag.m_hasColor		= 1;
				break;
			case PackFile::VertexSemantic::e_vx_uv_channel:
				meshFlag.m_uvCount		+= 1;
				break;
			case PackFile::VertexSemantic::e_vx_bone_weights:
				meshFlag.m_hasSkinning	= (attr.format != PackFile::e_none);
				break;
			}
		}
		return	meshFlag;
	}
#if _SCE_TARGET_OS_ORBIS
	sce::Gnm::DataFormat	getGnmDataFormat(PackFile::VertexAttribFormat	packFormat)
	{
		switch (packFormat) {
		case PackFile::e_float16_2:			return	sce::Gnm::kDataFormatR16G16Float;
		case PackFile::e_float16_4:			return	sce::Gnm::kDataFormatR16G16B16A16Float;
		case PackFile::e_float32:			return	sce::Gnm::kDataFormatR32Float;
		case PackFile::e_float32_2:			return	sce::Gnm::kDataFormatR32G32Float;
		case PackFile::e_float32_3:			return	sce::Gnm::kDataFormatR32G32B32Float;
		case PackFile::e_float32_4:			return	sce::Gnm::kDataFormatR32G32B32A32Float;
		case PackFile::e_s_int8_4:			return	sce::Gnm::kDataFormatR8G8B8A8Sint;
		case PackFile::e_s_int16_2:			return	sce::Gnm::kDataFormatR16G16Sint;
		case PackFile::e_s_int16_4:			return	sce::Gnm::kDataFormatR16G16B16A16Sint;
		case PackFile::e_s_int32:			return	sce::Gnm::kDataFormatR32Sint;
		case PackFile::e_s_int32_2:			return	sce::Gnm::kDataFormatR32G32Sint;
		case PackFile::e_s_int32_3:			return	sce::Gnm::kDataFormatR32G32B32Sint;
		case PackFile::e_s_int32_4:			return	sce::Gnm::kDataFormatR32G32B32A32Sint;
		case PackFile::e_u_int8_4:			return	sce::Gnm::kDataFormatR8G8B8A8Uint;
		case PackFile::e_u_int16_2:			return	sce::Gnm::kDataFormatR16G16Uint;
		case PackFile::e_u_int16_4:			return	sce::Gnm::kDataFormatR16G16B16A16Uint;
		case PackFile::e_u_int32:			return	sce::Gnm::kDataFormatR32Uint;
		case PackFile::e_u_int32_2:			return	sce::Gnm::kDataFormatR32G32Uint;
		case PackFile::e_u_int32_3:			return	sce::Gnm::kDataFormatR32G32B32Uint;
		case PackFile::e_u_int32_4:			return	sce::Gnm::kDataFormatR32G32B32A32Uint;
		case PackFile::e_sn_int8_4:			return	sce::Gnm::kDataFormatR8G8B8A8Snorm;
		case PackFile::e_sn_int16_2:		return	sce::Gnm::kDataFormatR16G16Snorm;
		case PackFile::e_sn_int16_4:		return	sce::Gnm::kDataFormatR16G16B16A16Snorm;
		case PackFile::e_un_int8_4:			return	sce::Gnm::kDataFormatR8G8B8A8Unorm;
		case PackFile::e_un_int16_2:		return	sce::Gnm::kDataFormatR16G16Unorm;
		case PackFile::e_un_int16_4:		return	sce::Gnm::kDataFormatR16G16B16A16Unorm;
		case PackFile::e_float11_11_10:		return	sce::Gnm::kDataFormatR10G11B11Float;
		case PackFile::e_sn_int11_11_10:	return	{ {{sce::Gnm::kSurfaceFormat10_11_11, sce::Gnm::kTextureChannelTypeSNorm, sce::Gnm::kTextureChannelX,  sce::Gnm::kTextureChannelY,  sce::Gnm::kTextureChannelZ,  sce::Gnm::kTextureChannelConstant1, 0 }} };
		case PackFile::e_un_int11_11_10:	return	{ {{sce::Gnm::kSurfaceFormat10_11_11, sce::Gnm::kTextureChannelTypeUNorm, sce::Gnm::kTextureChannelX,  sce::Gnm::kTextureChannelY,  sce::Gnm::kTextureChannelZ,  sce::Gnm::kTextureChannelConstant1, 0 }} };
		case PackFile::e_sn_int10_10_10_2:	return	{ {{sce::Gnm::kSurfaceFormat2_10_10_10, sce::Gnm::kTextureChannelTypeSNorm, sce::Gnm::kTextureChannelX,  sce::Gnm::kTextureChannelY,  sce::Gnm::kTextureChannelZ,  sce::Gnm::kTextureChannelW, 0 }} };
		case PackFile::e_un_int10_10_10_2:	return	{ {{sce::Gnm::kSurfaceFormat2_10_10_10, sce::Gnm::kTextureChannelTypeUNorm, sce::Gnm::kTextureChannelX,  sce::Gnm::kTextureChannelY,  sce::Gnm::kTextureChannelZ,  sce::Gnm::kTextureChannelW, 0 }} };
		case PackFile::e_none:				return	sce::Gnm::kDataFormatInvalid;
		}

		return	sce::Gnm::kDataFormatInvalid;
	}

	sce::Gnm::PrimitiveType	getGnmPrimitiveType(PackFile::PrimitiveType	packPrimitiveType)
	{
		switch (packPrimitiveType) {
		case PackFile::PrimitiveType::e_none:					return	sce::Gnm::kPrimitiveTypeNone;
		case PackFile::PrimitiveType::e_point_list:				return	sce::Gnm::kPrimitiveTypePointList;
		case PackFile::PrimitiveType::e_line_list:				return	sce::Gnm::kPrimitiveTypeLineList;
		case PackFile::PrimitiveType::e_line_strip:				return	sce::Gnm::kPrimitiveTypeLineStrip;
		case PackFile::PrimitiveType::e_tri_list:				return	sce::Gnm::kPrimitiveTypeTriList;
		case PackFile::PrimitiveType::e_tri_fan:				return	sce::Gnm::kPrimitiveTypeTriFan;
		case PackFile::PrimitiveType::e_tri_strip:				return	sce::Gnm::kPrimitiveTypeTriStrip;
		case PackFile::PrimitiveType::e_patch:					return	sce::Gnm::kPrimitiveTypePatch;
		case PackFile::PrimitiveType::e_line_list_adjacency:	return	sce::Gnm::kPrimitiveTypeLineListAdjacency;
		case PackFile::PrimitiveType::e_line_strip_adjacency:	return	sce::Gnm::kPrimitiveTypeLineStripAdjacency;
		case PackFile::PrimitiveType::e_tri_list_adjacency:		return	sce::Gnm::kPrimitiveTypeTriListAdjacency;
		case PackFile::PrimitiveType::e_tri_strip_adjacency:	return	sce::Gnm::kPrimitiveTypeTriStripAdjacency;
		case PackFile::PrimitiveType::e_rect_list:				return	sce::Gnm::kPrimitiveTypeRectList;
		case PackFile::PrimitiveType::e_line_loop:				return	sce::Gnm::kPrimitiveTypeLineLoop;
		case PackFile::PrimitiveType::e_polygon:				return	sce::Gnm::kPrimitiveTypePolygon;
		}
		return	sce::Gnm::kPrimitiveTypeNone;
	}
#endif
#if _SCE_TARGET_OS_PROSPERO
	void	packVertexAttrs2hash(uint64_t	*pOutHash, const std::vector<PackFile::VertexAttribute>	&attrs)
	{
		pOutHash[0] = pOutHash[1] = pOutHash[2] = pOutHash[3] = 0ul;
		for (int i = 0; i < attrs.size(); i++) {
			const PackFile::VertexAttribute	&attr = attrs[i];
			uint64_t hashElem = 0ul;
			hashElem |= attr.format; // 8bits
			hashElem |= attr.semantic << 8; // 3bits
			hashElem |= attr.vertex_buffer_index << 11; // 3bits
			hashElem |= attr.offset << 14; // 12bits
			const int sa = i * 26;
			pOutHash[sa / 64] |= (sa % 64 > 0) ? (hashElem & ((1ul << (64 - sa % 64))- 1ul)) << (sa % 64) : hashElem;
			pOutHash[sa / 64 + 1] |= (sa % 64 > 0) ? (hashElem >> (64 - sa % 64)) : 0ul;
		}
	}
#endif
} // anonymous namespace

namespace sce { namespace SampleUtil { namespace Graphics {
Mesh::Mesh(const std::string	&name, VideoAllocator &videoMemory, uint32_t vertexCount, uint32_t indexCount, bool	hasSkinning, WindingOrder	frontFace, PrimitiveType	primType)
{
	TAG_THIS_CLASS;
	initializeMesh(*this, name, videoMemory, vertexCount, indexCount, hasSkinning, frontFace, primType);
}

Mesh::Mesh(PackFile::Mesh const	&params, PackFile::Buffer const	*pBuffers, void	*pBufferMemory, VideoAllocator	&allocator)
{
	int ret = SCE_OK; (void)ret;

	TAG_THIS_CLASS;

	SCE_SAMPLE_UTIL_ASSERT(pBufferMemory != nullptr);
	SCE_SAMPLE_UTIL_ASSERT(pBuffers != nullptr);

	if (pBufferMemory == nullptr || pBuffers == nullptr) {
		return;
	}
	// The Vertex shader expect the attributes in this order
	//  position normal tangent color uv0 uv1 bone_idx weight
	// that "coincidentally" is the same as the samantic enum value.
	std::vector<PackFile::VertexAttribute>	sortedAttributes(params.attributes.begin(), params.attributes.end());
	auto	attribteSorter = [](PackFile::VertexAttribute	&a, PackFile::VertexAttribute	&b)
	{
		if (a.semantic == b.semantic) return	a.index < b.index;
		return	a.semantic < b.semantic;
	};
	std::sort(sortedAttributes.begin(), sortedAttributes.end(), attribteSorter);

	m_vertexAttributeFlags = getAttributeFlag(sortedAttributes);
	if (m_vertexAttributeFlags.m_hasSkinning) {
		for (auto &attr : sortedAttributes) {
			if (attr.semantic == PackFile::VertexSemantic::e_vx_bone_weights) {
				m_bonesPerVertex = getChannelCount(attr.format);
				break;
			}
		}
	}
#if _SCE_TARGET_OS_PROSPERO
	for (auto	packAttr : sortedAttributes) {
		sce::Agc::Core::VertexAttribute	attribute = {
			packAttr.vertex_buffer_index,
			PackFile::to_agc_vertex_format(packAttr.format),
			packAttr.offset,
			sce::Agc::Core::VertexAttribute::Index::kVertexId
		};
		m_vertexAttributes.push_back(attribute);
	}
	packVertexAttrs2hash(m_vertexAttrsHash, sortedAttributes);
#endif

	uint8_t	*pMem = static_cast<uint8_t *>(pBufferMemory);
#if _SCE_TARGET_OS_PROSPERO
	m_vertexBuffers.resize(params.vertex_buffers.getCount());
	for (int i = 0; i < params.vertex_buffers.getCount(); ++i) {
		auto	&buffer = pBuffers[params.vertex_buffers[i]];
		allocator.registerResource(pMem + buffer.offset, buffer.size, std::string("Vertices:") + std::string(params.name) + "#" + std::to_string(i), { sce::Agc::ResourceRegistration::ResourceType::kVertexBufferBaseAddress });
		sce::Agc::Core::BufferSpec	vbufSpec;
		vbufSpec.initAsRegularBuffer(pMem + buffer.offset, buffer.stride, buffer.elem_count);
		ret = sce::Agc::Core::initialize(&m_vertexBuffers[i], &vbufSpec);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			return;
		}
	}
#endif
#if _SCE_TARGET_OS_ORBIS
	for (int i = 0; i < params.vertex_buffers.size(); ++i) {
		const auto	&buffer	= pBuffers[params.vertex_buffers[i]];
		allocator.registerResource(pMem + buffer.offset, buffer.size, std::string("Vertices:") + std::string(params.name) + "#" + std::to_string(i), { sce::Gnm::kResourceTypeVertexBufferBaseAddress });
	}
	m_vertexBuffers.resize(sortedAttributes.size());
	for (int i = 0; i < sortedAttributes.size(); i++) {
		const auto	&attr	= sortedAttributes[i];
		const auto	&buffer	= pBuffers[params.vertex_buffers[attr.vertex_buffer_index]];
		m_vertexBuffers[i].initAsVertexBuffer(pMem + buffer.offset + attr.offset, getGnmDataFormat(attr.format), buffer.stride, buffer.elem_count);
	}
#endif

	const auto	&indexBuffer	= pBuffers[params.index_buffer];
	m_pIndexBuffer		= pMem + indexBuffer.offset;
#if _SCE_TARGET_OS_PROSPERO
	allocator.registerResource(m_pIndexBuffer, indexBuffer.size, std::string("Indices") + std::string(params.name), { sce::Agc::ResourceRegistration::ResourceType::kIndexBufferBaseAddress });
	m_primType			= PackFile::to_agc_primitive_type(params.primitive_type);
#endif
#if _SCE_TARGET_OS_ORBIS
	allocator.registerResource(m_pIndexBuffer, indexBuffer.size, std::string("Indices") + std::string(params.name), { sce::Gnm::kResourceTypeIndexBufferBaseAddress });
	m_primType			= getGnmPrimitiveType(params.primitive_type);
#endif
	m_indexElementSizeInBytes	= params.index_elem_size / 8;
	m_indexSizeInBytes			= params.index_count * m_indexElementSizeInBytes;

	sce::Geometry::Aos::Bounds bounds(sce::Geometry::Math::Aos::Point3(params.bounding_box.min_corner),
										sce::Geometry::Math::Aos::Point3(params.bounding_box.max_corner));

	m_bounds	= bounds;
}

Mesh::Mesh(PackFile::Mesh const	&params, PackFile::Buffer const	*pBuffers, PackFile::Array<PackFile::DataMapping> const	&dataMapping, VideoAllocator	&allocator)
{
	int ret = SCE_OK; (void)ret;

	TAG_THIS_CLASS;

	SCE_SAMPLE_UTIL_ASSERT(pBuffers != nullptr);

	if (pBuffers == nullptr) {
		return;
	}
	// The Vertex shader expect the attributes in this order
	//  position normal tangent color uv0 uv1 bone_idx weight
	// that "coincidentally" is the same as the samantic enum value.
	std::vector<PackFile::VertexAttribute>	sortedAttributes(params.attributes.begin(), params.attributes.end());
	auto	attribteSorter	= [](PackFile::VertexAttribute& a, PackFile::VertexAttribute& b)
	{
		if (a.semantic == b.semantic) return	a.index < b.index;
		return	a.semantic < b.semantic;
	};
	std::sort(sortedAttributes.begin(), sortedAttributes.end(), attribteSorter);

	m_vertexAttributeFlags	= getAttributeFlag(sortedAttributes);
	if (m_vertexAttributeFlags.m_hasSkinning) {
		for (auto& attr : sortedAttributes) {
			if (attr.semantic == PackFile::VertexSemantic::e_vx_bone_weights) {
				m_bonesPerVertex	= getChannelCount(attr.format);
				break;
			}
		}
	}
#if _SCE_TARGET_OS_PROSPERO
	for (auto packAttr : sortedAttributes) {
		sce::Agc::Core::VertexAttribute	attribute = {
			packAttr.vertex_buffer_index,
			PackFile::to_agc_vertex_format(packAttr.format),
			packAttr.offset,
			sce::Agc::Core::VertexAttribute::Index::kVertexId
		};
		m_vertexAttributes.push_back(attribute);
	}
	packVertexAttrs2hash(m_vertexAttrsHash, sortedAttributes);
#endif

	m_vertexBuffers.resize(params.vertex_buffers.getCount());
	for (int i = 0; i < params.vertex_buffers.getCount(); ++i) {
		auto	&buffer = pBuffers[params.vertex_buffers[i]];
		PackFile::Serialization::DataOffset	bufferDataOffset
		{
			.offset			= buffer.offset,
			.size			= (uint32_t)buffer.size,
			.mapping_idx	= buffer.data_section,
		};
		const void	*ptr = PackFile::Serialization::get_ptr(dataMapping, bufferDataOffset);
#if _SCE_TARGET_OS_PROSPERO
		allocator.registerResource(ptr, buffer.size, std::string("Vertices:") + std::string(params.name) + "#" + std::to_string(i), { sce::Agc::ResourceRegistration::ResourceType::kVertexBufferBaseAddress });
		sce::Agc::Core::BufferSpec	vbufSpec;
		vbufSpec.initAsRegularBuffer(ptr, buffer.stride, buffer.elem_count);
		ret = sce::Agc::Core::initialize(&m_vertexBuffers[i], &vbufSpec);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			return;
		}
#endif
#if _SCE_TARGET_OS_ORBIS
		allocator.registerResource(ptr, buffer.size, std::string("Vertices:") + std::string(params.name) + "#" + std::to_string(i), { sce::Gnm::kResourceTypeVertexBufferBaseAddress });
#endif
	}
#if _SCE_TARGET_OS_ORBIS
	for (int i = 0; i < sortedAttributes.size(); i++) {
		const auto	&attr	= sortedAttributes[i];
		const auto	&buffer	= pBuffers[params.vertex_buffers[attr.vertex_buffer_index]];
		PackFile::Serialization::DataOffset	bufferDataOffset
		{
			.offset = buffer.offset,
			.size = (uint32_t)buffer.size,
			.mapping_idx = buffer.data_section,
		};
		auto	ptr = (uint8_t *)PackFile::Serialization::get_ptr(dataMapping, bufferDataOffset);
		m_vertexBuffers[i].initAsVertexBuffer(ptr + attr.offset, getGnmDataFormat(attr.format), buffer.stride, buffer.elem_count);
	}
#endif

	const auto	&indexBuffer	= pBuffers[params.index_buffer];
	PackFile::Serialization::DataOffset indexDataOffset
	{
		.offset			= indexBuffer.offset,
		.size			= (uint32_t)indexBuffer.size,
		.mapping_idx	= indexBuffer.data_section,
	};
	m_pIndexBuffer				= PackFile::Serialization::get_ptr(dataMapping, indexDataOffset);
#if _SCE_TARGET_OS_PROSPERO
	allocator.registerResource(m_pIndexBuffer, indexBuffer.size, std::string("Indices") + std::string(params.name), { sce::Agc::ResourceRegistration::ResourceType::kIndexBufferBaseAddress });
	m_primType					= PackFile::to_agc_primitive_type(params.primitive_type);
#endif
#if _SCE_TARGET_OS_ORBIS
	allocator.registerResource(m_pIndexBuffer, indexBuffer.size, std::string("Indices") + std::string(params.name), { sce::Gnm::kResourceTypeIndexBufferBaseAddress });
	m_primType					= getGnmPrimitiveType(params.primitive_type);
#endif
	m_indexElementSizeInBytes	= params.index_elem_size / 8;
	m_indexSizeInBytes			= params.index_count * m_indexElementSizeInBytes;

	sce::Geometry::Aos::Bounds	bounds(sce::Geometry::Math::Aos::Point3(params.bounding_box.min_corner),
										sce::Geometry::Math::Aos::Point3(params.bounding_box.max_corner));

	m_bounds	= bounds;
}


Mesh::~Mesh()
{
	if (m_gpuMemory.get() != nullptr) {
		auto	deleter		= m_gpuMemory.get_deleter();
		auto	allocator	= reinterpret_cast<VideoAllocator *>(deleter.m_alloc);
		for (auto &vertexBuffer : m_vertexBuffers) {
#if _SCE_TARGET_OS_PROSPERO
			allocator->unregisterResource(vertexBuffer.getDataAddress());
#endif
#if _SCE_TARGET_OS_ORBIS
			allocator->unregisterResource(vertexBuffer.getBaseAddress());
#endif
		}
		allocator->unregisterResource(m_pIndexBuffer);
	}
	UNTAG_THIS_CLASS;
}

SphereMesh::SphereMesh(VideoAllocator &videoMemory, float radius, long xdiv, long ydiv)
	: Mesh("SphereMesh", videoMemory, (xdiv + 1) * (ydiv + 1), (xdiv * (ydiv - 1) * 2) * 3)
{
	TAG_THIS_DERIVED_CLASS;
	m_bounds.setMinimum(Point3(-radius)).setMaximum(Point3(radius));
#if _SCE_TARGET_OS_PROSPERO
	auto outV = reinterpret_cast<Vertex *>(m_vertexBuffers[0].getDataAddress());
#endif
#if _SCE_TARGET_OS_ORBIS
	auto outV = reinterpret_cast<Vertex *>(m_vertexBuffers[0].getBaseAddress());
#endif
	auto outI = reinterpret_cast<sce::SampleUtil::Graphics::Index *>(m_pIndexBuffer);

	const float gx = 2.f * M_PI / xdiv;
	const float gy = M_PI / ydiv;

	for (long i = 0; i < xdiv; ++i)
	{
		const float theta = (float)i * gx;
		const float ct = cosf(theta);
		const float st = sinf(theta);

		const long k = i * (ydiv + 1);
		for (long j = 1; j < ydiv; ++j)
		{
			const float phi = (float) j * gy;
			const float sp = sinf(phi);
			const float x = ct * sp;
			const float y = st * sp;
			const float z = cosf(phi);

			outV[k + j].m_position	= Vector3( x * radius, y * radius, z * radius );
			outV[k + j].m_normal	= Vector3( x, y, z );
			outV[k + j].m_uv		= Vector2( theta * 0.1591549430918953f, phi * 0.31830988618379f );
		}
	}

	const long kk = xdiv * (ydiv + 1);
	for (long j = 1; j < ydiv; ++j)
	{
		const float phi = (float)j * gy;
		const float x = sinf(phi);
		const float z = cosf(phi);

		outV[kk + j].m_position	= Vector3( x * radius, 0.f, z * radius );
		outV[kk + j].m_normal	= Vector3( x, 0.f, z );
		outV[kk + j].m_uv		= Vector2( 1.f, phi * 0.31830988618379f );
	}

	for (long i = 0; i < xdiv; i++)
	{
		const long k1 = i * (ydiv + 1) + 1;
		const long k2 = (i + 1) * (ydiv + 1) + 1;
		const float s = (outV[k1].m_uv.x + outV[k2].m_uv.x) * 0.5f;

		outV[k1        - 1].m_position	= Vector3( 0.f, 0.f, radius );
		outV[k1        - 1].m_normal	= Vector3::zAxis();
		outV[k1        - 1].m_uv		= Vector2( s, 0.f );

		outV[k1 + ydiv - 1].m_position	= Vector3( 0.f, 0.f, -radius );
		outV[k1 + ydiv - 1].m_normal	= -Vector3::zAxis();
		outV[k1 + ydiv - 1].m_uv		= Vector2( s, 1.f );
	}

	outV[xdiv*(ydiv+1)].m_position		= outV[0].m_position;
	outV[xdiv*(ydiv+1)].m_normal		= outV[0].m_normal;
	outV[xdiv*(ydiv+1)].m_uv			= outV[0].m_uv;

	outV[xdiv*(ydiv+1)+ydiv].m_position	= outV[ydiv].m_position;
	outV[xdiv*(ydiv+1)+ydiv].m_normal	= outV[ydiv].m_normal;
	outV[xdiv*(ydiv+1)+ydiv].m_uv		= outV[ydiv].m_uv;

	long ii = 0;
	for(long i = 0; i < xdiv; ++i)
	{
		const long k = i * (ydiv + 1);

		outI[ii+0] = (uint16_t) k;
		outI[ii+1] = (uint16_t) (k + 1);
		outI[ii+2] = (uint16_t) (k + ydiv + 2);
		ii += 3;

		for(long j = 1; j < ydiv - 1; ++j)
		{
			outI[ii+0] = (uint16_t) (k + j);
			outI[ii+1] = (uint16_t) (k + j + 1);
			outI[ii+2] = (uint16_t) (k + j + ydiv + 2);
			outI[ii+3] = (uint16_t) (k + j);
			outI[ii+4] = (uint16_t) (k + j + ydiv + 2);
			outI[ii+5] = (uint16_t) (k + j + ydiv + 1);
			ii += 6;
		}

		outI[ii+0] = (uint16_t) (k + ydiv - 1);
		outI[ii+1] = (uint16_t) (k + ydiv);
		outI[ii+2] = (uint16_t) (k + ydiv * 2);
		ii += 3;
	}

	// Double texcoords
	for(uint32_t i = 0; i < (xdiv + 1) * (ydiv + 1); ++i)
	{
		outV[i].m_uv = Vector2(outV[i].m_uv.x * 4.f, outV[i].m_uv.y * 2.f);
	}

	// Calculate tangents
	Vector3	*tan1 = new Vector3[(xdiv + 1) * (ydiv + 1)];
	Vector3	*tan2 = new Vector3[(xdiv + 1) * (ydiv + 1)];

	memset(tan1, 0, sizeof(Vector3) * (xdiv + 1) * (ydiv + 1));
	memset(tan2, 0, sizeof(Vector3) * (xdiv + 1) * (ydiv + 1));

	for(uint32_t i = 0; i < m_indexSizeInBytes / 6; ++i)
	{
		const long i1 = outI[i*3+0];
		const long i2 = outI[i*3+1];
		const long i3 = outI[i*3+2];
		const Vector3 v1(outV[i1].m_position);
		const Vector3 v2(outV[i2].m_position);
		const Vector3 v3(outV[i3].m_position);
		const Vector2 w1(outV[i1].m_uv);
		const Vector2 w2(outV[i2].m_uv);
		const Vector2 w3(outV[i3].m_uv);

		const float x1 = v2[0] - v1[0];
		const float x2 = v3[0] - v1[0];
		const float y1 = v2[1] - v1[1];
		const float y2 = v3[1] - v1[1];
		const float z1 = v2[2] - v1[2];
		const float z2 = v3[2] - v1[2];

		const float s1 = w2[0] - w1[0];
		const float s2 = w3[0] - w1[0];
		const float t1 = w2[1] - w1[1];
		const float t2 = w3[1] - w1[1];

		const float r = 1.f / (s1*t2-s2*t1);
		const Vector3 sdir( (t2*x1-t1*x2)*r, (t2*y1-t1*y2)*r, (t2*z1-t1*z2)*r );
		const Vector3 tdir( (s1*x2-s2*x1)*r, (s1*y2-s2*y1)*r, (s1*z2-s2*z1)*r );

		tan1[i1] = tan1[i1] + sdir;
		tan1[i2] = tan1[i2] + sdir;
		tan1[i3] = tan1[i3] + sdir;
		tan2[i1] = tan2[i1] + tdir;
		tan2[i2] = tan2[i2] + tdir;
		tan2[i3] = tan2[i3] + tdir;
	}
	const long count = (xdiv + 1) * (ydiv + 1);
	for(long i = 0; i < count; ++i)
	{
		const Vector3 n(outV[i].m_normal);
		const Vector3 t = tan1[i];
		const float nDotT = dot(n, t);
		const Vector3 tan_a = t - n * nDotT;
		if (length(tan_a) > 0.f)
		{
			const float ooLen = 1.f / length(tan_a);
			outV[i].m_tangent = Vector4(tan_a[0] * ooLen, tan_a[1] * ooLen, tan_a[2] * ooLen, 1.f);
		}
	}

	delete[] tan1;
	delete[] tan2;
}

SphereMesh::~SphereMesh()
{
	UNTAG_THIS_DERIVED_CLASS;
}

CylinderMesh::CylinderMesh(VideoAllocator &videoMemory, float radius, float height, unsigned div)
	: Mesh("CylinderMesh", videoMemory, ((div + 1) * 2) + (div * 4), ((div * 2) + (div * 2)) * 3)
{
	// Total Triangles	= ( div * 2 ) + div * 2
	// Total Vertices	= ( ( div + 1 ) * 2 ) + ( div * 4 )
	// Total Indices	= ( ( div * 2 ) + ( div * 2 ) ) * 3
	TAG_THIS_DERIVED_CLASS;
	SCE_SAMPLE_UTIL_ASSERT( 3 <= div );

	m_bounds.setMinimum(Point3(-height, -radius, -radius)).setMaximum(Point3(height, radius, radius));
#if _SCE_TARGET_OS_PROSPERO
	auto outVertices	= reinterpret_cast<Vertex *>(m_vertexBuffers[0].getDataAddress());
#endif
#if _SCE_TARGET_OS_ORBIS
	auto outVertices	= reinterpret_cast<Vertex *>(m_vertexBuffers[0].getBaseAddress());
#endif
	auto outIndices		= reinterpret_cast<sce::SampleUtil::Graphics::Index *>(m_pIndexBuffer);

	const float dividedRadians = ( 360.0f / div ) * 0.017453293f;

	std::vector< Vector3 > circles[ 2 ];

	// generate vertices: left circle
	for ( int iii = 0 ; iii < div ; iii++ )
	{
		const float x = -height;
		const float y = std::sinf( dividedRadians * iii ) * radius;
		const float z = std::cosf( dividedRadians * iii ) * radius;

		circles[ 0 ].push_back( Vector3( x, y, z ) );
	}
	// generate vertices: right circle
	std::for_each ( circles[ 0 ].begin( ), circles[ 0 ].end( ), [ &circles ] ( const Vector3 & vertexLeft )
						{
							const Vector3 vertexRight = -vertexLeft;
							circles[ 1 ].push_back( vertexRight );
						}
				  );

	//
	// generate meshes
	//
	int vertexNumber	= 0;
	int indexNumber		= 0;

	// generate meshes: left circle
	//
	// center vertex
	outVertices[ vertexNumber ].m_position	= Vector3( -height, 0.0f, 0.0f );
	outVertices[ vertexNumber ].m_normal	= Vector3( -1.0f, 0.0f, 0.0f );
	const int leftCircleCenterVertexIndex		= vertexNumber;
	vertexNumber++;

	const int lefetCircleStartVertexNumber		= vertexNumber;
	std::for_each ( circles[ 0 ].begin( ), circles[ 0 ].end( ), [ & ] ( const Vector3 & vertexLeft )
	{
		outVertices[ vertexNumber ].m_position	= vertexLeft;
		outVertices[ vertexNumber ].m_normal	= Vector3( -1.0f, 0.0f, 0.0f );
		vertexNumber++;
	} );

	for ( int iii = 0 ; iii < div ; iii++ )
	{
		for ( int jjj = 0 ; jjj < 2 ; jjj++ )
		{
			const int vp = ( ( iii + jjj ) >= circles[ 0 ].size( ) ) ? ( iii + jjj ) % circles[ 0 ].size( ) : ( iii + jjj );
			outIndices[ indexNumber++ ] = lefetCircleStartVertexNumber + vp;
		}
		outIndices[ indexNumber++ ] = leftCircleCenterVertexIndex;
	}
	// generate meshes: right circle
	//
	// center vertex
	outVertices[ vertexNumber ].m_position	= Vector3( height, 0.0f, 0.0f );
	outVertices[ vertexNumber ].m_normal	= Vector3( 1.0f, 0.0f, 0.0f );
	const int rightCircleCenterVertexIndex	= vertexNumber;
	vertexNumber++;

	const int rightCircleStartVertexNumber	= vertexNumber;
	std::for_each ( circles[ 1 ].begin( ), circles[ 1 ].end( ), [ & ] ( const Vector3 & vertexRight )
	{
		outVertices[ vertexNumber ].m_position	= vertexRight;
		outVertices[ vertexNumber ].m_normal	= Vector3( 1.0f, 0.0f, 0.0f );
		vertexNumber++;
	} );

	for ( int iii = 0 ; iii < div ; iii++ )
	{
		for ( int jjj = 1 ; jjj >= 0 ; jjj-- )
		{
			const int vp = ( ( iii + jjj ) >= circles[ 1 ].size( ) ) ? ( iii + jjj ) % circles[ 1 ].size( ) : ( iii + jjj );
			outIndices[ indexNumber++ ] = rightCircleStartVertexNumber + vp;
		}
		outIndices[ indexNumber++ ] = rightCircleCenterVertexIndex;
	}

	// generate meshes: side faces
	const std::vector< Vector3 > & circle = circles[ 0 ];
	for ( int iii = 0 ; iii < circle.size( ) ; iii++ )
	{
		const int vid_0 = ( 0 + iii ) % circle.size( );
		const int vid_1 = ( 1 + iii ) % circle.size( );

		outVertices[ vertexNumber + 0 ].m_position = Vector3(  circle[ vid_0 ].getX( ), circle[ vid_0 ].getY( ), circle[ vid_0 ].getZ( ) );
		outVertices[ vertexNumber + 1 ].m_position = Vector3( -circle[ vid_0 ].getX( ), circle[ vid_0 ].getY( ), circle[ vid_0 ].getZ( ) );
		outVertices[ vertexNumber + 2 ].m_position = Vector3( -circle[ vid_1 ].getX( ), circle[ vid_1 ].getY( ), circle[ vid_1 ].getZ( ) );
		outVertices[ vertexNumber + 3 ].m_position = Vector3(  circle[ vid_1 ].getX( ), circle[ vid_1 ].getY( ), circle[ vid_1 ].getZ( ) );


		const Vector3 v0 = Vector3( outVertices[ vertexNumber + 0 ].m_position.x, outVertices[ vertexNumber + 0 ].m_position.y, outVertices[ vertexNumber + 0 ].m_position.z );
		const Vector3 v1 = Vector3( outVertices[ vertexNumber + 1 ].m_position.x, outVertices[ vertexNumber + 1 ].m_position.y, outVertices[ vertexNumber + 1 ].m_position.z );
		const Vector3 v3 = Vector3( outVertices[ vertexNumber + 3 ].m_position.x, outVertices[ vertexNumber + 3 ].m_position.y, outVertices[ vertexNumber + 3 ].m_position.z );

		const Vector3 normal = Vectormath::Simd::Aos::normalize
									(
										Vectormath::Simd::Aos::cross(  v1 - v0, v3 - v0 )
									);

		outVertices[ vertexNumber + 0 ].m_normal = normal;
		outVertices[ vertexNumber + 1 ].m_normal = normal;
		outVertices[ vertexNumber + 2 ].m_normal = normal;
		outVertices[ vertexNumber + 3 ].m_normal = normal;

		outIndices[ indexNumber++ ] = vertexNumber + 0;
		outIndices[ indexNumber++ ] = vertexNumber + 1;
		outIndices[ indexNumber++ ] = vertexNumber + 2;
		outIndices[ indexNumber++ ] = vertexNumber + 2;
		outIndices[ indexNumber++ ] = vertexNumber + 3;
		outIndices[ indexNumber++ ] = vertexNumber + 0;

		vertexNumber += 4;
	}
}

CylinderMesh::~CylinderMesh( )
{
	UNTAG_THIS_DERIVED_CLASS;
}

ConeMesh::ConeMesh(VideoAllocator &videoMemory, float radius, float height, unsigned div)
	: Mesh("ConeMesh", videoMemory, div * 3 + 1, 6 * div)
{
	TAG_THIS_DERIVED_CLASS;
	m_bounds.setMinimum(Point3(-radius, -radius, -height)).setMaximum(Point3(radius, radius, 0.f));
#if _SCE_TARGET_OS_PROSPERO
	auto outV = reinterpret_cast<Vertex *>(m_vertexBuffers[0].getDataAddress());
#endif
#if _SCE_TARGET_OS_ORBIS
	auto outV = reinterpret_cast<Vertex *>(m_vertexBuffers[0].getBaseAddress());
#endif
	auto outI = reinterpret_cast<sce::SampleUtil::Graphics::Index *>(m_pIndexBuffer);

	const float g = 2.f * M_PI / div;
	Vector3 centre = -height * Vector3::zAxis();
	Vector3 apex = Vector3::zero();
	uint32_t ov = 0;
	for (uint32_t i = 0; i < div; ++i)
	{
		memset(&outV[ov], 0, sizeof(outV[ov]));
		const float theta = (float)i * g;
		outV[ov].m_position.x = cosf(theta) * radius;
		outV[ov].m_position.y = sinf(theta) * radius;
		outV[ov].m_position.z = -height;
		outV[ov].m_normal = -Vector3::zAxis();
		++ov;
	}
	for (uint32_t i = 0; i < div; ++i)
	{
		memset(&outV[ov], 0, sizeof(outV[ov]));
		outV[ov].m_position = outV[ov -div].m_position;
		Vector3 pos(outV[ov].m_position);
		storeXYZ(normalize(cross(apex - pos, cross(pos - centre, apex - centre))), &outV[ov].m_normal.x);
		++ov;
	}
	for (uint32_t i = 0; i < div; ++i)
	{
		memset(&outV[ov], 0, sizeof(outV[ov]));
		storeXYZ(apex, &outV[ov].m_position.x);
		storeXYZ(normalize(Vector3(outV[div + ov % div].m_normal) + Vector3(outV[div + (ov + 1) % div].m_normal)), &outV[ov].m_normal.x);
		++ov;
	}
	memset(&outV[ov], 0, sizeof(outV[ov]));
	storeXYZ(centre, &outV[ov].m_position.x);
	outV[ov].m_normal = -Vector3::zAxis();
	++ov;

	uint32_t oi = 0;
	for (uint32_t i = 0; i < div; ++i)
	{
		outI[oi++] = (i + 1) % div;
		outI[oi++] = i;
		outI[oi++] = div * 3;
		outI[oi++] = div + i;
		outI[oi++] = div + (i + 1) % div;
		outI[oi++] = div * 2 + i;
	}
}

ConeMesh::~ConeMesh()
{
	UNTAG_THIS_DERIVED_CLASS;
}

BoxMesh::BoxMesh(VideoAllocator &videoMemory, float width, float height, float depth)
	: Mesh("BoxMesh", videoMemory, 4 * 6, 3 * 6 * 2)
{
	TAG_THIS_DERIVED_CLASS;
	m_bounds.setMinimum(Point3(-width * 0.5f, -height * 0.5f, -depth * 0.5f)).setMaximum(Point3(width * 0.5f, height * 0.5f, depth * 0.5f));
#if _SCE_TARGET_OS_PROSPERO
	auto outV = reinterpret_cast<Vertex *>(m_vertexBuffers[0].getDataAddress());
#endif
#if _SCE_TARGET_OS_ORBIS
	auto outV = reinterpret_cast<Vertex *>(m_vertexBuffers[0].getBaseAddress());
#endif
	auto outI = reinterpret_cast<sce::SampleUtil::Graphics::Index *>(m_pIndexBuffer);

	Vector3 normals[6] =
	{
		Vector3(+1.f, 0.f, 0.f ),
		Vector3(-1.f, 0.f, 0.f ),
		Vector3(0.f, +1.f, 0.f ),
		Vector3(0.f, -1.f, 0.f ),
		Vector3(0.f, 0.f, +1.f ),
		Vector3(0.f, 0.f, -1.f )
	};
	Vector3 tangents[6] =
	{
		Vector3(0.f, 0.f, -1.f ),
		Vector3(0.f, 0.f, +1.f ),
		Vector3(0.f, 0.f, -1.f ),
		Vector3(0.f, 0.f, +1.f ),
		Vector3(+1.f, 0.f, 0.f ),
		Vector3(-1.f, 0.f, 0.f )
	};

	for (int face = 0; face < 6; face++)
	{
		Vector3 binormal = cross(normals[face], tangents[face]);
		for (int vertex = 0; vertex < 4; vertex++)
		{
			float x = (vertex & 1) ? 1.f : -1.f;
			float y = (vertex & 2) ? 1.f : -1.f;
			Vector3 pos = (normals[face] + x * tangents[face] + y * binormal) * 0.5f;
			outV[face * 4 + vertex].m_position	= Vector3( pos.getX() * width, pos.getY() * height, pos.getZ() * depth );
			outV[face * 4 + vertex].m_uv		= Vector2( x * 0.5f + 0.5f, -y * 0.5f + 0.5f );
			outV[face * 4 + vertex].m_normal	= Vector3( normals[face].getX(), normals[face].getY(), normals[face].getZ() );
			outV[face * 4 + vertex].m_tangent	= Vector4( tangents[face].getX(), tangents[face].getY(), tangents[face].getZ(), 0 );
		}
		outI[face * 6 + 0] = face * 4 + 0;
		outI[face * 6 + 1] = face * 4 + 1;
		outI[face * 6 + 2] = face * 4 + 2;
		outI[face * 6 + 3] = face * 4 + 2;
		outI[face * 6 + 4] = face * 4 + 1;
		outI[face * 6 + 5] = face * 4 + 3;
	}
}

BoxMesh::~BoxMesh()
{
	UNTAG_THIS_DERIVED_CLASS;
}

}}} // namespace sce::SampleUtil::Graphics