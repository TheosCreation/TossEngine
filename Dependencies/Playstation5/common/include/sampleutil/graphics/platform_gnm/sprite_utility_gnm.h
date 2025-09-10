/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2023 Sony Interactive Entertainment Inc. 
* 
*/


#pragma once

#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include <vectormath/cpp/vectormath_aos.h>
#include <gnmx.h>
#include "sampleutil/memory.h"
#include "sampleutil/graphics/platform_gnm/shader_gnm.h"
#include "sampleutil/graphics/mesh.h"
#include "sampleutil/graphics/graphics_memory.h"
#include "../source/imgui/imgui.h"

EXTERN_SHADER(sce::Gnmx::PsShader, sprite_utility_fill_p);
EXTERN_SHADER(sce::Gnmx::PsShader, sprite_utility_3d_fill_p);

namespace sce {	namespace SampleUtil { namespace Graphics {
namespace SpriteUtil {

	/*!
	 * @~Japanese
	 * @brief YUV変換モード
	 */
	enum class YuvMode : uint8_t
	{
		kBt601,
		kBt709,
		kBt2020
	};

	/*!
		* @~Japanese
		* @brief SpriteUtilの初期化処理
		* @param videoMemory Videoメモリのアロケータ
		* 
		* @retval SCE_OK 成功
		* @details 主にシェーダの初期化、必要な頂点・インデックスバッファの確保を行います。
		*/
	int	initialize(SampleUtil::Graphics::VideoAllocator	&videoMemory);

	/*!
		* @~Japanese
		* @brief SpriteUtilの終了処理
		* 
		* @retval SCE_OK 成功
		* @details 主にinitializeで確保したリソースの開放を行います
		*/
	int	finalize();

	/*!
	 * @~English
	 * @brief Set render target size
	 * @param width render target width
	 * @param height render target height
	 * @retval SCE_OK Success
	 * @retval (<0) Error code
	 * @details Set render target size for SpriteUtility font rendering
	 * @~Japanese
	 * @brief レンダーターゲットの設定
	 * @param width レンダーターゲットの幅
	 * @param height レンダーターゲットの高さ
	 * @retval SCE_OK 成功
	 * @retval (<0) エラーコード
	 * @details SpriteUtilityのフォント描画のためのレンダーターゲットサイズの設定
	 */
	int	setRenderTargetSize(uint32_t	width, uint32_t	height);

	/*!
	 * @~English
	 * @brief Enables alpha blend
	 * @param enable Enables alpha blend
	 * @details Sets enable/disable for SpriterUtility draws after this call
	 * @~Japanese
	 * @brief アルファブレンドの有効化
	 * @param enable アルファブレンドを有効化
	 * @details 以降のSpriteUtilityの描画のアルファブレンドの有効・無効を設定
	 */
	void	setAlphaBlendEnable(bool	enable);

	/*!
	 * @~English
	 * @brief Obtains alpha blend setting
	 * @return Alpha blend enabled or disabled
	 * @details Obtains current setting of SampleUtil's alpha blend enable/disable
	 * @~Japanese
	 * @brief アルファブレンド設定の取得
	 * @return アルファブレンド有効・無効
	 * @details 現在のSpriteUtilityの描画のアルファブレンドの有効・無効を取得
	 */
	bool	getAlphaBlendEnable();

	/*!
		* @~English
		* @brief Draws a square 
		* @param dcb Pointer to DrawCommandBuffer
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param size Size of a square (Specifies the width and height)
		* @param rgba Color of a square (Specifies RGBA)
		* @param depth Depth value
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details This draws a square. The square is filled with the specified color. 
		* @~Japanese
		* @brief 四角形の描画 
		* @param dcb DrawCommandBufferへのポインタ
		* @param position 描画する位置（左上の座標を指定）
		* @param size 四角形のサイズ（幅・高さを指定）
		* @param rgba 四角形の色（RGBAを指定）
		* @param depth デプス値
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details 四角形を描画します。四角形は指定された色により塗りつぶされます。 
		*/
	int	fillRect(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		sce::Vectormath::Simd::Aos::Vector2_arg	size,
		sce::Vectormath::Simd::Aos::Vector4_arg	rgba = sce::Vectormath::Simd::Aos::Vector4(1.f),
		float	depth = 0.f,
		const sce::Gnmx::PsShader *fillPs = SHADER(sprite_utility_fill_p),
		uint32_t psSrtSizeInDw = SIZE_OF_SRT(sprite_utility_fill_p),
		const void	*fillUserData = nullptr);

	/*!
		* @~English
		* @brief Draws a square 
		* @param dcb Pointer to DrawCommandBuffer
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param size Size of a square (Specifies the width and height)
		* @param rgba Color of a square (Specifies RGBA)
		* @param depth Depth value
		* @param width Line width
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details This draws a square. 
		* @~Japanese
		* @brief 四角形の描画 
		* @param dcb DrawCommandBufferへのポインタ
		* @param position 描画する位置（左上の座標を指定）
		* @param size 四角形のサイズ（幅・高さを指定）
		* @param rgba 四角形の色（RGBAを指定）
		* @param depth デプス値
		* @param width 線分の幅
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details 四角形を描画します。 
		*/
	int	drawRect(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		sce::Vectormath::Simd::Aos::Vector2_arg	size,
		sce::Vectormath::Simd::Aos::Vector4_arg	rgba = sce::Vectormath::Simd::Aos::Vector4(1.f), 
		float	depth = 0.f,
		uint32_t	width = 8);

	/*!
		* @~English
		* @brief Draws an ellipse. 
		* @param dcb Pointer to DrawCommandBuffer
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param size Size of an ellipse (Specifies the width and height)
		* @param rgba Color of an ellipse (Specifies RGBA)
		* @param depth Depth value
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details This draws an ellipse. The ellipse is filled with the specified color. 
		* @~Japanese
		* @brief 楕円の描画 
		* @param dcb DrawCommandBufferへのポインタ
		* @param position 描画する位置（左上の座標を指定）
		* @param size 楕円のサイズ（幅・高さを指定）
		* @param rgba 楕円の色（RGBAを指定）
		* @param depth デプス値
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details 楕円を描画します。楕円は指定された色により塗りつぶされます。 
		*/
	int	fillOval(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		sce::Vectormath::Simd::Aos::Vector2_arg	size,
		sce::Vectormath::Simd::Aos::Vector4_arg	rgba = sce::Vectormath::Simd::Aos::Vector4(1.f),
		float	depth = 0.f,
		const sce::Gnmx::PsShader *fillPs = SHADER(sprite_utility_fill_p),
		uint32_t psSrtSizeInDw = SIZE_OF_SRT(sprite_utility_fill_p),
		const void	*fillUserData = nullptr);

