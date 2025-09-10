/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#ifndef _SCE_SAMPLE_UTIL_INPUT_TOUCH_H
#define _SCE_SAMPLE_UTIL_INPUT_TOUCH_H


#include <scebase_common.h>
#include <sampleutil/sampleutil_common.h>
#include <vectormath.h>

#ifdef __psp2__
#include <touch.h>
#include <systemgesture.h>
#endif

namespace sce
{
	namespace SampleUtil
	{
		namespace Input
		{


#if _SCE_TARGET_OS_PSP2

			/*!
			 * @~English
			 * @brief Structure for setting TouchContext 
			 * @details This is the structure to handle the set value of TouchContext. Used for the argument used when the initialize of TouchContext is executed. 
			 * @~Japanese
			 * @brief TouchContext設定用の構造体 
			 * @details TouchContextの設定値を扱う構造体です。TouchContextのinitialize実行時の引数で使用します。 
			 */
			struct TouchContextOption
			{
				/*!
				 * @~English
				 * @brief Number of buffers to receive touch input data 
				 * @details This sets the number of buffers to receive touch input data for each touch port. The initial values of SCE_TOUCH_PORT_FRONT and SCE_TOUCH_PORT_BACK are 1 and 0, respectively. For details on touch input data, refer to the "Touch Service Reference" document. 
				 * @~Japanese
				 * @brief タッチ入力データを受け取るバッファの数 
				 * @details タッチ入力データを受け取るバッファの数を、タッチポート毎に設定します。初期値は、SCE_TOUCH_PORT_FRONTが1、SCE_TOUCH_PORT_BACKが0です。タッチ入力データについての詳細は、「タッチサービス リファレンス」を参照してください。 
				 */
				uint32_t	numBufs[SCE_TOUCH_PORT_MAX_NUM];

			public:
				TouchContextOption()
				{
					numBufs[SCE_TOUCH_PORT_FRONT]	= 1;
					numBufs[SCE_TOUCH_PORT_BACK]	= 1;
				}
			};

			/*!
			 * @~English
			 * 
			 * @brief The touch port enum value 
			 * @details This is used for the designation of the touch panel port. 
			 * @~Japanese
			 * 
			 * @brief タッチポートのenum値 
			 * @details タッチパネルのポート指定に使用します。 
			 */
			enum TouchPort
			{
				/*!
				 * @~English
				 * @brief Front touch panel 
				 * @~Japanese
				 * @brief 前面タッチパネル 
				 */
				kTouchPortFront = 0,

				/*!
				 * @~English
				 * @brief Back touch panel 
				 * @~Japanese
				 * @brief 背面タッチパネル 
				 */
				kTouchPortBack			= 1
			};

			/*!
			 * @~English
			 * 
			 * @brief Function flag enum value 
			 * @details This is used for the designation of the function flag. 
			 * @~Japanese
			 * 
			 * @brief ファンクションフラグのenum値 
			 * @details ファンクションフラグの指定に使用します。 
			 */
			enum TouchContextFunctionFlag
			{
				/*!
				 * @~English
				 * @brief Tap gesture event 
				 * @~Japanese
				 * @brief タップジェスチャイベント 
				 */
				kTouchContextFunctionFlagTapGestureEvent				= 0x00000001,

				/*!
				 * @~English
				 * @brief Drag gesture event 
				 * @~Japanese
				 * @brief ドラッグジェスチャイベント 
				 */
				kTouchContextFunctionFlagDragGestureEvent				= 0x00000002,

				/*!
				 * @~English
				 * @brief Tap and hold gesture event 
				 * @~Japanese
				 * @brief タップアンドホールドジェスチャイベント 
				 */
				kTouchContextFunctionFlagTapAndHoldGestureEvent		= 0x00000004,

