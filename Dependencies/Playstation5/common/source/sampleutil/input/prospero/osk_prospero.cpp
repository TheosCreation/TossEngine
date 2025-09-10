/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2021 Sony Interactive Entertainment Inc. 
 * 
 */


#include <libsysmodule.h>
#include <ime_dialog.h>
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO

#include <sampleutil/input.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/input/osk.h>

#pragma comment(lib, "libSceImeDialog_stub_weak.a" )

namespace
{
	SceImeType getSceImeType(uint32_t type)
	{
		switch (type)
		{
		case sce::SampleUtil::Input::ImeType::kDecimal:
			return SCE_IME_TYPE_NUMBER;			//<E Number input UI
		case sce::SampleUtil::Input::ImeType::kAlphanumeric:
			return SCE_IME_TYPE_BASIC_LATIN;	//<E Alphanumeric input UI
		default:
			break;
		}
		return SCE_IME_TYPE_DEFAULT;			//<E Normal text input UI
	}

	uint32_t getSceImeOption(uint32_t type)
	{
		switch (type)
		{
		case sce::SampleUtil::Input::ImeType::kDecimal:
		case sce::SampleUtil::Input::ImeType::kAlphanumeric:
			return SCE_IME_OPTION_DEFAULT;
		case sce::SampleUtil::Input::ImeType::kJapanese:
			return SCE_IME_OPTION_LANGUAGES_FORCED;
		default:
			break;
		}
		return SCE_IME_OPTION_MULTILINE;
	}

	uint64_t getSceSupportedLanguage(uint32_t type)
	{
		switch (type)
		{
		case sce::SampleUtil::Input::ImeType::kJapanese:
			return SCE_IME_LANGUAGE_JAPANESE;
		default:
			break;
		}
		return 0;
	}
}

sce::SampleUtil::Input::OskContext::OskContext(sce::SampleUtil::System::UserId userId)
{
	int ret = initialize(userId); (void)ret;
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
}

sce::SampleUtil::Input::OskContext::~OskContext()
{
	finalize();
}

int sce::SampleUtil::Input::OskContext::initialize(System::UserId userId)
{
	int ret = SCE_OK;

	m_userId = userId;

	ret = sceSysmoduleLoadModule( SCE_SYSMODULE_IME_DIALOG );
	if (ret != SCE_OK)
	{
		return ret;
	}


	return SCE_OK;
}

int sce::SampleUtil::Input::OskContext::finalize()
{
	int ret = 0;
	ret = sceSysmoduleUnloadModule( SCE_SYSMODULE_IME_DIALOG );
	if (ret != SCE_OK)
	{
		return ret;
	}
	return ret;
}

int sce::SampleUtil::Input::OskContext::start(const OskParam *param)
{
	int ret = 0;

	OskParam oskParam;
	if (param != nullptr)
	{
		oskParam = *param;
	}

	m_resultBuffer = new wchar_t[oskParam.m_maxTextLength + 1];
	m_resultBuffer[0] = 0;
	m_resultBufferSize = oskParam.m_maxTextLength + 1;

	SceImeDialogParam	dialogParam;

	//E initalize parameter of ime dialog
	//J IMEダイアログパラメータ初期化
	sceImeDialogParamInit(&dialogParam);

	dialogParam.userId = m_userId;							//E Set user Id
	dialogParam.type = getSceImeType(oskParam.m_type);		//E Set input UI types
	dialogParam.option = getSceImeOption(oskParam.m_type);	//E Set options settings
	dialogParam.supportedLanguages = getSceSupportedLanguage(oskParam.m_type);	//E Set supported languages by the IME, or 0
	dialogParam.enterLabel = SCE_IME_ENTER_LABEL_DEFAULT;	//E Set label of Enter key
	dialogParam.inputMethod = SCE_IME_INPUT_METHOD_DEFAULT; //E Set input method
	dialogParam.title = oskParam.m_title;					//E Set title character string of the IME dialog, or NULL
	dialogParam.placeholder = oskParam.m_placeholder;		//E Set guide string, or NULL
	dialogParam.inputTextBuffer = m_resultBuffer;	//E Set buffer to store input string
	dialogParam.maxTextLength = oskParam.m_maxTextLength;	//E Set maximum length of string that can be input

	//E Get ime dialog size.
	//J ダイアログサイズの取得
	sceImeDialogGetPanelSizeExtended(&dialogParam, nullptr, &m_dialogWidth, &m_dialogHeight);

	dialogParam.posx = 1920.0f / 2.f - m_dialogWidth / 2.f;	//E Set X coordinate of the display position
	dialogParam.posy = 1080.0f / 2.f - m_dialogHeight / 2.f;	//E Set Y coordinate of the display position
	dialogParam.horizontalAlignment = SCE_IME_HALIGN_LEFT;	//E Set IME dialog horizontal display origins
	dialogParam.verticalAlignment = SCE_IME_VALIGN_TOP;		//E Set IME dialog vertical display origins
	dialogParam.filter = nullptr;								//E Set character string filter callback function or NULL

	ret = sceImeDialogInit(&dialogParam, nullptr);
	if (ret != SCE_OK)
	{
		if (ret == SCE_IME_ERROR_NOT_ACTIVE) return SCE_SAMPLE_UTIL_ERROR_NOT_ACTIVE;
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	}

	return ret;
}