	/*!
		* @~English
		* @brief Draws an ellipse. 
		* @param dcb Pointer to DrawCommandBuffer
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param size Size of an ellipse (Specifies the width and height)
		* @param rgba Color of an ellipse (Specifies RGBA)
		* @param depth Depth value
		* @param width Line width
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details This draws an ellipse. 
		* @~Japanese
		* @brief 楕円の描画 
		* @param dcb DrawCommandBufferへのポインタ
		* @param position 描画する位置（左上の座標を指定）
		* @param size 楕円のサイズ（幅・高さを指定）
		* @param rgba 楕円の色（RGBAを指定）
		* @param depth デプス値
		* @param width 線分の幅
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details 楕円を描画します。 
		*/
	int	drawOval(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		sce::Vectormath::Simd::Aos::Vector2_arg	size,
		sce::Vectormath::Simd::Aos::Vector4_arg	rgba = sce::Vectormath::Simd::Aos::Vector4(1.f),
		float	depth = 0.f,
		uint32_t	width = 1);

	/*!
		* @~English
		* @brief Draws a line segment. 
		* @param dcb Pointer to DrawCommandBuffer
		* @param begin Line segment beginning position
		* @param end Line segment ending position
		* @param rgba Color of a line segment (Specifies RGBA)
		* @param depth Depth value
		* @param width Line width
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details This draws a line segment. 
		* @~Japanese
		* @brief 線分の描画 
		* @param dcb DrawCommandBufferへのポインタ
		* @param begin 線分の開始位置
		* @param end 線分の終了位置
		* @param rgba 線分の色（RGBAを指定）
		* @param depth デプス値
		* @param width 線分の幅
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details 線分を描画します。 
		*/
	int	drawLine(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Vector2_arg	begin,
		sce::Vectormath::Simd::Aos::Vector2_arg	end,
		sce::Vectormath::Simd::Aos::Vector4_arg	rgba = sce::Vectormath::Simd::Aos::Vector4(1.f),
		float	depth = 0.f,
		uint32_t	width = 1);


	/*!
		* @~English
		* @brief Draws a texture 
		* @param dcb Pointer to DrawCommandBuffer
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param size Size of the render within the render target (specify width and height)
		* @param textureOffset Offset within the texture
		* @param sizeInTexture Size in the texture
		* @param colorCoeff Color coefficient of a texture (Specifies RGBA)
		* @param texture Reference to Texture
		* @param sampler Reference to Texture-Sampler that will be used to sample 'texture'
		* @param depth Depth value
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details This draws a texture. 
		* @~Japanese
		* @brief テクスチャの描画 
		* @param dcb DrawCommandBufferへのポインタ
		* @param position 描画する位置（左上の座標を指定）
		* @param size レンダーターゲット内の描画のサイズ（幅・高さを指定）
		* @param textureOffset テクスチャ内のオフセット
		* @param sizeInTexture テクスチャ内のサイズ
		* @param colorCoeff テクスチャの色係数（RGBAを指定）
		* @param texture Textureへの参照
		* @param sampler 'texture'をサンプリングするために使用されるテクスチャサンプラーへの参照
		* @param depth デプス値
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details テクスチャを描画します。 
		*/
	int	drawTexture(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		sce::Vectormath::Simd::Aos::Vector2_arg	size,
		sce::Vectormath::Simd::Aos::Vector2_arg	textureOffset,
		sce::Vectormath::Simd::Aos::Vector2_arg	sizeInTexture,
		const sce::Gnm::Texture	&texture, 
		const sce::Gnm::Sampler	&sampler,
		sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff = sce::Vectormath::Simd::Aos::Vector4(1.f),
		float	depth = 0.f
		);

	/*!
		* @~English
		* @brief Draws a texture 
		* @param dcb Pointer to DrawCommandBuffer
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param size Size of the render within the render target (specify width and height)
		* @param colorCoeff Color coefficient of a texture (Specifies RGBA)
		* @param texture Reference to Texture
		* @param sampler Reference to Texture-Sampler that will be used to sample 'texture'
		* @param depth Depth value
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details This draws a texture. 
		* @~Japanese
		* @brief テクスチャの描画 
		* @param dcb DrawCommandBufferへのポインタ
		* @param position 描画する位置（左上の座標を指定）
		* @param size レンダーターゲット内の描画のサイズ（幅・高さを指定）
		* @param colorCoeff テクスチャの色係数（RGBAを指定）
		* @param texture Textureへの参照
		* @param sampler 'texture'をサンプリングするために使用されるテクスチャサンプラーへの参照
		* @param depth デプス値
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details テクスチャを描画します。 
		*/
	static inline int	drawTexture(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		sce::Vectormath::Simd::Aos::Vector2_arg	size,
		const sce::Gnm::Texture	&texture,
		const sce::Gnm::Sampler	&sampler,
		sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff = sce::Vectormath::Simd::Aos::Vector4(1.f),
		float	depth = 0.0f)
	{
		return drawTexture(dcb, position, size, sce::Vectormath::Simd::Aos::Vector2(0), sce::Vectormath::Simd::Aos::Vector2(1), texture, sampler, colorCoeff, depth);
	}


