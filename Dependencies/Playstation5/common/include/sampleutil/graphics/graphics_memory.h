/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <sys/types.h> // off_t
#include <kernel.h>
#include <scebase_common.h>
#include "sampleutil/graphics/compat.h"
#include "sampleutil/memory/heap_allocator.h"
#include "sampleutil/memory/ring_allocator.h"

namespace sce { namespace SampleUtil { namespace Graphics {

extern bool	g_isResourceRegistrationInitialized;

/*!
 * @~English
 * @brief Initializes ResourceRegistration
 * @param maxOwnersAndResources The number of owners and resources to be supported
 * @param maxNameLength The maximum length of a name to be stored (including <c>NULL</c> terminator)
 * @~Japanese
 * @brief ResourceRegistrationの初期化
 * @param maxOwnersAndResources サポートするオーナーとリソースの数
 * @param maxNameLength 格納する文字の最大長(<c>NULL</c>終端を含む)
 */
void	initializeResourceRegistration(uint32_t	maxOwnersAndResources, uint32_t	maxNameLength);
/*!
 * @~English
 * @brief Finalizes ResourceRegistration
 * @~Japanese
 * @brief ResourceRegistrationの終了処理
 */
void	finalizeResourceRegistration();

/*!
 * @~English
 * @brief Video memory allocator
 * @~Japanese
 * @brief ビデオメモリアロケータ
 */
class VideoAllocator : public sce::SampleUtil::Memory::HeapAllocator
{
public:
	off_t					m_start;
	SceKernelMemoryType		m_type;
	Compat::OwnerHandle		m_owner;

	/*!
	 * @~English
	 * @brief Constructor
	 * @param size heap size(in bytes)
	 * @param type The type of direct memory to be allocated
	 * @param ammDirect Specify if allocated as AMM DIRECT region
	 * @param prot Protection attribute
	 * @param pName The name of allocator
	 * @~Japanese
	 * @brief コンストラクタ
	 * @param size ヒープサイズ(バイト)
	 * @param type 確保するダイレクトメモリのタイプ
	 * @param ammDirect AMM DIRECT領域として確保するかどうか
	 * @param prot プロテクション属性
	 * @param pName アロケータの名前
	 */
#if _SCE_TARGET_OS_PROSPERO
	VideoAllocator(size_t	size, SceKernelMemoryType	type, bool ammDirect = false, int	prot = SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW | SCE_KERNEL_PROT_AMPR_RW, const char	*pName = "VIDEO MEMORY");
#endif
#if _SCE_TARGET_OS_ORBIS
	VideoAllocator(size_t	size, SceKernelMemoryType	type, int	prot = SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW, const char	*pName = "VIDEO MEMORY");
#endif
	VideoAllocator(const VideoAllocator&) = delete;
	virtual ~VideoAllocator();
	/*!
	 * @~English
	 * @brief Initializes allocator
	 * @param size heap size(in bytes)
	 * @param type The type of direct memory to be allocated
	 * @param pName Allocator name
	 * @param ammDirect Specify if allocated as AMM DIRECT region
	 * @param prot Protection attribute
	 * @~Japanese
	 * @brief 初期化
	 * @param size ヒープサイズ(バイト)
	 * @param type 確保するダイレクトメモリのタイプ
	 * @param pName アロケータの名前
	 * @param ammDirect AMM DIRECT領域として確保するかどうか
	 * @param prot プロテクション属性
	 */
#if _SCE_TARGET_OS_PROSPERO
	void	initialize(size_t	size, SceKernelMemoryType	type, const char	*pName = "VIDEO MEMORY", bool ammDirect = false, int	prot = SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW | SCE_KERNEL_PROT_AMPR_RW);
#endif
#if _SCE_TARGET_OS_ORBIS
	void	initialize(size_t	size, SceKernelMemoryType	type, const char	*pName = "VIDEO MEMORY", int	prot = SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW);
#endif
	/*!
	 * @~English
	 * @brief Registers resource name and resource types
	 * @param ptr Address of memory to be registered
	 * @param size Size of memory to be registered(in bytes)
	 * @param name Registration name
	 * @param resTypes Registration resource types
	 * @~Japanese
	 * @brief リソース名とリソースタイプの登録
	 * @param ptr 登録するメモリの先頭アドレス
	 * @param size 登録するメモリの先頭サイズ(バイト)
	 * @param name 登録名
	 * @param resTypes 登録リソースタイプ
	 */
	virtual void	registerResource(const void	*ptr, size_t	size, const std::string	&name, const std::vector<Compat::ResourceType>	&resTypes);
	/*!
	 * @~English
	 * @brief Unregisters resource
	 * @param ptr Address of memory to be unregistered
	 * @~Japanese
	 * @brief リソースの登録解除
	 * @param ptr 登録を解除するメモリの先頭アドレス
	 */
	void	unregisterResource(const void	*ptr);
	/*!
	 * @~English
	 * @brief Memory free call back
	 * @details This is called when memory is freed
	 * @param ptr Address of freed memory
	 * @param size Size of freed memory(in bytes)
	 * @~Japanese
	 * @brief メモリ解放コールバック
	 * @details メモリ解放の際に呼び出されます
	 * @param ptr 解放したメモリの先頭アドレス
	 * @param size 解放したメモリのサイズ(バイト)
	 */
	virtual void	freeHook(void	*ptr, size_t	size);

