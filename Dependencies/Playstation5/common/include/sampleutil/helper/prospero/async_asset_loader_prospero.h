/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <cinttypes>
#include <stddef.h>
#include <sys/types.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <kernel.h>
#include <agc/resourceregistration.h>
#include <agc/commandbuffer.h>
#include <ampr.h>
#include "sampleutil/graphics/compat.h"
#include "sampleutil/thread/thread.h"
#include "sampleutil/memory/dmem_mapper.h"

namespace sce { namespace SampleUtil { namespace Helper {
	/*!
	 * @~English
	 * @brief Structure for initializing AsyncAssetLoader
	 * @details This is the structure for initializing AsyncAssetLoader. This is used by specifying it to the argument "option" of AsyncAssetLoader::initialize().
	 * @~Japanese
	 * @brief AsyncAssetLoaderの初期化用構造体
	 * @details AsyncAssetLoaderの初期化用構造体です。AsyncAssetLoader::initialize()の引数optionに指定することで利用します。
	*/
	struct AsyncAssetLoaderOption
	{
		/*!
		 * @~English
		 * @brief The size of virtual address range reserved for AsyncAssetLoader
		 * @details Specify the size in bytes of virtual address range reserved for AsyncAssetLoader. This size has to be multiple of SCE_KERNEL_PAGE_SIZE.
		 * @~Japanese
		 * @brief AsyncAssetLoader用に予約した仮想アドレス範囲のサイズ
		 * @details AsyncAssetLoader用に予約する仮想アドレスレンジのサイズ(バイト)を指定。SCE_KERNEL_PAGE_SIZEの倍数で指定。
		 */
		size_t		virtualAddressSizeInBytes;
		/*!
		 * @~English
		 * @brief The maximum number of manageable assets by AsyncAssetLoader
		 * @details Specify maximum number of assets managed by AsyncAssetLoader.
		 * @~Japanese
		 * @brief AsyncAssetLoaderで管理するアセットの最大数
		 * @details AsyncAssetLoaderで管理するアセットの最大数を指定する。
		 */
		uint32_t	numMaxAssets;

		AsyncAssetLoaderOption()
			: virtualAddressSizeInBytes(1ul << 35/*=64GiB*/)
			, numMaxAssets(1 << 14)
		{}
	};

	/*!
	 * @~English
	 * @brief Asynchronous asset loader
	 * @details This maps direct memory and load file content to that memory afterwards asynchronously. Loaded asset is only accessible after checking load completion asynchronously.
	 * @~Japanese
	 * @brief 非同期アセットローダー
	 * @details 非同期にダイレクトメモリをマップし、アセットファイルをロードします。ロードの完了を後で非同期に確認して初めて実際にアセットにアクセスできます。
	 */
	struct AsyncAssetLoader : public Thread::LockableObject
	{
		/*!
		 * @~English
		 * @brief Load state
		 * @~Japanese
		 * @brief ロード状態
		 */
		enum class LoadState : uint64_t
		{
			/*!
			 * @~English
			 * @brief Unloaded state
			 * @~Japanese
			 * @brief 未ロード状態
			 */
			kUnloaded	= 0x0ul,
			/*!
			 * @~English
			 * @brief Loading state
			 * @~Japanese
			 * @brief ロード中状態
			 */
			kLoading	= 0x1ul,
			/*!
			 * @~English
			 * @brief Loaded state
			 * @~Japanese
			 * @brief ロード済状態
			 */
			kLoaded		= 0x2ul
		};

		/*!
		 * @~English
		 * @brief Map state
		 * @~Japanese
		 * @brief マップ状態
		 */
		enum class MapState : uint64_t
		{
			/*!
			 * @~English
			 * @brief Unmapped state
			 * @~Japanese
			 * @brief 未マップ状態
			 */
			kUnmapped	= 0x0ul,
			/*!
			 * @~English
			 * @brief Mapping state
			 * @~Japanese
			 * @brief マップ中状態
			 */
			kMapping	= 0x1ul,
			/*!
			 * @~English
			 * @brief Mapped state
			 * @~Japanese
			 * @brief マップ済状態
			 */
			kMapped		= 0x2ul,
			/*!
			 * @~English
			 * @brief Unmapping state
			 * @~Japanese
			 * @brief アンマップ中状態
			 */
			kUnmapping	= 0x3ul
		};

		/*!
		 * @~English
		 * @brief Load result
		 * @~Japanese
		 * @brief ロード結果
		 */
		struct Result
		{
			/*!
			 * @~English
			 * @brief Load target virtual address
			 * @~Japanese
			 * @brief ロード先仮想アドレス
			 */
			void		*m_ptr;