				/*!
				 * @~English
				 * @brief Pinch out-in gesture event 
				 * @~Japanese
				 * @brief ピンチアウトインジェスチャイベント 
				 */
				kTouchContextFunctionFlagPinchOutInGestureEvent		= 0x00000008,

				/*!
				 * @~English
				 * @brief Rotation gesture event 
				 * @~Japanese
				 * @brief ローテーションジェスチャイベント 
				 */
				kTouchContextFunctionFlagRotationGestureEvent			= 0x00000010,

				/*!
				 * @~English
				 * @brief Primitive gesture event 
				 * @~Japanese
				 * @brief プリミティブジェスチャイベント 
				 */
				kTouchContextFunctionFlagPrimitiveGestureEvent			= 0x40000000,

				/*!
				 * @~English
				 * @brief Touch data 
				 * @~Japanese
				 * @brief タッチデータ 
				 */
				kTouchContextFunctionFlagRawData						= 0x80000000
			};

			/*!
			 * @~English
			 * @brief Structure for getting the primitive gesture event 
			 * @details This is the structure used for getting the primitive gesture event data. 
			 * @~Japanese
			 * @brief プリミティブジェスチャイベント取得用の構造体 
			 * @details プリミティブジェスチャイベントデータの取得に使用する構造体です。 
			 */
			struct TouchPrimitiveGestureEvent
			{
				/*!
				 * @~English
				 * @brief Touch panel port 
				 * @~Japanese
				 * @brief タッチパネルのポート 
				 */
				TouchPort  port;

				/*!
				 * @~English
				 * @brief Primitive gesture event structure For details, refer to the "libsystemgesture Reference" document. 
				 * @~Japanese
				 * @brief プリミティブジェスチャイベント構造体。詳細は「libsystemgesture リファレンス」を参照してください。 
				 */
				SceSystemGesturePrimitiveTouchEvent  event;
			};

			/*!
			 * @~English
			 * @brief Structure for getting the gesture event 
			 * @details This is the structure used for getting the gesture event data. 
			 * @~Japanese
			 * @brief ジェスチャイベント取得用の構造体 
			 * @details ジェスチャイベントデータの取得に使用する構造体です。 
			 */
			struct TouchGestureEvent
			{
				/*!
				 * @~English
				 * @brief Touch panel port 
				 * @~Japanese
				 * @brief タッチパネルのポート 
				 */
				TouchPort  port;

				/*!
				 * @~English
				 * @brief Gesture event structure For details, refer to the "libsystemgesture Reference" document. 
				 * @~Japanese
				 * @brief ジェスチャイベント構造体。詳細は「libsystemgesture リファレンス」を参照してください。 
				 */
				SceSystemGestureTouchEvent  event;
			};

			/*!
			 * @~English
			 * @brief Class to handle input data from the touch panel 
			 * @details This reads input data from the touch panel and recognizes the consecutive touch operations. For details on touch input data, refer to the "Touch Service Reference" document. For details on consecutive touch operations, refer to the "libsystemgesture Reference" document. 
			 * @~Japanese
			 * @brief タッチパネルからの入力データを扱うクラス 
			 * @details タッチパネルからの入力データ読み取り、および連続的なタッチ操作の認識を行います。タッチ入力データについての詳細は、「タッチサービス リファレンス」を参照してください。連続的なタッチ操作についての詳細は、「libsystemgesture リファレンス」を参照してください。 
			 */
			class SCE_SAMPLE_UTIL_API TouchContext : public Resource
			{
			public:
				virtual ~TouchContext(void){}

