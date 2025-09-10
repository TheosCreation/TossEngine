/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <unordered_map>
#include <istream>
#include <string>

namespace sce { namespace SampleUtil {
	//! Forward declarations:
	class SampleSkeleton;

	/*!
	 * @~English
	 * @brief Application Script
	 * @details Script executer to run application script
	 * @~Japanese
	 * @brief アプリケーションスクリプト
	 * @details アプリケーションスクリプトの実行器
	 */
	class ApplicationScript
	{
	public:
		/*!
		 * @~English
		 * @brief Script command callback
		 * @param app The application which command is executed on
		 * @param param Parameter for command
		 * @details The type definition of a callback that is called when a command is found.
		 * @~Japanese
		 * @brief スクリプトコマンドコールバック
		 * @param app コマンド実行対象のアプリケーション
		 * @param param コマンドのパラメータ
		 * @details コマンド実行時に呼ばれる関数の型定義
		 */
		typedef int	(*ApplicationScriptCallback)(sce::SampleUtil::SampleSkeleton &app, const std::string	&param);

		/*!
		 * @~English
		 * @brief Render frames
		 * @param app The target application of the script command.
		 * @param param The string containing the number of frames
		 * @details Render specified number of frames
		 * @~Japanese
		 * @brief フレームをレンダリング
		 * @param app スクリプトコマンドの実行対象アプリケーション
		 * @param param フレーム数を含む文字列
		 * @details 指定したフレーム鵜をレンダリング
		 */
		static int	frames(sce::SampleUtil::SampleSkeleton &app, const std::string &params);

		/*!
		 * @~English
		 * @brief Dump the current framerate
		 * @param app The target application of the script command.
		 * @param param The string to be dumped
		 * @details Dumps the current framerate and framecount and to TTY.
		 * @~Japanese
		 * @brief 現在のフレームレートをダンプ
		 * @param app スクリプトコマンドの実行対象アプリケーション
		 * @param param ダンプする文字列
		 * @details 現在のフレームレートとフレーム数をTTYにダンプする
		 */
		static int	framerate(sce::SampleUtil::SampleSkeleton &app, const std::string &params);

		/*!
		 * @~English
		 * @brief Screenshot function
		 * @param app The target application of the script command.
		 * @param param The string containing image filename and video out bus 
		 * @details Save screenshot to specified file
		 * @~Japanese
		 * @brief スクリーンショット関数
		 * @param app スクリプトコマンドの実行対象アプリケーション
		 * @param param イメージファイル名とビデオアウトバスを含む文字列
		 * @details 指定したファイルにスクリーンショットをセーブ
		 */
		static int	screenshot(sce::SampleUtil::SampleSkeleton &app, const std::string &params);

		/*!
		 * @~English
		 * @brief Button press function
		 * @param app The target application of the script command.
		 * @param param The string containing button name to be pressed
		 * @details Execute specified pad button press
		 * @~Japanese
		 * @brief ボタン押下関数
		 * @param app スクリプトコマンドの実行対象アプリケーション
		 * @param param 押下ボタンを含む文字列
		 * @details 指定したパッドボタンを押下
		 */
		static int	buttonpress(sce::SampleUtil::SampleSkeleton &app, const std::string &params);

		/*!
		 * @~English
		 * @brief Button down function
		 * @param app The target application of the script command.
		 * @param param The string containing button name to be down
		 * @details Execute specified pad button down
		 * @~Japanese
		 * @brief ボタン押下関数
		 * @param app スクリプトコマンドの実行対象アプリケーション
		 * @param param 押下ボタンを含む文字列
		 * @details 指定したパッドボタンを押下
		 */
		static int	buttonDown(sce::SampleUtil::SampleSkeleton &app, const std::string &params);

		/*!
		 * @~English
		 * @brief Button up function
		 * @param app The target application of the script command.
		 * @param param The string containing button name to be up
		 * @details Execute specified pad button up
		 * @~Japanese
		 * @brief ボタン解放関数
		 * @param app スクリプトコマンドの実行対象アプリケーション
		 * @param param 解放ボタンを含む文字列
		 * @details 指定したパッドボタンを解放
		 */
		static int	buttonUp(sce::SampleUtil::SampleSkeleton &app, const std::string &params);

		/*!
		 * @~English
		 * @brief Quit application
		 * @param app The target application of the script command.
		 * @param param Not used
		 * @details Quit target application
		 * @~Japanese
		 * @brief アプリケーションを終了
		 * @param app スクリプトコマンドの実行対象アプリケーション
		 * @param param 未使用
		 * @details アプリケーションを終了
		 */
		static int	quit(sce::SampleUtil::SampleSkeleton &app, const std::string &params);

		/*!
		 * @~English
		 * @brief Constructor
		 * @details Constructor
		 * @~Japanese
		 * @brief コンストラクタ
		 * @details コンストラクタ
		 */
		ApplicationScript();

		/*!
		 * @~English
		 * @brief Execute next command
		 * @param script Script which next command is fetched
		 * @param app The target application of the script command.
		 * @details Execute next command in script
		 * @~Japanese
		 * @brief 次のコマンドを実行
		 * @param script 次のコマンドをフェッチする対象スクリプト
		 * @param app スクリプトコマンドの実行対象アプリケーション
		 * @details スクリプトの次のコマンドを実行
		 */
		int	executeNextCommand(std::istream	&script, sce::SampleUtil::SampleSkeleton	&app);

		std::unordered_map<std::string, ApplicationScriptCallback>	m_commands;
	}; // class ApplicationScript
}} // namespace sce::SampleUtil

