/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2021 Sony Interactive Entertainment Inc. 
 * 
 */

#pragma once

#include <vector>
#include <scebase_common.h>
#include <vectormath/cpp/vectormath_aos.h>
#include <pad.h>
#include <kernel.h>
#if _SCE_TARGET_OS_PROSPERO
#include <pad_vr_controller.h>
#endif
#include <sampleutil/sampleutil_common.h>
#include <sampleutil/input/controller.h>
#include <sampleutil/input/motion.h>
#include <sampleutil/input/keyboard.h>
#include <sampleutil/input/osk.h>
#include <sampleutil/system.h>


namespace sce
{
	namespace SampleUtil
	{
		/*!
		 * @~English
		 * @brief Input-associated definitions 
		 * @details sce::SampleUtil::Input is the name space associated with the input of the SampleUtil library. 
		 * @~Japanese
		 * @brief Input関連の定義 
		 * @details sce::SampleUtil::InputはSampleUtilライブラリの入力関連の名前空間です。 
		 */
		namespace Input
		{
			/*!
			 * @~English
			 * @brief Maximum history size
			 * @~Japanese
			 * @brief 最大履歴サイズ
			 */
			static const size_t kMaxNumPadDataBufs = SCE_PAD_MAX_DATA_NUM;
			/*!
			 * @~English
			 * @brief Default history size
			 * @~Japanese
			 * @brief 既定履歴サイズ
			 */
			static const size_t kDefaultNumPadDataBufs = 16;

			/*!
			 * @~English
			 * 
			 * @brief Structure for initializing PadContext 
			 * @details This is the structure for initializing PadContext. This is used by specifying it to the argument "option" of PadContext::initialize(). 
			 * @~Japanese
			 * 
			 * @brief PadContextの初期化用構造体 
			 * @details PadContextの初期化用構造体です。PadContext::initialize()の引数optionに指定することで利用します。 
			 */
			struct PadContextOption
			{
				/*!
				 * @~English
				 * @brief scePadOpen() option argument 
				 * @details This is the scePadOpen() option argument. 
				 * @~Japanese
				 * @brief scePadOpen()用のオプション引数 
				 * @details scePadOpen()用のオプション引数です。 
				 */
				const ScePadOpenParam* padOpenParam;
				/*!
				 * @~English
				 * @brief Number of the buffers of the ScePadData structure retained within PadContext 
				 * @details Specify a value from 1 to 64. The default value is 16. 
				 * @~Japanese
				 * @brief PadContext内で保持するScePadData構造体のバッファ数 
				 * @details 1～64の値を指定してください。デフォルト値は16です。 
				 */
				uint32_t	numBufs;

				/*!
				 * @~English
				 * @brief Constructor 
				 * @details This is a constructor. 
				 * @~Japanese
				 * @brief コンクトラクタ 
				 * @details コンストラクタです。 
				 */
				PadContextOption():

				padOpenParam(nullptr),
				numBufs(16)
				{
				}
			};

			/*!
			 * @~English
			 * 
			 * @brief Touch point data 
			 * @details This structure stores the information for a single touch point from the touch data. 
			 * @~Japanese
			 * 
			 * @brief タッチ点のデータ 
			 * @details タッチデータのうちタッチ点１つぶんの情報を格納する構造体です。 
			 */
			struct TouchPadData
			{
				/*!
				 * @~English
				 * @brief x relative position (0 to 1) of touch point 
				 * @~Japanese
				 * @brief タッチ点のxの相対位置(0～1) 
				 */
				float x;
				/*!
				 * @~English
				 * @brief y relative position (0 to 1) of touch point 
				 * @~Japanese
				 * @brief タッチ点のyの相対位置(0～1) 
				 */
				float y;
				/*!
				 * @~English
				 * @brief Touch report ID (0 to 127) 
				 * @~Japanese
				 * @brief タッチレポートID（0～127） 
				 */
				uint8_t  id;
			};

			/*!
			 * @~English
			 * 
			 * @brief Magnetometer data 
			 * @details This structure stores magenetometer data. 
			 * @~Japanese
			 * 
			 * @brief 地磁気センサーのデータ 
			 * @details 地磁気センサーのデータを格納する構造体です。 
			 */
			struct MagnetometerData
			{
				/*!
				 * @~English
				 * @brief 4x4 rotation matrix for orientation based on NED (North East Down) 
				 * @~Japanese
				 * @brief NED（North East Down）に基づいた方向用の4x4回転行列 
				 */
				sce::Vectormath::Simd::Aos::Matrix4		northEastDownOrientation;
				/*!
				 * @~English
				 * @brief Value that indicates magnetic field stability 
				 * @~Japanese
				 * @brief 磁場の安定性を表す値 
				 */
				uint8_t									magneticFieldStability;
			};

			/*!
			 * @~English
			 * 
			 * @brief Motion data 
			 * @details This structure stores motion data (acceleration, angular velocity, orientation). 
			 * @~Japanese
			 * 
			 * @brief モーションデータ 
			 * @details モーションデータ（加速度、角速度、姿勢）を格納する構造体です。 
			 */
			struct MotionData
			{
				/*!
				 * @~English
				 * @brief Controller orientation value 
				 * @~Japanese
				 * @brief コントローラーの姿勢値 
				 */
				sce::Vectormath::Simd::Aos::Quat				orientation;	
				/*!
				 * @~English
				 * @brief Acceleration sensor XYZ reading (G) 
				 * @~Japanese
				 * @brief 加速度センサーのXYZ測定値（G） 
				 */
				sce::Vectormath::Simd::Aos::Vector3				acceleration;	
				/*!
				 * @~English
				 * @brief Angular velocity sensor XYZ reading (rad/s) 
				 * @~Japanese
				 * @brief 角速度センサーのXYZ測定値（rad/s） 
				 */
				sce::Vectormath::Simd::Aos::Vector3				angularVelocity;
				/*!
				 * @~English
				 * @brief Basic device orientation based on gravity<x, y, z> 
				 * @~Japanese
				 * @brief 重力を基準にしたデバイスの簡易的な方向<x, y, z> 
				 */
				sce::Vectormath::Simd::Aos::Vector3				userInterfaceOrientation;

