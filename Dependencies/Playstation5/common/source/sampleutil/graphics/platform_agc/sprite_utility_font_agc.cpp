/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2024 Sony Interactive Entertainment Inc.
* 
*/

#include <scebase_common/scebase_target.h>

#if _SCE_TARGET_OS_PROSPERO

#include <sampleutil/graphics/sprite_utility.h>
#include "sampleutil/ui_framework/imgui/imgui_libfont.h"
#include "sampleutil/sampleutil_error.h"
#include "../../../imgui/imgui_internal.h"
#include <ces.h>

namespace
{
	uint32_t	s_renderTargetWidth		= 3840;
	uint32_t	s_renderTargetHeight	= 2160;

	// Put colored text on ImGui current window
	static void putTextColored(ImVec2 pos, float depth, ImVec4 color, const char *text)
	{
		const char *text_end = text + strlen(text); // FIXME-OPT
		if (text != text_end)
		{
			ImGuiContext& g = *GImGui;
			ImGuiWindow* window = g.CurrentWindow;

			window->DrawList->SetDepth(depth);
			window->DrawList->AddText(g.Font, g.FontSize, pos, ImGui::ColorConvertFloat4ToU32(color), text, text_end);
			window->DrawList->ResetDepth();
		}
	}

	// Text only
	static void drawText(ImVec2 pos, float depth, ImVec4 color, const char *format, ...)
	{
		char buffer[256];
		va_list ap;
		va_start(ap, format);
		vsnprintf(buffer, sizeof(buffer), format, ap);
		va_end(ap);

		putTextColored(pos, depth, color, buffer);
	}

	// Text with colored rectangle BG
	static void drawText(ImVec2 pos, float depth, ImVec4 fontColor, ImVec4 bgColor, const char *format, ...)
	{
		char buffer[256];
		va_list ap;
		va_start(ap, format);
		vsnprintf(buffer, sizeof(buffer), format, ap);
		va_end(ap);

		// Rectangle
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		const char *text_end = buffer + strlen(buffer); // FIXME-OPT
		const ImVec2 text_size = ImGui::CalcTextSize(buffer, text_end, false);
		window->DrawList->SetDepth(depth);
		window->DrawList->AddRectFilled(pos, ImVec2(pos.x + text_size.x, pos.y + text_size.y), ImGui::ColorConvertFloat4ToU32(bgColor));
		window->DrawList->ResetDepth();

		// Text
		putTextColored(pos, depth, fontColor, buffer);
	}

	struct DrawStringData
	{
		ImFont									*m_pFont;
		bool									m_isScreenCoordinatePos;
		sce::Vectormath::Simd::Aos::Vector2		m_position;
		sce::Vectormath::Simd::Aos::Vector4		m_colorCoeff;
		bool									m_hasBgColor;
		sce::Vectormath::Simd::Aos::Vector4		m_backgroundColor;
		float									m_depth;
		float									m_scale;
		float									m_fontHeight;
		char									*m_pString;

		void	createDrawImguiCommands()
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImVec2(s_renderTargetWidth, s_renderTargetHeight), ImGuiCond_Once);
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4());
			ImGui::PushFont(m_pFont);

			ImGui::Begin(WINDOW_NAME_DEBUG_STRING, nullptr, IMGUI_FLAG_FOR_DEBUG_STRING);
			{
				// Adjust font scale
				if (m_scale > 0.f)
				{
					ImGui::SetWindowFontScale(m_scale);
				} else {
					ImGuiContext *g = ImGui::GetCurrentContext();
					ImGui::SetWindowFontScale((m_fontHeight * s_renderTargetHeight) / g->FontBaseSize);
				}

				if (m_hasBgColor)
				{
					drawText(
						m_isScreenCoordinatePos ? ImVec2(m_position.getX(), m_position.getY()) : ImVec2(s_renderTargetWidth * m_position.getX(), s_renderTargetHeight * m_position.getY()),
						m_depth,
						ImVec4(m_colorCoeff.getX(), m_colorCoeff.getY(), m_colorCoeff.getZ(), m_colorCoeff.getW()),
						ImVec4(m_backgroundColor.getX(), m_backgroundColor.getY(), m_backgroundColor.getZ(), m_backgroundColor.getW()),
						m_pString);
				} else {
					drawText(
						m_isScreenCoordinatePos ? ImVec2(m_position.getX(), m_position.getY()) : ImVec2(s_renderTargetWidth * m_position.getX(), s_renderTargetHeight * m_position.getY()),
						m_depth,
						ImVec4(m_colorCoeff.getX(), m_colorCoeff.getY(), m_colorCoeff.getZ(), m_colorCoeff.getW()),
						m_pString);
				}
			}
			ImGui::End();

			ImGui::PopFont();
			ImGui::PopStyleColor(1);
		}
	};

	static std::vector<DrawStringData>	s_deferredDrawStringQueue;
}