	/*!
		* @~English
		* @brief Draws a texture in YUY2 format 
		* @param dcb Pointer to DrawCommandBuffer
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param size Size of the render within the render target (specify width and height)
		* @param textureOffset Offset within the texture
		* @param sizeInTexture Size in the texture
		* @param colorCoeff Color coefficient of a texture (Specifies RGBA)
		* @param texture Reference to Texture
		* @param sampler Reference to Texture-Sampler that will be used to sample 'texture'
		* @param depth Depth value
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details The RGBA elements of the texture are considered YUY2 format YUYV and are converted to RGB. 1.0f will be set for A. 
		* @~Japanese
		* @brief YUY2フォーマットのテクスチャを描画する 
		* @param dcb DrawCommandBufferへのポインタ
		* @param position 描画する位置（左上の座標を指定）
		* @param size レンダーターゲット内の描画のサイズ（幅・高さを指定）
		* @param textureOffset テクスチャ内のオフセット
		* @param sizeInTexture テクスチャ内のサイズ
		* @param colorCoeff テクスチャの色係数（RGBAを指定）
		* @param texture Textureへの参照
		* @param sampler 'texture'をサンプリングするために使用されるテクスチャサンプラーへの参照
		* @param depth デプス値
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details テクスチャのRGBAの要素を、YUY2フォーマットのYUYVとみなし、 RGBに変換して描画します。Aには1.0fが設定されます。 
		*/
	int	drawTextureYuy2(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		sce::Vectormath::Simd::Aos::Vector2_arg	size,
		sce::Vectormath::Simd::Aos::Vector2_arg	textureOffset,
		sce::Vectormath::Simd::Aos::Vector2_arg	sizeInTexture,
		const sce::Gnm::Texture	&texture, 
		const sce::Gnm::Sampler	&sampler,
		sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff = sce::Vectormath::Simd::Aos::Vector4(1.f),
		float	depth = 0.f
		);

	/*!
		* @~English
		* @brief Draws a texture in YUY2 format 
		* @param dcb Pointer to DrawCommandBuffer
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param size Size of the render within the render target (specify width and height)
		* @param colorCoeff Color coefficient of a texture (Specifies RGBA)
		* @param texture Reference to Texture
		* @param sampler Reference to Texture-Sampler that will be used to sample 'texture'
		* @param depth Depth value
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details The RGBA elements of the texture are considered YUY2 format YUYV and are converted to RGB. 1.0f will be set for A. 
		* @~Japanese
		* @brief YUY2フォーマットのテクスチャを描画する 
		* @param dcb DrawCommandBufferへのポインタ
		* @param position 描画する位置（左上の座標を指定）
		* @param size レンダーターゲット内の描画のサイズ（幅・高さを指定）
		* @param colorCoeff テクスチャの色係数（RGBAを指定）
		* @param texture Textureへの参照
		* @param sampler 'texture'をサンプリングするために使用されるテクスチャサンプラーへの参照
		* @param depth デプス値
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details テクスチャのRGBAの要素を、YUY2フォーマットのYUYVとみなし、 RGBに変換して描画します。Aには1.0fが設定されます。 
		*/
	static inline int	drawTextureYuy2(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		sce::Vectormath::Simd::Aos::Vector2_arg	size,
		const sce::Gnm::Texture	&texture,
		const sce::Gnm::Sampler	&sampler,
		sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff = sce::Vectormath::Simd::Aos::Vector4(1.f),
		float	depth = 0.f)
	{
		return drawTextureYuy2(dcb, position, size, sce::Vectormath::Simd::Aos::Vector2(0), sce::Vectormath::Simd::Aos::Vector2(1), texture, sampler, colorCoeff, depth);
	}


	/*!
		* @~English
		* @brief Draws a texture in YCbCr format 
		* @param dcb Pointer to DrawCommandBuffer
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param size Size of the render within the render target (specify width and height)
		* @param textureOffset Offset within the texture
		* @param sizeInTexture Size in the texture
		* @param colorCoeff Color coefficient of a texture (Specifies RGBA)
		* @param yTexture Reference to the Texture containing the Y value
		* @param ySampler Reference to Texture-Sampler that will be used to sample 'yTexture'
		* @param cbcrTexture Reference to the Texture containing the Cb value and Cr value
		* @param cbcrSampler Reference to Texture-Sampler that will be used to sample 'cbcrTexture'
		* @param depth Depth value
		* @param is10bits 10bits YCbCr
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details Evaluates the R element of the first texture as Y, the RG element of the second texture as CbCr, and renders by converting them to the RBG format within the shader.  1.0f will be set for A. 
		* @~Japanese
		* @brief YCbCrフォーマットのテクスチャを描画する 
		* @param dcb DrawCommandBufferへのポインタ
		* @param position 描画する位置（左上の座標を指定）
		* @param size レンダーターゲット内の描画のサイズ（幅・高さを指定）
		* @param textureOffset テクスチャ内のオフセット
		* @param sizeInTexture テクスチャ内のサイズ
		* @param colorCoeff テクスチャの色係数（RGBAを指定）
		* @param yTexture Y値を含むTextureへの参照
		* @param ySampler 'yTexture'をサンプリングするために使用されるテクスチャサンプラーへの参照
		* @param cbcrTexture Cb値Cr値を含むTextureへの参照
		* @param cbcrSampler 'cbcrTexture'をサンプリングするために使用されるテクスチャサンプラーへの参照
		* @param depth デプス値
		* @param depth Depth value
		* @param yuvMode YUV変換モード
		* @param bitDepth YUV値のビット深度
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details 最初のテクスチャのRの要素をYとみなし、２番めのテクスチャのRG要素をCbCrとみなし、シェーダー内でRGBフォーマットに変換して描画します。 Aには1.0fが設定されます。 
		*/
	int	drawTextureYcbcr(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		sce::Vectormath::Simd::Aos::Vector2_arg	size,
		sce::Vectormath::Simd::Aos::Vector2_arg	textureOffset,
		sce::Vectormath::Simd::Aos::Vector2_arg	sizeInTexture,
		const sce::Gnm::Texture	&yTexture,
		const sce::Gnm::Sampler	&ySampler,
		const sce::Gnm::Texture	&cbcrTexture,
		const sce::Gnm::Sampler	&cbcrSampler,
		sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff = sce::Vectormath::Simd::Aos::Vector4(1.f),
		float	depth = 0.0f,
		YuvMode	yuvMode = YuvMode::kBt709,
		uint8_t	bitDepth  = 8
		);

