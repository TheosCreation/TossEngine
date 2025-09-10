/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. 
 * 
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <scebase_common.h>
#include <kernel.h>
#include <razorcpu.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/drawcommandbuffer.h>
#endif
#if _SCE_TARGET_OS_ORBIS
#include <gnmx/context.h>
#endif
#include "sampleutil/graphics/compat.h"

//! @defgroup perf Peformance instrument
//! @{
/*!
 * @~English
 * @brief Tags this class
 * @details This macro has to be called inside constructor of a class which you want to tag.
 * @~Japanese
 * @brief 現在のクラスをタグ付け
 * @details このマクロはタグ付けしたいクラスのコンストラクタ内で呼び出します。
 */
#define TAG_THIS_CLASS sce::SampleUtil::Debug::TagClass<decltype(this)> __tag(__FUNCTION__, this, sizeof(this))
/*!
 * @~English
 * @brief Tags this class
 * @details Use this for derived class
 * @~Japanese
 * @brief 現在のクラスをタグ付け
 * @details 派生クラスの場合に使用する
 */
#define TAG_THIS_DERIVED_CLASS UNTAG_THIS_CLASS; TAG_THIS_CLASS
/*!
 * @~English
 * @brief Tags this class
 * @param name Name
 * @~Japanese
 * @brief 現在のクラスを名前付きタグ付け
 * @param name 名前
 */
#define TAG_THIS_CLASS_WITH_NAME(name) sce::SampleUtil::Debug::TagClass<decltype(this)> __tag(std::string(__FUNCTION__) + ":" + name, this, sizeof(this))
/*!
 * @~English
 * @brief Tags this class
 * @details Use this for derived class
 * @param name Name
 * @~Japanese
 * @brief 現在のクラスを名前付きタグ付け
 * @details 派生クラスの場合に使用する
 * @param name 名前
 */
#define TAG_THIS_DERIVED_CLASS_WITH_NAME(name) UNTAG_THIS_CLASS; TAG_THIS_CLASS_WITH_NAME(name)
/*!
 * @~English
 * @brief Untags this class
 * @details This macro has to be called inside destructor of a class which you want to untag.
 * @~Japanese
 * @brief 現在のクラスのタグ付けを解除
 * @details このマクロはタグ付け解除をしたいクラスのデストラクタで呼び出します。
 */
#define UNTAG_THIS_CLASS sce::SampleUtil::Debug::UnTagClass<decltype(this)> __untag(this)
/*!
 * @~English
 * @brief Untags this class
 * @details Use this for derived class
 * @~Japanese
 * @brief 現在のクラスのタグ付けを解除
 * @details 派生クラスの場合に使用する
 */
#define UNTAG_THIS_DERIVED_CLASS // derived class will be untaged in super class destructor

//! @}

namespace
{
	uint32_t hsv2rgb(uint32_t hsv)
	{
		float h = (float)((hsv >> 16) & 0xffu) / 255.f;
		float s = (float)((hsv >> 8) & 0xffu) / 255.f;
		float v = (float)(hsv & 0xffu) / 255.f;
		float r, g, b;
		if (s == 0.f)
		{
			/* Grayscale */
			r = g = b = v;
		} else {
			if (1.f <= h)
			{
				h -= 1.f;
			}
			h *= 6.f;
			float i = floor(h);
			float f = h - i;
			float aa = v * (1 - s);
			float bb = v * (1 - (s * f));
			float cc = v * (1 - (s * (1 - f)));
			if (i < 1)
			{
				r = v; g = cc; b = aa;
			} else if (i < 2) {
				r = bb; g = v; b = aa;
			} else if (i < 3) {
				r = aa; g = v; b = cc;
			} else if (i < 4) {
				r = aa; g = bb; b = v;
			} else if (i < 5) {
				r = cc; g = aa; b = v;
			} else {
				r = v; g = aa;  b = bb;
			}
		}

		return (((uint32_t)(r * 255.f)) << 16) | (((uint32_t)(g * 255.f)) << 8) | (uint32_t)(b * 255.f);
	}
}

