/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */
#pragma once
#include <cstdint>
#include <unordered_map>
#include <fstream>
#include <mspace.h>
#include <video_out.h>
#include <vectormath/cpp/vectormath_aos.h>
#include <mat.h>
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <hmd2/reprojection.h>
#include <agc/commandbuffer.h> /* Agc::Label */
#include <agc/core/ringbuffer.h>
#include <agc/toolkit/toolkit.h>
#include <fr_toolkit.h>
#endif
#include <sampleutil/graphics/compat.h>
#include <sampleutil/memory.h>
#include <sampleutil/graphics/render_target.h>
#include <sampleutil/graphics/depth_render_target.h>
#include <sampleutil/graphics/pack_model.h>
#include <sampleutil/graphics/texture_library.h>
#if _SCE_TARGET_OS_PROSPERO
#include "sampleutil/helper/prospero/async_asset_loader_prospero.h"
#endif

namespace sce {	namespace SampleUtil { namespace Graphics {
	/*!
	 * @~English
	 * @brief Specify which eye of HMD
	 * @~Japanese
	 * @brief HMDのどちらの目かを指定
	 */
	enum class HmdEye : uint32_t
	{
		kLeft = 0,
		kRight
	};

	/*!
	 * @~English
	 * @brief Specify layer of HMD
	 * @~Japanese
	 * @brief HMDのレイヤを指定
	 */
	enum class HmdLayer : uint32_t
	{
		kVr3d = 0,
		kUi2d,
		kVr2d,
		kNum
	};

	/*!
	 * @~English
	 * @brief Refresh rates for rendering and reprojection
	 * @~Japanese
	 * @brief レンダリングとリプロジェクションのリフレッシュレート
	 */
	enum class HmdRefreshRate : uint32_t
	{
		/*!
		 * @~English
		 * @brief Render with 60Hz and reprojec with 120Hz(HDMI sync point is FORMER 8.3ms in 16.6ms frame)
		 * @~Japanese
		 * @brief 60Hzでレンダリングし、120Hzでリプロジェクションする(HDMI sync pointは16.6msフレーム中の前半8.3ms)
		 */
		kRender60Reprojection120SyncPhaseHdmiFormerHalf,
		/*!
		 * @~English
		 * @brief Render with 60Hz and reprojec with 120Hz(HDMI sync point is LATTER 8.3ms in 16.6ms frame)
		 * @~Japanese
		 * @brief 60Hzでレンダリングし、120Hzでリプロジェクションする(HDMI sync pointは16.6msフレーム中の後半8.3ms)
		 */
		kRender60Reprojection120SyncPhaseHdmiLatterHalf,
		/*!
		 * @~English
		 * @brief Render with 120Hz and reprojec with 120Hz
		 * @~Japanese
		 * @brief 120Hzでレンダリングし、120Hzでリプロジェクションする
		 */
		kRender120Reprojection120,
		/*!
		 * @~English
		 * @brief Render with 90Hz and reprojec with 90Hz
		 * @~Japanese
		 * @brief 90Hzでレンダリングし、90Hzでリプロジェクションする
		 */
		kRender90Reprojection90
	};

	/*!
	 * @~English
	 * @brief Reprojection timing parameter. Reprojection starts designated USEC before flip.
	 * @~Japanese
	 * @brief リプロジェクションタイミングパラメータ、フリップの何マイクロ秒前にリプロジェクション開始するかを指定
	 */
	enum class HmdReprojectionTiming : uint32_t
	{
		k2000usec,
		k3000usec,
		k4000usec,
		k5000usec,
		k6000usec
	};

	/*!
	 * @~English
	 * @brief Render target for FSR enabled HMD
	 * @~Japanese
	 * @brief HMDにFSR描画する際のレンダーターゲット
	 */
	enum class HmdFsrTarget : uint32_t
	{
		/*!
		 * @~English
		 * @brief No FSR is applied
		 * @~Japanese
		 * @brief FSRを使用しない
		 */
		kNone,
		/*!
		 * @~English
		 * @brief Left eye render target
		 * @~Japanese
		 * @brief 左目用のレンダーターゲット
		 */
		kLeft,
		/*!
		 * @~English
		 * @brief Right eye render target
		 * @~Japanese
		 * @brief 右目用のレンダーターゲット
		 */
		kRight,
		/*!
		 * @~English
		 * @brief Stereo expansion render target
		 * @~Japanese
		 * @brief ステレオエキスパンションのレンダーターゲット
		 */
		kStereoExpansion
	};

	/*!
	 * @~English
	 * @brief Default Workload Stream ID
	 * @~Japanese
	 * @brief 既定のWorkload Stream ID
	 */
	static const uint32_t	kDefaultWorkloadStreamId = 1;

	/*!
	 * @~English
	 *
	 * @brief Structure for initializing GraphicsContext
	 * @details This is the structure for initializing GraphicsContext. This is used by specifying it to the argument "option" of GrahicsContext::initialize().
	 * @~Japanese
	 *
	 * @brief GraphicsContextの初期化用構造体
	 * @details GraphicsContextの初期化用構造体です。GrahicsContext::initialize()の引数optionに指定することで利用します。
	 */
	struct GraphicsContextOption
	{
		/*!
		 * @~English
		 * @brief Number of ring buffers for the render target displayed on the MAIN port (Default: 3)
		 * @~Japanese
		 * @brief ディスプレイに表示するMAIN port用レンダーターゲットのリングバッファ数(デフォルト: 3)
		 */
		uint32_t numFrameBufferMain;
#if _SCE_TARGET_OS_ORBIS
		/*!
		 * @~Japanese
		 * @brief GPUへ描画コマンドを投入するために使用するコンテキスト数(デフォルト: 3)
		 */
		uint32_t numContexts;

		/*!
		 * @~English
		 * @brief Heap size to be used as a onion graphics resource. The heap will be allocated onto direct memory of SCE_KERNEL_MTYPE_WB_ONION.
		 * @~Japanese
		 * @brief Graphicsリソース用に使われるOnionのヒープのサイズ。ヒープはSCE_KERNEL_MTYPE_WB_ONIONのダイレクトメモリに確保されます。
		 */
		size_t onionHeapSize;
#endif
		/*!
		 * @~English
		 * @brief Heap size to be used as a graphics resource. The heap will be allocated onto direct memory of SCE_KERNEL_MTYPE_C_SHARED.
		 * @~Japanese
		 * @brief Graphicsリソース用に使われるヒープのサイズ。ヒープはSCE_KERNEL_MTYPE_C_SHAREDのダイレクトメモリに確保されます。
		 */
		size_t videoHeapSize;
#if _SCE_TARGET_OS_PROSPERO
		/*!
		 * @~English
		 * @brief Heap size to be used as a graphics resource. The heap will be allocated onto physical address contiguous direct memory of SCE_KERNEL_MTYPE_C_SHARED.
		 * @~Japanese
		 * @brief Graphicsリソース用に使われるヒープのサイズ。ヒープはSCE_KERNEL_MTYPE_C_SHAREDの物理アドレス連続なダイレクトメモリに確保されます。
		 */
		size_t paContVideoHeapSize;
#endif
		/*!
		 * @~English
		 * @brief Graphics memory ringbuffer size to allocate per-frame lifetime memory
		 * @~Japanese
		 * @brief フレーム単位の生存期間のGraphics用メモリを確保するリングバッファサイズ
		 */
		size_t videoRingSize;

