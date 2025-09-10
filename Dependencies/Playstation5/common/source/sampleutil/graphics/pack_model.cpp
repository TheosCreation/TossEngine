/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc.
 * 
 */

#include <fstream>
#include <scebase_common.h>
 // SampleUtil
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/sampleutil_common.h>
#include <pack/pack_file.h>
#if _SCE_TARGET_OS_PROSPERO
#include <pack/ext/ext_bvh.h>
#endif
#include <pack/ext/ext_edge_anim.h>
#include <sampleutil/memory.h>
#include <sampleutil/graphics/pack_model.h>
#include <sampleutil/graphics/graphics_memory.h>
#if _SCE_TARGET_OS_PROSPERO
#ifdef _DEBUG
#pragma comment(lib, "libSceDbg_nosubmission_stub_weak.a")
#pragma comment(lib, "libScePsr_debug_nosubmission.a")
#else
#pragma comment(lib, "libScePsr.a")
#endif
#if SCE_DBG_ASSERTS_ENABLED
#pragma comment(lib, "libSceDbg_nosubmission_stub_weak.a")
#endif
#endif
#if _SCE_TARGET_OS_ORBIS
#if SCE_DBG_ASSERTS_ENABLED
#pragma comment(lib, "libSceDbg_stub_weak.a")
#endif
#endif

using namespace sce::Vectormath::Simd::Aos;

namespace {
	Matrix4	matrixPack(PackFile::Float4x4	M)
	{
		Matrix4	result;
		for (int i = 0; i < 4; i++) {
			result.setCol(i, Vector4(&M.values[i * 4]));
		}

		return	result;
	}

	Transform3	getWorldPose(PackFile::Package const	&pack, uint32_t	n)
	{
		PackFile::Node const *pNode = &pack.nodes[n];
		Matrix4 transform = matrixPack(pNode->matrix);
		while (pNode->parent_node_index >= 0) {
			pNode = &pack.nodes[pNode->parent_node_index];
			const Matrix4 nodeTransform = matrixPack(pNode->matrix);
			transform = nodeTransform * transform;
		}

		return	(Transform3)transform;
	}
	
	void	computeSceneBounds(sce::SampleUtil::Graphics::PackModel	&model)
	{
		model.m_bounds.setMinimum(Point3(INFINITY));
		model.m_bounds.setMaximum(Point3(-INFINITY));

		// Go thru each instance in the model
		// expand scene bounds accordingly
		for (auto &instance : model.m_instances) {
			model.m_bounds.extend(instance.m_bounds);
		}
	}

	sce::Geometry::Aos::Bounds  transformBounds(Transform3_arg	transform, sce::Geometry::Aos::Bounds_arg	bounds)
	{
		Point3	AMin, AMax;
		Point3	Min, Max;

		float	a, b;

		// Copy box A into min and max array.
		AMin = bounds.getMinimum();
		AMax = bounds.getMaximum();

		// Begin at transform.
		Min = Max = Point3::origin() + transform.getTranslation();

		// Find extreme points by considering product of 
		// min and max with each component of transform.
		for (int j = 0; j < 3; j++) {
			for (int i = 0; i < 3; i++) {
				a = transform.getElem(i, j) * AMin[i];
				b = transform.getElem(i, j) * AMax[i];
				if (a < b) {
					Min[j] += a;
					Max[j] += b;
				} else {
					Min[j] += b;
					Max[j] += a;
				}
			}
		}

		return	sce::Geometry::Aos::Bounds(Min, Max);
	}

#if _SCE_TARGET_OS_PROSPERO
	void	*getPtr(PackFile::Buffer const	&buffer, PackFile::Array<PackFile::DataMapping> const	&dataMapping)
	{
		const PackFile::Serialization::DataOffset	bufferDataOffset
		{
			.offset			= buffer.offset,
			.size			= (uint32_t)buffer.size,
			.mapping_idx	= buffer.data_section,
		};

		return	PackFile::Serialization::get_ptr(dataMapping, bufferDataOffset);
	}

