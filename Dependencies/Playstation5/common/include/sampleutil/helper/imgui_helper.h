/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once

#include "../../../source/imgui/imgui.h"

namespace sce { namespace SampleUtil { namespace Helper {
/*!
 * @~English
 * @brief ImGui Utility of sample util.
 * @~Japanese
 * @brief SampleUtilのImGuiユーティリティです。
 */
namespace ImGui
{
	/*!
	 * @~English
	 * @brief Spinner busy indicator ImGui wdiget
	 * @param pLabel ImGui idintifier
	 * @param radius Spinner ring radius
	 * @param thickness Spinner ring thickness
	 * @param color Spinner ring color
	 * @return true if spinner is drawn, otherwise false
	 * @~Japanese
	 * @brief スピナービジーインジケータImGuiウィジェット
	 * @param pLabel ImGuiアイデンティファイア
	 * @param radius スピナーリングの半径
	 * @param thickness スピナーリングの幅
	 * @param color スピナーリングの色
	 * @return ウィジェットが描画された場合はtrue、そうでない場合はfalse
	 */
	bool	Spinner(const char	*pLabel, float	radius, int	thickness, const ImU32	&color);
} // namespace ImGui
}}} // namespace sce::SampleUtil::Helper
