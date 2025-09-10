/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc.
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include <sampleutil/graphics/platform_gnm/shader_gnm.h>
#include <sampleutil/graphics/pack_model.h>

DEFINE_SHADER(sce::Gnmx::VsShader, default_pack_model_NCTU0S_vv);
DEFINE_SHADER(sce::Gnmx::VsShader, default_pack_model_NU0S_vv);
DEFINE_SHADER(sce::Gnmx::PsShader, default_pack_model_diffuse_NCTU0_p);
DEFINE_SHADER(sce::Gnmx::PsShader, default_pack_model_diffuse_NU0_p);

namespace
{
	std::unordered_map<sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags, const sce::SampleUtil::Graphics::Shader<sce::Gnmx::VsShader> *>	g_defaultPackModelVsMap;
	std::unordered_map<sce::SampleUtil::Graphics::PixelShaderFlags			, const sce::SampleUtil::Graphics::Shader<sce::Gnmx::PsShader> *>	g_defaultPackModelPsMap;
} // anonymous namespace

namespace sce { namespace SampleUtil { namespace Graphics {
	void	initializeDefaultPackModelShaders(VideoAllocator	&allocator)
	{
		// create vertex shader map
		CREATE_SHADER(sce::Gnmx::VsShader, default_pack_model_NCTU0S_vv, &allocator);
		CREATE_SHADER(sce::Gnmx::VsShader, default_pack_model_NU0S_vv, &allocator);
		//								  +---------- has normal
		//								  | +-------- has color
		//								  | | +------ has tangent
		//								  | | | +---- uv count
		//								  | | | | +-- has skinning
		//								  | | | | |
		g_defaultPackModelVsMap.insert({ {1,1,1,1,1}, g_shader_default_pack_model_NCTU0S_vv.get()});
		g_defaultPackModelVsMap.insert({ {1,0,0,1,1}, g_shader_default_pack_model_NU0S_vv.get()});

		// create pixel shader map
		CREATE_SHADER(sce::Gnmx::PsShader, default_pack_model_diffuse_NCTU0_p, &allocator);
		CREATE_SHADER(sce::Gnmx::PsShader, default_pack_model_diffuse_NU0_p, &allocator);
		//																				  +---------- has normal
		//																				  | +-------- has color
		//																				  | | +------ has tangent
		//																				  | | | +---- uv count
		//																				  | | | | +-- alpha test
		//																				  | | | | |
		g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_diffuse_only	, 1,1,1,1,1}, g_shader_default_pack_model_diffuse_NCTU0_p.get() });
		g_defaultPackModelPsMap.insert({ { PackFile::MaterialModel::e_mt_diffuse_only	, 1,0,0,1,1}, g_shader_default_pack_model_diffuse_NU0_p.get()});
	}

	void	finalizeDefaultPackModelShaders()
	{
		DESTROY_SHADER(default_pack_model_NCTU0S_vv);
		DESTROY_SHADER(default_pack_model_NU0S_vv);
		DESTROY_SHADER(default_pack_model_diffuse_NCTU0_p);
		DESTROY_SHADER(default_pack_model_diffuse_NU0_p);
	}

	const Shader<sce::Gnmx::VsShader>	*PackModel::getDefaultVertexShader(const PackModel	&packModel)
	{
		auto	foundVs = g_defaultPackModelVsMap.find(packModel.m_meshes[0].m_vertexAttributeFlags);
		SCE_SAMPLE_UTIL_ASSERT(foundVs != g_defaultPackModelVsMap.end());
		if (foundVs == g_defaultPackModelVsMap.end()) {
			return	nullptr;
		}
		return	foundVs->second;
	}

	const Shader<sce::Gnmx::PsShader>	*PackModel::getDefaultPixelShader(const PackModel	&packModel)
	{
		const ShaderMaterial &material = packModel.m_materialsMemory.get()[0];
		PixelShaderFlags	flags = {
			(PackFile::MaterialModel)material.m_model,
			packModel.m_meshes[0].m_vertexAttributeFlags.m_hasNormal,
			packModel.m_meshes[0].m_vertexAttributeFlags.m_hasTangent,
			packModel.m_meshes[0].m_vertexAttributeFlags.m_hasColor,
			packModel.m_meshes[0].m_vertexAttributeFlags.m_uvCount,
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

