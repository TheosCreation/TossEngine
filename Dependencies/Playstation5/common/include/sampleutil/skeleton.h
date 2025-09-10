/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2022 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once

#include <scebase_common.h>
#include <sampleutil/memory.h>
#include <sampleutil/graphics.h>
#include <sampleutil/input.h>
#include <sampleutil/system.h>
#include <sampleutil/debug/console.h>
#include <sampleutil/debug/api_capture.h>
#include <sampleutil/debug/perf.h>
#include <sampleutil/ui_framework/ui_framework.h>
#include <sampleutil/thread/job.h>
#include <sampleutil/audio/audio_out.h>
#include <sampleutil/script.h>
#if _SCE_TARGET_OS_PROSPERO
#include <sampleutil/helper/prospero/async_asset_loader_prospero.h>
#include <sampleutil/input/vr_tracker.h>
#endif


namespace sce
{
	namespace SampleUtil
	{
		/*!
		 * @~English
		 * @brief Sample skeleton 
		 * @details This is the sample skeleton which has a sample utility function. 
		 * @~Japanese
		 * @brief サンプルスケルトン 
		 * @details サンプルユーティリティの機能を持つサンプルスケルトンです。 
		 */
		class SampleSkeleton
		{
		public:
			/*!
			 * @~English
			 * @brief Constructor 
			 * @details This is a constructor. 
			 * @~Japanese
			 * @brief コンストラクタ 
			 * @details コンストラクタです。 
			 */
			SampleSkeleton();

			/*!
			 * @~English
			 * @brief Destructor 
			 * @details This is a destructor. 
			 * @~Japanese
			 * @brief デストラクタ 
			 * @details デストラクタです。 
			 */
			virtual ~SampleSkeleton();

			// Pure virtuals - application dependent

			/*!
			 * @~English
			 * @brief Initialization 
			 * @details This function defines the initialization processing. This is overridden by an application class for each sample. 
			 * @retval SCE_OK Success
			 * @retval (<0) Error code
			 * @~Japanese
			 * @brief 初期化 
			 * @details 初期化処理を定義する関数です。サンプルごとにアプリケーションクラスでオーバーライドされます。 
			 * @retval SCE_OK 成功
			 * @retval (<0) エラーコード
			 */
			virtual int initialize() = 0;

			/*!
			 * @~English
			 * @brief Update 
			 * @details This function defines the update processing. This is overridden by an application class for each sample. 
			 * @retval SCE_OK Success
			 * @retval (<0) Error code
			 * @~Japanese
			 * @brief 更新 
			 * @details 更新処理を定義する関数です。サンプルごとにアプリケーションクラスでオーバーライドされます。 
			 * @retval SCE_OK 成功
			 * @retval (<0) エラーコード
			 */
			virtual int update() = 0;

			/*!
			 * @~English
			 * @brief Drawing 
			 * @details This function defines the render processing. This is overridden by an application class for each sample. 
			 * @~Japanese
			 * @brief 描画 
			 * @details 描画処理を定義する関数です。サンプルごとにアプリケーションクラスでオーバーライドされます。 
			 */
			virtual void render() = 0;

			/*!
			 * @~English
			 * @brief Termination processing 
			 * @details This function defines the termination processing. This is overridden by an application class for each sample. 
			 * @retval SCE_OK Success
			 * @retval (<0) Error code
			 * @~Japanese
			 * @brief 終了処理 
			 * @details 終了処理を定義する関数です。サンプルごとにアプリケーションクラスでオーバーライドされます。 
			 * @retval SCE_OK 成功
			 * @retval (<0) エラーコード
			 */
			virtual int finalize() = 0;

			/*!
			 * @~English
			 * 
			 * @brief The enum value of the flag of the function to be used in SampleSkeleton 
			 * @details Specify this for initializeUtil(). 
			 * @~Japanese
			 * 
			 * @brief SampleSkeletonにて使用する機能のフラグのenum値 
			 * @details initializeUtil()に指定します。 
			 */
			enum SampleSkeletonFunctionFlag {

				/*!
				 * @~English
				 * @brief Graphics function flag value 
				 * @~Japanese
				 * @brief グラフィックス機能フラグ値 
				 */
				kFunctionFlagGraphics = (1<<0),

				/*!
				 * @~English
				 * @brief 2D sprite rendering function flag value 
				 * @~Japanese
				 * @brief 2Dスプライト描画機能フラグ値 
				 */
				kFunctionFlagSpriteUtility = (1<<1),