		/*!
		 * @~English
		 * @brief Specify whether ResourceRegistration is enabled or not
		 * @~Japanese
		 * @brief ResourceRegistrationを有効化するかどうかを指定
		 */
		bool enableResourceRegistration;

		/*!
		 * @~English
		 * @brief The number of owners and resources to be supported by ResourceRegistration
		 * @~Japanese
		 * @brief ResourceRegistrationがサポートするオーナーとリソースの数
		 */
		uint32_t resourceRegistrationMaxOwnersAndResources;

		/*!
		 * @~English
		 * @brief The maximum length of a name to be stored (including <c>NULL</c> terminator) by ResourceRegistration.
		 * @~Japanese
		 * @brief ResourceRegistrationで格納する文字の最大長(<c>NULL</c>終端を含む)
		 */
		uint32_t resourceRegistrationMaxNameLength;

		/*!
		 * @~English
		 * @brief Heap size to be used as a CPU resource. The heap will be allocated onto direct memory of SCE_KERNEL_MTYPE_C.
		 * @~Japanese
		 * @brief CPUリソース用に使われるヒープのサイズ。ヒープはSCE_KERNEL_MTYPE_Cのダイレクトメモリに確保されます。
		 */
		size_t cpuHeapSize;

		/*!
		 * @~English
		 * @brief The size of DrawCommand buffer(in bytes)
		 * @~Japanese
		 * @brief DrawCommandバッファのサイズ(byte)
		 */
		size_t dcbSize;

		/*!
		 * @~English
		 * @brief When this flag is true, suspendPoint() will not be called from within GraphicsContext::flip()
		 * @~Japanese
		 * @brief このフラグが真のときはGraphicsContext::flip()の中でsuspendPoint()が呼ばれない
		 */
		bool disableSuspendPointInFlip;

		/*!
		 * @~English
		 * @brief Enable HDR output for MAIN port
		 * @~Japanese
		 * @brief MAIN portのビデオ出力をHDR出力可能に設定します。
		 */
		bool enableHdrViewMain;

		/*!
		 * @~English
		 * @brief No gamma for MAIN port
		 * @~Japanese
		 * @brief MAIN portのビデオ出力にガンマ補正を適用しない
		 */
		bool noGammaMain;

		/*!
		 * @~English
		 * @brief Flip mode for MAIN port
		 * @~Japanese
		 * @brief MAIN portのフリップモード
		 */
		SceVideoOutFlipMode	flipModeMain;

#if _SCE_TARGET_OS_PROSPERO
		/*!
		 * @~English
		 * @brief 
		 * @~Japanese
		 * @brief ビデオ出力を行うバスを指定
		 */
		uint64_t	videoOutBusUseBitmap;

		/*!
		 * @~English
		 * @brief Number of ring buffers for the render target displayed on the OVERLAY port (Default: 3)
		 * @~Japanese
		 * @brief ディスプレイに表示するOVERLAY port用レンダーターゲットのリングバッファ数(デフォルト: 3)
		 */
		uint32_t numFrameBufferOverlay;

		/*!
		 * @~English
		 * @brief Enable HDR output for Overlay port
		 * @~Japanese
		 * @brief Overlay portのビデオ出力をHDR出力可能に設定します。
		 */
		bool enableHdrViewOverlay;

		/*!
		 * @~English
		 * @brief No gamma for OVERLAY port
		 * @~Japanese
		 * @brief OVERLAY portのビデオ出力にガンマ補正を適用しない
		 */
		bool noGammaOverlay;

		/*!
		 * @~English
		 * @brief Enable DCC scanout for MAIN port
		 * @~Japanese
		 * @brief MAIN portのscanoutをDCCに設定
		 */
		bool enableDccMain;

		/*!
		 * @~English
		 * @brief Enable DCC scanout for OVERLAY port
		 * @~Japanese
		 * @brief OVERLAY portのscanoutをDCCに設定
		 */
		bool enableDccOverlay;

		/*!
		 * @~English
		 * @brief Flip mode for OVERLAY port
		 * @~Japanese
		 * @brief OVERLAY portのフリップモード
		 */
		SceVideoOutFlipMode	flipModeOverlay;

