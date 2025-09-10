/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2022 Sony Interactive Entertainment Inc.
 * 
 */

#include <scebase_common/scebase_target.h>

#if _SCE_TARGET_OS_ORBIS

#include <array>
#include <ces.h>
#include <gnmx.h>
#include <mouse.h>			// for using mouse
#include <vectormath.h>		// for using mouse
#include <libsysmodule.h>	// for using mouse
#include <sampleutil/debug/api_capture.h>

#include "../source/imgui/imgui.h"
#include "../source/imgui/imgui_internal.h"
#include "sampleutil/graphics/graphics_memory.h"
#include "sampleutil/graphics/platform_gnm/shader_gnm.h"
#include "sampleutil/input.h"
#include "sampleutil/ui_framework/imgui/imgui_libfont.h"
#include "sampleutil/ui_framework/imgui/platform_gnm/imgui_impl_gnm.h"

#include "sampleutil/graphics.h"

#include "../shaders/imgui_srt_common.h"


#pragma comment (lib, "libSceCes.a")
#pragma comment(lib, "libSceMouse_stub_weak.a")

namespace vecmath = sce::Vectormath::Simd::Aos;
using namespace sce;

#define BUFFERING		3

DEFINE_SHADER(Gnmx::VsShader, sampleutil_imgui_vv);
DEFINE_SHADER(Gnmx::PsShader, sampleutil_imgui_p);

namespace
{
	class MouseContext
	{
	private:
		SceMouseData	m_mouseData[8];
		int				m_wheel;
		int				m_tilt;
		int32_t			m_mouseHandle;
		int				m_mouseHistory;
		uint32_t		m_frameNoMoved;
		bool			m_setFlag;

	public:
		int				m_mouseX;
		int				m_mouseY;
		bool			m_leftDown;
		bool			m_rightDown;
		bool			m_drag;

	public:
		MouseContext()
			: m_leftDown(false)
			, m_rightDown(false)
			, m_drag(false)
		{}
		~MouseContext() {}

		int initialize(sce::SampleUtil::System::UserId userId)
		{
			int ret = SCE_OK;

			ret = sceSysmoduleLoadModule(SCE_SYSMODULE_MOUSE);
			if (ret != SCE_OK) {
				return ret;
			}

			ret = sceMouseInit();

			SceMouseOpenParam param;
			param.behaviorFlag = 0;
			param.behaviorFlag |= SCE_MOUSE_OPEN_PARAM_MERGED;
			m_mouseHandle = sceMouseOpen(userId, SCE_MOUSE_PORT_TYPE_STANDARD, 0, &param);
			if (m_mouseHandle < SCE_OK) {
				return m_mouseHandle;
			}

			m_mouseX = 0;
			m_mouseY = 0;
			m_wheel = 0;
			m_tilt = 0;
			m_mouseHistory = 0;
			m_frameNoMoved = 0;
			m_setFlag = false;
			m_leftDown = false;
			m_rightDown = false;
			m_drag = false;

			return ret;
		}

		int update(uint32_t	screenWidth, uint32_t	screenHeight)
		{
			m_mouseHistory = sceMouseRead(m_mouseHandle, m_mouseData, sizeof(m_mouseData) / sizeof(SceMouseData));
			if (m_mouseHistory <= SCE_OK)
			{
				return m_mouseHistory;
			}

			// clear
			m_wheel = 0;
			m_tilt = 0;

			float deltaX = 0;
			float deltaY = 0;
			bool clickedL = false;
			bool clickedR = false;
			for (int i = 0; i < m_mouseHistory; i++)
			{
				deltaX += m_mouseData[i].xAxis;
				deltaY += m_mouseData[i].yAxis;
				m_wheel += m_mouseData[i].wheel;
				m_tilt += m_mouseData[i].tilt;
				if (m_mouseData[i].buttons & SCE_MOUSE_BUTTON_PRIMARY)
				{
					clickedL = true;
				}
				if (m_mouseData[i].buttons & SCE_MOUSE_BUTTON_SECONDARY)
				{
					clickedR = true;
				}
			}

			ImGuiIO& io = ImGui::GetIO();

			// Keep mouse button state
			if (m_mouseHistory > 0)
			{
				io.MouseDown[0] = clickedL;
				io.MouseDown[1] = clickedR;
				m_leftDown = clickedL;
				m_rightDown = clickedR;

				// Wheel
				io.MouseWheel = m_wheel;

				// End drag
				if (m_drag && !clickedL)
				{
					m_drag = false;
				}
				// Start drag
				if (clickedL)
				{
					m_drag = true;
				}
			}

			// Calc mouse position and adjust to display area
			if (abs(deltaX) > 0 || abs(deltaY) > 0)
			{
				constexpr float kMarginCursor = 10.0f;
				// Apply to ImGui mouse data
				io.MousePos.x += deltaX;
				io.MousePos.y += deltaY;
				if (io.MousePos.x < 0)								io.MousePos.x = 0;
				if (io.MousePos.x > (float)screenWidth - kMarginCursor)	io.MousePos.x = (float)screenWidth - kMarginCursor;
				if (io.MousePos.y < 0)								io.MousePos.y = 0;
				if (io.MousePos.y > (float)screenHeight - kMarginCursor)io.MousePos.y = (float)screenHeight - kMarginCursor;

				m_mouseX = io.MousePos.x;
				m_mouseY = io.MousePos.y;

				if (!io.MouseDrawCursor)
				{
					io.MouseDrawCursor = true;
					m_setFlag = true;
				}
				// Clear no input frame
				m_frameNoMoved = 0;
			}
			else
			{
				// After few frames passed with staying same position, hide mouse cursor.
				m_frameNoMoved++;
				if (m_setFlag && m_frameNoMoved > 60)
				{
					io.MouseDrawCursor = false;
					m_setFlag = false;
				}
			}

			return SCE_OK;
		}

