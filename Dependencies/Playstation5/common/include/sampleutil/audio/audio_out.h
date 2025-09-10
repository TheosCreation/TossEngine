/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2020 Sony Interactive Entertainment Inc. 
* 
*/

#pragma once

#include <scebase_common.h>
#include <sampleutil/sampleutil_common.h>
#include <sampleutil/system.h>
#include <sampleutil/thread/thread.h>

namespace sce
{
	namespace SampleUtil
	{
		/*!
		 * @~English
		 * @brief Audio-associated definitions 
		 * @details sce::SampleUtil::Audio is the name space associated with the audio of the SampleUtil library. 
		 * @~Japanese
		 * @brief Audio関連の定義 
		 * @details sce::SampleUtil::AudioはSampleUtilライブラリのオーディオ関連の名前空間です。 
		 */
		namespace Audio
		{
			/*!
			 * @~English
			 * @brief The maximum number of MAIN virtual device ports
			 * @~Japanese
			 * @brief MAIN仮想デバイスの最大数
			 */
			static const uint32_t kMaxPortsMainVirtualDevice = 8;
			/*!
			 * @~English
			 * @brief The maximum number of BGM virtual device ports
			 * @~Japanese
			 * @brief BGM仮想デバイスの最大数
			 */
			static const uint32_t kMaxPortsBgmVirtualDevice = 1;
			/*!
			 * @~English
			 * @brief The maximum number of VOICE virtual device ports
			 * @~Japanese
			 * @brief VOICE仮想デバイスの最大数
			 */
			static const uint32_t kMaxPortsVoiceVirtualDevice = 4;
			/*!
			 * @~English
			 * @brief The maximum numberof PERSONAL virtual device ports
			 * @~Japanese
			 * @brief PERSONAL仮想デバイスの最大数
			 */
			static const uint32_t kMaxPortsPersonalVirtualDevice = 4;
			/*!
			 * @~English
			 * @brief The maximum numberof PADSPK virtual device ports
			 * @~Japanese
			 * @brief PADSPK仮想デバイスの最大数
			 */
			static const uint32_t kMaxPortsPadsplVirtualDevice = 4;
			/*!
			 * @~English
			 * @brief The maximum numberof AUX virtual device ports
			 * @~Japanese
			 * @brief AUX仮想デバイスの最大数
			 */
			static const uint32_t kMaxPortsAuxVirtualDevice = 1;
			/*!
			 * @~English
			 * 
			 * @brief The enumeration type to be used for specifying audio output port type
			 * @details Used for specifying an argument for creating an AudioOutPort
			 * @~Japanese
			 * 
			 * @brief オーディオ出力ポート型を指定するために利用する列挙型 
			 * @details AudioOutPortを作成する際に指定
			 */
			enum class OutPortType
			{
				/*!
				 * @~English
				 * @brief MAIN virtual device port
				 * @~Japanese
				 * @brief MAIN仮想デバイスのポート
				 */
				kMain,
				/*!
				 * @~English
				 * @brief BGM virtual device port
				 * @~Japanese
				 * @brief BGM仮想デバイスのポート
				 */
				kBgm,
				/*!
				 * @~English
				 * @brief VOICE virtual device port
				 * @~Japanese
				 * @brief VOICE仮想デバイスのポート
				 */
				kVoice,
				/*!
				 * @~English
				 * @brief PERSONAL virtual device port
				 * @~Japanese
				 * @brief PERSONAL仮想デバイスのポート
				 */
				kPersonal,
				/*!
				 * @~English
				 * @brief PADSPK virtual device port
				 * @~Japanese
				 * @brief PADSPK仮想デバイスのポート
				 */
				kPadspk,
#if _SCE_TARGET_OS_PROSPERO
				/*!
				 * @~English
				 * @brief VIBRATION virtual device port
				 * @~Japanese
				 * @brief VIBRATION仮想デバイスのポート
				 */
				kVibration,
#endif
				/*!
				 * @~English
				 * @brief AUX virtual device port
				 * @~Japanese
				 * @brief AUX仮想デバイスのポート
				 */
				kAux
			};