namespace sce { namespace SampleUtil { namespace Debug {

/*!
 * @~English
 *
 * @brief Structure for initializing Performance instrumentation
 * @details This is the structure for initializing Performance instrumentation. This is used by specifying it to the "perfOption" member variable of SampleSkeletonOption.
 * @~Japanese
 *
 * @brief パフォーマンス測定の初期化用構造体
 * @details パフォーマンス測定の初期化用構造体です。SampleSkeletonOptionのperfOptionに指定します。
 */
struct PerfOption
{
	/*!
	 * @~English
	 * @brief Specify whether Razor CPU user marker is used or not
	 * @~Japanese
	 * @brief Razor CPUユーザーマーカーを使用するかどうかを指定
	 */
	bool useCpuMarker;
	/*!
	 * @~English
	 * @brief Specify whether Razor GPU user marker is used or not
	 * @~Japanese
	 * @brief Razor GPUユーザーマーカーを使用するかどうかを指定
	 */
	bool useGpuMarker;
	/*!
	 * @~English
	 * @brief Specify whether Razor CPU data sampling buffer tagging is used or not
	 * @~Japanese
	 * @brief Razor CPUデータサンプリングのバッファタグ付けを行うかどうかを指定
	 */
	bool useDataTag;
	/*!
	 * @~English
	 * @brief Maximum number of Razor CPU data tags
	 * @~Japanese
	 * @brief Razor CPUデータサンプリングの最大タグ数
	 */
	uint32_t maxNumDataTags;
	/*!
	 * @~English
	 * @brief Specify whether Mat is used or not
	 * @~Japanese
	 * @brief Matを行うかどうかを指定
	 */
	bool useMat;
	/*!
	 * @~English
	 * @brief Specify whether sceRazorCpuSync() is called
	 * @~Japanese
	 * @brief sceRazorCpuSync()を呼ぶかどうかを指定
	 */
	bool disableRazorCpuSync;

	/*!
	 * @~English
	 * @brief Constructor
	 * @details This is a constructor.
	 * @~Japanese
	 * @brief コンストラクタ
	 * @details コンストラクタです。
	 */
	PerfOption() :
		useCpuMarker(true),
		useGpuMarker(true),
		useDataTag(true),
		maxNumDataTags(100000),
		useMat(true),
		disableRazorCpuSync(false)
	{}
};

/*!
 * @~English
 * @brief Marker target
 * @~Japanese
 * @brief マーカー対象
 */
enum MarkerTarget
{
	/*!
	 * @~English
	 * @brief GPU
	 * @~Japanese
	 * @brief GPU
	 */
	kGpuMarker = 1,
	/*!
	 * @~English
	 * @brief CPU
	 * @~Japanese
	 * @brief CPU
	 */
	kCpuMarker = 2
};

/*!
 * @~English
 * @brief Performance instrument
 * @~Japanese
 * @brief パフォーマンス計測
 */
struct	Perf
{
	struct Head
	{
		uint32_t	lev;
		uint32_t	hue;
		bool		isPopped;
	};

	/*!
	 * @~English
	 * @brief Performance instrument context
	 * @details Create this instance per thread
	 * @~Japanese
	 * @brief パフォーマンス計測コンテキスト
	 * @details スレッドごとに作成
	 */
	struct Context
	{
		static const size_t				kNumTags = 10000;
		Head							markerHead;

		Context()
		{
			markerHead.lev = 0;
			markerHead.isPopped = false;
		}

		~Context()
		{
		}
	};
	static bool																m_isInitialized;
	static std::vector<uint32_t>											m_markerFlags;
	static bool																m_isDataTagEnabled;
#if _SCE_TARGET_OS_PROSPERO
	static std::unordered_map<const Agc::CommandBuffer *, Context *>		m_gpuCtxs;
	static Context *getCtx(const Agc::CommandBuffer	*cb);
#endif
#if _SCE_TARGET_OS_ORBIS
	static std::unordered_map<const Gnmx::GnmxGfxContext *, Context *>		m_gpuCtxs;
	static Context *getCtx(const Gnmx::GnmxGfxContext	*ctx);
#endif
	static std::unordered_map<ScePthread, Context *>						m_cpuCtxs;

