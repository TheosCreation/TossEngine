/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once

// C/C++ standard library
#include <vector>
#include <unordered_map>
#include <string>
#include <mspace.h>
// SDK library
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/core/buffer.h>
#include <agc/core/ringbuffer.h>
#include <psr/psr.h>
#endif
#if _SCE_TARGET_OS_ORBIS
#include <gnm/buffer.h>
#endif
#include <sce_geometry.h>
// SampleUtil
#include <pack/material_models.h>
#include "mesh.h"
#include <sampleutil/memory.h>
#include <sampleutil/debug/perf.h>
#include <sampleutil/graphics/graphics_memory.h>
#include <sampleutil/graphics/texture_library.h>
#include <sampleutil/graphics/material.h>
#include <sampleutil/graphics/pack_model_common.h>
#include <sampleutil/graphics/skeletal_animation.h>
#if _SCE_TARGET_OS_PROSPERO
#include <sampleutil/graphics/platform_agc/ringbuffer_safe_srt_binding.h>
#endif
#if _SCE_TARGET_OS_ORBIS
#include <sampleutil/graphics/platform_gnm/shader_gnm.h>
#endif

namespace sce { namespace SampleUtil { namespace Graphics {

struct PixelShaderFlags
{
	PackFile::MaterialModel	m_shadingModel	: 4;
	uint16_t				m_hasNormal		: 1;
	uint16_t				m_hasColor		: 1;
	uint16_t				m_hasTangent	: 1;
	uint16_t				m_uvCount		: 2;
	uint16_t				m_hasAlphaTest	: 1;

	bool	operator==(const PixelShaderFlags &rhs) const
	{
		return
			m_shadingModel	== rhs.m_shadingModel &&
			m_hasNormal		== rhs.m_hasNormal &&
			m_hasColor		== rhs.m_hasColor &&
			m_hasTangent	== rhs.m_hasTangent &&
			m_uvCount		== rhs.m_uvCount &&
			m_hasAlphaTest	== rhs.m_hasAlphaTest;
	}
};

struct MeshInstance
{
	std::string					m_name;
	uint32_t					m_meshIndex;
	uint32_t					m_instanceIndex;
	bool						m_isVisible;
	sce::Geometry::Aos::Bounds	m_bounds;
};

class PackModel
{
public:
	std::string														m_name;
	std::vector<Mesh>												m_meshes;
	std::vector<MeshInstance>										m_instances;
	std::vector<SkeletalAnimation>									m_animations;
	std::vector<RegularBuffer<MeshInstanceUserData>>				m_gpuInstanceUserDataArray;
	RegularBuffer<ShaderMaterial>									m_materialBuffer;

	sce::Geometry::Aos::Bounds										m_bounds;

	// Cpu Data
	std::vector<Memory::Cpu::unique_ptr<uint8_t[]>>					m_packCpuMemory;
	// Gpu Data
	std::vector<Memory::Gpu::unique_ptr<uint8_t[]>>					m_packGpuMemory;
	std::vector<Memory::Gpu::unique_ptr<MeshInstanceUserData[]>>	m_meshInstancesGpuMemory;
	Memory::Gpu::unique_ptr<ShaderMaterial[]>						m_materialsMemory;
#if _SCE_TARGET_OS_PROSPERO
	std::vector<sce::Psr::BottomLevelBvhDescriptor>					m_allBvh;
	sce::SampleUtil::Helper::AsyncAssetLoader						*m_pAsyncLoader;
	sce::SampleUtil::Helper::AsyncAssetLoader::Result				m_packData;
	std::vector<sce::SampleUtil::Helper::AsyncAssetLoader::Result>	m_loadedData;
	bool															m_isResolved;
#endif

	TextureLibrary													*m_pTextureLibrary;

	/*!
	 * @~English
	 * @brief Resolves pack scene instantiation
	 * @details Make sures pack scene load is comleted, and setup loaded data for later drawing
	 * @~Japanese
	 * @brief PackModel生成の解決
	 * @details PackModelのロード完了を確認し、ロードデータが描画可能な状態にセットアップする
	 */
	void	resolve();

	PackModel(const std::string	&packFilename, VideoAllocator	&gpuMemory, SceLibcMspace	cpuMemory, TextureLibrary	&texLibrary, const std::string	&name = "", const std::string &namePrefix = ""
#if _SCE_TARGET_OS_PROSPERO
				, sce::SampleUtil::Helper::AsyncAssetLoader *pLoader = nullptr
#endif
	);
	PackModel()
	{
		TAG_THIS_CLASS;
	}
	PackModel(const PackModel &) = delete;
	const PackModel &operator=(const PackModel &) = delete;

	PackModel(PackModel &&rhs)
	{
		TAG_THIS_CLASS;
		*this = std::move(rhs);
	}

	virtual ~PackModel();

