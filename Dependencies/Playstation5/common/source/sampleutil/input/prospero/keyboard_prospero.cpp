/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2022 Sony Interactive Entertainment Inc. 
 * 
 */

#include <unordered_map>
#include <libsysmodule.h>
#include <convert_keycode.h>
#include <libime.h>
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO

#include <sampleutil/input.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/input/keyboard.h>

#pragma comment(lib,"libSceConvertKeycode_stub_weak.a")
#pragma comment(lib,"libSceIme_stub_weak.a")

namespace
{
	bool s_keyboardIsOpened = false;
	std::unordered_map<SceUserServiceUserId, sce::SampleUtil::Input::KeyboardContext*>	s_keyboardContexts;
}

void sce::SampleUtil::Input::KeyboardContext::imeHandler(void *arg, const SceImeEvent *e)
{
	int ret = SCE_OK;

	(void)arg;

	switch (e->id)
	{
	case SCE_IME_KEYBOARD_EVENT_KEYCODE_DOWN:
	case SCE_IME_KEYBOARD_EVENT_KEYCODE_UP:
	case SCE_IME_KEYBOARD_EVENT_KEYCODE_REPEAT:
	{
		if (s_keyboardContexts.find(e->param.keycode.userId) == s_keyboardContexts.end()) {
			return; // discard result if relevant keyboard context is not created.
		}
		KeyboardContext *keyboard = s_keyboardContexts[e->param.keycode.userId];
		uint16_t vkeycode;

		SceImeKeyboardType keyboardType;
		ret = sceConvertKeycodeGetImeKeyboardType(e->param.keycode.userId, &keyboardType);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceConvertKeycodeGetVirtualKeycode(e->param.keycode.keycode, keyboardType, &vkeycode);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (e->id == SCE_IME_KEYBOARD_EVENT_KEYCODE_DOWN)
		{
			keyboard->m_keyStateArray[vkeycode] = KeyboardContext::kKeyStatePressed | KeyboardContext::kKeyStateDown;
		}
		else if (e->id == SCE_IME_KEYBOARD_EVENT_KEYCODE_UP)
		{
			keyboard->m_keyStateArray[vkeycode] = KeyboardContext::kKeyStateReleased | KeyboardContext::kKeyStateUp;
		}
		else
		{
			keyboard->m_keyStateArray[vkeycode] = KeyboardContext::kKeyStateRepeated | KeyboardContext::kKeyStateDown;
		}
		if ((e->param.keycode.status & SCE_IME_KEYCODE_STATE_CHARACTER_VALID) && (e->id == SCE_IME_KEYBOARD_EVENT_KEYCODE_DOWN || e->id == SCE_IME_KEYBOARD_EVENT_KEYCODE_REPEAT))
		{
			int len = wcslen(((KeyboardContext*)keyboard)->m_charBuffer);
			((KeyboardContext*)keyboard)->m_charBuffer[len] = e->param.keycode.character;
			((KeyboardContext*)keyboard)->m_charBuffer[len + 1] = 0;
		}
	}
	break;
	default:
		break;
	}
}

int sce::SampleUtil::Input::KeyboardContext::initialize(System::UserId userId)
{
	int ret = SCE_OK;

	m_userId = userId;
	for (int i = 0; i < 0x100; i++)
	{
		m_keyStateArray[i] = kKeyStateUp;
	}

	ret = sceSysmoduleLoadModule( SCE_SYSMODULE_CONVERT_KEYCODE );
	if (ret != SCE_OK)
	{
		return ret;
	}
	ret = sceSysmoduleLoadModule( SCE_SYSMODULE_LIBIME );
	if (ret != SCE_OK)
	{
		return ret;
	}

	SceImeKeyboardParam keyParam;
	memset(&keyParam, 0, sizeof(SceImeKeyboardParam));

	keyParam.option = SCE_IME_KEYBOARD_OPTION_DEFAULT;
	keyParam.arg = nullptr;
	keyParam.handler = imeHandler;

	if (!s_keyboardIsOpened)
	{
		ret = sceImeKeyboardOpen(SCE_USER_SERVICE_USER_ID_EVERYONE, &keyParam);
		if (ret != SCE_OK)
		{
			if (ret == SCE_IME_ERROR_BUSY) return SCE_SAMPLE_UTIL_ERROR_BUSY;
			else if (ret == SCE_IME_ERROR_NO_MEMORY) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			else if (ret == SCE_IME_ERROR_CONNECTION_FAILED) return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
			else if (ret == SCE_IME_ERROR_NOT_ACTIVE) return SCE_SAMPLE_UTIL_ERROR_NOT_ACTIVE;
			else if (ret == SCE_IME_ERROR_INVALID_USER_ID) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			else if (ret == SCE_IME_ERROR_INTERNAL) return SCE_SAMPLE_UTIL_ERROR_FATAL;
			SCE_SAMPLE_UTIL_ASSERT(false);
		}
		s_keyboardIsOpened = true;
	}

	s_keyboardContexts.insert(std::make_pair(m_userId, this));

	return SCE_OK;
}

int sce::SampleUtil::Input::KeyboardContext::finalize()
{
	int ret = SCE_OK;
	SCE_SAMPLE_UTIL_ASSERT(s_keyboardContexts.find(m_userId) != s_keyboardContexts.end());
	s_keyboardContexts.erase(m_userId);
	if (s_keyboardContexts.empty())
	{
		ret = sceImeKeyboardClose(SCE_USER_SERVICE_USER_ID_EVERYONE);
		s_keyboardIsOpened = false;
	}
	return ret;
}

int sce::SampleUtil::Input::KeyboardContext::update()
{
	int ret = 0;
	
	for (int i = 0; i < 0x100; i++)
	{
		m_keyStateArray[i] &= ~((uint32_t)kKeyStatePressed | (uint32_t)kKeyStateReleased);
	}
	m_charBuffer[0] = 0;

	ret = sceImeUpdate(imeHandler);
	if (ret != SCE_OK)
	{
		if (ret == SCE_IME_ERROR_NOT_OPENED) return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		else if (ret == SCE_IME_ERROR_CONNECTION_FAILED) return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		else if (ret == SCE_IME_ERROR_EVENT_OVERFLOW) return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		else if (ret == SCE_IME_ERROR_INTERNAL) return SCE_SAMPLE_UTIL_ERROR_FATAL;
		SCE_SAMPLE_UTIL_ASSERT(false);
	}

	return SCE_OK;
}

bool sce::SampleUtil::Input::KeyboardContext::isConnected() const
{
	SceImeKeyboardInfo info;

	int ret = SCE_OK;
	SceImeKeyboardResourceIdArray	resourceIds;
	ret = sceImeKeyboardGetResourceId(m_userId, &resourceIds);
	if (ret == SCE_IME_ERROR_NOT_OPENED ||
		ret == SCE_IME_ERROR_CONNECTION_FAILED ||
		ret == SCE_IME_ERROR_IME_SUSPENDING ||
		ret == SCE_IME_ERROR_NO_RESOURCE_ID) return false;
	for (int i = 0; i < SCE_IME_KEYBOARD_MAX_NUMBER; i++) {
		if (resourceIds.resourceId[i] == SCE_IME_KEYBOARD_RESOURCE_ID_INVALID) {
			break;
		}
		ret = sceImeKeyboardGetInfo(resourceIds.resourceId[i], &info);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		if (info.status == SCE_IME_KEYBOARD_STATE_CONNECTED) {
			return true;
		}
	}

	return false;
}

#endif //_SCE_TARGET_OS_PROSPERO
