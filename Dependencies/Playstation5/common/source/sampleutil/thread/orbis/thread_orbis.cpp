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

	ThreadOption Thread::m_defaultOption;

	Thread::Thread()
		:
		m_started(false),
		m_alive(false),
		m_pFuncObj(nullptr)
	{}
	Thread::Thread(const char * name, int32_t priority, uint32_t stackSize, const ThreadOption * pOption)
		:
		m_started(false),
		m_alive(false),
		m_pFuncObj(nullptr)
	{
		initialize(name, priority, stackSize, pOption);
	}

	Thread::~Thread()
	{
		finalize();
	}

	int Thread::initialize(const char * name, int32_t priority, uint32_t stackSize, const ThreadOption * pOption)
	{
		// reserve thread's name
		m_threadName = (name != nullptr) ? name : "";

		// construct thread's attributes
		SceKernelSchedParam sched_param;
		scePthreadAttrInit(&m_attr);
		scePthreadAttrSetstacksize(&m_attr, stackSize);
		scePthreadAttrSetschedpolicy(&m_attr, SCE_KERNEL_SCHED_FIFO);
		sched_param.sched_priority = priority;
		scePthreadAttrSetschedparam(&m_attr, &sched_param);
		scePthreadAttrSetinheritsched(&m_attr, SCE_PTHREAD_EXPLICIT_SCHED);
		return SCE_OK;
	}

	int Thread::finalize()
	{
		return SCE_OK;
	}


	int Thread::start(ThreadFunction* m_pFuncObj)
	{
		if (m_started)
		{
			return SCE_SAMPLE_UTIL_ERROR_THREAD_ALREADY_STARTED;
		}

		SCE_SAMPLE_UTIL_ASSERT(m_pFuncObj);
		this->m_pFuncObj = m_pFuncObj;

		int ret;
		ret = scePthreadCreate(&m_thID, &m_attr, threadFunc, this, m_threadName.substr(0, kNameLenMax).c_str());
		scePthreadAttrDestroy(&m_attr);
		if (ret != SCE_OK) {
			// error codes differ on each platform...
			return ret;
		}
		m_started = true;
		m_alive = true;
		return 0;
	}



	int Thread::join()
	{
		void *value_ptr;
		return scePthreadJoin(m_thID, &value_ptr);
	}

	bool Thread::isAlive()
	{
		return m_alive;
	}

	ThreadID Thread::getId()
	{
		return m_thID;
	}

	int Thread::changePriority(int32_t priority)
	{
		return scePthreadSetprio(m_thID, priority);
	}

	void Thread::setDefaultOption(const ThreadOption& option)
	{
		m_defaultOption = option;
	}

	/**
	* スレッドの実行ハンドラ
	*/
	void *Thread::threadFunc(void *args)
	{
		Thread* This = (Thread*)args;

		// 受け取った処理実行オブジェクトを実行する
		This->m_pFuncObj->run();

		This->m_alive = false;
		scePthreadExit(nullptr);
		return nullptr;
	}

	int Thread::sleep(int milisec)
	{
		SceKernelTimespec t;
		t.tv_sec = milisec / 1000;
		t.tv_nsec = (milisec % 1000) * (1000 * 1000);
		return sceKernelNanosleep(&t, nullptr);
	}
}}}

#endif // _SCE_TARGET_OS_ORBIS
