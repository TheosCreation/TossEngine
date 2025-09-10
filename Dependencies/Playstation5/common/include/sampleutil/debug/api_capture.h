/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once

#include <cinttypes>
#include <vector>
#include <unordered_map>
#include <kernel.h>
#include <mspace.h>
#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include <gnmx/context.h>
#endif
#if _SCE_TARGET_OS_PROSPERO
#include <agc/drawcommandbuffer.h>
#endif
#include "sampleutil/graphics/compat.h"
#include "sampleutil/ui_framework/ui_framework.h"
#include "sampleutil/system.h"
#include "sampleutil/thread/thread.h"

#define STRINGIFY(n) #n
#define TOSTRING(n) STRINGIFY(n)

#ifndef SCE_SAMPLE_UTIL_ENABLE_API_CAPTURE
#define API_CAPTURE_BEGIN_FRAME
#define API_CAPTURE_END_FRAME
#define API_CAPTURE(statement)	statement
#define API_CAPTURE_PUSH_MARKER(label)
#define API_CAPTURE_POP_MARKER
#else
/*! @def API_CAPTURE_BEGIN_FRAME 
 * @~English
 * 
 * @brief Beginning of API capture frame
 * @details Put this macro at the beginning of API capture frame
 * @~Japanese
 * 
 * @brief APIキャプチャフレームの開始
 * @details APIキャプチャフレームの開始点にこのマクロを置きます。
 */
#define API_CAPTURE_BEGIN_FRAME																		\
{																									\
		sce::SampleUtil::Debug::ApiCapture::Frames::begin();										\
}
/*! @def API_CAPTURE_END_FRAME 
 * @~English
 * 
 * @brief End of API capture frame
 * @details Put this macro at the end of API capture frame
 * @~Japanese
 * 
 * @brief APIキャプチャフレームの終了
 * @details APIキャプチャフレームの終了点にこのマクロを置きます。
 */
#define API_CAPTURE_END_FRAME																		\
{																									\
		sce::SampleUtil::Debug::ApiCapture::Frames::end();											\
}
/*! @def API_CAPTURE(statement)
 * @~English
 * 
 * @param statement Statement invokation to capture
 * 
 * @brief API capture
 * @details The statement enclosed by this macro is captured
 * @~Japanese
 * 
 * @param statement キャプチャするステートメント呼び出し
 * 
 * @brief APIのキャプチャ
 * @details マクロで囲ったステートメント呼び出しがキャプチャされます。
 */