			/*!
			 * @~English
			 * 
			 * @brief The enumeration type to be used for specifying audio data format
			 * @details Used for specifying an argument for creating an AudioOutPort
			 * @~Japanese
			 * 
			 * @brief 出力するオーディオデータのフォーマットを指定するために利用する列挙型 
			 * @details AudioOutPortを作成する際に指定
			 */
			enum class Format
			{
				/*!
				 * @~English
				 * @brief Signed 16-bit monaural
				 * @~Japanese
				 * @brief 符号付き16bit モノラル
				 */
				kS16_Mono,
				/*!
				 * @~English
				 * @brief Signed 16-bit 2ch stereo
				 * @~Japanese
				 * @brief 符号付き16bit 2chステレオ
				 */
				kS16_Stereo,
				/*!
				 * @~English
				 * @brief Signed 16-bit 7.1 multi-channel (L-R-C-LFE-Lsurround-Rsurround-Lextend-Rextend interleaved)
				 * @~Japanese
				 * @brief 符号付き16bit 7.1chマルチチャンネル（L-R-C-LFE-Lsurround-Rsurround-Lextend-Rextendインターリーブ）
				 */
				kS16_8Ch,
				/*!
				 * @~English
				 * @brief Float 32-bit monaural
				 * @~Japanese
				 * @brief Float 32bit モノラル
				 */
				kFloat_Mono,
				/*!
				 * @~English
				 * @brief Float 32-bit 2ch stereo
				 * @~Japanese
				 * @brief Float 32bit 2chステレオ
				 */
				kFloat_Stereo,
				/*!
				 * @~English
				 * @brief Float 32-bit 7.1 multi-channel (L-R-C-LFE-Lsurround-Rsurround-Lextend-Rextend interleaved)
				 * @~Japanese
				 * @brief Float 32bit 7.1chマルチチャンネル（L-R-C-LFE-Lsurround-Rsurround-Lextend-Rextendインターリーブ）
				 */
				kFloat_8Ch,
				/*!
				 * @~English
				 * @brief Signed 16-bit 7.1 multi-channel (L-R-C-LFE-Lextend-Rextend-Lsurround-Rsurround interleaved)
				 * @~Japanese
				 * @brief 符号付き16bit 7.1chマルチチャンネル（L-R-C-LFE-Lextend-Rextend-Lsurround-Rsurroundインターリーブ）
				 */
				kS16_8chStd,
				/*!
				 * @~English
				 * @brief Float 32-bit 7.1ch multi-channel (L-R-C-LFE-Lextend-Rextend-Lsurround-Rsurround interleaved)
				 * @~Japanese
				 * @brief Float 32bit 7.1chマルチチャンネル（L-R-C-LFE-Lextend-Rextend-Lsurround-Rsurroundインターリーブ）
				 */
				kFloat_8ChStd
			};

			/*!
			 * @~English
			 * 
			 * @brief Structure for initializing AudioOutUser
			 * @details This is the structure for initializing AudioOutUser. This is used by specifying it to the argument "option" of createAudioOutUser(). 
			 * @~Japanese
			 * 
			 * @brief AudioOutUserの初期化用構造体 
			 * @details AudioOutUserの初期化用構造体です。
			 */
			struct AudioOutUserOption
			{
				/*!
				 * @~English
				 * @brief User id
				 * @~Japanese
				 * @brief ユーザID
				 */
				sce::SampleUtil::System::UserId m_userId;
			};

			/*!
			 * @~English
			 * 
			 * @brief Structure for initializing AudioOutPort
			 * @details This is the structure for initializing AudioOutPort. This is used by specifying it to the argument "option" of createAudioOutPort. 
			 * @~Japanese
			 * 
			 * @brief AudioOutPortの初期化用構造体 
			 * @details AudioOutPortの初期化用構造体です。
			 */
			struct AudioOutPortOption
			{
				/*!
				 * @~English
				 * @brief Output port type
				 * @details Specify output port virtual device type
				 * @~Japanese
				 * @brief 出力ポートタイプ
				 * @details 出力ポートの仮想デバイスタイプを指定します。
				 */
				OutPortType					m_type;
				/*!
				 * @~English
				 * @brief Audio output data format
				 * @details Specify output audio data format
				 * @~Japanese
				 * @brief オーディオデータのフォーマット
				 * @details 出力するオーディオデータのフォーマットを指定します。
				 */
				Format						m_format;
				/*!
				 * @~English
				 * @brief Sampling rate
				 * @details Specify output audio data sampling rate
				 * @~Japanese
				 * @brief サンプリングレート
				 * @details 出力するオーディオデータのサンプリングレートをしてします。
				 */
				uint32_t					m_sampleRateInHz;
			};


			/*!
			 * @~English
			 * 
			 * @brief Structure for initializing AudioOutContext
			 * @details This is the structure for initializing AudioOutContext. This is used by specifying it to the argument "option" of createAudioOutContext(). 
			 * @~Japanese
			 * 
			 * @brief AudioOutContextの初期化用構造体 
			 * @details AudioOutContextの初期化用構造体です。createAudioOutContext()の引数optionに指定することで利用します。 
			 */
			struct AudioOutContextOption
			{
				/*!
				 * @~English
				 *
				 * @brief Specify user's info
				 * @details Refer to description of 'AudioOutUserOption' structure
				 * @~Japanese
				 *
				 * @brief ユーザ情報を指定する
				 * @details 「AudioOutUserOption」構造体の記述を参照
				 */
				AudioOutUserOption		*m_audioOutUserOption;

