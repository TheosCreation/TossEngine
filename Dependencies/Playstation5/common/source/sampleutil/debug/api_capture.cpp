/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2020 Sony Interactive Entertainment Inc.
* 
*/

#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/toolkit/toolkit.h>
#endif
#if _SCE_TARGET_OS_ORBIS
#include "sampleutil/graphics/surface_utility.h"
#endif
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/debug/api_capture.h"
#include "../../imgui/imgui_internal.h"

namespace
{
	bool	isHovered(const std::string	&strId)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiContext& g = *GImGui;

		ImGuiID id = window->GetID(strId.c_str());
		return (g.NavId == id && !g.NavDisableHighlight && g.NavDisableMouseHover && (g.ActiveId == 0 || g.ActiveId == id || g.ActiveId == window->MoveId));
	}

} // anonymous namespace

namespace sce {	namespace SampleUtil { namespace Debug {
namespace ApiCapture {
	PushPopTags			PushPopTags::m_singleton;
	Frames				Frames::m_singleton;
	RenderHudContext	RenderHudContext::m_singleton; //!< context used for hud rendering

	void	PushPopTags::push(const char	*pLabel)
	{
		Thread::ScopedLock criticalSection(&m_singleton, Thread::LockableObjectAccessAttr::kWrite);

		ScePthread self = scePthreadSelf();
		if (m_singleton.m_tags.find(self) == m_singleton.m_tags.end())
		{
			m_singleton.m_tags.insert(std::make_pair(self, std::vector<Tag *>(0)));
		}

		auto &tags = m_singleton.m_tags[self];
		tags.push_back(new Tag(pLabel));
	}

	void	PushPopTags::pop()
	{
		Thread::ScopedLock criticalSection(&m_singleton, Thread::LockableObjectAccessAttr::kRead);

		ScePthread self = scePthreadSelf();
		SCE_SAMPLE_UTIL_ASSERT(m_singleton.m_tags.find(self) != m_singleton.m_tags.end());

		auto &tags = m_singleton.m_tags[self];
		SCE_SAMPLE_UTIL_ASSERT(tags.size() > 0);
		delete tags.back();
		tags.pop_back();
	}

	void	Frames::begin()
	{
		Thread::ScopedLock criticalSection(&m_singleton, Thread::LockableObjectAccessAttr::kWrite);

		m_singleton.m_isInFrame = true;

		uint64_t frameStartTime = sceKernelGetProcessTime();
		for (ScePthread thread : m_singleton.m_threads)
		{
			auto &frame = get(true, thread);

			frame.m_capturedApis.clear();
			frame.m_currentNestLevel	= 0;
			frame.m_frameStartTime		= frameStartTime;
		}
	}

	void	Frames::end()
	{
		Thread::ScopedLock criticalSection(&Frames::m_singleton, Thread::LockableObjectAccessAttr::kWrite);

		if (!m_singleton.m_isInFrame) return;
		m_singleton.m_isInFrame = false;

		uint64_t frameEndTime = sceKernelGetProcessTime();
		for (ScePthread thread : m_singleton.m_threads)
		{
			auto &frame = get(true, thread);

			frame.m_frameEndTime	= frameEndTime;
#ifdef SCE_SAMPLE_UTIL_ENABLE_API_CAPTURE_TTY_OUTPUT
			if (frame.m_capturedApis.size() > 0)
			{
				printf("\n\t===== API_CAPTURE frame dump ====\n");
				printf("frame time: %ldus\n", frame.m_frameEndTime - frame.m_frameStartTime);
				char pThreadName[32]; scePthreadGetname(thread, pThreadName);
				printf("Thread : %s\n", pThreadName);
				printf("             |CPU core|  start-    end(duration[ratio])| statement\n");
				printf("-------------+--------+--------------------------------+----------\n");
				for (const Frame::ApiData &api : frame.m_capturedApis)
				{
					if (api.m_startCpuCoreId == api.m_endCpuCoreId)
					{
						printf("[API_CAPTURE] CPU%d     ", api.m_startCpuCoreId);
					} else {
						printf("[API_CAPTURE] CPU%d>%d   ", api.m_startCpuCoreId, api.m_endCpuCoreId);
					}
					uint64_t endTime = std::min(api.m_endTime, frame.m_frameEndTime);
					printf("%7ld-%7ld(%6ldus[%5.2f%%]) ",
						api.m_startTime - frame.m_frameStartTime,
						endTime - frame.m_frameStartTime,
						endTime - api.m_startTime,
						100.0f * (float)(endTime - api.m_startTime) / (float)(frame.m_frameEndTime - frame.m_frameStartTime));

					for (int i = 0; i < (int)api.m_nestLevel; i++)
					{
						if (i == (int)api.m_nestLevel - 1)
						{
							printf("|--");
						} else {
							printf("|  ");
						}
					}
					printf("%s\n", api.m_pApiStatement);
				}
			}
#endif
		}
		// flip
		m_singleton.m_backBufferIndex = m_singleton.m_bufferIndex;
		m_singleton.m_threadsBack = m_singleton.m_threads;
		m_singleton.m_bufferIndex = (m_singleton.m_bufferIndex + 1) % NUM_API_CAPTURE_BUFFER;
		++m_singleton.m_frameCount;
	}

