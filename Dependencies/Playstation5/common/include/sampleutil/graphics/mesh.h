/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <cstdint>
#include <array>
#include <functional>
#include <string>
#include <scebase_common.h>
#include <sce_geometry.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/core/vertexattribute.h>
#endif
#include "sampleutil/graphics/vertex_define.h"
#include "sampleutil/graphics/compat.h"
#include "sampleutil/memory.h"
#include "mesh_common.h"
#if _SCE_TARGET_OS_PROSPERO
#include "sampleutil/helper/prospero/asset_pack_prospero.h"
#endif

// forward declaration
namespace PackFile
{
	struct Mesh;
	struct Buffer;
	struct DataMapping;
	template<typename T> class Array;
}

namespace sce { namespace SampleUtil { namespace Graphics {
	/*!
	 * @~English
	 * @brief Vertex index
	 * @~Japanese
	 * @brief 頂点インデックス
	 */
	typedef uint16_t	Index;

	/*!
	 * @~English
	 * @brief Mesh
	 * @details Structure to contain mesh index buffer and vertex buffer and so on.
	 * @~Japanese
	 * @brief メッシュ
	 * @details メッシュのインデックスバッファ、頂点バッファを格納する構造体
	 */
	struct Mesh
	{
		enum IndexCountMask : uint32_t
		{
			kIndexSize32bit	= 0x80000000u,		//! Index size is 32bit
			kCount			= ~kIndexSize32bit	//! Index count or offset
		};

		struct VertexAttributeFlags
		{
			uint8_t		m_hasNormal		: 1;
			uint8_t		m_hasColor		: 1;
			uint8_t		m_hasTangent	: 1;
			uint8_t		m_uvCount		: 2;
			uint8_t		m_hasSkinning	: 1;

			VertexAttributeFlags()
				: m_hasNormal	(false)
				, m_hasColor	(0)
				, m_hasTangent	(0)
				, m_uvCount		(0)
				, m_hasSkinning	(0)
			{}
			VertexAttributeFlags(bool	hasNormal, bool	hasColor, bool	hasTangent, uint32_t	uvCount, bool	hasSkinning)
				: m_hasNormal	(hasNormal)
				, m_hasColor	(hasColor)
				, m_hasTangent	(hasTangent)
				, m_uvCount		(uvCount)
				, m_hasSkinning	(hasSkinning)
			{}

			bool	operator==(const VertexAttributeFlags &rhs) const
			{
				return
					m_hasNormal		== rhs.m_hasNormal &&
					m_hasColor		== rhs.m_hasColor &&
					m_hasTangent	== rhs.m_hasTangent &&
					m_uvCount		== rhs.m_uvCount &&
					m_hasSkinning	== rhs.m_hasSkinning;
			}
		};

		std::string										m_name;
		WindingOrder									m_frontFace;
		Compat::PrimitiveType							m_primType;
		uint32_t										m_indexElementSizeInBytes;
		uint32_t										m_indexSizeInBytes;
		uint32_t										m_bonesPerVertex;
		VertexAttributeFlags							m_vertexAttributeFlags;
		Memory::Gpu::unique_ptr<uint8_t[]>				m_gpuMemory;
		Memory::Gpu::unique_ptr<uint8_t[]>				m_cpuMemory;
		void											*m_pIndexBuffer;

		sce::Geometry::Aos::Bounds						m_bounds;
#if _SCE_TARGET_OS_PROSPERO
		std::vector<sce::Agc::Core::VertexAttribute>	m_vertexAttributes;
		uint64_t										m_vertexAttrsHash[4];
#endif
		std::vector<Compat::Buffer>						m_vertexBuffers;

		/*!
		 * @~English
		 * @brief Constructor
		 * @param name Mesh name
		 * @param videoMemory Memory allocator to be used to allocate index buffer and vertex buffer
		 * @param vertexCount Vertex count
		 * @param indexCount Index count
		 * @param hasSkinning Skinning enabled or not
		 * @param frontFace Front face winding
		 * @param primType Primitive type
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param name メッシュ名
		 * @param videoMemory インデックスバッファ・頂点バッファを確保する際に使用するメモリアロケータ
		 * @param vertexCount 頂点数
		 * @param indexCount インデックス数
		 * @param hasSkinning スキニングを行うかどうか
		 * @param frontFace 表面の向き
		 * @param primType プリミティブタイプ
		 */
		Mesh(const std::string	&name, VideoAllocator &videoMemory, uint32_t vertexCount, uint32_t indexCount, bool	hasSkinning = false, WindingOrder frontFace = WindingOrder::kCcw, PrimitiveType primType = PrimitiveType::kTriList);
		Mesh(PackFile::Mesh const	&params, PackFile::Buffer const	*pBuffers, void	*pBufferMemory, VideoAllocator &videoMemory);
		Mesh(PackFile::Mesh const	&params, PackFile::Buffer const	*pBuffers, PackFile::Array<PackFile::DataMapping> const	&dataMapping, VideoAllocator	&videoMemory);
		Mesh()
		{
			TAG_THIS_CLASS;
		}
		Mesh(const Mesh &)					= delete;
		const Mesh &operator=(const Mesh &)	= delete;

