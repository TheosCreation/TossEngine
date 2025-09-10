/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <memory>
#include <utility>
#include <string>
#include <mspace.h>
#include <sanitizer/asan_interface.h>
#include "sampleutil/graphics/graphics_memory.h"
#include "sampleutil/memory/cpu_memory.h"
#include "sampleutil/memory/dmem_mapper.h"
#include "sampleutil/memory/memory_analyzer.h"
#include "sampleutil/debug/perf.h"
#include "sampleutil/sampleutil_common.h"

namespace sce { namespace SampleUtil {
/*!
 * @~English
 * @brief Memory
 * @details Memory allocator
 * @~Japanese
 * @brief メモリ
 * @details メモリアロケータ
 */
namespace Memory {

	/*!
	 * @~English
	 * @brief Video memory
	 * @details Allocated memory usable for both CPU and GPU
	 * @~Japanese
	 * @brief ビデオメモリ
	 * @details CPU/GPU両方で使用可能なメモリを確保します
	 */
	namespace Gpu
	{
		/*!
		 * @~English
		 * @brief std::unique_ptr which contains GPU accessible memory
		 * @~Japanese
		 * @brief GPUからアクセス可能なメモリを格納するstd::unique_ptr
		 */
		template<typename T>
		using unique_ptr = std::unique_ptr<T, sce::SampleUtil::Memory::AllocatorBase::Dealloc<T>>;

		/*!
		 * @~English
		 * @brief Constructs Gpu::unique_ptr object of non-array type T
		 * @tparam T Type
		 * @tparam Args Arguments of T's constructor
		 * @param alignment Memory alignment(in bytes)
		 * @param allocator Video memory allocator
		 * @param args Arguments of T constructor
		 * @param name Resource name
		 * @return std::unique_ptr of allocated memory
		 * @~Japanese
		 * @brief 非配列型TのGpu::unique_ptrオブジェクトを構築する
		 * @tparam T 型
		 * @tparam Args T型のコンストラクタ引数
		 * @param alignment メモリアライメント(バイト)
		 * @param allocator ビデオメモリアロケータ
		 * @param args Tのコンストラクタの引数
		 * @param name リソース名
		 * @return 確保したメモリのstd::unique_ptr
		 */
		template<typename T, typename... Args>
		sce::SampleUtil::Memory::Gpu::unique_ptr<T>	make_unique(size_t	alignment, sce::SampleUtil::Memory::AllocatorBase	&allocator, Args&&... args, const std::string	&name = "no name")
		{
			auto ptr = static_cast<T*>(allocator.allocate(sizeof(T), alignment, name));
			SCE_SAMPLE_UTIL_ASSERT(ptr != nullptr);
#ifdef _DEBUG
			sce::SampleUtil::Debug::Perf::tagBuffer(name, ptr, 1);
#endif
			auto ret = sce::SampleUtil::Memory::Gpu::unique_ptr<T>(ptr, sce::SampleUtil::Graphics::VideoAllocator::Dealloc<T>{ &allocator, false });;
			new (ret.get()) T{ std::forward<Args>(args)... };

			return ret;
		}

		/*!
		 * @~English
		 * @brief Constructs Gpu::unique_ptr object of array of T type
		 * @tparam T Type
		 * @param numElements The number of elements
		 * @param alignment Memory alignment(in bytes)
		 * @param allocator Video memory allocator
		 * @param name Resource name
		 * @return std::unique_ptr of allocated memory
		 * @~Japanese
		 * @brief Tの配列型のGpu::unique_ptrオブジェクトを構築する
		 * @tparam T 型
		 * @param numElements 配列エレメント数
		 * @param alignment メモリアライメント(バイト)
		 * @param allocator ビデオメモリアロケータ
		 * @param name リソース名
		 * @return 確保したメモリのstd::unique_ptr
		 */
		template<typename T>
		sce::SampleUtil::Memory::Gpu::unique_ptr<T[]>	make_unique(size_t	numElements, size_t	alignment, sce::SampleUtil::Memory::AllocatorBase	&allocator, const std::string	&name = "no name")
		{
			if (numElements == 0)
			{
				return sce::SampleUtil::Memory::Gpu::unique_ptr<T[]>{nullptr};
			}
			auto ptr = static_cast<T*>(allocator.allocate(sizeof(T) * numElements, alignment, name));
			SCE_SAMPLE_UTIL_ASSERT(ptr != nullptr);
#ifdef _DEBUG
			sce::SampleUtil::Debug::Perf::tagBuffer(name, ptr, numElements);
#endif
			return	sce::SampleUtil::Memory::Gpu::unique_ptr<T[]>{static_cast<T*>(ptr), sce::SampleUtil::Graphics::VideoAllocator::Dealloc<T[]>(&allocator, false) };
		}

