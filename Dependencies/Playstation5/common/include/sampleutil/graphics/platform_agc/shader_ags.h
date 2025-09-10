/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2020 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#include <agc/shader.h>
#include <agc/resourceregistration.h>
#include "sampleutil/sampleutil_common.h"

namespace sce { namespace SampleUtil { namespace Graphics {
	/*!
	 * @~English
	 * @brief Resource Registration of shader binary
	 * @details After Resource Registration of a shader binary, that shader shows up in a list of Shader Debugger Shader view.
	 * @param pShader A shader to be registered
	 * @param pName Resource name used when Resource Registration
	 * @retval SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief シェーダバイナリのResource Registration
	 * @details シェーダバイナリをResource RegistrationすることでShader Debuggerのシェーダ viewにそのシェーダがリスト表示される
	 * @param pShader Resoure Registrationするシェーダ
	 * @param pName 登録するリソース名
	 * @retval SCE_OK 成功
	 * @retval (<0) エラーコード
	 */
	int	registerShaderBinary(const Agc::Shader	*pShader, const char *pName);
}}} // namespace sce::SampleUtil::Graphics

/*!
 * @~English
 * @brief Defines a shader embedded in elf
 * @param name Shader name. \e shader \e name part of [CxxSymbol("Shader::shader name")] defined in shader as system attribute.
 * @~Japanese
 * @brief elf埋め込みシェーダを定義
 * @param name シェーダ名。シェーダ内でシステム属性にて、[CxxSymbol("Shader::シェーダ名")]と定義した際のシェーダ名の部分
 */
#define DEFINE_SHADER(name)				\
namespace Shader {						\
extern char name##_header[];			\
extern const char name##_text[];		\
extern const char name##_metadata[];	\
sce::Agc::Shader *name = nullptr;\
}

/*!
 * @~English
 * @brief Initializes a shader embedded in elf
 * @details Needs to be called prior to use shader. sce::Agc::createShader is calle inside it.
 * @param name Shader name defined in DEFINE_SHADER
 * @~Japanese
 * @brief elf埋め込みシェーダを初期化
 * @details シェーダを使用する前に一度呼び出す必要があります。内部でsce::Agc::createShaderが呼ばれます。
 * @param name DEFINE_SHADERで指定したシェーダ名
 */
#define CREATE_SHADER(name) \
if (::Shader::name == nullptr) {													\
int ret = SCE_OK;(void)ret;															\
ret = sce::Agc::createShader(&::Shader::name, ::Shader::name##_header, ::Shader::name##_text);	\
SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);											\
ret = sce::SampleUtil::Graphics::registerShaderBinary(::Shader::name, #name);		\
SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);\
}

/*!
 * @~English
 * @brief Obtains metadata of a shader embedded in elf
 * @param name Shader name defined in DEFINE_SHADER
 * @~Japanese
 * @brief elf埋め込みシェーダのmetadataを取得
 * @param name DEFINE_SHADERで指定したシェーダ名
 */
#define SHADER_METADATA(name) ((SceShaderMetadataSectionHandle)::Shader::name##_metadata)
