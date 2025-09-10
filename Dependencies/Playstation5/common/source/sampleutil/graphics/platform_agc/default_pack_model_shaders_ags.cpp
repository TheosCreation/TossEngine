/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc.
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <sampleutil/graphics/platform_agc/shader_ags.h>
#include <sampleutil/graphics/pack_model.h>

DEFINE_SHADER(default_pack_model_vv);
DEFINE_SHADER(default_pack_model_C_vv);
DEFINE_SHADER(default_pack_model_U0_vv);
DEFINE_SHADER(default_pack_model_CU0_vv);
DEFINE_SHADER(default_pack_model_U1_vv);
DEFINE_SHADER(default_pack_model_CU1_vv);
DEFINE_SHADER(default_pack_model_N_vv);
DEFINE_SHADER(default_pack_model_NC_vv);
DEFINE_SHADER(default_pack_model_NU0_vv);
DEFINE_SHADER(default_pack_model_NCU0_vv);
DEFINE_SHADER(default_pack_model_NTU0_vv);
DEFINE_SHADER(default_pack_model_NCTU0_vv);
DEFINE_SHADER(default_pack_model_NU1_vv);
DEFINE_SHADER(default_pack_model_NCU1_vv);
DEFINE_SHADER(default_pack_model_NTU1_vv);
DEFINE_SHADER(default_pack_model_NCTU1_vv);
DEFINE_SHADER(default_pack_model_S_vv);
DEFINE_SHADER(default_pack_model_CS_vv);
DEFINE_SHADER(default_pack_model_U0S_vv);
DEFINE_SHADER(default_pack_model_CU0S_vv);
DEFINE_SHADER(default_pack_model_U1S_vv);
DEFINE_SHADER(default_pack_model_CU1S_vv);
DEFINE_SHADER(default_pack_model_NS_vv);
DEFINE_SHADER(default_pack_model_NCS_vv);
DEFINE_SHADER(default_pack_model_NU0S_vv);
DEFINE_SHADER(default_pack_model_NCU0S_vv);
DEFINE_SHADER(default_pack_model_NTU0S_vv);
DEFINE_SHADER(default_pack_model_NCTU0S_vv);
DEFINE_SHADER(default_pack_model_NU1S_vv);
DEFINE_SHADER(default_pack_model_NCU1S_vv);
DEFINE_SHADER(default_pack_model_NTU1S_vv);
DEFINE_SHADER(default_pack_model_NCTU1S_vv);

DEFINE_SHADER(default_pack_model_diffuse_NA_p);
DEFINE_SHADER(default_pack_model_diffuse_NU0A_p);
DEFINE_SHADER(default_pack_model_diffuse_NTU0A_p);
DEFINE_SHADER(default_pack_model_diffuse_NCU0A_p);
DEFINE_SHADER(default_pack_model_diffuse_NCTU0A_p);
DEFINE_SHADER(default_pack_model_diffuse_N_p);
DEFINE_SHADER(default_pack_model_diffuse_NU0_p);
DEFINE_SHADER(default_pack_model_diffuse_NTU0_p);
DEFINE_SHADER(default_pack_model_diffuse_NCU0_p);
DEFINE_SHADER(default_pack_model_diffuse_NCTU0_p);
DEFINE_SHADER(default_pack_model_phong_NA_p);
DEFINE_SHADER(default_pack_model_phong_NU0A_p);
DEFINE_SHADER(default_pack_model_phong_NTU0A_p);
DEFINE_SHADER(default_pack_model_phong_NCU0A_p);
DEFINE_SHADER(default_pack_model_phong_NCTU0A_p);
DEFINE_SHADER(default_pack_model_phong_N_p);
DEFINE_SHADER(default_pack_model_phong_NU0_p);
DEFINE_SHADER(default_pack_model_phong_NTU0_p);
DEFINE_SHADER(default_pack_model_phong_NCU0_p);
DEFINE_SHADER(default_pack_model_phong_NCTU0_p);

namespace
{
	std::unordered_map<sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags, const sce::Agc::Shader *>		g_defaultPackModelVsMap;
	std::unordered_map<sce::SampleUtil::Graphics::PixelShaderFlags, const sce::Agc::Shader *>				g_defaultPackModelPsMap;
} // anonymous namespace

