/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. 
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <sce_atomic.h>
#include <agc/baselayer.h>
#include "sampleutil/memory/dmem_mapper.h"
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/sampleutil_error.h"
#include "sampleutil/helper/prospero/async_asset_loader_prospero.h"
#include "sampleutil/graphics/graphics_memory.h"
#include "sampleutil/debug/perf.h"

namespace
{
	struct ScopedLock
	{
		ScePthreadMutex	&m_lock;
		ScopedLock(ScePthreadMutex &lock) :m_lock(lock)
		{
			int ret = SCE_OK; (void)ret;
			ret = scePthreadMutexLock(&m_lock);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}
		~ScopedLock()
		{
			int ret = SCE_OK; (void)ret;
			ret = scePthreadMutexUnlock(&m_lock);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}
	};

	void	*allocateLinear(uint32_t	size, uint32_t	align, void *base, size_t	maxSize, size_t	&usedSize)
	{
		usedSize = ((usedSize + align - 1) / align) * align;
		void *ptr = (void *)((uintptr_t)base + usedSize);
		usedSize += size;
		return (usedSize <= maxSize) ? ptr : nullptr;
	}

	static constexpr uint32_t kAmmCbSize = 256;
	static constexpr uint32_t kAprCbSize = 256;
} // anonymous namespace

namespace sce { namespace SampleUtil { namespace Helper {
	AsyncAssetLoader::AsyncAssetLoader(const AsyncAssetLoaderOption	&option)
	{
		TAG_THIS_CLASS;
		int ret = SCE_OK; (void)ret;
#ifdef _DEBUG
		if (Graphics::g_isResourceRegistrationInitialized)
		{
			ret = Agc::ResourceRegistration::registerOwner(&m_owner, "sce::SampleUtil::Helper::AAL");
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
			if (Memory::g_isMatInitialized && (ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG))
			{
				sceMatAgcRegisterOwner(m_owner, "sce::SampleUtil::Helper::AAL");
			}
		}
		if (Memory::g_isMatInitialized)
		{
			Memory::registerMatGroup(this, "sce::SampleUtil::Helper::AAL");
		}
#endif
		SCE_SAMPLE_UTIL_ASSERT(option.virtualAddressSizeInBytes % (2ul * 1024ul * 1024ul) == 0);
		m_mappableVirtualAddressSizeInBytes = option.virtualAddressSizeInBytes;
		m_mappableVirtualAddressStart = (void *)Memory::allocateAmmVirtualAddressRange(m_mappableVirtualAddressSizeInBytes, 2ul * 1024ul * 1024ul);
		SCE_SAMPLE_UTIL_ASSERT(m_mappableVirtualAddressStart != 0ul);
		m_usedVirtualAddressSizeInBytes = 0ul;

		const uint32_t kDeviceMemoryPerAsset = 32 * 1024;
		m_deviceMemorySizeInBytes = option.numMaxAssets * kDeviceMemoryPerAsset;
		Memory::mapDirectMemory(&m_pDeviceMemory, m_deviceMemorySizeInBytes, SCE_KERNEL_MTYPE_C_SHARED, SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW | SCE_KERNEL_PROT_AMPR_ALL, 0, SCE_KERNEL_PAGE_SIZE);
		SCE_SAMPLE_UTIL_ASSERT(m_pDeviceMemory != nullptr);
		m_usedDeviceMemorySizeInBytes = 0ul;

		m_assets.clear();

		constexpr int kAmmEqId = 0x100;
		m_ammListener.initialize(kAmmEqId);

		constexpr int kAprEqId = 0x101;
		m_aprListener.initialize(kAprEqId);
	}

	AsyncAssetLoader::~AsyncAssetLoader()
	{
		UNTAG_THIS_CLASS;
		int ret = SCE_OK; (void)ret;

#ifdef _DEBUG
		if (Graphics::g_isResourceRegistrationInitialized)
		{
			ret = Agc::ResourceRegistration::unregisterOwnerAndResources(m_owner);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
			if (Memory::g_isMatInitialized && (ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG))
			{
				sceMatAgcUnregisterOwnerAndResources(m_owner);
			}
		}
#endif
		for (auto asset : m_assets)
		{
			const std::string	&assetName	= asset.first.c_str();
			AssetInfo			info		= asset.second;
			release(assetName.c_str());
			m_ammListener.waitIf({ info.m_pMapState, EventListener::Condition::kEqual, (uint64_t)MapState::kUnmapping, nullptr, 0ul });
		}
		m_assets.clear();
		m_loadStates.clear();

		m_ammListener.finalize();
		m_aprListener.finalize();

		ret = Memory::munmap(m_pDeviceMemory, m_deviceMemorySizeInBytes);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		Memory::freeAmmVirtualAddressRange((uintptr_t)m_mappableVirtualAddressStart);
	}

