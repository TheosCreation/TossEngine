/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc.
 * 
 */
#ifndef SCE_SAMPLE_UTIL_DISABLE_MODEL_RENDER
#include <scebase_common.h>
#include <vectormath/cpp/vectormath_aos.h>
#include "sampleutil/graphics/surface_utility.h"
#include "sampleutil/graphics/material.h"
#include "pack/pack_file.h"

using namespace sce::Vectormath::Simd::Aos;

namespace
{
	size_t dataSize(PackFile::MaterialPropertyType::DataType dataType)
	{
		if (dataType == PackFile::MaterialPropertyType::e_mpt_texture) {
			return sizeof(PackFile::TextureProperties);
		}
		switch (dataType) {
		case PackFile::MaterialPropertyType::e_mpt_char:   return sizeof(uint8_t);
		case PackFile::MaterialPropertyType::e_mpt_short:  return sizeof(uint16_t);
		case PackFile::MaterialPropertyType::e_mpt_int:    return sizeof(int32_t);
		case PackFile::MaterialPropertyType::e_mpt_uchar:  return sizeof(uint8_t);
		case PackFile::MaterialPropertyType::e_mpt_ushort: return sizeof(uint16_t);
		case PackFile::MaterialPropertyType::e_mpt_uint:   return sizeof(uint32_t);
		case PackFile::MaterialPropertyType::e_mpt_half:   return sizeof(short);
		case PackFile::MaterialPropertyType::e_mpt_float:  return sizeof(float);
		default: return 0;
		}
	}

	float	ptrToFloat(PackFile::MaterialPropertyType::DataType dataType, void const *ptr)
	{
		switch (dataType) {
		case PackFile::MaterialPropertyType::e_mpt_char:   return (float)(*(uint8_t *)(ptr));
		case PackFile::MaterialPropertyType::e_mpt_short:  return (float)(*(uint16_t *)(ptr));
		case PackFile::MaterialPropertyType::e_mpt_int:    return (float)(*(int32_t *)(ptr));
		case PackFile::MaterialPropertyType::e_mpt_uchar:  return (float)(*(uint8_t *)(ptr));
		case PackFile::MaterialPropertyType::e_mpt_ushort: return (float)(*(uint16_t *)(ptr));
		case PackFile::MaterialPropertyType::e_mpt_uint:   return (float)(*(uint32_t *)(ptr));
		case PackFile::MaterialPropertyType::e_mpt_half:   return (float)(*(short *)(ptr)); // TODO: is there a native type for half in Clang?
		case PackFile::MaterialPropertyType::e_mpt_float:  return *(float *)(ptr);
		default: return 0.f;
		}
	}

	float	attributeToFloat(PackFile::MaterialProperty const	&properties)
	{
		auto elemType = PackFile::MaterialPropertyType::get_data_type(properties.type);
		SCE_SAMPLE_UTIL_ASSERT(PackFile::MaterialPropertyType::get_multiplicity(properties.type) == PackFile::MaterialPropertyType::e_mpt_scalar);
		return	ptrToFloat(elemType, properties.value_ptr);
	}

	Vector3	attributeToVector3(PackFile::MaterialProperty const &properties)
	{
		auto elemType = PackFile::MaterialPropertyType::get_data_type(properties.type);
		auto elemMult = PackFile::MaterialPropertyType::get_multiplicity(properties.type);
		auto elemSize = dataSize(elemType);
		uint8_t const *elemPtr = (uint8_t *)properties.value_ptr;
		Vector3 retValue;
		if (elemMult == PackFile::MaterialPropertyType::e_mpt_scalar) {
			float x = ptrToFloat(elemType, elemPtr);
			retValue.setX(x);
			retValue.setY(x);
			retValue.setZ(x);
		} else if (elemMult == PackFile::MaterialPropertyType::e_mpt_vec2) {
			retValue.setX(ptrToFloat(elemType, elemPtr));
			retValue.setY(ptrToFloat(elemType, elemPtr + elemSize));
			retValue.setZ(0.f);
		} else if (elemMult == PackFile::MaterialPropertyType::e_mpt_vec3) {
			retValue.setX(ptrToFloat(elemType, elemPtr));
			retValue.setY(ptrToFloat(elemType, elemPtr + elemSize));
			retValue.setZ(ptrToFloat(elemType, elemPtr + elemSize * 2));
		} else if (elemMult == PackFile::MaterialPropertyType::e_mpt_vec4) {
			retValue.setX(ptrToFloat(elemType, elemPtr));
			retValue.setY(ptrToFloat(elemType, elemPtr + elemSize));
			retValue.setZ(ptrToFloat(elemType, elemPtr + elemSize * 2));
			// Ignore 4th component
		}

		return retValue;
	}