	/*!
		* @~English
		* @brief Draws a texture in YCbCr format 
		* @param dcb Pointer to DrawCommandBuffer
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param size Size of the render within the render target (specify width and height)
		* @param colorCoeff Color coefficient of a texture (Specifies RGBA)
		* @param yTexture Reference to the Texture containing the Y value
		* @param ySampler Reference to Texture-Sampler that will be used to sample 'yTexture'
		* @param cbcrTexture Reference to the Texture containing the Cb value and Cr value
		* @param cbcrSampler Reference to Texture-Sampler that will be used to sample 'cbcrTexture'
		* @param depth Depth value
		* @param is10bits 10bits YCbCr
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details Evaluates the R element of the first texture as Y, the RG element of the second texture as CbCr, and renders by converting them to the RBG format within the shader.  1.0f will be set for A. 
		* @~Japanese
		* @brief YCbCrフォーマットのテクスチャを描画する 
		* @param dcb DrawCommandBufferへのポインタ
		* @param position 描画する位置（左上の座標を指定）
		* @param size レンダーターゲット内の描画のサイズ（幅・高さを指定）
		* @param colorCoeff テクスチャの色係数（RGBAを指定）
		* @param yTexture Y値を含むTextureへの参照
		* @param ySampler 'yTexture'をサンプリングするために使用されるテクスチャサンプラーへの参照
		* @param cbcrTexture Cb値Cr値を含むTextureへの参照
		* @param cbcrSampler 'cbcrTexture'をサンプリングするために使用されるテクスチャサンプラーへの参照
		* @param depth デプス値
		* @param yuvMode YUV変換モード
		* @param bitDepth YUV値のビット深度
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details 最初のテクスチャのRの要素をYとみなし、２番めのテクスチャのRG要素をCbCrとみなし、シェーダー内でRGBフォーマットに変換して描画します。 Aには1.0fが設定されます。 
		*/
	static inline int	drawTextureYcbcr(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		sce::Vectormath::Simd::Aos::Vector2_arg	size,
		const sce::Gnm::Texture	&yTexture,
		const sce::Gnm::Sampler	&ySampler,
		const sce::Gnm::Texture	&cbcrTexture,
		const sce::Gnm::Sampler	&cbcrSampler,
		sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff = sce::Vectormath::Simd::Aos::Vector4(1.f),
		float	depth = 0.f,
		YuvMode	yuvMode = YuvMode::kBt709,
		uint8_t	bitDepth = 8)
	{
		return drawTextureYcbcr(dcb, position, size, sce::Vectormath::Simd::Aos::Vector2(0), sce::Vectormath::Simd::Aos::Vector2(1), yTexture, ySampler, cbcrTexture, cbcrSampler, colorCoeff, depth, yuvMode, bitDepth);
	}


	/*!
		* @~English
		* @brief Draws a dot 
		* @param dcb Pointer to DrawCommandBuffer
		* @param psize Point size
		* @param colorCoeff Color coefficient of a texture (Specifies RGBA)
		* @param texture Reference to Texture
		* @param sampler Reference to Texture-Sampler that will be used to sample 'texture'
		* @param vertices Pointer to VertexBuffer
		* @param indices Pointer to IndexBuffer
		* @param numIndices Number of indices
		* @param depth Depth value
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details This draws a dot. 
		* @~Japanese
		* @brief 点の描画 
		* @param dcb DrawCommandBufferへのポインタ
		* @param psize 点の大きさ
		* @param colorCoeff テクスチャの色係数（RGBAを指定）
		* @param texture Textureへの参照
		* @param sampler 'texture'をサンプリングするために使用されるテクスチャサンプラーへの参照
		* @param vertices VertexBufferへのポインタ
		* @param indices IndexBufferへのポインタ
		* @param numIndices indicesの数
		* @param depth デプス値
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details 点を描画します。 
		*/
	int	drawPoints(
		sce::Gnm::DrawCommandBuffer *dcb,
		float psize,
		const sce::Gnm::Texture	&texture,
		const sce::Gnm::Sampler	&sampler,
		const sce::Gnm::Buffer	&vertices,
		Memory::Gpu::unique_ptr<Index[]> & indices, 
		uint32_t numIndices,
		sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff = sce::Vectormath::Simd::Aos::Vector4(1.f),
		float	depth = 0.f
		);

	/*!
		* @~English
		* @brief Draws a character texture 
		* @param font Pointer to Font
		* @param ucs2Charcode Pointer to a UCS2 character code
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param colorCoeff Color coefficient of a texture (Specifies RGBA)
		* @param depth Depth value
		* @param scale Scale (enlargement factor)
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details This creates a character texture by pointer specification of a UCS2 character code and draws it. It is possible to specify multiple characters. Make sure to always set the termination of a UCS2 character code to NULL. 
		* @details *When drawString is used, it is necessary to execute setRenderTargetSize after initializing SpriteRenderer. 
		* @~Japanese
		* @brief 文字テクスチャの描画 
		* @param font Fontへのポインタ
		* @param ucs2Charcode UCS2文字コードへのポインタ
		* @param position 描画する位置（左上の座標を指定）
		* @param colorCoeff テクスチャの色係数（RGBAを指定）
		* @param depth デプス値
		* @param scale 倍率
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details UCS2文字コードへのポインタ指定により、文字テクスチャを作成して描画します。複数文字の指定が可能です。UCS2文字コードの終端は、必ずNULLとしてください。 
		* @details ※drawString使用時は、SpriteRendererの初期化後にsetRenderTargetSizeを行う必要があります。 
		*/
	int	drawString(
		ImFont	*font,
		const uint16_t	*ucs2Charcode,
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
		float	depth = 0.f,
		float	scale = 1.f
		);

	/*!
		* @~English
		* @brief Gets the size of a character texture. 
		* @param font Pointer to Font
		* @param ucs2Charcode Pointer to a UCS2 character code
		* @param scale Scale (enlargement factor)
		* 
		* @retval sce::Vectormath::Simd::Aos::Vector2 Character texture size
		* @details This returns the size of a character texture created by pointer specification of a UCS2 character code. It is possible to specify multiple characters. Make sure to always set the termination of a UCS2 character code to NULL. 
		* @~Japanese
		* @brief 文字テクスチャのサイズ取得 
		* @param font Fontへのポインタ
		* @param ucs2Charcode UCS2文字コードへのポインタ
		* @param scale 倍率
		* 
		* @retval sce::Vectormath::Simd::Aos::Vector2 文字テクスチャのサイズ
		* @details UCS2文字コードへのポインタ指定により、作成される文字テクスチャのサイズを返します。複数文字の指定が可能です。UCS2文字コードの終端は、必ずNULLとしてください。 
		*/
	sce::Vectormath::Simd::Aos::Vector2	getStringTextureSize(
		ImFont	*font,
		const uint16_t	*ucs2Charcode,
		float	scale = 1.f
		);

