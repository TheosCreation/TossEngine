/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#pragma once

#include <scebase_common.h>
#include <sampleutil/sampleutil_common.h>
#include <sampleutil/system.h>

namespace sce {	namespace SampleUtil { namespace Input {
	/*!
	 * @~English
	 * @brief The type of IME dialog
	 * @details This specifies the type of IME dialog.
	 *
	 * @~Japanese
	 * @brief IMEダイアログのタイプ
	 * @details IMEダイアログのタイプです
	 */
	enum ImeType
	{
		/*!
		 * @~English
		 * @brief Default type
		 *
		 * @~Japanese
		 * @brief デフォルト
		 */
		kDefault,
		/*!
		 * @~English
		 * @brief Type for number input
		 *
		 * @~Japanese
		 * @brief 数値入力タイプ
		 */
		kDecimal,
		/*!
		 * @~English
		 * @brief Type for alphanumeric character input
		 *
		 * @~Japanese
		 * @brief 英数値入力タイプ
		 */
		kAlphanumeric,
		/*!
		 * @~English
		 * @brief Type for Japanese input
		 *
		 * @~Japanese
		 * @brief 日本語入力タイプ
		 */
		kJapanese,
	};

	/*!
	 * @~English
	 * @brief Parameter of IME dialog
	 * @details This is a parameter used for displaying IME dialog.
	 *
	 * @~Japanese
	 * @brief IMEダイアログのパラメーター
	 * @details IMEダイアログを表示する際に使用するパラメーターです
	 */
	struct OskParam
	{
		/*!
		 * @~English
		 * @brief The name of IME dialog title
		 *
		 * @~Japanese
		 * @brief IMEダイアログのタイトル名
		 */
		const wchar_t	*m_title;
		/*!
		 * @~English
		 * @brief The place holder text of IME dialog
		 *
		 * @~Japanese
		 * @brief IMEダイアログのプレースホルダ文字列
		 */
		const wchar_t	*m_placeholder;
		/*!
		 * @~English
		 * @brief The type of IME dialog
		 *
		 * @~Japanese
		 * @brief IMEダイアログのタイプ
		 */
		uint32_t		m_type;
		/*!
		 * @~English
		 * @brief The name of IME dialog title
		 *
		 * @~Japanese
		 * @brief IMEダイアログのタイトル名
		 */
		uint32_t		m_maxTextLength;

		OskParam()
		{
			m_title			= L"ime dialog title";
			m_placeholder	= L"place holder";
			m_type			= ImeType::kDefault;
			m_maxTextLength = 256;
		}
	};

	/*!
	 * @~English
	 * 
	 * @brief Class for handling on-screen keyboard operation 
	 * @details OskContext is a class that enables applications to easily support on-screen keyboard operation. It is possible to get string inputted from on-screen keyboard.
	 * @~Japanese
	 * 
	 * @brief オンスクリーンキーボード操作を扱うためのクラス 
	 * @details OskContextはアプリケーションがオンスクリーンキーボード入力をサポートするのを簡便に行えるようにするためのクラスです。オンスクリーンキーボードから入力された文字列を取得します。
	 */
	class OskContext
	{
	public:
		OskContext() {}
		/*!
		* @~English
		* @brief Constructor
		* @param userId User ID
		* @details This is a constructor.
		* @~Japanese
		* @brief コンストラクタ
		* @param userId ユーザーID
		* @details コンストラクタです。
		*/
		OskContext(System::UserId userId);
		/*!
		 * @~English
		 * @brief Destructor 
		 * @details This is a destructor. 
		 * @~Japanese
		 * @brief デストラクタ 
		 * @details デストラクタです。 
		 */
		virtual ~OskContext();
		/*!
		 * @~English
		 * @brief Function to start on-screen keyboard context
		 * @param param IME dialog parameter(default parameter is used when nullptr is specified)
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @details This function opens IME dialog, and starts on-screen keyboard input
		 * @~Japanese
		 * @brief オンスクリーンキーボードコンテキストを開始する関数 
		 * @param param IMEダイアログパラメータ(nullptr指定時、既定パラメータを使用)
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 * @details この関数はIMEダイアログを開き、オンスクリーンキーボード入力を開始します。
		 */
		int start(const OskParam *param = nullptr);
		/*!
		 * @~English
		 * 
		 * @brief Function to start on-screen keyboard context
		 * 
		 * @param dialogTitle The name of IME dialog title
		 * @param placeholderText The place holder text of IME dialog
		 * @param maxTextLength Maximum character length
		 * 
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @details This function opens IME dialog, and starts on-screen keyboard input
		 * @~Japanese
		 * 
		 * @brief オンスクリーンキーボードコンテキストを開始する関数 
		 * 
		 * @param dialogTitle IMEダイアログのタイトル名
		 * @param placeholderText IMEダイアログのプレースホルダ文字列
		 * @param maxTextLength 最大文字数
		 * 
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 * @details この関数はIMEダイアログを開き、オンスクリーンキーボード入力を開始します。
		 */
		int start(const wchar_t *dialogTitle, const wchar_t *placeholderText, uint32_t maxTextLength);
		/*!
		 * @~English
		 * 
		 * @brief Function to check wether IME dialog input is finished
		 * 
		 * @retval true IME dialog input is finished
		 * @retval false IME dialog input is not finished
		 * @details This function checks if on-screen context started by calling OskContext::start() is finished. getResult() can be called after this function returns true.
		 * @~Japanese
		 * 
		 * @brief IMEダイアログが終了したことを確認する	関数 
		 * 
		 * @retval true IMEダイアログは終了している
		 * @retval false IMEダイアログは終了していない
		 * @details この関数はOskContext::start()の呼び出しで開始したオンスクリーンキーボードコンテキストが完了したことを確認します。getResult()はこの関数がtrueを返した後に呼び出すことができます。
		 */
		bool isFinished();
		/*!
		 * @~English
		 * 
		 * @brief Function to obtain on-screen keyboard input string data newly held by OskContext started with the last OskContext::start() call 
		 * 
		 * @param result Pointer to string buffer to which inputted string is stored.
		 * @param result Destination of obtained string
		 * 
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @details This function obtains on-screen keyboard input data newly held by OskContext started with the last OskContext::start() call. Obtained on-screen keyboard input data is stored in the data array in order from oldest to newest with the value specified. 
		 * @~Japanese
		 * 
		 * @brief 最後のKeyboardContext::start()の呼び出しで開始したKeyboardContextが新規に保持したオンスクリーンキーボード入力文字列を取得する関数 
		 * @param result 取得した文字列の格納先
		 * 
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 * @details 最後のOskContext::start()の呼び出しで開始したOskContextが新規に保持したオンスクリーンキーボード入力文字列を取得する関数です。取得したオンスクリーンキーボード入力を古いのものから順に配列に格納します。 
		 */
		int getResult(wchar_t *result);

		/*!
		 * @~English
		 * @brief Initializes IME dialog
		 * @param userId User ID used to initialize IME dialog
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief IMEダイアログを初期化
		 * @param userId IMEダイアログ初期化に使用するユーザID
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int		initialize(System::UserId	userId);
		/*!
		 * @~English
		 * @brief Finalizes IME dialog
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief IMEダイアログの終了処理
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int		finalize();

		System::UserId	m_userId;
		uint32_t		m_dialogWidth;
		uint32_t		m_dialogHeight;
		wchar_t			*m_resultBuffer;
		uint32_t		m_resultBufferSize;
	}; // class OskContext
} } } // namespace sce::SampleUtil::Input