	void	getDiffuseMaterialData(sce::SampleUtil::Graphics::ShaderMaterial &outData, PackFile::Material const &inputMaterial, const PackFile::Array<PackFile::TextureRef> &srcTextures, sce::SampleUtil::Graphics::TextureLibrary &texLibrary, const std::string &namePrefix)
	{
		using namespace PackFile;

		// pssl::to_float3(white, pOutData->albedo_tx);
		initializeShaderMaterial(outData, texLibrary);
		outData.m_diffuseWeights = Vector3Unaligned{ 1.f, 1.f, 1.f };

		Vector3 emissive{ 1.f, 1.f, 1.f };
		bool isEmissiveFound = false;
		for (auto p : inputMaterial.properties) {
			if (MaterialPropertyType::get_data_type(p.type) == MaterialPropertyType::e_mpt_texture) {
				PackFile::TextureProperties ref;
				std::memcpy(&ref, p.value_ptr, sizeof(ref));
				const PackFile::TextureRef &texRef = srcTextures[ref.tx_idx];
				// Texture
				switch (p.semantic) {
				case e_mps_albedo:
				case e_mps_base_color:
					texLibrary.getTexture(outData.m_diffuseMap, namePrefix + texRef.path.data(), sce::SampleUtil::Graphics::TextureType::kDiffuse);
					break;
				case e_mps_normal:
					texLibrary.getTexture(outData.m_normalMap, namePrefix + texRef.path.data(), sce::SampleUtil::Graphics::TextureType::kNormal);
					break;
				case e_mps_emissive:
					texLibrary.getTexture(outData.m_emissiveMap, namePrefix + texRef.path.data(), sce::SampleUtil::Graphics::TextureType::kEmissive);
					break;
				case e_mps_ambient_occlusion:
					texLibrary.getTexture(outData.m_ambientOcclusionMap, namePrefix + texRef.path.data(), sce::SampleUtil::Graphics::TextureType::kAmbientOcclusion);
					break;
				default:
					// ignored
					break;
				}
			} else {
				// Constant property
				switch (p.semantic) {
				case e_mps_albedo:
				case e_mps_base_color:
					outData.m_diffuseWeights = ToVector3Unaligned(attributeToVector3(p));
					break;
				case e_mps_ambient:
					outData.m_ambientWeights = ToVector3Unaligned(attributeToVector3(p));
					break;
				case e_mps_emissive:
					emissive = mulPerElem(emissive, attributeToVector3(p));
					isEmissiveFound = true;
					break;
				case e_mps_ambient_occlusion:
					outData.m_ambientOcclusionWeight = attributeToFloat(p);
					break;
				case e_mps_normal_strength:
					outData.m_normalWeight = attributeToFloat(p);
					break;
				default:
					// ignored
					break;
				}
			}
		}
		if (isEmissiveFound) {
			outData.m_emissiveWeights = ToVector3Unaligned(emissive);
		}
	}

	void	getPhongMaterialData(sce::SampleUtil::Graphics::ShaderMaterial &outData, PackFile::Material const &inputMaterial, const PackFile::Array<PackFile::TextureRef> &srcTextures, sce::SampleUtil::Graphics::TextureLibrary &texLibrary, const std::string &namePrefix)
	{
		using namespace PackFile;

		getDiffuseMaterialData(outData, inputMaterial, srcTextures, texLibrary, namePrefix);

		for (auto p : inputMaterial.properties) {
			if (MaterialPropertyType::get_data_type(p.type) == MaterialPropertyType::e_mpt_texture) {
				PackFile::TextureProperties ref;
				std::memcpy(&ref, p.value_ptr, sizeof(ref));
				const PackFile::TextureRef &texRef = srcTextures[ref.tx_idx];
				switch (p.semantic) {
				case e_mps_shininess:
					texLibrary.getTexture(outData.m_shininessMap, namePrefix + texRef.path.data(), sce::SampleUtil::Graphics::TextureType::kShininess);
					break;
				default:
					// ignored
					break;
				}
			} else {
				switch (p.semantic) {
				case e_mps_specular:
					outData.m_specularWeights = ToVector3Unaligned(attributeToVector3(p));
					break;
				default:
					// ignored
					break;
				}
			}
		}
	}

