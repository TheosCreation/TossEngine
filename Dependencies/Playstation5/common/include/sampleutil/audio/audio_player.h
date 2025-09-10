/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <ajm.h>
#include <scebase_common.h>
#include <sampleutil/audio/audio_out.h>
#include <sampleutil/memory/cpu_memory.h>

namespace sce { namespace SampleUtil { namespace Audio
{
	/*!
	 * @~English
	 * 
	 * @brief	Audio play functions.
	 * @details	Provides to play various audio formats..
	 * @~Japanese
	 * 
	 * @brief	オーディオ再生機能
	 * @details	各種オーディオの再生機能を提供します
	 */
	class Player
	{
	private:
		struct Context;

	public:
		/*!
		 * @~English
		 * 
		 * @brief	Supported Codecs
		 * @details	Codecs list supported by player
		 * @~Japanese
		 * 
		 * @brief	サポートコーデック
		 * @details	プレイヤーがサポートするコーデック一覧
		 */
		enum class Codec : uint32_t
		{
			/*!
			 * @~English
			 * @brief MP3
			 * @~Japanese
			 * @brief MP3
			 */
			  kMp3

			/*!
			 * @~English
			 * @brief Atrac9
			 * @~Japanese
			 * @brief Atrac9
			 */
			, kAtrac9
		};

//		static constexpr size_t kWorkingMemorySizeInBytes = 4 * 1024 * 1024;

		/*!
		 * @~English
		 * 
		 * @brief	The size of default working memory.
		 * @~Japanese
		 * 
		 * @brief	デフォルトワーキングメモリサイズ
		 */
		static constexpr size_t kWorkingMemorySizeInBytes = 0x400 * 2;

	private:
		Context * m_context;

	public:
		/*!
		 * @~English
		 * @brief Consstructor
		 * @details This is a constructor.
		 * @~Japanese
		 * @brief コンストラクタ
		 * @details コンストラクタです。
		 */
		Player( );

		/*!
		 * @~English
		 * @brief Destructor 
		 * @details This is a destructor. 
		 * @~Japanese
		 * @brief デストラクタ 
		 * @details デストラクタです。 
		 */
		virtual ~Player( );

		/*!
		 * @~English
		 * @brief Initializes Player. 
		 * @param codec Specifies codec.
		 * @param pData Specifies PCM data.
		 * @param dataSizeInBytes Specifies size of PCM data.
		 * @param workingMemory Specifies pointer to working memory.
		 * @param workingMemorySizeInBytes  Specifies size of working memory.
		 *
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
 		 * @details Initializes player from PCM data.
		 * @~Japanese
		 * @brief プレイヤーを初期化します。
		 * @param codec コーデックを指定する。
		 * @param pData PCMデータを指定する。
		 * @param dataSizeInBytes PCMデータのサイズを指定する。
		 * @param workingMemory ワーキングメモリを指定する。
		 * @param workingMemorySizeInBytes ワーキングメモリサイズを指定する。
		 *
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
 		 * @details プレイヤーをPCMデータで初期化します。
		 */
		int		initialize(Codec codec, const void	*pData, const size_t dataSizeInBytes, void * workingMemory, const size_t workingMemorySizeInBytes );

		/*!
		 * @~English
		 * @brief Initializes Player. 
		 * @param codec Specifies codec.
		 * @param pFilename Specifies a file.
		 * @param workingMemory Specifies pointer to working memory.
		 * @param workingMemorySizeInBytes  Specifies size of working memory.
		 *
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @details Initializes player from a file.
		 * @~Japanese
		 * @brief プレイヤーを初期化します。
		 * @param codec コーデックを指定する。
		 * @param pFilename ファイルを指定します。
		 * @param workingMemory ワーキングメモリを指定する。
		 * @param workingMemorySizeInBytes ワーキングメモリサイズを指定する。
		 *
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 * @details プレイヤーをファイルから初期化します。
		 */
		int		initialize(Codec codec, const char	*pFilename, void * workingMemory, const size_t workingMemorySizeInBytes );

		/*!
		 * @~English
		 * @brief Finalizes Player. 
		 *
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief プレイヤーを終了します。
		 *
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int		finalize();

		/*!
		 * @~English
		 * @brief Plays audio.
		 * @param audioOut Specifies audio context to use to play.
		 * @param pOutIsFinished Specifies bool-type pointer to know whether finished play.
		 *
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @details Plays audio by initializing the info.
		 * @~Japanese
		 * @brief オーディオの再生を行います。
		 * @param audioOut 再生に使用するAudiOutContextを指定します。
		 * @param pOutIsFinished 再生が完了したことを知るためのbool型のポインタを指定します。再生が完了した場合は、trueが設定されます。
		 *
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 * @details 初期化情報に基づきオーディオの再生を行います。
		 */
		int		play(sce::SampleUtil::Audio::AudioOutContext	&audioOut, bool	*pOutIsFinished = nullptr);

		/*!
		 * @~English
		 * @brief Gets sampling rate.
		 * @param outSamplingRate Specifies the variable to receive for sampling rate.
		 *
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @details Gets sampling rate is specified audio at initializing.
		 * @~Japanese
		 * @brief サンプリングレートを取得します
		 * @param outSamplingRate サンプリングレートを受け取るための変数を指定します。
		 *
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 * @details 初期化時に指定されたオーディオのサンプリングレートを取得します。
		 */
		int		getTargetSamplingRate( uint & outSamplingRate ) const;

		/*!
		 * @~English
		 * @brief Gets audio format.
		 * @param outAudioFormat Specifies the variable to receive for audio format.
		 *
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @details Gets audio format is specified audio at initializing.
		 * @~Japanese
		 * @brief オーディオフォーマットを取得します
		 * @param outAudioFormat オーディオフォーマットを受け取るための変数を指定します。
		 *
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 * @details 初期化時に指定されたオーディオのフォーマットを取得します。
		 */
		int		getTargetAudioFormat( Format & outAudioFormat ) const;

	};
}}} // namespace sce::SampleUtil::Audio