				/*!
				 * @~English
				 *
				 * @brief Specify port info
				 * @details Refer to description of 'AudioOutPortOption' structure
				 * @~Japanese
				 *
				 * @brief ポート情報を指定する
				 * @details 「AudioOutPortOption」構造体の記述を参照
				 */
				AudioOutPortOption		*m_audioOutPortOption;
			};
			
			/*!
			 * @~English
			 * 
			 * @brief Structure for Audio Output Parameter
			 * @details This structure is used to specify output parameter at the time of output request
			 * @~Japanese
			 * 
			 * @brief オーディオ出力パラメータのための構造体
			 * @details この構造体は、出力要求時の出力パラメータ指定に使用されます
			 */
			struct OutputParam
			{
				/*!
				 * @~English
				 * @brief Port number
				 * @details Output port number
				 * @~Japanese
				 * @brief ポートId
				 * @details 出力先ポートId
				 */
				int							m_portId;

				/*!
				 * @~English
				 * @brief Output PCM data
				 * @details Specify output data. Possible only PCM format
				 * @~Japanese
				 * @brief 出力PCMデータ
				 * @details 出力データを指定する。PCMフォーマットのみ可能
				 */
				const void					*m_audioData;

				/*!
				 * @~English
				 * @brief Output PCM data size in bytes
				 * @details Specify output data size in bytes
				 * @~Japanese 
				 * @brief 出力PCMデータサイズ(バイト単位)
				 * @details 出力PCMデータサイズを指定する(バイト単位)
				 */
				size_t						m_audioDataSizeInBytes;
			};

			/*!
			 * @~English
			 * 
			 * @brief Class for outputting audio data
			 * @details Create an instance per audio output port, and output audio data through that instance
			 * @~Japanese
			 * 
			 * @brief オーディオ出力を行うためのクラス
			 * @details ポートごとにこのクラスのインスタンスを作成し、オーディオデータの出力を行います
			 */
			class AudioOutContext
			{
			public:
				/*!
				 * @~English
				 * @brief Consstructor
				 * @param contextOption AudioOutContextOption structure.
				 * @details This is a constructor.
				 * @~Japanese
				 * @brief コンストラクタ
				 * @param contextOption AudioOutContextOption構造体。
				 * @details コンストラクタです。
				 */
				AudioOutContext(const AudioOutContextOption *contextOption = nullptr);

				/*!
				 * @~English
				 * @brief Destructor 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * @details デストラクタです。 
				 */
				virtual ~AudioOutContext();

				/*!
				 * @~English
				 * @brief Initializes audio context.
				 * @param contextOption AudioOutContextOption structure.
				 * @details Initializes audio context. This function can be to destroy the current context and reinitialize context.
	 			 * @retval SCE_OK Success
	 			 * @retval (<0) Error code
				 * @~Japanese
				 * @brief オーディオコンテキストを初期化します。
				 * @param contextOption AudioOutContextOption構造体。
				 * @details オーディオコンテキストを初期化します。現在のコンテキストを破棄して、再度初期化を行うことができます。
	 			 * @retval SCE_OK Success
	 			 * @retval (<0) Error code
				 */
				int initialize(const AudioOutContextOption *contextOption = nullptr);

				/*!
				 * @~English
				 *
				 * @brief Create a user
				 * @param audioOutUserOption Specify info to create a user. Refer to description of 'AudioOutUserOption' structure
				 * @details Create a user of owner of audio object
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese 
				 *
				 * @brief ユーザを作成する
				 * @param audioOutUserOption ユーザ作成情報を指定する。「AudioOutUserOption」構造体の記述を参照
				 * @details オーディオオブジェクトのオーナユーザを作成する
	 			 * @retval SCE_OK 成功
	 			 * @retval (<0) エラーコード
				 */
				int createAudioOutUser(const AudioOutUserOption *audioOutUserOption);

				/*!
				 * @~English
				 *
				 * @brief Destroy a user.
				 * @details Destroy a user of owner of audio object.
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 *
				 * @brief ユーザを破棄する
				 * @details オーディオオブジェクトのオーナユーザを破棄する
	 			 * @retval SCE_OK 成功
	 			 * @retval (<0) エラーコード
				 */
				int destroyAudioOutUser();