		/*!
		 * @~English
		 * @brief Constructs Gpu::unique_ptr object of array of T type, and register resource types
		 * @tparam T Type
		 * @tparam Args Arguments of T's constructor
		 * @param alignment Memory alignment(in bytes)
		 * @param resTypes Resource types to be regsistered
		 * @param allocator Video memory allocator
		 * @param args Arguments of T constructor
		 * @param name Resource name
		 * @return std::unique_ptr of allocated memory
		 * @~Japanese
		 * @brief 非配列型TのGpu::unique_ptrオブジェクトを構築し、リソース登録を行う
		 * @tparam T 型
		 * @tparam Args T型のコンストラクタ引数
		 * @param alignment メモリアライメント(バイト)
		 * @param resTypes 設定するリソースタイプ
		 * @param allocator ビデオメモリアロケータ
		 * @param args Tのコンストラクタの引数
		 * @param name リソース名
		 * @return 確保したメモリのstd::unique_ptr
		 */
		template<typename T, typename... Args>
		sce::SampleUtil::Memory::Gpu::unique_ptr<T>	make_unique(size_t	alignment, const std::vector<sce::SampleUtil::Graphics::Compat::ResourceType>	&resTypes, sce::SampleUtil::Memory::AllocatorBase	&allocator, Args&&...	args, const std::string	&name = "no name")
		{
			auto ptr = static_cast<T*>(allocator.allocate(sizeof(T), alignment, name));
			SCE_SAMPLE_UTIL_ASSERT(ptr != nullptr);
#ifdef _DEBUG
			sce::SampleUtil::Debug::Perf::tagBuffer(name, ptr, 1);
#endif
			auto ret = sce::SampleUtil::Memory::Gpu::unique_ptr<T>(ptr, sce::SampleUtil::Graphics::VideoAllocator::Dealloc<T>{ &allocator, false });
			new (ret.get()) T{ std::forward<Args>(args)... };
			if (name != "" && ptr != nullptr)
			{
				allocator.registerResource(ptr, sizeof(T), name, resTypes);
			}
			return ret;
		}

		/*!
		 * @~English
		 * @brief Constructs Gpu::unique_ptr object of array type T, and register resource types
		 * @tparam T Type
		 * @param numElements The number of elements
		 * @param alignment Memory alignment(in bytes)
		 * @param resTypes Resource types to be regsistered
		 * @param allocator Video memory allocator
		 * @param name Resource name
		 * @return std::unique_ptr of allocated memory
		 * @~Japanese
		 * @brief Tの配列型のGpu::unique_ptrオブジェクトを構築し、リソース登録を行う
		 * @tparam T 型
		 * @param numElements 配列エレメント数
		 * @param alignment メモリアライメント(バイト)
		 * @param resTypes 設定するリソースタイプ
		 * @param allocator ビデオメモリアロケータ
		 * @param name リソース名
		 * @return 確保したメモリのstd::unique_ptr
		 */
		template<typename T>
		sce::SampleUtil::Memory::Gpu::unique_ptr<T[]>	make_unique(size_t	numElements, size_t	alignment, const std::vector<sce::SampleUtil::Graphics::Compat::ResourceType>	&resTypes, sce::SampleUtil::Memory::AllocatorBase	&allocator, const std::string	&name = "no name")
		{
			auto ret = make_unique<T>(numElements, alignment, allocator, name);
			if (name != "" && ret.get() != nullptr)
			{
				allocator.registerResource(ret.get(), sizeof(T) * numElements, name, resTypes);
			}
			return	ret;
		}
	} // namespace Gpu

	/*!
	 * @~English
	 * @brief CPU memory
	 * @details Allocates memory usable only for CPU
	 * @~Japanese
	 * @brief CPU用メモリ
	 * @details CPUでのみ使用可能なメモリを確保します
	 */
	namespace Cpu
	{
		/*!
		 * @~English
		 * @brief std::unique_ptr which contains CPU accessible memory
		 * @~Japanese
		 * @brief CPUからアクセス可能なメモリを格納するstd::unique_ptr
		 */
		template<typename T>
		using unique_ptr = std::unique_ptr<T, sce::SampleUtil::Memory::AllocatorBase::Dealloc<T>>;

