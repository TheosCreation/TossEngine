/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2022 Sony Interactive Entertainment Inc. 
 * 
 */

#pragma once

#include <scebase_common.h>
#include <sampleutil/sampleutil_common.h>
#include <sampleutil/system.h>
#include <convert_keycode/ime_virtual_keycode.h>
#include <libime.h>

namespace sce
{
	namespace SampleUtil
	{
		namespace Input
		{
			/*!
			 * @~English
			 * 
			 * @brief The enumeration type to be used for specifying key input
			 * @details Used for specifying for the argument of the KeyboardContext member functions isKeyDown(), isKeyUp(), isKeyPressed(), and isKeyRepeated(). 
			 * @~Japanese
			 * 
			 * @brief キー入力を指定するために利用する列挙型
			 * @details KeyboardContextのメンバ関数isKeyDown()、isKeyUp()、isKeyPressed()、isKeyRepeated()の引数に指定して利用します。
			 */
			enum Key
			{
				kINVALID = SCE_IME_VK_INVALID,
				kCANCEL = SCE_IME_VK_CANCEL,
				kBACK = SCE_IME_VK_BACK,
				kTAB = SCE_IME_VK_TAB,
				kCLEAR = SCE_IME_VK_CLEAR,
				kRETURN = SCE_IME_VK_RETURN,
				kSHIFT = SCE_IME_VK_SHIFT,
				kCONTROL = SCE_IME_VK_CONTROL,
				kMENU = SCE_IME_VK_MENU,
				kPAUSE = SCE_IME_VK_PAUSE,
				kCAPITAL = SCE_IME_VK_CAPITAL,
				kKANA = SCE_IME_VK_KANA,
				kHANGUL = SCE_IME_VK_HANGUL,
				kKANJI = SCE_IME_VK_KANJI,
				kESCAPE = SCE_IME_VK_ESCAPE,
				kCONVERT = SCE_IME_VK_CONVERT,
				kSPACE = SCE_IME_VK_SPACE,
				kPRIOR = SCE_IME_VK_PRIOR,
				kNEXT = SCE_IME_VK_NEXT,
				kEND = SCE_IME_VK_END,
				kHOME = SCE_IME_VK_HOME,
				kLEFT = SCE_IME_VK_LEFT,
				kUP = SCE_IME_VK_UP,
				kRIGHT = SCE_IME_VK_RIGHT,
				kDOWN = SCE_IME_VK_DOWN,
				kSELECT = SCE_IME_VK_SELECT,
				kPRINT = SCE_IME_VK_PRINT,
				kEXECUTE = SCE_IME_VK_EXECUTE,
				kSNAPSHOT = SCE_IME_VK_SNAPSHOT,
				kINSERT = SCE_IME_VK_INSERT,
				kDELETE = SCE_IME_VK_DELETE,
				kHELP = SCE_IME_VK_HELP,
				k0 = SCE_IME_VK_0,
				k1 = SCE_IME_VK_1,
				k2 = SCE_IME_VK_2,
				k3 = SCE_IME_VK_3,
				k4 = SCE_IME_VK_4,
				k5 = SCE_IME_VK_5,
				k6 = SCE_IME_VK_6,
				k7 = SCE_IME_VK_7,
				k8 = SCE_IME_VK_8,
				k9 = SCE_IME_VK_9,
				kA = SCE_IME_VK_A,
				kB = SCE_IME_VK_B,
				kC = SCE_IME_VK_C,
				kD = SCE_IME_VK_D,
				kE = SCE_IME_VK_E,
				kF = SCE_IME_VK_F,
				kG = SCE_IME_VK_G,
				kH = SCE_IME_VK_H,
				kI = SCE_IME_VK_I,
				kJ = SCE_IME_VK_J,
				kK = SCE_IME_VK_K,
				kL = SCE_IME_VK_L,
				kM = SCE_IME_VK_M,
				kN = SCE_IME_VK_N,
				kO = SCE_IME_VK_O,
				kP = SCE_IME_VK_P,
				kQ = SCE_IME_VK_Q,
				kR = SCE_IME_VK_R,
				kS = SCE_IME_VK_S,
				kT = SCE_IME_VK_T,
				kU = SCE_IME_VK_U,
				kV = SCE_IME_VK_V,
				kW = SCE_IME_VK_W,
				kX = SCE_IME_VK_X,
				kY = SCE_IME_VK_Y,
				kZ = SCE_IME_VK_Z,
				kLWIN = SCE_IME_VK_LWIN,
				kRWIN = SCE_IME_VK_RWIN,
				kAPPS = SCE_IME_VK_APPS,
				kSLEEP = SCE_IME_VK_SLEEP,
				kNUMPAD0 = SCE_IME_VK_NUMPAD0,
				kNUMPAD1 = SCE_IME_VK_NUMPAD1,
				kNUMPAD2 = SCE_IME_VK_NUMPAD2,
				kNUMPAD3 = SCE_IME_VK_NUMPAD3,
				kNUMPAD4 = SCE_IME_VK_NUMPAD4,
				kNUMPAD5 = SCE_IME_VK_NUMPAD5,
				kNUMPAD6 = SCE_IME_VK_NUMPAD6,
				kNUMPAD7 = SCE_IME_VK_NUMPAD7,
				kNUMPAD8 = SCE_IME_VK_NUMPAD8,
				kNUMPAD9 = SCE_IME_VK_NUMPAD9,
				kMULTIPLY = SCE_IME_VK_MULTIPLY,
				kADD = SCE_IME_VK_ADD,
				kSEPARATOR = SCE_IME_VK_SEPARATOR,
				kSUBTRACT = SCE_IME_VK_SUBTRACT,
				kDECIMAL = SCE_IME_VK_DECIMAL,
				kDIVIDE = SCE_IME_VK_DIVIDE,
				kF1 = SCE_IME_VK_F1,
				kF2 = SCE_IME_VK_F2,
				kF3 = SCE_IME_VK_F3,
				kF4 = SCE_IME_VK_F4,
				kF5 = SCE_IME_VK_F5,
				kF6 = SCE_IME_VK_F6,
				kF7 = SCE_IME_VK_F7,
				kF8 = SCE_IME_VK_F8,
				kF9 = SCE_IME_VK_F9,
				kF10 = SCE_IME_VK_F10,
				kF11 = SCE_IME_VK_F11,
				kF12 = SCE_IME_VK_F12,
				kF13 = SCE_IME_VK_F13,
				kF14 = SCE_IME_VK_F14,
				kF15 = SCE_IME_VK_F15,
				kF16 = SCE_IME_VK_F16,
				kF17 = SCE_IME_VK_F17,
				kF18 = SCE_IME_VK_F18,
				kF19 = SCE_IME_VK_F19,
				kF20 = SCE_IME_VK_F20,
				kF21 = SCE_IME_VK_F21,
				kF22 = SCE_IME_VK_F22,
				kF23 = SCE_IME_VK_F23,
				kF24 = SCE_IME_VK_F24,
				kNUMLOCK = SCE_IME_VK_NUMLOCK,
				kSCROLL = SCE_IME_VK_SCROLL,
				kLSHIFT = SCE_IME_VK_LSHIFT,
				kRSHIFT = SCE_IME_VK_RSHIFT,
				kLCONTROL = SCE_IME_VK_LCONTROL,
				kRCONTROL = SCE_IME_VK_RCONTROL,
				kLMENU = SCE_IME_VK_LMENU,
				kRMENU = SCE_IME_VK_RMENU,
				kOEM_1 = SCE_IME_VK_OEM_1,
				kOEM_PLUS = SCE_IME_VK_OEM_PLUS,
				kOEM_COMMA = SCE_IME_VK_OEM_COMMA,
				kOEM_MINUS = SCE_IME_VK_OEM_MINUS,
				kOEM_PERIOD = SCE_IME_VK_OEM_PERIOD,
				kOEM_2 = SCE_IME_VK_OEM_2,
				kOEM_3 = SCE_IME_VK_OEM_3,
				kOEM_4 = SCE_IME_VK_OEM_4,
				kOEM_5 = SCE_IME_VK_OEM_5,
				kOEM_6 = SCE_IME_VK_OEM_6,
				kOEM_7 = SCE_IME_VK_OEM_7,
				kOEM_8 = SCE_IME_VK_OEM_8,
				kOEM_102 = SCE_IME_VK_OEM_102,
				kOEM_COPY = SCE_IME_VK_OEM_COPY,
				kOEM_AUTO = SCE_IME_VK_OEM_AUTO,
				kCRSEL = SCE_IME_VK_CRSEL,
				kNONE = SCE_IME_VK_NONE
			};

