/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <cstdlib>
#include <string>
#include <mspace.h>
#ifdef _DEBUG
#include <sanitizer/asan_interface.h>
#include <sampleutil/debug/perf.h>
#endif
#include <sampleutil/graphics/compat.h>
#include <sampleutil/memory/memory_analyzer.h>

namespace sce {namespace SampleUtil { namespace Memory {
class AllocatorBase
{
public:
	/*!
	 * @~English
	 * @brief Deleter object
	 * @details This object is used as deleter when the memory allocated by memory allocator which is derived from AllocatorBase is set to std::unique_ptr
	 * @~Japanese
	 * @brief デリータオブジェクト
	 * @details AllocatorBaseの派生メモリアロケータで確保したメモリをstd::unique_ptrにセットする際のデリータとして使用します
	 */
	template<typename T = void>
	struct Dealloc
	{
		void	*m_alloc;
		bool	m_isMspace;

		constexpr Dealloc() noexcept = default;
		constexpr Dealloc(void *alloc, bool	isMspace = false) : m_alloc(alloc), m_isMspace(isMspace) {}

		template<typename U, typename std::enable_if<std::is_convertible<U*, T*>::value, std::nullptr_t>::type = nullptr>
		Dealloc(const Dealloc<U> &) noexcept {}

		void operator()(T	*ptr) const
		{
			ptr->~T();
			if (ptr)
			{
				if (m_isMspace)
				{
					SceLibcMspace mspace = (SceLibcMspace)m_alloc;
#ifdef _DEBUG
					size_t allocSize = sceLibcMspaceMallocUsableSize(ptr);
					ASAN_POISON_MEMORY_REGION(ptr, allocSize);
					if (g_isMatInitialized)
					{
						sceMatFree(ptr);
					}
					sce::SampleUtil::Debug::Perf::unTagBuffer(ptr);
#endif
					sceLibcMspaceFree(mspace, ptr);
				} else {
					((AllocatorBase *)m_alloc)->free(ptr);
				}
			}
		}
	};

	/*!
	 * @~English
	 * @brief Deleter object for array type
	 * @details This object is used as deleter when the memory of array type allocated by memory allocator which is derived from AllocatorBase is set to std::unique_ptr
	 * @~Japanese
	 * @brief 配列型用デリータオブジェクト
	 * @details AllocatorBaseの派生メモリアロケータで確保した配列型のメモリをstd::unique_ptrにセットする際のデリータとして使用します
	 */
	template<typename T>
	struct Dealloc<T[]>
	{
		void *m_alloc;
		bool	m_isMspace;

		constexpr Dealloc() noexcept = default;
		constexpr Dealloc(void *alloc, bool	isMspace = false) : m_alloc(alloc), m_isMspace(isMspace) {}

		template<typename U, typename std::enable_if<std::is_convertible<U (*)[], T (*)[]>::value, std::nullptr_t>::type = nullptr>
		Dealloc(const Dealloc<U[]> &) noexcept {}

		void operator()(T *ptr) const
		{
			if (ptr) {
				if (m_isMspace) {
					SceLibcMspace mspace = (SceLibcMspace)m_alloc;
#ifdef _DEBUG
					size_t allocSize = sceLibcMspaceMallocUsableSize(ptr);
					ASAN_POISON_MEMORY_REGION(ptr, allocSize);
					if (g_isMatInitialized) {
						sceMatFree(ptr);
					}
					sce::SampleUtil::Debug::Perf::unTagBuffer(ptr);
#endif
					sceLibcMspaceFree(mspace, ptr);
				} else {
					((AllocatorBase *)m_alloc)->free(ptr);
				}
			}
		}
	};

	virtual ~AllocatorBase() {}
	virtual void	*allocate(size_t	size, size_t	alignment, const std::string	&name = "") = 0;
	virtual void	free(void	*ptr) = 0;
	virtual void	registerResource(const void	*ptr, size_t	sizeInBytes, const std::string	&name, const std::vector<sce::SampleUtil::Graphics::Compat::ResourceType>	&resTypes)
	{
		(void)ptr; (void)sizeInBytes; (void)name; (void)resTypes;
	}
	virtual MatGroup	getMatGroup()
	{
		return sce::SampleUtil::Memory::getMatGroup(this);
	}
};
}}} // namespace sce::SampleUtil::Memory