	int		Tag::begin(const char	*pApiStatement)
	{
		Thread::ScopedLock criticalSection(&Frames::m_singleton, Thread::LockableObjectAccessAttr::kWrite);

		if (!Frames::m_singleton.m_isInFrame)
		{
			return -1;
		}

		ScePthread self = scePthreadSelf();
		if (std::find(Frames::m_singleton.m_threads.begin(), Frames::m_singleton.m_threads.end(), self) == Frames::m_singleton.m_threads.end())
		{
			SCE_SAMPLE_UTIL_ASSERT(Frames::m_singleton.m_threads.size() < NUM_API_CAPTURE_MAX_THREADS);
			Frames::m_singleton.m_threads.push_back(self);
			auto &frame = Frames::get(true, self);
			frame.m_currentNestLevel = 0;
		}

		auto &frame = Frames::get(true, self);

		frame.m_capturedApis.emplace_back();
		Frame::ApiData &data = frame.m_capturedApis.back();
		data.m_pApiStatement	= pApiStatement;
		data.m_startTime		= sceKernelGetProcessTime();
		data.m_endTime			= UINT64_MAX;
		data.m_startCpuCoreId	= sceKernelGetCurrentCpu();
		data.m_index			= frame.m_capturedApis.size();
		data.m_nestLevel		= frame.m_currentNestLevel++;
		m_frameCount = Frames::m_singleton.m_frameCount;
		return frame.m_capturedApis.size() - 1;
	}

	void	Tag::end(int	index)
	{
		Thread::ScopedLock criticalSection(&Frames::m_singleton, Thread::LockableObjectAccessAttr::kWrite);

		if (m_frameCount != Frames::m_singleton.m_frameCount || !Frames::m_singleton.m_isInFrame)
		{
			return;
		}

		auto &frame = Frames::get(true);

		Frame::ApiData &data = frame.m_capturedApis[index];
		data.m_endTime		= sceKernelGetProcessTime();
		data.m_endCpuCoreId	= sceKernelGetCurrentCpu();
		--frame.m_currentNestLevel;
	}

