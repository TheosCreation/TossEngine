/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#pragma once

#include <sampleutil/sampleutil_common.h>
#include <sampleutil/sampleutil_error.h>

#include <user_service.h>
#include <rtc.h>
#define SCE_SAMPLE_UTIL_MAX_LOGIN_USERS SCE_USER_SERVICE_MAX_LOGIN_USERS
#define SCE_SAMPLE_UTIL_USER_ID_INVALID SCE_USER_SERVICE_USER_ID_INVALID

namespace sce
{
	namespace SampleUtil
	{

		/*!
		 * @~English
		 * @brief System-associated definitions 
		 * @details These are the System-associated definitions. 
		 * @~Japanese
		 * @brief System関連の定義 
		 * @details System関連の定義です。 
		 */
		namespace System
		{
			/*!
			 * @~English
			 * 
			 * @brief User ID type 
			 * @details This is the user ID type. 
			 * @~Japanese
			 * 
			 * @brief ユーザーIDの型 
			 * @details ユーザーIDの型です。 
			 */
			typedef SceUserServiceUserId UserId;
			/*!
			 * @~English
			 * @brief Invalid user id
			 * @~Japanese
			 * @brief 無効なユーザーID
			 */
			static const UserId kInvalidUserId		= SCE_USER_SERVICE_USER_ID_INVALID;
			/*!
			 * @~English
			 * @brief Maximum user name length
			 * @~Japanese
			 * @brief ユーザー名最大長
			 */
			static const size_t kMaxUserNameLength	= SCE_USER_SERVICE_MAX_USER_NAME_LENGTH;
			/*!
			 * @~English
			 * @brief Maximum number of login users
			 * @~Japanese
			 * @brief 最大ログインユーザー数
			 */
			static const uint32_t kMaxLoginUsers	= SCE_USER_SERVICE_MAX_LOGIN_USERS;

			/*!
			 * @~English
			 * @brief Structure that stores the user ID list 
			 * @details This structure stores the user ID list. 
			 * @~Japanese
			 * @brief ユーザーIDリストを格納する構造体 
			 * @details ユーザーIDリストを格納する構造体です。 
			 */
			struct UserIdList {
				/*!
				 * @~English
				 * @brief This is the user ID list. 
				 * @~Japanese
				 * @brief ユーザーIDリストです。 
				 */
				UserId userId[SCE_SAMPLE_UTIL_MAX_LOGIN_USERS];
			};

			/*!
			 * @~English
			 * @brief Structure that stores the user login status (logged in/logged out) change events 
			 * @details This structure stores the user login status (logged in/logged out) change events. 
			 * @~Japanese
			 * @brief ユーザーのログイン状態(ログイン/ログアウト)の変更イベントを格納する構造体 
			 * @details ユーザーのログイン状態(ログイン/ログアウト)の変更イベントを格納する構造体です。 
			 */
			struct UserLoginStatusChangedEvents {
				/*!
				 * @~English
				 * @brief User ID list of logged in users that joined the game. 
				 * @~Japanese
				 * @brief ログインしてゲームに参加したユーザーのユーザーIDリストです。 
				 */
				UserIdList joinedUserIdList;
				/*!
				 * @~English
				 * @brief User ID list of users that have logged out and left the game. 
				 * @~Japanese
				 * @brief ログアウトしてゲームから抜けたユーザーのユーザーIDリストです。 
				 */
				UserIdList leftUserIdList;
			};

			/*!
			 * @~English
			 * 
			 * @brief Enumerator type for user colors 
			 * @details This is the enumerator type for user colors. 
			 * @~Japanese
			 * 
			 * @brief ユーザーカラーの列挙型 
			 * @details ユーザーカラーの列挙型です。 
			 */
			enum UserColor
			{
				/*!
				 * @~English
				 * @brief Blue 
				 * @~Japanese
				 * @brief 青 
				 */
				kUserColorBlue = 0,
				/*!
				 * @~English
				 * @brief Red 
				 * @~Japanese
				 * @brief 赤 
				 */
				kUserColorRed = 1,
				/*!
				 * @~English
				 * @brief Green 
				 * @~Japanese
				 * @brief 緑 
				 */
				kUserColorGreen = 2,
				/*!
				 * @~English
				 * @brief Pink 
				 * @~Japanese
				 * @brief ピンク 
				 */
				kUserColorPink = 3
			};

			/*!
			* @~English
			*
			* @brief Options structure for UserIdManager
			* @details This is an options structure for UserIdManager. This is used by specifying it to the option argument of UserIdManager constructor.
			* @~Japanese
			*
			* @brief UserIdManagerのオプション構造体
			* @details UserIdManagerのオプション構造体です。UserIdManagerコンストラクタの引数optionに指定することで利用します。
			*/
			typedef struct UserIdManagerOption {
			} UserIdManagerOption;

