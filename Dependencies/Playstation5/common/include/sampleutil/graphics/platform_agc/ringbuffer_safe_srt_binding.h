/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2025 Sony Interactive Entertainment Inc.
 * 
 */
#pragma once

#include <vector>

#include <agc/core/binder.h>
#include <agc/core/ringbuffer.h>
#include <sampleutil/sampleutil_common.h>

namespace sce { namespace SampleUtil { namespace Graphics {
	/*!
	 * @~English
	 * @brief The base class which gives the ability to patch pointers to resource data classes
	 * @details Starts pointer patch with beginPatch() for specified shader stages and does pointer patchings when endPatch() is called based on m_patchList. DeferredSrtPatchBinder and DeferredPatchSrt template class inherit this class.
	 * @~Japanese
	 * @brief リソースデータクラスにポインタパッチ機能を付与する基底クラス
	 * @details beginPatch()で対象のシェーダステージを指定してパッチを開始し、endPatch()呼び出し時にm_patchListの指示に従い、ポインタパッチを行う。このクラスを継承してDeferredSrtPatchBinderやDeferredPatchSrtテンプレートクラスが作られる。
	 */
	struct Patchable
	{
		/*!
		 * @~English
		 * @brief Patch pointer
		 * @details Contains information needed to patch pointer
		 * @~Japanese
		 * @brief パッチポインター
		 * @details ポインタパッチに必要な情報を保持
		 */
		struct PatchPointer
		{
			/*!
			 * @~English
			 * @brief The index in target buffer indicating pointer which is to be patched.
			 * @~Japanese
			 * @brief パッチするポインタのターゲットバッファ中での位置を示すインデックス
			 */
			int		m_targetIndex;
			/*!
			 * @~English
			 * @brief The pointer value to be patched
			 * @~Japanese
			 * @brief パッチ時に書き込むポインタ値
			 */
			void	*m_patchValue;
		};

		/*!
		 * @~English
		 * @brief Patch pointer list
		 * @~Japanese
		 * @brief パッチポインタ―リスト
		 */
		std::vector<PatchPointer>	m_patchList[(uint32_t)Agc::ShaderType::kLegacyCount];
		uint32_t					m_activeStages = 0;

		/*!
		 * @~English
		 * @brief Returns patch target buffer
		 * @param type shader type
		 * @return Target buffer to be patched
		 * @details Returns target buffer to be patched for specified shader stage. defined in derived class.
		 * @~Japanese
		 * @brief パッチターゲットバッファを取得する
		 * @param type シェーダータイプ
		 * @return パッチすべきターゲットバッファ
		 * @details 指定したシェーダーステージのパッチすべきターゲットバッファを取得する仮想関数。実体は派生先で定義
		 */
		virtual void						*getTargetBuffer(Agc::ShaderType	type) = 0;

		/*!
		 * @~English
		 * @brief Starts patch
		 * @param activeStages Shader stages where patching is applied.
		 * @details Clears 'm_patchList' and start new accumulation to patch list
		 * @~Japanese
		 * @brief パッチ開始
		 * @param activeStages パッチ対象のシェーダステージ
		 * @details m_patchListをクリアし、新たなpatch listの収集を開始します。
		 */
		void	beginPatch(uint32_t	activeStages)
		{
			m_activeStages = activeStages;
			for (int type = 0; type < (int)Agc::ShaderType::kLegacyCount; type++)
			{
				if ((m_activeStages & (1u << type)) != 0)
				{
					m_patchList[type].clear();
				}
			}
		}

		/*!
		 * @~English
		 * @brief Ends patch
		 * @details Patches pointers in target buffer returned by getTargetBuffer() virtual function based on contets of 'm_patchList'.
		 * @~Japanese
		 * @brief パッチ終了
		 * @details m_patchListの内容に従い、getTargetBuffer()仮想関数の戻り値で示されるターゲットバッファ中のポインタをパッチ
		 */
		void	endPatch()
		{
			for (int type = 0; type < (int)Agc::ShaderType::kLegacyCount; type++)
			{
				if ((m_activeStages & (1u << type)) != 0)
				{
					void	*pTargetBuffer = getTargetBuffer((Agc::ShaderType)type);
					for (const auto &patch : m_patchList[type])
					{
						memcpy(&((uint8_t*)pTargetBuffer)[patch.m_targetIndex * sizeof(void *)], &patch.m_patchValue, sizeof(void *));
					}
				}
			}
		}