	void	AsyncAssetLoader::EventListener::initialize(int	eqId)
	{
		int ret = SCE_OK;

		m_eqId = eqId;
		m_isValid = true;

		ret = sceKernelCreateEqueue(&m_eventQueue, "sce::SampleUtil::Helper::AAL");
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		ret = sceKernelAddAmprEvent(m_eventQueue, eqId, nullptr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceKernelAddUserEvent(m_eventQueue, eqId);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		ScePthreadCondattr condAttr;
		scePthreadCondattrInit(&condAttr);
		ret = scePthreadCondInit(&m_cond, &condAttr, "sce::SampleUtil::Helper::AAL");
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		scePthreadCondattrDestroy(&condAttr);


		ScePthreadMutexattr mutexAttr;
		scePthreadMutexattrInit(&mutexAttr);
		ret = scePthreadMutexInit(&m_lockForCond, &mutexAttr, "sce::SampleUtil::Helper::AAL");
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		scePthreadMutexattrDestroy(&mutexAttr);

		ret = scePthreadCreate(&m_listenerThr, nullptr,
								[](void *arg)->void *
								{
									reinterpret_cast<EventListener *>(arg)->run();

									return nullptr;
								}, this,
								"sce::SampleUtil::Helper::AAL");
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	}

	void	AsyncAssetLoader::EventListener::finalize()
	{
		int ret = SCE_OK; (void)ret;

		m_isValid = false;
		ret = sceKernelTriggerUserEvent(m_eventQueue, m_eqId, nullptr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = scePthreadJoin(m_listenerThr, nullptr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = scePthreadMutexDestroy(&m_lockForCond);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = scePthreadCondDestroy(&m_cond);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceKernelDeleteEqueue(m_eventQueue);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	}

	void	AsyncAssetLoader::EventListener::run()
	{
		int ret = SCE_OK; (void)ret;

		while(m_isValid)
		{
			decltype(m_waiters)	wokenUpWaiters;;

			{
				ScopedLock criticalSection(m_lockForCond);

				bool needWakeup = false;
				decltype(m_waiters) newWaiters;
				for (WaitSpec spec : m_waiters)
				{
					if ((spec.m_waitCondition == Condition::kEqual && spec.m_pState->m_value == spec.m_checkValue) ||
						(spec.m_waitCondition == Condition::kNotEqual && spec.m_pState->m_value != spec.m_checkValue)) {
						newWaiters.push_back(spec);
					} else if (spec.m_onWaitDone != nullptr) {
						wokenUpWaiters.push_back(spec);
					} else {
						needWakeup = true;
					}
				}
				m_waiters = newWaiters;
				if (needWakeup)
				{
					ret = scePthreadCondSignal(&m_cond);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				}
			}

			for (WaitSpec waiter : wokenUpWaiters) {
				waiter.m_onWaitDone(waiter.m_onWaitDoneArg);
			}

			SceKernelEvent ev;
			int num;
			ret = sceKernelWaitEqueue(m_eventQueue, &ev, 1, &num, nullptr);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}
	}

	void	AsyncAssetLoader::EventListener::waitIf(WaitSpec	spec)
	{
		int ret = SCE_OK; (void)ret;

		ScopedLock	criticalSection(m_lockForCond);

		if ((spec.m_waitCondition == Condition::kEqual && spec.m_pState->m_value != spec.m_checkValue) ||
			(spec.m_waitCondition == Condition::kNotEqual && spec.m_pState->m_value == spec.m_checkValue)) {
			return;
		}

		m_waiters.push_back(spec);

		if (spec.m_onWaitDone == nullptr) {
			while ((spec.m_waitCondition == Condition::kEqual && spec.m_pState->m_value == spec.m_checkValue) ||
				(spec.m_waitCondition == Condition::kNotEqual && spec.m_pState->m_value != spec.m_checkValue)) {
				ret = scePthreadCondWait(&m_cond, &m_lockForCond);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			}
		}
	}

	AsyncAssetLoader::Result	AsyncAssetLoader::allocate(const char *tag, size_t	sizeInBytes, uint32_t	alignment, SceKernelMemoryType type, int prot, const std::vector<sce::Agc::ResourceRegistration::ResourceType> &resourceTypes)
	{
		int ret = SCE_OK; (void)ret;

		AssetInfo info;
		info.m_loadStatus.m_ptr = nullptr;

		{
			Thread::ScopedLock criticalSection(this, Thread::LockableObjectAccessAttr::kWrite);

			if (m_assets.find(tag) != m_assets.end())
			{
				info = m_assets[tag];
			} else {
				// allocate virtual address
				if (sizeInBytes == 0ul) {
					const char	*filePath = tag;
					SceKernelStat	stat;
					ret = sceKernelStat(filePath, &stat);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					sizeInBytes = stat.st_size;
				}
				void	*va = allocateLinear(sizeInBytes, std::max((uint32_t)SCE_KERNEL_PAGE_SIZE, alignment), m_mappableVirtualAddressStart, m_mappableVirtualAddressSizeInBytes, m_usedVirtualAddressSizeInBytes);
				SCE_SAMPLE_UTIL_ASSERT(va != nullptr);

				// allocate AMM command buffer
				void *pAmmCb = allocateLinear(kAmmCbSize, 16 * 1024, m_pDeviceMemory, m_deviceMemorySizeInBytes, m_usedDeviceMemorySizeInBytes);
				SCE_SAMPLE_UTIL_ASSERT(pAmmCb != nullptr);

				// allocate APR command buffer
				void *pAprCb = allocateLinear(kAprCbSize, 16 * 1024, m_pDeviceMemory, m_deviceMemorySizeInBytes, m_usedDeviceMemorySizeInBytes);
				SCE_SAMPLE_UTIL_ASSERT(pAprCb != nullptr);

				// allocate state
				Agc::Label	*pState = reinterpret_cast<Agc::Label *>(allocateLinear(sizeof(Agc::Label) * 2, Agc::Alignment::kLabel, m_pDeviceMemory, m_deviceMemorySizeInBytes, m_usedDeviceMemorySizeInBytes));
				SCE_SAMPLE_UTIL_ASSERT(pState != nullptr);

				info.m_loadStatus.m_ptr					= va;
				info.m_loadStatus.m_pLoadState			= pState;
				info.m_loadStatus.m_pLoadState->m_value	= (uint64_t)LoadState::kUnloaded;
				info.m_pMapState						= pState + 1;
				info.m_pMapState->m_value 				= (uint64_t)MapState::kUnmapped;
				info.m_fileId							= SCE_AMPR_APR_FILEID_INVALID;
				info.m_sizeInBytes						= sizeInBytes;
				info.m_type								= type;
				info.m_prot								= prot;
				info.m_pAmmCommandBuffer				= pAmmCb;
				info.m_pAprCommandBuffer				= pAprCb;
				strncpy(info.m_pName, tag, 256); info.m_pName[255] = 0;

				m_assets.insert({ tag, info });
				m_loadStates.insert(std::make_pair(va, info.m_loadStatus.m_pLoadState));
#ifdef _DEBUG
				if (Graphics::g_isResourceRegistrationInitialized) {
					for (const auto &resType : resourceTypes) {
						Agc::ResourceRegistration::ResourceHandle rh;
						ret = Agc::ResourceRegistration::registerResource(&rh, m_owner, va, sizeInBytes, tag, resType, 0);
						SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
						if (Memory::g_isMatInitialized && (ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG)) {
							sceMatAgcRegisterResource(rh, m_owner, va, sizeInBytes, tag, (uint32_t)resType, 0);
						}
					}
				}
				if (Memory::g_isMatInitialized) {
					sceMatAlloc(va, sizeInBytes, 0, Memory::getMatGroup(this));
					sceMatTagAllocation(va, tag);
					sceMatAlloc(pAmmCb, kAmmCbSize, 0, Memory::getMatGroup(this));
					sceMatTagAllocation(pAmmCb, (std::string(tag) + "_ammCb").c_str());
					sceMatAlloc(pAprCb, kAprCbSize, 0, Memory::getMatGroup(this));
					sceMatTagAllocation(pAprCb, (std::string(tag) + "_aprCb").c_str());
					sceMatAlloc(pState, sizeof(Agc::Label) * 2, 0, Memory::getMatGroup(this));
					sceMatTagAllocation(pState, (std::string(tag) + "_ammAprState").c_str());
				}
				Debug::Perf::tagBuffer(tag, va, sizeInBytes, 1);
#endif // _DEBUG
			}
		}

		// if 'unmapping', wait until finished
		Agc::Label *pMapState = info.m_pMapState;
		m_ammListener.waitIf({ pMapState, EventListener::Condition::kEqual, (uint64_t)MapState::kUnmapping, nullptr, 0ul });

		// check if 'unmapped'. if so, set to 'mapping'
		if (sceAtomicCompareAndSwap64((volatile int64_t *)&pMapState->m_value, (int64_t)MapState::kUnmapped, (int64_t)MapState::kMapping) == (int64_t)MapState::kUnmapped) {
			Ampr::AmmCommandBuffer ammCommandBuffer;
			ret = ammCommandBuffer.setBuffer(info.m_pAmmCommandBuffer, kAmmCbSize);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			ret = ammCommandBuffer.reset();
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

			ret = ammCommandBuffer.map((uintptr_t)info.m_loadStatus.m_ptr, ((info.m_sizeInBytes + SCE_KERNEL_PAGE_SIZE - 1) / SCE_KERNEL_PAGE_SIZE) * SCE_KERNEL_PAGE_SIZE, info.m_type, info.m_prot);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			ret = ammCommandBuffer.writeAddressOnCompletion(&pMapState->m_value, (uint64_t)MapState::kMapped);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

			ret = Ampr::Amm::submitCommandBuffer(&ammCommandBuffer, Ampr::Amm::Priority::kMid);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}

		return	info.m_loadStatus;
	}

	int	AsyncAssetLoader::load(const char	*tag, const char	*assetFilePath, off_t	offsetInBytes, size_t	sizeInBytes)
	{
		int ret = SCE_OK; (void)ret;

		const bool noload = (assetFilePath == nullptr);
		AssetInfo	info;
		info.m_loadStatus.m_ptr = nullptr;
		{
			Thread::ScopedLock criticalSection(this, Thread::LockableObjectAccessAttr::kWrite);

			SCE_SAMPLE_UTIL_ASSERT(m_assets.find(tag) != m_assets.end());
			if (m_assets.find(tag) == m_assets.end()) {
				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			if ((m_assets[tag].m_fileId == SCE_AMPR_APR_FILEID_INVALID && !noload) || sizeInBytes == 0ul) {
				// resolve file id and file size
				uint32_t	errorIdx;
				ret = sce::Ampr::Apr::resolveFilepathsToIdsAndFileSizes(&assetFilePath, 1, &m_assets[tag].m_fileId, &m_assets[tag].m_sizeInBytes, &errorIdx);
				if (ret == SCE_KERNEL_ERROR_EINVAL || ret == SCE_KERNEL_ERROR_ENAMETOOLONG || ret == SCE_KERNEL_ERROR_ENOENT || ret == SCE_KERNEL_ERROR_ENOPLAYGOENT || ret == SCE_KERNEL_ERROR_ENOBLK) {
					return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				}
				if (sizeInBytes == 0ul) {
					sizeInBytes = m_assets[tag].m_sizeInBytes;
				}
			}
			info = m_assets[tag];
		}
		SCE_SAMPLE_UTIL_ASSERT(offsetInBytes + sizeInBytes <= info.m_sizeInBytes);

		Agc::Label *pLoadState = info.m_loadStatus.m_pLoadState;
		// check if 'unloaded'. if so, set to 'loading'
		if (sceAtomicCompareAndSwap64((volatile int64_t *)&pLoadState->m_value, (int64_t)LoadState::kUnloaded, (int64_t)LoadState::kLoading) == (int64_t)LoadState::kUnloaded) {
			Ampr::AprCommandBuffer aprCommandBuffer;
			ret = aprCommandBuffer.setBuffer(info.m_pAprCommandBuffer, kAprCbSize);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			ret = aprCommandBuffer.reset();
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

			if (info.m_pMapState->m_value == (uint64_t)MapState::kMapping)
			{
				ret = aprCommandBuffer.waitOnAddress(&info.m_pMapState->m_value, (uint64_t)MapState::kMapped, Ampr::WaitCompare::kEqual, Ampr::WaitFlush::kDisable);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			}
			if (info.m_fileId != SCE_AMPR_APR_FILEID_INVALID)
			{
				ret = aprCommandBuffer.readFile(info.m_fileId, info.m_loadStatus.m_ptr, sizeInBytes, offsetInBytes);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			}
			ret = aprCommandBuffer.writeAddressOnCompletion(&pLoadState->m_value, (uint64_t)LoadState::kLoaded);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			ret = aprCommandBuffer.writeKernelEventQueueOnCompletion(m_aprListener.m_eventQueue, m_aprListener.m_eqId, 0ul);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

#ifndef APR_DEBUG
			ret = Ampr::Apr::submitCommandBuffer(&aprCommandBuffer, Ampr::Apr::Priority::kPriority3);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
#else
			SceAprResultBuffer result;
			SceAprSubmitId submitId;
			ret = Ampr::Apr::submitCommandBufferAndGetResult(&aprCommandBuffer, Ampr::Apr::Priority::kPriority3, &result, &submitId);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			ret = Ampr::Apr::waitCommandBufferCompletion(submitId);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK && result.result == SCE_OK);
#endif

		}

		return	SCE_OK;
	}

	int	AsyncAssetLoader::release(const char	*assetFilePath, Agc::Label	*pKickLabel)
	{
		int ret = SCE_OK; (void)ret;

		AssetInfo info;
		{
			Thread::ScopedLock criticalSection(this, Thread::LockableObjectAccessAttr::kRead); // lock to access container safely

			SCE_SAMPLE_UTIL_ASSERT(m_assets.find(assetFilePath) != m_assets.end());
			info = m_assets[assetFilePath];
		}

		Agc::Label *pLoadState = info.m_loadStatus.m_pLoadState;
		if (pLoadState->m_value == (uint64_t)LoadState::kUnloaded)
		{
			return SCE_OK;
		}

		// if 'loading', wait until finished.
		m_aprListener.waitIf({ pLoadState, EventListener::Condition::kEqual, (uint64_t)LoadState::kLoading, nullptr, 0ul });

		// check if 'loaded', if so set to 'unloaded'
		if (sceAtomicCompareAndSwap64((volatile int64_t *)&pLoadState->m_value, (int64_t)LoadState::kLoaded, (int64_t)LoadState::kUnloaded) == (int64_t)LoadState::kLoaded)
		{
			info.m_pMapState->m_value = (uint64_t)MapState::kUnmapping;
			Ampr::AmmCommandBuffer ammCommandBuffer;
			ret = ammCommandBuffer.setBuffer(info.m_pAmmCommandBuffer, kAmmCbSize);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			ret = ammCommandBuffer.reset();
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

			if (pKickLabel != nullptr)
			{
				ret = ammCommandBuffer.waitOnAddress(&pKickLabel->m_value, 0ul, Ampr::WaitCompare::kNotEqual, Ampr::WaitFlush::kDisable);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			}
			ret = ammCommandBuffer.unmap((uintptr_t)info.m_loadStatus.m_ptr, ((info.m_sizeInBytes + SCE_KERNEL_PAGE_SIZE - 1) / SCE_KERNEL_PAGE_SIZE) * SCE_KERNEL_PAGE_SIZE);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			ret = ammCommandBuffer.writeAddressOnCompletion(&info.m_pMapState->m_value, (uint64_t)MapState::kUnmapped);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			ret = ammCommandBuffer.writeKernelEventQueueOnCompletion(m_ammListener.m_eventQueue, m_ammListener.m_eqId, 0ul);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

			ret = Ampr::Amm::submitCommandBuffer(&ammCommandBuffer, Ampr::Amm::Priority::kMid);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
#ifdef _DEBUG
			if (Graphics::g_isResourceRegistrationInitialized)
			{
				std::vector<Agc::ResourceRegistration::ResourceHandle> resourceHandles;
				Agc::ResourceRegistration::findResources(info.m_loadStatus.m_ptr, 16, [](Agc::ResourceRegistration::ResourceHandle resourceHandle, Agc::ResourceRegistration::OwnerHandle ownerHandle, uint64_t callbackdata)
				{
					std::vector<Agc::ResourceRegistration::ResourceHandle> *pResourceHandles = reinterpret_cast<std::vector<Agc::ResourceRegistration::ResourceHandle> *>(callbackdata);
					pResourceHandles->push_back(resourceHandle);
					return SCE_OK;
				}, reinterpret_cast<uintptr_t>(&resourceHandles));
				for (auto resourceHandle : resourceHandles)
				{
					ret = Agc::ResourceRegistration::unregisterResource(resourceHandle);
					SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
				}
			}
			if (Memory::g_isMatInitialized)
			{
				sceMatFree(info.m_loadStatus.m_ptr);
			}
			if (info.m_pName[0] != 0)
			{
				Debug::Perf::unTagBuffer(info.m_loadStatus.m_ptr);
			}
#endif // _DEBUG
		}

		return SCE_OK;
	}
} } } // namespace sce::SampleUtil::Helper
#endif // _SCE_TARGET_OS_PROSPERO