			/*!
			 * @~English
			 * @brief Load state flag
			 * @~Japanese
			 * @brief ロード状態フラグ
			 */
			Agc::Label	*m_pLoadState;
		};

		/*!
		 * @~English
		 * @brief Asset management information
		 * @~Japanese
		 * @brief アセット管理情報
		 */
		struct AssetInfo
		{
			/*!
			 * @~English
			 * @brief Load result
			 * @~Japanese
			 * @brief ロード結果
			 */
			Result				m_loadStatus;
			/*!
			 * @~English
			 * @brief Map state
			 * @~Japanese
			 * @brief マップ状態
			 */
			Agc::Label			*m_pMapState;
			/*!
			 * @~English
			 * @brief File id of asset file
			 * @~Japanese
			 * @brief アセットファイルのファイルID
			 */
			SceAprFileId		m_fileId;
			/*!
			 * @~English
			 * @brief Asset file size
			 * @~Japanese
			 * @brief アセットファイルサイズ
			 */
			size_t				m_sizeInBytes;
			/*!
			 * @~English
			 * @brief Memory type
			 * @~Japanese
			 * @brief メモリのタイプ
			 */
			SceKernelMemoryType	m_type;
			/*!
			 * @~English
			 * @brief Memory protection
			 * @~Japanese
			 * @brief メモリのプロテクション
			 */
			int					m_prot;
			/*!
			 * @~English
			 * @brief Memory for AMM command buffer
			 * @~Japanese
			 * @brief AMMコマンドバッファ用メモリ
			 */
			void				*m_pAmmCommandBuffer;
			/*!
			 * @~English
			 * @brief Memory for APR command buffer
			 * @~Japanese
			 * @brief APRコマンドバッファ用メモリ
			 */
			void				*m_pAprCommandBuffer;
			/*!
			 * @~English
			 * @brief Asset name
			 * @~Japanese
			 * @brief アセット名
			 */
			char				m_pName[256];
		}; // struct AssetInfo

		/*!
		 * @~English
		 * @brief AMM/APR event listener
		 * @~Japanese
		 * @brief AMM/APRイベントリスナー
		 */
		struct EventListener
		{
			ScePthread				m_listenerThr;
			SceKernelEqueue			m_eventQueue;
			int						m_eqId;
			ScePthreadCond			m_cond;
			ScePthreadMutex			m_lockForCond;

			enum Condition
			{
				kEqual,
				kNotEqual
			};

			struct WaitSpec
			{
				Agc::Label						*m_pState;
				Condition						m_waitCondition;
				uint64_t						m_checkValue;
				std::function<void(uintptr_t)>	m_onWaitDone;
				uintptr_t						m_onWaitDoneArg;
			};
			std::vector<WaitSpec>	m_waiters;
			bool					m_isValid;

			/*!
			 * @~English
			 * @brief Initializes event listener
			 * @param eqId Event queue id to wait for
			 * @~Japanese
			 * @param eqId 待機するイベントキューID
			 * @brief イベントリスナー初期化
			 */
			void	initialize(int	eqId);
			/*!
			 * @~English
			 * @brief Finalizes event queue id
			 * @~Japanese
			 * @brief イベントリスナー終了処理
			 */
			void	finalize();
			/*!
			 * @~English
			 * @brief Wait while specified condition is met
			 * @param spec Specify the condition to wait for
			 * @details Wait while condition is met by blocking.
			 * @~Japanese
			 * @brief 指定した条件が成り立つ間待つ
			 * @param spec 待つべき条件を指定
			 * @details 指定した条件が成り立っている間、ブロッキングで待ちます。
			 */
			void	waitIf(WaitSpec	spec);
			/*!
			 * @~English
			 * @brief Event listener thread entry function
			 * @~Japanese
			 * @brief イベントリスナースレッドエントリ関数
			 */
			void	run();
		}; // struct EventListener

		sce::Agc::ResourceRegistration::OwnerHandle		m_owner;
		EventListener									m_ammListener;
		EventListener									m_aprListener;
		void											*m_mappableVirtualAddressStart;
		size_t											m_mappableVirtualAddressSizeInBytes;
		size_t											m_usedVirtualAddressSizeInBytes;
		void											*m_pDeviceMemory;
		size_t											m_deviceMemorySizeInBytes;
		size_t											m_usedDeviceMemorySizeInBytes;
		std::unordered_map<std::string, AssetInfo>		m_assets;
		std::unordered_map<const void *, Agc::Label *>	m_loadStates;

		/*!
		 * @~English
		 * @brief Constructor
		 * @param option Constructor parameter
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param option コンストラクタパラメータ
		 */
		AsyncAssetLoader(const AsyncAssetLoaderOption	&option);
		/*!
		 * @~English
		 * @brief Destructor
		 * @~Japanese
		 * @brief デストラクタ
		 */
		~AsyncAssetLoader();

