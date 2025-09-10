/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once
#include "shader_common.h"
#ifndef __PSSL__
#include <string>
#include <array>
#ifndef WIN32
#include "pack/pack_file.h"
#include "sampleutil/graphics/texture_library.h"
#endif
#endif

namespace sce { namespace SampleUtil { namespace Graphics {
#ifndef WIN32
/*!
 * @~English
 * @brief Shader material
 * @details Parameter to be passed to pixel shader when drawing models
 * @~Japanese
 * @brief シェーダマテリアル
 * @details モデル描画時にピクセルシェーダに渡すパラメータ
 */
struct ShaderMaterial
{
	uint							m_model;
	uint							m_enableAlphaTest;
	uint							m_isTwoSided;
	uint							__padding__[2];
	Vector3Unaligned				m_emissiveWeights;
	uint							m_materialId;
	Vector3Unaligned				m_diffuseWeights;
	uint							m_hasTransparency;
	Vector3Unaligned				m_specularWeights;
	float							m_normalWeight;
	Vector3Unaligned				m_ambientWeights;
	float							m_ambientOcclusionWeight;
	float							m_metalnessWeight;
	float							m_roughnessWeight;
	float							m_reflectivityWeight;
	Texture2D<Vector3Unaligned>		m_emissiveMap;
	Texture2D<Vector4Unaligned>		m_diffuseMap;
	Texture2D<Vector3Unaligned>		m_specularMap;
	Texture2D<Vector3Unaligned>		m_normalMap;
	Texture2D<float>				m_maskMap;
	Texture2D<float>				m_roughnessMap;
	Texture2D<float>				m_metalnessMap;
	Texture2D<Vector3Unaligned>		m_ambientOcclusionMap;
	Texture2D<Vector3Unaligned>		m_shininessMap;
};
#endif
#ifndef __PSSL__
/*!
 * @~English
 * @brief Offline material
 * @details Material to be embedded to model data file
 * @~Japanese
 * @brief オフラインマテリアル
 * @details モデルデータファイルに埋め込むマテリアル
 */
struct Material
{
	std::array<float, 3>			m_emissiveWeights;
	uint32_t						m_materialId;
	std::array<float, 3>			m_diffuseWeights;
	bool							m_hasTransparency;
	std::array<float, 3>			m_specularWeights;
	std::array<float, 3>			m_ambientWeights;
	std::string						m_emissiveMapFilename;
	std::string						m_diffuseMapFilename;
	std::string						m_specularMapFilename;
	std::string						m_normalMapFilename;
	std::string						m_maskMapFilename;
	std::string						m_roughnessMapFilename;
	std::string						m_metalnessMapFilename;

	Material()
	: m_materialId			(0)
	, m_hasTransparency		(false)
	, m_emissiveMapFilename	("")
	, m_diffuseMapFilename	("")
	, m_specularMapFilename	("")
	, m_normalMapFilename	("")
	, m_maskMapFilename		("")
	, m_roughnessMapFilename("")
	, m_metalnessMapFilename("")
	{
		TAG_THIS_CLASS;
		m_emissiveWeights.fill(0.f);
		m_diffuseWeights.fill(1.f);
		m_specularWeights.fill(1.f);
		m_ambientWeights.fill(0.f);
	}
	Material(const Material &) = delete;
	const Material &operator=(const Material &) = delete;

	Material(Material &&rhs)
	{
		TAG_THIS_CLASS;
		*this = std::move(rhs);
	}

	~Material() { UNTAG_THIS_CLASS; }

	const Material	&operator=(Material &&rhs)
	{
		m_emissiveWeights		= std::move(rhs.m_emissiveWeights);
		m_materialId			= rhs.m_materialId; rhs.m_materialId = 0;
		m_diffuseWeights		= std::move(rhs.m_diffuseWeights);
		m_hasTransparency		= rhs.m_hasTransparency; rhs.m_hasTransparency = false;
		m_specularWeights		= std::move(rhs.m_specularWeights);
		m_ambientWeights		= std::move(rhs.m_ambientWeights);
		m_emissiveMapFilename	= std::move(rhs.m_emissiveMapFilename);
		m_diffuseMapFilename	= std::move(rhs.m_diffuseMapFilename);
		m_specularMapFilename	= std::move(rhs.m_specularMapFilename);
		m_normalMapFilename		= std::move(rhs.m_normalMapFilename);
		m_maskMapFilename		= std::move(rhs.m_maskMapFilename);
		m_roughnessMapFilename	= std::move(rhs.m_roughnessMapFilename);
		m_metalnessMapFilename	= std::move(rhs.m_metalnessMapFilename);

		return	*this;
	}
}; // struct Material
#ifndef WIN32
void	initializeShaderMaterial(ShaderMaterial	&shaderMaterial, TextureLibrary	&texLibrary);
/*!
 * @~English
 * @brief Initialized shader material
 * @details Create shader material from offline material
 * @param shaderMaterial Shader material to be created
 * @param source Material in Pack file
 * @param srcTextures Texture array in Pack file
 * @param texLibrary Texture library to store loaded textures
 * @param namePrefix Prefix of material names
 * @~Japanese
 * @brief シェーダマテリアルの初期化
 * @details オフラインマテリアルからシェーダマテリアルを作成します
 * @param shaderMaterial 作成するシェーダマテリアル
 * @param source Packファイルのマテリアル
 * @param srcTextures Packファイルのテクスチャ配列
 * @param texLibrary ロードしたテクスチャライブラリ
 * @param namePrefix マテリアル名につけるprefix
 */
void	initializeShaderMaterialFromPackMaterial(ShaderMaterial	&shaderMaterial, const PackFile::Material &source, const PackFile::Array<PackFile::TextureRef> &srcTextures, TextureLibrary &texLibrary, const std::string &namePrefix);
/*!
 * @~English
 * @brief Resolves shader material
 * @details Checks if texture loadings are finished, and fixes up Gnf files.
 * @param shaderMaterial Shader material to be resolved
 * @param texLibrary Texture library by which textures are loaded.
 * @~Japanese
 * @brief シェーダマテリアルの解決
 * @details テクスチャのロードを確認し、GnfファイルをFixUpします。
 * @param shaderMaterial 解決するシェーダマテリアル
 * @param texLibrary テクスチャをロードしたテクスチャライブラリ
 */
void	resolveShaderMaterial(ShaderMaterial	&shaderMaterial, TextureLibrary &texLibrary);
#endif
#endif
}}} // namespace sce::SampleUtil::Graphics
