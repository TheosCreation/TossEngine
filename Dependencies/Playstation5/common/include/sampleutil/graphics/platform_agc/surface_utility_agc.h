/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <cstdint>
#include <vectormath/cpp/vectormath_aos.h>
#include <agc/resourceregistration.h>
#include <agc/core.h>
#include "sampleutil/sampleutil_common.h"

namespace sce { namespace SampleUtil { namespace Graphics {
namespace SurfaceUtil
{
	/*!
	 * @~English
	 * @brief Obtains resource name
	 * @param target Texture to get resource name
	 * @return Resource name
	 * @~Japanese
	 * @brief リソース名の取得
	 * @param target リソース名を取得するテクスチャ
	 * @return リソース名
	 */
	std::string	getName(const Agc::Core::Texture &target);
	/*!
	 * @~English
	 * @brief Obtains resource name
	 * @param target Buffer to get resource name
	 * @return Resource name
	 * @~Japanese
	 * @brief リソース名の取得
	 * @param target リソース名を取得するバッファ
	 * @return リソース名
	 */
	std::string	getName(const Agc::Core::Buffer &target);
	/*!
	 * @~English
	 * @brief Obtains resource name
	 * @param target Render target to get resource name
	 * @return Resource name
	 * @~Japanese
	 * @brief リソース名の取得
	 * @param target リソース名を取得するレンダーターゲット
	 * @return リソース名
	 */
	std::string	getName(const Agc::CxRenderTarget	&target);
	/*!
	 * @~English
	 * @brief Obtains resource name
	 * @param target Render target to get resource name
	 * @return Resource name
	 * @~Japanese
	 * @brief リソース名の取得
	 * @param target リソース名を取得するレンダーターゲット
	 * @return リソース名
	 */
	std::string	getName(const RenderTargetWrapper	&target);
	/*!
	 * @~English
	 * @brief Obtains resource name
	 * @param target Depth render target to get resource name
	 * @return Resource name
	 * @~Japanese
	 * @brief リソース名の取得
	 * @param target リソース名を取得するデプスレンダーターゲット
	 * @return リソース名
	 */
	std::string	getName(const Agc::CxDepthRenderTarget	&target);
	/*!
	 * @~English
	 * @brief Obtains resource name
	 * @param target Depth render target to get resource name
	 * @return Resource name
	 * @~Japanese
	 * @brief リソース名の取得
	 * @param target リソース名を取得するデプスレンダーターゲット
	 * @return リソース名
	 */
	std::string	getName(const DepthRenderTargetWrapper	&target);

	// Assign to any buffers that are unused to prevent validation from
	// reporting an error about a buffer with an invalid base address

	/*!
	 * @~English
	 * @brief Obtains dummy texture
	 * @details Unexpected validation errors can be avoided by assigning this texture to unused resource slots.
	 * @return Dummy texture
	 * @~Japanese
	 * @brief ダミーテクスチャの取得
	 * @details このテクスチャを未使用リソーススロットに割り当てることで期待しないバリデーションエラーを回避します。
	 * @return ダミーテクスチャ
	 */
	Agc::Core::Texture	getPassValidationTexture();
	/*!
	 * @~English
	 * @brief Obtains dummy buffer
	 * @details Unexpected validation errors can be avoided by assigning this buffer to unused resource slots.
	 * @return Dummy buffer
	 * @~Japanese
	 * @brief ダミーバッファの取得
	 * @details このバッファを未使用リソーススロットに割り当てることで期待しないバリデーションエラーを回避します。
	 * @return ダミーバッファ
	 */
	Agc::Core::Buffer	getPassValidationBuffer();

	Agc::Core::Texture	getNormalMapTexture();

	static inline std::string	getResourceName(void	*ptr)
	{
		(void)ptr;
		char name[256] = "";
#ifdef _DEBUG
		Agc::ResourceRegistration::ResourceHandle resourceHandle;
		Agc::ResourceRegistration::findResources(ptr, 16, [](Agc::ResourceRegistration::ResourceHandle resourceHandle, Agc::ResourceRegistration::OwnerHandle ownerHandle, uint64_t callbackdata)
		{
			(void)ownerHandle;
			Agc::ResourceRegistration::ResourceHandle *pResourceHandles = reinterpret_cast<Agc::ResourceRegistration::ResourceHandle *>(callbackdata);
			*pResourceHandles = resourceHandle;
			return SCE_OK;
		}, reinterpret_cast<uintptr_t>(&resourceHandle));

		Agc::ResourceRegistration::getResourceName(resourceHandle, name, 256);

#endif // _DEBUG
		return std::string(name);
	}
} // namespace SurafceUtil
}}} // namespace sce::SampleUtil::Graphics