		int finalize()
		{
			int ret = sceMouseClose(m_mouseHandle);
			if (ret != SCE_OK) {
				return ret;
			}

			ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_MOUSE);
			if (ret != SCE_OK) {
				return ret;
			}

			return SCE_OK;
		}
	};

	struct CombinedVertexBuffer
	{
		ImDrawVert	*m_vb;
		uint32_t	m_size;

		CombinedVertexBuffer()
		{
			m_vb = nullptr;
			m_size = 0;
		}
	};

	struct CombinedIndexBuffer
	{
		ImDrawIdx	*m_ib;
		uint32_t	m_size;

		CombinedIndexBuffer()
		{
			m_ib = nullptr;
			m_size = 0;
		}
	};

	SampleUtil::Graphics::VideoAllocator	*s_allocator = nullptr;


	int setRenderState(Gnmx::GfxContext *gfxc, uint32_t slot)
	{
		// shaders
		gfxc->setVsShader(SHADER(sampleutil_imgui_vv), 0, nullptr, OFFSETS_TABLE(sampleutil_imgui_vv));
		gfxc->setPsShader(SHADER(sampleutil_imgui_p), OFFSETS_TABLE(sampleutil_imgui_p));

		// prim type
		gfxc->setPrimitiveType(Gnm::kPrimitiveTypeTriList);

		// set index data
		gfxc->setIndexSize(Gnm::kIndexSize16);

		// mask
		gfxc->setRenderTargetMask(0xf << slot);

		// culling
		Gnm::PrimitiveSetup ps;
		ps.init();
		ps.setCullFace(Gnm::kPrimitiveSetupCullFaceNone);
		ps.setFrontFace(Gnm::kPrimitiveSetupFrontFaceCw);
		ps.setPolygonMode(Gnm::kPrimitiveSetupPolygonModeFill, Gnm::kPrimitiveSetupPolygonModeFill);
		gfxc->setPrimitiveSetup(ps);

		// depth stencil control
		Gnm::DepthStencilControl dsc;
		dsc.init();
		if (ImGui::GetCurrentContext()->EnableDepth)
		{
			dsc.setDepthEnable(true);
			dsc.setDepthControl(Gnm::kDepthControlZWriteEnable, Gnm::kCompareFuncLessEqual);
		}
		else
		{
			dsc.setDepthControl(Gnm::kDepthControlZWriteDisable, Gnm::kCompareFuncAlways);
		}
		gfxc->setDepthStencilControl(dsc);

		// blend control
		Gnm::BlendControl bc;
		bc.init();
		bc.setBlendEnable(true);
		bc.setColorEquation(Gnm::kBlendMultiplierSrcAlpha, Gnm::kBlendFuncAdd, Gnm::kBlendMultiplierOneMinusSrcAlpha);
		gfxc->setBlendControl(slot, bc);

		return SCE_OK;
	}
}

std::mutex	ImGui_ImplSampleUtil_writeString_lock;
char		*ImGui_ImplSampleUtil_writeString;
bool		ImGui_ImplSampleUtil_keepOskFocus = false;
bool		ImGui_ImplSampleUtil_useCustomPad = false;
bool		ImGui_ImplSampleUtil_useCustomKeyboard = false;
uint32_t	ImGui_ImplSampleUtil_defaultFontSize = 48;
ImFont		*ImGui_ImplSampleUtil_defaultFont = nullptr;


static std::array<CombinedVertexBuffer, BUFFERING>	g_VBs;
static std::array<CombinedIndexBuffer, BUFFERING>	g_IBs;

static Gnm::Texture	*g_pFontTextureView = nullptr;

static SampleUtil::Input::KeyboardContext	*g_pKeyboardContextInFocus;
static SampleUtil::Input::PadContext		*g_pPadContextInFocus;
static SampleUtil::Input::OskContext		*g_pOskContextInFocus;

static uint64_t             g_Time = 0;

static SampleUtil::Input::TouchPadData		s_prevData = { 0.0f, 0.0f, 255 };
static bool									s_addedMouseFlag = false;

static MouseContext							*s_mouseContext = nullptr;
static float								s_noInputOfTouchpad = 0.0f;
static float								s_updateInterval = 0.0f;

