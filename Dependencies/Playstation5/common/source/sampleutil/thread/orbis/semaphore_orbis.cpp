/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */
#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS

#include <sampleutil/sampleutil_error.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/event.h>
#include <kernel.h>
#include <user_service.h>
#include <sampleutil/thread/thread.h>

namespace sce { namespace SampleUtil { namespace Thread {

	Semaphore::Semaphore(const char * name, int initCount, int maxCount, const SemaphoreOption * pOption)
	{
		initialize(name, initCount, maxCount, pOption);
	}

	Semaphore::~Semaphore()
	{
		finalize();
	}

	int Semaphore::initialize(const char * name, int initCount, int maxCount, const SemaphoreOption * pOption)
	{
		std::string nameSema(name);
		nameSema = nameSema.substr(0, kNameLenMax - 1);
		int ret = sceKernelCreateSema(&m_sema, nameSema.c_str(), SCE_KERNEL_SEMA_ATTR_TH_PRIO, initCount, maxCount, nullptr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		return ret;
	}

	int Semaphore::finalize()
	{
		int ret = sceKernelDeleteSema(m_sema);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		return ret;
	}
	int Semaphore::signal(int count)
	{
		int ret = sceKernelSignalSema(m_sema, count);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		return ret;
	}
	int Semaphore::poll(int count)
	{
		int ret = sceKernelPollSema(m_sema, count);
		if (ret == SCE_KERNEL_ERROR_EBUSY) {
			return SCE_SAMPLE_UTIL_ERROR_BUSY;
		}
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		return ret;
	}
	int Semaphore::wait(int count, Useconds * pTimeout)
	{
		int ret = sceKernelWaitSema(m_sema, count, pTimeout);
		if (ret == SCE_KERNEL_ERROR_ETIMEDOUT) {
			return SCE_SAMPLE_UTIL_ERROR_TIMEDOUT;
		}
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		return 0;
	}
}}}

#endif // _SCE_TARGET_OS_ORBIS
