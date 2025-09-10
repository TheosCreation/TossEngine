/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2019 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#include <scebase_common.h>
#include <sampleutil/sampleutil_common.h>

namespace sce
{
	namespace SampleUtil
	{
		/*!
		 * @~English
		 * @brief Network-associated definitions
		 * @details These are the Network-associated definitions.
		 * @~Japanese
		 * @brief Network関連の定義
		 * @details Network関連の定義です。
		 */
		namespace Network
		{
			/*!
			 * @~English
			 * @brief Flags to specify NP function initialize
			 * @details Specify NP functionalities to be initialized
			 * @~Japanese
			 * @brief NP機能初期化に指定するフラグ
			 * @details 初期化するNP機能を指定します。
			 */
			enum NpInitializeFlag
			{
				/*!
				 * @~English
				 * @brief Initialize SSL
				 * @~Japanese
				 * @brief SSLの初期化
				 */
				kSSL = 0x1 << 0,
				/*!
				 * @~English
				 * @brief Initialize HTTP
				 * @~Japanese
				 * @brief HTTPの初期化
				 */
				kHTTP = 0x1 << 1,
				/*!
				 * @~English
				 * @brief Initialize NP WebApi
				 * @~Japanese
				 * @brief NP WebApiの初期化
				 */
				kWebApi = 0x1 << 2,
				/*!
				 * @~English
				 * @brief Initialize JSON
				 * @~Japanese
				 * @brief JSONの初期化
				 */
				kJSON = 0x1 << 3,
				/*!
				* @~English
				* @brief Initialize sceNet only
				* @~Japanese
				* @brief sceNetのみの初期化
				*/
				kSceNetOnly = 0x1 << 4,
				/*!
				 * @~English
				 * @brief Initialize SSL and HTTP
				 * @~Japanese
				 * @brief SSLとHTTPの初期化
				 */
				kNet = (kSSL | kHTTP),
			};

			/*!
			 * @~English
			 * @brief Parameter used for initialize NP system
			 * @details Specify parameters used to initialize NP
			 * @~Japanese
			 * @brief NPシステムを初期化するパラメータ
			 * @details NPを初期化するパラメータを指定してください。
			 */
			struct NpInitializeParam
			{
				/*!
				 * @~English
				 * @brief initialize flag
				 * @~Japanese
				 * @brief 初期化フラグ
				 */
				uint32_t	m_initFlag;
				/*!
				 * @~English
				 * @brief heap size for sceNet
				 * @~Japanese
				 * @brief sceNetのヒープサイズ
				 */
				size_t		m_netHeapSize;
				/*!
				 * @~English
				 * @brief heap size for sceSsl
				 * @~Japanese
				 * @brief sceSslのヒープサイズ
				 */
				size_t		m_sslHeapSize;
				/*!
				 * @~English
				 * @brief heap size for sceHttp
				 * @~Japanese
				 * @brief sceHttpのヒープサイズ
				 */
				size_t		m_httpHeapSize;
				/*!
				 * @~English
				 * @brief heap size for sceNpWebApiInitialize
				 * @~Japanese
				 * @brief sceNpWebApiInitializeで指定するヒープサイズ
				 */
				size_t		m_webapiHeapSize;
				/*!
				 * @~English
				 * @brief Default constructor
				 * @~Japanese
				 * @brief デフォルトコンストラクタ
				 */
				NpInitializeParam()
				{
					memset(this, 0, sizeof(NpInitializeParam));
				}

				/*!
				 * @~English
				 * @brief Constructor
				 * @param flag initialize flag
				 * @param net heap size for sceNet
				 * @param ssl heap size for sceSsl
				 * @param http heap size for sceHttp
				 * @param webapi heap size for sceNpWebApiInitialize
				 * @~Japanese
				 * @brief コンストラクタ
				 * @param flag 初期化フラグ
				 * @param net sceNetのヒープサイズ
				 * @param ssl sceSslのヒープサイズ
				 * @param http sceHttpのヒープサイズ
				 * @param webapi sceNpWebApiInitializeのヒープサイズ
				 */
				NpInitializeParam(uint32_t flag, size_t net, size_t ssl, size_t http, size_t webapi)
				{
					m_initFlag = flag;
					m_netHeapSize = net;
					m_sslHeapSize = ssl;
					m_httpHeapSize = http;
					m_webapiHeapSize = webapi;
				}
			};

