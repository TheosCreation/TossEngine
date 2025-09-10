/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc. 
 * 
 */

#include <scebase_common.h>

#if _SCE_TARGET_OS_PROSPERO || _SCE_TARGET_OS_ORBIS

#include <net.h>
#include <libhttp2.h>
#include <libsysmodule.h>
#include <sampleutil/sampleutil_common.h>
#include <sampleutil/Network/initialize.h>
#include <sampleutil/sampleutil_error.h>
#include <np.h>
#include <np\np_webapi2.h>

#if __has_feature(cxx_rtti)
#include <json2_rtti.h>
#else
#include <json2.h>
#endif

#pragma comment(lib,"libSceNpWebApi2_stub_weak.a")
#pragma comment(lib,"libSceJson2_stub_weak.a")

namespace
{
	// for JSON parser

	static sce::Json::Initializer *s_initializer;

#if __has_feature(cxx_rtti)
	static void* Malloc(size_t size, void *user_data)
	{
		void *p = malloc(size);
		return p;
	}
	static void Free(void *ptr, void *user_data)
	{
		free(ptr);
	}
	static void	notifyError(int32_t error, size_t size, void* userData)
	{
		printf("Json Error: 0x%x", error);
	}

	static void initJson()
	{
		sce::Json::AllocParamRtti allocParam(Malloc, Free, notifyError);

		sce::Json::InitParameterRtti2 initparam;
		initparam.setAllocatorRtti(&allocParam, 0);
		initparam.setFileBufferSize(512);
		initparam.setSpecialFloatFormatType(sce::Json::kSpecialFloatFormatTypeSymbol);

		sce::Json::Initializer initializer;
		void* p = malloc(sizeof(sce::Json::Initializer));
		SCE_SAMPLE_UTIL_ASSERT(p);
		s_initializer = new (p) sce::Json::Initializer();
		s_initializer->initialize(&initparam);
	}

	static int termJson()
	{
		s_initializer->terminate();

		s_initializer->~Initializer();
		free(s_initializer);
		s_initializer = nullptr;

		int ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_JSON2);
		if (ret < SCE_OK)
		{
			printf("sceSysmoduleUnloadModule(SCE_SYSMODULE_JSON2) : %x\n", ret);
		}
		return ret;
	}

#else
	class MyAllocator : public sce::Json::MemAllocator
	{
	public:
		MyAllocator() {};
		~MyAllocator() {};

		virtual void *allocate(size_t size, void *user_data) {
			void *p = malloc(size);
			return p;
		}
		virtual void deallocate(void *ptr, void *user_data) {
			free(ptr);
		}
	};
	static MyAllocator *s_allc;

	static void initJson()
	{
		void* p = malloc(sizeof(MyAllocator));
		SCE_SAMPLE_UTIL_ASSERT(p);
		s_allc = new (p) MyAllocator();
		sce::Json::InitParameter2 initparam;
		initparam.setAllocator(s_allc, nullptr);
		initparam.setFileBufferSize(128 * 1024);
		initparam.setSpecialFloatFormatType(sce::Json::kSpecialFloatFormatTypeSymbol);

		p = malloc(sizeof(sce::Json::Initializer));
		SCE_SAMPLE_UTIL_ASSERT(p);
		s_initializer = new (p) sce::Json::Initializer();
		s_initializer->initialize(&initparam);
	}

	static int termJson()
	{
		s_initializer->terminate();

		s_initializer->~Initializer();
		free(s_initializer);
		s_initializer = nullptr;

		s_allc->~MyAllocator();
		free(s_allc);
		s_allc = nullptr;

		int ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_JSON2);
		if (ret < SCE_OK)
		{
			printf("sceSysmoduleUnloadModule(SCE_SYSMODULE_JSON2) : %x\n", ret);
		}
		return ret;
	}
#endif
}

namespace sce
{
	namespace SampleUtil
	{
		namespace Network
		{
			class NpContext
			{
			public:

				uint32_t	m_initFlag;
				int			m_libnetMemId;
				int			m_libsslContextId;
				int			m_libhttpContextId;
				int			m_webapiLibContextId;

				NpContext():m_initFlag(0), m_libnetMemId(0), m_libsslContextId(0), m_libhttpContextId(0), m_webapiLibContextId(0)
				{
				}
				~NpContext()
				{
				}

				int initializeSslAndHttp(NpInitializeParam &param);
				int	initialize(SceNpTitleId npTitleId, SceNpTitleSecret npTitleSecret, const char *name, NpInitializeParam &param);
				int	update();
				int finalize();
			};
		}
	}
}

static sce::SampleUtil::Network::NpContext s_npContext;

