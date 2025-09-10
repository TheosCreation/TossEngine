/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#ifndef _SCE_SAMPLE_UTIL_INPUT_MOTION_H
#define _SCE_SAMPLE_UTIL_INPUT_MOTION_H


#include <scebase_common.h>
#include <sampleutil/sampleutil_common.h>
#include <vectormath.h>
#ifdef __psp2__
#include <motion.h>
#endif

namespace sce
{
	namespace SampleUtil
	{
		namespace Input
		{
#ifdef __psp2__

			/*!
			 * @~English
			 * 
			 * @brief enum value of the flag for the feature to use with the motion context 
			 * @details This is used to specify the flag for the feature to use with the motion context 
			 * @~Japanese
			 * 
			 * @brief モーションコンテクストで使用する機能のフラグのenum値 
			 * @details モーションコンテクストで使用する機能のフラグの指定に使用します。 
			 */
			enum MotionContextFunctionFlag
			{
				/*!
				 * @~English
				 * 
				 * @brief Accelerometer sensor and gyro sensor flag value 
				 * @~Japanese
				 * 
				 * @brief 加速度センサーおよびジャイロセンサーフラグ値 
				 */
				kMotionContextFunctionTypeAccelerometerAndGyro = 0x00000001,
				/*!
				 * @~English
				 * 
				 * @brief Magnetometer flag value 
				 * @~Japanese
				 * 
				 * @brief 地磁気センサーフラグ値 
				 */
				kMotionContextFunctionTypeMagnetometer= 0x00000002
			};

			/*!
			 * @~English
			 * @brief Structure that handles device states and motion sensor values 
			 * @details This structure handles device states and motion sensor values. For details on device coordinate systems, refer to the "libmotion Overview" document. 
			 * @~Japanese
			 * @brief デバイスの状態やモーションセンサーの値を扱う構造体 
			 * @details デバイスの状態やモーションセンサーの値を扱う構造体です。デバイス座標系の詳細については、「libmotion 概要」ドキュメントを参照してください。 
			 */
			struct MotionContextData
			{
				/*!
				 * @~English
				 * @brief Device timestamp of sensor device (microseconds) 
				 * @~Japanese
				 * @brief センサーデバイスのデバイスタイムスタンプ（μs） 
				 */
				uint32_t								deviceTimestamp;
				/*!
				 * @~English
				 * @brief Timestamp for time when data packet was received from host (microseconds) 
				 * @~Japanese
				 * @brief ホストがデータパケットを受信した時刻のタイムスタンプ（μs） 
				 */
				uint64_t								systemTimestamp;

				/*!
				 * @~English
				 * @brief Device orientation expressed in quaternions <x, y, z, w> 
				 * @~Japanese
				 * @brief クォータニオン<x, y, z, w>で表されたデバイスの方向 
				 */
				sce::Vectormath::Simd::Aos::Quat		relativeOrientation;
				/*!
				 * @~English
				 * @brief Accelerometer reading <x, y, z> (G) 
				 * @~Japanese
				 * @brief 加速度センサーの測定値<x, y, z>（G） 
				 */
				sce::Vectormath::Simd::Aos::Vector3		linearAcceleration;
				/*!
				 * @~English
				 * @brief Gyro sensor reading <x, y, z> (rad/s) 
				 * @~Japanese
				 * @brief ジャイロセンサーの測定値<x, y, z>（rad/s） 
				 */
				sce::Vectormath::Simd::Aos::Vector3		angularVelocity;
				/*!
				 * @~English
				 * @brief Basic device orientation based on gravity<x, y, z> 
				 * @~Japanese
				 * @brief 重力を基準にしたデバイスの簡易的な方向<x, y, z> 
				 */
				sce::Vectormath::Simd::Aos::Vector3		userInterfaceOrientation;

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
			 * @brief Class that handles motion sensor data 
			 * @details MotionContext is a class that handles motion sensor values and device orientation values calculated from the motion sensor. 
			 * @~Japanese
			 * @brief モーションセンサーのデータを扱うクラス 
			 * @details MotionContextはモーションセンサーの値やモーションセンサーから計算されたデバイスの姿勢値を扱うクラスです。 
			 */
			class MotionContext : public Resource
			{
			public:
				virtual ~MotionContext(){}

				/*!
				 * @~English
				 * @brief MotionContext class update 
				 * 
				 * @retval (>=0) Success
				 * @retval (<0) Failure
				 * @details This updates the MotionContext class. 
				 * @~Japanese
				 * @brief MotionContextクラスの更新 
				 * 
				 * @retval (>=0) 成功
				 * @retval (<0) 失敗
				 * @details MotionContextクラスを更新します。 
				 */
				virtual int update() = 0;