		/*!
		 * @~English
		 * @brief Width of OVERLAY port frame buffer(same as one for MAIN port if '0' is specified)
		 * @~Japanese
		 * @brief OVERLAY portのフレームバッファの幅(0を指定した場合MAIN portと同じ)
		 */
		uint32_t	frameBufferWidthOverlay;
		/*!
		 * @~English
		 * @brief Height of OVERLAY port frame buffer(same as one for MAIN port if '0' is specified)
		 * @~Japanese
		 * @brief OVERLAY portのフレームバッファの高さ(0を指定した場合MAIN portと同じ)
		 */
		uint32_t	frameBufferHeightOverlay;
		/*!
		 * @~English
		 * @brief Agc::Core::RingBuffer segment memory size(in bytes)
		 * @~Japanese
		 * @brief Agc::Core::RingBufferのセグメントメモリサイズ(バイト)
		 */
		uint32_t	ringBufferSegmentSize;
		/*!
		 * @~English
		 * @brief The number of Agc::Core::RingBuffer segments
		 * @~Japanese
		 * @brief Agc::Core::RingBufferのセグメント数
		 */
		uint32_t	numRingBufferSegments;
		/*!
		 * @~English
		 * @brief base frame bus
		 * @~Japanese
		 * @brief ベースフレームバス
		 */
		int	baseFrameBus;
		/*!
		 * @~English
		 * @brief Number of ring buffers for the render target displayed on the HMD MAIN port (Default: 3)
		 * @~Japanese
		 * @brief ディスプレイに表示するHMD2 MAIN port用レンダーターゲットのリングバッファ数(デフォルト: 3)
		 */
		uint32_t numFrameBufferHmdMain;
		/*!
		 * @~English
		 * @brief Flip mode for HMD MAIN port
		 * @~Japanese
		 * @brief HMD MAIN portのフリップモード
		 */
		SceVideoOutFlipMode	flipModeHmdMain;
		/*!
		 * @~English
		 * @brief Spec of HMD MAIN port VR3d layer frame buffer
		 * @~Japanese
		 * @brief HMD MAIN port VR3Dレイヤのフレームバッファのスペック
		 */
		sce::Agc::Core::RenderTargetSpec	frameBufferHmdMainVr3d;
		bool	enableHmdVr3d;
		bool	enableHmdVr3dFsr;
		bool	enableHmdVr3dSideBySide;
		/*!
		 * @~English
		 * @brief Spec of HMD MAIN port UI2D layer frame buffer
		 * @~Japanese
		 * @brief HMD MAIN port UI2Dレイヤのフレームバッファのスペック
		 */
		sce::Agc::Core::RenderTargetSpec	frameBufferHmdMainUi2d;
		bool	enableHmdUi2d;
		/*!
		 * @~English
		 * @brief Spec of HMD MAIN port VR2D layer frame buffer
		 * @~Japanese
		 * @brief HMD MAIN port VR2Dレイヤのフレームバッファのスペック
		 */
		sce::Agc::Core::RenderTargetSpec	frameBufferHmdMainVr2d;
		bool	enableHmdVr2d;
		/*!
		 * @~English
		 * @brief Refresh rates for rendering and reprojection
		 * @~Japanese
		 * @brief レンダリングとリプロジェクションのリフレッシュレート
		 */
		HmdRefreshRate	hmdReprojectionSyncMode;
		/*!
		 * @~English
		 * @brief Reprojection timing parameter. Reprojection starts designated USEC before flip.
		 * @~Japanese
		 * @brief リプロジェクションタイミングパラメータ、フリップの何マイクロ秒前にリプロジェクション開始するかを指定
		 */
		HmdReprojectionTiming	hmdReprojectionTiming;
		/*!
		 * @~English
		 * @brief Output mirroring result of HMD2 MAIN port to MAIN port
		 * @~Japanese
		 * @brief MAINポートにHMD2 MAINポートの出力結果をミラーリング出力する
		 */
		bool	enableHmdReprojectionMirroring;
		/*!
		 * @~English
		 * @brief Workload Stream ID used for Graphics Context
		 * @~Japanese
		 * @brief Graphics Contextで使用するWorkload Stream ID
		 */
		uint32_t	workloadStreamId;
		/*!
		 * @~English
		 * @brief Set MAIN port display buffer to Linear format when MAIN port is HDR view
		 * @~Japanese
		 * @brief MAIN portがHDR viewに設定されている場合に、MAIN port ディスプレイバッファをlinearフォーマットに設定
		 */
		bool	enableLinearHdrMain;
		/*!
		 * @~English
		 * @brief Set OVERLAY port display buffer to Linear format when OVERLAY port is HDR view
		 * @~Japanese
		 * @brief OVERLAY portがHDR viewに設定されている場合に、OVERLAY port ディスプレイバッファをlinearフォーマットに設定
		 */
		bool	enableLinearHdrOverlay;
#endif

		/*!
		 * @~English
		 * @brief Constructor
		 * @details This is a constructor.
		 * @~Japanese
		 * @brief コンストラクタ
		 * @details コンストラクタです。
		 */
		GraphicsContextOption(GraphicsContextOption *option = nullptr) :
			numFrameBufferMain(3),
#if _SCE_TARGET_OS_ORBIS
			numContexts(3),
			onionHeapSize(64 * 1024 * 1024), // 64 MiB
#endif
			videoHeapSize(1024*1024*1024), // 1 GiB
#if _SCE_TARGET_OS_PROSPERO
			paContVideoHeapSize(512*1024*1024), // 512 MiB
#endif
			videoRingSize(16*1024*1024), // 16 MiB
			enableResourceRegistration(true),
			resourceRegistrationMaxOwnersAndResources(4096),
			resourceRegistrationMaxNameLength(64),
			cpuHeapSize(1024*1024*1024), // 1 GiB
			dcbSize(10 * 1024 * 1024), // 10 MiB
			disableSuspendPointInFlip(false),
			enableHdrViewMain(false),
			noGammaMain(false),
			flipModeMain(SCE_VIDEO_OUT_FLIP_MODE_VSYNC)
#if _SCE_TARGET_OS_PROSPERO
			,videoOutBusUseBitmap(1ul << SCE_VIDEO_OUT_BUS_TYPE_MAIN),
			numFrameBufferOverlay(3),
			enableHdrViewOverlay(false),
			noGammaOverlay(false),
			enableDccMain(false),
			enableDccOverlay(false),
			flipModeOverlay(SCE_VIDEO_OUT_FLIP_MODE_VSYNC),
			frameBufferWidthOverlay(0),
			frameBufferHeightOverlay(0),
			ringBufferSegmentSize(4 * 1024 * 1024),
			numRingBufferSegments(8),
			baseFrameBus(SCE_VIDEO_OUT_BUS_TYPE_MAIN),
			numFrameBufferHmdMain(3),
			flipModeHmdMain(SCE_VIDEO_OUT_FLIP_MODE_ASAP),
			frameBufferHmdMainVr3d(sce::Agc::Core::RenderTargetSpec().init().setWidth(2000).setHeight(2040).setFormat({ sce::Agc::Core::TypedFormat::k10_10_10_2UNorm, sce::Agc::Core::Swizzle::kRGBA_R4S4 })),
			enableHmdVr3d(false),
			enableHmdVr3dFsr(false),
			enableHmdVr3dSideBySide(false),
			frameBufferHmdMainUi2d(sce::Agc::Core::RenderTargetSpec().init().setWidth(2000).setHeight(2040).setFormat({ sce::Agc::Core::TypedFormat::k8_8_8_8UNorm, sce::Agc::Core::Swizzle::kRGBA_R4S4 })),
			enableHmdUi2d(false),
			frameBufferHmdMainVr2d(sce::Agc::Core::RenderTargetSpec().init().setWidth(2000).setHeight(2040).setFormat({ sce::Agc::Core::TypedFormat::k10_10_10_2UNorm, sce::Agc::Core::Swizzle::kRGBA_R4S4 })),
			enableHmdVr2d(false),
			hmdReprojectionSyncMode(HmdRefreshRate::kRender120Reprojection120),
			hmdReprojectionTiming(HmdReprojectionTiming::k3000usec),
			enableHmdReprojectionMirroring(true),
			workloadStreamId(kDefaultWorkloadStreamId),
			enableLinearHdrMain(false),
			enableLinearHdrOverlay(false)
#endif
		{
			if (option != nullptr)
			{
				*this = *option;
			}
		}
	};