namespace sce {	namespace SampleUtil { namespace Graphics {
	namespace SpriteUtil {

		int	setRenderTargetSize(uint32_t	width, uint32_t	height)
		{
			SCE_SAMPLE_UTIL_ASSERT(width > 0 && width <= 7680 && height > 0 && height <= 4320);
			if (!(width > 0 && width <= 7680 && height > 0 && height <= 4320))
			{
				return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			s_renderTargetWidth		= width;
			s_renderTargetHeight	= height;

			return	SCE_OK;
		}

		int	drawString(
			ImFont	*font,
			const uint16_t	*ucs2Charcode,
			sce::Vectormath::Simd::Aos::Vector2_arg	position,			// screen coordinate
			sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
			float	depth/* = 0.f*/,
			float	scale/* = 1.f*/
		)
		{
			uint8_t buffer[256];
			memset(buffer, '\0', sizeof(buffer));

			uint8_t *pbuf = buffer;
			const uint16_t *pucs = ucs2Charcode;
			uint32_t utf8len;
			while(*pucs != 0)
			{
				if (0 > sceCesUcs2ToUtf8(*pucs, pbuf++, sizeof(buffer), &utf8len))
				{
					break;
				}
				pucs++;
			}

			DrawStringData data;
			data.m_pFont					= font;
			data.m_position					= position;
			data.m_scale					= scale;
			data.m_depth					= depth;
			data.m_isScreenCoordinatePos	= true;
			data.m_colorCoeff				= colorCoeff;
			data.m_hasBgColor				= false;
			data.m_pString					= new char[strlen((char *)buffer) + 1];
			strncpy(data.m_pString, (char *)buffer, strlen((char *)buffer) + 1);

			s_deferredDrawStringQueue.push_back(data);

			return SCE_OK;
		}

		sce::Vectormath::Simd::Aos::Vector2	getStringTextureSize(
			ImFont	*font,
			const uint16_t	*ucs2Charcode,
			float	scale/* = 1.f*/
		)
		{
			char buffer[256];
			memset(buffer, '\0', sizeof(buffer));

			uint8_t *pbuf = (uint8_t*)buffer;
			const uint16_t *pucs = ucs2Charcode;
			uint32_t utf8len;
			while (*pucs != 0)
			{
				if (0 > sceCesUcs2ToUtf8(*pucs, pbuf++, sizeof(buffer), &utf8len))
				{
					break;
				}
				pucs++;
			}

			// String is empty
			if (0 == (pucs - ucs2Charcode))
			{
				return sce::Vectormath::Simd::Aos::Vector2(0.0f, font->FontSize);
			}

			// Create font atlas which has only characters in "ucs2Charcode"
			bool buildAtlas = false;
			if (font->FontSize == 0.0f)
			{
				ImGuiLibFont::BuildFontAtlasLimitedChar(ImGui::GetIO().Fonts, font, ucs2Charcode);
				buildAtlas = true;
			}

			const char *text_end = buffer + strlen(buffer); // FIXME-OPT
			sce::Vectormath::Simd::Aos::Vector2 retVal;
			// Refer from ImGui::CalcTextSize
			{
				const char* text_display_end = text_end;
				const float font_size = font->FontSize;
				if (buffer == text_display_end)
				{
					retVal = sce::Vectormath::Simd::Aos::Vector2(0.0f, font_size);
				}
				else
				{
					ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, buffer, text_display_end, nullptr);

					// Cancel out character spacing for the last character of a line (it is baked into glyph->AdvanceX field)
					const float font_scale = font_size / font->FontSize;
					const float character_spacing_x = 1.0f * font_scale;
					if (text_size.x > 0.0f)
					{
						text_size.x -= character_spacing_x;
					}
					text_size.x = (float)(int)(text_size.x + 0.95f);

					retVal = sce::Vectormath::Simd::Aos::Vector2(text_size.x, text_size.y);
				}
			}

			// Destroy font altas created here
			if (buildAtlas)
			{
				ImGuiLibFont::DestroyFontAtlasLimitedChar(ImGui::GetIO().Fonts, font);
			}

			return retVal;
		}