namespace sce {	namespace SampleUtil { namespace Graphics {

void	initializeDefaultPackModelShaders()
{
	// create vertex shader map
	CREATE_SHADER(default_pack_model_vv);
	CREATE_SHADER(default_pack_model_C_vv);
	CREATE_SHADER(default_pack_model_U0_vv);
	CREATE_SHADER(default_pack_model_CU0_vv);
	CREATE_SHADER(default_pack_model_U1_vv);
	CREATE_SHADER(default_pack_model_CU1_vv);
	CREATE_SHADER(default_pack_model_N_vv);
	CREATE_SHADER(default_pack_model_NC_vv);
	CREATE_SHADER(default_pack_model_NU0_vv);
	CREATE_SHADER(default_pack_model_NCU0_vv);
	CREATE_SHADER(default_pack_model_NTU0_vv);
	CREATE_SHADER(default_pack_model_NCTU0_vv);
	CREATE_SHADER(default_pack_model_NU1_vv);
	CREATE_SHADER(default_pack_model_NCU1_vv);
	CREATE_SHADER(default_pack_model_NTU1_vv);
	CREATE_SHADER(default_pack_model_NCTU1_vv);
	CREATE_SHADER(default_pack_model_S_vv);
	CREATE_SHADER(default_pack_model_CS_vv);
	CREATE_SHADER(default_pack_model_U0S_vv);
	CREATE_SHADER(default_pack_model_CU0S_vv);
	CREATE_SHADER(default_pack_model_U1S_vv);
	CREATE_SHADER(default_pack_model_CU1S_vv);
	CREATE_SHADER(default_pack_model_NS_vv);
	CREATE_SHADER(default_pack_model_NCS_vv);
	CREATE_SHADER(default_pack_model_NU0S_vv);
	CREATE_SHADER(default_pack_model_NCU0S_vv);
	CREATE_SHADER(default_pack_model_NTU0S_vv);
	CREATE_SHADER(default_pack_model_NCTU0S_vv);
	CREATE_SHADER(default_pack_model_NU1S_vv);
	CREATE_SHADER(default_pack_model_NCU1S_vv);
	CREATE_SHADER(default_pack_model_NTU1S_vv);
	CREATE_SHADER(default_pack_model_NCTU1S_vv);
	//								  +---------- has normal
	//								  | +-------- has color
	//								  | | +------ has tangent
	//								  | | | +---- uv count
	//								  | | | | +-- has skinning
	//								  | | | | |
	g_defaultPackModelVsMap.insert({ {0,0,0,0,0}, ::Shader::default_pack_model_vv });
	g_defaultPackModelVsMap.insert({ {0,1,0,0,0}, ::Shader::default_pack_model_C_vv });
	g_defaultPackModelVsMap.insert({ {0,0,0,1,0}, ::Shader::default_pack_model_U0_vv });
	g_defaultPackModelVsMap.insert({ {0,1,0,1,0}, ::Shader::default_pack_model_CU0_vv });
	g_defaultPackModelVsMap.insert({ {0,0,0,2,0}, ::Shader::default_pack_model_U1_vv });
	g_defaultPackModelVsMap.insert({ {0,1,0,2,0}, ::Shader::default_pack_model_CU1_vv });
	g_defaultPackModelVsMap.insert({ {1,0,0,0,0}, ::Shader::default_pack_model_N_vv });
	g_defaultPackModelVsMap.insert({ {1,1,0,0,0}, ::Shader::default_pack_model_NC_vv });
	g_defaultPackModelVsMap.insert({ {1,0,0,1,0}, ::Shader::default_pack_model_NU0_vv });
	g_defaultPackModelVsMap.insert({ {1,1,0,1,0}, ::Shader::default_pack_model_NCU0_vv });
	g_defaultPackModelVsMap.insert({ {1,0,1,1,0}, ::Shader::default_pack_model_NTU0_vv });
	g_defaultPackModelVsMap.insert({ {1,1,1,1,0}, ::Shader::default_pack_model_NCTU0_vv });
	g_defaultPackModelVsMap.insert({ {1,0,0,2,0}, ::Shader::default_pack_model_NU1_vv });
	g_defaultPackModelVsMap.insert({ {1,1,0,2,0}, ::Shader::default_pack_model_NCU1_vv });
	g_defaultPackModelVsMap.insert({ {1,0,1,2,0}, ::Shader::default_pack_model_NTU1_vv });
	g_defaultPackModelVsMap.insert({ {1,1,1,2,0}, ::Shader::default_pack_model_NCTU1_vv });
	g_defaultPackModelVsMap.insert({ {0,0,0,0,1}, ::Shader::default_pack_model_S_vv });
	g_defaultPackModelVsMap.insert({ {0,1,0,0,1}, ::Shader::default_pack_model_CS_vv });
	g_defaultPackModelVsMap.insert({ {0,0,0,1,1}, ::Shader::default_pack_model_U0S_vv });
	g_defaultPackModelVsMap.insert({ {0,1,0,1,1}, ::Shader::default_pack_model_CU0S_vv });
	g_defaultPackModelVsMap.insert({ {0,0,0,2,1}, ::Shader::default_pack_model_U1S_vv });
	g_defaultPackModelVsMap.insert({ {0,1,0,2,1}, ::Shader::default_pack_model_CU1S_vv });
	g_defaultPackModelVsMap.insert({ {1,0,0,0,1}, ::Shader::default_pack_model_NS_vv });
	g_defaultPackModelVsMap.insert({ {1,1,0,0,1}, ::Shader::default_pack_model_NCS_vv });
	g_defaultPackModelVsMap.insert({ {1,0,0,1,1}, ::Shader::default_pack_model_NU0S_vv });
	g_defaultPackModelVsMap.insert({ {1,1,0,1,1}, ::Shader::default_pack_model_NCU0S_vv });
	g_defaultPackModelVsMap.insert({ {1,0,1,1,1}, ::Shader::default_pack_model_NTU0S_vv });
	g_defaultPackModelVsMap.insert({ {1,1,1,1,1}, ::Shader::default_pack_model_NCTU0S_vv });
	g_defaultPackModelVsMap.insert({ {1,0,0,2,1}, ::Shader::default_pack_model_NU1S_vv });
	g_defaultPackModelVsMap.insert({ {1,1,0,2,1}, ::Shader::default_pack_model_NCU1S_vv });
	g_defaultPackModelVsMap.insert({ {1,0,1,2,1}, ::Shader::default_pack_model_NTU1S_vv });
	g_defaultPackModelVsMap.insert({ {1,1,1,2,1}, ::Shader::default_pack_model_NCTU1S_vv });

	// create pixel shader map
	CREATE_SHADER(default_pack_model_diffuse_NA_p);
	CREATE_SHADER(default_pack_model_diffuse_NU0A_p);
	CREATE_SHADER(default_pack_model_diffuse_NTU0A_p);
	CREATE_SHADER(default_pack_model_diffuse_NCU0A_p);
	CREATE_SHADER(default_pack_model_diffuse_NCTU0A_p);
	CREATE_SHADER(default_pack_model_diffuse_N_p);
	CREATE_SHADER(default_pack_model_diffuse_NU0_p);
	CREATE_SHADER(default_pack_model_diffuse_NTU0_p);
	CREATE_SHADER(default_pack_model_diffuse_NCU0_p);
	CREATE_SHADER(default_pack_model_diffuse_NCTU0_p);
	CREATE_SHADER(default_pack_model_phong_NA_p);
	CREATE_SHADER(default_pack_model_phong_NU0A_p);
	CREATE_SHADER(default_pack_model_phong_NTU0A_p);
	CREATE_SHADER(default_pack_model_phong_NCU0A_p);
	CREATE_SHADER(default_pack_model_phong_NCTU0A_p);
	CREATE_SHADER(default_pack_model_phong_N_p);
	CREATE_SHADER(default_pack_model_phong_NU0_p);
	CREATE_SHADER(default_pack_model_phong_NTU0_p);
	CREATE_SHADER(default_pack_model_phong_NCU0_p);
	CREATE_SHADER(default_pack_model_phong_NCTU0_p);
	//																				  +---------- has normal
	//																				  | +-------- has color
	//																				  | | +------ has tangent
	//																				  | | | +---- uv count
	//																				  | | | | +-- alpha test
	//																				  | | | | |
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_diffuse_only	, 1,0,0,0,1}, ::Shader::default_pack_model_diffuse_NA_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_diffuse_only	, 1,0,0,1,1}, ::Shader::default_pack_model_diffuse_NU0A_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_diffuse_only	, 1,0,1,1,1}, ::Shader::default_pack_model_diffuse_NTU0A_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_diffuse_only	, 1,1,0,1,1}, ::Shader::default_pack_model_diffuse_NCU0A_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_diffuse_only	, 1,1,1,1,1}, ::Shader::default_pack_model_diffuse_NCTU0A_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_diffuse_only	, 1,0,0,0,0}, ::Shader::default_pack_model_diffuse_N_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_diffuse_only	, 1,0,0,1,0}, ::Shader::default_pack_model_diffuse_NU0_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_diffuse_only	, 1,0,1,1,0}, ::Shader::default_pack_model_diffuse_NTU0_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_diffuse_only	, 1,1,0,1,0}, ::Shader::default_pack_model_diffuse_NCU0_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_diffuse_only	, 1,1,1,1,0}, ::Shader::default_pack_model_diffuse_NCTU0_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_phong			, 1,0,0,0,1}, ::Shader::default_pack_model_phong_NA_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_phong			, 1,0,0,1,1}, ::Shader::default_pack_model_phong_NU0A_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_phong			, 1,0,1,1,1}, ::Shader::default_pack_model_phong_NTU0A_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_phong			, 1,1,0,1,1}, ::Shader::default_pack_model_phong_NCU0A_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_phong			, 1,1,1,1,1}, ::Shader::default_pack_model_phong_NCTU0A_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_phong			, 1,0,0,0,0}, ::Shader::default_pack_model_phong_N_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_phong			, 1,0,0,1,0}, ::Shader::default_pack_model_phong_NU0_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_phong			, 1,0,1,1,0}, ::Shader::default_pack_model_phong_NTU0_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_phong			, 1,1,0,1,0}, ::Shader::default_pack_model_phong_NCU0_p });
	g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_phong			, 1,1,1,1,0}, ::Shader::default_pack_model_phong_NCTU0_p });
}