				/*!
				 * @~English
				 * @brief User ID manager flag value 
				 * @~Japanese
				 * @brief ユーザーIDマネージャーフラグ値 
				 */
				kFunctionFlagUserIdManager = (1<<3),
				/*!
				 * @~English
				 * @brief Controller function flag value of the initial user 
				 * @~Japanese
				 * @brief Initial userのコントローラー機能フラグ値 
				 */
				kFunctionFlagPadOfInitialUser = (1<<4),
				/*!
				 * @~English
				 * @brief ImGui function flag value
				 * @~Japanese
				 * @brief ImGui機能フラグ値 
				 */
				kFunctionFlagImGui = (1<<5),
				/*!
				 * @~English
				 * @brief JobQueue function flag value
				 * @~Japanese
				 * @brief JobQueue機能フラグ値
				 */
				kFunctionFlagJobQueue = (1 << 6),
				/*!
				 * @~English
				 * @brief Debug console function flag value
				 * @~Japanese
				 * @brief デバッグコンソール機能フラグ値 
				 */
				kFunctionFlagDebugConsole = (1<<7),
				/*!
				 * @~English
				 * @brief Audio out fuction flag value
				 * @~Japanese
				 * @brief オーディオアウト機能フラグ値 
				 */
				kFunctionFlagAudioOut = ( 1 << 8 ),
				/*!
				 * @~English
				 * @brief VR function flag value
				 * @~Japanese
				 * @brief VR機能フラグ値
				 */
				kFunctionFlagVr = ( 1 << 9 ),


				/*!
				 * @~English
				 * @brief Default function (graphics function, 2D sprite rendering function, user ID manager, ImGui, audio out) flag value 
				 * @~Japanese
				 * @brief デフォルト機能 (グラフィックス機能、2Dスプライト描画機能、ユーザーIDマネージャー、ImGui) フラグ値 
				 */
				kFunctionFlagDefault = kFunctionFlagGraphics | kFunctionFlagSpriteUtility | kFunctionFlagUserIdManager | kFunctionFlagImGui | kFunctionFlagAudioOut,
				
				/*!
				 * @~English
				 * @brief All function flag value 
				 * @~Japanese
				 * @brief 全機能フラグ値 
				 */
				kFunctionFlagAll =	kFunctionFlagGraphics			|
									kFunctionFlagSpriteUtility		|
									kFunctionFlagUserIdManager		|
									kFunctionFlagPadOfInitialUser	|
									kFunctionFlagImGui				|
									kFunctionFlagJobQueue           |
									kFunctionFlagAudioOut
			};

			/*!
			 * @~English
			 * 
			 * @brief Sample skeleton option 
			 * @details This is the sample skeleton option. 
			 * @~Japanese
			 * 
			 * @brief サンプルスケルトンオプション 
			 * @details サンプルスケルトンオプションです。 
			 */
			struct SampleSkeletonOption
			{
			public:
				/*!
				 * @~English
				 * @brief Graphics option 
				 * @~Japanese
				 * @brief グラフィックスオプション 
				 */
				Graphics::GraphicsContextOption	*graphicsOption;

				/*!
				 * @~English
				 * @brief PadContext option 
				 * @~Japanese
				 * @brief PadContextオプション 
				 */
				Input::PadContextOption	*padOption;

				/*!
				 * @~English
				 * @brief ImGui option 
				 * @~Japanese
				 * @brief ImGuiオプション 
				 */
				UIFramework::ImGuiOption	*imGuiOption;

				/*!
				 * @~English
				 * @brief DebugConsole option 
				 * @~Japanese
				 * @brief DebugConsoleオプション 
				 */
				Debug::ConsoleOption	*debugConsoleOption;

				/*!
				 * @~English
				 * @brief JobQueue option
				 * @~Japanese
				 * @brief JobQueue オプション
				 */
				Thread::JobQueueOption	*jobQueueOption;

				/*!
				 * @~English
				 * @brief AudioOut option
				 * @~Japanese
				 * @brief AudioOut オプション
				 */
				Audio::AudioOutContextOption	*audioOutContextOption;

				/*!
				 * @~English
				 * @brief Memory option
				 * @~Japanese
				 * @brief Memory オプション
				 */
				Memory::MemoryOption	*memoryOption;

