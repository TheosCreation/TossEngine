/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <deque>
#include <functional>
#include <mspace.h>
#include <sanitizer/asan_interface.h>
#include <sampleutil/debug/perf.h>
#include <sampleutil/memory/ring_allocator.h>


namespace sce { namespace SampleUtil { namespace Memory {
	/*!
	 * @~English
	 * @brief CPU memory ring allocator
	 * @details Ring allocator to allocate memory used for CPU
	 * @~Japanese
	 * @brief CPUメモリリングアロケータ
	 * @details CPUで使用するメモリを確保するリングアロケータ
	 */
	class CpuRingAllocator : public SampleUtil::Memory::RingAllocator
	{
	public:
		std::deque<void *>		m_frameLabels;

		/*!
		 * @~English
		 * @brief Constructor
		 * @param size Maximum memory size to be allocated(in bytes)
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param size アロケータで確保できる最大サイズ(バイト)
		 */
		CpuRingAllocator(size_t	size);
		CpuRingAllocator(const CpuRingAllocator &) = delete;
		virtual ~CpuRingAllocator();
		/*!
		 * @~English
		 * @brief Initializes allocator
		 * @param size Maximum memory size to be allocated(in bytes)
		 * @param pName Allocator name
		 * @~Japanese
		 * @brief 初期化
		 * @param size アロケータで確保できる最大サイズ(バイト)
		 * @param pName アロケータの名前
		 */
		void	initialize(size_t	size, const char	*pName = "RING BUFFER CPU MEMORY");
		/*!
		 * @~English
		 * @brief Memory allocation call back
		 * @details This is called when memory is allocated
		 * @param ptr Address of allocated memory
		 * @param size Size of allocated memory(in bytes)
		 * @param name Resource name
		 * @~Japanese
		 * @brief メモリ確保コールバック
		 * @details メモリ確保の際に呼び出されます
		 * @param ptr 確保したメモリの先頭アドレス
		 * @param size 確保したメモリのサイズ(バイト)
		 * @param name リソース名
		 */
		virtual void	allocateHook(void	*ptr, size_t	size, const std::string	&name);
		/*!
		 * @~English
		 * @brief Memory free call back
		 * @details This is called when memory is freed
		 * @param ptr Address of freed memory
		 * @param size Size of freed memory(in bytes)
		 * @~Japanese
		 * @brief メモリ解放コールバック
		 * @details メモリ解放の際に呼び出されます
		 * @param ptr 解放したメモリの先頭アドレス
		 * @param size 解放したメモリのサイズ(バイト)
		 */
		virtual void	freeHook(void	*ptr, size_t	size);
	};
} } } // namespace sce::SampleUtil::Memory