	const PackModel &operator=(PackModel &&rhs)
	{
		m_name						= std::move(rhs.m_name); rhs.m_name = "";
		m_meshes					= std::move(rhs.m_meshes); rhs.m_meshes.clear();
		m_instances					= std::move(rhs.m_instances); rhs.m_instances.clear();
		m_animations				= std::move(rhs.m_animations); rhs.m_animations.clear();
		m_gpuInstanceUserDataArray	= std::move(rhs.m_gpuInstanceUserDataArray); rhs.m_gpuInstanceUserDataArray.clear();
		m_materialBuffer			= rhs.m_materialBuffer;
#if _SCE_TARGET_OS_PROSPERO
		rhs.m_materialBuffer.init();
#endif
#if _SCE_TARGET_OS_ORBIS
		rhs.m_materialBuffer.setBaseAddress(nullptr);
#endif
		m_bounds 					= rhs.m_bounds; rhs.m_bounds = {};
		m_packCpuMemory				= std::move(rhs.m_packCpuMemory); rhs.m_packCpuMemory.clear();
		m_packGpuMemory 			= std::move(rhs.m_packGpuMemory); rhs.m_packGpuMemory.clear();
		m_meshInstancesGpuMemory	= std::move(rhs.m_meshInstancesGpuMemory); rhs.m_meshInstancesGpuMemory.clear();
		m_materialsMemory			= std::move(rhs.m_materialsMemory); rhs.m_materialsMemory.reset(nullptr);
#if _SCE_TARGET_OS_PROSPERO
		m_allBvh					= std::move(rhs.m_allBvh); rhs.m_allBvh.clear();
		m_pAsyncLoader				= rhs.m_pAsyncLoader; rhs.m_pAsyncLoader = nullptr;
		m_packData					= rhs.m_packData; rhs.m_packData = {};
		m_loadedData				= std::move(rhs.m_loadedData); rhs.m_loadedData.clear();
		m_isResolved				= rhs.m_isResolved; rhs.m_isResolved = false;
#endif
		m_pTextureLibrary			= rhs.m_pTextureLibrary; rhs.m_pTextureLibrary = nullptr;

		return	*this;
	}

#if _SCE_TARGET_OS_PROSPERO
	static const sce::Agc::Shader		*getDefaultVertexShader(const PackModel &packModel, const sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags	&validVertexAttributes = sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags(true, true, true, 2, true));
	static const sce::Agc::Shader		*getDefaultPixelShader(const PackModel	&packModel, const sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags &validVertexAttributes = sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags(true, true, true, 2, true));
#endif
#if _SCE_TARGET_OS_ORBIS
	static const sce::SampleUtil::Graphics::Shader<sce::Gnmx::VsShader>	*getDefaultVertexShader(const PackModel	&packModel);
	static const sce::SampleUtil::Graphics::Shader<sce::Gnmx::PsShader>	*getDefaultPixelShader(const PackModel	&packModel);
#endif
}; // PackModel

/*!
 * @~English
 * @brief Pack Scene instance
 * @details This contains unique infomations needed to animate or draw.
 * @~Japanese
 * @brief PackModelインスタンス
 * @details PackModelをアニメーションさせたり、描画したりする際にインスタンスごとに持つ必要がある情報を保持
 */
struct PackModelInstance
{
	PackModel 													*m_pPackModel;
	std::string													m_name;
	std::vector<int>											m_drawOrder;
	Memory::Gpu::unique_ptr<Compat::OcclusionQueryResults[]>	m_occlusionQueryResults;
	Vectormath::Simd::Aos::Transform3							m_modelMatrix;
	bool														m_isVisible;
	std::vector<SkeletalAnimationInstance>						m_animInstances;
	Compat::Buffer												*m_pSkinningMatrixBuffers;

	virtual void						*getIndexBuffer(int	meshIndex) { return m_pPackModel->m_meshes[meshIndex].m_pIndexBuffer; }
	virtual uint32_t					getIndexElementSizeInBytes(int	meshIndex) { return m_pPackModel->m_meshes[meshIndex].m_indexElementSizeInBytes; }
	virtual uint32_t					getIndexCount(int	meshIndex)
	{
		const auto	&mesh	= m_pPackModel->m_meshes[meshIndex];
		return	mesh.m_indexSizeInBytes / mesh.m_indexElementSizeInBytes;
	}
	virtual sce::Geometry::Aos::Bounds	&getBounds() { return	m_pPackModel->m_bounds; }

