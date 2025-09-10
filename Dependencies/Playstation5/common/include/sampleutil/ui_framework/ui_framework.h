/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#include <mspace.h>
#include <scebase_common.h>

#include <sampleutil/input.h>

#if _SCE_TARGET_OS_PROSPERO
#include <agc/drawcommandbuffer.h>
#include "imgui/platform_agc/imgui_impl_agc.h"
#endif
#if _SCE_TARGET_OS_ORBIS
#include <gnmx/context.h>
#include "imgui/platform_gnm/imgui_impl_gnm.h"
#endif

namespace sce
{
	namespace SampleUtil
	{
		/*!
		 * @~English
		 * @brief UIFramework-associated definitions 
		 * @details sce::SampleUtil::UIFrameowrk is the name space associated with the input of the SampleUtil library. 
		 * @~Japanese
		 * @brief UIFramework関連の定義 
		 * @details sce::SampleUtil::UIFrameworkはSampleUtilライブラリの入力関連の名前空間です。 
		 */
		namespace UIFramework
		{
			/*!
			 * @~English
			 * 
			 * @brief ImGui option 
			 * @details This is the ImGui option. 
			 * @~Japanese
			 * 
			 * @brief ImGuiオプション 
			 * @details ImGuiオプションです。 
			 */
			struct ImGuiOption
			{
				/*!
				 * @~English
				 * @brief Default heap memory size(in bytes)
				 * @~Japanese
				 * @brief 既定のヒープメモリサイズ(バイト)
				 */
				static const size_t	kDefaultHeapMemorySizeInBytes = 30 * 1024 * 1024;

				/*!
				 * @~English
				 * @brief Specify the amount of memory for memory allocator used for ImGui(kDefaultHeapMemorySizeInBytes is specified if 0 is specified)
				 * @~Japanese
				 * @brief ImGuiで使用するメモリアロケータに与えるメモリ量を指定します(0を指定するとkDefaultHeapMemorySizeInBytesが指定されます)
				 */
				size_t	heapMemorySize;
				/*!
				* @~English
				* @brief Flag for drawing ImGui with depth. (False by default)
				* @~Japanese
				* @brief ImGuiでDepthを有効にするかどうかのフラグ（デフォルトは無効） 
				*/
				bool	enableDepth;
				/*!
				* @~English
				* @brief Disable ImGui mousu operation(Enabled by default)
				* @~Japanese
				* @brief ImGuiのマウス操作を無効化（デフォルトは有効）
				*/
				bool	disableMouse;
			};

			/*!
			 * @~English
			 * 
			 * @brief ImGui update parameter
			 * @details This is the parameter to specify when calling ImGuiContext::newFrame()
			 * @~Japanese
			 * 
			 * @brief ImGui更新パラメータ
			 * @details ImGuiContext::newFrame()に指定する引数
			 */
			struct ImGuiUpdateParam
			{
				/*!
				 * @~English
				 * 
				 * @brief PadContext used for ImGui manipulation
				 * @~Japanese
				 * 
				 * @brief ImGuiの操作に使用するPadContext
				 */
				Input::PadContext		*pad;
				/*!
				 * @~English
				 * 
				 * @brief KeyboardContext used for ImGui manipulation and input
				 * @~Japanese
				 * 
				 * @brief ImGuiの操作・入力に使用するKeyboardContext
				 */
				Input::KeyboardContext	*keyboard;
				/*!
				 * @~English
				 * 
				 * @brief OskContext used for ImGui input
				 * @~Japanese
				 * 
				 * @brief ImGuiの入力に使用するOskContext
				 */
				Input::OskContext		*osk;
				/*!
				* @~English
				*
				* @brief Width of render target
				* @~Japanese
				*
				* @brief レンダーターゲットの幅
				*/
				uint32_t				renderTargetWidth;
				/*!
				* @~English
				*
				* @brief Height of render target
				* @~Japanese
				*
				* @brief レンダーターゲットの高さ
				*/
				uint32_t				renderTargetHeight;
#if _SCE_TARGET_OS_PROSPERO
				/*!
				* @~English
				*
				* @brief Specify true if VR controller is used for ImGui
				* @~Japanese
				*
				* @brief ImGui操作にVRコントローラを使う場合にtrue
				*/
				bool					isVrController;
				/*!
				* @~English
				*
				* @brief Left hand VR controller used for ImGui manipulation
				* @~Japanese
				*
				* @brief ImGuiの操作に使用する左手VRコントローラ
				*/
				Input::PadContext		*vrControllerLeft;
#endif
			};