				/*!
				 * @~English
				 * @brief Updates the TouchContext class. 
				 * 
				 * @retval SCE_OK Success
				 * @retval SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED TouchContext is not initialized
				 * @retval SCE_SAMPLE_UTIL_ERROR_NO_RAW_DATA_BUFFER No touch input data buffer
				 * @retval <SCE_OK Failure (Error codes of libsystemgesture and touch service)
				 * @details This updates the TouchContext class. 
				 * @~Japanese
				 * @brief TouchContextクラスの更新 
				 * 
				 * @retval SCE_OK 成功
				 * @retval SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED TouchContextが初期化されていない
				 * @retval SCE_SAMPLE_UTIL_ERROR_NO_RAW_DATA_BUFFER タッチ入力データバッファが無い
				 * @retval <SCE_OK 失敗（libsystemgestureとタッチサービスのエラーコード）
				 * @details TouchContextクラスを更新します。 
				 */
				virtual int update(void) = 0; 

				/*!
				 * @~English
				 * @brief Function flag settings 
				 * 
				 * @param port Touch panel port number
				 * @param functionFlag Function flag
				 * 
				 * @retval SCE_OK Success
				 * @retval SCE_SAMPLE_UTIL_ERROR_NO_RAW_DATA_BUFFER No touch input data buffer
				 * @retval SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED TouchContext is not initialized
				 * @retval <SCE_OK Failure (Error codes of touch service)
				 * @details This is for enabling or disabling each function for the TouchContext. The argument functionFlag is treated as a bit pattern. For functionFlag specification, add the required items from the TouchContextFunctionFlag definition with an OR operation. 
				 * @~Japanese
				 * @brief ファンクションフラグの設定 
				 * 
				 * @param port タッチパネルポート番号
				 * @param functionFlag ファンクションフラグ
				 * 
				 * @retval SCE_OK 成功
				 * @retval SCE_SAMPLE_UTIL_ERROR_NO_RAW_DATA_BUFFER タッチ入力データバッファが無い
				 * @retval SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED TouchContextが初期化されていない
				 * @retval <SCE_OK 失敗（タッチサービスのエラーコード）
				 * @details TouchContextの各機能について、機能の有効／無効の設定を行います。引数functionFlagはビットパターンとして扱われます。functionFlagの指定には、TouchContextFunctionFlagの定義から必要なものを論理和で追加してください。 
				 */
				virtual int setFunctionFlag( const TouchPort port, const uint32_t functionFlag ) = 0;

				/*!
				 * @~English
				 * @brief Gets the function flag. 
				 * 
				 * @param port Touch panel port number
				 * @retval uint32_t Function flag
				 * @details This gets the current function flag set values. 
				 * @~Japanese
				 * @brief ファンクションフラグの取得 
				 * 
				 * @param port タッチパネルポート番号
				 * @retval uint32_t ファンクションフラグ
				 * @details 現在のファンクションフラグ設定値を取得します。 
				 */
				virtual uint32_t getFunctionFlag( const TouchPort port ) const = 0;

				/*!
				 * @~English
				 * @brief Gets touch panel input data. 
				 * @param port Touch panel port number
				 * @param touchData Pointer to the touch panel input data structure
				 * @param size Number of touch panel input data items received
				 * 
				 * @retval 0 or greater Number of touch panel input data items obtained (success)
				 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
				 * @retval SCE_SAMPLE_UTIL_ERROR_NO_RAW_DATA_BUFFER No touch input data buffer
				 * @retval SCE_SAMPLE_UTIL_ERROR_FUNCTION_NOT_ENABLED Function flag is not enabled
				 * @details This gets the data input from touch panel. For details on SceTouchData, refer to the "Touch Service Reference" document. 
				 * @~Japanese
				 * @brief タッチパネル入力データの取得 
				 * @param port タッチパネルポート番号
				 * @param touchData タッチパネル入力データ構造体へのポインタ
				 * @param size タッチパネル入力データを受け取る数
				 * 
				 * @retval 0以上 取得したタッチパネル入力データの数（成功）
				 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
				 * @retval SCE_SAMPLE_UTIL_ERROR_NO_RAW_DATA_BUFFER タッチ入力データバッファが無い
				 * @retval SCE_SAMPLE_UTIL_ERROR_FUNCTION_NOT_ENABLED ファンクションフラグが有効では無い
				 * @details タッチパネルから入力されたデータを取得します。SceTouchDataについての詳細は、「タッチサービス リファレンス」を参照してください。 
				 */
				virtual int getRawData(const TouchPort port, SceTouchData* touchData, const uint32_t size ) const = 0;