				/*!
				 * @~English
				 * @brief Magnetometer data 
				 * @details Data is only stored for PlayStation(R)Vita. 
				 * @~Japanese
				 * @brief 地磁気センサーのデータ 
				 * @details PlayStation(R)Vitaでのみデータが格納されます。 
				 */
				MagnetometerData								magnetometer;
			};

			/*!
			 * @~English
			 * 
			 * @brief Output data received from the controller 
			 * @details This structure is for obtaining the controller data information. 
			 * @~Japanese
			 * 
			 * @brief コントローラーから受け取った出力データ 
			 * @details コントローラーデータの情報を取得するための構造体です。 
			 */
			struct PadData 
			{
				/*!
				 * @~English
				 * @brief System timestamp when the data is obtained 
				 * @~Japanese
				 * @brief データ取得時のシステムタイムスタンプ 
				 */
				uint64_t	timeStamp;
				/*!
				 * @~English
				 * @brief Controller connection flag value 
				 * @~Japanese
				 * @brief コントローラー接続フラグ 
				 */
				bool		connected;
				/*!
				 * @~English
				 * @brief Digital button data 
				 * @~Japanese
				 * @brief デジタルボタンデータ 
				 */
				uint32_t	buttons;	
				/*!
				 * @~English
				 * @brief x coordinate for the left analog stick data (0 to 255) 
				 * @~Japanese
				 * @brief 左アナログスティックのx座標(0～255) 
				 */
				uint8_t		lx;			
				/*!
				 * @~English
				 * @brief y coordinate for the left analog stick data (0 to 255) 
				 * @~Japanese
				 * @brief 左アナログスティックのy座標(0～255) 
				 */
				uint8_t		ly;			
				/*!
				 * @~English
				 * @brief x coordinate for the right analog stick data (0 to 255) 
				 * @~Japanese
				 * @brief 右アナログスティックのx座標(0～255) 
				 */
				uint8_t		rx;			
				/*!
				 * @~English
				 * @brief y coordinate for the right analog stick data (0 to 255) 
				 * @~Japanese
				 * @brief 右アナログスティックのy座標(0～255) 
				 */
				uint8_t		ry;			

				/*!
				 * @~English
				 * @brief L2 analog value 
				 * @~Japanese
				 * @brief L2のアナログ値 
				 */
				uint8_t		l2;
				/*!
				 * @~English
				 * @brief R2 analog value 
				 * @~Japanese
				 * @brief R2のアナログ値 
				 */
				uint8_t		r2;
				/*!
				 * @~English
				 * @brief Number of touch points 
				 * @~Japanese
				 * @brief タッチ点の個数 
				 */
				uint8_t		touchNumber;
				/*!
				 * @~English
				 * @brief Touch point data array 
				 * @~Japanese
				 * @brief タッチ点データの配列 
				 */
				TouchPadData touchPadData[2];
				/*!
				 * @~English
				 * @brief Motion data 
				 * @~Japanese
				 * @brief モーションデータ 
				 */
				MotionData   motionData;

				/*!
				 * @~English
				 * @brief Flag to indicate whether or not the controller is taken by the system software 
				 * @~Japanese
				 * @brief コントローラーがシステムソフトに奪われているかを示すフラグ 
				 */
				bool         intercepted;
				/*!
				 * @~English
				 * @brief Controller connection count
				 * @~Japanese
				 * @brief コントローラー接続の回数
				 */
				uint8_t		connectedCount;
			};