			/*!
			 * @~English
			 * 
			 * @brief Class for handling keyboard operation 
			 * @details KeyboardContext is a class that enables applications to easily support keyboard operation. It is possible to get keyboard key states.
			 * @~Japanese
			 * 
			 * @brief キーボード操作を扱うためのクラス 
			 * @details KeyboardContextはアプリケーションがキーボード入力をサポートするのを簡便に行えるようにするためのクラスです。キーボードのキー状態を取得します。
			 */
			class KeyboardContext
			{
			public:
				KeyboardContext() {}
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
				KeyboardContext(SampleUtil::System::UserId userId);
				/*!
				 * @~English
				 * @brief Destructor 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * @details デストラクタです。 
				 */
				virtual ~KeyboardContext();
				/*!
				 * @~English
				 * 
				 * @brief Update 
				 * @details This is the update function of KeyboardContext. 
				 * @retval >=SCE_OK Success (SCE_OK when there is no keyboard)
				 * @retval (<0) Error code
				 * @~Japanese
				 * 
				 * @brief 更新 
				 * @details KeyboardContextの更新関数です。 
				 * @retval >=SCE_OK 成功(キーボードがない場合はSCE_OK)
				 * @retval (<0) エラーコード
				 */
				int update();
				/*!
				 * @~English
				 * 
				 * @brief Function to determine whether a keyboard is connected
				 * @retval true keyboard is connected
				 * @retval false keyboard is not connected 
				 * @details Determine whether a keybaord is connected
				 * @~Japanese
				 * 
				 * @brief キーボードが接続されていることを判定する関数 
				 * @retval true buttonsで指定されているボタンがpatternで指定されたパターンで見たときに押されている状態である
				 * @retval false buttonsで指定されているボタンがpatternで指定されたパターンで見たときに押されている状態ではない
				 * @details キーボードが接続されていることを判定します。 
				 */
				bool isConnected() const;
				/*!
				 * @~English
				 * 
				 * @brief Function to determine whether a key is being pressed 
				 * 
				 * @param key The enum value of Key used to judge the function
				 * 
				 * @retval true The key specified in key is being pressed
				 * @retval false The key specified in key is not being pressed
				 * @details This function determines whether the key specified in the key argument is currently being pressed. 
				 * @~Japanese
				 * 
				 * @brief キーが現在押されていることを判定する関数 
				 * 
				 * @param key 関数の判定に利用するKeyのenum値
				 * 
				 * @retval true keyで指定されているキーが押されている状態である
				 * @retval false keyで指定されているキーが押されている状態ではない
				 * @details 引数keyで指定されたキーが現在押されているかを判定する関数です。 
				 */
				bool isKeyDown(Key key) const;
				/*!
				 * @~English
				 * 
				 * @brief Function to determine whether a key is not currently being pressed 
				 * 
				 * @param key The enum value of Key used to judge the function
				 * 
				 * @retval true The key specified in key is not being pressed
				 * @retval false The key specified in key is being pressed
				 * @details This function determines whether the key specified in the key argument is currently being pressed. 
				 * @~Japanese
				 * 
				 * @brief キーが現在押されていないことを判定する関数 
				 * 
				 * @param key 関数の判定に利用するKeyのenum値
				 * 
				 * @retval true keyで指定されているキーが押されている状態ではない
				 * @retval false keyで指定されているキーが押されている状態である
				 * @details 引数keyで指定されたキーが現在押されていないかを判定する関数です。 
				 */
				bool isKeyUp(Key key) const;
				/*!
				 * @~English
				 * 
				 * @brief Function to determine whether a key has been pressed 
				 * 
				 * @param key The enum value of Key used to judge the function
				 * 
				 * @retval true The key specified in key has been pressed
				 * @retval false The key specified in key has not been pressed
				 * @details This function determines whether the key specified in the key argument has been pressed
				 * @~Japanese
				 * 
				 * @brief キーが押されたことを判定する関数 
				 * 
				 * @param key 関数の判定に利用するKeyのenum値
				 * 
				 * @retval true keyで指定されているキーが押された
				 * @retval false keyで指定されているキーが押されていない
				 * @details 引数keyで指定されたキーが押されたかどうかを判定する関数です。 
				 */
				bool isKeyPressed(Key key) const;
				/*!
				 * @~English
				 * 
				 * @brief Function to determine whether a key has been repeated
				 * 
				 * @param key The enum value of Key used to judge the function
				 * 
				 * @retval true The key specified in key has been repeated
				 * @retval false The key specified in key has not been repeated
				 * @details This function determines whether the key specified in the key argument has been repeated
				 * @~Japanese
				 * 
				 * @brief キーがリピートされたことを判定する関数 
				 * 
				 * @param key 関数の判定に利用するKeyのenum値
				 * 
				 * @retval true keyで指定されているキーがリピートされた
				 * @retval false keyで指定されているキーがリピートされていない
				 * @details 引数keyで指定されたキーがリピートされたかどうかを判定する関数です。 
				 */
				bool isKeyRepeated(Key key) const;
				/*!
				 * @~English
				 * 
				 * @brief Function to determine whether a key has been pressed or repeated
				 * 
				 * @param key The enum value of Key used to judge the function
				 * 
				 * @retval true The key specified in key has been pressed or repeated
				 * @retval false The key specified in key has not been pressed or repeated
				 * @details This function determines whether the key specified in the key argument has been pressed or repeated
				 * @~Japanese
				 * 
				 * @brief キーが押されたかリピートされたことを判定する関数 
				 * 
				 * @param key 関数の判定に利用するKeyのenum値
				 * 
				 * @retval true keyで指定されているキーが押されたかリピートされた
				 * @retval false keyで指定されているキーが押されたかリピートされていない
				 * @details 引数keyで指定されたキーが押されたかリピートされたかどうかを判定する関数です。 
				 */
				bool isKeyPressedOrRepeated(Key key) const
				{
					return isKeyPressed(key) || isKeyRepeated(key);
				}
				/*!
				 * @~English
				 * 
				 * @brief Function to determine whether a key has been released
				 * 
				 * @param key The enum value of Key used to judge the function
				 * 
				 * @retval true The key specified in key has been released
				 * @retval false The key specified in key has not been released
				 * @details This function determines whether the key specified in the key argument has been released
				 * @~Japanese
				 * 
				 * @brief キーが離されたことを判定する関数 
				 * 
				 * @param key 関数の判定に利用するKeyのenum値
				 * 
				 * @retval true keyで指定されているキーが離された
				 * @retval false keyで指定されているキーが離されていない
				 * @details 引数keyで指定されたキーが離されたかどうかを判定する関数です。 
				 */
				bool isKeyReleased(Key key) const;
				/*!
				 * @~English
				 * 
				 * @brief Function to obtain keyboard data newly held by KeyboardContext in the last KeyboardContext::update() call 
				 * 
				 * @return Returns the pointer to string buffer which has input key characters. 
				 * @details This function obtains keyboard input data newly held by KeyboardContext in the last KeyboardContext::update() call. Obtained keyboard input data is stored in the data array in order from oldest to newest with the value specified. 
				 * @~Japanese
				 * 
				 * @brief 最後のKeyboardContext::update()の呼び出しでKeyboardContextが新規に保持したキーボードデータを取得する関数 
				 * 
				 * @return キー入力データを格納した文字列バッファへのポインタ 
				 * @details 最後のKeyboardContext::update()の呼び出しでKeyboardContextが新規に保持したキーボード入力データを取得する関数です。取得したキーボード入力データを古いのものから順に配列に格納します。 
				 */
				wchar_t *getInputChars()
				{
					return m_charBuffer;
				}
#if !defined(DOXYGEN_IGNORE)
				static void imeHandler(void *arg, const SceImeEvent *e);

				enum Keystate
				{
					kKeyStatePressed = 1 << 0,
					kKeyStateDown = 1 << 1,
					kKeyStateReleased = 1 << 2,
					kKeyStateUp = 1 << 3,
					kKeyStateRepeated = 1 << 4
				};

				uint32_t m_keyStateArray[0x100];
				System::UserId m_userId;
#if _SCE_TARGET_OS_ORBIS
				uint32_t m_resourceId;
#endif
				wchar_t m_charBuffer[1024];
#endif
				/*!
				 * @~English
				 * @brief Initializes Keyboard context
				 * @param userId User ID
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief Keyboardコンテキスト初期化
				 * @param userId ユーザーID
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int initialize(System::UserId userId);

				/*!
				 * @~English
				 * @brief Finalizes Keyboard context
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief Keyboardコンテキストの終了処理
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int finalize();

			}; // class KeyboardContext
		}
	}
}