		/*!
		 * @~English
		 * @brief Map direct memory for asset load
		 * @details Starts mapping direct memory asynchronously
		 * @param tag Tag to identify asset
		 * @param sizeInBytes Size of memory to map(in bytes)
		 * @param alignment Memory alignment
		 * @param type Memory type
		 * @param prot Memory protection
		 * @param resourceTypes Resource types to be registered
		 * @return Map result
		 * @~Japanese
		 * @brief アセットロード用のダイレクトメモリのマップ
		 * @details ダイレクトメモリのマップを非同期に開始する。
		 * @param tag アセット識別用のタグ
		 * @param sizeInBytes マップするメモリサイズ(バイト)
		 * @param alignment メモリのアラインメント
		 * @param type メモリのタイプ
		 * @param prot メモリのプロテクション
		 * @param resourceTypes 登録するリソースタイプ
		 * @return マップ結果
		 */
		Result	allocate(const char	*tag, size_t	sizeInBytes, uint32_t	alignment, SceKernelMemoryType	type, int	prot, const std::vector<sce::Agc::ResourceRegistration::ResourceType>	&resourceTypes = {});

		/*!
		 * @~English
		 * @brief Asset load
		 * @details Starts loading asset to memory mapped by 'allocate' asynchronously
		 * @param tag Tag to identify asset
		 * @param assetFilePath Asset file path
		 * @param offsetInBytes Offset position in asset to start loading(in bytes)
		 * @param sizeInBytes Load size(in bytes)
		 * @retval SCE_OK Success.
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief アセットのロード
		 * @details allocateでマップしたメモリにアセットのロードを非同期に開始する。
		 * @param tag アセット識別用のタグ
		 * @param assetFilePath アセットのファイルパス
		 * @param offsetInBytes アセットの内のロードを開始するオフセット位置(バイト)
		 * @param sizeInBytes ロードサイズ(バイト)
		 * @retval SCE_OK 成功。
		 * @retval (<0) エラーコード
		 */
		int	load(const char	*tag, const char	*assetFilePath, off_t	offsetInBytes, size_t	sizeInBytes);

		/*!
		 * @~English
		 * @brief Asynchronous asset file load
		 * @details Starts mapping direct memory and loading asset file afterwards asynchronously
		 * @param assetFilePath Asset file path name
		 * @param alignment Memory alignment
		 * @param type Memory type
		 * @param prot Memory protection
		 * @param resourceTypes Resource types to be registered
		 * @return Load result
		 * @~Japanese
		 * @brief 非同期アセットファイルロード
		 * @details ダイレクトメモリのマップ・アセットファイルのロードを非同期に開始する。
		 * @param assetFilePath アセットファイルパス名
		 * @param alignment メモリのアラインメント
		 * @param type メモリのタイプ
		 * @param prot メモリのプロテクション
		 * @param resourceTypes 登録するリソースタイプ
		 * @return ロード結果
		 */
		Result	acquire(const char	*assetFilePath, uint32_t	alignment, SceKernelMemoryType type, int prot, const std::vector<sce::Agc::ResourceRegistration::ResourceType>	&resourceTypes = {}, bool	noLoad = false)
		{
			Result	result = allocate(assetFilePath, 0ul, alignment, type, prot, resourceTypes);
			int ret = load(!noLoad ? assetFilePath : nullptr, assetFilePath, 0l, 0ul);
			return (ret == SCE_OK) ? result : Result{};
		}
		Result	acquireNoLoad(const char	*assetFilePath,uint32_t	alignment, SceKernelMemoryType	type, int	prot, const std::vector<sce::Agc::ResourceRegistration::ResourceType>	&resourceTypes = {})
		{
			return	acquire(assetFilePath, alignment, type, prot, resourceTypes, true);
		}

		/*!
		 * @~English
		 * @brief Starts releasing asset
		 * @details Starts unmapping direct memory allocated for specified asset.
		 * @param assetFilePath Asset file path name
		 * @param pKickLabel Unmapping starts when non-zero value is written to this label.
		 * @retval SCE_OK Success.
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief アセットの解放を開始
		 * @details アセット領域のダイレクトメモリを解放を開始する。
		 * @param assetFilePath アセットファイルパス名
		 * @param pKickLabel このラベルに非ゼロがセットされた後に解放が開始される。
		 * @retval SCE_OK 成功。
		 * @retval (<0) エラーコード
		 */
		int	release(const char *assetFilePath, Agc::Label	*pKickLabel = nullptr);
	}; // struct AsyncAssetLoader
}}} // namespace sce::SampleUtil::Helper