	/*!
	 * @~English
	 *
	 * @brief Graphics context
	 *
	 * @details Manages the data required for drawing of graphics and sets the state.
	 * @~Japanese
	 *
	 * @brief グラフィックスコンテキスト
	 *
	 * @details グラフィックスの描画に必要なデータの管理、状態の設定を行います。
	 */
	struct GraphicsContext
	{
		/*!
		 * @~English
		 * @brief Display mode
		 * @details This is the display mode. The normal display mode, HDR display mode and HDR Linear display mode are supported.
		 * @~Japanese
		 * @brief ディスプレイモード
		 * @details ティスプレイモードです。通常のディスプレイモード、HDRのディスプレイモード、もしくはHDR Linearのディスプレイモードの3つをサポートします。
		 */
		enum class	DisplayMode : uint32_t
		{
			/*!
			 * @~English
			 * @brief The normal display mode
			 * @~Japanese
			 * @brief 通常のディスプレイモード
			 */
			kNormal,
			/*!
			 * @~English
			 * @brief HDR display mode
			 * @~Japanese
			 * @brief HDRディスプレイモード
			 */
			kHdrView,
			/*!
			 * @~English
			 * @brief HDR Linear display mode
			 * @~Japanese
			 * @brief HDR Linearディスプレイモード
			 */
			kHdrLinearView,
		};
		struct DeferredGpuMemoryRelease
		{
			static VideoAllocator							*m_pAlloc;

			bool											m_isClosed;
			std::vector<void *>								m_waitingPointers;
#if _SCE_TARGET_OS_PROSPERO
			Agc::Label										*m_pReleaseLabel;
#endif
#if _SCE_TARGET_OS_ORBIS
			uint64_t										*m_pReleaseLabel;
#endif
		};
#if _SCE_TARGET_OS_PROSPERO
		struct ReprojectionParameter
		{
			SceHmd2FieldOfView						m_fov;
			SceHmd2ReprojectionPoseData				m_pose;
			bool									m_enablePositionalReprojection;
			// parameters for positional reprojection
			SceHmd2ReprojectionPoseData				m_leftEyePose;
			SceHmd2ReprojectionPoseData				m_rightEyePose;
			sce::Agc::Core::Texture					m_depthTextures[2];
			float									m_zNear;
			float									m_zFar;
			bool									m_enableAdditionalPose;
			SceHmd2ReprojectionAdditionalPoseData	m_additionalPose;
			struct UIPlane
			{
				SceHmd2TextureViewport				m_viewport;
				float								m_width;
				float								m_height;
				sce::Vectormath::Simd::Aos::Point3	m_position;
				sce::Vectormath::Simd::Aos::Quat	m_orientation;
			}										m_ui2dPlane;
			bool									m_enableUi2d;
			SceHmd2TextureViewport					m_vr2dTexViewport;
			bool									m_enableSeeThrough;
			SceHmd2ReprojectionSeeThroughStyle		m_seeThroughStyle;
		};
#endif
		int													m_errorCode;
		bool												m_isCleared;
		GraphicsContextOption								m_option;
		std::vector<DeferredGpuMemoryRelease>				m_deferredGpuMemoryRelease;
		VideoAllocator										m_videoMemory;
#if _SCE_TARGET_OS_PROSPERO
		VideoAllocator										m_phyAddrContiguousVideoMemory;
#endif
#if _SCE_TARGET_OS_ORBIS
		VideoAllocator										m_onionVideoMemory;
#endif
		VideoRingAllocator									m_videoRingMemory;
		uint32_t											m_numRenderTargets;
#if _SCE_TARGET_OS_PROSPERO
		Agc::Core::RingBuffer								m_rb;
		Agc::Core::RingBuffer::SegmentChangeCallback		m_rbSegmentCallback;
		void												*m_rbSegmentPayload;
		Agc::TwoSidedAllocator								m_pDataAllocators[(int)sce::Agc::ShaderType::kRealCount];
		Agc::Core::RingBuffer::DataCallbackPayload			m_dataOomPayload;
		SampleUtil::Memory::Gpu::unique_ptr<uint32_t[]>		m_rbSemgemtMem;
		Agc::Core::BasicContext								m_ctx;
		Agc::Core::BasicContext								*m_currentCtx;
		Agc::CxRenderTarget									m_pCurrentRenderTargets[8];
		Agc::CxDepthRenderTarget							m_currentDepthRenderTarget;
		Memory::Gpu::unique_ptr<Agc::Label[]>				m_flipLabelsMain;
		Memory::Gpu::unique_ptr<Agc::Label[]>				m_flipLabelsOverlay;
		Memory::Gpu::unique_ptr<Agc::Label[]>				m_flipLabelsHmd;
		Memory::Gpu::unique_ptr<uint8_t[]>					m_scanoutBuffersMemMain;
		Memory::Gpu::unique_ptr<uint8_t[]>					m_scanoutBuffersMemOverlay;
		Memory::Gpu::unique_ptr<uint8_t[]>					m_scanoutBuffersMemHmdVr3d;
		Memory::Gpu::unique_ptr<uint8_t[]>					m_scanoutBuffersMemHmdUi2d;
		Memory::Gpu::unique_ptr<uint8_t[]>					m_hmdReprojectionMirroringWorkMemory;
		Memory::Gpu::unique_ptr<uint8_t[]>					m_hmdReprojectionMirroringDisplayMemory;
		bool												m_isFlipRequestPendingMain;
		bool												m_isFlipRequestPendingOverlay;
		bool												m_isFlipRequestPendingHmd;
#endif
#if _SCE_TARGET_OS_ORBIS
		Gnm::WorkloadStream									m_workload;
		uint64_t											m_workloadId;

#if SCE_GNMX_ENABLE_GFX_LCUE
		Memory::Gpu::unique_ptr<uint8_t[]>					*m_lcueResBuf;
#else
		Memory::Gpu::unique_ptr<uint8_t[]>					*m_cueHeap;
		Memory::Gpu::unique_ptr<uint8_t[]>					*m_ccbData;
#endif
		Memory::Gpu::unique_ptr<uint8_t[]>					*m_dcbData;
		Memory::Gpu::unique_ptr<uint8_t[]>					*m_globalResourceTable;
		Memory::Gpu::unique_ptr<uint8_t[]>					m_tessellationFactorRingBufferMem;
		Gnm::Buffer											m_tessellationFactorRingBuffer;