			/*!
			* @~English
			*
			* @brief ImGui initialize
			* @details Call this function once before starting use of ImGui
			* @param option ImGui initialize option(initialized with default value if nullptr is specified)
			* @param memoryAllocator Heap memory allocator used by ImGui(direct memory is allocated in ImGui if nullptr is specified)
			* @param pVideoMemoryAllocator Video memory allocator used for ImGui rendering(direct memory is allocated in ImGui if nullptr is specified)
			* @param pVideoMemoryRingAllocator Video memory ring allocator used for ImGui rendering(allocated from command buffer by allocateTopDown if nullptr is specified)
			* @param idManagerForMouse Used for binding mouse used for ImGui manipulation(no mouse is bound if nullptr is specified)
			* @retval SCE_OK Success
			* @retval (<0) Error code
			* @~Japanese
			*
			* @brief ImGuiの初期化
			* @details ImGuiの使用開始前に一度だけ呼び出します
			* @param option ImGui初期化オプション(nullptrを指定した場合には既定値で初期化されます)
			* @param memoryAllocator ImGui内部で使用するヒープメモリアロケータ(nullptrを指定した場合には内部でダイレクトメモリが割り当てられます)
			* @param pVideoMemoryAllocator ImGuiレンダリングで使用するビデオメモリアロケータ(nullptrを指定した場合には内部でダイレクトメモリが割り当てられます)
			* @param pVideoMemoryRingAllocator ImGuiレンダリングで使用するビデオメモリリングアロケータ(nullptrを指定した場合にはコマンドバッファからallocateTopDownで割り当てたメモリを使用します)
			* @param idManagerForMouse ImGui操作で使用するマウスをバインドする際に使用します(nullptrを指定した場合にImGuiはマウスをバインドしません)
			* @retval SCE_OK 成功
			* @retval (<0) エラーコード
			*/
			int	imGuiInitialize(const ImGuiOption	*option = nullptr, SceLibcMspace	memoryAllocator = nullptr, Graphics::VideoAllocator	*pVideoMemoryAllocator = nullptr, Graphics::VideoRingAllocator	*pVideoMemoryRingAllocator = nullptr, System::UserIdManager	*idManagerForMouse = nullptr);

			/*!
			* @~English
			*
			* @brief ImGui finalize
			* @details Releases resources allocated for ImGui
			* @retval SCE_OK Success
			* @retval (<0) Error code
			* @~Japanese
			*
			* @brief ImGuiの終了処理
			* @details ImGuiで確保したリソースを解放します
			* @retval SCE_OK 成功
			* @retval (<0) エラーコード
			*/
			int imGuiFinalize();

			/*!
			* @~English
			*
			* @brief Starts ImGui new frame
			* @param param ImGui update parameters such as pad,keyboard,osk and screen size
			* @retval SCE_OK Success
			* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_STATE imGuiStartNewFrame is called before ending previous frame by calling imGuiEndFrame
			* @retval SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED imGuiInitialize is never called
			* @~Japanese
			*
			* @brief ImGuiの新しいフレームの開始
			* @param param pad,keyboard,oskや画面サイズなどのパラメータを指定
			* @retval SCE_OK 成功
			* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_STATE imGuiEndFrameを呼んで前のフレームを終了する前にimGuiStartNewFrameが呼ばれた
			* @retval SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED imGuiInitializeが呼ばれていない
			*/
			int	imGuiStartNewFrame(const ImGuiUpdateParam	&param);


			/*!
			* @~English
			*
			* @brief Ends ImGui frame
			* @retval SCE_OK Success
			* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_STATE imGuiEndFrame is called without starting frame by calling imGuiStartNewFrame
			* @~Japanese
			*
			* @brief ImGuiのフレーム終了
			* @retval SCE_OK 成功
			* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_STATE imGuiStartNewFrameでフレームを開始しない状態でimGuiEndFrameが呼ばれた
			*/
			int	imGuiEndFrame();

			/*!
			* @~English
			*
			* @brief Render ImGui
			* @param dcb Command buffer for ImGui rendering
			* @retval SCE_OK Success
			* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_STATE ImGui frame is not ended
			* @retval SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED imGuiInitialize is never called

			*
			* @brief ImGuiの描画
			* @param dcb 描画に使用するコマンドバッファ
			* @retval SCE_OK 成功
			* @retval SCE_SAMPLE_UTIL_ERROR_INVALID_STATE ImGuiのフレームが終了していない
			* @retval SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED imGuiInitializeが呼ばれていない
			*/
#if _SCE_TARGET_OS_PROSPERO
			int	imGuiRender(sce::Agc::DrawCommandBuffer	&dcb);
#endif
#if _SCE_TARGET_OS_ORBIS
			int	imGuiRender(sce::Gnmx::GfxContext	&gfxc);
#endif
		}
	}
}
