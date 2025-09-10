/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#pragma once

#include <condition_variable>
#include <sampleutil/sampleutil_common.h>

namespace sce
{
	namespace SampleUtil
	{
		namespace Debug
		{
			/*!
			 * @~English
			 *
			 * @brief Console option
			 * @details This is Console option.
			 * @~Japanese
			 *
			 * @brief コンソールオプション
			 * @details コンソールオプションです。
			 */
			struct ConsoleOption
			{
				/*!
				 * @~English
				 * @brief The name of console
				 * @~Japanese
				 * @brief コンソールの名前
				 */
				char name[256] = "";
				/*!
				 * @~English
				 * @brief The height of font in percentage to screen height
				 * @~Japanese
				 * @brief スクリーンの高さに対する割合でのフォントの高さ
				 */
				float fontHeightInPercentageToScreenHeight;
				/*!
				 * @~English
				 * @brief The line spacing in percentage to screen height
				 * @~Japanese
				 * @brief スクリーンの高さに対する割合でのラインスペーシング
				 */
				float lineSpacingInPercentageToScreenHeight;
				/*!
				 * @~English
				 * @brief The console width in character
				 * @~Japanese
				 * @brief キャラクタ数でのコンソールの幅
				 */
				uint32_t widthInChars;
				/*!
				 * @~English
				 * @brief The console height in character
				 * @~Japanese
				 * @brief キャラクタ数でのコンソールの高さ
				 */
				uint32_t heightInChars;
				/*!
				 * @~English
				 * @brief The X cordinate of upper left screen position in percentage to screen width(Default position is used if negative value is specified)
				 * @~Japanese
				 * @brief スクリーンの幅に対する割合でのコンソールの左上位置のX座標(負値の場合は既定位置となる)
				 */
				float positionXInPercentageToScreenWidth = -1.f;
				/*!
				 * @~English
				 * @brief The Y cordinate of upper left screen position in percentage to screen height(Default position is used if negative value is specified)
				 * @~Japanese
				 * @brief スクリーンの高さに対する割合でのコンソールの左上位置のY座標(負値の場合は既定位置となる)
				 */
				float positionYInPercentageToScreenHeight = -1.f;
				/*!
				 * @~English
				 * @brief The foreground color(Default color if negative value is specified)
				 * @~Japanese
				 * @brief フォアグラウンドカラー(負値の場合は既定カラー)
				 */
				float fgColor[3] = { -1.f, -1.f, -1.f };
				/*!
				 * @~English
				 * @brief The background color(Default color if negative value is specified)
				 * @~Japanese
				 * @brief バックグラウンドカラー(負値の場合は既定カラー)
				 */
				float bgColor[3] = { -1.f, -1.f, -1.f };
			};

			/*!
			 * @~English
			 * @brief Debug console
			 * @details The class definition to realize target side TTY console
			 * @~Japanese
			 * @brief デバッグ用コンソール
			 * @details ターゲット側でのTTY機能を実現するクラス
			 */
			class Console
			{
			public:
				/*!
				 * @~English
				 * @brief Constructor
				 * @param lineHeight The font height in percentage to screen height
				 * @param lines The console height in character
				 * @param nCharsPerLine The console width in character
				 * @param lineSpacing The line spacing in percentage to screen height
				 * @param position Upper left screen position in percentage to screen width/height(Default position is used if negative value is specified)
				 * @param name The name of console
				 * @param fgColor The foreground color(Default color if negative value is specified)
				 * @param bgColor The background color(Default color if negative value is specified)
				 * @details This is a constructor.
				 * @~Japanese
				 * @brief コンストラクタ
				 * @param lineHeight スクリーンの高さに対する割合でのフォントの高さ
				 * @param lines キャラクタ数でのコンソールの高さ
				 * @param nCharsPerLine キャラクタ数でのコンソールの幅
				 * @param lineSpacing スクリーンの高さに対する割合でのラインスペーシング
				 * @param position スクリーンの幅・高さに対する割合でのコンソールの左上位置座標(負値の場合は既定位置となる)
				 * @param name コンソールの名前
				 * @param fgColor フォアグラウンドカラー(負値の場合は既定カラー)
				 * @param bgColor バックグラウンドカラー(負値の場合は既定カラー)
				 * @details コンストラクタです。
				 */
				Console(float lineHeight, uint32_t lines, uint32_t nCharsPerLine, float lineSpacing, float position[2], const char *name, float fgColor[3], float bgColor[3]);