	/*!
		* @~English
		* @brief Drawing a debug character string 
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param fontHeight Height of the font drawn
		* @param colorCoeff Color coefficient of a texture (Specifies RGBA)
		* @param backgroundColor Background color (specifies RGBA)
		* @param depth Depth value
		* @param string Character string to render
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details Draws a C Language style character string (Char array terminated with NULL) on the screen. 
		* @~Japanese
		* @brief デバッグ文字列の描画 
		* @param position 描画する位置（左上の座標を指定）
		* @param fontHeight 描画するフォントの高さ
		* @param colorCoeff テクスチャの色係数（RGBAを指定）
		* @param backgroundColor 背景色（RGBAを指定）
		* @param depth デプス値
		* @param string 描画する文字列
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details C言語スタイルの文字列(NULL終端のChar配列)を画面上に描画します 
		*/
	int	drawDebugString(
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		float	fontHeight,
		sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
		sce::Vectormath::Simd::Aos::Vector4_arg	backgroundColor,
		float	depth,
		const char	*string);

	/*!
		* @~English
		* @brief Drawing a debug character string 
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param fontHeight Height of the font drawn
		* @param colorCoeff Color coefficient of a texture (Specifies RGBA)
		* @param string Character string to render
		* 
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details Draws a C Language style character string (Char array terminated with NULL) on the screen. 
		* @~Japanese
		* @brief デバッグ文字列の描画 
		* @param position 描画する位置（左上の座標を指定）
		* @param fontHeight 描画するフォントの高さ
		* @param colorCoeff テクスチャの色係数（RGBAを指定）
		* @param string 描画する文字列
		* 
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details C言語スタイルの文字列(NULL終端のChar配列)を画面上に描画します 
		*/
	int	drawDebugString(
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		float	fontHeight,
		sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
		const char	*string);

	/*!
		* @~English
		* @brief Drawing a debug character string 
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param fontHeight Height of the font drawn
		* @param colorCoeff Color coefficient of a texture (Specifies RGBA)
		* @param backgroundColor Background color (specifies RGBA)
		* @param depth Depth value (set 1 to match other debugString functions)
		* @param format Character string designated in the C Language style (Char array terminated with NULL)
		* @param ... Variable arguments
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details This draws a character string designated in the C Language style (Char array terminated with NULL) on the screen. 
		* @~Japanese
		* @brief デバッグ文字列の描画 
		* @param position 描画する位置（左上の座標を指定）
		* @param fontHeight 描画するフォントの高さ
		* @param colorCoeff テクスチャの色係数（RGBAを指定）
		* @param backgroundColor 背景色（RGBAを指定）
		* @param depth デプス値(他のdebugString系関数と整合を取るには、1を設定してください)
		* @param format C言語スタイルの書式指定された文字列(NULL終端のChar配列)
		* @param ... 可変個引数
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details C言語スタイルの書式指定された文字列(NULL終端のChar配列)を画面上に描画します 
		*/
	int	drawDebugStringf(
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		float	fontHeight,
		sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
		sce::Vectormath::Simd::Aos::Vector4_arg	backgroundColor,
		float	depth,
		const char	*format, ...);

	/*!
		* @~English
		* @brief Drawing a debug character string 
		* @param position Position of drawing (Specifies the upper left coordinate)
		* @param fontHeight Height of the font drawn
		* @param colorCoeff Color coefficient of a texture (Specifies RGBA)
		* @param format Character string designated in the C Language style (Char array terminated with NULL)
		* @param ... Variable arguments
		* @retval SCE_OK Success
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
		* @details This draws a character string designated in the C Language style (Char array terminated with NULL) on the screen. 
		* @~Japanese
		* @brief デバッグ文字列の描画 
		* @param position 描画する位置（左上の座標を指定）
		* @param fontHeight 描画するフォントの高さ
		* @param colorCoeff テクスチャの色係数（RGBAを指定）
		* @param format C言語スタイルの書式指定された文字列(NULL終端のChar配列)
		* @param ... 可変個引数
		* @retval SCE_OK 成功
		* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
		* @details C言語スタイルの書式指定された文字列(NULL終端のChar配列)を画面上に描画します 
		*/
	int	drawDebugStringf(
		sce::Vectormath::Simd::Aos::Vector2_arg	position,
		float	fontHeight,
		sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
		const char	*format, ...);

	/*!
		* @~English
		* @brief Gets width of a debug character. 
		* @param charHeight Height of a debug character
		* 
		* @retval float Width of a debug character
		* @details Returns character width for the designated height of specified characters in the current render target. 
		* @~Japanese
		* @brief デバッグ文字の幅を取得 
		* @param charHeight デバッグ文字の高さ
		* 
		* @retval float デバッグ文字の幅
		* @details 現在のレンダーターゲット内での、指定された文字の高さに対する文字幅を返します 
		*/
	float	getWidthOfDebugChar(float	charHeight);