				/*!
				 * @~English
				 * @brief Perf option
				 * @~Japanese
				 * @brief Perf オプション
				 */
				Debug::PerfOption	*perfOption;
#if _SCE_TARGET_OS_PROSPERO
				/*!
				 * @~English
				 * @brief AsyncAssetLoader option
				 * @~Japanese
				 * @brief AsyncAssetLoader オプション
				 */
				Helper::AsyncAssetLoaderOption	*asyncAssetLoaderOption;

				/*!
				 * @~English
				 * @brief VrTracker option
				 * @~Japanese
				 * @brief VrTracker オプション
				 */
				Input::VrTrackerOption	*vrTrackerOption;

				/*!
				 * @~English
				 * @brief Reprojection option
				 * @~Japanese
				 * @brief Reprojection オプション
				 */
				Graphics::ReprojectionOption	*reprojectionOption;
#endif
				/*!
				 * @~English
				 * @brief Specify if HMD is enabled by VR Setup Dialog(valid if kFunctionFlagVr is specified)
				 * @~Japanese
				 * @brief VR Setup Dialogを使用してHMDを有効化するかどうかを指定(kFunctionFlagVr指定時のみ有効)
				 */
				bool	useVrSetupDialog;
				/*!
				 * @~English
				 * @brief Constructor 
				 * @details This is a constructor. 
				 * @~Japanese
				 * @brief コンストラクタ 
				 * @details コンストラクタです。 
				 */
				SampleSkeletonOption();
			};

			/*!
			 * @~English
			 * 
			 * @brief parameter for updateUtil()
			 * @details This is the parameter to specify when calling Sample Skeleton updateUtil()
			 * @~Japanese
			 * 
			 * @brief updateUtil()のパラメータ
			 * @details サンプルスケルトンのupdateUtil()呼び出し時に指定するパラメータ
			 */
			struct SampleSkeletonUpdateParam
			{
				/*!
				 * @~English
				 * 
				 * @brief The ImGui update parameter
				 * @~Japanese
				 * 
				 * @brief ImGuiの更新パラメータ
				 */
				UIFramework::ImGuiUpdateParam *imguiParam;
			};
#if !defined(DOXYGEN_IGNORE)
			Input::PadContext *m_pPadForHud = nullptr;
			int initializeUtilInternal(uint32_t functionFlags, int32_t displayWidth, int32_t displayHeight, SampleSkeletonOption* option, const char	*pScriptFilename = nullptr);
			int renderUtilInternal(
#if _SCE_TARGET_OS_PROSPERO
									sce::Agc::Core::BasicContext	*currentCtx
#endif
									);
#endif
			
			bool								m_isScriptRunning;
			int									m_scriptWaitFrames;
			sce::SampleUtil::ApplicationScript	*m_pScript;
			std::ifstream						m_scriptFileStream;
			Input::PadData						m_scriptPadData;
			uint32_t							m_scriptPadButtonClearMask;

