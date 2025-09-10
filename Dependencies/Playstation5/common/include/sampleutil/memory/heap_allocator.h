/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <cstddef>
#include <string>
#include <set>
#include <unordered_map>
#include <mutex>
#include <mat.h>
#include <sampleutil/memory/allocator_base.h>

namespace sce {namespace SampleUtil { namespace Memory {
/*!
 * @~English
 * @brief Heap memory allocator
 * @~Japanese
 * @brief ヒープメモリアロケータ
 */
class HeapAllocator : public AllocatorBase
{
	struct Chunk;
	struct ChunkCompare
	{
		bool operator()(const Chunk	*lhs, const Chunk	*rhs) const;
	};
	std::set<Chunk*,ChunkCompare>		m_freeChunks;
	std::unordered_map<void*,Chunk*>	m_chunks;
	void								*m_mapStart;
	size_t								m_size;
	std::mutex							m_mutex;
	bool								m_noPhysicalMemory;
	MatPool								m_pool = (MatPool)-1;
protected:
	void	setInfo(void	*mapStart, size_t	size, const std::string	&name, bool	noPhysicalMemory = false);
public:
	/*!
	 * @~English
	 * @brief Constructor
	 * @param mapStart Top address of memory managed by allocator
	 * @param size Size(in bytes) of memory managed by allocator
	 * @param name The name of allocator
	 * @param noPhysicalMemory Set true if virtual address range which isn't mapped to physical memory is specified.
	 * @~Japanese
	 * @brief コンストラクタ
	 * @param mapStart アロケータで管理するメモリの先頭アドレス
	 * @param size アロケータで管理するメモリのサイズ(バイト)
	 * @param name アロケータの名前
	 * @param noPhysicalMemory 物理メモリが割り当てられていない仮想アドレスレンジを指定した場合はtrue
	 */
	HeapAllocator(void	*mapStart, size_t	size, const std::string	&name = "", bool noPhysicalMemory = false);
	HeapAllocator() : HeapAllocator(nullptr, 0ul) {}
	virtual ~HeapAllocator();
	/*!
	 * @~English
	 * @brief Allocates memory
	 * @param size Size(in bytes) of memory to be allocated
	 * @param alignment Alignment(in bytes) of memory to be allocated
	 * @param name Resource name
	 * @return Top address of allocated memory
	 * @~Japanese
	 * @brief メモリ割り当て
	 * @param size 割り当てメモリサイズ(バイト)
	 * @param alignment 割り当てメモリアライメント(バイト)
	 * @param name リソース名
	 * @return 確保したメモリの先頭アドレス
	 */
	virtual void	*allocate(size_t	size, size_t	alignment, const std::string	&name = "");
	/*!
	 * @~English
	 * @brief Frees allocated memory
	 * @param ptr Memory to be freed
	 * @~Japanese
	 * @brief メモリ開放
	 * @param ptr 開放するメモリ
	 */
	virtual void	free(void	*ptr);
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
	virtual void	allocateHook(void	*ptr, size_t	size, const std::string	&name) { (void)ptr; (void)size; (void)name; }	// unused variables
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
	virtual void	freeHook(void	*ptr, size_t	size) { (void)ptr; (void)size; }	// 	unused variables
	/*!
	 * @~English
	 * @brief Size of memory managed by allocator
	 * @return Size(in bytes) of memory managed by allocator
	 * @~Japanese
	 * @brief アロケータが管理するメモリサイズ
	 * @return アロケータが管理するメモリサイズ(バイト)
	 */
	size_t	size() const { return	m_size; }
	/*!
	 * @~English
	 * @brief Top address of memory managed by allocator
	 * @return Top address of memory managed by allocator
	 * @~Japanese
	 * @brief アロケータが管理するメモリの先頭アドレス
	 * @return アロケータが管理するメモリの先頭アドレス
	 */
	void	*mapStart() const { return	m_mapStart; }
};
} } } // namespace sce::SampleUtil::Memory