	using sce::SampleUtil::Memory::HeapAllocator::allocate;
	/*!
	 * @~English
	 * @brief Memory allocation by using SizeAlign
	 * @param sizeAlign Size and align of memory to be allocated
	 * @param name Resource registration name
	 * @return Top address of allocated memroy
	 * @~Japanese
	 * @brief SizeAlign指定でのメモリ確保
	 * @param sizeAlign 確保するメモリのサイズとアライン(バイト)
	 * @param name リソース登録名
	 * @return 確保されたメモリアドレス
	 */
	void	*allocate(const Compat::SizeAlign &sizeAlign, const std::string	&name = "")
	{
		return allocate(sizeAlign.m_size, sizeAlign.m_align, name);
	}
};

/*!
 * @~English
 * @brief Video memory ring allocator
 * @~Japanese
 * @brief ビデオメモリリングアロケータ
 */
class VideoRingAllocator : public sce::SampleUtil::Memory::RingAllocator
{
public:
	SceKernelMemoryType		m_type;
	Compat::OwnerHandle		m_owner;
	struct FrameInfo
	{
		void															*m_label;
#ifdef _DEBUG
		std::vector<sce::SampleUtil::Graphics::Compat::ResourceHandle>	m_registeredResources;
#endif
	};
	std::deque<FrameInfo>	m_frameInfos;