		/*!
		 * @~English
		 * @brief Constructs Cpu::unique_ptr object of non-array type T
		 * @tparam T Type
		 * @tparam Args Arguments of T's constructor
		 * @param alignment Memory alignment(in bytes)
		 * @param mspace SceLibcMspace
		 * @param args Arguments of T constructor
		 * @param name Unused
		 * @return std::unique_ptr of allocated memory
		 * @~Japanese
		 * @brief 非配列型TのCpu::unique_ptrオブジェクトを構築する
		 * @tparam T 型
		 * @tparam Args T型のコンストラクタ引数
		 * @param alignment メモリアライメント(バイト)
		 * @param mspace SceLibcMspace
		 * @param args Tのコンストラクタの引数
		 * @param name 未使用
		 * @return 確保したメモリのstd::unique_ptr
		 */
		template<typename T, typename... Args>
		sce::SampleUtil::Memory::Cpu::unique_ptr<T>	make_unique(size_t	alignment, SceLibcMspace	mspace, Args&&... args, const std::string	&name = "no name")
		{
			(void)name; // unused
			auto ptr = static_cast<T*>(sceLibcMspaceMemalign(mspace, alignment, sizeof(T)));
			SCE_SAMPLE_UTIL_ASSERT(ptr != nullptr);
#ifdef _DEBUG
			ASAN_UNPOISON_MEMORY_REGION(ptr, sizeof(T));
			if (Memory::g_isMatInitialized)
			{
				sceMatAlloc(ptr, sizeof(T), 0, Memory::getMatGroup(mspace));
				sceMatTagAllocation(ptr, name.c_str());
			}
			sce::SampleUtil::Debug::Perf::tagBuffer(name, ptr, 1);
#endif
			auto ret = Cpu::unique_ptr<T>(ptr, AllocatorBase::Dealloc<T>{ (void *)mspace, true });
			new (ret.get()) T{ std::forward<Args>(args)... };

			return ret;
		}

		/*!
		 * @~English
		 * @brief Constructs Cpu::unique_ptr object of array of T type
		 * @tparam T Type
		 * @param numElements The number of elements
		 * @param alignment Memory alignment(in bytes)
		 * @param mspace SceLibcMspace
		 * @param name Unused
		 * @return std::unique_ptr of allocated memory
		 * @~Japanese
		 * @brief Tの配列型のCpu::unique_ptrオブジェクトを構築する
		 * @tparam T 型
		 * @param numElements 配列エレメント数
		 * @param alignment メモリアライメント(バイト)
		 * @param mspace SceLibcMspace
		 * @param name 未使用
		 * @return 確保したメモリのstd::unique_ptr
		 */
		template<typename T>
		sce::SampleUtil::Memory::Cpu::unique_ptr<T[]>	make_unique(size_t	numElements, size_t	alignment, SceLibcMspace	mspace, const std::string	&name = "no name")
		{
			(void)name; // unused
			if (numElements == 0)
			{
				return sce::SampleUtil::Memory::Cpu::unique_ptr<T[]>{nullptr};
			}
			auto ptr = static_cast<T*>(sceLibcMspaceMemalign(mspace, alignment, sizeof(T) * numElements));
			SCE_SAMPLE_UTIL_ASSERT(ptr != nullptr);
#ifdef _DEBUG
			ASAN_UNPOISON_MEMORY_REGION(ptr, sizeof(T) * numElements);
			if (Memory::g_isMatInitialized)
			{
				sceMatAlloc(ptr, sizeof(T) * numElements, 0, Memory::getMatGroup(mspace));
				sceMatTagAllocation(ptr, name.c_str());
			}
			sce::SampleUtil::Debug::Perf::tagBuffer(name, ptr, numElements);
#endif
			return	sce::SampleUtil::Memory::Cpu::unique_ptr<T[]>{static_cast<T*>(ptr), sce::SampleUtil::Memory::AllocatorBase::Dealloc<T[]>{ (void *)mspace, true }};
		}

