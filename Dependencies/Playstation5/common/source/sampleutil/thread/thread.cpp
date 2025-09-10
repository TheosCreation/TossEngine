/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc.
 * 
 */

#include <sampleutil/sampleutil_error.h>

#include <sampleutil/thread/thread.h>

namespace sce { namespace SampleUtil { namespace Thread {

	int ThreadFunction::sleep(int milisec)
	{
		return Thread::sleep(milisec);
	}

	int LockableObject::lock(LockableObjectAccessAttr attr)
	{
		int ret = SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		if (attr == LockableObjectAccessAttr::kWrite) {
			ret = m_rwlock->wrlock();
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}
		else if ((uint32_t)attr == ((uint32_t)LockableObjectAccessAttr::kRead | (uint32_t)LockableObjectAccessAttr::kWrite)) {
			ret = m_rwlock->wrlock();
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}
		else if (attr == LockableObjectAccessAttr::kRead) {
			ret = m_rwlock->rdlock();
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}
		else {
			SCE_SAMPLE_UTIL_ASSERT(0);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		if (ret != SCE_OK) {
			ret = SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}
		return ret;
	}
}}}