		Gnmx::GnmxGfxContext								*m_ctxs;
		Gnmx::GnmxGfxContext								*m_currentCtx;
		Gnm::RenderTarget									*m_pCurrentRenderTargets;
		Gnm::DepthRenderTarget								*m_pCurrentDepthRenderTarget;
		Memory::Gpu::unique_ptr<uint64_t[]>					m_flipLabelsMain;
		SceKernelEqueue										m_eopEqueue;
#endif
		int													m_currentBus;
		RenderTargetWrapper									*m_pFrameBufferRenderTargetsMain;
		DepthRenderTargetWrapper							m_frameBufferDepthRenderTargetMain;
		DisplayMode											m_displayModeMain;
		int													m_videoHandleMain;
		uint32_t											m_flipCountMain;
		std::unordered_map<void *, int>						m_busTypeTable;
#if _SCE_TARGET_OS_PROSPERO
		RenderTargetWrapper									*m_pFrameBufferRenderTargetsOverlay;
		DepthRenderTargetWrapper							m_frameBufferDepthRenderTargetOverlay;
		RenderTargetWrapper									*m_ppFrameBufferRenderTargetsHmdVr3d[2];
		DepthRenderTargetWrapper							m_frameBufferDepthRenderTargetsHmdVr3d[2];
		RenderTargetWrapper									*m_pFrameBufferRenderTargetsHmdUi2d;
		DepthRenderTargetWrapper							m_frameBufferDepthRenderTargetHmdUi2d;
		RenderTargetWrapper									*m_pFrameBufferRenderTargetsHmdVr2d;
		DepthRenderTargetWrapper							m_frameBufferDepthRenderTargetHmdVr2d;
		DisplayMode											m_displayModeOverlay;
		DisplayMode											m_displayModeHmd;
		int													m_videoHandleOverlay;
		int													m_videoHandleHmd;
		uint32_t											m_flipCountOverlay;
		uint32_t											m_flipCountHmd;
		bool												m_enableDccScanoutMain;
		bool												m_enableDccScanoutOverlay;
		Memory::Gpu::unique_ptr<uint8_t[]>					m_hmdVr3dFsrLUTsMemory;
		std::vector<SceHmd2ReprojectionFsrLut>				m_hmdVr3dFsrLuts;
		sce::FrToolkit::UcFsrView							m_hmdVr3dFsrViews[2];
		HmdFsrTarget										m_currentFsrTarget;
#endif
		uint32_t											m_width;
		uint32_t											m_height;
		off_t												m_cpuMemoryPhyAddr;
		void												*m_cpuMemoryMappedAddr;
		SceLibcMspace										m_cpuMemory;
		MatPool												m_cpuMemoryPool;
		TextureLibrary										m_texLib;
		SceKernelEqueue										m_flipEventQueueMain;
		SceKernelEqueue										m_flipEventQueueOverlay;
		SceKernelEqueue										m_flipEventQueueHmd;
		bool												m_enableRazorCpu;

#if _SCE_TARGET_OS_PROSPERO
		int	setupVideoOut(const GraphicsContextOption	*prevOption = nullptr);
		int	clearVideoOut(const GraphicsContextOption	*nextOption = nullptr);
#endif
		GraphicsContext(uint32_t	width, uint32_t	height,
#if _SCE_TARGET_OS_PROSPERO
						Helper::AsyncAssetLoader	*asyncAssetLoader,
#endif
						GraphicsContextOption	*option = nullptr);
		virtual ~GraphicsContext();

		/*!
		 * @~English
		 * @brief Returns error code
		 * @details Returns error code occured for GraphicsContext
		 * @~Japanese
		 * @brief エラーコードを返す
		 * @details GraphicsContextで発生したエラーを返す
		 */
		int	getErrorCode() const
		{
			return	m_errorCode;
		}

		/*!
		 * @~English
		 * @brief GPU memory deferred release
		 * @details Released GPU memory after GPU's reference to it
		 * @param ptr Pointer to be released
		 * @~Japanese
		 * @brief GPUメモリ遅延解放
		 * @details GPUでの使用が完了するのを待ってから解放
		 * @param ptr 解放するポインタ
		 */
		void	deferredReleaseGpuMemory(void	*ptr)
		{
			m_deferredGpuMemoryRelease.back().m_waitingPointers.push_back(ptr);
		}

		/*!
		 * @~English
		 * @brief GPU memory deferred release processing
		 * @param isFinalizing true if finalizing
		 * @details By periodically calling this function, it checks memory not referred to by GPU any more
		 * @~Japanese
		 * @brief GPUメモリ遅延解放処理
		 * @param isFinalizing 終了処理中はtrue
		 * @details この関数を定期的に実行することで、GPUに参照されていないメモリかをチェック
		 */
		void	handleDeferredGpuMemoryRelease(bool	isFinalizing = false);

		int	setRenderTarget(
#if _SCE_TARGET_OS_PROSPERO
							sce::Agc::Core::BasicContext* currentCtx = nullptr
#endif
		);

		/*!
		 * @~English
		 * @brief Clears the render targets that are currently set.
		 *
		 * @details This fills the current render targets with the specified color.
		 * @param color Color used for filling
		 * @~Japanese
		 * @brief 現在設定されているレンダーターゲットをクリアする
		 *
		 * @details 指定された色で現在のレンダーターゲットを塗りつぶします。
		 * @param color 塗りつぶしに使う色
		 */
		void	clearRenderTarget(uint32_t	color
#if _SCE_TARGET_OS_PROSPERO
								, sce::Agc::Core::BasicContext	*currentCtx = nullptr
#endif
		);

