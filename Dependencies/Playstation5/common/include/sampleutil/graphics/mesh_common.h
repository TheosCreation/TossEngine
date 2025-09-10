/* SIE CONFIDENTIAL
 PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc.
 * 
 */
// mesh definitions common for C++ and PSSL2
#pragma once

#include "shader_common.h"
#include "vertex_define.h"

namespace sce { namespace SampleUtil { namespace Graphics {

/*!
 * @~English
 * @brief Winding order
 * @~Japanese
 * @brief ワインディング方向
 */
enum class WindingOrder : uint
{
	/*!
	 * @~English
	 * @brief Clockwise
	 * @~Japanese
	 * @brief 時計周り
	 */
	kCw = 0,
	/*!
	 * @~English
	 * @brief Counter clockwise
	 * @~Japanese
	 * @brief 反時計周り
	 */
	kCcw = 1
};

/*!
 * @~English
 * @brief Culling setting
 * @~Japanese
 * @brief カリング設定
 */
enum class CullFace : uint
{
	/*!
	 * @~English
	 * @brief No culling
	 * @~Japanese
	 * @brief カリングなし
	 */
	kNone = 0,
	/*!
	 * @~English
	 * @brief Back face culling
	 * @~Japanese
	 * @brief 背面カリング
	 */
	kBack = 1,
	/*!
	 * @~English
	 * @brief Front face culling
	 * @~Japanese
	 * @brief 表面カリング
	 */
	kFront = 2
};

/*!
 * @~English
 * @brief Primitive type
 * @~Japanese
 * @brief プリミティブタイプ
 */
enum class PrimitiveType : uint
{
	/*!
	 * @~English
	 * @brief Triangle list
	 * @~Japanese
	 * @brief トライアングルリスト
	 */
	kTriList = 0,
	/*!
	 * @~English
	 * @brief Patch
	 * @~Japanese
	 * @brief パッチ
	 */
	kPatch = 1
};

/*!
 * @~English
 * @brief Mesh user data
 * @details This is passed to shader when drawing meshes
 * @~Japanese
 * @brief メッシュユーザデータ
 * @details メッシュを描画する際にシェーダに渡されます
 */
struct MeshUserData
{
	uint								m_indexByteOffset;
	uint								m_vertexOffset;
	WindingOrder						m_frontFace;
	CullFace							m_cullFace;
	PrimitiveType						m_primType;
	RegularBuffer<Matrix4x3Unaligned>	m_transform;
	RegularBuffer<Matrix4Unaligned>		m_skinningMatrices;
};

} } } // namespace sce::SampleUtil::Graphics