			/*!
			 * @~English
			 * 
			 * @brief Class for handling controller operation 
			 * @details PadContext is a class that enables applications to easily support controller operation. It is possible to perform management of controller button states/analog stick values, management of analog stick dead zones, button "Press" and "Release" state detection, motion sensor enabled/disabled settings, tilt correction enabled/disabled settings, controller orientation value resetting, angular velocity sensor deadband filter enabled/disabled settings, and controller vibration control. 
			 * @~Japanese
			 * 
			 * @brief コントローラー操作を扱うためのクラス 
			 * @details PadContextはアプリケーションがコントローラー操作をサポートするのを簡便に行えるようにするためのクラスです。コントローラーのボタン状態やアナログスティック値の管理、 アナログスティックのデッドゾーンの管理、ボタンの「Press」および「Release」ステートの検出、モーションセンサーの有効・無効の設定、傾き補正の有効・無効の設定、コントローラー姿勢値のリセット、角速度センサーのデッドバンドフィルタの有効・無効の設定、コントローラーの振動制御を行うことが可能です。 
			 */
			class PadContext
			{
			public:
				PadContext() {}
				/*!
				* @~English
				* @brief Constructor
				* @param userId User ID
				* @param type Controller type
				* @param index Device index (specify 0)
				* @param option PadContextOption structure. This is initialized by the default value if NULL is specified.
				* @details This is a constructor.
				* @~Japanese
				* @brief コンストラクタ
				* @param userId ユーザーID
				* @param type コントローラーのタイプ
				* @param index デバイスインデックス（0を指定すること）
				* @param option PadContextOption構造体。NULLを指定するとデフォルトの値で初期化されます。
				* @details コンストラクタです。
				*/
				PadContext(const sce::SampleUtil::System::UserId userId, const int32_t type, const int32_t index, const PadContextOption* option);
				/*!
				 * @~English
				 * @brief Destructor 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * @details デストラクタです。 
				 */
				virtual ~PadContext();
				/*!
				 * @~English
				 * 
				 * @brief Update 
				 * @param pScriptPadData Pad data provided by script
				 * @details This is the update function of PadContext. 
				 * @retval >=SCE_OK Success (SCE_OK when there is no controller)
				 * @retval (<0) Error code
				 * @~Japanese
				 * 
				 * @brief 更新 
				 * @param pScriptPadData スクリプトから与えられたパッドデータ
				 * @details PadContextの更新関数です。 
				 * @retval >=SCE_OK 成功(コントローラーがない場合はSCE_OK)
				 * @retval (<0) エラーコード
				 */
				int update(Input::PadData	*pScriptPadData = nullptr);
				/*!
				 * @~English
				 * 
				 * @brief Function to determine whether a button is being pressed 
				 * 
				 * @param buttons The enum value of Button used to judge the function
				 * @param pattern The matching pattern of the button. PATTERN_ALL is specified as the default parameter.
				 * 
				 * @retval true The button specified in buttons is being pressed in reference to the pattern specified by pattern
				 * @retval false The button specified in buttons is not being pressed in reference to the pattern specified by pattern
				 * @details This function refers to the matching pattern specified in the pattern argument to determine whether the button specified in the buttons argument is currently being pressed. 
				 * @~Japanese
				 * 
				 * @brief ボタンが現在押されていることを判定する関数 
				 * 
				 * @param buttons 関数の判定に利用するButtonのenum値
				 * @param pattern ボタンの一致パターン。デフォルトパラメータとしてPATTERN_ALLが指定されています。
				 * 
				 * @retval true buttonsで指定されているボタンがpatternで指定されたパターンで見たときに押されている状態である
				 * @retval false buttonsで指定されているボタンがpatternで指定されたパターンで見たときに押されている状態ではない
				 * @details 引数buttonsで指定されたボタンが引数patternで指定された一致パターンで判定した際に、現在押されているかを判定する関数です。 
				 */
				bool isButtonDown(uint32_t buttons, ButtonEventPattern pattern=kButtonEventPatternAll) const;
				/*!
				 * @~English
				 * 
				 * @brief Function to determine that a button is not currently being pressed 
				 * 
				 * @param buttons The enum value of Button used to judge the function
				 * @param pattern The matching pattern of the button. PATTERN_ALL is specified as the default parameter.
				 * 
				 * @retval true The button specified in buttons is not being pressed in reference to the pattern specified by pattern
				 * @retval false The button specified in buttons is being pressed in reference to the pattern specified by pattern
				 * @details This function refers to the matching pattern specified in the pattern argument to determine whether the button specified in the buttons argument is currently not being pressed. 
				 * @~Japanese
				 * 
				 * @brief ボタンが現在押されていないことを判定する関数 
				 * 
				 * @param buttons 関数の判定に利用するButtonのenum値
				 * @param pattern ボタンの一致パターン。デフォルトパラメータとしてPATTERN_ALLが指定されています。
				 * 
				 * @retval true buttonsで指定されているボタンがpatternで指定されたパターンで見たときに押されていない状態である
				 * @retval false buttonsで指定されているボタンがpatternで指定されたパターンで見たときに押されていない状態ではない
				 * @details 引数buttonsで指定されたボタンが引数patternで指定された一致パターンで判定した際に、現在押されていないことをを判定する関数です。 
				 */
				bool isButtonUp(uint32_t buttons, ButtonEventPattern pattern=kButtonEventPatternAll) const;
				/*!
				 * @~English
				 * 
				 * @brief Function to determine that a button has been pressed 
				 * 
				 * @param buttons The enum value of Button used to judge the function
				 * @param pattern The matching pattern of the button. PATTERN_ALL is specified as the default parameter.
				 * 
				 * @retval true The button specified in buttons has been pressed in reference to the pattern specified by pattern
				 * @retval false The button specified in buttons has not been pressed in reference to the pattern specified by pattern
				 * @details This function refers to the matching pattern specified in the pattern argument for the button specified in the buttons arguments and compares the controller values obtained with the last update() call and the update() call preceding it to determine whether the button has been pressed. 
				 * @~Japanese
				 * 
				 * @brief ボタンが押されたことを判定する関数 
				 * 
				 * @param buttons 関数の判定に利用するButtonのenum値
				 * @param pattern ボタンの一致パターン。デフォルトパラメータとしてPATTERN_ALLが指定されています。
				 * 
				 * @retval true buttonsで指定されているボタンがpatternで指定されたパターンで見たときに押された
				 * @retval false buttonsで指定されているボタンがpatternで指定されたパターンで見たときに押されていない
				 * @details 引数buttonsで指定されたボタンが引数patternで指定された一致パターンで判定した際に、最後と最後から一つ前のupdate()で取得されたコントローラーの値と比較して、ボタンが押されたかどうかを判定する関数です。 
				 */
				bool isButtonPressed(uint32_t buttons, ButtonEventPattern pattern=kButtonEventPatternAll) const;
				/*!
				 * @~English
				 * 
				 * @brief Function to determine that a button has been released 
				 * 
				 * @param buttons The enum value of Button used to judge the function
				 * @param pattern The matching pattern of the button. PATTERN_ALL is specified as the default parameter.
				 * 
				 * @retval true The button specified in buttons has been released in reference to the pattern specified by pattern
				 * @retval false The button specified in buttons has not been released in reference to the pattern specified by pattern
				 * @details This function refers to the matching pattern specified in the pattern argument for the button specified in the buttons arguments and compares the controller values obtained with the last update() call and the update() call preceding it to determine whether the button has been released. 
				 * @~Japanese
				 * 
				 * @brief ボタンが離されたことを判定する関数 
				 * 
				 * @param buttons 関数の判定に利用するButtonのenum値
				 * @param pattern ボタンの一致パターン。デフォルトパラメータとしてPATTERN_ALLが指定されています。
				 * 
				 * @retval true buttonsで指定されているボタンがpatternで指定されたパターンで見たときに離された
				 * @retval false buttonsで指定されているボタンがpatternで指定されたパターンで見たときに離されていない
				 * @details 引数buttonsで指定されたボタンが引数patternで指定された一致パターンで判定した際に、最後と最後から一つ前のupdate()で取得されたコントローラーの値と比較して、ボタンが離されたかどうかを判定する関数です。 
				 */
				bool isButtonReleased(uint32_t buttons, ButtonEventPattern pattern=kButtonEventPatternAll) const;
				/*!
				 * @~English
				 * 
				 * @brief Function to get left analog stick data 
				 * 
				 * @return Returns the value of the left analog stick in the Vector2 type. In the X and Y values of Vector2, the X direction and Y direction of the left analog stick are stored in a float value, respectively. 
				 * @details This is the function to get left analog stick data. In the X and Y values of Vector2 of the return value, the X direction and Y direction of the left analog stick are stored in a float value, respectively. In this function, the dead zone is also taken into account; this dead zone can be changed with setLeftAnalogStickDeadZone(). 
				 * @~Japanese
				 * 
				 * @brief 左アナログスティックのデータ取得関数 
				 * 
				 * @return 左アナログスティックの値をVector2型で返します。Vector2のX値に左アナログスティックのX方向がfloat値で、Vector2のY値に左アナログスティックのY方向がfloat値で格納されています。 
				 * @details 左アナログスティックのデータ取得関数です。返り値のVector2のX値に左アナログスティックのX方向がfloat値で、Vector2のY値に左アナログスティックのY方向がfloat値で格納されています。また、本関数ではデッドゾーンが考慮されており、このデッドゾーンはsetLeftAnalogStickDeadZone()で変更可能です。 
				 */
				const sce::Vectormath::Simd::Aos::Vector2& getLeftStick() const;
				/*!
				 * @~English
				 * 
				 * @brief Function to get right analog stick data 
				 * 
				 * @return Returns the value of the right analog stick in the Vector2 type. In the X and Y values of Vector2, the X direction and Y direction of the right analog stick are stored in a float value, respectively. 
				 * @details This is the function to get right analog stick data. In the X and Y values of Vector2 of the return value, the X direction and Y direction of the right analog stick are stored in a float value, respectively. In this function, the dead zone is also taken into account; this dead zone can be changed with setRightAnalogStickDeadZone(). 
				 * @~Japanese
				 * 
				 * @brief 右アナログスティックのデータ取得関数 
				 * 
				 * @return 右アナログスティックの値をVector2型で返します。Vector2のX値に右アナログスティックのX方向がfloat値で、Vector2のY値に右アナログスティックのY方向がfloat値で格納されています。 
				 * @details 右アナログスティックのデータ取得関数です。返り値のVector2のX値に右アナログスティックのX方向がfloat値で、Vector2のY値に右アナログスティックのY方向がfloat値で格納されています。また、本関数ではデッドゾーンが考慮されており、このデッドゾーンはsetRightAnalogStickDeadZone()で変更可能です。 
				 */
				const sce::Vectormath::Simd::Aos::Vector2& getRightStick() const;
				/*!
				 * @~English
				 * 
				 * @brief Function to set right stick dead zones 
				 * 
				 * @param rightAnalogStickDeadZone Dead zone value of the right stick
				 * @details This is the function to set the dead zones of the right stick. The default value is set to 0.25 for PlayStation(R)Vita and to the value obtained from the controller for PlayStation(R)4. For details, see "Pad Library Overview". 
				 * @~Japanese
				 * 
				 * @brief 右スティックのデッドゾーン設定関数 
				 * 
				 * @param rightAnalogStickDeadZone 右スティックのデッドゾーン値
				 * @details 右スティックのデッドゾーンを設定する関数です。デフォルト値としてPlayStation(R)Vitaでは0.25、PlayStation(R)4ではコントローラーから取得された値が設定されています。ワイヤレスコントローラー（DUALSHOCK(R)4）の詳細は「Padライブラリ 概要」を参照ください。 
				 */
				void setRightAnalogStickDeadZone(float rightAnalogStickDeadZone);
				/*!
				 * @~English
				 * 
				 * @brief Function to set left stick dead zones 
				 * 
				 * @param leftAnalogStickDeadZone Dead zone value of the left stick
				 * @details This is the function to set the dead zones of the left stick. The default value is set to 0.25 for PlayStation(R)Vita and to the value obtained from the controller for PlayStation(R)4. For details, see "Pad Library Overview". 
				 * @~Japanese
				 * 
				 * @brief 左スティックのデッドゾーン設定関数 
				 * 
				 * @param leftAnalogStickDeadZone 左スティックのデッドゾーン値
				 * @details 左スティックのデッドゾーンを設定する関数です。デフォルト値としてPlayStation(R)Vitaでは0.25、PlayStation(R)4ではコントローラーから取得された値が設定されています。ワイヤレスコントローラー（DUALSHOCK(R)4）の詳細は「Padライブラリ 概要」を参照ください。 
				 */
				void setLeftAnalogStickDeadZone(float leftAnalogStickDeadZone);
				/*!
				 * @~English
				 * 
				 * @brief Function to obtain controller data newly held by PadContext in the last PadContext::update() call 
				 * 
				 * @param data The array of the write destination of the PadData structure
				 * @param length Number of elements in the data array (0 to 64)
				 * @return Returns the number of controller data stored in data for success (0 if none is stored). Returns an error code for failures. 
				 * @details This function obtains controller data newly held by PadContext in the last PadContext::update() call. Obtained controller data is stored in the data array in order from newest to oldest with the value specified in length as the upper limit. 
				 * @details When the return value is 0, this indicates that there is no controller data newly held by PadContext for the PadContext::update() call. 
				 * @~Japanese
				 * 
				 * @brief 最後のPadContext::update()の呼び出しでPadContextが新規に保持したコントローラーデータを取得する関数 
				 * 
				 * @param data PadData構造体の書き込み先の配列
				 * @param length data配列の要素数（0～64）
				 * @return 成功時はコントローラーデータがdataに格納された個数（格納されなかった場合は0）。失敗時はエラーコード。 
				 * @details 最後のPadContext::update()の呼び出しでPadContextが新規に保持したコントローラーデータを取得する関数です。取得したコントローラーデータを最新のものから順に、lengthで指定された値を上限としてdata配列に格納します。 
				 * @details 返り値が0だった場合、PadContext::update()呼び出しでPadContextが新規に保持したコントローラーデータがなかったことを表します。 
				 */
				int getData(PadData *data, uint32_t length) const;