	void	RenderHudContext::generateTimelineImguiCommands(uint32_t	targetWidth)
	{
		ImGuiContext& g = *ImGui::GetCurrentContext();
		ImGuiWindow* window = g.CurrentWindow;

		// Left-top
		const ImVec2 kBasePos(window->Size.x / 19.2, window->Size.y / 10.8f);
		// Vertical margin
		const float kMarginY = window->Size.y / 34.0f;
		
		uint32_t numThreads = 0;
		for (ScePthread thread : Frames::m_singleton.m_threadsBack)
		{
			Frame &frame = Frames::getBack(true, thread);
			if (frame.m_capturedApis.size() > 0)
			{
				++numThreads;
			}
		}
		if (numThreads == 0)
		{
			return;
		}

		const float heightScale		= std::max(1.3f - 0.3f * (float)numThreads, 0.2f);
		const float timelineHeight	= 128.0f * heightScale;
		const float marginY			= kMarginY * heightScale;
		uint32_t fontHeight	= (ImGui_ImplSampleUtil_defaultFontSize * (uint32_t)window->Size.y / 2160) * std::max(5 - numThreads, 1u) / 6;

		// Timeline size of 1 line
		const ImVec2 timelineSize(targetWidth - kBasePos.x * 2.0f, timelineHeight);
		// Calculate timing
		float baseHeight = kBasePos.y;
		for (ScePthread thread : Frames::m_singleton.m_threadsBack)
		{
			Frame &frame = Frames::getBack(true, thread);
			const float totalTime = frame.m_frameEndTime - frame.m_frameStartTime;
			float nextBaseHeight = baseHeight;
			std::vector<float> maxX;
			for (const auto &api : frame.m_capturedApis)
			{
				uint64_t endTime = std::min(api.m_endTime, frame.m_frameEndTime);
				const float startInTotal	= ((float)(api.m_startTime - frame.m_frameStartTime)) / totalTime;
				const float endInTotal		= ((float)(endTime - frame.m_frameStartTime)) / totalTime;
				while (maxX.size() <= api.m_nestLevel)
				{
					maxX.push_back(0.f);
				}
				const float startX			= std::max(kBasePos.x + timelineSize.x * startInTotal, maxX[api.m_nestLevel]);
				const float endX			= std::max(kBasePos.x + timelineSize.x * endInTotal, startX + 1.f);
				maxX[api.m_nestLevel] = std::max(endX, maxX[api.m_nestLevel]);
				// Calculate position
				const ImVec2 barTopLeft(startX, (timelineSize.y + marginY) * api.m_nestLevel + baseHeight);
				const ImVec2 barBottomRight(endX, barTopLeft.y + timelineSize.y);
				nextBaseHeight = std::max(nextBaseHeight, barBottomRight.y);

				std::string str = api.m_pApiStatement;
				std::string statement = str.substr(str.find(": ") + 2);
				ImU32 hashColor = ImHashStr(statement.c_str(), statement.length()) | 0xff000000;	// Use as color
				std::string str_id = statement + "##TreeItem" + std::to_string(api.m_index)+ "_" + std::to_string((uintptr_t)thread);

				statement = statement.substr(0, std::min((size_t)statement.find("("), (size_t)64u));

				// Draw bar rectangle
				if (str_id == m_hilitedStrId)
				{
					window->DrawList->AddRectFilled(barTopLeft, barBottomRight, hashColor | 0xfcfcfc);
				} else {
					window->DrawList->AddRectFilled(barTopLeft, barBottomRight, hashColor);
				}

				const float scale = fontHeight / ImGui::GetTextLineHeight();
				const ImVec2 textSize = ImGui::CalcTextSize(statement.c_str());
				// Draw statement
				if (textSize.x * scale > barBottomRight.x - barTopLeft.x)
				{
					window->DrawList->AddText(nullptr,
						fontHeight,
						ImVec2(barTopLeft.x - textSize.x * scale * 0.5f, barTopLeft.y + (timelineSize.y - fontHeight) / 2.0f),
						0xff0f0f0f,
						statement.c_str());
				} else {
					window->DrawList->PushClipRect(barTopLeft, barBottomRight);
					window->DrawList->AddText(nullptr,
						fontHeight,
						ImVec2(barTopLeft.x, barTopLeft.y + (timelineSize.y - fontHeight) / 2.0f),
						0xff0f0f0f,
						statement.c_str());
					window->DrawList->PopClipRect();
				}
			}
			baseHeight = nextBaseHeight + marginY;
		}
	}