				/*!
				 * @~English
				 * @brief Destructor 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * @details デストラクタです。 
				 */
				virtual ~Console();
				/*!
				 * @~English
				 * 
				 * @brief Update 
				 * @details This is the update function of Console. 
				 * @retval >=SCE_OK Success (SCE_OK when there is no controller)
				 * @retval (<0) Error code
				 * @~Japanese
				 * 
				 * @brief 更新 
				 * @details Consoleの更新関数です。 
				 * @retval >=SCE_OK 成功(コントローラーがない場合はSCE_OK)
				 * @retval (<0) エラーコード
				 */
				int update();
				/*!
				 * @~English
				 * 
				 * @brief vprintf
				 * @details vprintf to print to console
				 *
				 * @param format format string by which output is produced
				 * @param ap variable length arguments list
				 *
				 * @retval >= SCE_OK Success(returns the number of characters printed (excluding the null byte used to end output to strings))
				 * @retval (<0) Error code
				 * @~Japanese
				 * 
				 * @brief vprintf
				 * @details vprintfの結果をコンソールに出力
				 *
				 * @param format 出力を生成する書式文字列
				 * @param ap 可変長引数リスト
				 *
				 * @retval >=SCE_OK 成功(書き込まれた文字数を返す (文字列の最後を示すために使用するヌルバイトは数に含まれない))
				 * @retval (<0) エラーコード
				 */
				int vprintf(const char *format, va_list ap);
				/*!
				 * @~English
				 * 
				 * @brief printf
				 * @details printf to print to console
				 *
				 * @param format format string by which output is produced
				 *
				 * @retval >= SCE_OK Success(returns the number of characters printed (excluding the null byte used to end output to strings))
				 * @retval (<0) Error code
				 * @~Japanese
				 * 
				 * @brief printf
				 * @details printfの結果をコンソールに出力
				 *
				 * @param format 出力を生成する書式文字列
				 *
				 * @retval >=SCE_OK 成功(書き込まれた文字数を返す (文字列の最後を示すために使用するヌルバイトは数に含まれない))
				 * @retval (<0) エラーコード
				 */
				int  printf(const char *format, ...);
				/*!
				 * @~English
				 * 
				 * @brief vscanf
				 * @details Input data from console input using variable length variable list
				 *
				 * @param format pointer to a null-terminated character string specifying how to read the input. 
				 * @param ap variable length arguments list
				 *
				 * @retval >=SCE_OK Success(Number of receiving arguments successfully assigned)
				 * @retval EOF Read failure occurs before the first receiving argument was assigned.
				 * @~Japanese
				 * 
				 * @brief vscanf
				 * @details コンソール入力から可変長引数リストを用いてデータを読込みます。
				 *
				 * @param format 入力からの読み込みを指定するNULL終端書式文字列
				 * @param ap 可変長引数リスト
				 *
				 * @retval >=SCE_OK 成功(代入された入力項目の個数)
				 * @retval EOF 失敗
				 */
				int vscanf(const char *format, va_list ap);
				/*!
				 * @~English
				 * 
				 * @brief scanf
				 * @details Input data from console input using variable length variable list
				 *
				 * @param format pointer to a null-terminated character string specifying how to read the input. 
				 *
				 * @retval >=SCE_OK Success(Number of receiving arguments successfully assigned)
				 * @retval EOF Read failure occurs before the first receiving argument was assigned.
				 * @~Japanese
				 * 
				 * @brief scanf
				 * @details コンソール入力から可変長引数リストを用いてデータを読込みます。
				 *
				 * @param format 入力からの読み込みを指定するNULL終端書式文字列
				 *
				 * @retval >=SCE_OK 成功(代入された入力項目の個数)
				 * @retval EOF 失敗
				 */
				int  scanf(const char *format, ...);

			private:
				void writeToConsole(char *str);

				float					m_fontHeight;
				float					m_lineHeight;
				int						m_lines;
				uint32_t				m_nCharsPerLine;
				ImVec2         			m_consoleSize;
				ImVec4					m_fgColor;
				ImVec4					m_bgColor;
				char					m_name[256];
				ImVec2					m_position;

				char					*m_stringBuffer;
				int						m_stringBufferSize;
				bool					m_enterPressed;

				std::condition_variable	m_writeString_cv;
				bool					m_isWritingString = false;

				std::mutex				m_scanf_m;
				std::condition_variable	m_scanf_cv;
				char					*m_scanbuffer = nullptr;
			};
		}
	}
}
