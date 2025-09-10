/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#include <wchar.h>
#include <mutex>
#include <condition_variable>
#include <stdio.h>
#include <sampleutil.h>
#include <vectormath.h>

using namespace sce::SampleUtil::Debug;

namespace ssd  = sce::SampleUtil::Debug;

ssd::Console::Console(float fontHeight, uint32_t lines, uint32_t nCharsPerLine, float lineSpacing, float position[2], const char *name, float fgColor[3], float bgColor[3])
{
	ImVec2 vPosition(position[0], position[1]);
	if (vPosition.x < 0.f || vPosition.y < 0.f)
	{
		vPosition.x = (fontHeight + lineSpacing) * 4.f;
		vPosition.y = (fontHeight + lineSpacing) * 4.f;
	}
	ImVec4 vFgColor(fgColor[0], fgColor[1], fgColor[2], 1.f);
	if (vFgColor.x < 0.f || vFgColor.y < 0.f || vFgColor.z < 0.f)
	{
		vFgColor.w = -1.f;
	}
	ImVec4 vBgColor(bgColor[0], bgColor[1], bgColor[2], 1.f);
	if (vBgColor.x < 0.f || vBgColor.y < 0.f || vBgColor.z < 0.f)
	{
		vBgColor.w = -1.f;
	}

	strncpy(this->m_name, name, 256);
	this->m_name[255] = '\0';
	this->m_position = vPosition;
	this->m_nCharsPerLine = nCharsPerLine;
	this->m_fontHeight = fontHeight;
	this->m_lines      = lines;
	this->m_lineHeight = lineSpacing + fontHeight;

	this->m_fgColor = vFgColor;
	this->m_bgColor = vBgColor;

	this->m_stringBufferSize = 128 * 1024;
	m_stringBuffer = (char*)malloc(sizeof(char) * this->m_stringBufferSize);

	ImGui_ImplSampleUtil_keepOskFocus = true;
}

ssd::Console::~Console()
{
	free(this->m_stringBuffer);
}

int ssd::Console::update()
{
	ImGui::SetNextWindowPos(m_position);
	float windowMargin = m_lineHeight * 1.f;
	ImGui::SetNextWindowSize(ImVec2(m_lineHeight * m_nCharsPerLine * 0.5f + windowMargin * 0.25f, m_lineHeight * m_lines + windowMargin));
	if (m_fgColor.w > 0.f)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, m_fgColor);
	}
	if (m_bgColor.w > 0.f)
	{
		ImGui::PushStyleColor(ImGuiCol_WindowBg, m_bgColor);
	}
	ImGui::Begin(m_name, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);
	ImGui::SetKeyboardFocusHere();
	{
		std::unique_lock<std::mutex> lock(m_scanf_m);
		m_enterPressed = (ImGui::GetIO().InputQueueCharacters.size() > 0 && ImGui::GetIO().InputQueueCharacters.back() == '\r');
		ImGui::InputTextMultiline("##source", m_stringBuffer, m_stringBufferSize, ImVec2(m_lineHeight * m_nCharsPerLine * 0.5f, m_lineHeight * m_lines), ImGuiInputTextFlags_None);
	}
	if (m_scanbuffer != nullptr && m_enterPressed)
	{
		m_scanf_cv.notify_one();
	}
	if (m_isWritingString && ImGui_ImplSampleUtil_writeString[0] == '\0')
	{
		{
			std::unique_lock<std::mutex> lock(ImGui_ImplSampleUtil_writeString_lock);
			m_isWritingString = false;
		}
		m_writeString_cv.notify_all();
	}

	ImGui::End();
	if (m_fgColor.w > 0.f)
	{
		ImGui::PopStyleColor();
	}
	if (m_bgColor.w > 0.f)
	{
		ImGui::PopStyleColor();
	}

	return SCE_OK;
}

void  ssd::Console::writeToConsole(char *str)
{
	std::unique_lock<std::mutex> lock(ImGui_ImplSampleUtil_writeString_lock);

	while (m_isWritingString)
	{
		m_writeString_cv.wait(lock, [this] {return !this->m_isWritingString; });
	}

	strncat(ImGui_ImplSampleUtil_writeString, str, 1024);
	m_isWritingString = true;

	m_writeString_cv.wait(lock, [this] {return !this->m_isWritingString; });

	lock.unlock();
}


int ssd::Console::vprintf(const char *format, va_list ap)
{
	char buffer[1024];
	int n = vsnprintf(buffer, 1024, format, ap);
	writeToConsole(buffer);
		
	return n;
}

int  ssd::Console::printf(const char *format, ...)
{
	if(nullptr == format){
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}

	va_list ap;
	va_start(ap, format);
	int ret = vprintf(format ,ap);
	va_end(ap);
	return ret;
}

int ssd::Console::vscanf(const char *format, va_list ap)
{
	std::unique_lock<std::mutex> lock(m_scanf_m);

	m_scanbuffer = &m_stringBuffer[strlen(m_stringBuffer)];
	m_scanf_cv.wait(lock, [this] {return m_enterPressed; });

	int n = vsscanf(m_scanbuffer, format, ap);
	m_scanbuffer = nullptr;

	lock.unlock();

	return n;
}

int  ssd::Console::scanf(const char *format, ...)
{
	if(nullptr == format){
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}

	va_list ap;
	va_start(ap, format);
	int ret = vscanf(format ,ap);
	va_end(ap);
	return ret;
}