				/*!
				 * @~English
				 * @brief Function flag settings 
				 * @param functionFlag Function flag
				 * @retval (>=0) Success
				 * @retval (<0) Failure
				 * @details This is for enabling or disabling each feature for the MotionContext. The argument functionFlag is treated as a bit pattern. For functionFlag specification, add the required items from the MotionContextFunctionFlag definition with an OR operation. When handling the magnetometer, kMotionContextFunctionTypeAccelerometerAndGyro is required in addition to kMotionContextFunctionTypeMagnetometer. 
				 * @~Japanese
				 * @brief ファンクションフラグの設定 
				 * @param functionFlag ファンクションフラグ
				 * @retval (>=0) 成功
				 * @retval (<0) 失敗
				 * @details MotionContextの各機能について、機能の有効／無効の設定を行います。引数functionFlagはビットパターンとして扱われます。functionFlagの指定には、MotionContextFunctionFlagの定義から必要なものを論理和で追加してください。地磁気センサーを扱うときはkMotionContextFunctionTypeMagnetometerだけでなく、kMotionContextFunctionTypeAccelerometerAndGyroも必要です。 
				 */
				virtual int enableFunctions(uint32_t functionFlag) = 0;

				/*!
				 * @~English
				 * @brief Gets the function flag. 
				 * 
				 * @retval uint32_t Function flag
				 * @details This gets the current function flag set values. 
				 * @~Japanese
				 * @brief ファンクションフラグの取得 
				 * 
				 * @retval uint32_t ファンクションフラグ
				 * @details 現在のファンクションフラグ設定値を取得します。 
				 */
				virtual uint32_t getEnabledFunctions() const = 0;

				/*!
				 * @~English
				 * @brief Get current device state and motion sensor data 
				 * @param motionState Pointer to SceMotionState structure
				 * 
				 * @retval (>=0) Success
				 * @retval (<0) Failure
				 * @details This obtains the current motion sensor data and device state calculated from the motion sensor. 
				 * @~Japanese
				 * @brief 現在のデバイスの状態やモーションセンサーのデータの取得 
				 * @param motionState SceMotionState構造体へのポインタ
				 * 
				 * @retval (>=0) 成功
				 * @retval (<0) 失敗
				 * @details 現在のモーションセンサーのデータとモーションセンサーから計算されたデバイスの状態を取得します。 
				 */
				virtual int getMotionState(SceMotionState* motionState) const = 0;

				/*!
				 * @~English
				 * @brief Get current device state and motion sensor data 
				 * @param motionContextData Pointer to MotionContextData structure
				 * 
				 * @retval (>=0) Success
				 * @retval (<0) Failure
				 * @details This obtains the current motion sensor data and device state calculated from the motion sensor. 
				 * @~Japanese
				 * @brief 現在のデバイスの状態やモーションセンサーのデータの取得 
				 * @param motionContextData MotionContextData構造体へのポインタ
				 * 
				 * @retval (>=0) 成功
				 * @retval (<0) 失敗
				 * @details 現在のモーションセンサーのデータとモーションセンサーから計算されたデバイスの状態を取得します。 
				 */
				virtual int getMotionContextData(MotionContextData* motionContextData) const = 0;

				/*!
				 * @~English
				 * @brief Reset device orientation 
				 * 
				 * @retval (>=0) Success
				 * @retval (<0) Failure
				 * @details This function resets the device orientation. Even if a reset is called, the filter settings and sampling mode will not change. For the behavior during a reset, refer to the "Using the Library" chapter of the "libmotion Overview" document. 
				 * @~Japanese
				 * @brief デバイス方向をリセット 
				 * 
				 * @retval (>=0) 成功
				 * @retval (<0) 失敗
				 * @details この関数はデバイス方向をリセットします。リセットが呼び出されても、フィルタ設定やサンプリングモードは変更されません。リセット時の挙動については、「libmotion 概要」ドキュメントの「使用方法」章を参照してください。 
				 */
				virtual int resetReferenceOrientation() = 0;

				/*!
				 * @~English
				 * @brief Device yaw rotation 
				 * @param radians Yaw rotation angle
				 * 
				 * @retval (>=0) Success
				 * @retval (<0) Failure
				 * @details When this function is used, the device's yaw can be corrected with yaw rotation. 
				 * @~Japanese
				 * @brief デバイスのヨー方向を回転 
				 * @param radians ヨー方向に回転させる角度
				 * 
				 * @retval (>=0) 成功
				 * @retval (<0) 失敗
				 * @details この関数を使うと、ヨー方向に回転を行うことにより、デバイスのヨー方向を補正することができます。 
				 */
				virtual int rotateReferenceOrientationYaw(float radians) = 0;