	/*!
	 * @~English
	 * @brief Cube render 
	 * @param dcb Pointer to DrawCommandBuffer
	 * @param world Transformation matrix for world coordinate system
	 * @param view View matrix
	 * @param projection Projection matrix
	 * @param lightPosition Point light position
	 * @param color Color
	 * @param ambient Ambient value for phone shader
	 * @param shininess Shininess of phone shader
	 * 
	 * @retval SCE_OK Success
	 * @retval <SCE_OK Error
	 * @details Renders a filled cube in a 3D space. The original cube will have the center as the starting point, the length of each side will be 1, and all sides will be parallel to the coordinate axes. 
	 * @~Japanese
	 * @brief 立方体の描画 
	 * @param dcb DrawCommandBufferへのポインタ
	 * @param world world座標系への変換行列
	 * @param view 視点行列
	 * @param projection 射影行列
	 * @param lightPosition ポイントライトの位置
	 * @param color 色
	 * @param ambient phoneシェーダのアンビエント値
	 * @param shininess phoneシェーダの輝度
	 * 
	 * @retval SCE_OK 成功
	 * @retval <SCE_OK エラー
	 * @details 3D空間に塗りつぶされた立方体を描画します。元の立方体は中心が原点、各辺の長さが１、全ての辺が座標軸に平行な立方体です。 
	 */
	int fillCube(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Matrix4_arg world,
		sce::Vectormath::Simd::Aos::Matrix4_arg view,
		sce::Vectormath::Simd::Aos::Matrix4_arg projection,
		sce::Vectormath::Simd::Aos::Vector3_arg lightPosition,
		sce::Vectormath::Simd::Aos::Vector4_arg color,
		float ambient,
		float shininess,
		const sce::Gnmx::PsShader *fillPs = SHADER(sprite_utility_3d_fill_p),
		uint32_t psSrtSizeInDw = SIZE_OF_SRT(sprite_utility_3d_fill_p),
		const void	*fillUserData = nullptr);

	/*!
	 * @~English
	 * @brief Cube render 
	 * @param dcb Pointer to DrawCommandBuffer
	 * @param world Transformation matrix for world coordinate system
	 * @param view View matrix
	 * @param projection Projection matrix
	 * @param color Color
	 * @param lineWidth Line width
	 * 
	 * @retval SCE_OK Success
	 * @retval <SCE_OK Error
	 * @details Renders a wire frame cube in a 3D space. The original cube will have the center as the starting point, the length of each side will be 1, and all sides will be parallel to the coordinate axes. 
	 * @~Japanese
	 * @brief 立方体の描画 
	 * @param dcb DrawCommandBufferへのポインタ
	 * @param world world座標系への変換行列
	 * @param view 視点行列
	 * @param projection 射影行列
	 * @param color 色
	 * @param lineWidth 線の太さ
	 * 
	 * @retval SCE_OK 成功
	 * @retval <SCE_OK エラー
	 * @details 3D空間にワイヤフレームの立方体を描画します。元の立方体は中心が原点、各辺の長さが１、全ての辺が座標軸に平行な立方体です。 
	 */
	int drawCube(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Matrix4_arg world,
		sce::Vectormath::Simd::Aos::Matrix4_arg view,
		sce::Vectormath::Simd::Aos::Matrix4_arg projection,
		sce::Vectormath::Simd::Aos::Vector4_arg color,
		uint32_t lineWidth);

	/*!
	 * @~English
	 * @brief Sphere rendering 
	 * @param dcb Pointer to DrawCommandBuffer
	 * @param world Transformation matrix for world coordinate system
	 * @param view View matrix
	 * @param projection Projection matrix
	 * @param lightPosition Point light position
	 * @param color Color
	 * @param ambient Ambient value for phone shader
	 * @param shininess Shininess of phone shader
	 * 
	 * @retval SCE_OK Success
	 * @retval <SCE_OK Error
	 * @details Renders a colored-in sphere in 3D space. The original sphere has its center at the origin with a radius of 0.5. 
	 * @~Japanese
	 * @brief 球の描画 
	 * @param dcb DrawCommandBufferへのポインタ
	 * @param world world座標系への変換行列
	 * @param view 視点行列
	 * @param projection 射影行列
	 * @param lightPosition ポイントライトの位置
	 * @param color 色
	 * @param ambient phoneシェーダのアンビエント値
	 * @param shininess phoneシェーダの輝度
	 * 
	 * @retval SCE_OK 成功
	 * @retval <SCE_OK エラー
	 * @details 3D空間に塗りつぶされた球を描画します。元の球は中心が原点、半径0.5です。 
	 */
	int fillSphere(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Matrix4_arg world,
		sce::Vectormath::Simd::Aos::Matrix4_arg view,
		sce::Vectormath::Simd::Aos::Matrix4_arg projection,
		sce::Vectormath::Simd::Aos::Vector3_arg lightPosition,
		sce::Vectormath::Simd::Aos::Vector4_arg color,
		float ambient,
		float shininess,
		const sce::Gnmx::PsShader *fillPs = SHADER(sprite_utility_3d_fill_p),
		uint32_t psSrtSizeInDw = SIZE_OF_SRT(sprite_utility_3d_fill_p),
		const void	*fillUserData = nullptr);

	/*!
	 * @~English
	 * @brief Sphere rendering 
	 * @param dcb Pointer to DrawCommandBuffer
	 * @param world Transformation matrix for world coordinate system
	 * @param view View matrix
	 * @param projection Projection matrix
	 * @param color Color
	 * @param lineWidth Line width
	 * 
	 * @retval SCE_OK Success
	 * @retval <SCE_OK Error
	 * @details Renders a wire frame sphere in 3D space. The original sphere has its center at the origin with a diameter of 1. 
	 * @~Japanese
	 * @brief 球の描画 
	 * @param dcb DrawCommandBufferへのポインタ
	 * @param world world座標系への変換行列
	 * @param view 視点行列
	 * @param projection 射影行列
	 * @param color 色
	 * @param lineWidth 線の太さ
	 * 
	 * @retval SCE_OK 成功
	 * @retval <SCE_OK エラー
	 * @details 3D空間にワイヤフレームの球を描画します。元の球は中心が原点、直径が１の球です。 
	 */
	int drawSphere(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Matrix4_arg world,
		sce::Vectormath::Simd::Aos::Matrix4_arg view,
		sce::Vectormath::Simd::Aos::Matrix4_arg projection,
		sce::Vectormath::Simd::Aos::Vector4_arg color,
		uint32_t lineWidth);