	/*!
	 * @~English
	 * @brief Constructor
	 * @param size Size(in bytes) of memory managed by ring allocator
	 * @param type The type of direct memory managed by ring allocator
	 * @param pName Allocator name
	 * @~Japanese
	 * @brief コンストラクタ
	 * @param size リングアロケータで管理するメモリのサイズ(バイト)
	 * @param type リングアロケータで管理するダイレクトメモリのタイプ
	 * @param pName アロケータ名
	 */
	VideoRingAllocator(size_t	size, SceKernelMemoryType	type, const char	*pName = "RING BUFFER VIDEO MEMORY");
	VideoRingAllocator(const VideoRingAllocator &) = delete;
	virtual ~VideoRingAllocator();
	/*!
	 * @~English
	 * @brief Initializes video memory ring allocator
	 * @param size Size(in bytes) of memory managed by ring allocator
	 * @param type The type of direct memory to be allocated
	 * @param pName Allocator name
	 * @~Japanese
	 * @brief ビデオメモリリングアロケータを初期化
	 * @param size リングアロケータで管理するメモリのサイズ(バイト)
	 * @param type 確保するダイレクトメモリのタイプ
	 * @param pName アロケータ名
	 */
	void	initialize(size_t	size, SceKernelMemoryType	type, const char	*pName = "RING BUFFER VIDEO MEMORY");
	/*!
	 * @~English
	 * @brief The function to be called at the beginning of frame
	 * @details This function invalidates GPU GL0/1/2 caches so that memory allocated here after can be initialized by CPU, then can be handed to GPU.
	 * @param dcb DrawCommandBuffer to be used for inivalidating GPU GL0/1/2 caches
	 * @~Japanese
	 * @brief フレームの先頭で呼ぶ関数
	 * @details この関数を呼び出すとGPUのGL0/1/2キャッシュがinvalidateされ、これ以降確保したメモリをCPUで初期化してGPUに渡せるようになります。
	 * @param dcb GPU GL0/1/2キャッシュをinvalidateするために使用するDrawCommandBuffer
	 */
	void	beginFrame(Compat::DrawCommandBuffer	&dcb);
	/*!
	 * @~English
	 * @brief The function to be called at the end of frame
	 * @details This function submits allocated memory in this frame, and submitted memory can be reclaimed after GPU finished using it.
	 * @param dcb DrawCommandBuffer to be used for submitting memory
	 * @~Japanese
	 * @brief フレームの終わりに呼ぶ関数
	 * @details この関数を呼び出すと現フレームで確保されたメモリがGPUに投入されます。投入されたメモリはGPUでの使用が終わると再び利用できるようになります。
	 * @param dcb メモリをGPUに投入するために使用するDrawCommandBuffer
	 */
	void	endFrame(Compat::DrawCommandBuffer	&dcb);
	/*!
	 * @~English
	 * @brief Registers resource name and resource types
	 * @param ptr Address of memory to be registered
	 * @param size Size of memory to be registered(in bytes)
	 * @param name Registration name
	 * @param resTypes Registration resource types
	 * @~Japanese
	 * @brief リソース名とリソースタイプの登録
	 * @param ptr 登録するメモリの先頭アドレス
	 * @param size 登録するメモリの先頭サイズ(バイト)
	 * @param name 登録名
	 * @param resTypes 登録リソースタイプ
	 */
	void	registerResource(const void	*ptr, size_t	size, const std::string	&name, const std::vector<Compat::ResourceType>	&resTypes);
#ifdef _DEBUG
	/*!
	 * @~English
	 * @brief Unregisters resources
	 * @param resourceHandles handles of resources to be unregistered
	 * @~Japanese
	 * @brief リソースの登録解除
	 * @param resourceHandles 登録を解除するリソースのハンドル
	 */
	void	unregisterResources(const std::vector<sce::SampleUtil::Graphics::Compat::ResourceHandle>	&resourceHandles);
#endif
	/*!
	 * @~English
	 * @brief Memory allocation call back
	 * @details This is called when memory is allocated
	 * @param ptr Address of allocated memory
	 * @param size Size of allocated memory(in bytes)
	 * @param name Resource name
	 * @~Japanese
	 * @brief メモリ確保コールバック
	 * @details メモリ確保の際に呼び出されます
	 * @param ptr 確保したメモリの先頭アドレス
	 * @param size 確保したメモリのサイズ(バイト)
	 * @param name リソース名
	 */
	virtual void	allocateHook(void	*ptr, size_t	size, const std::string	&name);
	/*!
	 * @~English
	 * @brief Memory free call back
	 * @details This is called when memory is freed
	 * @param ptr Address of freed memory
	 * @param size Size of freed memory(in bytes)
	 * @~Japanese
	 * @brief メモリ解放コールバック
	 * @details メモリ解放の際に呼び出されます
	 * @param ptr 解放したメモリの先頭アドレス
	 * @param size 解放したメモリのサイズ(バイト)
	 */
	virtual void	freeHook(void	*ptr, size_t	size);
	/*!
	 * @~English
	 * @brief Callback called when running short of frame memory
	 * @details Switches to next frame if next frame is available, otherwise blocks until it becomes abailable
	 * @retval true endFrame() was called
	 * @retval false no endFrame() was called
	 * @~Japanese
	 * @brief フレームメモリを使い果たした際に呼ばれるコールバック
	 * @details 次のフレームが空いていればフレームを切り替えます。そうでなければ空くまで内部で待機します。
	 * @retval true endFrame() が呼ばれた
	 * @retval false endFrame() が呼ばれなかった
	 */
	virtual bool	allocateFailedHook();
	using sce::SampleUtil::Memory::RingAllocator::allocate;
	/*!
	 * @~English
	 * @brief Memory allocation by using SizeAlign
	 * @param sizeAlign Size and align of memory to be allocated
	 * @param name Resource registration name
	 * @~Japanese
	 * @brief SizeAlign指定でのメモリ確保
	 * @param sizeAlign 確保するメモリのサイズとアライン(バイト)
	 * @param name リソース登録名
	 */
	void	*allocate(const Compat::SizeAlign &sizeAlign, const std::string	&name = "")
	{
		return allocate(sizeAlign.m_size, sizeAlign.m_align, name);
	}
};
}}} // namespace sce::SampleUtil::Graphics