	void	getPbrMetalnessMaterialData(sce::SampleUtil::Graphics::ShaderMaterial &outData, PackFile::Material const &inputMaterial, const PackFile::Array<PackFile::TextureRef> &srcTextures, sce::SampleUtil::Graphics::TextureLibrary &texLibrary, const std::string &namePrefix)
	{
		using namespace PackFile;

		getDiffuseMaterialData(outData, inputMaterial, srcTextures, texLibrary, namePrefix);

		for (auto p : inputMaterial.properties) {
			if (MaterialPropertyType::get_data_type(p.type) == MaterialPropertyType::e_mpt_texture) {
				PackFile::TextureProperties ref;
				std::memcpy(&ref, p.value_ptr, sizeof(ref));
				const PackFile::TextureRef &texRef = srcTextures[ref.tx_idx];
				switch (p.semantic) {
				case e_mps_metalness:
					texLibrary.getTexture(outData.m_metalnessMap, namePrefix + texRef.path.data(), sce::SampleUtil::Graphics::TextureType::kMetalness);
					break;
				case e_mps_roughness:
					texLibrary.getTexture(outData.m_roughnessMap, namePrefix + texRef.path.data(), sce::SampleUtil::Graphics::TextureType::kRoughness);
					break;
				default:
					// ignored
					break;
				}
			} else {
				switch (p.semantic) {
				case e_mps_metalness:
					outData.m_metalnessWeight = attributeToFloat(p);
					break;
				case e_mps_roughness:
					outData.m_roughnessWeight = attributeToFloat(p);
					break;
				case e_mps_reflectivity:
					outData.m_reflectivityWeight = attributeToFloat(p);
					break;
				default:
					// ignored
					break;
				}
			}
		}
	}

	void	getPbrSpecularMaterialData(sce::SampleUtil::Graphics::ShaderMaterial &outData, PackFile::Material const &inputMaterial, const PackFile::Array<PackFile::TextureRef> &srcTextures, sce::SampleUtil::Graphics::TextureLibrary &texLibrary, const std::string &namePrefix)
	{
		using namespace PackFile;

		getDiffuseMaterialData(outData, inputMaterial, srcTextures, texLibrary, namePrefix);

		for (auto p : inputMaterial.properties) {
			if (MaterialPropertyType::get_data_type(p.type) == MaterialPropertyType::e_mpt_texture) {
				PackFile::TextureProperties ref;
				std::memcpy(&ref, p.value_ptr, sizeof(ref));
				const PackFile::TextureRef &texRef = srcTextures[ref.tx_idx];
				switch (p.semantic) {
				case e_mps_specular:
					texLibrary.getTexture(outData.m_specularMap, namePrefix + texRef.path.data(), sce::SampleUtil::Graphics::TextureType::kSpecular);
					break;
				default:
					// ignored
					break;
				}
			} else {
				switch (p.semantic) {
				case e_mps_specular:
					outData.m_specularWeights = ToVector3Unaligned(attributeToVector3(p));
					break;
				default:
					// ignored
					break;
				}
			}
		}
	}