	/*!
	 * @~English
	 * @brief Cylinder rendering 
	 * @param dcb Pointer to DrawCommandBuffer
	 * @param world Transformation matrix for world coordinate system
	 * @param view View matrix
	 * @param projection Projection matrix
	 * @param lightPosition Point light position
	 * @param color Color
	 * @param ambient Ambient value for phone shader
	 * @param shininess Shininess of phone shader
	 * 
	 * @retval SCE_OK Success
	 * @retval <SCE_OK Error
	 * @details Renders a colored-in cylinder in 3D space. The original cylinder has its circle center at the origin with a radius of 0.5, and a height of 1.0. 
	 * @~Japanese
	 * @brief 円柱の描画 
	 * @param dcb DrawCommandBufferへのポインタ
	 * @param world world座標系への変換行列
	 * @param view 視点行列
	 * @param projection 射影行列
	 * @param lightPosition ポイントライトの位置
	 * @param color 色
	 * @param ambient phoneシェーダのアンビエント値
	 * @param shininess phoneシェーダの輝度
	 * 
	 * @retval SCE_OK 成功
	 * @retval <SCE_OK エラー
	 * @details 3D空間に塗りつぶされた円柱を描画します。元の円柱は中心が原点、円の半径が0.5、高さが1.0です。 
	 */
	int fillCylinder(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Matrix4_arg world,
		sce::Vectormath::Simd::Aos::Matrix4_arg view,
		sce::Vectormath::Simd::Aos::Matrix4_arg projection,
		sce::Vectormath::Simd::Aos::Vector3_arg lightPosition,
		sce::Vectormath::Simd::Aos::Vector4_arg color,
		float ambient,
		float shininess,
		const sce::Gnmx::PsShader *fillPs = SHADER(sprite_utility_3d_fill_p),
		uint32_t psSrtSizeInDw = SIZE_OF_SRT(sprite_utility_3d_fill_p),
		const void	*fillUserData = nullptr);


	/*!
	 * @~English
	 * @brief Cylinder rendering 
	 * @param dcb Pointer to DrawCommandBuffer
	 * @param world Transformation matrix for world coordinate system
	 * @param view View matrix
	 * @param projection Projection matrix
	 * @param color Color
	 * @param lineWidth Line width
	 * 
	 * @retval SCE_OK Success
	 * @retval <SCE_OK Error
	 * @details Renders a wire frame cylinder in 3D space. The original cylinder has its circle center at the origin with a radius of 0.5, and a height of 1.0. 
	 * @~Japanese
	 * @brief 円柱の描画 
	 * @param dcb DrawCommandBufferへのポインタ
	 * @param world world座標系への変換行列
	 * @param view 視点行列
	 * @param projection 射影行列
	 * @param color 色
	 * @param lineWidth 線の太さ
	 * 
	 * @retval SCE_OK 成功
	 * @retval <SCE_OK エラー
	 * @details 3D空間にワイヤフレームの円柱を描画します。元の円柱は中心が原点、円の半径が0.5、高さが1.0です。 
	 */
	int drawCylinder(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Matrix4_arg world,
		sce::Vectormath::Simd::Aos::Matrix4_arg view,
		sce::Vectormath::Simd::Aos::Matrix4_arg projection,
		sce::Vectormath::Simd::Aos::Vector4_arg color,
		uint32_t lineWidth);

	/*!
	 * @~English
	 * @brief Cone rendering 
	 * @param dcb Pointer to DrawCommandBuffer
	 * @param world Transformation matrix for world coordinate system
	 * @param view View matrix
	 * @param projection Projection matrix
	 * @param lightPosition Point light position
	 * @param color Color
	 * @param ambient Ambient value for phone shader
	 * @param shininess Shininess of phone shader
	 * 
	 * @retval SCE_OK Success
	 * @retval <SCE_OK Error
	 * @details Renders a colored-in cone in 3D space. The original cone has its circle center at the origin with a radius of 0.5, and a height of 1.0. 
	 * @~Japanese
	 * @brief 円錐の描画 
	 * @param dcb DrawCommandBufferへのポインタ
	 * @param world world座標系への変換行列
	 * @param view 視点行列
	 * @param projection 射影行列
	 * @param lightPosition ポイントライトの位置
	 * @param color 色
	 * @param ambient phoneシェーダのアンビエント値
	 * @param shininess phoneシェーダの輝度
	 * 
	 * @retval SCE_OK 成功
	 * @retval <SCE_OK エラー
	 * @details 3D空間に塗りつぶされた円錐を描画します。元の円錐は中心が原点、円の半径が0.5、高さが1.0です。 
	 */
	int fillCone(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Matrix4_arg world,
		sce::Vectormath::Simd::Aos::Matrix4_arg view,
		sce::Vectormath::Simd::Aos::Matrix4_arg projection,
		sce::Vectormath::Simd::Aos::Vector3_arg lightPosition,
		sce::Vectormath::Simd::Aos::Vector4_arg color,
		float ambient,
		float shininess,
		const sce::Gnmx::PsShader *fillPs = SHADER(sprite_utility_3d_fill_p),
		uint32_t psSrtSizeInDw = SIZE_OF_SRT(sprite_utility_3d_fill_p),
		const void	*fillUserData = nullptr);


	/*!
	 * @~English
	 * @brief Cone rendering 
	 * @param dcb Pointer to DrawCommandBuffer
	 * @param world Transformation matrix for world coordinate system
	 * @param view View matrix
	 * @param projection Projection matrix
	 * @param color Color
	 * @param lineWidth Line width
	 * 
	 * @retval SCE_OK Success
	 * @retval <SCE_OK Error
	 * @details Renders a wire frame cone in 3D space. The original cone has its circle center at the origin with a radius of 0.5, and a height of 1.0. 
	 * @~Japanese
	 * @brief 円錐の描画 
	 * @param dcb DrawCommandBufferへのポインタ
	 * @param world world座標系への変換行列
	 * @param view 視点行列
	 * @param projection 射影行列
	 * @param color 色
	 * @param lineWidth 線の太さ
	 * 
	 * @retval SCE_OK 成功
	 * @retval <SCE_OK エラー
	 * @details 3D空間にワイヤフレームの円錐を描画します。元の円錐は中心が原点、円の半径が0.5、高さが1.0です。 
	 */
	int drawCone(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Matrix4_arg world,
		sce::Vectormath::Simd::Aos::Matrix4_arg view,
		sce::Vectormath::Simd::Aos::Matrix4_arg projection,
		sce::Vectormath::Simd::Aos::Vector4_arg color,
		uint32_t lineWidth);