	void	RenderHudContext::generateTreeviewImguiCommands()
	{
		Thread::ScopedLock criticalSection(&Frames::m_singleton, Thread::LockableObjectAccessAttr::kRead);

		ImGuiContext& g = *ImGui::GetCurrentContext();
		ImGuiWindow* window = g.CurrentWindow;
		const float baseX = window->Size.x / 24.f;

		const auto &frame = Frames::getBack();
		// header
		ImGui::SetCursorPosX(baseX);	ImGui::Text("--------------------------------+----------\n");
		ImGui::SetCursorPosX(baseX);	ImGui::Text("frame time: %ldus\n", frame.m_frameEndTime - frame.m_frameStartTime);
		ImGui::SetCursorPosX(baseX);	ImGui::Text("  start-    end(duration[ratio])| statement\n");
		ImGui::SetCursorPosX(baseX);	ImGui::Text("--------------------------------+----------\n");

		uint32_t numThreads = 0;
		for (ScePthread thread : Frames::m_singleton.m_threads)
		{
			Frame &frame = Frames::getBack(true, thread);
			if (frame.m_capturedApis.size() > 0)
			{
				++numThreads;
			}
		}
		if (numThreads == 0)
		{
			return;
		}

		m_hilitedStrId = "";
		for (ScePthread thread : Frames::m_singleton.m_threads)
		{
			Frame &frame = Frames::getBack(true, thread);
			if (numThreads > 1 && frame.m_capturedApis.size() > 0)
			{
				char pThreadName[32];
				scePthreadGetname(thread, pThreadName);
				ImGui::Text("[%s]", pThreadName);
			}

			bool prevOpened = false;
			int prevNest = 0;
			int skipNestLevel = 1000;
			float statementBaseX = (float)window->Size.x / 4.f;
			for (auto apiIter = frame.m_capturedApis.begin(); apiIter != frame.m_capturedApis.end(); apiIter++)
			{
				const Frame::ApiData &api = *apiIter;
				if (skipNestLevel < api.m_nestLevel) continue; // skip because ancestor tree is not open

				if (prevOpened && api.m_nestLevel <= prevNest) // pop for leaf node
				{
					ImGui::TreePop();
				}

				if (api.m_nestLevel < prevNest) // pops downto current nest level
				{
					int numPops = prevNest - api.m_nestLevel;
					while (numPops-- > 0)
					{
						ImGui::TreePop();
					}
				}
			
				std::string str = api.m_pApiStatement;
				std::string statement = str.substr(str.find(": ") + 2);
				std::string str_id = statement + "##TreeItem" + std::to_string(api.m_index)+ "_" + std::to_string((uintptr_t)thread);

				uint64_t endTime = std::min(api.m_endTime, frame.m_frameEndTime);
				char buffer[128];
				sprintf_s(buffer, sizeof(buffer), "%7ld-%7ld(%6ldus[%5.2f%%])",
					api.m_startTime - frame.m_frameStartTime,
					endTime - frame.m_frameStartTime,
					endTime - api.m_startTime,
					100.0f * (float)(endTime - api.m_startTime) / (float)(frame.m_frameEndTime - frame.m_frameStartTime));

				// put time
				ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Once);
				ImGui::SetCursorPosX(baseX);
				{
					char temp[128];
					sprintf_s(temp, sizeof(temp), "%40s : ", buffer);
					ImVec2 statementSize = ImGui::CalcTextSize(temp);
					statementBaseX = std::max(statementBaseX, baseX + statementSize.x + 5);
					ImGui::Text("%s", temp);
				}
				// put statement
				ImGui::SameLine();
				ImGui::SetCursorPosX(statementBaseX + api.m_nestLevel * ((float)window->Size.x / 38.4f));
	
				bool isLeaf = (apiIter == frame.m_capturedApis.end() - 1) || (api.m_nestLevel >= (apiIter + 1)->m_nestLevel);
				if (isHovered(str_id))
				{
					m_hilitedStrId = str_id;
				}
				bool opened = ImGui::TreeNodeEx(str_id.c_str(), isLeaf ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None, "");
				if (apiIter == frame.m_capturedApis.begin())
				{
					ImGui::SetItemDefaultFocus();
				}
				ImU32 hashColor = ImHashStr(statement.c_str(), statement.length()) | 0xff000000;	// Use as color
				ImGui::SameLine();	ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(hashColor), "%s", statement.c_str());

				skipNestLevel = opened ? 1000 : api.m_nestLevel;

				prevNest = api.m_nestLevel;
				prevOpened = opened;
			}
			if (prevOpened) // pop for leaf node
			{
				ImGui::TreePop();
			}
			while (prevNest-- > 0)
			{
				ImGui::TreePop();
			}
		}