#define API_CAPTURE(statement)																		\
({																									\
	sce::SampleUtil::Debug::ApiCapture::Tag _tag(__FILE__ ":" TOSTRING(__LINE__) ": " #statement);	\
	(statement);																					\
})
/*! @def API_CAPTURE_PUSH_MARKER(label)
 * @~English
 * 
 * @param label Label string with which marker is started
 * 
 * @brief Push marker
 * @details This function starts marker with specified label string
 * @~Japanese
 * 
 * @param label マーカーを開始する際にしているするラベル文字列
 * 
 * @brief マーカーのプッシュ
 * @details この関数は指定したラベル文字列でマーカーを開始する
 */
#define API_CAPTURE_PUSH_MARKER(label) sce::SampleUtil::Debug::ApiCapture::PushPopTags::push(label)
/*! @def API_CAPTURE_POP_MARKER
 * @~English
 * 
 * @brief Pop marker
 * @details This function ends marker
 * @~Japanese
 * 
 * @brief マーカーのポップ
 * @details この関数はマーカーを終了する
 */
#define API_CAPTURE_POP_MARKER sce::SampleUtil::Debug::ApiCapture::PushPopTags::pop()
#endif

// Double buffering Captured Apis
#define NUM_API_CAPTURE_BUFFER	2
#define NUM_API_CAPTURE_MAX_THREADS 64

namespace sce {	namespace SampleUtil { namespace Debug {
namespace ApiCapture {
	/*!
	 * @~Japanese
	 * @brief renderApiCaptureHud()のパラメータ
	 * @~English
	 * @brief Parameter to renderApiCaptureHud()
	 */
	struct RenderHudParam
	{
		/*!
		 * @~English
		 * @brief Pad context to be used(default pad is used if nullptr is specified)
		 * @~Japanese
		 * @brief HUD操作で使用するPad context(nullptr指定時には既定のPad contextが使用される)
		 */
		Input::PadContext					*m_pPad;
		/*!
		 * @~English
		 * @brief Render target to draw HUD(default render target is used if nullptr is specified)
		 * @~Japanese
		 * @brief HUDの描画先レンダーターゲット(nullptr指定時には既定のレンダーターゲットが使用される)
		 */
		Graphics::Compat::RenderTarget		*m_pRenderTarget;
#if _SCE_TARGET_OS_PROSPERO
		/*!
		 * @~English
		 * @brief draw command buffer to be used
		 * @~Japanese
		 * @brief 描画に使用するDrawCommandBuffer
		 */
		Agc::DrawCommandBuffer				*m_pDcb;
#endif
#if _SCE_TARGET_OS_ORBIS
		Gnmx::GnmxGfxContext				*m_pGfxc;			//!< gfx context to be used
#endif
		uint32_t							m_targetWidth;
		uint32_t							m_targetHeight;
		bool								m_hasOwnImgui;
		SceLibcMspace						m_cpuMemory;
		RenderHudParam() :m_pPad(nullptr), m_pRenderTarget(nullptr), m_hasOwnImgui(false), m_cpuMemory(nullptr) {}
	};

	/*!
	 * @~Japanese
	 * @brief HUD描画コンテキスト
	 * @~English
	 * @brief Context to draw HUD
	 */
	class RenderHudContext
	{
	public:
		Input::PadContext		*m_pPad;
		bool					m_showHud;
		std::string				m_hilitedStrId;

		RenderHudContext() : m_pPad(nullptr), m_showHud(false) {}

		static RenderHudContext m_singleton;
		static RenderHudContext	&get() { return m_singleton; }

		void	generateTimelineImguiCommands(uint32_t	targetWidth);
		void	generateTreeviewImguiCommands();
		void	generateImguiCommands(uint32_t	targetWidth, uint32_t	targetHeight);

		/*!
		 * @~English		
		 * @brief Render HUD
		 * @details This function renders captured APIs in a frame in HUD style
		 * @param param Parameters to draw HUD
		 * @~Japanese		
		 * @brief HUDを描画
		 * @details 1フレーム分のキャプチャされたAPIをHUDスタイルで描画
		 * @param param HUD描画のパラメータ
		*/
		void	render(RenderHudParam	param);
	};

	static inline void	renderHud(RenderHudParam	param)
	{
		RenderHudContext::get().render(param);
	}

	class Frame
	{
	public:
		struct ApiData
		{
			const char	*m_pApiStatement;
			uint64_t	m_startTime;
			uint64_t	m_endTime;
			int			m_startCpuCoreId;
			int			m_endCpuCoreId;

			int			m_index;
			int			m_nestLevel;
		};

		uint64_t				m_frameStartTime;
		uint64_t				m_frameEndTime;
		std::vector<ApiData>	m_capturedApis;
		int						m_currentNestLevel;
	}; // class Frame

	class Frames : public Thread::LockableObject
	{
	public:
		static Frames	m_singleton;

		bool					m_isInFrame;
		uint64_t				m_frameCount = 0;
		uint32_t				m_bufferIndex;
		uint32_t				m_backBufferIndex;
		Frame					m_frameData[NUM_API_CAPTURE_BUFFER][NUM_API_CAPTURE_MAX_THREADS];
		std::vector<ScePthread>	m_threads;
		std::vector<ScePthread>	m_threadsBack;
		static Frame	&get(bool isAlreadyLocked = false, ScePthread thread = scePthreadSelf())
		{
			int thread_i = std::find(m_singleton.m_threads.begin(), m_singleton.m_threads.end(), thread) - m_singleton.m_threads.begin();
			if (isAlreadyLocked)
			{
				return m_singleton.m_frameData[m_singleton.m_bufferIndex][thread_i];
			}
			Thread::ScopedLock criticalSection(&Frames::m_singleton, Thread::LockableObjectAccessAttr::kRead);
			return m_singleton.m_frameData[m_singleton.m_bufferIndex][thread_i];
		}
		static Frame	&getBack(bool isAlreadyLocked = false, ScePthread thread = scePthreadSelf())
		{
			int thread_i = std::find(m_singleton.m_threadsBack.begin(), m_singleton.m_threadsBack.end(), thread) - m_singleton.m_threadsBack.begin();
			if (isAlreadyLocked)
			{
				return m_singleton.m_frameData[m_singleton.m_backBufferIndex][thread_i];
			}
			Thread::ScopedLock criticalSection(&Frames::m_singleton, Thread::LockableObjectAccessAttr::kRead);
			return m_singleton.m_frameData[m_singleton.m_backBufferIndex][thread_i];
		}
		static bool		isRender(bool isAlreadyLocked = false)
		{
			if (isAlreadyLocked)
			{
				return (m_singleton.m_bufferIndex != m_singleton.m_backBufferIndex);
			}
			Thread::ScopedLock criticalSection(&Frames::m_singleton, Thread::LockableObjectAccessAttr::kRead);
			return (m_singleton.m_bufferIndex != m_singleton.m_backBufferIndex);
		}

		__attribute__((noinline))
		static void	begin();
		__attribute__((noinline))
		static void	end();
	}; // class Frames

	class Tag
	{
		int	m_index;
		uint64_t m_frameCount;
		__attribute__((noinline))
		int begin(const char *pApiStatement);
		__attribute__((noinline))
		void end(int	index);
	public:
		__attribute__((always_inline)) Tag(const char *pApiStatement)
		{
			{
				m_index = begin(pApiStatement);
			}
		}
		__attribute__((always_inline)) ~Tag()
		{
			{
				if (m_index >= 0)
				{
					end(m_index);
				}
			}
		}
	}; // class Tag

	class PushPopTags : public Thread::LockableObject
	{
		static PushPopTags	m_singleton;
		std::unordered_map<ScePthread, std::vector<Tag *>> m_tags;
	public:
		static void	push(const char	*pLabel);
		static void	pop();
	};
} // namespace ApiCapture
} } } // namespace sce::SampleUtil::Debug
