/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2021 Sony Interactive Entertainment Inc. 
 * 
 */
// dear imgui: Renderer for Agc

// Implemented features:
//  [X] Renderer: User texture binding. Use 'ID3D11ShaderResourceView*' as ImTextureID. Read the FAQ about ImTextureID in imgui.cpp.
//  [ ] Platform: Clipboard support (for Win32 this is actually part of core imgui)
//  [ ] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Keyboard arrays indexed using VK_* Virtual Key Codes, e.g. ImGui::IsKeyPressed(VK_SPACE).
//  [X] Platform: Gamepad support (best leaving it to user application to fill io.NavInputs[] with gamepad inputs from their source of choice).

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#pragma once
#include "../source/imgui/imgui.h"
#include "sampleutil/ui_framework/imgui/imgui_libfont.h"
#include "sampleutil/graphics/graphics_memory.h"
#include "sampleutil/input.h"
#include <mutex>
#include <agc/drawcommandbuffer.h>

//! @defgroup imgui ImGui
//! @{


/*!
 * @~English
 * @brief Mutex which needs to be locked when accessing console output write buffer
 * @~Japanese
 * @brief コンソール出力書き込みバッファアクセス用mutex
 */
extern std::mutex ImGui_ImplSampleUtil_writeString_lock;
/*!
 * @~English
 * @brief Console output write buffer
 * @~Japanese
 * @brief コンソール出力書き込みバッファ
 */
extern char *ImGui_ImplSampleUtil_writeString;
/*!
 * @~English
 * @brief Specify if IME dialog focus is kept
 * @~Japanese
 * @brief IMEダイアログのフォーカスを維持し続けるかどうかを指定
 */
extern bool ImGui_ImplSampleUtil_keepOskFocus;
/*!
 * @~English
 * @brief Specify true if you want to implement ImGui Pad control without using sce::SampleUtil::Input::PadContext.
 * @~Japanese
 * @brief sce::SampleUtil::Input::PadContext以外の方法でImGuiのPad操作を実装する際にtrueを指定
 */
extern bool ImGui_ImplSampleUtil_useCustomPad;
/*!
 * @~English
 * @brief Specify true if you want to implement ImGui key control without using sce::SampleUtil::Input::KeyContext.
 * @~Japanese
 * @brief sce::SampleUtil::Input::KeyboardContext以外の方法でImGuiのキー操作を実装する際にtrueを指定
 */
extern bool ImGui_ImplSampleUtil_useCustomKeyboard;
/*!
 * @~English
 * @brief ImGui default font size
 * @~Japanese
 * @brief ImGuiの基底フォントサイズ
 */
extern uint32_t ImGui_ImplSampleUtil_defaultFontSize;
/*!
 * @~English
 * @brief ImGui default font
 * @~Japanese
 * @brief ImGuiの基底フォント
 */
extern ImFont *ImGui_ImplSampleUtil_defaultFont;

/*!
 * @~English
 * @brief Obtains Keyboard context to which ImGui owns focus
 * @return If ImGui owns keyboard focus, returns its Keyboard context, otherwise, returns nullptr.
 * @~Japanese
 * @brief ImGuiでフォーカスのあるKeyboardコンテキストを取得
 * @return フォーカスがある場合は、そのKeyboardコンテキストを返す。そうでない場合はnullptrを返す。
 */
sce::SampleUtil::Input::KeyboardContext *ImGui_ImplSampleUtil_GetKeyboardContextInFocus();
/*!
 * @~English
 * @brief Obtains Pad context to which ImGui owns focus
 * @return If ImGui owns pad focus, returns its Pad context, otherwise, returns nullptr.
 * @~Japanese
 * @brief ImGuiでフォーカスのあるPadコンテキストを取得
 * @return フォーカスがある場合は、そのPadコンテキストを返す。そうでない場合はnullptrを返す。
 */
sce::SampleUtil::Input::PadContext *ImGui_ImplSampleUtil_GetPadContextInFocus();
/*!
 * @~English
 * @brief Obtains Osk context to which ImGui owns focus
 * @return If ImGui owns Osk focus, returns its Osk context, otherwise, returns nullptr.
 * @~Japanese
 * @brief ImGuiでフォーカスのあるOskコンテキストを取得
 * @return フォーカスがある場合は、そのOskコンテキストを返す。そうでない場合はnullptrを返す。
 */