				/*!
				 * @~English
				 * @brief Gets touch panel information. 
				 * @param port Touch panel port number
				 * @param panelInfo Pointer to SceTouchPanelInfo
				 * 
				 * @retval SCE_OK Success
				 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
				 * @details This gets touch panel information. For details on SceTouchPanelInfo, refer to the "Touch Service Reference" document. 
				 * @~Japanese
				 * @brief タッチパネル情報の取得 
				 * @param port タッチパネルポート番号
				 * @param panelInfo SceTouchPanelInfoへのポインタ
				 * 
				 * @retval SCE_OK 成功
				 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
				 * @details タッチパネル情報を取得します。SceTouchPanelInfoについての詳細は、「タッチサービス リファレンス」を参照してください。 
				 */
				virtual int getPanelInfo(const TouchPort port, SceTouchPanelInfo* panelInfo ) const = 0;

				/*!
				 * @~English
				 * @brief Gets the primitive gesture event count. 
				 * 
				 * @retval 0 or greater Primitive gesture event count (success)
				 * @retval SCE_SAMPLE_UTIL_ERROR_FUNCTION_NOT_ENABLED Function flag is not enabled
				 * @details This gets the total number of primitive gesture events. For details on primitive gesture events, refer to the "libsystemgesture Reference" document. 
				 * @~Japanese
				 * @brief プリミティブジェスチャイベント数の取得 
				 * 
				 * @retval 0以上 プリミティブジェスチャイベント数（成功）
				 * @retval SCE_SAMPLE_UTIL_ERROR_FUNCTION_NOT_ENABLED ファンクションフラグが有効では無い
				 * @details プリミティブジェスチャイベントの総数を取得します。プリミティブジェスチャイベントについての詳細は、「libsystemgesture リファレンス」を参照してください。 
				 */
				virtual int getNumberOfPrimitiveGestureEvents() const = 0;

				/*!
				 * @~English
				 * @brief Gets the primitive gesture event. 
				 * @param events Pointer to a primitive gesture event structure
				 * @param size Number of primitive gesture events received
				 * 
				 * @retval 0 or greater Primitive gesture event count obtained (success)
				 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
				 * @retval SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED TouchContext is not initialized
				 * @retval SCE_SAMPLE_UTIL_ERROR_FUNCTION_NOT_ENABLED Function flag is not enabled
				 * @details This gets the primitive gesture events. For details on primitive gesture events, refer to the "libsystemgesture Reference" document. 
				 * @~Japanese
				 * @brief プリミティブジェスチャイベントの取得 
				 * @param events プリミティブジェスチャイベント構造体へのポインタ
				 * @param size プリミティブジェスチャイベントを受け取る数
				 * 
				 * @retval 0以上 取得したプリミティブジェスチャイベントの数（成功）
				 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
				 * @retval SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED TouchContextが初期化されていない
				 * @retval SCE_SAMPLE_UTIL_ERROR_FUNCTION_NOT_ENABLED ファンクションフラグが有効では無い
				 * @details プリミティブジェスチャイベントを取得します。プリミティブジェスチャイベントについての詳細は、「libsystemgesture リファレンス」を参照してください。 
				 */
				virtual int getPrimitiveGestureEvents( TouchPrimitiveGestureEvent* events, const uint32_t size ) const = 0;