	/*!
	 * @~English
	 * @brief Constructor
	 * @param packModel Source pack scene from which instance is to be crated
	 * @param cpuMemory CPU memory allocator. This is not needed unless animation is contained in model(nullptr can be specified)
	 * @param gpuMemory Video memory allocator used to allocate Occlusion Query memory. This is not needed unless Occlusion Query is used(nullptr can be specified)
	 * @param modelMatrix Initial model matrix
	 * @param name Instance name
	 * @~Japanese
	 * @brief コンストラクタ
	 * @param packModel インスタンスを作成する元のPackModel
	 * @param cpuMemory CPUメモリアロケータ。アニメーションがない場合は不要(nullptr指定可能)
	 * @param gpuMemory ビデオメモリアロケータ。Occlusion Query用メモリ確保に使用。Occlusion Queryを使用しない場合は不要(nullptr指定可能)
	 * @param modelMatrix 初期モデル行列
	 * @param name インスタンス名
	 */
	PackModelInstance(PackModel	&packModel, SceLibcMspace	cpuMemory, VideoAllocator	*gpuMemory, sce::Vectormath::Simd::Aos::Transform3_arg	modelMatrix = sce::Vectormath::Simd::Aos::Transform3::identity(), const std::string	&name = "");
	PackModelInstance()
	{
		TAG_THIS_CLASS;
	}
	PackModelInstance(const PackModelInstance &) = delete;
	const PackModelInstance &operator=(const PackModelInstance &) = delete;
	PackModelInstance(PackModelInstance &&rhs)
	{
		TAG_THIS_CLASS;
		*this = std::move(rhs);
	}
	virtual ~PackModelInstance()
	{
		UNTAG_THIS_CLASS;
	}
	const PackModelInstance &operator=(PackModelInstance &&rhs)
	{
		m_pPackModel				= rhs.m_pPackModel; rhs.m_pPackModel = nullptr;
		m_name 						= std::move(rhs.m_name);
		m_drawOrder					= std::move(rhs.m_drawOrder);
		m_occlusionQueryResults		= std::move(rhs.m_occlusionQueryResults);
		m_modelMatrix				= rhs.m_modelMatrix; rhs.m_modelMatrix = Vectormath::Simd::Aos::Transform3::identity();
		m_isVisible					= rhs.m_isVisible; rhs.m_isVisible = false;
		m_animInstances				= std::move(rhs.m_animInstances); rhs.m_animInstances.clear();
		m_pSkinningMatrixBuffers	= rhs.m_pSkinningMatrixBuffers; rhs.m_pSkinningMatrixBuffers = nullptr;

		return *this;
	}
};

/*!
 * @~English
 * @brief PackModel draw context
 * @details Manages draw states which need to be maintaines between multiple pack scene draws
 * @~Japanese
 * @brief PackModel描画コンテキスト
 * @details 複数のPackModel描画間で維持する必要がある描画ステートの管理を行う
 */
struct PackModelDrawContext
{
	/*!
	 * @~English
	 * @brief Rendering mode
	 * @~Japanese
	 * @brief レンダリングモード
	 */
	enum class RenderMode : uint32_t
	{
		/*!
		 * @~English
		 * @brief Normal rendering
		 * @~Japanese
		 * @brief 通常レンダリング
		 */
		kNormal,
		/*!
		 * @~English
		 * @brief Executes occlusion query
		 * @~Japanese
		 * @brief オクルージョンクエリを実行
		 */
		kOcclusionQuery,
		 /*!
		  * @~English
		  * @brief Executes conditional rendering
		  * @~Japanese
		  * @brief コンディショナルレンダリングを実行
		  */
		kConditional
	};

	RenderMode													m_renderMode;
	bool														m_prevIsWireframe;
	bool														m_isMirrorImage = false;
	uint32_t													m_indexElementSize;
	Compat::PrimitiveType										m_primitiveType;
	uint32_t													m_numInstances;
	sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags		m_validVertexAttributeFlags;

#if _SCE_TARGET_OS_PROSPERO
	DeferredSrtPatchBinder<sce::Agc::Core::LegacyBinder>		m_binder;
	const sce::Agc::Shader										*m_ppBoundShaders[(int)sce::Agc::ShaderType::kLegacyCount];
	uint32_t													m_activeStages;

	sce::Agc::Core::InternalGlobalTable							*m_pGlobalTablePtr;
	bool														m_isGlobalTableSet;
	sce::Agc::CxPrimitiveSetup::CullFace						m_forceCullFace;

	// previous states
	const sce::Agc::Shader										*m_pPrevPsShader;
	sce::Agc::CxPrimitiveSetup::FrontFace						m_prevFrontFace;
	sce::Agc::CxPrimitiveSetup::CullFace						m_prevCullFace;

	/*!
	 * @~English
	 * @brief Begin draw
	 * @details Call this before drawing series of pack scenes
	 * @param dcb DrawCommandBuffer
	 * @param rb RingBuffer
	 * @param validVertexAttributes Valid vertex attributes in Mesh
	 * @~Japanese
	 * @brief 描画開始
	 * @details 一連のPackModel描画を行う前にこの関数を呼びます。
	 * @param dcb DrawCommandBuffer
	 * @param rb RingBuffer
	 * @param validVertexAttributes Mesh内の有効な頂点属性
	 */
	void	beginDraw(sce::Agc::DrawCommandBuffer &dcb, sce::Agc::Core::RingBuffer *rb = nullptr, sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags	validVertexAttributes = { 1,1,1,2,1 });
	/*!
	 * @~English
	 * @brief Sets LS/HS shaders
	 * @param sb StateBuffer to be used for draw
	 * @param pLs LS shader to be set
	 * @param pHs HS shader to be set
	 * @param pFusedShader Fused shader of HS and LS
	 * @retval >=SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief LS/HSシェーダの設定
	 * @param sb 描画に使用するStateBuffer
	 * @param pLs 設定するLSシェーダ
	 * @param pHs 設定するHSシェーダ
	 * @param pFusedShader HS/LSをヒューズしたシェーダ
	 * @retval  SCE_OK 成功。
	 * @retval (<0) エラーコード
	 */
	int		setLsHsShader(sce::Agc::Core::StateBuffer &sb, const sce::Agc::Shader *pLs, const sce::Agc::Shader *pHs, const sce::Agc::Shader *pFusedShader);
	/*!
	 * @~English
	 * @brief Sets ES/GS shaders
	 * @param sb StateBuffer to be used for draw
	 * @param pEs ES shader to be set
	 * @param pGs GS shader to be set
	 * @param pFusedShader Fused shader of ES and GS
	 * @retval >=SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief ES/GSシェーダの設定
	 * @param sb 描画に使用するStateBuffer
	 * @param pEs 設定するESシェーダ
	 * @param pGs 設定するGSシェーダ
	 * @param pFusedShader ES/GSをヒューズしたシェーダ
	 * @retval  SCE_OK 成功。
	 * @retval (<0) エラーコード
	 */
	int		setEsGsShader(sce::Agc::Core::StateBuffer &sb, const sce::Agc::Shader *pEs, const sce::Agc::Shader *pGs, const sce::Agc::Shader *pFusedShader);
	/*!
	 * @~English
	 * @brief Sets VS shader
	 * @param sb StateBuffer to be used for draw
	 * @param pVs VS shader to be set
	 * @retval >=SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief VSシェーダの設定
	 * @param sb 描画に使用するStateBuffer
	 * @param pVs 設定するVSシェーダ
	 * @retval  SCE_OK 成功。
	 * @retval (<0) エラーコード
	 */
	int		setVsShader(sce::Agc::Core::StateBuffer &sb, const sce::Agc::Shader *pVs);