				/*!
				 * @~English
				 * 
				 * @brief Function to obtain controller data newly held by PadContext in the PadContext::update() call preceding the last PadContext::update() call 
				 * 
				 * @param data PadData structure write destination array. This function stores controller data obtained with the PadContext::update() call preceding the last PadContext::update() call in the data array in order from newest to oldest.
				 * @param length Length of the data array
				 * @return Returns the number of padData written in the PadContext::update() call preceding the last PadContext::update() call for success. When length is larger than the number of controller data newly held by PadContext, returns the number of controller data newly held by PadContext. 
				 * @details This function obtains controller data newly held by PadContext in the PadContext::update() call preceding the last PadContext::update() call. Controller data obtained with the PadContext::update() call preceding the last PadContext::update() call is stored in the data array in order from newest to oldest. When 1 is specified to length, only the latest controller data obtained with the PadContext::update() call preceding the last PadContext::update() call will be stored in data. When value specified to length is larger than the number of controller data newly held by PadContext, this function stores all the controller data newly held by PadContext with the PadContext::update() call preceding the last PadContext::update() call. When the value specified to length is less than the number of controller data newly held by PadContext, this function stores length-number of controller data. 
				 * @~Japanese
				 * 
				 * @brief 最後から一つ前のPadContext::update()の呼び出しでPadContextが保持したコントローラーデータを取得する関数 
				 * 
				 * @param data PadData構造体の書き込み先の配列。最後から一つ前のPadContext::update()の呼び出しで取得したコントローラーデータの中の最新のものから順に配列に格納。
				 * @param length dataの配列の長さ
				 * @return 成功時は最後から一つ前のPadContext::update()の呼び出しで書きこまれたpadDataの数。lengthがPadContextが新規に保持したコントローラーデータの数よりも大きかった場合、PadContextが新規に保持したコントローラーデータの数を返す。 
				 * @details 最後から一つ前のPadContext::update()の呼び出しでPadContextが新規に保持したコントローラーデータを取得する関数です。最後から一つ前のPadContext::update()の呼び出しで取得したコントローラーデータをその中での最新のものから順にdata配列に格納します。lengthに1を指定した場合dataには最後から一つ前のPadContext::update()の呼び出しで得た中での最新のコントローラーデータのみ格納します。lengthに指定した値がPadContextが新規に保持したコントローラーデータの数よりも大きかった場合、最後から一つ前のPadContext::update()の呼び出しでPadContextが保持したコントローラーデータの数分格納し、lengthに指定した値がPadContextが新規に保持したコントローラーデータの数よりも小さかった場合、length分コントローラーデータを格納します。 
				 */
				int getPreviousUpdateData(PadData *data, uint32_t length) const;