	void	getUnlitMaterialData(sce::SampleUtil::Graphics::ShaderMaterial &outData, PackFile::Material const &inputMaterial, const PackFile::Array<PackFile::TextureRef> &srcTextures, sce::SampleUtil::Graphics::TextureLibrary &texLibrary, const std::string &namePrefix)
	{
		using namespace PackFile;

		initializeShaderMaterial(outData, texLibrary);
#if _SCE_TARGET_OS_PROSPERO
		outData.m_diffuseMap.setFormat(sce::Agc::Core::TypedFormat::k8_8_8_8UNorm);
		outData.m_diffuseMap.setSwizzle(sce::Agc::Core::Swizzle::k1111);
#endif
#if _SCE_TARGET_OS_ORBIS
		outData.m_diffuseMap.setDataFormat(sce::Gnm::DataFormat::build(sce::Gnm::kSurfaceFormat8_8_8_8, sce::Gnm::kTextureChannelTypeUNorm, sce::Gnm::kTextureChannelConstant1, sce::Gnm::kTextureChannelConstant1, sce::Gnm::kTextureChannelConstant1, sce::Gnm::kTextureChannelConstant1));
#endif
		outData.m_diffuseWeights = Vector3Unaligned{ 1.f, 1.f, 1.f };

		for (auto p : inputMaterial.properties) {
			if (MaterialPropertyType::get_data_type(p.type) == MaterialPropertyType::e_mpt_texture) {
				PackFile::TextureProperties ref;
				std::memcpy(&ref, p.value_ptr, sizeof(ref));
				const PackFile::TextureRef &texRef = srcTextures[ref.tx_idx];
				switch (p.semantic) {
				case e_mps_base_color:
					texLibrary.getTexture(outData.m_diffuseMap, namePrefix + texRef.path.data(), sce::SampleUtil::Graphics::TextureType::kDiffuse);
					break;
				default:
					// ignored
					break;
				}
			} else {
				switch (p.semantic) {
				case e_mps_base_color:
					outData.m_diffuseWeights = ToVector3Unaligned(attributeToVector3(p));
					break;
				default:
					// ignored
					break;
				}
			}
		}
	}

	void	getToonMaterialData(sce::SampleUtil::Graphics::ShaderMaterial &outData, PackFile::Material const &inputMaterial, const PackFile::Array<PackFile::TextureRef> &srcTextures, sce::SampleUtil::Graphics::TextureLibrary &texLibrary, const std::string &namePrefix)
	{
		using namespace PackFile;

		initializeShaderMaterial(outData, texLibrary);
#if _SCE_TARGET_OS_PROSPERO
		outData.m_diffuseMap.setFormat(sce::Agc::Core::TypedFormat::k8_8_8_8UNorm);
		outData.m_diffuseMap.setSwizzle(sce::Agc::Core::Swizzle::k1111);
		outData.m_emissiveMap.setFormat(sce::Agc::Core::TypedFormat::k8_8_8_8UNorm);
		outData.m_emissiveMap.setSwizzle(sce::Agc::Core::Swizzle::k0001);
		outData.m_ambientOcclusionMap.setFormat(sce::Agc::Core::TypedFormat::k8_8_8_8UNorm);
		outData.m_ambientOcclusionMap.setSwizzle(sce::Agc::Core::Swizzle::k1111);
#endif
#if _SCE_TARGET_OS_ORBIS
		outData.m_diffuseMap.setDataFormat(sce::Gnm::DataFormat::build(sce::Gnm::kSurfaceFormat8_8_8_8, sce::Gnm::kTextureChannelTypeUNorm, sce::Gnm::kTextureChannelConstant1, sce::Gnm::kTextureChannelConstant1, sce::Gnm::kTextureChannelConstant1, sce::Gnm::kTextureChannelConstant1));
		outData.m_emissiveMap.setDataFormat(sce::Gnm::DataFormat::build(sce::Gnm::kSurfaceFormat8_8_8_8, sce::Gnm::kTextureChannelTypeUNorm, sce::Gnm::kTextureChannelConstant0, sce::Gnm::kTextureChannelConstant0, sce::Gnm::kTextureChannelConstant0, sce::Gnm::kTextureChannelConstant1));
		outData.m_ambientOcclusionMap.setDataFormat(sce::Gnm::DataFormat::build(sce::Gnm::kSurfaceFormat8_8_8_8, sce::Gnm::kTextureChannelTypeUNorm, sce::Gnm::kTextureChannelConstant1, sce::Gnm::kTextureChannelConstant1, sce::Gnm::kTextureChannelConstant1, sce::Gnm::kTextureChannelConstant1));
#endif
		outData.m_diffuseWeights = Vector3Unaligned{ 1.f, 1.f, 1.f };

		for (auto p : inputMaterial.properties) {
			if (MaterialPropertyType::get_data_type(p.type) == MaterialPropertyType::e_mpt_texture) {
				PackFile::TextureProperties ref;
				std::memcpy(&ref, p.value_ptr, sizeof(ref));
				const PackFile::TextureRef &texRef = srcTextures[ref.tx_idx];
				switch (p.semantic) {
				case e_mps_base_color:
					texLibrary.getTexture(outData.m_diffuseMap, namePrefix + texRef.path.data(), sce::SampleUtil::Graphics::TextureType::kDiffuse);
					break;
				case e_mps_emissive:
					texLibrary.getTexture(outData.m_emissiveMap, namePrefix + texRef.path.data(), sce::SampleUtil::Graphics::TextureType::kEmissive);
					break;
				case e_mps_ambient_occlusion:
					texLibrary.getTexture(outData.m_ambientOcclusionMap, namePrefix + texRef.path.data(), sce::SampleUtil::Graphics::TextureType::kAmbientOcclusion);
					break;
				default:
					// ignored
					break;
				}
			} else {
				switch (p.semantic) {
				case e_mps_ambient:
					outData.m_ambientWeights = ToVector3Unaligned(attributeToVector3(p));
					break;
				case e_mps_base_color:
					outData.m_diffuseWeights = ToVector3Unaligned(attributeToVector3(p));
					break;
				case e_mps_emissive:
					outData.m_emissiveWeights = ToVector3Unaligned(attributeToVector3(p));
					break;
				case e_mps_ambient_occlusion:
					outData.m_ambientOcclusionWeight = attributeToFloat(p);
					break;
				default:
					// ignored
					break;
				}
			}
		}
	}
} // anonymous namespace