static int initializeNet(const char *name, size_t heapSize)
{
	int ret = 0;

	/* libnet */
	/*ret = sceNetInit();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return SCE_SAMPLE_UTIL_ERROR_FATAL;
	}*/
	ret = sceNetPoolCreate(name, heapSize, 0);
	SCE_SAMPLE_UTIL_ASSERT(ret >= 0);
	if (ret < 0) {
		// sceNetTerm();
		if (ret == SCE_NET_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		else if (ret == SCE_NET_ERROR_ENFILE) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		else if (ret == SCE_NET_ERROR_ENOALLOCMEM) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		else if (ret == SCE_NET_ERROR_ENAMETOOLONG) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		return ret;
	}

	return ret;
}

static int finalizeNet(int memId)
{
	int ret = SCE_OK;

	/* libnet */
	ret = sceNetPoolDestroy(memId);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret < 0) {
		if (ret == SCE_NET_ERROR_EBADF) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		else if (ret == SCE_NET_ERROR_ENOTEMPTY) return SCE_SAMPLE_UTIL_ERROR_BUSY;
		return ret;
	}
	/*ret = sceNetTerm();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret < 0) {
		return SCE_SAMPLE_UTIL_ERROR_FATAL;
	}*/

	return ret;
}

int sce::SampleUtil::Network::NpContext::initializeSslAndHttp(NpInitializeParam &param)
{
	int ret = 0;

	// ssl
	if (param.m_initFlag & kSSL)
	{
		SCE_SAMPLE_UTIL_ASSERT(param.m_sslHeapSize > 0);
		if (param.m_sslHeapSize == 0)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		ret = sceSslInit(param.m_sslHeapSize);
		SCE_SAMPLE_UTIL_ASSERT(ret > 0);
		if (ret <= 0) {
			if (ret == SCE_SSL_ERROR_OUT_OF_MEMORY) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			else if (ret == SCE_SSL_ERROR_OUT_OF_SIZE) return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
			return ret;
		}
		m_libsslContextId = ret;
	}

	// http
	if (param.m_initFlag & kHTTP)
	{
		SCE_SAMPLE_UTIL_ASSERT(param.m_initFlag & kSSL);	// need m_libsslContextId
		if (!(param.m_initFlag & kSSL))
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		SCE_SAMPLE_UTIL_ASSERT(param.m_httpHeapSize > 0);
		if (param.m_httpHeapSize == 0)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		SceSslMemoryPoolStats sslMemoryStats;
		ret = sceSslGetMemoryPoolStats(m_libsslContextId, &sslMemoryStats);

		ret = sceHttp2Init(m_libnetMemId, m_libsslContextId, param.m_httpHeapSize, 3);
		SCE_SAMPLE_UTIL_ASSERT(ret > 0);
		if (ret <= 0) {
			if (ret == SCE_HTTP2_ERROR_OUT_OF_MEMORY) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			else if (ret == SCE_HTTP2_ERROR_INSUFFICIENT_STACKSIZE) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			return ret;
		}
		m_libhttpContextId = ret;
	}

	return SCE_OK;
}

int	sce::SampleUtil::Network::NpContext::initialize(SceNpTitleId npTitleId, SceNpTitleSecret npTitleSecret, const char *name, NpInitializeParam &param)
{
	int ret = 0;

	// Keep flag
	m_initFlag = param.m_initFlag;

	ret = initializeNet(name, param.m_netHeapSize);
	SCE_SAMPLE_UTIL_ASSERT(ret >= 0);
	if (ret < 0) {
		return ret;
	}
	m_libnetMemId = ret;

	// Net only
	if (param.m_initFlag & kSceNetOnly) {
		// Should be only kSceNetOnly flag
		uint32_t flag = param.m_initFlag & ~(kSceNetOnly);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(flag, 0);
		if (flag != 0)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		return SCE_OK;
	}

	ret = sceNpSetNpTitleId(&npTitleId, &npTitleSecret);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		finalizeNet(m_libnetMemId);
		return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}

	// SSL, Http
	ret = initializeSslAndHttp(param);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return ret;
	}

	// NP WebApi
	if (param.m_initFlag & kWebApi)
	{
		ret = sceNpWebApi2Initialize(m_libhttpContextId, param.m_webapiHeapSize);
		if (ret <= SCE_OK) {
			if (ret == SCE_NP_WEBAPI2_ERROR_OUT_OF_MEMORY) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			if (ret == SCE_NP_WEBAPI2_ERROR_INVALID_ARGUMENT) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			if (ret == SCE_NP_WEBAPI2_ERROR_NOT_SIGNED_IN) return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		m_webapiLibContextId = ret;
	}

	/*E JSON parser */
	if (param.m_initFlag & kJSON)
	{
		ret = sceSysmoduleLoadModule(SCE_SYSMODULE_JSON2);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			if (ret == SCE_KERNEL_ERROR_ENOMEM) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		initJson();
	}

	return SCE_OK;
}