				/*!
				 * @~English
				 * 
				 * @brief Enables/disables motion sensor 
				 * 
				 * @param bEnable Enables or disables. Enabled if it is true. The default is true
				 * @return Returns SCE_OK for success. Returns an error code for failures. 
				 * @details Enables/disables motion sensor. Valid values will be stored in the orientation, acceleration, and angularVelocity members of MotionData of PadData. 
				 * @~Japanese
				 * 
				 * @brief モーションセンサーの有効・無効を設定する 
				 * 
				 * @param bEnable 有効・無効の指定。trueの場合有効。デフォルトはtrue
				 * @return 成功時はSCE_OK。失敗時はエラーコード。 
				 * @details モーションセンサーの有効・無効を設定します。PadDataのMotionDataのorientation, acceleration, angularVelocityメンバに有効な値が格納されるようになります。 
				 */
				int enableMotionSensor(bool bEnable);

				/*!
				 * @~English
				 * 
				 * @brief Enables or disables tilt correction. 
				 * 
				 * @param bEnable Enables or disables. Enabled if it is true. The default is false
				 * @return Returns SCE_OK for success. Returns an error code for failures. 
				 * @details Enables/disables the feature for correcting the controller orientation data based on the acceleration sensor. 
				 * @details When tilt correction is enabled, tilt correction will apply to the orientation data to be stored in the orientation member of MotionData of PadData. 
				 * @~Japanese
				 * 
				 * @brief 傾き補正の有効・無効を設定する 
				 * 
				 * @param bEnable 有効・無効の指定。trueの場合有効。デフォルトはfalse
				 * @return 成功時はSCE_OK。失敗時はエラーコード。 
				 * @details コントローラー姿勢データを加速度センサーに基づいて補正する機能の有効・無効を設定します。 
				 * @details 傾き補正を有効にすると、PadDataのMotionDataのorientationメンバに格納される姿勢データに対して傾き補正が適用されます。 
				 */
				int enableTiltCorrection(bool bEnable);
				/*!
				 * @~English
				 * 
				 * @brief Enables/disables the deadband filter of the angular velocity sensor 
				 * 
				 * @param bEnable Enables or disables. Enabled if it is true. The default is false
				 * @return Returns SCE_OK for success. Returns an error code for failures. 
				 * @details Enables/disables the deadband filter of the angular velocity sensor (gyro sensor). 
				 * @details When the deadband filter is enabled, the orientation and angularVelocity members of MotionData of PadData will be calculated based on the angular velocity value after noise removal with the deadband filter. 
				 * @~Japanese
				 * 
				 * @brief 角速度センサーのデッドバンドフィルタの有効・無効を設定する 
				 * 
				 * @param bEnable 有効・無効の指定。trueの場合有効。デフォルトはfalse
				 * @return 成功時はSCE_OK。失敗時はエラーコード。 
				 * @details 角速度センサー(ジャイロセンサー)のデッドバンドフィルタの有効・無効を設定します。 
				 * @details デッドバンドフィルタを有効にすると、PadDataのMotionDataのorientationおよびangularVelocityメンバは、デッドバンドフィルタによってノイズを取り除いた角速度値に基づいて算出されるようになります。 
				 */
				int enableAngularVelocityDeadband(bool bEnable);
				/*!
				 * @~English
				 * 
				 * @brief Enables/disables motion sensor 
				 * 
				 * @return Returns SCE_OK for success. Returns an error code for failures. 
				 * @details This function obtains the controller data array currently held by PadContext. This is the buffer count designated at the time of initialization for maximum array length. 
				 * @~Japanese
				 * 
				 * @brief モーションセンサーの有効・無効を設定する 
				 * 
				 * @return 成功時はSCE_OK。失敗時はエラーコード。 
				 * @details PadContextが現在保持しているコントローラーデータの配列を取得するための関数です。配列の長さの最大は初期化時に指定したバッファ数です。 
				 */
				int resetOrientation();
				/*!
				 * @~English
				 * 
				 * @brief Gets the default dead zones of the left stick 
				 * 
				 * @return Returns SCE_OK for success. Returns an error code for failures. 
				 * @details This gets default dead zones of the left stick. 
				 * @~Japanese
				 * 
				 * @brief デフォルトの左スティックのデッドゾーンの取得 
				 * 
				 * @return 成功時はSCE_OK。失敗時はエラーコード。 
				 * @details デフォルトの左スティックのデッドゾーンを取得します。 
				 */
				float getDefaultLeftAnalogStickDeadZone();
				/*!
				 * @~English
				 * 
				 * @brief Gets the default dead zones of the right stick 
				 * 
				 * @return Returns SCE_OK for success. Returns an error code for failures. 
				 * @details This gets default dead zones of the right stick. 
				 * @~Japanese
				 * 
				 * @brief デフォルトの右スティックのデッドゾーンの取得 
				 * 
				 * @return 成功時はSCE_OK。失敗時はエラーコード。 
				 * @details デフォルトの右スティックのデッドゾーンを取得します。 
				 */
				float getDefaultRightAnalogStickDeadZone();