	/*!
	 * @~English
	 * @brief Initialize
	 * @param option Initialize option
	 * @~Japanese
	 * @param option 初期化オプション
	 * @brief 初期化
	 */
	static void	initialize(PerfOption	*option);
	/*!
	 * @~English
	 * @brief Finalize
	 * @~Japanese
	 * @brief 終了処理
	 */
	static void	finalize();
	/*!
	 * @~English
	 * @brief Obtains context
	 * @details Returns the context bound to thread
	 * @param self Thread
	 * @return Context
	 * @~Japanese
	 * @brief コンテキストの取得
	 * @details スレッドに紐づくコンテキストを返す
	 * @param self スレッド
	 * @return コンテキスト
	 */
	static Context *getCtx(ScePthread	self);

	/*!
	 * @~English
	 * @brief Pushes a marker
	 * @tparam CommandBuffer Type of command buffer
	 * @param cb Command buffer
	 * @param name Marker name
	 * @param markerTargets Marker target
	 * @~Japanese
	 * @brief マーカーのpush
	 * @tparam CommandBuffer コマンドバッファの型
	 * @param cb コマンドバッファ
	 * @param name マーカー名
	 * @param markerTargets マーカー対象
	 */
	template<class CommandBuffer> static void	pushMarker(CommandBuffer	*cb, const std::string	&name, uint32_t	markerTargets = kGpuMarker)
	{
		std::hash<std::string>	hash_fn;
		size_t					hval	= hash_fn(name);
		uint32_t				hval8	= 0;
		for (int i = 0; i < 8; i++)
		{
			hval8 ^= hval & 0xffu;
			hval >>= 8;
		}

		for (unsigned int i = 0; i < m_markerFlags.size(); i++)
		{
			if (markerTargets & m_markerFlags[i])
			{
				Head &head = (m_markerFlags[i] == kGpuMarker) ? getCtx(cb)->markerHead : getCtx(scePthreadSelf())->markerHead;
				if (head.lev == 0)
				{
					head.hue = hval8;
				} else if (head.isPopped) {
					head.hue ^= hval8 & (0x7f >> head.lev);
				}
				uint32_t color = (hsv2rgb((head.hue << 16) | ((0xff - head.lev * 32) << 8) | 0xff) << 8) | 0xffu /* alpha = 1.0*/;
				if (m_markerFlags[i] == kGpuMarker)
				{
					cb->pushMarker(name.c_str(), color);
				} else {
#ifdef _DEBUG
					sceRazorCpuPushMarker(name.c_str(), color, SCE_RAZOR_CPU_MARKER_PROMISE_SCOPED);
#endif
				}
				++head.lev;
				head.isPopped = false;
			}
		}
	}

	/*!
	 * @~English
	 * @brief Pops a marker
	 * @tparam CommandBuffer Type of command buffer
	 * @param cb Command buffer
	 * @param markerTargets Marker target
	 * @~Japanese
	 * @brief マーカーのpop
	 * @tparam CommandBuffer コマンドバッファの型
	 * @param cb コマンドバッファ
	 * @param markerTargets マーカー対象
	 */
	template<class CommandBuffer> static void	popMarker(CommandBuffer	*cb, uint32_t	markerTargets = kGpuMarker)
	{
		for (unsigned int i = 0; i < m_markerFlags.size(); i++)
		{
			if (markerTargets & m_markerFlags[i])
			{
				Head &head = (m_markerFlags[i] == kGpuMarker) ? getCtx(cb)->markerHead : getCtx(scePthreadSelf())->markerHead;
				--head.lev;
				head.isPopped = true;
				if (m_markerFlags[i] == kGpuMarker)
				{
					cb->popMarker();
				} else {
#ifdef _DEBUG
					sceRazorCpuPopMarker();
#endif
				}
			}
		}
	}

	/*!
	 * @~English
	 * @brief Pushes a CPU marker
	 * @param name Marker name
	 * @~Japanese
	 * @brief CPUマーカーのpush
	 * @param name マーカー名
	 */
	static void	pushCpuMarker(const std::string	&name)
	{
#if _SCE_TARGET_OS_PROSPERO
		pushMarker<Agc::DrawCommandBuffer>(nullptr, name, kCpuMarker);
#endif
#if _SCE_TARGET_OS_ORBIS
		pushMarker<Gnmx::GnmxGfxContext>(nullptr, name, kCpuMarker);
#endif
	}