	sce::Psr::Cpu::TriangleFormat	getPsrTriangleFormatCpu(uint8_t	stride)
	{
		switch (stride)
		{
		case 8:		return	sce::Psr::Cpu::TriangleFormat::kUint8x3;
		case 16:	return	sce::Psr::Cpu::TriangleFormat::kUint16x3;
		case 32:	return	sce::Psr::Cpu::TriangleFormat::kUint32x3;
		default:
			SCE_SAMPLE_UTIL_ASSERT(false);
		}

		return	sce::Psr::Cpu::TriangleFormat::kUint32x3;
	};

	void	decompressBvh(sce::SampleUtil::Graphics::PackModel	*pOutPackModel, PackFile::Package const	&pack, PackFile::BVHData const	&bvhData,
							sce::SampleUtil::Graphics::VideoAllocator	&gpuMemory)
	{
		sce::Psr::Status	status	= sce::Psr::Status::kSuccess; (void)status;

		const size_t	bvhCount	= bvhData.compressed_bvh_descriptors.size();
		SCE_SAMPLE_UTIL_ASSERT(pack.meshes.size() == bvhCount);

		pOutPackModel->m_allBvh.resize(bvhCount);
		for (size_t i = 0; i < bvhCount; ++i)
		{
			sce::Psr::Cpu::GeometryConfig	meshGeometry;
			{
				const PackFile::Mesh	&mesh			= pack.meshes[i];
				const PackFile::Buffer	&vertexBuffer	= pack.buffers[mesh.vertex_buffers[0]];
				const PackFile::Buffer	&indexBuffer	= pack.buffers[mesh.index_buffer];

				meshGeometry.init();
				meshGeometry.m_mesh.m_vertexData		= getPtr(vertexBuffer, pack.data_mapping);
				meshGeometry.m_mesh.m_triangleData		= getPtr(indexBuffer, pack.data_mapping);
				meshGeometry.m_mesh.m_vertexStride		= vertexBuffer.stride;
				meshGeometry.m_mesh.m_vertexCount		= vertexBuffer.elem_count;
				meshGeometry.m_mesh.m_triangleStride	= indexBuffer.stride * 3;
				meshGeometry.m_mesh.m_triangleCount		= indexBuffer.elem_count / 3;
				meshGeometry.m_mesh.m_vertexFormat		= sce::Psr::Cpu::VertexFormat::kFp32x3;
				meshGeometry.m_mesh.m_triangleFormat	= getPsrTriangleFormatCpu(mesh.index_elem_size);
			}
			size_t	decompressedBvhSize	= 0;
			status	= sce::Psr::getDecompressedBottomLevelBvhSize(&decompressedBvhSize, bvhData.compressed_bvh_descriptors[i], bvhData.decompression_mode);
			SCE_SAMPLE_UTIL_ASSERT(status == sce::Psr::Status::kSuccess);
			size_t	scratchSize	= 0;
			status	= sce::Psr::getBottomLevelBvhDecompressionScratchSize(&scratchSize, bvhData.compressed_bvh_descriptors[i], bvhData.decompression_mode);
			SCE_SAMPLE_UTIL_ASSERT(status == sce::Psr::Status::kSuccess);

			pOutPackModel->m_packGpuMemory.push_back(sce::SampleUtil::Memory::Gpu::make_unique<uint8_t>(decompressedBvhSize, sce::Psr::kRequiredAlignment,
				{ sce::Agc::ResourceRegistration::ResourceType::kBottomLevelBvhBaseAddress }, gpuMemory, (pOutPackModel->m_name + "#bvh" + std::to_string(i)).c_str()));
			void	*pBvhData	= pOutPackModel->m_packGpuMemory.back().get();
			void	*pScratch	= malloc(scratchSize);
			SCE_SAMPLE_UTIL_ASSERT(pScratch != nullptr);

			status	= sce::Psr::Cpu::decompressBottomLevelBvh(&pOutPackModel->m_allBvh[i], bvhData.compressed_bvh_descriptors[i], bvhData.decompression_mode,
				&meshGeometry, 1u, pBvhData, decompressedBvhSize, pScratch, scratchSize);
			SCE_SAMPLE_UTIL_ASSERT(status == sce::Psr::Status::kSuccess);

			free(pScratch);
		}
	}

	constexpr uint64_t	kGpuDataLoading = 0xffu;
#endif