namespace sce { namespace SampleUtil { namespace Graphics {
	void	initializeShaderMaterial(ShaderMaterial	&shaderMaterial, TextureLibrary	&texLibrary)
	{
		memset(&shaderMaterial, 0, sizeof(ShaderMaterial));
		shaderMaterial.m_emissiveWeights		= ToVector3Unaligned(Vector3(0.f, 0.f, 0.f));
		shaderMaterial.m_diffuseWeights			= ToVector3Unaligned(Vector3(1.f, 1.f, 1.f));
		shaderMaterial.m_specularWeights		= ToVector3Unaligned(Vector3(1.f, 1.f, 1.f));
		shaderMaterial.m_ambientWeights			= ToVector3Unaligned(Vector3(0.f, 0.f, 0.f));
		shaderMaterial.m_normalWeight			= 1.f;
		shaderMaterial.m_ambientOcclusionWeight = 1.f;
		shaderMaterial.m_metalnessWeight		= 0.f;
		shaderMaterial.m_roughnessWeight		= 0.f;
		shaderMaterial.m_reflectivityWeight		= 0.f;

		texLibrary.getTexture(shaderMaterial.m_emissiveMap	, "default_emissive"	, TextureType::kEmissive);
		texLibrary.getTexture(shaderMaterial.m_diffuseMap	, "default_diffuse"		, TextureType::kDiffuse);
		texLibrary.getTexture(shaderMaterial.m_specularMap	, "default_specular"	, TextureType::kSpecular);
		texLibrary.getTexture(shaderMaterial.m_normalMap	, "default_normal"		, TextureType::kNormal);
		texLibrary.getTexture(shaderMaterial.m_maskMap		, "default_mask"		, TextureType::kMask);
		texLibrary.getTexture(shaderMaterial.m_roughnessMap	, "default_roughness"	, TextureType::kRoughness);
		texLibrary.getTexture(shaderMaterial.m_metalnessMap	, "default_metalness"	, TextureType::kMetalness);

		shaderMaterial.m_shininessMap = SurfaceUtil::getPassValidationTexture();
#if _SCE_TARGET_OS_PROSPERO
		shaderMaterial.m_shininessMap.setFormat(Agc::Core::TypedFormat::k8UNorm);
		shaderMaterial.m_shininessMap.setSwizzle(Agc::Core::Swizzle::k0000);
#endif
#if _SCE_TARGET_OS_ORBIS
		shaderMaterial.m_shininessMap.setDataFormat(Gnm::DataFormat::build(Gnm::kSurfaceFormat8, Gnm::kTextureChannelTypeUNorm, Gnm::kTextureChannelConstant1, Gnm::kTextureChannelConstant1, Gnm::kTextureChannelConstant1, Gnm::kTextureChannelConstant1));
#endif
	}

