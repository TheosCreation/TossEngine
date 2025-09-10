/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <mat.h>

namespace sce { namespace SampleUtil { namespace Memory {

	extern bool	g_isMatInitialized;

	/*!
	 * @~English
	 * @brief Initializes memory analyzer
	 * @~Japanese
	 * @brief メモリアナライザを初期化
	 */
	void	initializeMemoryAnalyzer();
	/*!
	 * @~English
	 * @brief Finalizes memory analyzer
	 * @~Japanese
	 * @brief メモリアナライザの終了処理
	 */
	void	finalizeMemoryAnalyzer();
	/*!
	 * @~English
	 * @brief Registers MatGroup of specified allocator
	 * @param pAllocator Allocator
	 * @param pName The name of allocator
	 * @~Japanese
	 * @brief アロケータのMatGroupを登録
	 * @param pAllocator アロケータ
	 * @param pName アロケータの名前
	 */
	void	registerMatGroup(const	void *pAllocator, const char	*pName);
	/*!
	 * @~English
	 * @brief Compute MatGroup of specified allocator
	 * @param pAllocator Allocator
	 * @return MatGroup of specified allocator
	 * @~Japanese
	 * @brief アロケータのMatGroupを求める
	 * @param pAllocator アロケータ
	 * @return 指定したアロケートのMatGroup
	 */
	MatGroup	getMatGroup(const	void *pAllocator);

}}} // sce::SampleUtil::Memory