	/*!
	 * @~English
	 * @brief Pops a CPU marker
	 * @~Japanese
	 * @brief CPUマーカーのpop
	 */
	static void	popCpuMarker()
	{
#if _SCE_TARGET_OS_PROSPERO
		popMarker<Agc::DrawCommandBuffer>(nullptr, kCpuMarker);
#endif
#if _SCE_TARGET_OS_ORBIS
		popMarker<Gnmx::GnmxGfxContext>(nullptr, kCpuMarker);
#endif
	}

	/*!
	 * @~English
	 * @brief Begins to tag a buffer
	 * @param name Marker name
	 * @param pBuffer Buffer top address
	 * @param sizeOfBuffer Buffer size(in bytes)
	 * @param numElements The number of elements
	 * @~Japanese
	 * @brief バッファへのタグ付け開始
	 * @param name マーカー名
	 * @param pBuffer バッファの先頭アドレス
	 * @param sizeOfBuffer バッファのサイズ(バイト)
	 * @param numElements バッファのエレメント数
	 */
	static void	beginTagBuffer(const std::string	&name, void	*pBuffer, size_t	sizeOfBuffer, size_t	numElements);
	/*!
	 * @~English
	 * @brief Ends tagging a buffer
	 * @~Japanese
	 * @brief バッファへのタグ付け終了
	 */
	static void	endTagBuffer();
	/*!
	 * @~English
	 * @brief Tags a buffer
	 * @param name Marker name
	 * @param pBuffer Buffer top address
	 * @param sizeOfBuffer Buffer size(in bytes)
	 * @param numElements The number of elements
	 * @~Japanese
	 * @brief バッファへのタグ付け
	 * @param name マーカー名
	 * @param pBuffer バッファの先頭アドレス
	 * @param sizeOfBuffer バッファのサイズ(バイト)
	 * @param numElements バッファのエレメント数
	 */
	static void	tagBuffer(const std::string	&name, void	*pBuffer, size_t	sizeOfBuffer, size_t	numElements = 1);

	/*!
	 * @~English
	 * @tparam T Type of buffer
	 * @brief Begins to tag a buffer
	 * @param name Marker name
	 * @param pBuffer Buffer top address
	 * @param numElements The number of elements
	 * @~Japanese
	 * @brief バッファへのタグ付け開始
	 * @tparam T バッファの型
	 * @param name マーカー名
	 * @param pBuffer バッファの先頭アドレス
	 * @param numElements バッファのエレメント数
	 */
	template<typename T>
	static void	beginTagBuffer(const std::string	&name, T	*pBuffer, size_t	numElements)
	{
		beginTagBuffer(name, pBuffer, sizeof(T), numElements);
	}

	/*!
	 * @~English
	 * @brief Tags a buffer
	 * @tparam T Type of buffer
	 * @param name Marker name
	 * @param pBuffer Buffer top address
	 * @param numElements The number of elements
	 * @~Japanese
	 * @brief バッファへのタグ付け
	 * @tparam T バッファの型
	 * @param name マーカー名
	 * @param pBuffer バッファの先頭アドレス
	 * @param numElements バッファのエレメント数
	 */
	template<typename T>
	static void	tagBuffer(const std::string	&name, T	*pBuffer, size_t	numElements = 1)
	{
		tagBuffer(name, pBuffer, sizeof(T), numElements);
	}