	void	initializeShaderMaterialFromPackMaterial(ShaderMaterial	&shaderMaterial, const PackFile::Material	&source, const PackFile::Array<PackFile::TextureRef>	&srcTextures, TextureLibrary	&texLibrary, const std::string	&namePrefix)
	{
		using PackFile::MaterialModel;

		shaderMaterial.m_model				= source.model;
		shaderMaterial.m_enableAlphaTest	= (bool)(source.flags & PackFile::e_mf_alpha_clip);
		shaderMaterial.m_isTwoSided			= (bool)(source.flags & PackFile::e_mf_two_side);
		switch (source.model) {
		case MaterialModel::e_mt_diffuse_only:
			getDiffuseMaterialData(shaderMaterial, source, srcTextures, texLibrary, namePrefix);
			break;
		case MaterialModel::e_mt_phong:
			getPhongMaterialData(shaderMaterial, source, srcTextures, texLibrary, namePrefix);
			break;
		case MaterialModel::e_mt_pbr_metalness:
			getPbrMetalnessMaterialData(shaderMaterial, source, srcTextures, texLibrary, namePrefix);
			break;
		case MaterialModel::e_mt_pbr_specular:
			getPbrSpecularMaterialData(shaderMaterial, source, srcTextures, texLibrary, namePrefix);
			break;
		case MaterialModel::e_mt_unlit:
			getUnlitMaterialData(shaderMaterial, source, srcTextures, texLibrary, namePrefix);
			break;
		case MaterialModel::e_mt_toon_shading:
			getToonMaterialData(shaderMaterial, source, srcTextures, texLibrary, namePrefix);
			break;
		}
	}

	void	resolveShaderMaterial(ShaderMaterial	&shaderMaterial, TextureLibrary &texLibrary)
	{
		texLibrary.resolveTexture(shaderMaterial.m_emissiveMap);
		texLibrary.resolveTexture(shaderMaterial.m_diffuseMap);
		texLibrary.resolveTexture(shaderMaterial.m_specularMap);
		texLibrary.resolveTexture(shaderMaterial.m_normalMap);
		texLibrary.resolveTexture(shaderMaterial.m_maskMap);
		texLibrary.resolveTexture(shaderMaterial.m_roughnessMap);
		texLibrary.resolveTexture(shaderMaterial.m_metalnessMap);
#if _SCE_TARGET_OS_PROSPERO
		void *pDiffuseMapData = shaderMaterial.m_diffuseMap.getDataAddress();
		void *pMaskMapData = shaderMaterial.m_maskMap.getDataAddress();
		void *pDummyTexData = SurfaceUtil::getPassValidationTexture().getDataAddress();
#endif
#if _SCE_TARGET_OS_ORBIS
		void *pDiffuseMapData = shaderMaterial.m_diffuseMap.getBaseAddress();
		void *pMaskMapData = shaderMaterial.m_maskMap.getBaseAddress();
		void *pDummyTexData = SurfaceUtil::getPassValidationTexture().getBaseAddress();
#endif
		if (pDiffuseMapData == pMaskMapData && pMaskMapData != pDummyTexData)
		{
			// if mask map is same as diffuse map, swizzle texel element so that same shader can sample mask value
#if _SCE_TARGET_OS_PROSPERO
			shaderMaterial.m_maskMap.setSwizzle(Agc::Core::Swizzle::kWWWW);
#endif
#if _SCE_TARGET_OS_ORBIS
			auto dataFormat = shaderMaterial.m_maskMap.getDataFormat();
			dataFormat.m_bits.m_channelX = Gnm::kTextureChannelW;
			dataFormat.m_bits.m_channelY = Gnm::kTextureChannelW;
			dataFormat.m_bits.m_channelZ = Gnm::kTextureChannelW;
			dataFormat.m_bits.m_channelW = Gnm::kTextureChannelW;
			shaderMaterial.m_maskMap.setDataFormat(dataFormat);
#endif
		}
	}
}}} // namespace sce::SampleUtil::Graphics
#endif