// Update ImGui Mouse data with Touchpad inputs
static
void updateMouseWithTouchpad(uint32_t displayWidth, uint32_t displayHeight, bool leftDown)
{
	if (g_pPadContextInFocus == nullptr)	return;

	ImGuiIO& io = ImGui::GetIO();

	SampleUtil::Input::PadData padData;
	int numData = g_pPadContextInFocus->getData(&padData, 1);
	if (numData > 0)
	{
		if (padData.touchNumber > 0)
		{
			if (!io.MouseDrawCursor)
			{
				io.MouseDrawCursor = true;
				s_addedMouseFlag = true;
			}

			// Move
			if (s_prevData.id != 255 && s_prevData.id == padData.touchPadData[0].id)
			{
				constexpr float kMarginCursor = 10.0f;
				// Add delta
				io.MousePos.x += ((padData.touchPadData[0].x - s_prevData.x) * (float)displayWidth / 4.0f);
				if (io.MousePos.x > displayWidth - kMarginCursor)	io.MousePos.x = displayWidth - kMarginCursor;
				if (io.MousePos.x < 0)								io.MousePos.x = 0;
				io.MousePos.y += ((padData.touchPadData[0].y - s_prevData.y) * (float)displayHeight / 4.0f);
				if (io.MousePos.y > displayHeight - kMarginCursor)	io.MousePos.y = displayHeight - kMarginCursor;
				if (io.MousePos.y < 0)								io.MousePos.y = 0;
			}
			// Left button
			io.MouseDown[0] = (g_pPadContextInFocus->isButtonDown(sce::SampleUtil::Input::kButtonTouchPad) | leftDown);

			// Keep current data to use in next frame
			s_prevData = padData.touchPadData[0];

			// Clear no input frame
			s_noInputOfTouchpad = 0.0f;
		}
		else
		{
			s_noInputOfTouchpad += io.DeltaTime;
			if (s_addedMouseFlag && s_noInputOfTouchpad >= 1.5f)
			{
				io.MouseDrawCursor = false;
				s_addedMouseFlag = false;
			}
			s_prevData.id = 255;
		}
	}

	// When cursor is drawing, treat X/L1/R1 button as Mouse left button.
	if (io.MouseDrawCursor)
	{
		if (g_pPadContextInFocus->isButtonDown(SampleUtil::Input::kButtonCross | SampleUtil::Input::kButtonL1 | SampleUtil::Input::kButtonR1, SampleUtil::Input::ButtonEventPattern::kButtonEventPatternAny))
		{
			io.MouseDown[0] = true;
			// Clear no input frame
			s_noInputOfTouchpad = 0.0f;
		}
		else
		{
			io.MouseDown[0] = false;
		}
	}
}

static
void updateMouseWithRightstick(uint32_t displayWidth, uint32_t displayHeight)
{
	ImGuiIO& io = ImGui::GetIO();

	if (g_pPadContextInFocus == nullptr)
	{
		// Hide mouse cursor when pad control is none
		io.MouseDrawCursor = false;
		s_addedMouseFlag = false;
		return;
	}

	// Move by Right stick
	constexpr float kMarginCursor = 10.0f;
	bool moved = false;
	vecmath::Vector2 rightStickValue = g_pPadContextInFocus->getRightStick();
	if (abs(rightStickValue.getX()) > 0.01f)
	{
		io.MousePos.x += (rightStickValue.getX() * (float)displayWidth / 80.0f);
		if (io.MousePos.x > displayWidth - kMarginCursor) io.MousePos.x = displayWidth - kMarginCursor;
		if (io.MousePos.x < 0) io.MousePos.x = 0;
		moved = true;
	}
	if (abs(rightStickValue.getY()) > 0.01f)
	{
		io.MousePos.y += (rightStickValue.getY() * (float)displayHeight / 80.0f);
		if (io.MousePos.y > displayHeight - kMarginCursor) io.MousePos.y = displayHeight - kMarginCursor;
		if (io.MousePos.y < 0) io.MousePos.y = 0;
		moved = true;
	}

	// When cursor is drawing, treat X/L1/R1 button as Mouse left button.
	if (io.MouseDrawCursor)
	{
		if (g_pPadContextInFocus->isButtonDown(SampleUtil::Input::kButtonCross | SampleUtil::Input::kButtonL1 | SampleUtil::Input::kButtonR1, SampleUtil::Input::ButtonEventPattern::kButtonEventPatternAny))
		{
			io.MouseDown[0] = true;
			// Clear no input frame
			s_noInputOfTouchpad = 0.0f;
		}
		else
		{
			io.MouseDown[0] = false;
		}
	}

	if (moved)
	{
		if (!io.MouseDrawCursor)
		{
			io.MouseDrawCursor = true;
			s_addedMouseFlag = true;
		}
		s_noInputOfTouchpad = 0.0f;
	}
	else
	{
		s_noInputOfTouchpad += io.DeltaTime;
		if (s_addedMouseFlag && s_noInputOfTouchpad >= 1.5f)
		{
			io.MouseDrawCursor = false;
			s_addedMouseFlag = false;
		}
	}
}


SampleUtil::Input::KeyboardContext *ImGui_ImplSampleUtil_GetKeyboardContextInFocus()
{
	return g_pKeyboardContextInFocus;
}

SampleUtil::Input::PadContext *ImGui_ImplSampleUtil_GetPadContextInFocus()
{
	return g_pPadContextInFocus;
}

SampleUtil::Input::OskContext *ImGui_ImplSampleUtil_GetOskContextInFocus()
{
	return g_pOskContextInFocus;
}