		int	drawDebugString(
			sce::Vectormath::Simd::Aos::Vector2_arg	position,		// texcoord coordinate
			float	fontHeight,
			sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
			sce::Vectormath::Simd::Aos::Vector4_arg	backgroundColor,
			float	depth,
			const char	*string)
		{
			ImGuiContext *g = ImGui::GetCurrentContext();
			DrawStringData data;
			data.m_pFont					= g->Font;
			data.m_position					= position;
			data.m_scale					= 0.f;
			data.m_depth					= depth;
			data.m_fontHeight				= fontHeight;
			data.m_isScreenCoordinatePos	= false;
			data.m_colorCoeff				= colorCoeff;
			data.m_backgroundColor			= backgroundColor;
			data.m_hasBgColor				= true;
			data.m_pString					= new char[strlen(string) + 1];
			strncpy(data.m_pString, string, strlen(string) + 1);

			s_deferredDrawStringQueue.push_back(data);

			return SCE_OK;
		}

		int	drawDebugString(
			sce::Vectormath::Simd::Aos::Vector2_arg	position,			// texcoord coordinate
			float	fontHeight,
			sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
			const char	*string)
		{
			ImGuiContext *g = ImGui::GetCurrentContext();
			DrawStringData data;
			data.m_pFont					= g->Font;
			data.m_position					= position;
			data.m_scale					= 0.f;
			data.m_depth					= 0.f;
			data.m_fontHeight				= fontHeight;
			data.m_isScreenCoordinatePos	= false;
			data.m_colorCoeff				= colorCoeff;
			data.m_hasBgColor				= false;
			data.m_pString					= new char[strlen(string) + 1];
			strncpy(data.m_pString, string, strlen(string) + 1);

			s_deferredDrawStringQueue.push_back(data);

			return SCE_OK;
		}

		int	drawDebugStringf(
			sce::Vectormath::Simd::Aos::Vector2_arg	position,
			float	fontHeight,
			sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
			const char	*format, ...)
		{
			char buffer[256];
			va_list ap;
			va_start(ap, format);
			vsnprintf(buffer, sizeof(buffer), format, ap);
			va_end(ap);

			return drawDebugString(position, fontHeight, colorCoeff, buffer);
		}
		int	drawDebugStringf(
			sce::Vectormath::Simd::Aos::Vector2_arg	position,
			float	fontHeight,
			sce::Vectormath::Simd::Aos::Vector4_arg	colorCoeff,
			sce::Vectormath::Simd::Aos::Vector4_arg	backgroundColor,
			float	depth,
			const char	*format, ...)
		{
			char buffer[256];
			va_list ap;
			va_start(ap, format);
			vsnprintf(buffer, sizeof(buffer), format, ap);
			va_end(ap);

			return drawDebugString(position, fontHeight, colorCoeff, backgroundColor, depth, buffer);
		}

		float	getWidthOfDebugChar(float charHeight)
		{
			return (charHeight / 2.f) * (float)s_renderTargetWidth / (float)s_renderTargetHeight;
		}

		void	deferredGenerateDrawStringImguiCommands()
		{
			for (auto data : s_deferredDrawStringQueue)
			{
				data.createDrawImguiCommands();
				delete [] data.m_pString;
			}
			s_deferredDrawStringQueue.clear();
		}
	}
}}}

#endif
