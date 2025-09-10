/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO

#include <string>
#include <kernel.h>
#include <sampleutil/sampleutil_common.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/thread/thread.h>

namespace sce { namespace SampleUtil { namespace Thread {

	Mutex::Mutex(const char	*name, const MutexOption	*pOption)
	{
		initialize(name, pOption);
	}

	Mutex::~Mutex()
	{
		finalize();
	}

	int Mutex::initialize(const char	*name, const MutexOption	*pOption)
	{
		int ret;
		ScePthreadMutexattr attr;
		scePthreadMutexattrInit(&attr);

		if (pOption && (pOption->m_recursive == MutexRecursiveAttr::kRecursive)) {
			scePthreadMutexattrSettype(&attr, SCE_PTHREAD_MUTEX_RECURSIVE);
		}

		std::string nameMutex(name);
		nameMutex = nameMutex.substr(0, kNameLenMax - 1);

		ret = scePthreadMutexInit(&m_mutex, &attr, nameMutex.c_str());
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = scePthreadMutexattrDestroy(&attr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		return ret;
	}

	int Mutex::finalize()
	{
		int ret;
		ret = scePthreadMutexDestroy(&m_mutex);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		return ret;
	}

	int Mutex::lock(MutexLockAttr attr, Useconds *pTimeout)
	{
		int ret;
		if (attr == MutexLockAttr::kNonBlocking) {
			ret = scePthreadMutexTrylock(&m_mutex);
		}
		else {
			if (pTimeout) {
				ret = scePthreadMutexTimedlock(&m_mutex, *pTimeout);
				if (ret == SCE_KERNEL_ERROR_ETIMEDOUT) {
					return SCE_SAMPLE_UTIL_ERROR_TIMEDOUT;
				}
			}
			else {
				ret = scePthreadMutexLock(&m_mutex);
			}
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK);
		}
		return ret;
	}

	int Mutex::unlock()
	{
		int ret;
		ret = scePthreadMutexUnlock(&m_mutex);
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK);
		return ret;
	}
}}}

#endif // _SCE_TARGET_OS_PROSPERO