bool     ImGui_ImplSampleUtil_Init(SampleUtil::Graphics::VideoAllocator &videoMemory, sce::SampleUtil::System::UserIdManager *pIdManager, bool disableMouse)
{
	// Setup back-end capabilities flags
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= (ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableSetMousePos);
	io.BackendFlags |= (ImGuiBackendFlags_HasGamepad | ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos);
	io.BackendPlatformName = "imgui_impl_sampleutil";

	// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
	io.KeyMap[ImGuiKey_Tab]			= SampleUtil::Input::kTAB;
	io.KeyMap[ImGuiKey_LeftArrow]	= SampleUtil::Input::kLEFT;
	io.KeyMap[ImGuiKey_RightArrow]	= SampleUtil::Input::kRIGHT;
	io.KeyMap[ImGuiKey_UpArrow]		= SampleUtil::Input::kUP;
	io.KeyMap[ImGuiKey_DownArrow]	= SampleUtil::Input::kDOWN;
	io.KeyMap[ImGuiKey_PageUp]		= SampleUtil::Input::kPRIOR;
	io.KeyMap[ImGuiKey_PageDown]	= SampleUtil::Input::kNEXT;
	io.KeyMap[ImGuiKey_Home]		= SampleUtil::Input::kHOME;
	io.KeyMap[ImGuiKey_End]			= SampleUtil::Input::kEND;
	io.KeyMap[ImGuiKey_Insert]		= SampleUtil::Input::kINSERT;
	io.KeyMap[ImGuiKey_Delete]		= SampleUtil::Input::kDELETE;
	io.KeyMap[ImGuiKey_Backspace]	= SampleUtil::Input::kBACK;
	io.KeyMap[ImGuiKey_Space]		= SampleUtil::Input::kSPACE;
	io.KeyMap[ImGuiKey_Enter]		= SampleUtil::Input::kRETURN;
	io.KeyMap[ImGuiKey_Escape]		= SampleUtil::Input::kESCAPE;
	io.KeyMap[ImGuiKey_A]			= SampleUtil::Input::kA;
	io.KeyMap[ImGuiKey_C]			= SampleUtil::Input::kC;
	io.KeyMap[ImGuiKey_V]			= SampleUtil::Input::kV;
	io.KeyMap[ImGuiKey_X]			= SampleUtil::Input::kX;
	io.KeyMap[ImGuiKey_Y]			= SampleUtil::Input::kY;
	io.KeyMap[ImGuiKey_Z]			= SampleUtil::Input::kZ;

	io.BackendRendererName = "imgui_impl_sampleutil";

	// Get factory from device
	s_allocator = &videoMemory;

	ImGui_ImplSampleUtil_writeString = new char[1024];
	ImGui_ImplSampleUtil_writeString[0] = '\0';

	ImGui_ImplSampleUtil_defaultFont = ImGuiLibFont::AddSystemFont(io.Fonts, (float)ImGui_ImplSampleUtil_defaultFontSize);

	// Load and keep font modules to rebuild font atlas dynamically
	int ret = ImGuiLibFont::Initialize();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return false;
	}

	// Get user ID
	SceUserServiceUserId userId = SCE_USER_SERVICE_USER_ID_INVALID;
	if (pIdManager != nullptr)
	{
		ret = pIdManager->getInitialUser(&userId);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return false;
		}
	}

	// Initailize mouse
	if (!disableMouse && (userId != SCE_USER_SERVICE_USER_ID_INVALID))
	{
		if (s_mouseContext == nullptr)
		{
			s_mouseContext = new MouseContext();
			SCE_SAMPLE_UTIL_ASSERT(s_mouseContext != nullptr);
			if (s_mouseContext == nullptr)
			{
				return false;
			}
		}
		s_mouseContext->initialize(userId);

		// Validate position
		if (!ImGui::IsMousePosValid(&io.MousePos))
		{
			io.MousePos = ImVec2(0, 0);
			if (s_mouseContext)
			{
				s_mouseContext->m_mouseX = io.MousePos.x;
				s_mouseContext->m_mouseY = io.MousePos.y;
			}
		}
	}

	return true;
}

void     ImGui_ImplSampleUtil_Shutdown()
{
	SCE_SAMPLE_UTIL_SAFE_DELETE(s_mouseContext);

	ImGuiIO& io = ImGui::GetIO();
	ImGuiLibFont::ClearSystemFont(io.Fonts);

	// Unload font molues
	int ret = ImGuiLibFont::Finalize();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	ImGui_ImplSampleUtil_InvalidateDeviceObjects();
	s_allocator = nullptr;

	delete[] ImGui_ImplSampleUtil_writeString;

	DESTROY_SHADER(sampleutil_imgui_vv);
	DESTROY_SHADER(sampleutil_imgui_p);

}