				/*!
				 * @~English
				 * @brief Gets the gesture event count. 
				 * 
				 * @retval 0 or greater Gesture event count (success)
				 * @retval SCE_SAMPLE_UTIL_ERROR_FUNCTION_NOT_ENABLED Function flag is not enabled
				 * @details This gets the total number of gesture events. For details on gesture events, refer to the "libsystemgesture Reference" document. 
				 * @~Japanese
				 * @brief ジェスチャイベント数の取得 
				 * 
				 * @retval 0以上 ジェスチャイベント数（成功）
				 * @retval SCE_SAMPLE_UTIL_ERROR_FUNCTION_NOT_ENABLED ファンクションフラグが有効では無い
				 * @details ジェスチャイベントの総数を取得します。ジェスチャイベントについての詳細は、「libsystemgesture リファレンス」を参照してください。 
				 */
				virtual int getNumberOfGestureEvents() const = 0;

				/*!
				 * @~English
				 * @brief Gets gesture events. 
				 * @param events Pointer to a gesture event structure
				 * @param size Number of gesture events received
				 * 
				 * @retval 0 or greater Gesture event count obtained (success)
				 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Parameter invalid
				 * @retval SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED TouchContext is not initialized
				 * @retval SCE_SAMPLE_UTIL_ERROR_FUNCTION_NOT_ENABLED Function flag is not enabled
				 * @details This gets gesture events. For details on gesture events, refer to the "libsystemgesture Reference" document. 
				 * @~Japanese
				 * @brief ジェスチャイベントの取得 
				 * @param events ジェスチャイベント構造体へのポインタ
				 * @param size ジェスチャイベントを受け取る数
				 * 
				 * @retval 0以上 取得したジェスチャイベントの数（成功）
				 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM パラメータ不正
				 * @retval SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED TouchContextが初期化されていない
				 * @retval SCE_SAMPLE_UTIL_ERROR_FUNCTION_NOT_ENABLED ファンクションフラグが有効では無い
				 * @details ジェスチャイベントを取得します。ジェスチャイベントについての詳細は、「libsystemgesture リファレンス」を参照してください。 
				 */
				virtual int getGestureEvents( TouchGestureEvent* events, const uint32_t size ) const = 0;

			};

			/*!
			 * @~English
			 * @brief Generates TouchContext. 
			 * @param outTouchContext Pointer to which the generated TouchContext returns
			 * @param option Pointer to TouchContextOption. This is initialized by the default value if NULL is specified.
			 * 
			 * @retval SCE_OK Success
			 * @retval <SCE_OK Failure
			 * @details This executes the initialization processing of the TouchContext class. If the argument option is omitted, all processing will be executed with the initial set values. 
			 * @~Japanese
			 * @brief TouchContextを生成 
			 * @param outTouchContext 生成されたTouchContextが返るポインタ
			 * @param option TouchContextOptionのポインタ。NULLを指定するとデフォルトの値で初期化されます。
			 * 
			 * @retval SCE_OK 成功
			 * @retval <SCE_OK 失敗
			 * @details TouchContextクラスの初期化処理を行います。引数optionが省略された場合は、全て初期設定値で行います。 
			 */
			SCE_SAMPLE_UTIL_API int createTouchContext(TouchContext **outTouchContext, const TouchContextOption* option=NULL);

#endif /* _SCE_TARGET_OS_PSP2 */
#if _SCE_TARGET_OS_WINDOWS
            class SCE_SAMPLE_UTIL_API TouchContext : public Resource
			{
			public:
				struct PrimitiveTouchEvent {
					int		eventState;
					POINT	currentPosition;
				};

				virtual ~TouchContext(void){}
				virtual int finalize() = 0;
				virtual int update() = 0;

				virtual int getNumberOfPrimitiveTouchEvents() const = 0;
				virtual const PrimitiveTouchEvent* getPrimitiveTouchEvents() const = 0;
			};

			SCE_SAMPLE_UTIL_API int createTouchContext(TouchContext **outTouchContext, HWND hWnd);

#endif /* _SCE_TARGET_OS_WINDOWS */

		}
	}
}


#endif /* _SCE_SAMPLE_UTIL_INPUT_TOUCH_H */