	/*!
	 * @~English
	 * @brief Draws a line segment. 
	 * @param dcb Pointer to DrawCommandBuffer
	 * @param view View matrix
	 * @param projection Projection matrix
	 * @param begin Line segment beginning position
	 * @param end Line segment ending position
	 * @param color Color
	 * @param lineWidth Line width
	 * 
	 * @retval SCE_OK Success
	 * @retval <SCE_OK Error
	 * @details Renders a line segment in 3D space. 
	 * @~Japanese
	 * @brief 線分の描画 
	 * @param dcb DrawCommandBufferへのポインタ
	 * @param view 視点行列
	 * @param projection 射影行列
	 * @param begin 線分の開始位置
	 * @param end 線分の終了位置
	 * @param color 色
	 * @param lineWidth 線の太さ
	 * 
	 * @retval SCE_OK 成功
	 * @retval <SCE_OK エラー
	 * @details 3D空間に線分を描画します。 
	 */
	int drawLine(
		sce::Gnm::DrawCommandBuffer *dcb,
		sce::Vectormath::Simd::Aos::Matrix4_arg view,
		sce::Vectormath::Simd::Aos::Matrix4_arg projection,
		sce::Vectormath::Simd::Aos::Vector3_arg begin,
		sce::Vectormath::Simd::Aos::Vector3_arg end,
		sce::Vectormath::Simd::Aos::Vector4_arg color,
		uint32_t lineWidth);


	/*!
	 * @~English
	 * @brief Draws a texture 
	 * @param dcb Pointer to DrawCommandBuffer
	 * @param model Transformation matrix for world coordinate system
	 * @param view View matrix
	 * @param projection Projection matrix
	 * @param textureOffset Offset within the texture
	 * @param sizeInTexture Size in the texture
	 * @param texture Reference to Texture
	 * @param sampler Reference to Texture-Sampler that will be used to sample 'texture'
	 * @param colorCoeff Color coefficient of a texture (Specifies RGBA)
	 * 
	 * @retval SCE_OK Success
	 * @retval <SCE_OK Error
	 * @details This draws a texture. 
	 * @~Japanese
	 * @brief テクスチャの描画 
	 * @param dcb DrawCommandBufferへのポインタ
	 * @param model world座標系への変換行列
	 * @param view 視点行列
	 * @param projection 射影行列
	 * @param textureOffset テクスチャ内のオフセット
	 * @param sizeInTexture テクスチャ内のサイズ
	 * @param texture Textureへの参照
	 * @param sampler 'texture'をサンプリングするために使用されるテクスチャサンプラーへの参照
	 * @param colorCoeff テクスチャの色係数（RGBAを指定）
	 * 
	 * @retval SCE_OK 成功
	 * @retval <SCE_OK エラー
	 * @details テクスチャを描画します。 
	 */
	int	drawTexture(
		sce::Gnm::DrawCommandBuffer	*dcb,
		sce::Vectormath::Simd::Aos::Matrix4_arg model,
		sce::Vectormath::Simd::Aos::Matrix4_arg view,
		sce::Vectormath::Simd::Aos::Matrix4_arg projection,
		sce::Vectormath::Simd::Aos::Vector2_arg	textureOffset,
		sce::Vectormath::Simd::Aos::Vector2_arg	sizeInTexture,
		const sce::Gnm::Texture	&texture, 
		const sce::Gnm::Sampler	&sampler,
		sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff = sce::Vectormath::Simd::Aos::Vector4(1.f)
		);


	/*!
	 * @~English
	 * @brief Draws Quads
	 * @param dcb Pointer to DrawCommandBuffer
	 * @param view View matrix
	 * @param projection Projection matrix
	 * @param models Pointer to transformation matrices for world coordinate system
	 * @param numInstances Number of models to render
	 * @param psShader Pointer to PixelShader for rending
	 * @param psUserData Pointer to UserData at Pixel-shading
	 * @param psUserDataSizeInDw Size of UserData at Pixel-shading(in Dwords)
	 * @param isModelsBufferManagedByUser Specifiy whether 'models' buffer is allocated by user. If Specify 'false', 'models' will be copied in internally.
	 *
	 * @retval SCE_OK Success
	 * @retval <SCE_OK Error
	 * @details This draws a texture. 
	 * @~Japanese
	 * @brief 矩形描画
	 * @param dcb DrawCommandBufferへのポインタ
	 * @param view 視点行列
	 * @param projection 射影行列
	 * @param models world座標系への変換行列へのポインタ
	 * @param numInstances レンダリングするモデル数
	 * @param psShader レンダリングのためのピクセルシェーダへのポインタ
	 * @param psUserData ピクセルシェーディングで使用されるユーザデータ
	 * @param psUserDataSizeInDw ピクセルシェーディングで使用されるユーザデータのサイズ(Dwords)
	 * @param isModelsBufferManagedByUser 'models'に指定したメモリがユーザによって確保されたメモリであるかどうかを指定する。「false」が指定された場合は、内部で'models'データのコピーが行われる
	 * 
	 * 
	 * @retval SCE_OK 成功
	 * @retval <SCE_OK エラー
	 * @details テクスチャを描画します。 
	 */
	int renderQuads(
		sce::Gnm::DrawCommandBuffer	*dcb,
		sce::Vectormath::Simd::Aos::Matrix4_arg view,
		sce::Vectormath::Simd::Aos::Matrix4_arg projection,
		const sce::Vectormath::Simd::Aos::Matrix4 * models,
		uint32_t numModels,
		const sce::Gnmx::PsShader *psShader,
		const void *psUserData,
		uint32_t psUserDataSizeInDw,
		bool isModelsBufferManagedByUser
	);

	/*!
	 * @~English
	 * @brief Deferred generate draw string ImGui Commands
	 * @details Generate draw string ImGui commnads deferred until this function is called
	 * @~Japanese
	 * @brief 文字列描画ImGuiコマンド生成の遅延
	 * @details この関数呼び出しまで遅延された文字列描画ImGuiコマンドの生成を行う
	 */
	void	deferredGenerateDrawStringImguiCommands();
} // namespace SpriteUtil
} } } // namespace SampleUtilGraphics
#endif