		void	patch(off_t	patchOffset, const void	*pPatchValue, bool	is48bitsAddress)
		{
			for (int type = 0; type < (int)Agc::ShaderType::kLegacyCount; type++) {
				if ((m_activeStages & (1u << type)) != 0) {
					void *pTargetBuffer = getTargetBuffer((Agc::ShaderType)type);
					if (pTargetBuffer != nullptr) {
						memcpy(&((uint8_t*)pTargetBuffer)[patchOffset], &pPatchValue, is48bitsAddress ? 6 : 8);
					}
				}
			}
		}
	};

	/*!
	 * @~English
	 * @brief Deferred area allocatable and patch capable SRT data
	 * @tparam T SRT structure type
	 * @details Has shadow of SRT data, defers SRT allocation by allocating "real" SRT, and copying content from shadow on destructor call, and patches pointers.
	 *
	 * If Ring Buffer segment switch occurs before SRT area allocated by TwoSidesAllocator is consumed by draw, we have forward segment reference situation.
	 * To avoid this, we create SRT shaodw for temporary storage, and defer "real" SRT area allocation until destructor call.
	 * On "real" SRT allocation, pointer of this SRT in upper SRT or Binder's User Data(oe EUD) is also patched.
	 * @code
	 * {
	 * 		DeferredPatchSrt<MySrt> mySrt(dcb, pUpperSrt, offsetof(UpperSrtType, m_pMySrt), 1u << (int)Agc::ShaderType::kPs); // shadow for PS is created here
	 * 		
	 * 		// fills shadow SRT
	 * 		mySrt.m_foo = ...;
	 * 		mySrt.m_bar = ...;
	 * 
	 * 		dcb.drawIndex(...);
	 * } // Destructor is called here, and "real" SRT is allocated. Since this is after 'drawIndex', we can gurantee that backward segment referecne is aovided.
	 * @endcode
	 * @~Japanese
	 * @brief 遅延領域確保・パッチ機能付きSRTデータ
	 * @tparam T SRTの構造体型
	 * @details 内部にSRTデータのシャドウを持ち、デストラクタ呼び出し時にTwoSidedAllocatorで本当のSRT領域を確保し、ここにコピーすることで、SRT領域確保を遅延させ、合わせてパッチも行う。
	 *
	 * TwoSidedAllocatorで確保したSRT領域をdrawで実際に使用するより前に、Ring Bufferのセグメント切り替えが起こると、前方セグメントへの参照が起こる。
	 * これを避けるため、SRTを一時領域にシャドウとして作成し、デストラクタ呼び出し時まで実SRT領域の確保を遅延させる。
	 * また、実SRT領域を確保した際には、それより上位のSRTやBinderの持つUser Data(or EUD)内の自身を指すポインタもパッチされます。
	 * @code
	 * {
	 * 		DeferredPatchSrt<MySrt> mySrt(dcb, pUpperSrt, offsetof(UpperSrtType, m_pMySrt), 1u << (int)Agc::ShaderType::kPs); // ここでPS用のシャドウが作られる
	 * 		
	 * 		// シャドウSRTに値を埋める
	 * 		mySrt.m_foo = ...;
	 * 		mySrt.m_bar = ...;
	 * 
	 * 		dcb.drawIndex(...);
	 * } // ここでデストラクタが呼ばれ、実SRT領域が確保されるので、drawより後になり、後方セグメント参照の回避が保証される
	 * @endcode
	 */
	template<typename T>
	struct DeferredPatchSrt : public Patchable, public T
	{
		Agc::TwoSidedAllocator	&m_allocator;
		Patchable				*m_pParent;
		int						m_myIndex[(uint32_t)Agc::ShaderType::kLegacyCount];
		void					*m_pRealSrt = nullptr;