				/*!
				 * @~English
				 * 
				 * @brief Control controller vibration 
				 * 
				 * @param largeMotorSpeed Large motor speed (0.0 to 1.0)
				 * @param smallMotorSpeed Small motor speed (0.0 to 1.0)
				 * @param vibrationTimeInMs Vibration time (0 to 2500 milliseconds)
				 * @return Returns SCE_OK for success. Returns an error code for failures. 
				 * @details This function starts (or stops) the vibration of the controller by setting the rotation of the large motor and small motor in the controller. 
				 * @details For both the large motor and the small motor, the motor is stopped when the speed value is 0.0, the speed increases the larger the value is above 0.0, and 1.0 is the maximum speed. Various vibration patterns can be created using speed combinations. 
				 * @details This function will not return an error even when a controller is not connected, but the settings made when a controller is not connected will be ignored. 
				 * @details When the motor speed is not set from 0.0 to 1.0, an error (SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM) will return. 
				 * @details When the vibration time is not set from 0 to 2500, an error (SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM) will return. 
				 * @details In order to stop vibration on time, an update() function must be called at regular intervals. 
				 * @details If an update() function is not called, vibration will be automatically stopped according to the Pad library specifications. 
				 * @details For details, refer to "scePadSetVibration" in "Pad Library Reference". 
				 * @~Japanese
				 * 
				 * @brief コントローラーの振動を制御する 
				 * 
				 * @param largeMotorSpeed 大モータの速度(0.0～1.0)
				 * @param smallMotorSpeed 小モータの速度(0.0～1.0)
				 * @param vibrationTimeInMs 振動させる時間(0～2500ミリ秒)
				 * @return 成功時はSCE_OK。失敗時はエラーコード。 
				 * @details コントローラーに内蔵された大モータおよび小モータを回転させることによってコントローラーを振動させます（または振動を停止させます）。 
				 * @details 大モータ、小モータともに、速度の値が0.0のとき停止、0.0より値が大きくなるほど回転が速くなり、1.0で最大速度となります。速度の組み合わせにより、さまざまな振動を作り出すことができます。 
				 * @details 本関数はコントローラー未接続状態にあってもエラーは返しませんが、未接続状態で行った設定は無視されます。 
				 * @details モータの速度を0.0～1.0で指定しない場合はエラー(SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM)が返ります。 
				 * @details 振動させる時間を0～2500ミリ秒で指定しない場合はエラー(SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM)が返ります。 
				 * @details 時間通りに振動を停止させるために定期的にupdate()関数を呼ぶ必要があります。 
				 * @details update()関数が呼ばれない場合はPadライブラリの仕様により一定時間で振動が自動停止します。 
				 * @details 詳細は、「Padライブラリ リファレンス」の「scePadSetVibration」を参照してください。 
				 */
				int setVibration(float largeMotorSpeed, float smallMotorSpeed, uint32_t vibrationTimeInMs = 2500);

