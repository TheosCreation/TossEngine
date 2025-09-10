/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO

#include <string>
#include <kernel.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/thread/thread.h>

namespace sce { namespace SampleUtil { namespace Thread {

	Cond::Cond(const char	*name, const CondOption	*pOption)
	{
		initialize(name, pOption);
	}

	Cond::~Cond()
	{
		finalize();
	}

	int Cond::initialize(const char	*name, const CondOption	*pOption)
	{
		int ret = scePthreadCondInit(&m_cond, nullptr, name);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		std::string nameMutex(name);
		// ミューテックス名の長さをシステムサポートに収めるために
		// Cond::kNameLenMaxはMutex::kNameLenMax-2にしてある
		nameMutex += "Mx";
		nameMutex = nameMutex.substr(0, kNameLenMax-1);
		ret = scePthreadMutexInit(&m_mutex, nullptr, nameMutex.c_str());
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		return ret;
	}

	int Cond::finalize()
	{
		int ret = scePthreadCondDestroy(&m_cond);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		ret = scePthreadMutexDestroy(&m_mutex);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		return ret;
	}

	int Cond::broadcast()
	{
		int ret = scePthreadCondBroadcast(&m_cond);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		return ret;
	}

	int Cond::signal()
	{
		int ret = scePthreadCondSignal(&m_cond);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		return ret;
	}

	int Cond::signalTo(Thread	*thread)
	{
		int ret = scePthreadCondSignalto(&m_cond, thread->getId());
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		return ret;
	}

	int Cond::wait(Useconds	*pTimeout)
	{
		int ret = SCE_OK;

		if (pTimeout) {
			ret = scePthreadCondTimedwait(&m_cond, &m_mutex, *pTimeout);
			if (ret == SCE_KERNEL_ERROR_ETIMEDOUT) {
				return SCE_SAMPLE_UTIL_ERROR_TIMEDOUT;
			}
		}
		else {
			ret = scePthreadCondWait(&m_cond, &m_mutex);
		}
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		return ret;
	}

	int Cond::lock()
	{
		return scePthreadMutexLock(&m_mutex);
	}

	int Cond::unlock()
	{
		return scePthreadMutexUnlock(&m_mutex);
	}
}}}

#endif // _SCE_TARGET_OS_PROSPERO