	void	loadPackage(sce::SampleUtil::Graphics::PackModel	*pOutPackModel, void	*pPackFileContent, size_t	packFileSizeInBytes,
						sce::SampleUtil::Graphics::VideoAllocator	&gpuMemory, SceLibcMspace	cpuMemory, sce::SampleUtil::Graphics::TextureLibrary	&texLibrary,
						const std::string	&textureNamePrefix, const std::string	&packFilename)
	{
		int ret = SCE_OK; (void)ret;

		PackFile::Package	pack = {};
		PackFile::EdgeData	edgeData = {};
#if _SCE_TARGET_OS_PROSPERO
		PackFile::BVHData	bvhData = {};
#endif

		// parse loaded package file to pack, edgeData and bvhData
		PackFile::BufferFile	packMemStream((const uint8_t *)pPackFileContent, (const uint8_t*)pPackFileContent + packFileSizeInBytes);

		PackFile::LoaderOptions	options;
		options.resolve_names	= true;
		options.report_warning	= true;

		// setup allocate callbacks called for GPU data
		struct GpuAllocatorArg
		{
			sce::SampleUtil::Graphics::PackModel		*m_pModel;
			sce::SampleUtil::Graphics::VideoAllocator	&m_gpuMemory;
		} gpuAllocArg = { pOutPackModel, gpuMemory };
		options.gpu_allocator.alloc_cb = [](size_t size, size_t alignment, void *userData, uint32_t header) {
			auto	*pArg = reinterpret_cast<GpuAllocatorArg *>(userData);
			pArg->m_pModel->m_packGpuMemory.push_back(sce::SampleUtil::Memory::Gpu::make_unique<uint8_t>(size, alignment, pArg->m_gpuMemory));
			return	(void *)pArg->m_pModel->m_packGpuMemory.back().get();
		};
		options.gpu_allocator.dealloc_cb = [](void	*ptr, void	*userData, uint32_t	header) {
			auto	*pArg = reinterpret_cast<GpuAllocatorArg *>(userData);
			auto	found = std::find_if(pArg->m_pModel->m_packGpuMemory.begin(), pArg->m_pModel->m_packGpuMemory.end(), [&](const auto	&mem) { return	mem.get() == ptr; });
			SCE_SAMPLE_UTIL_ASSERT(found != pArg->m_pModel->m_packGpuMemory.end());
			pArg->m_pModel->m_packGpuMemory.erase(found);
		};
		options.gpu_allocator.user_data	= &gpuAllocArg;
		options.gpu_allocator.header	= 0u;

		// setup allocate callbacks called for Edge Animation data
		struct CpuAllocatorArg
		{
			sce::SampleUtil::Graphics::PackModel	*m_pModel;
			SceLibcMspace							m_cpuMemory;
		} cpuAllocArg = { pOutPackModel, cpuMemory };
		options.cpu_allocator.alloc_cb = [](size_t size, size_t	alignment, void	*userData, uint32_t	header) {
			auto	*pArg	= reinterpret_cast<CpuAllocatorArg*>(userData);
			pArg->m_pModel->m_packCpuMemory.push_back(sce::SampleUtil::Memory::Cpu::make_unique<uint8_t>(size, alignment, pArg->m_cpuMemory));
			return	(void*)pArg->m_pModel->m_packCpuMemory.back().get();
		};
		options.cpu_allocator.dealloc_cb = [](void *ptr, void	*userData, uint32_t	header) {
			auto	*pArg = reinterpret_cast<CpuAllocatorArg*>(userData);
			auto	found = std::find_if(pArg->m_pModel->m_packCpuMemory.begin(), pArg->m_pModel->m_packCpuMemory.end(), [&](const auto	&mem) { return	mem.get() == ptr; });
			SCE_SAMPLE_UTIL_ASSERT(found != pArg->m_pModel->m_packCpuMemory.end());
			pArg->m_pModel->m_packCpuMemory.erase(found);
		};
		options.cpu_allocator.user_data	= &cpuAllocArg;
		options.cpu_allocator.header	= 0u;
		// Edge Animation and BVH custom sections
		options.custom_section_loaders	= {
			PackFile::SectionLoader{ PackFile::k_edge_header, PackFile::load_edge_section, &edgeData },
#if _SCE_TARGET_OS_PROSPERO
			PackFile::SectionLoader{ PackFile::k_bvh_header, PackFile::load_bvh_section, &bvhData },
#endif
		};
		ret = PackFile::load_package(packMemStream, pack, options);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

#if _SCE_TARGET_OS_PROSPERO
		auto	*pLoader = pOutPackModel->m_pAsyncLoader;
#endif

		// create gpu data file path by replacing suffix(.pack) to .gpu
		SCE_SAMPLE_UTIL_ASSERT(packFilename.substr(packFilename.length() - 5) == ".pack");
		const std::string gpuFilename = packFilename.substr(0, packFilename.length() - 5) + ".gpu";
		std::ifstream	*pGpuDataIfs = nullptr;

		// allocate and load GPU data and Edge Animation data
		for (int i = 0; i < pack.data_mapping.size() && pack.gpu_data_size == 0; ++i) {
			uint32_t  memFlag = SCE_KERNEL_PROT_CPU_READ;
#if _SCE_TARGET_OS_PROSPERO
			if (pLoader != nullptr) {
				memFlag |= SCE_KERNEL_PROT_AMPR_WRITE;
			}
#endif
			if (pack.data_mapping[i].mapping_flags & PackFile::DataMapping::MappingHint::cpu_rw) {
				memFlag |= SCE_KERNEL_PROT_CPU_RW;
			}
			if (pack.data_mapping[i].mapping_flags & PackFile::DataMapping::MappingHint::gpu_read) {
				memFlag |= SCE_KERNEL_PROT_GPU_READ;
			}
			if (pack.data_mapping[i].mapping_flags & PackFile::DataMapping::MappingHint::gpu_write) {
				memFlag |= SCE_KERNEL_PROT_GPU_WRITE;
			}
			void	*pAllocMem = nullptr;
			const std::string tag = gpuFilename + "#" + std::to_string(i);
#if _SCE_TARGET_OS_PROSPERO
			if (pLoader != nullptr) {
				// allocate and load using AMPR
				const auto gpuData = pLoader->allocate(tag.c_str(), pack.data_mapping[i].size, 65536, SCE_KERNEL_MTYPE_C, memFlag);
				ret = pLoader->load(tag.c_str(), gpuFilename.c_str(), pack.data_mapping[i].data_offset, pack.data_mapping[i].size);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				pOutPackModel->m_loadedData.push_back(gpuData);
				pAllocMem = gpuData.m_ptr;
			} else
#endif
			{
				// allocate and load using normal memory allocator and File I/O
				if ((memFlag & SCE_KERNEL_PROT_GPU_RW) != 0u) {
					pOutPackModel->m_packGpuMemory.push_back(sce::SampleUtil::Memory::Gpu::make_unique<uint8_t>(pack.data_mapping[i].size, 65536, gpuMemory));
					pAllocMem = pOutPackModel->m_packGpuMemory.back().get();
				} else {
					pOutPackModel->m_packCpuMemory.push_back(sce::SampleUtil::Memory::Cpu::make_unique<uint8_t>(pack.data_mapping[i].size, 65536, cpuMemory));
					pAllocMem = pOutPackModel->m_packCpuMemory.back().get();
				}
				if (pGpuDataIfs == nullptr) {
					pGpuDataIfs = new std::ifstream(gpuFilename.c_str(), std::ios::binary | std::ios::in);
					SCE_SAMPLE_UTIL_ASSERT(pGpuDataIfs && pGpuDataIfs->good());
				}
				pGpuDataIfs->seekg(pack.data_mapping[i].data_offset, std::ios::beg);
				pGpuDataIfs->read((char *)pAllocMem, pack.data_mapping[i].size);
			}
			pack.data_mapping[i].data_offset = reinterpret_cast<uint64_t>(pAllocMem);
		}

#if _SCE_TARGET_OS_PROSPERO
		if (pLoader != nullptr) {
			pOutPackModel->m_packData.m_pLoadState->m_value = kGpuDataLoading;
			// wake APR listener up
			ret = sceKernelAddUserEventEdge(pLoader->m_aprListener.m_eventQueue, pLoader->m_aprListener.m_eqId);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		} else
#endif
		if (pGpuDataIfs != nullptr) {
			delete pGpuDataIfs;
		}

		// parse mesh, and fix-up vertex data addresses
		for (uint32_t i = 0; i < pack.meshes.getCount(); ++i) {
			if (pack.data_mapping.getCount() > 0) {
				pOutPackModel->m_meshes.emplace_back(pack.meshes[i], pack.buffers.data(), pack.data_mapping, gpuMemory);
			} else {
				pOutPackModel->m_meshes.emplace_back(pack.meshes[i], pack.buffers.data(), pack.gpu_data, gpuMemory);
			}
		}

#if _SCE_TARGET_OS_PROSPERO
		// parse and fix-up bvh data
		const size_t bvhCount = bvhData.descriptors.size() + bvhData.compressed_bvh_descriptors.size();
		if (bvhCount > 0) {
			if (pack.data_mapping.getCount() > 0) {
				ret = PackFile::patch_data(bvhData, pack.data_mapping);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			}

			if (bvhData.compressed_bvh_descriptors.size() > 0) {
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(bvhData.descriptors.size(), 0);
				decompressBvh(pOutPackModel, pack, bvhData, gpuMemory);
			} else {
				pOutPackModel->m_allBvh	= std::vector<sce::Psr::BottomLevelBvhDescriptor>(bvhData.descriptors.begin(), bvhData.descriptors.end());
			}
		}
#endif

		// parse and fix-up edge data
		if (edgeData.skeletons.getCount() > 0 && pack.data_mapping.getCount() > 0) {
			ret = PackFile::patch_data(edgeData, pack.data_mapping);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}

		// Load Materials
#if _SCE_TARGET_OS_PROSPERO
		pOutPackModel->m_materialsMemory = sce::SampleUtil::Memory::Gpu::make_unique<sce::SampleUtil::Graphics::ShaderMaterial>(pack.materials.getCount() + 1, 16,
			{ sce::Agc::ResourceRegistration::ResourceType::kSrt }, gpuMemory, "Shader Materials");
		ret = sce::Agc::Core::initialize(&pOutPackModel->m_materialBuffer, &sce::Agc::Core::BufferSpec()
			.initAsRegularBuffer(pOutPackModel->m_materialsMemory.get(), sizeof(sce::SampleUtil::Graphics::ShaderMaterial), pack.materials.getCount() + 1));
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
#endif
#if _SCE_TARGET_OS_ORBIS
		pOutPackModel->m_materialsMemory = sce::SampleUtil::Memory::Gpu::make_unique<sce::SampleUtil::Graphics::ShaderMaterial>(pack.materials.getCount() + 1, 16,
			{ sce::Gnm::kResourceTypeBufferBaseAddress }, gpuMemory, "Shader Materials");
		pOutPackModel->m_materialBuffer.initAsRegularBuffer(pOutPackModel->m_materialsMemory.get(), sizeof(sce::SampleUtil::Graphics::ShaderMaterial), pack.materials.getCount() + 1);
#endif
		for (uint32_t i = 0; i < pack.materials.getCount(); ++i) {
			sce::SampleUtil::Graphics::initializeShaderMaterialFromPackMaterial(pOutPackModel->m_materialsMemory[i], pack.materials[i], pack.textures, texLibrary, textureNamePrefix);
		}
		const uint32_t	defaultMaterialIndex = pack.materials.getCount();
		sce::SampleUtil::Graphics::initializeShaderMaterial(pOutPackModel->m_materialsMemory[defaultMaterialIndex], texLibrary);

		for (int skelIndex = 0; skelIndex < pack.skeletons.getCount(); skelIndex++) {
			// serach for associated animation data
			const EdgeAnimAnimation* pEdgeAnimation = nullptr;
			for (int i = 0; i < edgeData.animations.getCount(); i++) {
				if (edgeData.anim_to_skeleton[i] == skelIndex) {
					pEdgeAnimation = edgeData.animations[i];
					break;
				}
			}
			if (pEdgeAnimation != nullptr) {
				pOutPackModel->m_animations.emplace_back(pOutPackModel, pack.skeletons[skelIndex], *edgeData.skeletons[skelIndex], pEdgeAnimation, cpuMemory);
			} else {
				// if associated animation is not found, just push empty animation to make it consistent with pack skeletons order
				pOutPackModel->m_animations.emplace_back();
			}
		}

		std::vector<std::vector<sce::SampleUtil::Graphics::MeshInstanceUserData>>	instanceUserDataArray(pack.meshes.getCount());

		// Nodes
		if (pack.nodes.getCount() == 0) {
			for (uint32_t meshIdx = 0; meshIdx < pack.meshes.getCount(); ++meshIdx) {
				sce::SampleUtil::Graphics::MeshInstance	instance;
				instance.m_name				= (pack.meshes[meshIdx].name[0] == '\0') ? std::to_string(meshIdx) : std::string(pack.meshes[meshIdx].name);
				instance.m_meshIndex		= meshIdx;
				instance.m_instanceIndex	= instanceUserDataArray[meshIdx].size();
				instance.m_isVisible		= true;
				instance.m_bounds			= pOutPackModel->m_meshes[meshIdx].m_bounds;
				pOutPackModel->m_instances.push_back(instance);

				// GPU data
				sce::SampleUtil::Graphics::MeshInstanceUserData	instanceUserData = {};
				instanceUserData.m_transform		= Transform3::identity();
				instanceUserData.m_materialIndex	= defaultMaterialIndex;
				instanceUserData.m_skeletonIndex	= pack.meshes[meshIdx].skeleton_index;
				instanceUserData.m_bonesPerVertex	= pOutPackModel->m_meshes[meshIdx].m_bonesPerVertex;
				instanceUserDataArray[meshIdx].push_back(instanceUserData);
			}
		} else {
			for (int node_i = 0; node_i < pack.nodes.getCount(); ++node_i) {
				const PackFile::Node& node = pack.nodes[node_i];
				Transform3	nodePose = getWorldPose(pack, node_i);

				for (int i = 0; i < node.meshes.size(); ++i) {
					const uint32_t	meshIdx = node.meshes[i];

					sce::SampleUtil::Graphics::MeshInstance	instance;
					instance.m_name				= std::string(node.name) + "#" + ((pack.meshes[meshIdx].name[0] == '\0') ? std::to_string(i) : std::string(pack.meshes[meshIdx].name));
					instance.m_meshIndex		= meshIdx;
					instance.m_instanceIndex	= instanceUserDataArray[meshIdx].size();
					instance.m_isVisible		= true;
					instance.m_bounds			= transformBounds(nodePose, pOutPackModel->m_meshes[meshIdx].m_bounds);
					pOutPackModel->m_instances.push_back(instance);

					// GPU data
					sce::SampleUtil::Graphics::MeshInstanceUserData	instanceUserData = {};
					instanceUserData.m_transform		= nodePose;
					instanceUserData.m_materialIndex	= (i < node.material_index.getCount()) ? node.material_index[i] : defaultMaterialIndex;
					instanceUserData.m_skeletonIndex	= pack.meshes[meshIdx].skeleton_index;
					instanceUserData.m_bonesPerVertex	= pOutPackModel->m_meshes[meshIdx].m_bonesPerVertex;
					instanceUserDataArray[meshIdx].push_back(instanceUserData);
				}
			}
		}

		// create mesh instances userdata
		pOutPackModel->m_gpuInstanceUserDataArray.resize(pack.meshes.getCount());
		pOutPackModel->m_meshInstancesGpuMemory.resize(pack.meshes.getCount());
		for (uint32_t meshIdx = 0; meshIdx < pack.meshes.getCount(); ++meshIdx) {
			const uint32_t	numInstances = instanceUserDataArray[meshIdx].size();
#if _SCE_TARGET_OS_PROSPERO
			pOutPackModel->m_meshInstancesGpuMemory[meshIdx] = sce::SampleUtil::Memory::Gpu::make_unique<sce::SampleUtil::Graphics::MeshInstanceUserData>(numInstances, 16,
				{ sce::Agc::ResourceRegistration::ResourceType::kSrt }, gpuMemory, "mesh#" + std::to_string(meshIdx) + " instance userdata");
#endif
#if _SCE_TARGET_OS_ORBIS
			pOutPackModel->m_meshInstancesGpuMemory[meshIdx] = sce::SampleUtil::Memory::Gpu::make_unique<sce::SampleUtil::Graphics::MeshInstanceUserData>(numInstances, 16,
				{ sce::Gnm::kResourceTypeGenericBuffer }, gpuMemory, "mesh#" + std::to_string(meshIdx) + " instance userdata");
#endif
			sce::SampleUtil::Graphics::MeshInstanceUserData	*pInstanceUserdata = pOutPackModel->m_meshInstancesGpuMemory[meshIdx].get();
			memcpy(pInstanceUserdata, instanceUserDataArray[meshIdx].data(), numInstances * sizeof(sce::SampleUtil::Graphics::MeshInstanceUserData));
#if _SCE_TARGET_OS_PROSPERO
			ret = sce::Agc::Core::initialize(&pOutPackModel->m_gpuInstanceUserDataArray[meshIdx], &sce::Agc::Core::BufferSpec()
				.initAsRegularBuffer(pInstanceUserdata, sizeof(sce::SampleUtil::Graphics::MeshInstanceUserData), numInstances));
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
#endif
#if _SCE_TARGET_OS_ORBIS
			pOutPackModel->m_gpuInstanceUserDataArray[meshIdx].initAsRegularBuffer(pInstanceUserdata, sizeof(sce::SampleUtil::Graphics::MeshInstanceUserData), numInstances);
#endif
		}

		computeSceneBounds(*pOutPackModel);
	}
} // anonymous namespace

