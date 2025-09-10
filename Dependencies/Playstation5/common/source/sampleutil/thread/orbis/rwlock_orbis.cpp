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

	Rwlock::Rwlock(const char * name, const RwlockOption * pOption)
	{
		initialize(name, pOption);
	}

	Rwlock::~Rwlock()
	{
		finalize();
	}

	int Rwlock::initialize(const char * name, const RwlockOption * pOption)
	{
		std::string nameRwlock(name);
		nameRwlock = nameRwlock.substr(0, Rwlock::kNameLenMax - 1);
		int ret = scePthreadRwlockInit(&m_rwlock, nullptr, nameRwlock.c_str());
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			ret = SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}
		return ret;
	}

	int Rwlock::finalize()
	{
		int ret = scePthreadRwlockDestroy(&m_rwlock);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			ret = SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}
		return ret;
	}

	int Rwlock::rdlock()
	{
		int ret = SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;

		ret = scePthreadRwlockRdlock(&m_rwlock);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		if (ret != SCE_OK) {
			ret = SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}
		return ret;
	}

	int Rwlock::wrlock()
	{
		int ret = SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		ret = scePthreadRwlockWrlock(&m_rwlock);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		if (ret != SCE_OK) {
			ret = SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}
		return ret;
	}


	int Rwlock::unlock()
	{
		int ret = scePthreadRwlockUnlock(&m_rwlock);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			ret = SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}
		return ret;
	}
}}}

#endif // _SCE_TARGET_OS_ORBIS
