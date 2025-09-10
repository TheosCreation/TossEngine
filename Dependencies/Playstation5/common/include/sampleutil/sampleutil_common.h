/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */



#pragma once


#include <scebase_common.h>


#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>

namespace sce
{
	/*!
	 * @~English
	 * @brief Sample utility 
	 * @details These are the sample utility definitions. 
	 * @~Japanese
	 * @brief サンプルユーティリティ 
	 * @details サンプルユーティリティの定義です。 
	 */
	namespace SampleUtil
	{
#if !defined(DOXYGEN_IGNORE)
		namespace Internal
		{
			void Assert(const char* file, int line, bool test, const char* format, ...);
			void AssertEqual32(const char* file, int line, int32_t value, int32_t expected, const char* format, ...);

		}
#endif
	}
}

#define SCE_SAMPLE_UTIL_SAFE_DELETE(ptr)	if(ptr) { delete ptr; ptr = nullptr; }

#ifndef SCE_SAMPLE_UTIL_ASSERTS_ENABLED
#define SCE_SAMPLE_UTIL_ASSERTS_ENABLED 0
#endif	/* SCE_SAMPLE_UTIL_ASSERTS_ENABLED */

#if SCE_SAMPLE_UTIL_ASSERTS_ENABLED

#define SCE_SAMPLE_UTIL_ASSERT(test)			        sce::SampleUtil::Internal::Assert(__FILE__, __LINE__, (bool)(test), "Assertion failed: %s\n",#test)
#define SCE_SAMPLE_UTIL_ASSERT_MSG(test, msg, ...)	    sce::SampleUtil::Internal::Assert(__FILE__, __LINE__, (bool)(test), msg, ##__VA_ARGS__)
#define SCE_SAMPLE_UTIL_ASSERT_EQUAL(value, expected)	sce::SampleUtil::Internal::AssertEqual32(__FILE__, __LINE__, value, expected, "Assertion failed, values not equal. actual value=%#x, expected value=%#x\n", value, expected)

#else

#define SCE_SAMPLE_UTIL_ASSERT(test)			        
#define SCE_SAMPLE_UTIL_ASSERT_MSG(test, msg, ...)	    
#define SCE_SAMPLE_UTIL_ASSERT_EQUAL(value, expected)	

#endif

#ifndef SCE_SAMPLE_UTIL_RELEASE

#define SCE_SAMPLE_UTIL_RELEASE 0

#endif