				/*!
				 * @~English
				 * 
				 * @brief Sets the light bar color 
				 * 
				 * @param pParam Light bar color setting parameters
				 * @return Returns SCE_OK for success. Returns an error code for failures. 
				 * @details The light bar color can be set to RGB (Red/Green/Blue) three color combinations with ranges of 0-255. 
				 * @details However, turning off the light bar is prohibited. 
				 * @details If the setting range is less than 5% (setting value 13) of the maximum value (largest value for each setting value) for the R/G/B settings in this API, an error (SCE_PAD_ERROR_INVALID_COLOR_SETTINGS) will return. 
				 * @~Japanese
				 * 
				 * @brief ライトバーの色を設定する 
				 * 
				 * @param pParam ライトバーの色設定パラメータ
				 * @return 成功時はSCE_OK。失敗時はエラーコード。 
				 * @details ライトバーの色をRGB(Red/Green/Blue)三色の組み合わせで0-255の範囲で設定することができます。 
				 * @details ただし、ライトバーを消灯することは禁止されています。 
				 * @details 本APIでR/G/B設定の最大値(各設定値の中で一番大きな値)が、設定レンジの5%(設定値13)を下回る場合はエラー(SCE_PAD_ERROR_INVALID_COLOR_SETTINGS)が返ります。 
				 */
				int setLightBar(const ScePadLightBarParam *pParam);

				/*!
				 * @~English
				 * 
				 * @brief Reset light bar color setting 
				 * 
				 * @return Returns SCE_OK for success. Returns an error code for failures. 
				 * @details This function resets the light bar color setting value that was set with setLightBar() and returns the setting to the user color for the user linked to the controller (the color assigned to the user by the Prospero system). 
				 * @details Regarding user colors on PlayStation(R)4, refer to "User Management Overview" document. 
				 * @~Japanese
				 * 
				 * @brief ライトバーの色設定をリセットする 
				 * 
				 * @return 成功時はSCE_OK。失敗時はエラーコード。 
				 * @details setLightBar()で設定したライトバーの色設定値をリセットし、コントローラーに紐づいているユーザーのユーザーカラー（Prosperoにおいてシステムがユーザーに割り当てた色）に戻す関数です。 
				 * @details PlayStation(R)4におけるユーザーカラーについては「ユーザー管理 概要」ドキュメントを参照してください。 
				 */
				int resetLightBar();

				/*!
				 * @~English
				 * 
				 * @brief Get the controller handle 
				 * 
				 * @return Returns SCE_OK for success. Returns an error code for failures. 
				 * @details This function returns the handle for the port by which to obtain the controller data. 
				 * @~Japanese
				 * 
				 * @brief コントローラーのハンドルの取得 
				 * 
				 * @return 成功時はSCE_OK。失敗時はエラーコード。 
				 * @details コントローラーのデータを取得するポートのハンドルを返します。 
				 */
				int getPadHandle();
				/*!
				 * @~English
				 * 
				 * @brief Gets the touch pad resolution 
				 * 
				 * @return Returns SCE_OK for success. Returns an error code for failures. 
				 * @details This obtains the touch pad resolution. 
				 * @~Japanese
				 * 
				 * @brief タッチパッドの解像度の取得 
				 * 
				 * @return 成功時はSCE_OK。失敗時はエラーコード。 
				 * @details タッチパッドの解像度を取得します。 
				 */
				float getTouchPadPixelDensity();
				/*!
				 * @~English
				 * 
				 * @brief Gets the touch pad aspect ratio 
				 * 
				 * @return Returns SCE_OK for success. Returns an error code for failures. 
				 * @details This gets the touch pad aspect ratio (the touch pad width/touch pad height). 
				 * @~Japanese
				 * 
				 * @brief タッチパッドのアスペクト比の取得 
				 * 
				 * @return 成功時はSCE_OK。失敗時はエラーコード。 
				 * @details タッチパッドのアスペクト比（タッチパッドの横幅/タッチパッドの高さ）を取得します。 
				 */
				float getTouchPadAspectRatio();
				/*!
				 * @~English
				 * 
				 * @brief Get the controller connection type 
				 * 
				 * @return Returns the controller connection type for success. Returns an error code for failures. 
				 * @details This function obtains the controller connection type. PadContext::update() must be called before this function is used. 
				 * @~Japanese
				 * 
				 * @brief コントローラーの接続のタイプの取得 
				 * 
				 * @return 成功時はコントローラーの接続のタイプ。失敗時はエラーコード。 
				 * @details コントローラーの接続のタイプを取得します。本関数を使用する前にPadContext::update()を呼んでいる必要があります。 
				 */
				int getConnectionType();