void     ImGui_ImplSampleUtil_NewFrame(uint32_t	displayWidth, uint32_t	displayHeight, SampleUtil::Input::KeyboardContext *keyboardContext, SampleUtil::Input::PadContext *padContext, SampleUtil::Input::OskContext *oskContext)
{
	int ret = 0;

	g_pKeyboardContextInFocus = keyboardContext;
	g_pPadContextInFocus = padContext;
	g_pOskContextInFocus = oskContext;

	if (g_pPadContextInFocus == nullptr && sce::SampleUtil::Debug::ApiCapture::RenderHudContext::get().m_showHud)
	{
		g_pPadContextInFocus = sce::SampleUtil::Debug::ApiCapture::RenderHudContext::get().m_pPad; // override pad if api capture is now being rendered
	}

	if((g_shader_sampleutil_imgui_vv == nullptr) || (SHADER(sampleutil_imgui_vv) == nullptr))
	{
		ImGui_ImplSampleUtil_CreateDeviceObjects();
	}

	ImGuiIO& io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	io.DisplaySize = ImVec2(displayWidth, displayHeight);

	// Setup time step
	uint64_t current_time;
	current_time = sceKernelGetProcessTime();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	io.DeltaTime = (float)(current_time - g_Time) / 1000000.f;
	g_Time = current_time;

	// Update inputs (mouse, pad) in about 60fps
	s_updateInterval += io.DeltaTime;
	if (s_updateInterval > 0.015f)
	{
		// Update Mouse
		if (s_mouseContext)	s_mouseContext->update(displayWidth, displayHeight);

		// Update alternative to mouse (right-stick, touchpad) when imgui is used for other than DebugString
		if (!ImGui::GetCurrentContext()->UsedOnlyDebugString)
		{
			// Alternative mouse control affects real mouse control. So do nothing when real mouse dragging.
			if (s_mouseContext && !s_mouseContext->m_drag)
			{
				// Use Touchpad as Mouse
				updateMouseWithTouchpad(displayWidth, displayHeight, s_mouseContext ? s_mouseContext->m_leftDown : false);
				// Use Rightstick as Mouse
				updateMouseWithRightstick(displayWidth, displayHeight);
			}
		}
		s_updateInterval = 0.0f;
	}

	// gamepad
	if (!ImGui_ImplSampleUtil_useCustomPad)
	{
		io.NavInputs[ImGuiNavInput_Activate] = (g_pPadContextInFocus && g_pPadContextInFocus->isButtonDown(SampleUtil::Input::kButtonCross)) ? 1.f : 0.f; // activate / open / toggle / tweak value       // e.g. Cross  (PS4), A (Xbox), A (Switch), Space (Keyboard)
		io.NavInputs[ImGuiNavInput_Cancel] = (g_pPadContextInFocus && g_pPadContextInFocus->isButtonDown(SampleUtil::Input::kButtonCircle)) ? 1.f : 0.f; // cancel / close / exit                        // e.g. Circle (PS4), B (Xbox), B (Switch), Escape (Keyboard)
		io.NavInputs[ImGuiNavInput_Input] = (g_pPadContextInFocus && g_pPadContextInFocus->isButtonDown(SampleUtil::Input::kButtonTriangle)) ? 1.f : 0.f; // text input / on-screen keyboard              // e.g. Triang.(PS4), Y (Xbox), X (Switch), Return (Keyboard)
		io.NavInputs[ImGuiNavInput_Menu] = (g_pPadContextInFocus && g_pPadContextInFocus->isButtonDown(SampleUtil::Input::kButtonSquare)) ? 1.f : 0.f; // tap: toggle menu / hold: focus, move, resize // e.g. Square (PS4), X (Xbox), Y (Switch), Alt (Keyboard)
		io.NavInputs[ImGuiNavInput_DpadLeft] = (g_pPadContextInFocus && g_pPadContextInFocus->isButtonDown(SampleUtil::Input::kButtonLeft)) ? 1.f : 0.f; // move / tweak / resize window (w/ PadMenu)    // e.g. D-pad Left/Right/Up/Down (Gamepads), Arrow keys (Keyboard)
		io.NavInputs[ImGuiNavInput_DpadRight] = (g_pPadContextInFocus && g_pPadContextInFocus->isButtonDown(SampleUtil::Input::kButtonRight)) ? 1.f : 0.f; // move / tweak / resize window (w/ PadMenu)    // e.g. D-pad Left/Right/Up/Down (Gamepads), Arrow keys (Keyboard)
		io.NavInputs[ImGuiNavInput_DpadUp] = (g_pPadContextInFocus && g_pPadContextInFocus->isButtonDown(SampleUtil::Input::kButtonUp)) ? 1.f : 0.f; // move / tweak / resize window (w/ PadMenu)    // e.g. D-pad Left/Right/Up/Down (Gamepads), Arrow keys (Keyboard)
		io.NavInputs[ImGuiNavInput_DpadDown] = (g_pPadContextInFocus && g_pPadContextInFocus->isButtonDown(SampleUtil::Input::kButtonDown)) ? 1.f : 0.f; // move / tweak / resize window (w/ PadMenu)    // e.g. D-pad Left/Right/Up/Down (Gamepads), Arrow keys (Keyboard)
		if (g_pPadContextInFocus != nullptr)
		{
			auto leftStickValue = g_pPadContextInFocus->getLeftStick();
			io.NavInputs[ImGuiNavInput_LStickLeft] = (leftStickValue.getX() < 0.f) ? -leftStickValue.getX() : 0.f;    // scroll / move window (w/ PadMenu)            // e.g. Left Analog Stick Left/Right/Up/Down      
			io.NavInputs[ImGuiNavInput_LStickRight] = (leftStickValue.getX() > 0.f) ? leftStickValue.getX() : 0.f;    // scroll / move window (w/ PadMenu)            // e.g. Left Analog Stick Left/Right/Up/Down      
			io.NavInputs[ImGuiNavInput_LStickDown] = (leftStickValue.getY() > 0.f) ? leftStickValue.getY() : 0.f;    // scroll / move window (w/ PadMenu)            // e.g. Left Analog Stick Left/Right/Up/Down      
			io.NavInputs[ImGuiNavInput_LStickUp] = (leftStickValue.getY() < 0.f) ? -leftStickValue.getY() : 0.f;    // scroll / move window (w/ PadMenu)            // e.g. Left Analog Stick Left/Right/Up/Down      
			io.NavInputs[ImGuiNavInput_FocusPrev] = g_pPadContextInFocus->isButtonDown(SampleUtil::Input::kButtonL1) ? 1.f : 0.f;          // next window (w/ PadMenu)                     // e.g. L1 or L2 (PS4), LB or LT (Xbox), L or ZL (Switch)
			io.NavInputs[ImGuiNavInput_FocusNext] = g_pPadContextInFocus->isButtonDown(SampleUtil::Input::kButtonR1) ? 1.f : 0.f;          // prev window (w/ PadMenu)                     // e.g. R1 or R2 (PS4), RB or RT (Xbox), R or ZL (Switch)
																																		   // Use analog value of L2/R2 for scrolling speed
			sce::SampleUtil::Input::PadData padData = {};
			g_pPadContextInFocus->getData(&padData, 1);
			io.NavInputs[ImGuiNavInput_TweakSlow] = (float)padData.l2 / 255.0f;          // slower tweaks                                // e.g. L1 or L2 (PS4), LB or LT (Xbox), L or ZL (Switch)
			io.NavInputs[ImGuiNavInput_TweakFast] = (float)padData.r2 / 255.0f;          // faster tweaks                                // e.g. R1 or R2 (PS4), RB or RT (Xbox), R or ZL (Switch)
		}
	}

	if (!ImGui_ImplSampleUtil_useCustomKeyboard)
	{
		// Read keyboard modifiers inputs
		io.KeyCtrl = (keyboardContext != nullptr && keyboardContext->isConnected()) ? keyboardContext->isKeyDown(SampleUtil::Input::kLCONTROL) || keyboardContext->isKeyDown(SampleUtil::Input::kRCONTROL) : false;
		io.KeyShift = (keyboardContext != nullptr && keyboardContext->isConnected()) ? keyboardContext->isKeyDown(SampleUtil::Input::kLSHIFT) || keyboardContext->isKeyDown(SampleUtil::Input::kRSHIFT) : false;
		io.KeyAlt = (keyboardContext != nullptr && keyboardContext->isConnected()) ? keyboardContext->isKeyDown(SampleUtil::Input::kLMENU) || keyboardContext->isKeyDown(SampleUtil::Input::kRMENU) : false;
	}
	io.KeySuper = false;

	if (!ImGui_ImplSampleUtil_useCustomKeyboard)
	{
		for (int i = 0; i < 0x100; i++)
		{
			io.KeysDown[i] = (keyboardContext != nullptr && keyboardContext->isConnected()) ? keyboardContext->isKeyDown((SampleUtil::Input::Key)i) : false;
		}
	}

	static bool isOskInUse = false;
	static wchar_t oskResult[256 + 2] = { '\0' };
	static int oskIdleCount = 0;
	if (oskIdleCount > 0) --oskIdleCount;
	if (!ImGui_ImplSampleUtil_keepOskFocus && oskIdleCount == 3)
	{
		io.KeysDown[SampleUtil::Input::kESCAPE] = true;// emulate pressing CTRL + ENTER key to unfocus text input
	}
	if ((keyboardContext == nullptr || !keyboardContext->isConnected()) && !ImGui_ImplSampleUtil_useCustomKeyboard && (oskContext != nullptr) && io.WantTextInput && !isOskInUse && oskIdleCount == 0)
	{
		// Setup param
		sce::SampleUtil::Input::OskParam param;
		{
			ImGuiContext *g = ImGui::GetCurrentContext();

			// Default 
			param.m_type = sce::SampleUtil::Input::kDefault;
			param.m_title = (g->InputTextState.DialogUserFlags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsScientific | ImGuiInputTextFlags_CharsHexadecimal)) ? L"Input numeric value" : L"Input dialog";
			param.m_placeholder = L"Placeholder";

			// Custom
			if (g->InputTextState.DialogUserFlags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsScientific))
			{
				param.m_type = sce::SampleUtil::Input::kDecimal;
			}
			else if (g->InputTextState.DialogUserFlags & ImGuiInputTextFlags_CharsHexadecimal)
			{
				param.m_type = sce::SampleUtil::Input::kAlphanumeric;
			}
			else if (g->InputTextState.DialogUserFlags & ImGuiInputTextFlags_JapaneseForced)
			{
				param.m_type = sce::SampleUtil::Input::kJapanese;
				param.m_title = (g->InputTextState.DialogUserFlags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsScientific | ImGuiInputTextFlags_CharsHexadecimal)) ? L"数値を入力してください" : L"入力ダイアログ";
				param.m_placeholder = L"プレースホルダー";
			}
			ImGuiID id = g->ActiveId;	// target text box ID
			if (nullptr != ImGui::GetInputTextTitle(id))		param.m_title = (wchar_t*)ImGui::GetInputTextTitle(id);
			if (nullptr != ImGui::GetInputTextPlaceholder(id))	param.m_placeholder = (wchar_t*)ImGui::GetInputTextPlaceholder(id);
		}
		oskContext->start(&param);
		isOskInUse = true;
	}
	else if (isOskInUse)
	{
		if (oskContext == nullptr)
		{
			isOskInUse = false;
		}
		else if (oskResult[0] == '\0' && oskContext->isFinished())
		{
			int ret = oskContext->getResult(oskResult);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (wcslen(oskResult) == 0)
			{
				wcsncat(oskResult, L"\b", 1); // added backspace
			}
			wcsncat(oskResult, L"\r", 1); // added ENTER
		}
	}

	if (io.WantTextInput)
	{
		if (oskResult[0] != '\0')
		{
			// osk string
			int i = 0;
			for (; i < 16 && oskResult[i]; i++)
			{
				if (oskResult[i] == L'\b')
				{
					io.KeysDown[SampleUtil::Input::kDELETE] = true;// emulate pressing delete key
				}
				io.AddInputCharacter(oskResult[i]);
			}
			memmove(oskResult, &oskResult[i], (wcslen(&oskResult[i]) + 1) * sizeof(wchar_t));
			if (oskResult[0] == '\0')
			{
				io.KeysDown[SampleUtil::Input::kRETURN] = true;// emulate pressing ENTER key
				isOskInUse = false;
				oskIdleCount = 5;
			}
		}
		{
			// write string
			int i = 0;
			for (; i < 16 && ImGui_ImplSampleUtil_writeString[i]; i++)
			{
				uint32_t len;
				uint16_t wchar;
				sceCesUtf8ToUcs2((uint8_t*)&ImGui_ImplSampleUtil_writeString[i], 1, &len, &wchar);
				io.AddInputCharacter(wchar);
			}
			{
				std::lock_guard<std::mutex> guard(ImGui_ImplSampleUtil_writeString_lock);
				memmove(ImGui_ImplSampleUtil_writeString, &ImGui_ImplSampleUtil_writeString[i], strlen(&ImGui_ImplSampleUtil_writeString[i]) + 1);
			}
		}

		if (keyboardContext != nullptr && keyboardContext->isConnected())
		{
			for (int i = 0; i < 1024 && keyboardContext->getInputChars()[i]; i++)
			{
				io.AddInputCharacter((keyboardContext->getInputChars()[i] == '\n') ? '\r' : keyboardContext->getInputChars()[i]); // replace LF with CR for imgui ENTER handling
				if (keyboardContext->getInputChars()[i] == '\n')
				{
					break; // cut key inputs at the point of ENTER key input
				}
			}
		}
	}
}