	/*!
	 * @~English
	 * @brief Begins to untag a tag
	 * @param pBuffer Buffer top address
	 * @~Japanese
	 * @brief バッファへのタグ削除の開始
	 * @param pBuffer バッファの先頭アドレス
	 */
	static void	beginUnTagBuffer(void	*pBuffer);
	/*!
	 * @~English
	 * @brief Ends untagging a tag
	 * @~Japanese
	 * @brief バッファへのタグ削除の終了
	 */
	static void	endUnTagBuffer();
	/*!
	 * @~English
	 * @brief Untags a tag
	 * @param pBuffer Top address of buffer
	 * @~Japanese
	 * @brief バッファへのタグ削除
	 * @param pBuffer バッファの先頭アドレス
	 */
	static void	unTagBuffer(void	*pBuffer);
};

/*!
 * @~English
 * @brief Scoped performance instrument
 * @tparam CommandBuffer Type of command buffer
 * @~Japanese
 * @brief スコープ内パフォーマンス計測
 * @tparam CommandBuffer コマンドバッファの型
 */
template<class CommandBuffer> class ScopedPerfOf
{
	CommandBuffer	*m_cb;
	uint32_t 		m_targets;
public:
	/*!
	 * @~English
	 * @brief Constructor
	 * @details Starts performance instrument
	 * @param cb Command buffer
	 * @param name Name
	 * @param markerTargets Marker targets
	 * @~Japanese
	 * @brief コンストラクタ
	 * @details パフォーマンス計測を開始
	 * @param cb コマンドバッファ
	 * @param name 名前
	 * @param markerTargets マーカー対象
	 */
	ScopedPerfOf(CommandBuffer	*cb, const std::string	&name, uint32_t	markerTargets = kGpuMarker)
		:m_cb(cb), m_targets(markerTargets)
	{
		Perf::pushMarker(m_cb, name, markerTargets);
	}
	/*!
	 * @~English
	 * @brief Destructor
	 * @details Ends performance instrument
	 * @~Japanese
	 * @brief デストラクタ
	 * @details パフォーマンス計測を終了
	 */
	~ScopedPerfOf()
	{
		Perf::popMarker(m_cb, m_targets);
	}
};

/*!
 * @~English
 * @brief Scoped CPU performance instrument
 * @~Japanese
 * @brief スコープ内CPUパフォーマンス計測
 */
class ScopedCpuPerf
{
public:
	/*!
	 * @~English
	 * @brief Constructor
	 * @details Starts CPU performance instrument
	 * @param name Name
	 * @~Japanese
	 * @brief コンストラクタ
	 * @details CPUパフォーマンス計測を開始
	 * @param name 名前
	 */
	ScopedCpuPerf(const std::string	&name)
	{
		Perf::pushCpuMarker(name);
	}
	/*!
	 * @~English
	 * @brief Destructor
	 * @details Ends CPU performance instrument
	 * @~Japanese
	 * @brief デストラクタ
	 * @details CPUパフォーマンス計測を終了
	 */
	~ScopedCpuPerf()
	{
		Perf::popCpuMarker();
	}
};

/*!
 * @~English
 * @brief Scoped tagging buffer
 * @tparam T Type of buffer
 * @~Japanese
 * @brief スコープ内バッファタグ付け
 * @tparam T バッファの型
 */
template<typename T>
class ScopedTagBuffer
{
	T	*m_pBuffer;
public:
	/*!
	 * @~English
	 * @brief Constructor
	 * @details Tags a buffer
	 * @param name Name
	 * @param pBuffer Buffer top address
	 * @param numElements The number of elements
	 * @param name Name
	 * @~Japanese
	 * @brief コンストラクタ
	 * @details バッファへタグ付け
	 * @param name 名前
	 * @param pBuffer バッファ先頭アドレス
	 * @param numElements エレメント数
	 */
	ScopedTagBuffer(const std::string	&name, T	*pBuffer, size_t	numElements = 1) :
		m_pBuffer(pBuffer)
	{
		Perf::tagBuffer(name, pBuffer, numElements);
	}
	/*!
	 * @~English
	 * @brief Destructor
	 * @details Untags a buffer
	 * @~Japanese
	 * @brief デストラクタ
	 * @details バッファへタグ付け解除
	 */
	~ScopedTagBuffer()
	{
		Perf::unTagBuffer(m_pBuffer);
	}
};

#if !defined(DOXYGEN_IGNORE)

template<typename T>
class TagClass
{
public:
	TagClass(const std::string	&name, T	pBuffer, size_t	sizeBuffer)
	{
		Perf::beginTagBuffer(name, pBuffer, sizeBuffer);
	}
	~TagClass()
	{
		Perf::endTagBuffer();
	}
};

template<typename T>
class UnTagClass
{
public:
	UnTagClass(T	pBuffer)
	{
		Perf::beginUnTagBuffer(pBuffer);
	}
	~UnTagClass()
	{
		Perf::endUnTagBuffer();
	}
};

#endif
}}} // namespace sce::SampleUtil::Debug