	/*!
	 * @~English
	 * @brief Mesh draw
	 * @param dcb DrawCommandBuffer to be used to draw model
	 * @param sb StateBuffer to be used to draw pack scene
	 * @param psShaders Array of pixel shaders to be used. It has following two flavors of shaders in it.
	 * -# alpha test disabled
	 * -# alpha test enabled
	 * .
	 * @param pCustomUserData Custom user data specified to pixel shader. Use it if you want to use custom user data for your pixel shader.
	 * @param isWireframe Specify if wireframe draw is enabled.
	 * @param PackModel PackModel to be drawn
	 * @param numPackModelInstances The number of PackModel instances
	 * @param meshIndex The index of Mesh to be drawn
	 * @param instanceIndex The index of instance to be drawn
	 * @param useInstancing true for instancing draw
	 * @param pIndexBuffer Index buffer for Mesh
	 * @param indexElementSizeInBytes Index element size(bytes)
	 * @param indexCount Index count for Mesh
	 * @retval >=SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief Meshの描画
	 * @param dcb 描画に使用するDrawCommandBuffer
	 * @param sb 描画に使用するStateBuffer
	 * @param psShaders 描画に使用するpixel shaderの配列
	 * -# アルファテストなし
	 * -# アルファテストあり
	 * .
	 * @param pCustomUserData pixel shaderに指定するカスタムユーザデータ。独自のユーザデータを渡したい場合に使用する。
	 * @param isWireframe ワイヤーフレーム描画を行うかどうかを指定
	 * @param PackModel 描画するPackModel
	 * @param pPackkModelInstanceUserData PackModelインスタンスのシェーダーUserData
	 * @param numPackModelInstances PackModelインスタンス数
	 * @param meshIndex 描画するMeshのindex
	 * @param instanceIndex 描画するインスタンスのindex
	 * @param useInstancing インスタンシング描画の時はtrue
	 * @param pIndexBuffer Meshのindex buffer
	 * @param indexElementSizeInBytes Meshのindexエレメントサイズ(バイト)
	 * @param indexCount Meshのindex数
	 * @retval  SCE_OK 成功。
	 * @retval (<0) エラーコード
	 */
	sce::Agc::Toolkit::Result	drawMesh(sce::Agc::DrawCommandBuffer	&dcb, sce::Agc::Core::StateBuffer	&sb, const std::array<const sce::Agc::Shader *, 2>	&psShaders, const void	*pCustomUserData, bool	isWireframe,
											const PackModel &packModel, const PackModelInstanceUserData	*pPackModelInstanceUserData, uint32_t	numPackModelInstances, uint32_t	meshIndex, uint32_t	instanceIndex, bool	useInstancing, const void	*pIndexBuffer, uint32_t	indexElementSizeInBytes, uint32_t	indexCount);

	/*!
	 * @~English
	 * @brief PackModel draw
	 * @details Draws a specified pack scene instance
	 * @param dcb DrawCommandBuffer to be used to draw model
	 * @param sb StateBuffer to be used to draw pack scene
	 * @param restoreCxState Specify whether CxState is restored after draw
	 * @param PackModelInstance PackModel instance to be drawn
	 * @param psShaders Array of pixel shaders to be used. It has following two flavors of shaders in it.
	 * -# alpha test disabled
	 * -# alpha test enabled
	 * .
	 * @param pCustomUserData Custom user data specified to pixel shader. Use it if you want to use custom user data for your pixel shader.
	 * @param isWireframe Specify if wireframe draw is enabled.
	 * @return  Error code and states altered after draw
	 * @~Japanese
	 * @brief PackModel描画
	 * @details 指定した一つのPackModelインスタンスを描画
	 * @param dcb 描画に使用するDrawCommandBuffer
	 * @param sb 描画に使用するStateBuffer
	 * @param restoreCxState 描画中に変更したCxステートを復帰するかどうかを指定
	 * @param PackModelInstance 描画するPackModelインスタンス
	 * @param psShaders 描画に使用するpixel shaderの配列
	 * -# アルファテストなし
	 * -# アルファテストあり
	 * .
	 * @param pCustomUserData pixel shaderに指定するカスタムユーザデータ。独自のユーザデータを渡したい場合に使用する。
	 * @param isWireframe ワイヤーフレーム描画を行うかどうかを指定
	 * @return  エラーコード、関数内で変更されたステート
	 */
	sce::Agc::Toolkit::Result		draw(sce::Agc::DrawCommandBuffer	&dcb, sce::Agc::Core::StateBuffer	&sb, sce::Agc::Toolkit::RestoreCxState	restoreCxState,
											PackModelInstance	&packModelInstance, const std::array<const sce::Agc::Shader *, 2>	&psShaders, const void	*pCustomUserData, bool	useInstancing = false, bool	isWireframe = false);