		/*!
		 * @~English
		 * @brief Constructs Cpu::unique_ptr object of non-array type T
		 * @tparam T Type
		 * @tparam Args Arguments of T's constructor
		 * @param alignment Memory alignment(in bytes)
		 * @param allocator CPU memory allocator
		 * @param args Arguments of T constructor
		 * @param name Unused
		 * @return std::unique_ptr of allocated memory
		 * @~Japanese
		 * @brief 非配列型TのCpu::unique_ptrオブジェクトを構築する
		 * @tparam T 型
		 * @tparam Args T型のコンストラクタ引数
		 * @param alignment メモリアライメント(バイト)
		 * @param allocator CPUメモリアロケータ
		 * @param args Tのコンストラクタの引数
		 * @param name 未使用
		 * @return 確保したメモリのstd::unique_ptr
		 */
		template<typename T, typename... Args>
		sce::SampleUtil::Memory::Cpu::unique_ptr<T>	make_unique(size_t	alignment, sce::SampleUtil::Memory::AllocatorBase	*allocator, Args&&... args, const std::string	&name = "no name")
		{
			(void)name; // unused
			auto ptr = static_cast<T*>(allocator->allocate(sizeof(T), alignment, name));
			SCE_SAMPLE_UTIL_ASSERT(ptr != nullptr);
#ifdef _DEBUG
			sce::SampleUtil::Debug::Perf::tagBuffer(name, ptr, 1);
#endif
			auto ret = Cpu::unique_ptr<T>(ptr, AllocatorBase::Dealloc<T>{ (void *)allocator, false });
			new (ret.get()) T{ std::forward<Args>(args)... };

			return ret;
		}

		/*!
		 * @~English
		 * @brief Constructs Cpu::unique_ptr object of array of T type
		 * @tparam T Type
		 * @param numElements The number of elements
		 * @param alignment Memory alignment(in bytes)
		 * @param allocator CPU memoruy allocator
		 * @param name Unused
		 * @return std::unique_ptr of allocated memory
		 * @~Japanese
		 * @brief Tの配列型のCpu::unique_ptrオブジェクトを構築する
		 * @tparam T 型
		 * @param numElements 配列エレメント数
		 * @param alignment メモリアライメント(バイト)
		 * @param allocator CPUメモリアロケータ
		 * @param name 未使用
		 * @return 確保したメモリのstd::unique_ptr
		 */
		template<typename T>
		sce::SampleUtil::Memory::Cpu::unique_ptr<T[]>	make_unique(size_t	numElements, size_t	alignment, sce::SampleUtil::Memory::AllocatorBase	*allocator, const std::string	&name = "no name")
		{
			(void)name; // unused
			if (numElements == 0)
			{
				return sce::SampleUtil::Memory::Cpu::unique_ptr<T[]>{nullptr};
			}
			auto ptr = static_cast<T*>(allocator->allocate(sizeof(T) * numElements, alignment, name));
			SCE_SAMPLE_UTIL_ASSERT(ptr != nullptr);
#ifdef _DEBUG
			sce::SampleUtil::Debug::Perf::tagBuffer(name, ptr, numElements);
#endif
			return	sce::SampleUtil::Memory::Cpu::unique_ptr<T[]>{static_cast<T*>(ptr), sce::SampleUtil::Memory::AllocatorBase::Dealloc<T[]>{ (void *)allocator, false }};
		}
	} // namespace Cpu

	/*!
	 * @~English
	 * @brief Allocates direct memory
	 * @param size	Allocation size
	 * @param align	Allocation align
	 * @param memType	Memory type
	 * @param prot	Memory protection
	 * @param name	Direct memory map name
	 * @return Pointer to allocated memory. nullptr is returned on allocation failure
	 * @~Japanese
	 * @brief ダイレクトメモリの確保
	 * @param size	アロケーションサイズ
	 * @param align	アロケーションアライン
	 * @param memType メモリタイプ
	 * @param prot メモリプロテクション
	 * @param name	ダイレクトメモリマップ名
	 * @return 確保したメモリへのポインタ。確保失敗時はnullptrを返す
	 */
	void 	*allocDmem(size_t	size, size_t	align, int memType, int prot, const std::string &name);

	/*!
	 * @~English
	 * @brief Frees direct memory
	 * @param ptr Pointer to memory to be freed
	 * @~Japanese
	 * @brief 確保したダイレクトメモリの解放
	 * @param ptr 解放するメモリへのポインタ
	 */
	void	freeDmem(void *ptr);
} // namespace Memory
} } // namespace sce::SampleUtil
