/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
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
#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include "../source/imgui/imgui.h"
#include "sampleutil/ui_framework/imgui/imgui_libfont.h"
#include "sampleutil/graphics/graphics_memory.h"
#include "sampleutil/input.h"
#include <mutex>
#include <gnmx/gfxcontext.h>

extern std::mutex ImGui_ImplSampleUtil_writeString_lock;
extern char *ImGui_ImplSampleUtil_writeString;
extern bool ImGui_ImplSampleUtil_keepOskFocus;
extern bool ImGui_ImplSampleUtil_useCustomPad;
extern bool ImGui_ImplSampleUtil_useCustomKeyboard;
extern uint32_t ImGui_ImplSampleUtil_defaultFontSize;
extern ImFont *ImGui_ImplSampleUtil_defaultFont;

sce::SampleUtil::Input::KeyboardContext *ImGui_ImplSampleUtil_GetKeyboardContextInFocus();
sce::SampleUtil::Input::PadContext *ImGui_ImplSampleUtil_GetPadContextInFocus();
sce::SampleUtil::Input::OskContext *ImGui_ImplSampleUtil_GetOskContextInFocus();
bool     ImGui_ImplSampleUtil_Init(sce::SampleUtil::Graphics::VideoAllocator &videoMemory, sce::SampleUtil::System::UserIdManager *pIdManager, bool disableMouse = false);
void     ImGui_ImplSampleUtil_Shutdown();
void     ImGui_ImplSampleUtil_NewFrame(uint32_t	displayWidth, uint32_t	displayHeight, sce::SampleUtil::Input::KeyboardContext *keyboardContext, sce::SampleUtil::Input::PadContext *padContext, sce::SampleUtil::Input::OskContext *oskContext);
void     ImGui_ImplSampleUtil_RenderDrawData(sce::Gnmx::GfxContext *gfxc, ImDrawData *draw_data);

// Use if you want to reset your rendering device without losing ImGui state.
void     ImGui_ImplSampleUtil_InvalidateDeviceObjects();
bool     ImGui_ImplSampleUtil_CreateDeviceObjects();
#endif