	/*!
	 * @~English
	 * @brief PackModel multiple instances draw
	 * @details Draws a specified pack model for specified number of instances
	 * @param dcb DrawCommandBuffer to be used to draw model
	 * @param sb StateBuffer to be used to draw pack scene
	 * @param restoreCxState Specify whether CxState is restored after draw
	 * @param packModel PackModel to be drawn
	 * @param pPackModelInstancingUserData PackModel instance shader user data
	 * @param numInstances Num of instances
	 * @param psShaders Array of pixel shaders to be used. It has following two flavors of shaders in it.
	 * -# alpha test disabled
	 * -# alpha test enabled
	 * .
	 * @param pCustomUserData Custom user data specified to pixel shader. Use it if you want to use custom user data for your pixel shader.
	 * @param useInstancing true for instancing draw
	 * @param isWireframe Specify if wireframe draw is enabled.
	 * @return  Error code and states altered after draw
	 * @~Japanese
	 * @brief packModelの複数インスタンス描画
	 * @details 指定した一つのPackModelを指定したインスタンシング数分描画
	 * @param dcb 描画に使用するDrawCommandBuffer
	 * @param sb 描画に使用するStateBuffer
	 * @param restoreCxState 描画中に変更したCxステートを復帰するかどうかを指定
	 * @param PackModel 描画するPackModel
	 * @param pPackModelInstancingUserData PackModelインスタンスのシェーダーUserData
	 * @param numInstances インスタンス数
	 * @param psShaders 描画に使用するpixel shaderの配列
	 * -# アルファテストなし
	 * -# アルファテストあり
	 * .
	 * @param pCustomUserData pixel shaderに指定するカスタムユーザデータ。独自のユーザデータを渡したい場合に使用する。
	 * @param useInstancing インスタンシング描画の時はtrue
	 * @param isWireframe ワイヤーフレーム描画を行うかどうかを指定
	 * @return  エラーコード、関数内で変更されたステート
	 */
	sce::Agc::Toolkit::Result		drawInstanced(sce::Agc::DrawCommandBuffer	&dcb, sce::Agc::Core::StateBuffer	&sb, sce::Agc::Toolkit::RestoreCxState	restoreCxState,
													PackModel	&packModel, const PackModelInstanceUserData	*pPackModelInstanceUserData, uint32_t	numInstances, const std::array<const sce::Agc::Shader *, 2>	&psShaders, const void	*pCustomUserData, bool	useInstancing = false, bool	isWireframe = false);
	/*!
	 * @~English
	 * @brief End draw
	 * @details Call this after drawing series of PackDraws
	 * @param dcb DrawCommandBuffer to be used to draw PackModels
	 * @~Japanese
	 * @brief 描画終了
	 * @details 一連のPackModel描画が終わった後にこの関数を呼びます。
	 * @param dcb 描画に使用するDrawCommandBuffer
	 */
	void	endDraw(sce::Agc::DrawCommandBuffer &dcb);
#endif
#if _SCE_TARGET_OS_ORBIS
	Gnm::PrimitiveSetupCullFaceMode								m_forceCullFace;

	// previous states
	const Shader<sce::Gnmx::PsShader>							*m_pPrevPsShader;
	Gnm::PrimitiveSetupFrontFace								m_prevFrontFace;
	Gnm::PrimitiveSetupCullFaceMode								m_prevCullFace;