		/*!
		 * @~English
		 * @brief Check display mode support
		 * @details This checks if the specified display mode is supported by the display.
		 * @param mode Display mode
		 * @return True if supported, false if not supported.
		 * @~Japanese
		 * @brief ディスプレイモードのサポートの確認
		 * @details ディスプレイが指定されたティスプレイモードをサポートしているかどうかの確認を行います。
		 * @param mode ディスプレイモード
		 * @return サポートされていればtrue, そうでなければfalse.
		 */
		bool	isDisplaymodeSupported(DisplayMode	mode) const;
		/*!
		 * @~English
		 * @brief Change the display mode
		 * @details This changes the display mode. An error will return if the specified mode is not supported by the display. The initial value for the display mode is kDisplayModeNormal.
		 * @param mode Display mode
		 * @param bus Frame buffer bus type to which display mode is set
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief ディスプレイモードの変更
		 * @details ディスプレイモードを変更します。ディスプレイが指定されたモードをサポートしてない場合はエラーが返ります。ディスプレイモードの初期値は kDisplayModeNormal です。
		 * @param mode ディスプレイモード
		 * @param bus ディスプレイモードをセットするフレームバッファのバスタイプ
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int	setDisplaymode(DisplayMode	mode, int	bus = SCE_VIDEO_OUT_BUS_TYPE_MAIN);

		/*!
		 * @~English
		 * @brief Obtain the display mode
		 * @details This obtains current display mode.
		 * @param bus Frame buffer bus type from which display mode is obtained
		 * @return Current display mode
		 * @~Japanese
		 * @brief ディスプレイモードの取得
		 * @details 現在のディスプレイモードを取得します。
		 * @param bus ディスプレイモードを取得するするフレームバッファのバスタイプ
		 * @return 現在のディスプレイモード
		 */
		DisplayMode	getDisplaymode(int	bus = SCE_VIDEO_OUT_BUS_TYPE_MAIN) const;

#if _SCE_TARGET_OS_PROSPERO
		/*!
		 * @~English
		 * @brief Change the frame buffer size
		 * @details This changes the frame buffer size. It cannot change to the size larger than the size which is specified when frame buffer is initialized.
		 * @param width Frame buffer width
		 * @param height Frame buffer height
		 * @param bus Frame buffer bus type to which size is changed.
		 * @param layer Frame buffer HMD layer to which size is changed.
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief フレームバッファサイズの変更
		 * @details フレームバッファのサイズを変更します。フレームバッファ初期化時に指定したサイズより大きいサイズに変更することは出来ません。
		 * @param width フレームバッファの幅
		 * @param height フレームバッファの高さ
		 * @param bus サイズを変更するフレームバッファのバスタイプ
		 * @param layer サイズを変更するフレームバッファのHMDレイヤ
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int	setFrameBufferSize(uint32_t	width, uint32_t	height, int	bus = SCE_VIDEO_OUT_BUS_TYPE_MAIN, HmdLayer layer = HmdLayer::kVr3d);
#endif

		/*!
		 * @~English
		 * @brief Gets the render target of the frame buffer to be drawn next.
		 * @details This gets the render target of the frame buffer to be drawn next.
		 * @param bus Frame buffer bus type of the frame buffer to be drawn next.
		 * @param layer Frame buffer layer of the frame buffer to be drawn next.
		 * @param eye Specify the eye you are going to render.
		 * @return Pointer to a render target
		 * @~Japanese
		 * @brief 次に描画するフレームバッファのレンダーターゲットを取得する
		 * @details 次に描画するフレームバッファのレンダーターゲットを取得します。
		 * @param bus 次に描画するフレームバッファのバスタイプ
		 * @param layer 次に描画するフレームバッファのレイヤ
		 * @param eye 次に描画するフレームバッファが右目化左目化を指定
		 * @return レンダーターゲットへのポインタ
		 */
		Compat::RenderTarget	*getNextRenderTarget(int	bus = SCE_VIDEO_OUT_BUS_TYPE_MAIN, HmdLayer layer = HmdLayer::kVr3d, HmdEye eye = HmdEye::kLeft);
		RenderTargetWrapper	*getNextRenderTargetWrapper(int	bus = SCE_VIDEO_OUT_BUS_TYPE_MAIN, HmdLayer layer = HmdLayer::kVr3d, HmdEye eye = HmdEye::kLeft);
		/*!
		 * @~English
		 * @brief Gets the depth and stencil target for frame buffer.
		 * @param bus Bus type
		 * @param layer Frame buffer layer of the frame buffer to be drawn next.
		 * @param eye Specify the eye you are going to render.
		 * @details This gets the depth and stencil target for frame buffer.
		 *
		 * @return Pointer to a depth and stencil target
		 * @~Japanese
		 * @brief フレームバッファ用デプスステンシルターゲットを取得する
		 * @param bus バスタイプ
		 * @param layer 次に描画するフレームバッファのレイヤ
		 * @param eye 次に描画するフレームバッファが右目化左目化を指定
		 * @details フレームバッファ用デプスステンシルターゲットを取得します。
		 *
		 * @return デプスステンシルターゲットへのポインタ
		 */
		Compat::DepthRenderTarget	*getDepthStencilSurface(int	bus = SCE_VIDEO_OUT_BUS_TYPE_MAIN, HmdLayer layer = HmdLayer::kVr3d, HmdEye eye = HmdEye::kLeft);

		/*!
		 * @~English
		 * @brief Starts drawing on the render target.
		 * @details This starts drawing on the render target.
		 *
		 * @param renderTarget Render target
		 * @param depthStencilSurface Depth stencil target
		 * @param numRenderTargets The number of rendertargets
		 * @param fsr Specify target to render with FSR
		 * @param currentCtx Render this scene by provided Agc::Core::BasicContext
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief レンダーターゲットへの描画を開始する
		 * @details レンダーターゲットへの描画を開始します。
		 *
		 * @param renderTarget レンダーターゲット
		 * @param depthStencilSurface デプスステンシルターゲット
		 * @param numRenderTargets レンダーターゲットの数
		 * @param fsr FSRで描画するターゲットを指定
		 * @param currentCtx 与えられたAgc::Core::BasicContextでシーンを描画
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int	beginScene(Compat::RenderTarget	*renderTarget, Compat::DepthRenderTarget	*depthStencilSurface, int	numRenderTargets = 1
#if _SCE_TARGET_OS_PROSPERO
						, HmdFsrTarget	fsr = HmdFsrTarget::kNone, sce::Agc::Core::BasicContext	*currentCtx = nullptr
#endif
		);
		/*!
		 * @~English
		 * @brief Terminates drawing on the render target and depth render target.
		 * @details This terminates drawing on the render target and depth render target, and waits for GL2 flush.
		 * @param renderResult The result returned from rendering in this scene
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief レンダーターゲット・デプスレンダーターゲットへの描画を終了する
		 * @details レンダーターゲット・デプスレンダーターゲットへの描画が完了し、GL2キャッシュへのフラッシュの完了を待ちます。
		 * @param renderResult このシーンの描画から返ってきたresult
		 *
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
#if _SCE_TARGET_OS_PROSPERO
		int	endScene(sce::Agc::Toolkit::Result	renderResult = {SCE_OK, sce::Agc::Toolkit::Result::StateChange::kNone, sce::Agc::Toolkit::Result::ActiveWork::kGsPs, sce::Agc::Toolkit::Result::Caches::kCbData | sce::Agc::Toolkit::Result::Caches::kDbData}, sce::Agc::Core::BasicContext	*currentCtx = nullptr);
#endif
#if _SCE_TARGET_OS_ORBIS
		int	endScene();
#endif

		/*!
		 * @~English
		 * @brief Flip processing of a frame buffer
		 * @details This executes flip processing of a frame buffer
		 *
		 * @param numVSync Number of VSYNCs to wait until flip(This parameter is invalid in case of reprojection)
		 * @param bus VideoOut port to flip
		 * @param flipArg argument to be passed to setFlip
		 * @param reprojectionParam Reprojection parameter
		 * @param currentCtx Set this if you want to use Agc::Core::BasicContext provided externally
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief フレームバッファのフリップ処理
		 * @details フレームバッファのフリップ処理を行います。
		 *
		 * @param numVSync フリップまでに待つVSYNC数(リプロジェクション時は無効)
		 * @param bus フリップするVideoOutポート
		 * @param flipArg setFlipに渡すflipArg
		 * @param reprojectionParam リプロジェクションパラメータ
		 * @param currentCtx 外部から与えたAgc::Core::BasicContextでflipする場合に使用
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int	flip(uint32_t	numVSync, int	bus = SCE_VIDEO_OUT_BUS_TYPE_MAIN, uint64_t	flipArg = 0
#if _SCE_TARGET_OS_PROSPERO
			, const ReprojectionParameter	*reprojectionParam = nullptr,
			sce::Agc::Core::BasicContext	*currentCtx = nullptr
#endif
		);

		/*!
		 * @~English
		 * @brief obtain flip status
		 * @details This sets SceVideoOutFlipStatus value for specified scanout bus
		 *
		 * @param outFlipStatus Variable where flip status is stored
		 * @param bus VideoOut port to obtain flip status
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief フリップの状態 の取得
		 * @details この関数は指定されたスキャンアウトバスのフリップの状態を取得します
		 *
		 * @param outFlipStatus フリップの状態が格納される変数
		 * @param bus フリップの状態を取得するVideoOutポート
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int	getFlipStatus(SceVideoOutFlipStatus	&outFlipStatus, int	bus = SCE_VIDEO_OUT_BUS_TYPE_MAIN);

		/*!
		 * @~English
		 * @brief obtain submitted flip count
		 * @details This sets submitted flip count for specified scanout bus
		 *
		 * @param outFlipCount Variable where submitted flip count is stored
		 * @param bus VideoOut port to obtain submitted flip count
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief 投入済みのフリップ回数の取得
		 * @details この関数は指定されたスキャンアウトバスの投入済みのフリップ回数を取得します
		 *
		 * @param outFlipCount フリップ回数が格納される変数
		 * @param bus フリップ回数を取得するVideoOutポート
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int	getSubmittedFlipCount(uint64_t	&outFlipCount, int	bus = SCE_VIDEO_OUT_BUS_TYPE_MAIN);

		/*!
		 * @~English
		 * @brief obtain VideoOut port handle
		 * @details This sets port handle for specified scanout bus
		 *
		 * @param outVideoOutPortHandle Variable where video out port handle is stored
		 * @param bus VideoOut port to obtain video out port handle
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief ビデオ出力ポートのポートハンドルを取得
		 * @details この関数は指定されたスキャンアウトバスのポートハンドルを取得します
		 *
		 * @param outVideoOutPortHandle ビデオ出力ポートハンドルが格納される変数
		 * @param bus ポートハンドルを取得するビデオ出力ポート
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int	getVideoOutPortHandle(int	&outVideoOutPortHandle, int	bus = SCE_VIDEO_OUT_BUS_TYPE_MAIN);

		/*!
		 * @~English
		 * @brief Gets the current render target.
		 * @details This gets the current render target.
		 * @param mrt_i MRT index
		 *
		 * @return Current render target
		 * @~Japanese
		 * @brief 現在のレンダーターゲットを取得する
		 * @details 現在のレンダーターゲットを取得します。
		 * @param mrt_i MRTインデックス
		 *
		 * @return 現在のレンダーターゲット
		 */
		Compat::RenderTarget	*getCurrentRenderTarget(int	mrt_i = 0);