		// for changing default focus
		{
			ImGuiContext *g = ImGui::GetCurrentContext();
			// Window出現時にのみ処理する
			if (g->CurrentWindow->Appearing)
			{
				g->NavDisableHighlight = false;
				g->NavDisableMouseHover = true;
			}
		}
	}

	void	RenderHudContext::generateImguiCommands(uint32_t	targetWidth, uint32_t	targetHeight)
	{
		// Render texture to FrameBuffer
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.8f));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
		ImGui::SetNextWindowSize(ImVec2(targetWidth, targetHeight), ImGuiCond_Once);
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
		ImGui::Begin("API Capture HUD", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::SetWindowFontScale((float)targetHeight / 2160.f);
		ImGui::SetCursorPos(ImVec2((float)targetWidth / 24.f, (float)targetHeight / 21.6f));
		ImGui::Text("===== API_CAPTURE frame dump ====");

		// Render details when backbuffer exists.
		if (Frames::isRender())
		{
			generateTimelineImguiCommands(targetWidth);
			ImGui::SetCursorPosY(targetHeight * 0.42f);

			generateTreeviewImguiCommands();
		}

		ImGui::End();
		ImGui::PopStyleColor(2);
	}

	void	RenderHudContext::render(RenderHudParam	param)
	{
		auto pPad = param.m_pPad ? param.m_pPad : RenderHudContext::get().m_pPad;
		SCE_SAMPLE_UTIL_ASSERT(pPad != nullptr || !param.m_hasOwnImgui);

		static bool isL1L3Down_prev = false;
		if (pPad)
		{
			bool isL1L3Down = pPad->isButtonDown(sce::SampleUtil::Input::kButtonL1) && pPad->isButtonDown(sce::SampleUtil::Input::kButtonL3);
			if (isL1L3Down_prev && !isL1L3Down)
			{
				m_showHud = !m_showHud;
			}
			isL1L3Down_prev = isL1L3Down;
		}
		if (!m_showHud)
		{
			return;
		}

		if (param.m_hasOwnImgui)
		{
			ImGuiIO	&io = ImGui::GetIO();
			if (ImGuiLibFont::BuildFontAtlas(io.Fonts, param.m_cpuMemory))
			{
				ImGui_ImplSampleUtil_InvalidateDeviceObjects();
				ImGui_ImplSampleUtil_CreateDeviceObjects();
			}
			ImGui_ImplSampleUtil_NewFrame(param.m_targetWidth, param.m_targetHeight, nullptr, m_pPad, nullptr);
			ImGui::NewFrame();
			ImGui::PushFont(ImGui_ImplSampleUtil_defaultFont);
		}

		generateImguiCommands(param.m_targetWidth, param.m_targetHeight);

		if (param.m_hasOwnImgui)
		{
#if _SCE_TARGET_OS_PROSPERO
			SCE_SAMPLE_UTIL_ASSERT(param.m_pDcb != nullptr);
			if (param.m_pRenderTarget != nullptr)
			{
				sce::Agc::Core::StateBuffer sb;
				sb.init(512, param.m_pDcb, param.m_pDcb);
				sce::Agc::Toolkit::clearRenderTargetPs(param.m_pDcb, &sb, param.m_pRenderTarget, Agc::Core::Encoder::encode({ 0.f, 0.f, 0.f, 0.f }));
				sb.postDraw();
			}
#endif
#if _SCE_TARGET_OS_ORBIS
			SCE_SAMPLE_UTIL_ASSERT(param.m_pGfxc != nullptr);
			if (param.m_pRenderTarget != nullptr)
			{
				Graphics::SurfaceUtil::clearRenderTarget(*param.m_pGfxc, *param.m_pRenderTarget, Vectormath::Simd::Aos::Vector4(0));
			}
#endif
			ImGui::PopFont();
			ImGui::Render();
#if _SCE_TARGET_OS_PROSPERO
			ImGui_ImplSampleUtil_RenderDrawData(param.m_pDcb, ImGui::GetDrawData());
#endif
#if _SCE_TARGET_OS_ORBIS
			ImGui_ImplSampleUtil_RenderDrawData(param.m_pGfxc, ImGui::GetDrawData());
#endif
		}
	}
} // namespace ApiCapture
} } } // namespace sce::SampleUtil::Debug