		Mesh(Mesh	&&rhs)
		{
			TAG_THIS_CLASS;
			*this = std::move(rhs);
		}

		virtual ~Mesh();

		const Mesh& operator=(Mesh &&rhs)
		{
			m_name						= std::move(rhs.m_name);
			m_frontFace					= rhs.m_frontFace;
			m_primType					= rhs.m_primType;
			m_indexElementSizeInBytes	= rhs.m_indexElementSizeInBytes; rhs.m_indexElementSizeInBytes = 0;
			m_indexSizeInBytes			= rhs.m_indexSizeInBytes; rhs.m_indexSizeInBytes = 0;
			m_bounds					= rhs.m_bounds;
			m_vertexBuffers				= std::move(rhs.m_vertexBuffers); rhs.m_vertexBuffers.clear();
			m_bonesPerVertex			= rhs.m_bonesPerVertex; rhs.m_bonesPerVertex = 0;
			m_vertexAttributeFlags		= rhs.m_vertexAttributeFlags; rhs.m_vertexAttributeFlags = {};
			m_gpuMemory					= std::move(rhs.m_gpuMemory); rhs.m_gpuMemory.release();
			m_cpuMemory					= std::move(rhs.m_cpuMemory); rhs.m_cpuMemory.release();
			m_pIndexBuffer				= rhs.m_pIndexBuffer; rhs.m_pIndexBuffer = nullptr;
#if _SCE_TARGET_OS_PROSPERO
			m_vertexAttributes			= std::move(rhs.m_vertexAttributes); rhs.m_vertexAttributes.clear();
#endif

			return	*this;
		}
	};

	/*!
	 * @~English
	 * @brief Sphere mesh
	 * @~Japanese
	 * @brief 球メッシュ
	 */
	struct SphereMesh : public Mesh
	{
		/*!
		 * @~English
		 * @brief Constructor
		 * @param videoMemory Memory allocator to be used to allocate index buffer and vertex buffer
		 * @param radius Radius
		 * @param xdiv Polygon split count in X-axis direction
		 * @param ydiv Polygon split count in Y-axis direction
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param videoMemory インデックスバッファ・頂点バッファを確保する際に使用するメモリアロケータ
		 * @param radius 半径
		 * @param xdiv X方向のポリゴン分割数
		 * @param ydiv Y方向のポリゴン分割数
		 */
		SphereMesh(VideoAllocator &videoMemory, float radius, long xdiv, long ydiv);
		virtual ~SphereMesh();
	};

	/*!
	 * @~English
	 * @brief Cylinder mesh
	 * @~Japanese
	 * @brief シリンダーメッシュ
	 */
	struct CylinderMesh : public Mesh
	{
		/*!
		 * @~English
		 * @brief Constructor
		 * @param videoMemory Memory allocator to be used to allocate index buffer and vertex buffer
		 * @param radius Radius
		 * @param height Height
		 * @param div Polygon split count in circumferential direction
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param videoMemory インデックスバッファ・頂点バッファを確保する際に使用するメモリアロケータ
		 * @param radius 半径
		 * @param height 高さ
		 * @param div 円周方向のポリゴン分割数
		 */
		CylinderMesh(VideoAllocator &videoMemory, float radius, float height, unsigned div);
		virtual ~CylinderMesh();
	};

	/*!
	 * @~English
	 * @brief Cone mesh
	 * @~Japanese
	 * @brief コーンメッシュ
	 */
	struct ConeMesh : public Mesh
	{
		/*!
		 * @~English
		 * @brief Constructor
		 * @param videoMemory Memory allocator to be used to allocate index buffer and vertex buffer
		 * @param radius Radius
		 * @param height Height
		 * @param div Polygon split count in circumferential direction
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param videoMemory インデックスバッファ・頂点バッファを確保する際に使用するメモリアロケータ
		 * @param radius 半径
		 * @param height 高さ
		 * @param div 円周方向のポリゴン分割数
		 */
		ConeMesh(VideoAllocator &videoMemory, float radius, float height, unsigned div);
		virtual ~ConeMesh();
	};

	/*!
	 * @~English
	 * @brief Box mesh
	 * @~Japanese
	 * @brief ボックスメッシュ
	 */
	struct BoxMesh : public Mesh
	{
		/*!
		 * @~English
		 * @brief Constructor
		 * @param videoMemory Memory allocator to be used to allocate index buffer and vertex buffer
		 * @param width Width
		 * @param height Height
		 * @param depth Depth
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param videoMemory インデックスバッファ・頂点バッファを確保する際に使用するメモリアロケータ
		 * @param width 幅
		 * @param height 高さ
		 * @param depth 奥行
		 */
		BoxMesh(VideoAllocator &videoMemory, float width, float height, float depth);
		virtual ~BoxMesh();
	};

} } } // namespace sce::SampleUtl::Graphics

namespace std
{
	template<>
	struct hash<sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags>
	{
		std::size_t	operator()(const sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags &data) const
		{
			return
				((std::size_t)data.m_hasNormal) |
				(((std::size_t)data.m_hasTangent) << 1) |
				(((std::size_t)data.m_hasColor) << 2) |
				(((std::size_t)data.m_uvCount) << 3) |
				(((std::size_t)data.m_hasSkinning) << 5);
		}
	};
}

