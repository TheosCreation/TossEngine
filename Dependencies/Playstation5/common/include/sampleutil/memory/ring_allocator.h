/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once
#include <cstddef>
#include <mutex>
#include <deque>
#include <vector>
#include <sys/types.h>
#include <string>
#include "sampleutil/memory/memory_analyzer.h"
#include "sampleutil/debug/perf.h"

namespace sce { namespace SampleUtil { namespace Memory {
/*!
 * @~English
 * @brief Ring memory allocator
 * @~Japanese
 * @brief リングメモリアロケータ
 */
class RingAllocator
{
protected:
	void								*m_mapStart;
	off_t								m_start;
	size_t								m_size;
	off_t								m_head;
	off_t								m_tail;
	std::mutex							m_mutex;
	std::deque<std::vector<uint64_t>>	m_frameAllocatedMemoriesArray;
	MatPool								m_pool;
public:
	RingAllocator(const std::string	&name = "no name") :m_mapStart(nullptr), m_start(0), m_size(0), m_head(0), m_tail(0)
	{
		TAG_THIS_CLASS;
		(void)name;
#ifdef _DEBUG
		if (g_isMatInitialized)
		{
			Memory::registerMatGroup(this, name.c_str());
		}
#endif
	}
	RingAllocator(const RingAllocator&) = delete;
	virtual ~RingAllocator() { UNTAG_THIS_CLASS; }
	/*!
	 * @~English
	 * @brief Allocates memory
	 * @param size Size(in bytes) of memory to be allocated
	 * @param alignment Alignment(in bytes) of memory to be allocated
	 * @param name Resource name
	 * @return Top address of allocate memory
	 * @~Japanese
	 * @brief メモリ割り当て
	 * @param size 割り当てメモリサイズ(バイト)
	 * @param alignment 割り当てメモリアライメント(バイト)
	 * @param name リソース名
	 * @return 確保したメモリの先導アドレス
	 */
	void	*allocate(size_t	size, size_t	alignment, const std::string	&name = "");
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
	 * @brief Callback called when running short of frame memory
	 * @details Switches to next frame if next frame is available, otherwise blocks until it becomes abailable
	 * @retval true endFrame() was called
	 * @retval false no endFrame() was called
	 * @~Japanese
	 * @brief フレームメモリを使い果たした際に呼ばれるコールバック
	 * @details 次のフレームが空いていればフレームを切り替えます。そうでなければ空くまで内部で待機します。
	 * @retval true endFrame() が呼ばれた
	 * @retval false endFrame() が呼ばれなかった
	 */
	virtual bool	allocateFailedHook() { return false; }
	/*!
	 * @~English
	 * @brief Begins frame memory
	 * @~Japanese
	 * @brief フレームメモリを開始
	 */
	void	beginFrame();
	/*!
	 * @~English
	 * @brief Ends frame memory
	 * @~Japanese
	 * @brief フレームメモリを終了
	 */
	void	endFrame();
};

} } } // namespace sce::SampleUtil::Memory