void     ImGui_ImplSampleUtil_RenderDrawData(Gnmx::GfxContext *gfxc, ImDrawData* draw_data)
{
	static uint32_t target = 0;

	// Create and grow vertex/index buffers if needed
	if (g_VBs[target].m_size < draw_data->TotalVtxCount * sizeof(ImDrawVert))
	{
		if(g_VBs[target].m_vb != nullptr)
		{
			s_allocator->free(g_VBs[target].m_vb);
		}
		CombinedVertexBuffer pVB;
		pVB.m_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
		pVB.m_vb = (ImDrawVert*)s_allocator->allocate(pVB.m_size, 4, "ImGui_VB");
		SCE_SAMPLE_UTIL_ASSERT(pVB.m_vb != nullptr);
		g_VBs[target] = pVB;
	}
	if (g_IBs[target].m_size < draw_data->TotalIdxCount * sizeof(ImDrawIdx))
	{
		if (g_IBs[target].m_ib != nullptr)
		{
			s_allocator->free(g_IBs[target].m_ib);
		}
		CombinedIndexBuffer pIB;
		pIB.m_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
		pIB.m_ib = (ImDrawIdx*)s_allocator->allocate(pIB.m_size, 4, "ImGui_IB");
		g_IBs[target] = pIB;
	}

	// Copy and convert all vertices into a single contiguous buffer
	ImDrawVert* vtx_dst = g_VBs[target].m_vb;
	ImDrawIdx* idx_dst = g_IBs[target].m_ib;

	_mm_sfence();
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtx_dst += cmd_list->VtxBuffer.Size;
		idx_dst += cmd_list->IdxBuffer.Size;
	}
	_mm_sfence();

	// Set up a viewport
	ImGuiIO io = ImGui::GetIO();
	Gnmx::setupScreenViewport(&gfxc->m_dcb, 0, 0, io.DisplaySize.x, io.DisplaySize.y, 1.0f, 0.0f);

	// SRT for ImGui
	SampleUtil::UIFramework::ImguiSrtData userData;

	// Setup orthographic projection matrix into our constant buffer
	// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). 
	{
		float L = draw_data->DisplayPos.x;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		float T = draw_data->DisplayPos.y;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
		float mvp[4][4] =
		{
			{ 2.0f / (R - L),		0.0f,				0.0f,       0.0f },
			{ 0.0f,					2.0f / (T - B),     0.0f,       0.0f },
			{ 0.0f,					0.0f,				0.5f,       0.0f },
			{ (R + L) / (L - R),	(T + B) / (B - T),	0.5f,       1.0f },
		};

		// setup SRT per-frame data
		userData.m_frameData = (SampleUtil::UIFramework::PerFrameData*)gfxc->allocateFromCommandBuffer(sizeof(SampleUtil::UIFramework::PerFrameData), Gnm::kEmbeddedDataAlignment16);
		{
			// prepare vertex data as RegularBuffer<ImDrawVert>
			Gnm::Buffer buf;
			buf.initAsRegularBuffer(g_VBs[target].m_vb, sizeof(ImDrawVert), draw_data->TotalVtxCount);

			memcpy(&userData.m_frameData->m_mvp, mvp, sizeof(mvp));
			userData.m_frameData->m_vertex = buf;

			userData.m_frameData->m_hdr = (uint)SampleUtil::Graphics::getHdr();
		}
	}

	// set render state
	setRenderState(gfxc, 0);
	// One instance
	gfxc->setNumInstances(1);

	// Render command lists
	int vtx_offset = 0;
	int idx_offset = 0;
	ImVec2 pos = draw_data->DisplayPos;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				// User callback (registered via ImDrawList::AddCallback)
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				// Apply scissor/clipping rectangle
				gfxc->setScreenScissor((uint32_t)(pcmd->ClipRect.x - pos.x), (uint32_t)(pcmd->ClipRect.y - pos.y), (uint32_t)(pcmd->ClipRect.z - pos.x), (uint32_t)(pcmd->ClipRect.w - pos.y));

				// setup SRT per-draw data
				userData.m_drawData = (SampleUtil::UIFramework::PerDrawData*)gfxc->allocateFromCommandBuffer(sizeof(SampleUtil::UIFramework::PerDrawData), Gnm::kEmbeddedDataAlignment16);
				userData.m_drawData->m_texture = *((Gnm::Texture*)pcmd->TextureId);
				userData.m_drawData->m_vtxOffset = vtx_offset;

				// bind SRT
				gfxc->setUserSrtBuffer(Gnm::ShaderStage::kShaderStageVs, &userData, sizeof(userData) / sizeof(uint32_t));
				gfxc->setUserSrtBuffer(Gnm::ShaderStage::kShaderStagePs, &userData, sizeof(userData) / sizeof(uint32_t));

				// draw
				gfxc->drawIndex(pcmd->ElemCount, g_IBs[target].m_ib + idx_offset);
			}
			idx_offset += pcmd->ElemCount;
		}
		vtx_offset += cmd_list->VtxBuffer.Size;
	}

	target = (target + 1) % BUFFERING;
}