			/*!
			 * @~English
			 * @brief Initializes a utility. 
			 * @details This initializes each function of the utility in accordance with functionFlags. For use of the 2D sprite rendering function (kFunctionFlagSpriteRenderer), the graphics function (kFunctionFlagGraphics) is required. To use the controller feature of the initial user (kFunctionFlagPadOfInitialUser), the user ID manager (kFunctionFlagUserIdManager) is required. The controller feature of the initial user (kFunctionFlagPadOfInitialUser) cannot be used with applications that have the InitialUserAlwaysLoggedIn flag set to disabled in param.sfo. When the InitialUserAlwaysLoggedIn flag is set to disabled in param.sfo and kFunctionFlagPadOfInitialUser is specified, SCE_USER_SERVICE_ERROR_OPERATION_NOT_SUPPORTED will return.  If displayHeight or displayWidth is smaller than zero, the resolution is set automatically.
			 * @param functionFlags Flag value for the function to use
			 * @param displayWidth Width of the main render target displayed on the display
			 * @param displayHeight Height of the main render target displayed on the display
			 * @param option Pointer to the SampleSkeletonOption structure. This is initialized by the default value if NULL is specified.
			 * @retval SCE_OK Success
			 * @retval (<0) Error code
			 * @~Japanese
			 * @brief ユーティリティの初期化 
			 * @details functionFlagsの指定に従い、ユーティリティの各機能の初期化を行います。2Dスプライト描画機能(kFunctionFlagSpriteRenderer)を使用するには、グラフィックス機能(kFunctionFlagGraphics)が必要です。オーディオ機能(kFunctionFlagAudio)のストリーミング再生には、FIOS2機能(kFunctionFlagFios2)が必要です。Initial userのコントローラー機能(kFunctionFlagPadOfInitialUser)を使用するには、ユーザーIDマネージャー(kFunctionFlagUserIdManager)が必要です。Initial userのコントローラー機能(kFunctionFlagPadOfInitialUser)は、param.sfoでInitialUserAlwaysLoggedInフラグを無効に設定したアプリケーションでは使用できません。param.sfoでInitialUserAlwaysLoggedInフラグを無効に設定し、kFunctionFlagPadOfInitialUserを指定すると、SCE_USER_SERVICE_ERROR_OPERATION_NOT_SUPPORTEDが返されます。 displayWidth, displayHeightのいずれかが負の値の場合解像度は自動的に選ばれます。
			 * @param functionFlags 使用する機能のフラグ値
			 * @param displayWidth ディスプレイに表示するメインレンダーターゲットの幅
			 * @param displayHeight ディスプレイに表示するメインレンダーターゲットの高さ
			 * @param option SampleSkeletonOption構造体のポインタ。NULLを指定するとデフォルトの値で初期化されます。
			 * @retval SCE_OK 成功
			 * @retval (<0) エラーコード
			 */
#if !defined(DOXYGEN_IGNORE)
			__attribute__((always_inline))
#endif
			int initializeUtil(uint32_t functionFlags=kFunctionFlagDefault,
				int32_t displayWidth=-1,
				int32_t displayHeight=-1,
				SampleSkeletonOption* option = nullptr, const char *pScriptFilename = nullptr)
			{
#ifdef SCE_SAMPLE_UTIL_ENABLE_API_CAPTURE
				functionFlags |= kFunctionFlagGraphics | kFunctionFlagImGui;
#endif
				return initializeUtilInternal(functionFlags, displayWidth, displayHeight, option, pScriptFilename);
			}

			/*!
			 * @~English
			 * @brief Termination processing of a utility. 
			 * @details This executes termination processing of a utility. 
			 * @retval SCE_OK Success
			 * @retval (<0) Error code
			 * @~Japanese
			 * @brief ユーティリティの終了処理 
			 * @details ユーティリティの終了処理を行います。 
			 * @retval SCE_OK 成功
			 * @retval (<0) エラーコード
			 */
			int finalizeUtil();

			/*!
			 * @~English
			 * @brief Update utility 
			 * @details This updates a utility. 
			 * @param param Update parameter
			 * @retval SCE_OK Success
			 * @retval (<0) Error code
			 * @~Japanese
			 * @brief ユーティリティの更新 
			 * @details ユーティリティを更新します。 
			 * @param param updateパラメータ
			 * @retval SCE_OK 成功
			 * @retval (<0) エラーコード
			 */
			int updateUtil(SampleSkeletonUpdateParam	*param);

			/*!
			 * @~English
			 * @brief Utility rendering
			 * @details Do rendering for utility
			 * @retval SCE_OK Success
			 * @retval (<0) Error code
			 * @~Japanese
			 * @brief ユーティリティのレンダリング
			 * @details ユーティリティをレンダリングします。
			 * @retval SCE_OK 成功
			 * @retval (<0) エラーコード
			 */
#if !defined(DOXYGEN_IGNORE)
			__attribute__((always_inline))
#endif
			int renderUtil(
#if _SCE_TARGET_OS_PROSPERO
							sce::Agc::Core::BasicContext	*currentCtx = nullptr
#endif
			)
			{
#if _SCE_TARGET_OS_PROSPERO
				if (currentCtx == nullptr) currentCtx = m_graphicsContext->m_currentCtx;
#endif
#ifdef SCE_SAMPLE_UTIL_ENABLE_API_CAPTURE
				Debug::ApiCapture::RenderHudParam hudParam;
#if _SCE_TARGET_OS_PROSPERO
				hudParam.m_pDcb			= &currentCtx->m_dcb;
#endif
#if _SCE_TARGET_OS_ORBIS
				hudParam.m_pGfxc		= m_graphicsContext->m_currentCtx;
#endif
				hudParam.m_pPad			= m_pPadForHud;
				hudParam.m_targetWidth	= getGraphicsContext()->m_width;
				hudParam.m_targetHeight	= getGraphicsContext()->m_height;
				Debug::ApiCapture::renderHud(hudParam);
#endif
#if _SCE_TARGET_OS_PROSPERO
				return renderUtilInternal(currentCtx);
#endif
#if _SCE_TARGET_OS_ORBIS
				return renderUtilInternal();
#endif
			}

