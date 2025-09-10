/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <libsysmodule.h>
#include <agc.h>
#include <vision/vr_tracker2.h>
#include <hmd2.h>
#include <vr_setup_dialog.h>
#pragma comment(lib, "SceVrSetupDialog_stub_weak")
#include "sampleutil/helper/prospero/async_asset_loader_prospero.h"
#endif
#include "sampleutil/skeleton.h"

sce::SampleUtil::SampleSkeleton::SampleSkeleton()
	: m_functionFlags(0)
	, m_graphicsContext(nullptr)
	, m_userIdManager(nullptr)
	, m_padContextOfInitialUser(nullptr)
	, m_keyboardContextOfInitialUser(nullptr)
	, m_oskContextOfInitialUser(nullptr)
	, m_debugConsole(nullptr)
	, m_jobQueue(nullptr)
	, m_audioOut(nullptr)
	, m_deltaTime(0.0f)
	, m_previousTime(0.0f)
	, m_updateIntervalOfInput(0.0f)
	, m_prevFlipCount((uint64_t)-1)
	, m_prevBaseFrameBus(-1)
	, m_useVrSetupDialog(false)
{

}

sce::SampleUtil::SampleSkeleton::~SampleSkeleton()
{

}

sce::SampleUtil::SampleSkeleton::SampleSkeletonOption::SampleSkeletonOption()
{
	graphicsOption = nullptr;
	padOption = nullptr;
	debugConsoleOption = nullptr;
	imGuiOption = nullptr;
	jobQueueOption = nullptr;
	audioOutContextOption = nullptr;
	memoryOption = nullptr;
	perfOption = nullptr;
#if _SCE_TARGET_OS_PROSPERO
	asyncAssetLoaderOption = nullptr;
	vrTrackerOption = nullptr;
	reprojectionOption = nullptr;
#endif
	useVrSetupDialog = false;
}

