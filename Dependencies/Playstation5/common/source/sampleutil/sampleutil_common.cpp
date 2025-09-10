/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2022 Sony Interactive Entertainment Inc. 
 * 
 */


#include <cstdio>
#include <cstdarg>
#include <kernel.h>
#include <sampleutil/sampleutil_common.h>

#define _TEXT(a) a

static void vdebugPrintfln(char * format, va_list ap)
{
	char buf[1024];
	vsnprintf(buf, 1024, format, ap);
	buf[1023] = 0;
	printf("%s", buf);
	fflush(stdout);
}

void sce::SampleUtil::Internal::Assert(const char* file, int line, bool test, const char* format, ...)
{
	if(!test){
		char buf[1024];
		snprintf(buf, 1024, _TEXT("Sample Util ASSERT, FILE:%s, LINE:%d, %s\n"), file, line, format);
		buf[1023] = 0;
		va_list argList;
		va_start(argList, format);
		vdebugPrintfln(buf, argList);
		va_end(argList);
#if SCE_SAMPLE_UTIL_RELEASE
		while(1){
			sceKernelUsleep(1000*1000*100);
		}
#else /* SCE_SAMPLE_UTIL_RELEASE */
			abort();
#endif /* SCE_SAMPLE_UTIL_RELEASE */
	}

}


void sce::SampleUtil::Internal::AssertEqual32(const char* file, int line, int32_t value, int32_t expected, const char* format, ...)
{
	if(value != expected){
		char buf[1024];
		snprintf(buf, 1024, _TEXT("Sample Util ASSERT, FILE:%s, LINE:%d, %s:  Actual value is %#x, expected value is %#x\n"), file, line, format, value, expected);
		buf[1023] = 0;
		va_list argList;
		va_start(argList, format);
		vdebugPrintfln(buf, argList);
		va_end(argList);
#if SCE_SAMPLE_UTIL_RELEASE
		while(1){
			sceKernelUsleep(1000*1000*100);
		}
#else /* SCE_SAMPLE_UTIL_RELEASE */
		abort();
#endif /* SCE_SAMPLE_UTIL_RELEASE */
	}

}