		/*!
		 * @~English
		 * @brief Gets the current depth and stencil targets.
		 * @details This gets the current depth and stencil targets.
		 *
		 * @return Current depth and stencil targets
		 * @~Japanese
		 * @brief 現在のデプスステンシルターゲットを取得する
		 * @details 現在のデプスステンシルターゲットを取得します。
		 *
		 * @return 現在のデプスステンシルターゲット
		 */
		Compat::DepthRenderTarget	*getCurrentDepthStencilSurface();

		/*!
		 * @~English
		 * @brief Change graphics context settings after it is created.
		 * @details This changes video out settings such as HDR or SDR while application is running.
		 *
		 * @param option new option to be set
		 * @param width new frame buffer width to be set(is not changed if 0 is specified)
		 * @param height new frame buffer height to be set(is not changed if 0 is specified)
		 * @retval SCE_OK Success
		 * @retval SCE_VIDEO_OUT_ERROR_UNAVAILABLE_OUTPUT_MODE Failed to configure Video Out port
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief グラフィックスコンテキストのオプションをコンテキスト作成後に変更
		 * @details 現在設定されている、HDR or SDRなどのビデオアウトの設定をアプリケーション実行中に切り替えます。
		 * @param option 変更後のオプション
		 * @param width 変更後のフレームバッファの幅(0を指定した場合は変更しない)
		 * @param height 変更後のフレームバッファの高さ(0を指定した場合は変更しない)
		 * @retval SCE_OK 成功
		 * @retval SCE_VIDEO_OUT_ERROR_UNAVAILABLE_OUTPUT_MODE VideoOutポートの設定に失敗した
		 * @retval (<0) エラーコード
		 */
		int changeOption(const GraphicsContextOption	&option, uint32_t	width = 0, uint32_t	height = 0);