			/*!
			 * @~English
			 * 
			 * @brief Class for managing user IDs 
			 * @details UserIdManager is a class for supporting the management of user IDs. 
			 * @~Japanese
			 * 
			 * @brief ユーザーIDを管理するためのクラス 
			 * @details UserIdManagerはユーザーIDの管理をサポートするためのクラスです。 
			 */
			class UserIdManager
			{
			public:
				/*!
				* @~English
				* @brief Constructor
				* @param option This is the UserIdManagerOption structure.
				* @details This is a constructor.
				* @~Japanese
				* @brief コンストラクタ
				* @param option UserIdManagerOption構造体。
				* @details コンストラクタです。
				*/
				UserIdManager(UserIdManagerOption *option = nullptr);

				/*!
				 * @~English
				 * @brief Destructor 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * @details デストラクタです。 
				 */
				virtual ~UserIdManager();

				/*!
				 * @~English
				 * 
				 * @brief Update 
				 * @details This is the update function of UserIdManager. The UserIdManager user ID list updates are performed in this function. 
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * 
				 * @brief 更新 
				 * @details UserIdManagerの更新関数です。本関数内でUserIdManagerのユーザーIDリストの更新が行われます。 
				 * @retval SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int update();

				/*!
				 * @~English
				 * 
				 * @brief Get user ID list of logged in users 
				 * @param list UserIdList pointer
				 * @details This gets the user ID list of the logged in users. 
				 * 
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * 
				 * @brief ログインしているユーザーのユーザーIDリストの取得 
				 * @param list UserIdListのポインタ
				 * @details ログインしているユーザーのユーザーIDリストを取得します。 
				 * 
				 * @retval SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int getLoginUserIdList(UserIdList *list);

				/*!
				 * @~English
				 * 
				 * @brief Get user login status (logged in/logged out) change event for last update 
				 * @param events UserLoginStatusChangedEvents pointer
				 * @details This gets the user login status (logged in/logged out) change event for the last update. 
				 * 
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * 
				 * @brief 最後に更新した際のユーザーのログイン状態(ログイン/ログアウト)の変更イベントの取得 
				 * @param events UserLoginStatusChangedEventsのポインタ
				 * @details 最後に更新した際のユーザーのログイン状態(ログイン/ログアウト)の変更イベントを取得します。 
				 * 
				 * @retval SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int getUserLoginStatusChangedEventsOfLastUpdate(UserLoginStatusChangedEvents *events);

				/*!
				 * @~English
				 * 
				 * @brief Obtains the user ID of the user that started the game 
				 * @param userId Pointer that stores user IDs
				 * @details Obtains the user ID of the user that started the game. In PlayStation(R)4, this function cannot be used with applications that have the InitialUserAlwaysLoggedIn flag set to disabled in param.sfo, and SCE_USER_SERVICE_ERROR_OPERATION_NOT_SUPPORTED will return. 
				 * 
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * 
				 * @brief ゲームを起動したユーザーのユーザーIDの取得 
				 * @param userId ユーザーIDを格納するポインタ
				 * @details ゲームを起動したユーザーのユーザーIDを取得します。PlayStation(R)4では、この関数はparam.sfoでInitialUserAlwaysLoggedInフラグを無効に設定したアプリケーションでは使用できず、SCE_USER_SERVICE_ERROR_OPERATION_NOT_SUPPORTEDが返されます。 
				 * 
				 * @retval SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int getInitialUser(UserId *userId);

				/*!
				 * @~English
				 * 
				 * @brief Obtains the user name 
				 * @param userId User ID
				 * @param userName Pointer for the array to store the user name
				 * @param size Size of userName
				 * @details Gets the user name. 
				 * 
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * 
				 * @brief ユーザー名の取得 
				 * @param userId ユーザーID
				 * @param userName ユーザー名を格納する配列のポインタ
				 * @param size userNameのサイズ
				 * @details ユーザー名を取得します。 
				 * 
				 * @retval SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int getUserName(const UserId userId, char *userName, const size_t size);
#if defined(_SCE_TARGET_OS_ORBIS) && _SCE_TARGET_OS_ORBIS
				/*!
				 * @~English
				 * 
				 * @brief Get user color 
				 * @param userId User ID
				 * @param userColor Pointer that stores user color
				 * @details This obtains the user color (color allocated by the system to a user on PlayStation(R)4). Regarding user colors on PlayStation(R)4, refer to "User Management Overview" document. 
				 * 
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * 
				 * @brief ユーザーカラーの取得 
				 * @param userId ユーザーID
				 * @param userColor ユーザーカラーを格納するポインタ
				 * @details ユーザーカラー（PlayStation(R)4においてシステムがユーザーに割り当てた色）を取得します。PlayStation(R)4におけるユーザーカラーについては「ユーザー管理 概要」ドキュメントを参照してください。 
				 * 
				 * @retval SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int getUserColor(const UserId userId, UserColor *userColor);
#endif

				/*!
				 * @~English
				 * @brief Initialize
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief 初期化
				 * @retval SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int initialize();
				/*!
				 * @~English
				 * @brief Finalize
				 * @retval SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief 終了処理
				 * @retval SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int finalize();

			private:
				UserIdList						m_loginUserIdList;
				UserLoginStatusChangedEvents	m_userLoginStatusChangedEvents;
				UserId							m_initialUserId;
				int								m_returnValueFromUSGetInitialUserFunction;
			};
		}
	}
}