			/*!
			 * @~English
			 * @brief Get graphics context 
			 * @details This gets a graphics context. If kFunctionFlagGraphics is not specified at the time of initialization, NULL will be returned. 
			 * @return Graphics context 
			 * @~Japanese
			 * @brief グラフィックスコンテキストの取得 
			 * @details グラフィックスコンテキストを取得します。初期化時にkFunctionFlagGraphicsが指定されていない場合、NULLが返ります。 
			 * @return グラフィックスコンテキスト 
			 */
			SampleUtil::Graphics::GraphicsContext *getGraphicsContext();
			
			/*!
			 * @~English
			 * @brief Get user ID manager 
			 * @details This gets the user ID manager. If kFunctionFlagUserIdManager is not specified at the time of initialization, NULL will be returned. 
			 * @return User ID manager 
			 * @~Japanese
			 * @brief ユーザーIDマネージャーの取得 
			 * @details ユーザーIDマネージャーを取得します。初期化時にkFunctionFlagUserIdManagerが指定されていない場合、NULLが返ります。 
			 * @return ユーザーIDマネージャー 
			 */
			SampleUtil::System::UserIdManager *getUserIdManager();

			/*!
			 * @~English
			 * @brief Get pad context of initial user 
			 * @details This gets the pad context of the initial user. If kFunctionFlagPadOfInitialUser is not specified at the time of initialization, NULL will be returned. 
			 * @return Pad context of initial user 
			 * @~Japanese
			 * @brief Initial userのパッドコンテキストの取得 
			 * @details Initial userのパッドコンテキストを取得します。初期化時にkFunctionFlagPadOfInitialUserが指定されていない場合、NULLが返ります。 
			 * @return Initial userのパッドコンテキスト 
			 */
			SampleUtil::Input::PadContext *getPadContextOfInitialUser(int type = 0);

			/*!
			 * @~English
			 * @brief Get kaybord context of initial user 
			 * @details This gets the keyboard context of the initial user. If kFunctionFlagPadOfInitialUser is not specified  or keyborad is not hooked at the time of initialization, NULL will be returned. 
			 * @return Keyboard context of initial user 
			 * @~Japanese
			 * @brief Initial userのキーボードコンテキストの取得 
			 * @details Initial userのキーボードコンテキストを取得します。初期化時にkFunctionFlagPadOfInitialUserが指定されていないかキーボードが接続されていない場合、NULLが返ります。 
			 * @return Initial userのキーボードコンテキスト 
			 */
			SampleUtil::Input::KeyboardContext *getKeyboardContextOfInitialUser();

			/*!
			 * @~English
			 * @brief Get osk context of initial user 
			 * @details This gets the osk context of the initial user. If kFunctionFlagPadOfInitialUser is not specified at the time of initialization, NULL will be returned. 
			 * @return OSK context of initial user 
			 * @~Japanese
			 * @brief Initial userのOSKコンテキストの取得 
			 * @details Initial userのOSKコンテキストを取得します。初期化時にkFunctionFlagPadOfInitialUserが指定されていない場合、NULLが返ります。 
			 * @return Initial userのOSKコンテキスト 
			 */
			SampleUtil::Input::OskContext *getOskContextOfInitialUser();

			/*!
			 * @~English
			 * @brief Get debug console
			 * @details This gets the debug console. If kFunctionFlagDebugConsole is not specified at the time of initialization, NULL will be returned. 
			 * @return Debug console
			 * @~Japanese
			 * @brief デバッグコンソールの取得 
			 * @details デバッグコンソールを取得します。初期化時にkFunctionFlagDebugConsoleが指定されていない場合、NULLが返ります。 
			 * @return デバッグコンソール
			 */
			SampleUtil::Debug::Console *getDebugConsole();

			/*!
			 * @~English
			 * @brief Get JobQueue
			 * @details This gets a JobQueue. If kFunctionFlagJobQueue is not specified at the time of initialization, NULL will be returned.
			 * @return JobQueue
			 * @~Japanese
			 * @brief JobQueueの取得
			 * @details JobQueueを取得します。初期化時にkFunctionFlagJobQueueが指定されていない場合、NULLが返ります。
			 * @return JobQueue
			 */
			SampleUtil::Thread::JobQueue *getJobQueue();