// Use if you want to reset your rendering device without losing ImGui state.
void     ImGui_ImplSampleUtil_InvalidateDeviceObjects()
{
	if (g_pFontTextureView != nullptr)
	{
		s_allocator->free(g_pFontTextureView->getBaseAddress());
		s_allocator->free(g_pFontTextureView);
		g_pFontTextureView = nullptr;
		ImGui::GetIO().Fonts->TexID = nullptr; // We copied g_pFontTextureView to io.Fonts->TexID so let's clear that as well.
	}
	for (auto &ib : g_IBs)
	{
		if (ib.m_ib != nullptr)
		{
			s_allocator->free(ib.m_ib);
			ib.m_ib = nullptr;
			ib.m_size = 0;
		}
	}
	for (auto &vb : g_VBs)
	{
		if (vb.m_vb != nullptr)
		{
			s_allocator->free(vb.m_vb);
			vb.m_vb = nullptr;
			vb.m_size = 0;
		}
	}
}

static int ImGui_ImplSampleUtil_CreateFontsTexture()
{
	int ret = 0; (void)ret;

	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

	// Create texture
	if (g_pFontTextureView == nullptr) {
		g_pFontTextureView = (Gnm::Texture*)s_allocator->allocate(sizeof(Gnm::Texture), 4, "FontText");
		SCE_SAMPLE_UTIL_ASSERT(g_pFontTextureView != nullptr);
		if (g_pFontTextureView == nullptr) {
			return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}

		Gnm::DataFormat dataformat = { {{ Gnm::kSurfaceFormat8, Gnm::kTextureChannelTypeUNorm, Gnm::kTextureChannelX, Gnm::kTextureChannelX, Gnm::kTextureChannelX, Gnm::kTextureChannelX, 0 }} };
		Gnm::TextureSpec spec;
		spec.init();
		spec.m_textureType = Gnm::kTextureType2d;
		spec.m_width = width;
		spec.m_height = height;
		spec.m_depth = 1;
		spec.m_numMipLevels = 1;
		spec.m_numSlices = 1;
		spec.m_format = dataformat;
		spec.m_tileModeHint = Gnm::kTileModeThin_2dThin;
		spec.m_numFragments = Gnm::kNumFragments1;
		int32_t status = g_pFontTextureView->init(&spec);
		if (status != 0) {
			return -1;
		}
		Gnm::SizeAlign sizeAlign = g_pFontTextureView->getSizeAlign();
		uint32_t alignedImageSize = sizeAlign.m_size;
		void *buffer = s_allocator->allocate(alignedImageSize, sizeAlign.m_align, "sce::SampleUtil::UIFramework::ImGui::fontAtlas");
		SCE_SAMPLE_UTIL_ASSERT(buffer != nullptr);
		if (buffer == nullptr) {
			return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}
		s_allocator->registerResource((const void *)buffer, sizeAlign.m_size, "sce::SampleUtil::UIFramework::ImGui::fontAtlas", { sce::Gnm::kResourceTypeTextureBaseAddress });

		// tileSurface
		sce::GpuAddress::TilingParameters tp;
		ret = tp.initFromTexture(g_pFontTextureView, 0, 0);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		ret = sce::GpuAddress::tileSurface(buffer, pixels, &tp);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		g_pFontTextureView->setBaseAddress(buffer);
	}

	// Store our identifier
	io.Fonts->TexID = (ImTextureID)g_pFontTextureView;

	return SCE_OK;
}


bool ImGui_ImplSampleUtil_CreateDeviceObjects()
{
	int ret = 0;

	if ((g_shader_sampleutil_imgui_vv != nullptr) && (SHADER(sampleutil_imgui_vv) != nullptr)) {
		ImGui_ImplSampleUtil_InvalidateDeviceObjects();
	}

	// Create shaders
	CREATE_SHADER(Gnmx::VsShader, sampleutil_imgui_vv, s_allocator);
	CREATE_SHADER(Gnmx::PsShader, sampleutil_imgui_p, s_allocator);

	ret = ImGui_ImplSampleUtil_CreateFontsTexture();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return false;
	}

	return true;
}
#endif