				/*!
				 * @~English
				 *
				 * @brief Determine whether user is enabled
				 * @details  Determine whether user is enabled
				 * @param outEnabled If user is enabled, to set 'true'. If user isn't enabled, to set false.
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 *
				 * @brief ユーザが有効かどうか判断する
				 * @details ユーザが有効かどうか判断する
				 * @param outEnabled ユーザが有効ならば、「true」が設定される。ユーザが有効でないならば、「false」が設定される。
				 * @retval SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int isAudioOutUserEnabled(bool *outEnabled) const;

				/*!
				 * @~English
				 *
				 * @brief Create a port
				 * @param outPortId Port ID is stored on funtion return
				 * @param audioOutPortOption Specify info to create port. Refer to description of 'AudioOutPortOption' structure
				 * @details Create a port for audio output
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese 
				 *
				 * @brief ポートを作成する
				 * @param outPortId ポートIDの格納先
				 * @param audioOutPortOption ポート作成情報を指定する。「AudioOutPortOption」構造体の記述を参照
				 * @details オーディオ出力用のポートを作成する
	 			 * @retval SCE_OK 成功
	 			 * @retval (<0) エラーコード
				 */
				int createAudioOutPort(int	*outPortId, const AudioOutPortOption *audioOutPortOption);

				/*!
				 * @~English
				 *
				 * @brief Destroy a port.
				 * @param portId Specify port to be destroyed
				 * @details Destroy a port for audio output
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 *
				 * @brief ポートを破棄する
				 * @param portId 破棄するポートを指定
				 * @details オーディオ出力用のポートを破棄する
	 			 * @retval SCE_OK 成功
	 			 * @retval (<0) エラーコード
				 */
				int destroyAudioOutPort(int	portId);

				/*!
				 * @~English
				 *
				 * @brief Destroy all ports.
				 * @details Destroy all ports for audio output
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 *
				 * @brief 全ポートを破棄する
				 * @details 全オーディオ出力用のポートを破棄する
	 			 * @retval SCE_OK 成功
	 			 * @retval (<0) エラーコード
				 */
				int destroyAudioOutPortsAll();

				/*!
				 * @~English
				 * 
				 * @brief Output audio data
				 * @param audioData Output PCM data
				 * @param audioDataSizeInBytes Output PCM data size in bytes
				 * @param condvar Condition variable to wait for completion of request
				 * @param condValue This value is used to watch for completion of this request
				 * @details Output specified number of PCM data.
				 * 
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * 
				 * @brief オーディオデータの出力
				 * @param audioData 出力PCMデータ
				 * @param audioDataSizeInBytes 出力PCMデータバイト数
				 * @param condvar リクエストの完了を監視するための条件変数
				 * @param condValue この値はリクエストの完了を監視するために使用されます。
				 * @details 指定した数のPCMデータを出力します。
				 * 
				 * @retval SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int output(const void *audioData, const size_t audioDataSizeInBytes, Thread::CondVar<uint64_t> *condvar = nullptr, const uint64_t condValue = 0);

				/*!
				 * @~English
				 * 
				 * @brief Output audio data
				 * @param outputParams Specify output parameters.
				 * @param numOutputParams Specify number of output parameters are included in 'outputParams'
				 * @param condvar Condition variable to wait for completion of request
				 * @param condValue This value is used to watch for completion of this request
				 * @details Outputs audio data per port.
				 * 
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * 
				 * @brief オーディオデータの出力
				 * @param outputParams 出力パラメータを指定
				 * @param numOutputParams 「outputParams」に含まれる、出力パラメータの数を指定する
				 * @param condvar リクエストの完了を監視するための条件変数
				 * @param condValue この値はリクエストの完了を監視するために使用されます。
				 * @details ポートごとにオーディオデータを出力する
				 * 
				 * @retval SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int outputMulti(const OutputParam * outputParams, uint32_t numOutputParams, Thread::CondVar<uint64_t> * condvar = nullptr, const uint64_t condValue = 0);

				/*!
				 * @~English
				 * 
				 * @brief Get current gains
				 * @param outGrainds If the function succeeds, receives the current gains
				 * @details Get current gains of audio output
	 			 * @retval SCE_OK Success
	 			 * @retval (<0) Error code
	 			 * @~Japanese
	 			 *
				 * @brief 現在のゲインを取得する
				 * @param outGrainds 関数が成功した場合は、現在のゲイン値が設定される
				 * @details 現在のオーディオ出力ゲインを取得する
	 			 * @retval SCE_OK Success
	 			 * @retval (<0) Error code
				*/
				int	getGrains(uint32_t *outGrainds) const;

				private:
					void *internalAudioContext;
			};
		}
	}
}