int sce::SampleUtil::Input::OskContext::start(const wchar_t *dialogTitle, const wchar_t *placeholderText, uint32_t maxTextLength)
{
	int ret = 0;

	m_resultBuffer = new wchar_t[maxTextLength];
	m_resultBuffer[0] = 0;
	m_resultBufferSize = maxTextLength;

	SceImeDialogParam	dialogParam;

	//E initalize parameter of ime dialog
	//J IMEダイアログパラメータ初期化
	sceImeDialogParamInit(&dialogParam);

	dialogParam.userId = m_userId;							//E Set user Id
	dialogParam.type = SCE_IME_TYPE_DEFAULT;				//E Set input UI types
	dialogParam.option = SCE_IME_OPTION_MULTILINE;			//E Set options settings
	dialogParam.supportedLanguages = 0;						//E Set supported languages by the IME, or 0
	dialogParam.enterLabel = SCE_IME_ENTER_LABEL_DEFAULT;	//E Set label of Enter key
	dialogParam.inputMethod = SCE_IME_INPUT_METHOD_DEFAULT; //E Set input method
	dialogParam.title = dialogTitle;						//E Set title character string of the IME dialog, or NULL
	dialogParam.placeholder = placeholderText;				//E Set guide string, or NULL
	dialogParam.inputTextBuffer = m_resultBuffer;	//E Set buffer to store input string
	dialogParam.maxTextLength = maxTextLength;				//E Set maximum length of string that can be input

	//E Get ime dialog size.
	//J ダイアログサイズの取得
	sceImeDialogGetPanelSizeExtended(&dialogParam, nullptr, &m_dialogWidth, &m_dialogHeight);

	dialogParam.posx = 1920.0f / 2.f - m_dialogWidth / 2.f;	//E Set X coordinate of the display position
	dialogParam.posy = 1080.0f / 2.f - m_dialogHeight / 2.f;	//E Set Y coordinate of the display position
	dialogParam.horizontalAlignment = SCE_IME_HALIGN_LEFT;	//E Set IME dialog horizontal display origins
	dialogParam.verticalAlignment = SCE_IME_VALIGN_TOP;		//E Set IME dialog vertical display origins
	dialogParam.filter = nullptr;								//E Set character string filter callback function or NULL

	ret = sceImeDialogInit(&dialogParam, nullptr);
	if (ret != SCE_OK)
	{
		if (ret == SCE_IME_ERROR_NOT_ACTIVE) return SCE_SAMPLE_UTIL_ERROR_NOT_ACTIVE;
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	}

	return ret;
}

bool sce::SampleUtil::Input::OskContext::isFinished()
{
	bool isFinished = false;

	SceImeDialogStatus	dialogStatus = sceImeDialogGetStatus();
	switch(dialogStatus)
	{
	case SCE_IME_DIALOG_STATUS_FINISHED:
	case SCE_IME_DIALOG_STATUS_NONE:
		isFinished = true;
		break;
	case SCE_IME_DIALOG_STATUS_RUNNING:
		isFinished = false;
		break;
	default:
		SCE_SAMPLE_UTIL_ASSERT(false);
		break;
	}

	return isFinished;
}

int sce::SampleUtil::Input::OskContext::getResult(wchar_t *result)
{
	int ret = 0;

	//E get ime dialog result
	//J IMEダイアログの終了状態を取得する
	SceImeDialogResult	dialogResult;
	memset(&dialogResult, 0, sizeof(SceImeDialogResult));
	ret = sceImeDialogGetResult(&dialogResult);
	if (ret == SCE_IME_DIALOG_ERROR_NOT_FINISHED) return SCE_SAMPLE_UTIL_ERROR_BUSY;
	else if (ret == SCE_IME_DIALOG_ERROR_NOT_IN_USE) return SCE_SAMPLE_UTIL_ERROR_NOT_ACTIVE;
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	//E terminate ime dialog
	//J IMEダイアログ終了処理
	ret = sceImeDialogTerm();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	if (dialogResult.endstatus == SCE_IME_DIALOG_END_STATUS_OK)
	{
		wcsncpy(result, m_resultBuffer, m_resultBufferSize);
	}
	delete [] m_resultBuffer;

	return SCE_OK;
}

#endif //_SCE_TARGET_OS_PROSPERO