		/*!
		 * @~English
		 * @brief Constructor
		 * @param alloc Allocator used to allocate "real" SRT area
		 * @param pParent Upper SRT(DeferredPatchSrt) or Binder(DeferredSrtPatchStageBinder) whose User Data has SRT setup
		 * @param patchOffset The offset address of this SRT's pointer in pParent
		 * @param activeStages Shader stages where patching is effective
		 * @details Adds an entry to pParent's patch list which stores information used to patch myself later
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param alloc 実SRT領域確保に使用するアロケータ
		 * @param pParent 上位のSRT(DeferredPatchSrt)もしくはUser DataにSRTを設定したBinder(DeferredSrtPatchStageBinder)
		 * @param patchOffset pParent内で本SRTのポインタがあるオフセットアドレス
		 * @param activeStages パッチを行うシェーダステージ
		 * @details pParentのパッチリストに自身のポインタを後でパッチするためのエントリを追加する。
		 */
		DeferredPatchSrt(Agc::TwoSidedAllocator	&alloc, Patchable	*pParent, off_t	patchOffset, uint32_t	activeStages)
			: m_allocator(alloc), m_pParent(pParent)
		{
			beginPatch(activeStages);
			SCE_SAMPLE_UTIL_ASSERT(m_pParent != nullptr);
			for (int type = 0; type < (int)Agc::ShaderType::kLegacyCount; type++)
			{
				if ((m_activeStages & (1u << type)) != 0)
				{
					m_pParent->m_patchList[type].push_back({ (int)(patchOffset / sizeof(void *)), nullptr });
					m_myIndex[type] = m_pParent->m_patchList[type].size() - 1;
				}
			}
		}

		/*!
		 * @~English
		 * @brief Returns patch target buffer
		 * @param type Not used
		 * @return "real" SRT buffer
		 * @details Allocates "real" SRT buffer, and copy the content of shadow.
		 * @~Japanese
		 * @brief パッチターゲットバッファを取得する
		 * @param type 無視される
		 * @return 本当のSRTバッファ
		 * @details 本当のSRTバッファを確保し、シャドウの内容をコピーする
		 */
		virtual void	*getTargetBuffer(Agc::ShaderType	type)
		{
			(void)type;
			if (m_pRealSrt == nullptr)
			{
				// allocate real SRT and copy content from shadow
				m_pRealSrt = reinterpret_cast<void *>(m_allocator.allocateTopDown(sizeof(T), alignof(T)));
				memcpy(m_pRealSrt, static_cast<T*>(this), sizeof(T));
			}

			return m_pRealSrt;
		}

		/*!
		 * @~English
		 * @brief Destructor
		 * @details Allocates "real" SRT area, and copies shadow content to it, and updates this SRT's entry in m_pParent's patch list.
		 * @~Japanese
		 * @brief デストラクタ
		 * @details 実SRT領域を確保し、シャドウから内容がコピーされる。m_pParentのパッチリストの自身のエントリも更新される。
		 */
		virtual ~DeferredPatchSrt()
		{
			for (int type = 0; type < (int)Agc::ShaderType::kLegacyCount; type++)
			{
				if ((m_activeStages & (1u << type)) != 0)
				{
					m_pParent->m_patchList[type][m_myIndex[type]].m_patchValue = getTargetBuffer((Agc::ShaderType)type);
				}
			}
			endPatch();
		}
	};