int sce::SampleUtil::SampleSkeleton::initializeUtilInternal(
	uint32_t functionFlags,
	int32_t width,
	int32_t height,
	SampleSkeletonOption* option,
	const char	*pScriptFilename)
{
	if (!(functionFlags & kFunctionFlagGraphics) && (functionFlags & (kFunctionFlagSpriteUtility | kFunctionFlagImGui | kFunctionFlagVr)))
	{
		return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}
	if (!(functionFlags & kFunctionFlagUserIdManager) && (functionFlags & (kFunctionFlagPadOfInitialUser | kFunctionFlagVr)))
	{
		return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}
	if (!(functionFlags & kFunctionFlagPadOfInitialUser) && (functionFlags & kFunctionFlagVr))
	{
		return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}
	if (!(functionFlags & kFunctionFlagImGui) && (functionFlags & kFunctionFlagDebugConsole))
	{
		return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}

	printf("## Sample Application: start initializing ##\n");

	int ret = SCE_OK;

	if (width <= 0 || height <= 0)
	{
#if _SCE_TARGET_OS_PROSPERO
		width = 3840;
		height = 2160;
#endif
#if _SCE_TARGET_OS_ORBIS
		width = 1920;
		height = 1080;
#endif
	}

	m_functionFlags = 0u;
	m_deltaTime = 0.0f;
	m_previousTime = 0.0f;
	m_updateIntervalOfInput = 0.0f;
	m_prevFlipCount = (uint64_t)-1;
	m_prevBaseFrameBus = -1;

	m_pScript = nullptr;
	m_graphicsContext = nullptr;
	m_userIdManager = nullptr;
	m_padContextOfInitialUser = nullptr;
	m_keyboardContextOfInitialUser = nullptr;
	m_oskContextOfInitialUser = nullptr;
	m_debugConsole = nullptr;
	m_jobQueue = nullptr;
	m_audioOut = nullptr;
#if _SCE_TARGET_OS_PROSPERO
	m_asyncAssetLoader = nullptr;
	m_vrControllerContextsOfInitialUser[0] = nullptr;
	m_vrControllerContextsOfInitialUser[1] = nullptr;
	m_hmdHandleOfInitialUser = -1;
	m_vrTracker = nullptr;
	m_reprojection = nullptr;
#endif

	Graphics::GraphicsContextOption	*graphicsOption = nullptr;
	Input::PadContextOption	*padOption = nullptr;
	Debug::ConsoleOption	*debugConsoleOption = nullptr;
	UIFramework::ImGuiOption	*imguiOption = nullptr;
	Thread::JobQueueOption	*jobQueueOption = nullptr;
	Audio::AudioOutContextOption	*audioOutContextOption = nullptr;
	Debug::PerfOption *perfOption = nullptr;
#if _SCE_TARGET_OS_PROSPERO
	Memory::MemoryOption *memoryOption = nullptr;
	Helper::AsyncAssetLoaderOption *asyncAssetLoaderOption = nullptr;
	Input::VrTrackerOption *vrTrackerOption = nullptr;
	Graphics::ReprojectionOption *reprojectionOption = nullptr;
#endif

	(void)padOption;

	if (option != nullptr)
	{
		graphicsOption = option->graphicsOption;
		padOption = option->padOption;
		debugConsoleOption = option->debugConsoleOption;
		imguiOption = option->imGuiOption;
		jobQueueOption = option->jobQueueOption;
		audioOutContextOption = option->audioOutContextOption;
		perfOption = option->perfOption;
#if _SCE_TARGET_OS_PROSPERO
		memoryOption = option->memoryOption;
		asyncAssetLoaderOption = option->asyncAssetLoaderOption;
		vrTrackerOption = option->vrTrackerOption;
		reprojectionOption = option->reprojectionOption;
#endif
		m_useVrSetupDialog = option->useVrSetupDialog;
	}

	Debug::PerfOption defaultPerfOption;
	if (perfOption == nullptr) perfOption = &defaultPerfOption;
#if _SCE_TARGET_OS_PROSPERO
	Helper::AsyncAssetLoaderOption defaultAsyncAssetLoaderOption;
	if (asyncAssetLoaderOption == nullptr) asyncAssetLoaderOption = &defaultAsyncAssetLoaderOption;
#endif
	if (perfOption->useMat)
	{
		Memory::initializeMemoryAnalyzer();
	}
	Debug::Perf::initialize(perfOption);

#if _SCE_TARGET_OS_PROSPERO
	Memory::initializeMapper(memoryOption);
#endif

	Graphics::GraphicsContextOption defaultGfxOption;
	if (graphicsOption == nullptr) graphicsOption = &defaultGfxOption;

	if (functionFlags & kFunctionFlagGraphics)
	{
#if _SCE_TARGET_OS_PROSPERO
		Agc::init();
#endif
		if (graphicsOption->enableResourceRegistration)
		{
			Graphics::initializeResourceRegistration(graphicsOption->resourceRegistrationMaxOwnersAndResources, graphicsOption->resourceRegistrationMaxNameLength);
		}
	}

#if _SCE_TARGET_OS_PROSPERO
#if	!SCE_SAMPLE_UTIL_DISABLE_AMPR
	m_asyncAssetLoader = new Helper::AsyncAssetLoader(*asyncAssetLoaderOption);
#endif
	if (functionFlags & kFunctionFlagVr)
	{
		m_reprojection = new Graphics::Reprojection(reprojectionOption);
		m_vrTracker = new Input::VrTracker(vrTrackerOption);
	}
#endif

	if (functionFlags & kFunctionFlagGraphics)
	{
		m_graphicsContext = new  Graphics::GraphicsContext(width, height,
#if _SCE_TARGET_OS_PROSPERO
															m_asyncAssetLoader,
#endif
															graphicsOption);
		SCE_SAMPLE_UTIL_ASSERT(m_graphicsContext != nullptr);
		if (m_graphicsContext == nullptr) {
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		const int	gfxCtxErrorCode = m_graphicsContext->getErrorCode();
		if (gfxCtxErrorCode != SCE_OK) {
			delete	m_graphicsContext;
			finalizeUtil();
			return	gfxCtxErrorCode;
		}
		m_graphicsContext->m_enableRazorCpu = !perfOption->disableRazorCpuSync;
		if (functionFlags & kFunctionFlagSpriteUtility)
		{
			ret = Graphics::SpriteUtil::initialize(m_graphicsContext->m_videoMemory);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return ret;
			}
			m_functionFlags |= kFunctionFlagSpriteUtility;
		}
		m_functionFlags |= kFunctionFlagGraphics;
	}

	if (functionFlags & kFunctionFlagUserIdManager) {
		m_userIdManager = new System::UserIdManager();
		SCE_SAMPLE_UTIL_ASSERT(m_userIdManager != nullptr);
		if (m_userIdManager == nullptr) {
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		m_functionFlags |= kFunctionFlagUserIdManager;
	}

	if (functionFlags & kFunctionFlagPadOfInitialUser) {
		System::UserId initialUserId = 0;
		ret = m_userIdManager->getInitialUser(&initialUserId);
		if (ret != SCE_OK) {
			return ret;
		}
#if _SCE_TARGET_OS_PROSPERO
		if (functionFlags & kFunctionFlagVr)
		{
			m_vrControllerContextsOfInitialUser[1] = new Input::PadContext(initialUserId, SCE_PAD_PORT_TYPE_VR_CONTROLLER_RIGHT, 0, padOption);
			ret = sceVrTracker2RegisterDevice(SCE_VR_TRACKER2_DEVICE_VR_CONTROLLER_RIGHT, m_vrControllerContextsOfInitialUser[1]->getPadHandle());
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

			m_vrControllerContextsOfInitialUser[0] = new Input::PadContext(initialUserId, SCE_PAD_PORT_TYPE_VR_CONTROLLER_LEFT, 0, padOption);
			ret = sceVrTracker2RegisterDevice(SCE_VR_TRACKER2_DEVICE_VR_CONTROLLER_LEFT, m_vrControllerContextsOfInitialUser[0]->getPadHandle());
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

			m_hmdHandleOfInitialUser = sceHmd2Open(initialUserId, 0, 0, nullptr);
			SCE_SAMPLE_UTIL_ASSERT(m_hmdHandleOfInitialUser >= SCE_OK);
			ret = sceVrTracker2RegisterDevice(SCE_VR_TRACKER2_DEVICE_HMD, m_hmdHandleOfInitialUser);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			m_functionFlags |= kFunctionFlagVr;
		}
#endif
		m_padContextOfInitialUser = new Input::PadContext(initialUserId, 0, 0, padOption);
		SCE_SAMPLE_UTIL_ASSERT(m_padContextOfInitialUser != nullptr);
		if (m_padContextOfInitialUser == nullptr)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		m_keyboardContextOfInitialUser = new Input::KeyboardContext(initialUserId);
		SCE_SAMPLE_UTIL_ASSERT(m_keyboardContextOfInitialUser != nullptr);
		if (m_keyboardContextOfInitialUser == nullptr) {
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		m_oskContextOfInitialUser = new Input::OskContext(initialUserId);
		SCE_SAMPLE_UTIL_ASSERT(m_oskContextOfInitialUser != nullptr);
		if (m_oskContextOfInitialUser == nullptr) {
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		m_functionFlags |= kFunctionFlagPadOfInitialUser;
	}

	if (functionFlags & kFunctionFlagImGui)
	{
		ret = UIFramework::imGuiInitialize(imguiOption, m_graphicsContext->m_cpuMemory, &m_graphicsContext->m_videoMemory, &m_graphicsContext->m_videoRingMemory, m_userIdManager);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		m_isImguiNewFrameCalled = false;
		m_functionFlags |= kFunctionFlagImGui;
	}

	if (functionFlags & kFunctionFlagDebugConsole) {
		const char *name = debugConsoleOption ? debugConsoleOption->name : "";
		float  position[2] = { -1.f, -1.f };
		float fgColor[3] = { -1.f, -1.f, -1.f };
		float bgColor[3] = { -1.f, -1.f, -1.f };
		if (debugConsoleOption)
		{
			memcpy(position, &debugConsoleOption->positionXInPercentageToScreenWidth, sizeof(float) * 2);
			memcpy(fgColor, &debugConsoleOption->fgColor, sizeof(float) * 3);
			memcpy(bgColor, &debugConsoleOption->bgColor, sizeof(float) * 3);
		}
		float lineSpacing = height * (debugConsoleOption ? debugConsoleOption->lineSpacingInPercentageToScreenHeight : 0.01f);
		float fontHeight  = height * (debugConsoleOption ? debugConsoleOption->fontHeightInPercentageToScreenHeight : 0.8f / 25.f - 0.01f);
		uint32_t lines = debugConsoleOption ? debugConsoleOption->heightInChars : 25;
		uint32_t nCharsPerLine = debugConsoleOption ? debugConsoleOption->widthInChars : 80;
		m_debugConsole = new Debug::Console(fontHeight, lines, nCharsPerLine, lineSpacing, position, name, fgColor, bgColor);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			return ret;
		}
		m_functionFlags |= kFunctionFlagDebugConsole;
	}

	if (functionFlags & kFunctionFlagJobQueue) {
		m_jobQueue = new Thread::JobQueue(jobQueueOption);
		SCE_SAMPLE_UTIL_ASSERT(m_jobQueue != nullptr);
		if (m_jobQueue == nullptr) {
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		m_functionFlags |= kFunctionFlagJobQueue;
	}

	if (functionFlags & kFunctionFlagAudioOut) {
		m_audioOut = new Audio::AudioOutContext(audioOutContextOption);
		SCE_SAMPLE_UTIL_ASSERT(m_audioOut != nullptr);
		if (m_audioOut == nullptr) {
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		m_functionFlags |= kFunctionFlagAudioOut;
	}

#ifdef _DEBUG
	if (Memory::g_isMatInitialized)
	{
		sceMatUpdateModuleList();
	}
#endif

	//Script
	m_isScriptRunning	= false;

	pScriptFilename = (pScriptFilename == nullptr) ? "/app0/script.txt" : pScriptFilename;
	m_scriptFileStream = std::ifstream(pScriptFilename, std::ifstream::in);
	if (m_scriptFileStream.good())
	{
		m_pScript = new sce::SampleUtil::ApplicationScript();
		SCE_SAMPLE_UTIL_ASSERT(m_pScript != nullptr);

		m_scriptPadData.lx	= 127;
		m_scriptPadData.ly	= 127;
		m_scriptPadData.rx	= 127;
		m_scriptPadData.ry	= 127;

		m_scriptPadData.l2	= 0;
		m_scriptPadData.r2	= 0;
		m_scriptPadButtonClearMask = ~0;
	}

	return SCE_OK;
}



int sce::SampleUtil::SampleSkeleton::updateUtil(SampleSkeletonUpdateParam *param)
{
	int ret = SCE_OK;

	uint64_t current_time;
	current_time = sceKernelGetProcessTime();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	m_deltaTime = (float)(current_time - m_previousTime) / 1000000.f;
	m_previousTime = current_time;

	if (m_pScript != nullptr)
	{
		if (!m_isScriptRunning)
		{
			m_isScriptRunning = true;
			printf("## Sample Application: script started ##\n");
		}
		m_scriptPadData.buttons &= ~m_scriptPadButtonClearMask;
		// First check if we are running a series of Frames and if so do complete before executing next script command
		if (m_scriptWaitFrames > 0)
		{
			--m_scriptWaitFrames;
		} else {
			m_pScript->executeNextCommand(m_scriptFileStream, *this);
			if (!m_isScriptRunning)
			{
				delete m_pScript; m_pScript = nullptr;
				printf("## Sample Application: script finished ##\n");
			}
		}
	}

	if (m_functionFlags & kFunctionFlagUserIdManager) {
		ret = m_userIdManager->update();
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	}

	if (m_functionFlags & kFunctionFlagPadOfInitialUser)
	{
		m_updateIntervalOfInput += m_deltaTime;
		// "4ms" is the minimum interval of pad update.
		//  This is a conditional statement for countermeasures at high frame rates (e.g. 2000fps)
		if (m_updateIntervalOfInput > 0.004f)
		{
			ret = m_padContextOfInitialUser->update((m_pScript != nullptr) ? &m_scriptPadData : nullptr);
			SCE_SAMPLE_UTIL_ASSERT(ret >= SCE_OK);
#if _SCE_TARGET_OS_PROSPERO
			if (m_vrControllerContextsOfInitialUser[0] != nullptr)
			{
				ret = m_vrControllerContextsOfInitialUser[0]->update();
				SCE_SAMPLE_UTIL_ASSERT(ret >= SCE_OK);
			}
			if ((m_vrControllerContextsOfInitialUser[1] != nullptr) && (m_vrControllerContextsOfInitialUser[1] != m_padContextOfInitialUser))
			{
				ret = m_vrControllerContextsOfInitialUser[1]->update();
				SCE_SAMPLE_UTIL_ASSERT(ret >= SCE_OK);
			}
#endif
			if (m_keyboardContextOfInitialUser)
			{
				ret = m_keyboardContextOfInitialUser->update();
				SCE_SAMPLE_UTIL_ASSERT(ret >= SCE_OK);
			}
			m_updateIntervalOfInput = 0.0f;
		}
	}

	if (m_functionFlags & kFunctionFlagImGui && param != nullptr && param->imguiParam != nullptr)
	{
		auto imguiParam = *param->imguiParam;
		if (imguiParam.renderTargetWidth == 0 || imguiParam.renderTargetHeight == 0)
		{
			imguiParam.renderTargetWidth	= m_graphicsContext->m_width;
			imguiParam.renderTargetHeight	= m_graphicsContext->m_height;
		}
		ret = Graphics::SpriteUtil::setRenderTargetSize(imguiParam.renderTargetWidth, imguiParam.renderTargetHeight); // force font rendering resolution consistent with one for ImGui because font rendering uses ImText
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		ret = UIFramework::imGuiStartNewFrame(imguiParam);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		m_isImguiNewFrameCalled = true;

		m_pPadForHud = param->imguiParam->pad;
	}

	if (m_functionFlags & kFunctionFlagDebugConsole) {
		ret = m_debugConsole->update();
		SCE_SAMPLE_UTIL_ASSERT(ret >= SCE_OK);
	}

	if (m_functionFlags & kFunctionFlagJobQueue) {
		m_jobQueue->check();
	}

	(void)ret;
	return SCE_OK;
}

int sce::SampleUtil::SampleSkeleton::renderUtilInternal(
#if _SCE_TARGET_OS_PROSPERO
														sce::Agc::Core::BasicContext	*currentCtx
#endif
														)
{
	if (m_functionFlags & kFunctionFlagImGui)
	{
		uint64_t currFlipCount;
#if _SCE_TARGET_OS_PROSPERO
		int ret = m_graphicsContext->getSubmittedFlipCount(currFlipCount, m_graphicsContext->m_option.baseFrameBus);
#endif
#if _SCE_TARGET_OS_ORBIS
		int ret = m_graphicsContext->getSubmittedFlipCount(currFlipCount, SCE_VIDEO_OUT_BUS_TYPE_MAIN);
#endif
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		if ((m_prevFlipCount != currFlipCount)
#if _SCE_TARGET_OS_PROSPERO
			|| (m_prevBaseFrameBus != m_graphicsContext->m_option.baseFrameBus)
#endif
			)
		{
			if (m_isImguiNewFrameCalled)
			{
				if (m_functionFlags & kFunctionFlagSpriteUtility)
				{
					Graphics::SpriteUtil::deferredGenerateDrawStringImguiCommands();
				}
				ret = UIFramework::imGuiEndFrame();
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				if (ret != SCE_OK)
				{
					return ret;
				}
			}

			m_prevFlipCount = currFlipCount;
#if _SCE_TARGET_OS_PROSPERO
			m_prevBaseFrameBus = m_graphicsContext->m_option.baseFrameBus;
#endif
		}
		if (m_isImguiNewFrameCalled)
		{
#if _SCE_TARGET_OS_PROSPERO
			ret = UIFramework::imGuiRender(currentCtx->m_dcb);
#endif
#if _SCE_TARGET_OS_ORBIS
			ret = UIFramework::imGuiRender(*m_graphicsContext->m_currentCtx);
#endif
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return ret;
			}
		}
		m_isImguiNewFrameCalled = false;
	}

	return SCE_OK;
}

int sce::SampleUtil::SampleSkeleton::finalizeUtil()
{
	int ret;

	SCE_SAMPLE_UTIL_SAFE_DELETE(m_pScript);

	if((m_functionFlags & kFunctionFlagDebugConsole) &&
		(m_functionFlags & kFunctionFlagImGui) &&
		(m_functionFlags & kFunctionFlagGraphics))
	{
		SCE_SAMPLE_UTIL_SAFE_DELETE(m_debugConsole);
	}

	if((m_functionFlags & kFunctionFlagImGui) &&
		(m_functionFlags & kFunctionFlagGraphics))
	{
		ret = UIFramework::imGuiFinalize();
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return ret;
		}
	}

	if (m_functionFlags & kFunctionFlagPadOfInitialUser) {
#if _SCE_TARGET_OS_PROSPERO
		if (m_functionFlags & kFunctionFlagVr)
		{
			if (m_vrControllerContextsOfInitialUser[0] != nullptr) {
				ret = sceVrTracker2UnregisterDevice(m_vrControllerContextsOfInitialUser[0]->getPadHandle());
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				SCE_SAMPLE_UTIL_SAFE_DELETE(m_vrControllerContextsOfInitialUser[0]);
			}

			if (m_vrControllerContextsOfInitialUser[1] != nullptr) {
				ret = sceVrTracker2UnregisterDevice(m_vrControllerContextsOfInitialUser[1]->getPadHandle());
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				SCE_SAMPLE_UTIL_SAFE_DELETE(m_vrControllerContextsOfInitialUser[1]);
			}

			if (m_hmdHandleOfInitialUser >= 0) {
				ret = sceVrTracker2UnregisterDevice(m_hmdHandleOfInitialUser);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				ret = sceHmd2Close(m_hmdHandleOfInitialUser);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			}
		}
#endif
		SCE_SAMPLE_UTIL_SAFE_DELETE(m_padContextOfInitialUser);
		SCE_SAMPLE_UTIL_SAFE_DELETE(m_keyboardContextOfInitialUser);
		SCE_SAMPLE_UTIL_SAFE_DELETE(m_oskContextOfInitialUser);
	}

	if(m_functionFlags & kFunctionFlagUserIdManager){
		SCE_SAMPLE_UTIL_SAFE_DELETE(m_userIdManager);
	}

	if(m_functionFlags & kFunctionFlagGraphics){
		while (m_graphicsContext->m_deferredGpuMemoryRelease.size() > 0)
		{
			m_graphicsContext->handleDeferredGpuMemoryRelease(true);
		}

		if (m_functionFlags & kFunctionFlagSpriteUtility)
		{
			ret = Graphics::SpriteUtil::finalize();
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK) {
				return ret;
			}
		}
		SCE_SAMPLE_UTIL_SAFE_DELETE(m_graphicsContext);
		Graphics::finalizeResourceRegistration();
	}

	if (m_functionFlags & kFunctionFlagJobQueue) {
		SCE_SAMPLE_UTIL_SAFE_DELETE(m_jobQueue);
	}

	if (m_functionFlags & kFunctionFlagAudioOut) {
		SCE_SAMPLE_UTIL_SAFE_DELETE(m_audioOut);
	}

#if	_SCE_TARGET_OS_PROSPERO
#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
	SCE_SAMPLE_UTIL_SAFE_DELETE(m_asyncAssetLoader);
#endif
	if ((m_functionFlags & kFunctionFlagVr))
	{
		SCE_SAMPLE_UTIL_SAFE_DELETE(m_vrTracker);
		SCE_SAMPLE_UTIL_SAFE_DELETE(m_reprojection);
	}
#endif

	Debug::Perf::finalize();
	Memory::finalizeMemoryAnalyzer();

	printf("## Sample Application: finalized ##\n");

	return SCE_OK;
}


sce::SampleUtil::Graphics::GraphicsContext *sce::SampleUtil::SampleSkeleton::getGraphicsContext()
{
	return (m_functionFlags & kFunctionFlagGraphics) ? m_graphicsContext : nullptr;
}

sce::SampleUtil::System::UserIdManager *sce::SampleUtil::SampleSkeleton::getUserIdManager()
{
	return (m_functionFlags & kFunctionFlagUserIdManager) ? m_userIdManager : nullptr;
}

sce::SampleUtil::Input::PadContext *sce::SampleUtil::SampleSkeleton::getPadContextOfInitialUser(int type)
{
#if _SCE_TARGET_OS_PROSPERO
	sce::SampleUtil::Input::PadContext *pad = nullptr;
	if (m_functionFlags & kFunctionFlagPadOfInitialUser)
	{
		if (type == 0)
		{
			pad = m_padContextOfInitialUser;
		} else if (m_functionFlags & kFunctionFlagVr)
		{
			if (type == SCE_PAD_PORT_TYPE_VR_CONTROLLER_LEFT)
			{
				pad = m_vrControllerContextsOfInitialUser[0];
			} else if (type == SCE_PAD_PORT_TYPE_VR_CONTROLLER_RIGHT)
			{
				pad = m_vrControllerContextsOfInitialUser[1];
			}
		}
	}
	return pad;
#else
	return (m_functionFlags & kFunctionFlagPadOfInitialUser) ? m_padContextOfInitialUser : nullptr;
#endif
}

sce::SampleUtil::Input::KeyboardContext *sce::SampleUtil::SampleSkeleton::getKeyboardContextOfInitialUser()
{
	return (m_functionFlags & kFunctionFlagPadOfInitialUser) ? m_keyboardContextOfInitialUser : nullptr;
}

sce::SampleUtil::Input::OskContext *sce::SampleUtil::SampleSkeleton::getOskContextOfInitialUser()
{
	return (m_functionFlags & kFunctionFlagPadOfInitialUser) ? m_oskContextOfInitialUser : nullptr;
}

sce::SampleUtil::Debug::Console *sce::SampleUtil::SampleSkeleton::getDebugConsole()
{
	return (m_functionFlags & kFunctionFlagDebugConsole) ? m_debugConsole : nullptr;
}

sce::SampleUtil::Thread::JobQueue *sce::SampleUtil::SampleSkeleton::getJobQueue()
{
	return (m_functionFlags & kFunctionFlagJobQueue) ? m_jobQueue : nullptr;
}

sce::SampleUtil::Audio::AudioOutContext *sce::SampleUtil::SampleSkeleton::getAudioOut()
{
	return (m_functionFlags & kFunctionFlagAudioOut) ? m_audioOut : nullptr;
}
#if _SCE_TARGET_OS_PROSPERO
sce::SampleUtil::Helper::AsyncAssetLoader *sce::SampleUtil::SampleSkeleton::getAsyncAssetLoader()
{
	return m_asyncAssetLoader;
}
#endif

bool	sce::SampleUtil::SampleSkeleton::enableHmd()
{
	int ret = SCE_OK; (void)ret;

	bool	isHmdEnabled = true;

	if ((m_functionFlags & kFunctionFlagVr) && m_useVrSetupDialog)
	{
#if _SCE_TARGET_OS_PROSPERO
		ret = sceSysmoduleLoadModule(SCE_SYSMODULE_VR_SETUP_DIALOG);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceVrSetupDialogInitialize();
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK);
	
		SceVrSetupDialogParam dialogParam;
		sceVrSetupDialogParamInitialize(&dialogParam);

		do
		{
			ret = sceVrSetupDialogOpen(&dialogParam);
			if (ret == SCE_VR_SETUP_DIALOG_ERROR_SERVICE_BUSY)
			{
				printf("## SampleUtil retrying sceVrSetupDialogOpen...");
				sceKernelUsleep(1000 * 1000);
			}
		} while (ret == SCE_VR_SETUP_DIALOG_ERROR_SERVICE_BUSY);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		ret = sceVrSetupDialogGetStatus();
		while (ret != SCE_VR_SETUP_DIALOG_STATUS_FINISHED)
		{
			ret = sceVrSetupDialogUpdateStatus();
		}

		SceVrSetupDialogResult result;
		result.result = SCE_VR_SETUP_DIALOG_RESULT_OK;
		ret = sceVrSetupDialogGetResult(&result);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		ret = sceVrSetupDialogTerminate();
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_VR_SETUP_DIALOG);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		isHmdEnabled = (result.result == SCE_VR_SETUP_DIALOG_RESULT_OK);
#endif
	}

	return isHmdEnabled;
}