			/*!
			 * @~English
			 *
			 * @brief Function to initialize various network libraries, and set NP title-id and NP title-secret.
			 *
			 * @param npTitleIdStr Application's NP title-id
			 * @param npTitleSecretHexStr Application's NP title-secret
			 * @param nameNetPool Net pool name
			 * @param param NP initialize parameters
			 *
			 * @retval SCE_OK Success
			 * @retval (<0) Error code
			 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Invalid argument specified.
			 * @retval SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY Insufficient memory.
			 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_STATE Invalid state.
			 * @retval SCE_SAMPLE_UTIL_ERROR_FATAL Fatal error occurred.
			 * @retval SCE_SAMPLE_UTIL_ERROR_NULL_POINTER Pointer is NULL.
			 * @details This function initializes various network libraries. The details of processings in this function will be describet with future SDK release.
			 * @~Japanese
			 *
			 * @brief 各種ネットワークライブラリの初期化とNPタイトルIDとNPタイトルシークレットのセットを行う関数
			 *
			 * @param npTitleIdStr アプリケーションのNPタイトルID
			 * @param npTitleSecretHexStr アプリケーションのNPタイトルシークレット
			 * @param nameNetPool Netプールの名前
			 * @param param NP初期化パラメータ
			 *
			 * @retval SCE_OK 成功
			 * @retval (<0) エラーコード
			 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM 不正な引数を指定した
			 * @retval SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY メモリ不足
			 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_STATE 不正な状態になった
			 * @retval SCE_SAMPLE_UTIL_ERROR_FATAL 致命的なエラー
			 * @retval SCE_SAMPLE_UTIL_ERROR_NULL_POINTER NULLポインターエラー
			 * @details 各種ネットワークライブラリの初期化を行います。行われる処理の内容は今後記載されます。
			 */
			int	initializeNp(const char *npTitleIdStr, const char *npTitleSecretHexStr, const char *nameNetPool, NpInitializeParam &param);

			/*!
			 * @~English
			 *
			 * @brief Update processing of network libraries
			 *
			 * @retval SCE_OK Success
			 * @retval (<0) Error code
			 * @details This function executes the processings in a batch which need to be executed periodically
			 * @~Japanese
			 *
			 * @brief ネットワークライブラリの更新処理
			 *
			 * @retval SCE_OK 成功
			 * @retval (<0) エラーコード
			 * @details この関数ではsceNpCheckCallback()など定期的に呼ぶべきものをまとめて実行されます
			 */
			int	updateNp();
			/*!
			 * @~English
			 *
			 * @brief Terminate processing of network libraries
			 *
			 * @retval SCE_OK Success
			 * @retval (<0) Error code
			 * @retval SCE_SAMPLE_UTIL_ERROR_BUSY Memory is in use.
			 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM Invalid argument specified.
			 * @retval SCE_SAMPLE_UTIL_ERROR_FATAL Fatal error occurred.
			 * @details This function terminate the process of network libraries
			 * @~Japanese
			 *
			 * @brief ネットワークライブラリの終了処理
			 *
			 * @retval SCE_OK 成功
			 * @retval (<0) エラーコード
			 * @retval SCE_SAMPLE_UTIL_ERROR_BUSY メモリが使用中である
			 * @retval SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM 不正な引数を指定した
			 * @retval SCE_SAMPLE_UTIL_ERROR_FATAL 致命的なエラー
			 * @details この関数ではネットワーク処理の終了処理を行います
			 */
			int	finalizeNp();
			/*!
			 * @~English
			 *
			 * @brief Get SSL context id
			 *
			 * @retval (>0) SSL context id
			 * @retval (<0) Error code
			 * @details This function returns SSL context id
			 * @~Japanese
			 *
			 * @brief SSLコンテキストIDを返す
			 *
			 * @retval (>0) SSLコンテキストID
			 * @retval (<0) エラーコード
			 * @details この関数ではSSLコンテキストIDを返します
			 */
			int getSslContextId();
			/*!
			 * @~English
			 *
			 * @brief Get HTTP context id
			 *
			 * @retval (>0) HTTP context id
			 * @retval (<0) Error code
			 * @details This function returns HTTP context id
			 * @~Japanese
			 *
			 * @brief HTTPコンテキストIDを返す
			 *
			 * @retval (>0) HTTPコンテキストID
			 * @retval (<0) エラーコード
			 * @details この関数ではHTTPコンテキストIDを返します
			 */
			int getHttpContextId();
			/*!
			 * @~English
			 *
			 * @brief Get WebApi library context id
			 *
			 * @retval (>0) WebApi context id
			 * @retval (<0) Error code
			 * @details This function returns WebApi library context id
			 * @~Japanese
			 *
			 * @brief WebApiライブラリコンテキストIDを返す
			 *
			 * @retval (>0) WebApiライブラリコンテキストID
			 * @retval (<0) エラーコード
			 * @details この関数ではWebApiライブラリコンテキストIDを返します
			 */
			int getWebApiLibContextId();
		}
	}
}