	/*!
	 * @~English
	 * @brief Deferred SRT patch capable Binder
	 * @tparam BaseBinder Base Binder class
	 * @details RingBuffer's auto submit is disabled from beginPatch() to endPatch(), and SRT in User Data(or EUD) is patched on endPatch().
	 * @code
	 * 	DeferredSrtPatchBinder<Agc::Core::Binder> bdr;
	 * 	bdr.init(dcb, dcb, ringBuffer);
	 * 	bdr.beginPatch(1u << (int)Agc::ShaderType::kPs); // stops auto-submit, and starts patching here
	 * 	bdr.setShader(shader);
	 *  {
	 * 		RootSrtType rootSrt;
	 * 		// fills root SRT
	 * 		rootSrt.m_foo = ....;
	 * 		rootSrt.m_bar = ....;
	 * 		bdr.setUserSrtBuffer(&rootSrt, sizeof(rootSrt) / 4); // sets root SRT -> will be patched later.
	 * 
	 * 		DeferredPatchSrt<LeafSrtType> leafSrt(dcb, &bdr, offsetof(RootSrtType, m_pLeaf), 1u << (int)Agc::ShaderType::kPs); // defines leaf SRT which root SRT has 'm_pLeaf' member pointer variable for self reference of
	 * 		// fills leaf SRT shadow
	 * 		leafSrt.m_foo = ....;
	 * 		leafSrt.m_bar = ....;
	 * 
	 * 		dcb.drawIndex(...); // draw
	 *  } // Destructor is called, and "real" SRT is allocated.
	 *  bdr.endPatch(); // Ends patching, and pointers on User Data(or EUD) are patched, then autosubmit suppression is released.
	 *  bdr.fini();
	 * @endcode
	 * @~Japanese
	 * @brief 遅延SRTパッチ機能付きBinder
	 * @tparam BaseBinder 派生元のBinder
	 * @details beginPatch()からendPatch()までの間、RingBufferの自動サブミットを抑止し、User Dataのパッチを可能にし、endPatch()時にUserData(or EUD)上のSRTにパッチを行う。
	 * @code
	 * 	DeferredSrtPatchBinder<Agc::Core::Binder> bdr;
	 * 	bdr.init(dcb, dcb, ringBuffer);
	 * 	bdr.beginPatch(1u << (int)Agc::ShaderType::kPs); // ここで自動サブミットが停止され、パッチを開始する
	 * 	bdr.setShader(shader);
	 *  {
	 * 		RootSrtType rootSrt;
	 * 		// root SRTに値を埋める
	 * 		rootSrt.m_foo = ....;
	 * 		rootSrt.m_bar = ....;
	 * 		bdr.setUserSrtBuffer(&rootSrt, sizeof(rootSrt) / 4); // root SRTをセット -> 後でパッチされる。
	 * 
	 * 		DeferredPatchSrt<LeafSrtType> leafSrt(dcb, &bdr, offsetof(RootSrtType, m_pLeaf), 1u << (int)Agc::ShaderType::kPs); // root SRT内にm_pLeafで自身のポインタを持つleaf SRTを宣言
	 * 		// leaf SRTのシャドウに値を埋める
	 * 		leafSrt.m_foo = ....;
	 * 		leafSrt.m_bar = ....;
	 * 
	 * 		dcb.drawIndex(...); // 描画
	 *  } // デストラクタが呼ばれ、実SRTの領域が確保される。
	 *  bdr.endPatch(); // パッチを終了し、User Data(or EUD)上のポインタがパッチされ、自動サブミットの抑止が解除される。
	 *  bdr.fini();
	 * @endcode
	 */
	template<typename BaseBinder>
	class DeferredSrtPatchBinder : public BaseBinder, public Patchable
	{
		Agc::Core::RingBuffer							*m_pRingbuffer;
		Agc::Core::RingBuffer::SegmentChangeCallback	*m_segmentChangeCallbacksSave;
		void											**m_segmentChangePayloadsSave;
		uint32_t										m_numSegmentChangeCallbacksSave;
		uint32_t										m_numSegmentCbSave;
		Agc::Core::RingBuffer::AutoSubmit				m_autoSubmitModeSave;
		uint16_t										m_autoSubmitThresholdSave;
	public:

		/*!
		 * @~English
		 * @brief Begins patch
		 * @param activeStages Shader stages where patching is effective
		 * @details Disables auto-submit of 'm_ringbuffer', and clears 'm_patchlist'.
		 * @~Japanese
		 * @brief パッチ開始
		 * @param activeStages パッチを行うシェーダステージ
		 * @details m_ringbufferの自動サブミットを抑止し、m_patchListをクリアします。
		 */
		void	beginPatch(uint32_t	activeStages)
		{
			if (m_pRingbuffer != nullptr)
			{
				m_autoSubmitModeSave		= m_pRingbuffer->m_autoSubmitMode;
				m_autoSubmitThresholdSave	= m_pRingbuffer->m_autoSubmitThreshold;
				m_pRingbuffer->setAutoSubmitMode(Agc::Core::RingBuffer::AutoSubmit::kDisable, 0);
			}
			Patchable::beginPatch(activeStages);
		}

		/*!
		 * @~English
		 * @brief Returns patch target buffer
		 * @param type Shader Type
		 * @return Pointer to SRT in User Data(or EUD)
		 * @details Returns pointer to SRT in User Data(or EUD) of specified shader type
		 * @~Japanese
		 * @brief パッチターゲットバッファを取得する
		 * @param type シェーダーステージ
		 * @return User Data(or EUD)上のSRTへのポインタ
		 * @details 指定したシェーダーステージのUser Data(or EUD)上のSRTへのポインタを返します。
		 */
		virtual void	*getTargetBuffer(Agc::ShaderType	type)
		{
			auto &stageBinder = BaseBinder::getStage(type);
			// compute patch target srt pointer
			uint16_t regStart = Agc::Core::getResourceUserDataStart(stageBinder.getShader(), Agc::UserDataLayout::DirectResourceType::kShaderResourceTable);
			if (regStart == 0xffff) return nullptr;
			if (regStart < Agc::Constants::kNumUserDataSlotsAvailable)
			{
				return (void *)(stageBinder.getUserDataPointer() + regStart);
			} else {
				return (void *)(stageBinder.getEudPointer() + regStart - Agc::Constants::kNumUserDataSlotsAvailable);
			}
		}