		/*!
		 * @~English
		 * @brief wait until V-blank
		 *
		 * @param bus VideoOut port to wait for V-blank
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief V-blankまで待つ
		 *
		 * @param bus V-blankを待つVideoOutポート
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int waitVBlank(int	bus = SCE_VIDEO_OUT_BUS_TYPE_MAIN);

		/*!
		 * @~English
		 * @brief wait until flip event
		 *
		 * @param bus VideoOut port to wait for flip event
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief フリップイベントの発生を待つ
		 *
		 * @param bus フリップイベントを待つVideoOutポート
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int waitFlip(int	bus = SCE_VIDEO_OUT_BUS_TYPE_MAIN);

#if _SCE_TARGET_OS_PROSPERO
		/*!
		 * @~English
		 * @brief Set bus blend setting of VideoOut
		 * @details Sets blend alpha value, blend space and alpha mode for bus
		 * @param bus	bus to change blend setting(OVERLAY)
		 * @param globalAlphaMode	Specify SCE_VIDEO_OUT_GLOBAL_ALPHA_MODE_NORMAL or SCE_VIDEO_OUT_GLOBAL_ALPHA_MODE_GLOBAL_ALPHA_ONLY
		 * @param globalAlphaValue	Specify value from 0 to 255. Default value is 255.
		 * @param space	Specify SCE_VIDEO_OUT_GLOBAL_BLEND_SPACE_GAMMA or SCE_VIDEO_OUT_GLOBAL_BLEND_SPACE_LINEAR
		 * @param a2lut	Specify pixel alpha value look-up table for 2bits(4 values) alpha format. Default value is {0, 85, 170, 255}. specify nullptr if you don't need to modify it. Since the content of a2lut array is saved in system, you can discard this array after returning from this function.
		 *
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 *
		 * @~Japanese
		 * @brief VideoOutのbusブレンドの設定
		 * @details busのブレンド時のalpha値やブレンド空間、alphaモードを設定する
		 * @param bus	ブレンド設定を行うbus(OVERLEY)
		 * @param globalAlphaMode	SCE_VIDEO_OUT_GLOBAL_ALPHA_MODE_NORMALかSCE_VIDEO_OUT_GLOBAL_ALPHA_MODE_GLOBAL_ALPHA_ONLYのいずれかを指定してください。
		 * @param globalAlphaValue	0から255の値を指定してください。デフォルト値は255です。
		 * @param space	SCE_VIDEO_OUT_GLOBAL_BLEND_SPACE_GAMMAかSCE_VIDEO_OUT_GLOBAL_BLEND_SPACE_LINEARのいずれかを指定してください。
		 * @param a2lut	アルファが2bit（4値）のピクセルフォーマットに対して、ピクセルアルファの値のルックアップテーブルを指定します。デフォルトは{0, 85, 170, 255}です。変更が不要な場合にはNULLを指定してください。a2lutで指定した配列の内容はシステム内部に保持されるため、本関数からリターンしたあとは内容を書き換えても破棄しても問題ありません。
		 *
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int	setVideoOutBlendControl(SceVideoOutBusType	bus, int	globalAlphaMode, int	globalAlphaValue = SCE_VIDEO_OUT_GLOBAL_ALPHA_MODE_NORMAL, uint32_t	space = SCE_VIDEO_OUT_GLOBAL_BLEND_SPACE_LINEAR, const uint8_t	*a2lut = nullptr);

		/*!
		 * @~English
		 * @brief Set FsrView based on FoveatedViewSpec
		 * @details Updates FsrView based on specified FoveatedViewSpec, and applies to rendering and flip after this point.
		 * @param eye eye to which FsrView is set
		 * @param spec FsrView is updated based on specified FoveatedViewSpec
		 *
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 *
		 * @~Japanese
		 * @brief FoveatedViewSpecでFsrViewを更新
		 * @details 指定したFoveatedViewSpecでFsrViewを更新し、以後の描画・フリップに適用する
		 * @param eye FsrViewを更新する目
		 * @param spec specで指定したFoveatedViewSpecでFsrViewが更新される
		 *
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int setFsrView(HmdEye eye, const sce::FrToolkit::FoveatedViewSpec &spec);

		/*!
		 * @~English
		 * @brief Set FsrView to be optimized for HMD lens
		 * @param width Screen width(frame buffer width is used when 0 is specified)
		 * @param height Screen height(frame buffer height is used when 0 is specified)
		 * @param isSideBySide true: side-by-side screen, false: per-eye screen(frame buffer's side-by-side setting is used when frame buffer width/height is used)
		 * @details Sets SCE_HMD2_FSR_CURVE_LEFT and SCE_HMD2_FSR_CURVE_RIGHT to FsrView
		 *
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 *
		 * @~Japanese
		 * @brief HMDレンズに最適化されたFsrViewの設定
		 * @param width スクリーンの幅(0を指定した場合、フレームバッファのスクリーンの幅が適用される)
		 * @param height スクリーンの高さ(0を指定した場合、フレームバッファのスクリーンの高さが適用される)
		 * @param isSideBySide true: スクリーンはside-by-side, false: スクリーンはper-eye(フレームバッファの幅・高さが適用される際には、フレームバッファのside-by-side設定が適用される)
		 * @details SCE_HMD2_FSR_CURVE_LEFT,SCE_HMD2_FSR_CURVE_RIGHTをFsrViewに設定
		 *
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int setLensOptimizedFsrView(uint32_t	width = 0, uint32_t	height = 0, bool	isSideBySide = false);
#endif

		/*!
		 * @~English
		 * @brief This generates a texture from an image file.
		 *
		 * @param outTexture Reference to generated texture
		 * @param filename Image file name
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief 画像ファイルからテクスチャを生成する
		 *
		 * @param outTexture 生成されたテクスチャが返される参照
		 * @param filename 画像ファイル名
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int createTextureFromFile(Compat::Texture &outTexture, const char *filename);

		/*!
		 * @~English
		 * @brief This generates a texture from an image data on memory.
		 *
		 * @param outTexture Reference to generated texture
		 * @param pImageData Pointer to image data on memory
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief メモリ上の画像データからテクスチャを生成する
		 *
		 * @param outTexture 生成されたテクスチャが返される参照
		 * @param pImageData 画像データの先頭を指すポインタ
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int createTextureFromMemory(Compat::Texture &outTexture, const void *pImageData);

		/*!
		 * @~English
		 * @brief This destroys a texture.
		 *
		 * @param texture Reference to texture to be destroyed
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief テクスチャを破棄する
		 *
		 * @param texture 破棄するテクスチャへの参照
		 * @retval SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int destroyTexture(Compat::Texture &texture);

		/*!
		 * @~English
		 * @brief Destroys model instance and model
		 *
		 * @param pModelInstance Model instance
		 * @~Japanese
		 * @brief モデルインスタンスとモデルの破棄
		 *
		 * @param pModelInstance モデルインスタンス
		 */
		void	destroyPackModelInstanceAndPackModel(PackModelInstance	*pPackModelInstance)
		{
			delete pPackModelInstance->m_pPackModel;
			delete pPackModelInstance;
		}

#if _SCE_TARGET_OS_PROSPERO
		/*!
		 * @~English
		 * @brief This updates dirty caches based on bound render target and depth render target.
		 *
		 * @param dirtyCaches dirty caches returned from render
		 * @~Japanese
		 * @brief バインドされているレンダーターゲットとデプスレンダーターゲットに基づいてdirty cachesを更新する
		 *
		 * @param dirtyCaches レンダーリングAPIがリターンしたdirty caches
		 */
		void	updateDirtyCaches(sce::Agc::Toolkit::Result::Caches	&dirtyCaches);
#endif

		/*!
		 * @~English
		 * @brief PackModel deferred release
		 * @details Released GPU memory after GPU's reference to it
		 * @param pModel Model to be released
		 * @~Japanese
		 * @brief PackModel遅延解放
		 * @details GPUでの使用が完了するのを待ってから解放
		 * @param pModel 解放するモデル
		 */
		void	deferredReleasePackModel(PackModel	*pPackModel);

		/*!
		 * @~English
		 * @brief PackModel Instance deferred release
		 * @details Released GPU memory after GPU's reference to it
		 * @param pPackModelInstance Model Instance to be released
		 * @~Japanese
		 * @brief PackModelインスタンス遅延解放
		 * @details GPUでの使用が完了するのを待ってから解放
		 * @param pPackModelInstance 解放するモデルインスタンス
		 */
		void	deferredReleasePackModelInstance(PackModelInstance	*pPackModelInstance);
	};
} } } // namespace sce::SampleUtil::Graphics