const sce::Agc::Shader	*PackModel::getDefaultVertexShader(const PackModel	&packModel, const sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags &validVertexAttributes)
{
	sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags	queryVertexAttributes	= packModel.m_meshes[0].m_vertexAttributeFlags;
	queryVertexAttributes.m_hasNormal	&= validVertexAttributes.m_hasNormal;
	queryVertexAttributes.m_hasColor	&= validVertexAttributes.m_hasColor;
	queryVertexAttributes.m_hasTangent	&= validVertexAttributes.m_hasTangent;
	queryVertexAttributes.m_uvCount		= std::min(queryVertexAttributes.m_uvCount, validVertexAttributes.m_uvCount);
	queryVertexAttributes.m_hasSkinning	&= validVertexAttributes.m_hasSkinning;
	auto	foundVs = g_defaultPackModelVsMap.find(queryVertexAttributes);
	SCE_SAMPLE_UTIL_ASSERT(foundVs != g_defaultPackModelVsMap.end());
	if (foundVs == g_defaultPackModelVsMap.end()) {
		return	nullptr;
	}
	return	foundVs->second;
}

const sce::Agc::Shader	*PackModel::getDefaultPixelShader(const PackModel	&packModel, const sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags &validVertexAttributes)
{
	const ShaderMaterial &material = packModel.m_materialsMemory.get()[0];
	sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags	queryVertexAttributes = packModel.m_meshes[0].m_vertexAttributeFlags;
	queryVertexAttributes.m_hasNormal	&= validVertexAttributes.m_hasNormal;
	queryVertexAttributes.m_hasColor	&= validVertexAttributes.m_hasColor;
	queryVertexAttributes.m_hasTangent	&= validVertexAttributes.m_hasTangent;
	queryVertexAttributes.m_uvCount		= std::min(queryVertexAttributes.m_uvCount, validVertexAttributes.m_uvCount);
	queryVertexAttributes.m_hasSkinning	&= validVertexAttributes.m_hasSkinning;
	PixelShaderFlags	flags = {
		(PackFile::MaterialModel)material.m_model,
		queryVertexAttributes.m_hasNormal,
		queryVertexAttributes.m_hasColor,
		queryVertexAttributes.m_hasTangent,
		queryVertexAttributes.m_uvCount,
		(bool)(material.m_enableAlphaTest)
	};
	auto	foundPs = g_defaultPackModelPsMap.find(flags);
	if (foundPs == g_defaultPackModelPsMap.end()) {
		// not found -> search for alpha test enabled one
		flags.m_hasAlphaTest = true;
		foundPs = g_defaultPackModelPsMap.find(flags);
		SCE_SAMPLE_UTIL_ASSERT(foundPs != g_defaultPackModelPsMap.end());
		if (foundPs == g_defaultPackModelPsMap.end()) {
			return	nullptr;
		}
	}
	return	foundPs->second;
}

}}} // namespace sce::SampleUtil::Graphics
#endif