sce::SampleUtil::Input::OskContext *ImGui_ImplSampleUtil_GetOskContextInFocus();
/*!
 * @~English
 * @brief Initializes ImGui
 * @param videoMemory Video memory allocator
 * @param videoRingMemory Video memory ring allocator
 * @param pIdManager UserID manager
 * @param disableMouse It invalidates Mouse for ImGui
 * @retval true Success
 * @retval false Failure
 * @~Japanese
 * @brief ImGui初期化
 * @param videoMemory ビデオメモリアロケータ
 * @param videoRingMemory ビデオメモリリングアロケータ
 * @param pIdManager ユーザーIDマネージャ
 * @param disableMouse ImGuiのマウスを無効化するか
 * @retval true 成功
 * @retval false 失敗
 */
bool     ImGui_ImplSampleUtil_Init(sce::SampleUtil::Graphics::VideoAllocator *videoMemory, sce::SampleUtil::Graphics::VideoRingAllocator *videoRingMemory, sce::SampleUtil::System::UserIdManager *pIdManager, bool disableMouse = false);
/*!
 * @~English
 * @brief Finalizes ImGui
 * @~Japanese
 * @brief ImGui終了処理
 */
void     ImGui_ImplSampleUtil_Shutdown();
/*!
 * @~English
 * @brief Starts new frame
 * @details Call this before starting pushing ImGui commands
 * @param displayWidth Display width
 * @param displayHeight Display height
 * @param keyboardContext Keyboard context to be used for ImGui control(nullptr if ImGui doesn't have keyboard focus)
 * @param padContext Pad context to be used for ImGui control(nullptr if ImGui doesn't have Pad focus)
 * @param oskContext Osk context to be used for ImGui control(nullptr if ImGui doesn't have IME dialog focus)
 * @param isVrController Use VR controller for ImGui control
 * @param vrControllerLeft Pad context of left hand VR controller for ImGui control
 * @~Japanese
 * @brief 新しいフレームの開始
 * @details ImGuiコマンドを積み始める前に呼び出します。
 * @param displayWidth ディスプレイの幅
 * @param displayHeight ディスプレイの高さ
 * @param keyboardContext ImGuiのキー操作に使用するKeyboardコンテキスト(ImGuiにキーボードフォーカスがない場合はnullptrを指定)
 * @param padContext ImGuiのキー操作に使用するPadコンテキスト(ImGuiにPadフォーカスがない場合はnullptrを指定)
 * @param oskContext ImGuiのキー操作に使用するOskコンテキスト(ImGuiにIME dialogのフォーカスがない場合はnullptrを指定)
 * @param isVrController ImGuiのキー操作にVRコントローラを使用
 * @param vrControllerLeft ImGuiのキー操作に使用する左手用VRコントローラPadコンテキスト
 */
void     ImGui_ImplSampleUtil_NewFrame(uint32_t	displayWidth, uint32_t	displayHeight, sce::SampleUtil::Input::KeyboardContext *keyboardContext, sce::SampleUtil::Input::PadContext *padContext, sce::SampleUtil::Input::OskContext *oskContext, bool isVrController = false, sce::SampleUtil::Input::PadContext *vrControllerLeft = nullptr);
/*!
 * @~English
 * @brief Draws ImGui
 * @param dcb DrawCommandBuffer to which draw commands are to be pushed
 * @param draw_data ImGui command buffer
 * @~Japanese
 * @brief ImGuiの描画
 * @param dcb 描画コマンドを積むDrawCommandBuffer
 * @param draw_data ImGuiコマンドバッファ
 */
void     ImGui_ImplSampleUtil_RenderDrawData(sce::Agc::DrawCommandBuffer *dcb, ImDrawData* draw_data);

// Use if you want to reset your rendering device without losing ImGui state.
/*!
 * @~English
 * @brief Invalidated internal rendering resources for ImGui
 * @~Japanese
 * @brief ImGui用に内部で確保したレンダリング用リソースを開放
 */
void     ImGui_ImplSampleUtil_InvalidateDeviceObjects();
/*!
 * @~English
 * @brief Creates internal rendering resources for ImGui
 * @~Japanese
 * @brief ImGui用に内部レンダリングリソースを確保
 */
bool     ImGui_ImplSampleUtil_CreateDeviceObjects();
//! @}