				/*!
				 * @~English
				 * @brief Set angle threshold value used when determining basic orientation output 
				 * @param radians Angle that sets the threshold value (degrees). Settings in a range from 0 to 45 degrees are available.
				 * 
				 * @retval (>=0) Success
				 * @retval (<0) Failure
				 * @details This sets the angle threshold value used when determining basic orientation output. For details on the angle threshold value, refer to the "libmotion Overview" document. 
				 * 
				 * @~Japanese
				 * @brief 簡易方向出力を決定する際に使われる角度しきい値を設定 
				 * @param radians しきい値を設定する角度（度）。0～45度の範囲で設定可能。
				 * 
				 * @retval (>=0) 成功
				 * @retval (<0) 失敗
				 * @details 簡易方向出力を決定する際に使われる角度しきい値を設定します。角度しきい値に関する詳細は、「libmotion 概要」ドキュメントを参照してください。 
				 * 
				 */
 
				virtual int setUserInterfaceOrientationThreshold(float radians) = 0;

				/*!
				 * @~English
				 * @brief Set accelerometer tilt correction filter 
				 * @param enable Enables or disables. Enabled if it is true. The default is true
				 * 
				 * @details This sets the accelerometer tilt correction filter. 
				 * @~Japanese
				 * @brief 加速度センサー傾き補正フィルタを設定 
				 * @param enable 有効・無効の指定。trueの場合有効。デフォルトはtrue
				 * 
				 * @details 加速度センサー傾き補正フィルタを設定します。 
				 */
				virtual void enableAccelerometerTiltCorrection(bool enable) = 0;

				/*!
				 * @~English
				 * @brief Set gyro dead-banding filter 
				 * @param enable Enables or disables. Enabled if it is true. The default is true
				 * 
				 * @details This sets the gyro dead-banding filter. 
				 * @~Japanese
				 * @brief ジャイロ不感帯フィルタを設定 
				 * @param enable 有効・無効の指定。trueの場合有効。デフォルトはtrue
				 * 
				 * @details ジャイロ不感帯フィルタを設定します。 
				 */
				virtual void enableGyroDeadband(bool enable) = 0;

				/*!
				 * @~English
				 * @brief Enable/disable gyro bias correction filter 
				 * @param enable Enables or disables. Enabled if it is true. The default is true
				 * 
				 * @details This function enables/disables the gyro bias correction filter. This function can be used even if sampling has not started. When the gyro bias correction filter is disabled, note that output from libmotion will drift. Therefore, the gyro bias correction filter is enabled by default in libmotion, and it is strongly recommended that this is not changed. For details on the gyro bias correction filter, refer to the "Using the Library" chapter of the "libmotion Overview" document. 
				 * @~Japanese
				 * @brief ジャイロバイアス補正フィルタの有効／無効を切り替え 
				 * @param enable 有効・無効の指定。trueの場合有効。デフォルトはtrue
				 * 
				 * @details この関数は、ジャイロバイアス補正フィルタの有効／無効を設定します。この関数は、サンプリングが開始されていなくても利用できます。ジャイロバイアス補正フィルタを無効にすると、結果としてlibmotionからの出力がドリフトする点に注意してください。そのため、libmotionは、デフォルトでジャイロバイアス補正フィルタを有効にしており、変更しないことを強く推奨します。ジャイロバイアス補正フィルタの詳細に関しては、「libmotion 概要」ドキュメントの「使用方法」章を参照してください。 
				 */
				virtual void enableGyroBiasCorrection(bool enable) = 0;
					
			};

			/*!
			 * @~English
			 * @brief Create MotionContext 
			 * @param outMotionContext Pointer to which the generated MotionContext returns
			 * 
			 * @retval (>=0) Success
			 * @retval (<0) Failure
			 * @details This executes the initialization processing of the MotionContext class. 
			 * @~Japanese
			 * @brief MotionContextの生成 
			 * @param outMotionContext 生成されたMotionContextが返るポインタ
			 * 
			 * @retval (>=0) 成功
			 * @retval (<0) 失敗
			 * @details MotionContextクラスの初期化処理を行います。 
			 */
			 int createMotionContext(MotionContext **outMotionContext);
#endif /* __psp2__ */

		}
	}
}

#endif /* _SCE_SAMPLE_UTIL_INPUT_MOTION_H */