			/*!
			 * @~English
			 * @brief Get AudioOutContext               
			 * @details This gets an AudioOut. If kFunctionFlagAudioOut is not specified at the time of initialization, NULL will be returned.
			 * @return AudioOutContext
			 * @~Japanese
			 * @brief AudioOutContextの取得
			 * @details AudioOutContextを取得します。初期化時にkFunctionFlagAudioOutが指定されていない場合、NULLが返ります。
			 * @return AudioOutContext
			 */
			SampleUtil::Audio::AudioOutContext *getAudioOut();
#if _SCE_TARGET_OS_PROSPERO
			/*!
			 * @~English
			 * @brief Get AsyncAssetLoader
			 * @details This gets an AsyncAssetLoader.
			 * @return AsyncAssetLoader
			 * @~Japanese
			 * @brief AsyncAssetLoaderの取得
			 * @details AsyncAssetLoaderを取得します。
			 * @return AsyncAssetLoader
			 */
			SampleUtil::Helper::AsyncAssetLoader *getAsyncAssetLoader();

			/*!
			 * @~English
			 * @brief Get VrTracker
			 * @details This gets a VrTracker.
			 * @return VrTracker
			 * @~Japanese
			 * @brief VrTrackerの取得
			 * @details VrTrackerを取得します。
			 * @return VrTracker
			 */
			SampleUtil::Input::VrTracker	*getVrTracker();

			/*!
			 * @~English
			 * @brief Get Reprojection
			 * @details This gets a Reprojection.
			 * @return Reprojection
			 * @~Japanese
			 * @brief Reprojectionの取得
			 * @details Reprojectionを取得します。
			 * @return Reprojection
			 */
			SampleUtil::Graphics::Reprojection	*getReprojection();

			/*!
			 * @~English
			 * @brief Get initial user's HMD device handle
			 * @details This gets initial user's HMD device handle. Negative valus is returned if it doesn't exist
			 * @return HMD device handle
			 * @~Japanese
			 * @brief Initial userのHMDデバイスハンドルの取得
			 * @details Initial userのHMDデバイスハンドルを返します。存在しない場合は負値を返します
			 * @return HMDデバイスハンドル
			 */
			int	getHmdHandleOfInitialUser() const
			{
				return m_hmdHandleOfInitialUser;
			}
#endif
			/*!
			 * @~English
			 * @brief Enables HMD
			 * @details If useVrSetupDialog in option is true, it enables HMD using VR Setup Dialog
			 * @retval true Succeeded to enable HMD
			 * @retval false Failed to enable HMD
			 * @~Japanese
			 * @brief HMDを有効化する
			 * @details オプションにuseVrSetupDialog=trueが指定されていれば、VR Setup Dialogを使用して、HMDを有効化する。
			 * @retval true HMDを有効化出来た
			 * @retval false HMDを有効化出来なかった
			 */
			bool enableHmd();

			uint32_t m_functionFlags;
			SampleUtil::Graphics::GraphicsContext *m_graphicsContext;
			SampleUtil::System::UserIdManager *m_userIdManager;
			SampleUtil::Input::PadContext *m_padContextOfInitialUser;
			SampleUtil::Input::KeyboardContext *m_keyboardContextOfInitialUser;
			SampleUtil::Input::OskContext *m_oskContextOfInitialUser;
			SampleUtil::Debug::Console *m_debugConsole;
			SampleUtil::Thread::JobQueue *m_jobQueue;
			SampleUtil::Audio::AudioOutContext *m_audioOut;
#if _SCE_TARGET_OS_PROSPERO
			SampleUtil::Helper::AsyncAssetLoader *m_asyncAssetLoader = nullptr;
			SampleUtil::Input::PadContext *m_vrControllerContextsOfInitialUser[2] = { nullptr, nullptr };
			int	m_hmdHandleOfInitialUser;
			SampleUtil::Input::VrTracker *m_vrTracker;
			SampleUtil::Graphics::Reprojection *m_reprojection;
#endif
			float m_deltaTime;
			float m_previousTime;
			float m_updateIntervalOfInput;
			uint64_t m_prevFlipCount;
			int m_prevBaseFrameBus;
			bool m_isImguiNewFrameCalled;
			bool m_useVrSetupDialog;
		};
	}
}