				/*!
				 * @~English
				 * 
				 * @brief Get the pressed digital button data
				 * 
				 * @param pressedButtons Pressed digital button data
				 * @return Return SCE_OK for success. Retuens an error code for failures.
				 * @details This function obtains the data of the digital buttons that was pressed on last update(). PadContext::update() must be called before this function is used. 
				 *
				 * @~Japanese
				 * 
				 * @brief 新たに押下されたボタンを取得する関数 
				 * 
				 * @param pressedButtons 新たに押下されたボタンデータ
				 * @return 成功時SCE_OK、失敗時はエラーコード。 
				 * @details 新たに押下されたボタンデータが取得できます。本関数を使用する前にPadContext::update()を呼んでいる必要があります。 
				 */
				int getPressedButtons(Button *pressedButtons);

				/*!
				 * @~English
				 * 
				 * @brief Get the released digital button data
				 * 
				 * @param releasedButtons Released digital button data
				 * @return Return SCE_OK for success. Retuens an error code for failures.
				 * @details This function obtains the data of the digital buttons that was released on last update(). PadContext::update() must be called before this function is used. 
				 *
				 * @~Japanese
				 * 
				 * @brief 新たに開放されたボタンを取得する関数 
				 * 
				 * @param releasedButtons 新たに開放されたボタンデータ
				 * @return 成功時SCE_OK、失敗時はエラーコード。 
				 * @details 新たに開放されたボタンデータが取得できます。本関数を使用する前にPadContext::update()を呼んでいる必要があります。 
				 */
				int getReleasedButtons(Button *releasedButtons);


				/*!
				 * @~English
				 * @brief Initializes pad
				 * @param userId User id of user whose pad is initialized
				 * @param type Pad type
				 * @param index Specify 0
				 * @param option Pad context option(default value is used when nullptr is specified)
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief Padの初期化
				 * @param userId Padを初期化するユーザID
				 * @param type Padタイプ
				 * @param index 0を指定
				 * @param option Padコンテキストのオプション(nullptr指定時は既定値が使用される)
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int initialize(System::UserId userId, int32_t type, int32_t index, const PadContextOption* option = nullptr);
				/*!
				 * @~English
				 * @brief Finalizes pad
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief Padの終了処理
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int finalize();

				/*!
				 * @~English
				 * @brief Obtains Pad user ID
				 * @return Pad user ID
				 * @~Japanese
				 * @brief PadのユーザIDの取得
				 * @return PadのユーザIDの
				 */
				System::UserId getUserId();
				/*!
				 * @~English
				 * @brief Obtains Pad type
				 * @return Pad type
				 * @~Japanese
				 * @brief Padタイプの取得
				 * @return Padタイプ
				 */
				int32_t getType();
				/*!
				 * @~English
				 * @brief Obtains Pad index
				 * @return Pad index
				 * @~Japanese
				 * @brief Padインデックスの取得
				 * @return Padインデックス
				 */
				int32_t getIndex();

			protected:
				static const float	m_recipMaxByteAsFloat;

				bool				m_isRightAnalogStickDeadZoneUpdated;
				bool				m_isLeftAnalogStickDeadZoneUpdated;
				float				m_rightAnalogStickDeadZone;
				float				m_leftAnalogStickDeadZone;
				float				m_defaultRightAnalogStickDeadZone;
				float				m_defaultLeftAnalogStickDeadZone;
				PadData				*m_currentPadData;
				PadData				*m_previousPadData;
				int					m_numPadData;
				int					m_numCurrentPadData;
				int					m_numPreviousPadData;
				uint32_t			m_pressedButtonData;
				uint32_t			m_releasedButtonData;
				System::UserId		m_userId;
				int32_t				m_type;
				int32_t				m_index;

				sce::Vectormath::Simd::Aos::Vector2 m_leftStickValue;
				sce::Vectormath::Simd::Aos::Vector2 m_rightStickValue;

			private:
				void				*m_currentData;
				SceKernelTimespec	m_previousTime;
				float				m_touchPadPixelDensity;
				float				m_touchPadAspectRatio;
				int					m_connectionType;
				int					m_currentConnectedCount;
				int					m_touchPadWidth;
				int					m_touchPadHeight;
				int32_t				m_handle;
				bool				m_isTimeSpecifiedVibing;
				int					m_vibrationTimeInMs;
				uint64_t			m_startVibrationTime;

				int updatePadData(Input::PadData	*pScriptPadData = nullptr);
				int open(System::UserId userId, int32_t type, int32_t index, const ScePadOpenParam* pParam);
#if _SCE_TARGET_OS_PROSPERO
				int setScePadDataForUserLogoutCase(ScePadVrControllerData* data);
#endif
				int setScePadDataForUserLogoutCase(ScePadData* data);
			};
		}
	}
}