	void	beginDraw(sce::Gnmx::GnmxGfxContext	&gfxc, sce::SampleUtil::Graphics::Mesh::VertexAttributeFlags	validVertexAttributes = { 1,1,1,1,1 });
	int		drawMesh(sce::Gnmx::GnmxGfxContext	&gfxc, const std::array<const Shader<sce::Gnmx::PsShader> *, 2>	&psShaders, const void	*pCustomUserData, bool	isWireframe,
						const PackModel	&packModel, PackModelInstanceUserData	*pPackModelInstanceUserData, uint32_t	numPackModelInstances, uint32_t	meshIndex, uint32_t	instanceIndex, bool	useInstancing, const void	*pIndexBuffer, uint32_t	indexElementSizeInBytes, uint32_t	indexCount);
	int		draw(sce::Gnmx::GnmxGfxContext	&gfxc, PackModelInstance	&packModelInstance, const std::array<const Shader<sce::Gnmx::PsShader> *, 2>	&psShaders, const void	*pCustomUserData, bool	useInstancing = false, bool	isWireframe = false);
	void	endDraw(sce::Gnmx::GnmxGfxContext	&gfxc);
#endif
};

/*!
* @~English
* @brief Initializes pack scene draw
* @details Initializes default vertex/pixel shaders for pack scene draw
* @~Japanese
* @brief PackModel描画の初期化
* @details PackModel描画用のデフォルトシェーダを初期化します
*/
void	initializePackModelDraw(
#if _SCE_TARGET_OS_ORBIS
	sce::SampleUtil::Graphics::VideoAllocator *allocator
#endif
);

/*!
* @~English
* @brief Finalizes pack scene draw
* @details Finalizes default vertex/pixel shaders for pack scene draw
* @~Japanese
* @brief PackModel描画の終了処理
* @details PackModel描画用のデフォルトシェーダを解放します。
*/
void	finalizePackModelDraw();

#if _SCE_TARGET_OS_PROSPERO
sce::Agc::Toolkit::Result	drawPackModelInternal(sce::Agc::DrawCommandBuffer	*dcb, sce::Agc::Core::StateBuffer	*sb, sce::Agc::Toolkit::RestoreCxState	restoreCxState, sce::Agc::Core::RingBuffer	*rb,
													PackModel	&packModel, SceLibcMspace	cpuMemory, VideoRingAllocator	&ringAlloc, const float	*pAnimationTimesInSec,
													const sce::Vectormath::Simd::Aos::Matrix4	*pModelMatrices, uint32_t	numInstances, sce::Vectormath::Simd::Aos::Matrix4_arg	viewMatrix, sce::Vectormath::Simd::Aos::Matrix4_arg	projectionMatrix, sce::Vectormath::Simd::Aos::Vector3_arg	lightPosition,
													float	ambient, float	shininess, const sce::Agc::Shader	*vs, const sce::Agc::Shader	*fillPs, const void	*fillUserData, const std::vector<JointTransformDesignation>	*pAuxTransforms, bool	drawSkeleton);
#endif
#if _SCE_TARGET_OS_ORBIS
int	drawPackModelInternal(sce::Gnmx::GnmxGfxContext	*gfxc, PackModel	&packModel, SceLibcMspace	cpuMemory, VideoRingAllocator	&ringAlloc, float	animationTimeInSec,
							sce::Vectormath::Simd::Aos::Matrix4_arg	modelMatrix, sce::Vectormath::Simd::Aos::Matrix4_arg	viewMatrix, sce::Vectormath::Simd::Aos::Matrix4_arg	projectionMatrix, sce::Vectormath::Simd::Aos::Vector3_arg	lightPosition,
							float	ambient, float	shininess, const Shader<sce::Gnmx::VsShader>	*vs, const Shader<sce::Gnmx::PsShader>	*fillPs, const void	*fillUserData, const std::vector<JointTransformDesignation>	&auxTransforms, bool	drawSkeleton);
#endif

/*!
 * @~English
 * @brief Draws a pack scene
 * @param dcb Pointer to DrawCommandBuffer
 * @param sb Pointer to StateBuffer
 * @param restoreCxState Specify whether Cx states are restored after call
 * @param packModel pack scene to draw
 * @param videoRingMemoryAlloc User Data memory is allocated from this allocator
 * @param modelMatrix Transformation matrix for world coordinate system
 * @param viewMatrix View matrix
 * @param projectionMatrix Projection matrix
 * @param lightPosition Point light position
 * @param ambient Ambient value for phone shader
 * @param shininess Shininess of phone shader
 * @param fillPs pixel shader used for drawing
 * @param fillPsUserData User Data bound to fillPs
 *
 * @return sce::Agc::Toolkit::Result
 * @details This draws a pack scene without animation.
 * @~Japanese
 * @brief PackModelの描画
 * @param dcb DrawCommandBufferへのポインタ
 * @param sb StateBufferへのポインタ
 * @param restoreCxState 呼出し後にCxステートを復元するかを指定
 * @param packModel 描画するPackModel
 * @param videoRingMemoryAlloc User Data領域がここから確保されます
 * @param modelMatrix world座標系への変換行列
 * @param viewMatrix 視点行列
 * @param projectionMatrix 射影行列
 * @param lightPosition ポイントライトの位置
 * @param ambient phoneシェーダのアンビエント値
 * @param shininess phoneシェーダの輝度
 * @param fillPs 描画に使用するpixel shader
 * @param fillPsUserData fillPsに設定するUser Data
 *
 * @return sce::Agc::Toolkit::Result
 * @details アニメーション無しPackModelを描画します。
 */
#if _SCE_TARGET_OS_PROSPERO
static inline sce::Agc::Toolkit::Result	drawPackModel(sce::Agc::DrawCommandBuffer	*dcb, sce::Agc::Core::StateBuffer	*sb, sce::Agc::Toolkit::RestoreCxState	restoreCxState, sce::Agc::Core::RingBuffer	*rb, PackModel	&packModel, VideoRingAllocator	&videoRingMemoryAlloc,
														sce::Vectormath::Simd::Aos::Matrix4_arg	modelMatrix, sce::Vectormath::Simd::Aos::Matrix4_arg	viewMatrix, sce::Vectormath::Simd::Aos::Matrix4_arg	projectionMatrix, sce::Vectormath::Simd::Aos::Vector3_arg	lightPosition, float	ambient, float	shininess,
														const sce::Agc::Shader	*fillPs = nullptr, const void	*fillPsUserData = nullptr)
{
	return	drawPackModelInternal(dcb, sb, restoreCxState, rb, packModel, nullptr, videoRingMemoryAlloc, nullptr, &modelMatrix, 1, viewMatrix, projectionMatrix, lightPosition, ambient, shininess, nullptr, fillPs, fillPsUserData, nullptr, false);
}

static inline sce::Agc::Toolkit::Result	drawPackModelInstanced(sce::Agc::DrawCommandBuffer	*dcb, sce::Agc::Core::StateBuffer	*sb, sce::Agc::Toolkit::RestoreCxState	restoreCxState, sce::Agc::Core::RingBuffer	*rb, PackModel	&packModel, VideoRingAllocator	&videoRingMemoryAlloc,
														const sce::Vectormath::Simd::Aos::Matrix4	*pModelMatrices, uint32_t	numInstances, sce::Vectormath::Simd::Aos::Matrix4_arg	viewMatrix, sce::Vectormath::Simd::Aos::Matrix4_arg	projectionMatrix, sce::Vectormath::Simd::Aos::Vector3_arg	lightPosition, float	ambient, float	shininess,
														const sce::Agc::Shader	*fillPs = nullptr, const void	*fillPsUserData = nullptr)
{
	return	drawPackModelInternal(dcb, sb, restoreCxState, rb, packModel, nullptr, videoRingMemoryAlloc, nullptr, pModelMatrices, numInstances, viewMatrix, projectionMatrix, lightPosition, ambient, shininess, nullptr, fillPs, fillPsUserData, nullptr, false);
}
#endif
#if _SCE_TARGET_OS_ORBIS
static inline int	drawPackModel(sce::Gnmx::GnmxGfxContext	*gfxc, PackModel	&packModel, VideoRingAllocator	&videoRingMemoryAlloc,
									sce::Vectormath::Simd::Aos::Matrix4_arg	modelMatrix, sce::Vectormath::Simd::Aos::Matrix4_arg	viewMatrix, sce::Vectormath::Simd::Aos::Matrix4_arg	projectionMatrix, sce::Vectormath::Simd::Aos::Vector3_arg	lightPosition, float	ambient, float	shininess,
									const Shader<sce::Gnmx::PsShader>	*fillPs = nullptr, const void	*fillUserData = nullptr)
{
	return	drawPackModelInternal(gfxc, packModel, nullptr, videoRingMemoryAlloc, 0.f, modelMatrix, viewMatrix, projectionMatrix, lightPosition, ambient, shininess, nullptr, fillPs, fillUserData, {}, false);
}
#endif

/*!
 * @~English
 * @brief Draws a pack scene
 * @param dcb Pointer to DrawCommandBuffer
 * @param sb Pointer to StateBuffer
 * @param restoreCxState Specify whether Cx states are restored after call
 * @param rb Pointer to RingBuffer
 * @param packModel pack scene to draw
 * @param edgeAnimMemoryAlloc Memory allocator used to allocate memory for edgeAnim(Allocated memory is accessed only by CPU)
 * @param videoRingMemoryAlloc Memory allocator used to allocate memory for skinning matrices and User Data region(Allocated memory is accessed by both CPU and GPU)
 * @param animationTimeInSec time in second to evaluate animation
 * @param modelMatrix Transformation matrix for world coordinate system
 * @param viewMatrix View matrix
 * @param projectionMatrix Projection matrix
 * @param lightPosition Point light position
 * @param ambient Ambient value for phone shader
 * @param shininess Shininess of phone shader
 * @param fillPs pixel shader used for drawing
 * @param fillPsUserData User Data bound to fillPs
 * @param auxRotations Auxiliary joint rotation designations
 * @param drawSkeleton draw skeleton
 *
 * @return sce::Agc::Toolkit::Result
 * @details This draws a animated pack scene.
 * @~Japanese
 * @brief PackModelの描画
 * @param dcb DrawCommandBufferへのポインタ
 * @param sb StateBufferへのポインタ
 * @param restoreCxState 呼出し後にCxステートを復元するかを指定
 * @param rb RingBufferへのポインタ
 * @param packModel 描画するPackModel
 * @param edgeAnimMemoryAlloc edgeAnim用メモリを確保するためのメモリアロケータ(CPU用メモリを確保)
 * @param videoRingMemoryAlloc スキニング行列用メモリやUser Data領域を確保するためのメモリアロケータ(CPU/GPU用メモリを確保)
 * @param animationTimeInSec アニメーションを評価する時刻(秒)
 * @param modelMatrix world座標系への変換行列
 * @param viewMatrix 視点行列
 * @param projectionMatrix 射影行列
 * @param lightPosition ポイントライトの位置
 * @param ambient phoneシェーダのアンビエント値
 * @param shininess phoneシェーダの輝度
 * @param fillPs 描画に使用するpixel shader
 * @param fillPsUserData fillPsに設定するUser Data
 * @param auxTransforms 追加のジョイント変換指示
 * @param drawSkeleton スケルトンを表示
 *
 * @return sce::Agc::Toolkit::Result
 * @details アニメーション付きPackModelを描画します。
 */
#if _SCE_TARGET_OS_PROSPERO
static inline	sce::Agc::Toolkit::Result	drawAnimatedPackModel(sce::Agc::DrawCommandBuffer	*dcb, sce::Agc::Core::StateBuffer	*sb, sce::Agc::Toolkit::RestoreCxState	restoreCxState, sce::Agc::Core::RingBuffer	*rb,
																PackModel	&packModel, SceLibcMspace	edgeAnimMemoryAlloc, VideoRingAllocator	&videoRingMemoryAlloc, float	animationTimeInSec,
																sce::Vectormath::Simd::Aos::Matrix4_arg	modelMatrix, sce::Vectormath::Simd::Aos::Matrix4_arg	viewMatrix, sce::Vectormath::Simd::Aos::Matrix4_arg	projectionMatrix, sce::Vectormath::Simd::Aos::Vector3_arg	lightPosition, float	ambient, float	shininess,
																const sce::Agc::Shader	*fillPs = nullptr, const void	*fillPsUserData = nullptr, const std::vector<JointTransformDesignation>	&auxTransforms = {}, bool	drawSkeleton = false)
{
	return	drawPackModelInternal(dcb, sb, restoreCxState, rb, packModel, edgeAnimMemoryAlloc, videoRingMemoryAlloc, &animationTimeInSec, &modelMatrix, 1, viewMatrix, projectionMatrix, lightPosition, ambient, shininess, nullptr, fillPs, fillPsUserData, &auxTransforms, drawSkeleton);
}

static inline	sce::Agc::Toolkit::Result	drawAnimatedPackModelInstanced(sce::Agc::DrawCommandBuffer	*dcb, sce::Agc::Core::StateBuffer	*sb, sce::Agc::Toolkit::RestoreCxState	restoreCxState, sce::Agc::Core::RingBuffer	*rb,
																PackModel	&packModel, SceLibcMspace	edgeAnimMemoryAlloc, VideoRingAllocator	&videoRingMemoryAlloc, const float	*pAnimationTimesInSec,
																const sce::Vectormath::Simd::Aos::Matrix4	*pModelMatrices, uint32_t	numInstances, sce::Vectormath::Simd::Aos::Matrix4_arg	viewMatrix, sce::Vectormath::Simd::Aos::Matrix4_arg	projectionMatrix, sce::Vectormath::Simd::Aos::Vector3_arg	lightPosition, float	ambient, float	shininess,
																const sce::Agc::Shader	*fillPs = nullptr, const void	*fillPsUserData = nullptr, const std::vector<JointTransformDesignation>	*pAuxTransforms = nullptr, bool	drawSkeleton = false)
{
	return	drawPackModelInternal(dcb, sb, restoreCxState, rb, packModel, edgeAnimMemoryAlloc, videoRingMemoryAlloc, pAnimationTimesInSec, pModelMatrices, numInstances, viewMatrix, projectionMatrix, lightPosition, ambient, shininess, nullptr, fillPs, fillPsUserData, pAuxTransforms, drawSkeleton);
}
#endif
#if _SCE_TARGET_OS_ORBIS
static inline int	drawAnimatedPackModel(sce::Gnmx::GnmxGfxContext	*gfxc, PackModel	&packModel, SceLibcMspace	edgeAnimMemoryAlloc, VideoRingAllocator	&videoRingMemoryAlloc, float	animationTimeInSec,
											sce::Vectormath::Simd::Aos::Matrix4_arg	modelMatrix, sce::Vectormath::Simd::Aos::Matrix4_arg	viewMatrix, sce::Vectormath::Simd::Aos::Matrix4_arg	projectionMatrix, sce::Vectormath::Simd::Aos::Vector3_arg	lightPosition, float	ambient, float	shininess,
											const Shader<sce::Gnmx::PsShader>	*fillPs = nullptr, const void	*fillUserData = nullptr, const std::vector<JointTransformDesignation>	&auxTransforms = {}, bool	drawSkeleton = false)
{
	return	drawPackModelInternal(gfxc, packModel, edgeAnimMemoryAlloc, videoRingMemoryAlloc, animationTimeInSec, modelMatrix, viewMatrix, projectionMatrix, lightPosition, ambient, shininess, nullptr, fillPs, fillUserData, auxTransforms, drawSkeleton);
}
#endif
}}} // namespace sce::SampleUtil::Graphics

namespace std
{
	template<>
	struct hash<sce::SampleUtil::Graphics::PixelShaderFlags>
	{
		std::size_t	operator()(const sce::SampleUtil::Graphics::PixelShaderFlags &data) const
		{
			return	((std::size_t)data.m_shadingModel) | (((std::size_t)data.m_hasNormal) << 4) | (((std::size_t)data.m_hasTangent) << 5) | (((std::size_t)data.m_hasColor) << 6) |	(((std::size_t)data.m_uvCount) << 7) | (((std::size_t)data.m_hasAlphaTest) << 9);
		}
	};
}
