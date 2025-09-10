/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2022 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

namespace sce {	namespace SampleUtil { namespace Graphics {
	/*!
	 * @~English
	 * @brief HDR Format
	 * @details HDR Format of pixel shader output
	 * @~Japanese
	 * @brief HDRフォーマット
	 * @details ピクセルシェーダの出力結果のHDRフォーマット
	 */
	enum class HdrFormat : uint32_t
	{
		/*!
		 * @~English
		 * @brief BT.709
		 * @~Japanese
		 * @brief BT.709
		 */
		kBT709,
		/*!
		 * @~English
		 * @brief BT.2020 + PQ encode
		 * @~Japanese
		 * @brief BT.2020 + PQ encode
		 */
		kBT2020_PQ,
		/*!
		 * @~English
		 * @brief BT.2020, Linear(in nits)
		 * @~Japanese
		 * @brief BT.2020, Linear(in nits)
		 */
		 kBT2020_Linear,
		 /*!
		 * @~English
		 * @brief P3D65
		 * @~Japanese
		 * @brief P3D65
		 */
		kP3D65
	};

	/*!
	 * @~English
	 * @brief Specify HDR Format
	 * @param newHdr	HDR Format to be applied from here
	 * @retval SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief HDRフォーマットの設定
	 * @param newHdr	設定するHDRフォーマット
	 * @retval SCE_OK 成功
	 * @retval (<0) エラーコード
	 */
	int	pushHdr(HdrFormat	newHdr);
	/*!
	 * @~English
	 * @brief Reverts HDR Format
	 * @retval SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief HDRフォーマットを戻す
	 * @retval SCE_OK 成功
	 * @retval (<0) エラーコード
	 */
	int	popHdr();
	/*!
	 * @~English
	 * @brief Returns current HDR Format
	 * @return HDR Format
	 * @~Japanese
	 * @brief 現在のHDRフォーマットの取得
	 * @return HDRフォーマット
	 */
	HdrFormat	getHdr();

}}} // namespace sce::SampleUtil::Graphics