namespace sce { namespace SampleUtil { namespace Graphics {

PackModel::PackModel(const std::string	&packFilename, VideoAllocator	&gpuMemory, SceLibcMspace	cpuMemory, TextureLibrary &texLibrary, const std::string	&name, const std::string	&namePrefix
#if _SCE_TARGET_OS_PROSPERO
						, sce::SampleUtil::Helper::AsyncAssetLoader	*pLoader
#endif
)
	: m_name			(name)
#if _SCE_TARGET_OS_PROSPERO
	, m_pAsyncLoader	(pLoader) // save loader here to be able to release AMM allocated memories when destructor is called
	, m_isResolved		(false)
#endif
	, m_pTextureLibrary	(&texLibrary)
{
	TAG_THIS_CLASS;
	int ret = SCE_OK; (void)ret;

#if _SCE_TARGET_OS_PROSPERO
	if (m_pAsyncLoader == nullptr)
#endif
	{
		// load and parse Package using normal memory allocator and File I/O
		std::ifstream	packIfs(packFilename, std::ios::binary);
		SCE_SAMPLE_UTIL_ASSERT(packIfs.good());
		packIfs.seekg(0, std::ios::end);
		const size_t packFileSizeInBytes = packIfs.tellg();
		char	*pPackFileContent	= new char[packFileSizeInBytes];
		SCE_SAMPLE_UTIL_ASSERT(pPackFileContent != nullptr);
		packIfs.seekg(0, std::ios::beg);
		packIfs.read(pPackFileContent, packFileSizeInBytes);
		loadPackage(this, pPackFileContent, packFileSizeInBytes, gpuMemory, cpuMemory, texLibrary, namePrefix, packFilename);
		delete[] pPackFileContent;
	}
#if _SCE_TARGET_OS_PROSPERO
	else {
		// load and parse Package using AMPR
		m_packData = m_pAsyncLoader->allocate(packFilename.c_str(), 0, 0, SCE_KERNEL_MTYPE_C, SCE_KERNEL_PROT_AMPR_WRITE | SCE_KERNEL_PROT_CPU_RW);
		ret = m_pAsyncLoader->load(packFilename.c_str(), packFilename.c_str(), 0, 0);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		m_pAsyncLoader->m_aprListener.waitIf({
			m_packData.m_pLoadState, sce::SampleUtil::Helper::AsyncAssetLoader::EventListener::Condition::kEqual, (uint64_t)sce::SampleUtil::Helper::AsyncAssetLoader::LoadState::kLoading,
			// Following lambda is the callback called when pack file load is completed.
			// Pack file is parsed in this callback, then gpu data loads which are specified by PackFile::Package::data_mapping[] are kicked.
			[packFilename, &gpuMemory, cpuMemory, &texLibrary, namePrefix](uintptr_t arg)
			{
				int ret = SCE_OK; (void)ret;

				auto	*pPackModel = reinterpret_cast<PackModel *>(arg);

				void	*pPackFileContent = pPackModel->m_packData.m_ptr;
				const size_t	packFileSizeInBytes = pPackModel->m_pAsyncLoader->m_assets[packFilename].m_sizeInBytes;
				loadPackage(pPackModel, pPackFileContent, packFileSizeInBytes, gpuMemory, cpuMemory, texLibrary, namePrefix, packFilename);
				ret = pPackModel->m_pAsyncLoader->release(packFilename.c_str());
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			}, (uintptr_t)this }
		);
	}
#endif
}

PackModel::~PackModel()
{
#if _SCE_TARGET_OS_PROSPERO
	resolve();
	if (m_pAsyncLoader != nullptr) {
		for (const auto	&load : m_loadedData) {
			const auto	assetIter	= find_if(m_pAsyncLoader->m_assets.begin(), m_pAsyncLoader->m_assets.end(), [&](auto iter) { return iter.second.m_loadStatus.m_ptr == load.m_ptr; });
			SCE_SAMPLE_UTIL_ASSERT(assetIter != m_pAsyncLoader->m_assets.end()); // loaded data must be found in loader's m_assets
			const std::string	&tag	= assetIter->first;
			m_pAsyncLoader->release(tag.c_str());
		}
	}
#endif
	UNTAG_THIS_CLASS;
}

void	PackModel::resolve()
{
#if _SCE_TARGET_OS_PROSPERO
	int ret = SCE_OK; (void)ret;

	if (!m_isResolved) {
		if (m_pAsyncLoader != nullptr) {
			m_pAsyncLoader->m_aprListener.waitIf({ m_packData.m_pLoadState, sce::SampleUtil::Helper::AsyncAssetLoader::EventListener::Condition::kNotEqual, kGpuDataLoading, nullptr, 0ul });
			// here GPU data loadings are guaranteed to be started, so we can avoid race condition to check GPU loads completions
			for (auto	&load : m_loadedData) {
				m_pAsyncLoader->m_aprListener.waitIf({ load.m_pLoadState, sce::SampleUtil::Helper::AsyncAssetLoader::EventListener::Condition::kEqual, (uint64_t)sce::SampleUtil::Helper::AsyncAssetLoader::LoadState::kLoading, nullptr, 0ul });
			}
		}
		m_isResolved = true;
	}
#endif
}

PackModelInstance::PackModelInstance(PackModel	&packModel, SceLibcMspace	cpuMemory, VideoAllocator *gpuMemory, Transform3_arg	modelMatrix, const std::string &name)
	: m_pPackModel				(&packModel)
	, m_name					(packModel.m_name != "" ? packModel.m_name + ":" + name : name)
	, m_occlusionQueryResults	(nullptr)
	, m_modelMatrix				(modelMatrix)
	, m_isVisible				(true)
{
	TAG_THIS_CLASS_WITH_NAME(m_name);

	for (int i = 0; i < packModel.m_instances.size(); i++) {
		m_drawOrder.push_back(i);
	}

	if (gpuMemory != nullptr) {
		m_occlusionQueryResults = Memory::Gpu::make_unique<Compat::OcclusionQueryResults>(packModel.m_instances.size(), (size_t)Compat::Alignment::kOcclusionQueryResult, *gpuMemory, std::string("OcclusionQueryResults:") + name);
	}

	if (cpuMemory != nullptr) {
		m_pPackModel->resolve();
		for (auto &animation : m_pPackModel->m_animations) {
			if (animation.m_pEdgeAnimation != nullptr) {
				m_animInstances.emplace_back(animation, cpuMemory, m_name);
			} else {
				// if associated animation is not found, just push empty animation to make it consistent with pack skeletons order
				m_animInstances.emplace_back();
			}
		}
	}
}

}}} // namespace sce::SampleUtil::Graphics