int	sce::SampleUtil::Network::NpContext::update()
{
	return SCE_OK;
}

int	sce::SampleUtil::Network::NpContext::finalize()
{
	int ret = SCE_OK;

	// json parser
	if (m_initFlag & kJSON)
	{
		ret = termJson();
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
	}

	// NP WebApi2
	if (m_initFlag & kWebApi)
	{
		ret = sceNpWebApi2Terminate(m_webapiLibContextId);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
	}

	// libhttp
	if (m_libhttpContextId > 0)
	{
		ret = sceHttp2Term(m_libhttpContextId);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		m_libhttpContextId = 0;
	}

	// libssl
	if (m_libsslContextId > 0)
	{
		ret = sceSslTerm(m_libsslContextId);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		m_libsslContextId = 0;
	}

	// libnet
	if (m_libnetMemId > 0)
	{
		/* libnet */
		ret = sceNetPoolDestroy(m_libnetMemId);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			if (ret == SCE_NET_ERROR_ENOTEMPTY) return SCE_SAMPLE_UTIL_ERROR_BUSY;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		/*ret = sceNetTerm();
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}*/
		m_libnetMemId = 0;
	}

	return ret;
}

static uint8_t hexCharToUint(char ch)
{
	uint8_t val = 0;

	if (isdigit(ch)) {
		val = (ch - '0');
	}
	else if (isupper(ch)) {
		val = (ch - 'A' + 10);
	}
	else {
		val = (ch - 'a' + 10);
	}

	return val;
}

static void hexStrToBin(const char *pHexStr, uint8_t *pBinBuf, size_t binBufSize)
{
	uint8_t val = 0;
	int hexStrLen = strlen(pHexStr);

	int binOffset = 0;
	for (int i = 0; i < hexStrLen; i++) {
		val |= hexCharToUint(*(pHexStr + i));
		if (i % 2 == 0) {
			val <<= 4;
		}
		else {
			if (pBinBuf != nullptr && binOffset < binBufSize) {
				memcpy(pBinBuf + binOffset, &val, 1);
				val = 0;
			}
			binOffset++;
		}
	}

	if (val != 0 && pBinBuf != nullptr && binOffset < binBufSize) {
		memcpy(pBinBuf + binOffset, &val, 1);
	}
}

int sce::SampleUtil::Network::initializeNp(const char *npTitleIdStr, const char *npTitleSecretHexStr, const char *name, sce::SampleUtil::Network::NpInitializeParam &param)
{
	// Initialize SceNet only
	if (param.m_initFlag & kSceNetOnly)
	{
		SCE_SAMPLE_UTIL_ASSERT(name != nullptr);
		if (name == nullptr)
		{
			return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
		}
		// Should be only kSceNetOnly flag
		uint32_t flag = param.m_initFlag & ~(kSceNetOnly);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(flag, 0);
		if (flag != 0)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		// NpTitle data is not used when initialize SceNetOnly
		SceNpTitleId npTitleId;
		SceNpTitleSecret npTitleSecret;
		memset(&npTitleId, 0, sizeof(npTitleId));
		memset(&npTitleSecret, 0, sizeof(npTitleSecret));

		return s_npContext.initialize(npTitleId, npTitleSecret, name, param);
	}

	SCE_SAMPLE_UTIL_ASSERT(npTitleIdStr != nullptr && npTitleSecretHexStr != nullptr && name != nullptr);
	if (npTitleIdStr == nullptr || npTitleSecretHexStr == nullptr || name == nullptr)
	{
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}

	SceNpTitleId npTitleId;
	SceNpTitleSecret npTitleSecret;

	memset(&npTitleId, 0, sizeof(npTitleId));
	strncpy(npTitleId.id, npTitleIdStr, strlen(npTitleIdStr));

	memset(&npTitleSecret, 0, sizeof(npTitleSecret));
	hexStrToBin(npTitleSecretHexStr, npTitleSecret.data, sizeof(npTitleSecret.data));

	return s_npContext.initialize(npTitleId, npTitleSecret, name, param);
}

int sce::SampleUtil::Network::updateNp()
{
	return s_npContext.update();
}

int sce::SampleUtil::Network::finalizeNp()
{
	return s_npContext.finalize();
}

int sce::SampleUtil::Network::getSslContextId()
{
	if (s_npContext.m_libsslContextId > 0) return s_npContext.m_libsslContextId;
	return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
}

int sce::SampleUtil::Network::getHttpContextId()
{
	if (s_npContext.m_libhttpContextId > 0) return s_npContext.m_libhttpContextId;
	return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
}

int sce::SampleUtil::Network::getWebApiLibContextId()
{
	if (s_npContext.m_webapiLibContextId > 0) return s_npContext.m_webapiLibContextId;
	return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
}

#endif /*_SCE_TARGET_OS_PROSPERO*/