		/*!
		 * @~English
		 * @brief Ends patch
		 * @details Patches User Data(or EUD) with information in 'm_patchList', and restores auto submit settings.
		 * @~Japanese
		 * @brief パッチ終了
		 * @details User Data(or EUD)をm_patchListに従いパッチし、AutoSubmitを復帰する
		 */
		void	endPatch()
		{
			Patchable::endPatch();
			if (m_pRingbuffer != nullptr)
			{
				// restore autosubmit mode
				m_pRingbuffer->setAutoSubmitMode(m_autoSubmitModeSave, m_autoSubmitThresholdSave);
			}
		}

		static void	segmentChangeCallback(void	*payload, Agc::TwoSidedAllocator	*buffer)
		{
			DeferredSrtPatchBinder	*pSelf = reinterpret_cast<DeferredSrtPatchBinder *>(payload);
			pSelf->handleLostSegment(buffer);
			for (uint32_t i = 0; i <= (uint32_t)sce::Agc::ShaderType::kLast; ++i) {
				if (pSelf->isStageActive((sce::Agc::ShaderType)i)) {
					pSelf->getStage((sce::Agc::ShaderType)i).forkUserData();
				}
			}
		}

		/*!
		 * @~English
		 * @brief Initialize
		 * @param cb Command Buffer
		 * @param scratch Scratch
		 * @param pRingbuffer Ring Buffer
		 * @return \*this
		 * @details Sets up ring buffer's segment change callback
		 * @~Japanese
		 * @brief 初期化関数
		 * @param cb コマンドバッファ
		 * @param scratch スクラッチ
		 * @param pRingbuffer リングバッファ
		 * @return \*this
		 * @details リングバッファのsegment changeコールバックを設定します。
		 */
		DeferredSrtPatchBinder	&init(Agc::CommandBuffer *cb, Agc::TwoSidedAllocator *scratch, Agc::Core::RingBuffer	*pRingbuffer)
		{
			BaseBinder::init(cb, scratch);
			m_pRingbuffer = pRingbuffer;
			if (m_pRingbuffer != nullptr)
			{
				// save segment change callback
				m_segmentChangeCallbacksSave	= m_pRingbuffer->m_segmentChangeCallbacks;
				m_numSegmentChangeCallbacksSave	= m_pRingbuffer->m_numSegmentChangeCallbacks;
				m_segmentChangePayloadsSave 	= m_pRingbuffer->m_segmentChangePayloads;
				uint32_t numCallbacks = m_pRingbuffer->m_numSegmentChangeCallbacks + 1;
				// copy segment callbacks and append new callback
				Agc::Core::RingBuffer::SegmentChangeCallback *pCallbacks = new Agc::Core::RingBuffer::SegmentChangeCallback[numCallbacks];
				void **pCallbackPayloads = new void *[numCallbacks];
				memcpy(pCallbacks, m_pRingbuffer->m_segmentChangeCallbacks, sizeof(Agc::Core::RingBuffer::SegmentChangeCallback) * (numCallbacks - 1));
				memcpy(pCallbackPayloads, m_pRingbuffer->m_segmentChangePayloads, sizeof(void *) * (numCallbacks - 1));
				pCallbacks[numCallbacks - 1] = segmentChangeCallback;
				pCallbackPayloads[numCallbacks - 1] = this;
				m_pRingbuffer->setSegmentChangeCallbacks(pCallbacks, pCallbackPayloads, numCallbacks);
			}
			return *this;
		}

		/*!
		 * @~English
		 * @brief Finalize
		 * @details Restores ring buffer's segment change callback.
		 * @~Japanese
		 * @brief 終了処理
		 * @details リングバッファのsegment change callbackを回復します。
		 */
		void fini()
		{
			if (m_pRingbuffer != nullptr)
			{
				// restore segment change callback
				delete [] m_pRingbuffer->m_segmentChangeCallbacks;
				delete [] m_pRingbuffer->m_segmentChangePayloads;
				m_pRingbuffer->setSegmentChangeCallbacks(m_segmentChangeCallbacksSave, m_segmentChangePayloadsSave, m_numSegmentChangeCallbacksSave);
			}
		}
	};
} } } // namespace sce::SampleUtil::Graphics