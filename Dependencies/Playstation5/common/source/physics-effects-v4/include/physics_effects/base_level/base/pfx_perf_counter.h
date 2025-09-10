/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2022 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_PERF_COUNTER_H
#define _SCE_PFX_PERF_COUNTER_H

#include "pfx_common.h"

//J パフォーマンス測定する場合はSCE_PFX_USE_PERFCOUNTERを定義
//J ブックマークを使用する場合はSCE_PFX_USE_BOOKMARKを定義

//E Define SCE_PFX_USE_PERFCOUNTER to check performance
//E Define SCE_PFX_USE_BOOKMARK to use bookmark

#if defined(__ORBIS__) || defined(__PROSPERO__)
	#include <kernel.h>
	#include <perf.h>
#endif

#define SCE_PFX_MAX_PERF_STR	32
#define SCE_PFX_MAX_PERF_COUNT	20

namespace sce {
namespace pfxv4 {
#ifdef SCE_PFX_USE_PERFCOUNTER

class SCE_PFX_API PfxPerfCounter
{
private:
	int   m_count,m_strCount;
	char  m_str[SCE_PFX_MAX_PERF_COUNT][SCE_PFX_MAX_PERF_STR];
	float freq;

#ifdef _WIN32
	LONGLONG  m_cnt[SCE_PFX_MAX_PERF_COUNT*2];
#elif defined(__ORBIS__) || defined(__PROSPERO__)
	uint64_t m_cnt[SCE_PFX_MAX_PERF_COUNT*2];
#endif

	void count(int i)
	{
#ifdef _WIN32
		QueryPerformanceCounter( (LARGE_INTEGER *)&m_cnt[i] );
#elif defined(__ORBIS__) || defined(__PROSPERO__)
		m_cnt[i] = sceKernelGetProcessTimeCounter();
#endif
	}

public:
	PfxPerfCounter()
	{
#ifdef _WIN32
		LARGE_INTEGER sPerfCountFreq;
		QueryPerformanceFrequency(&sPerfCountFreq);
		freq = (float)sPerfCountFreq.QuadPart;
#elif defined(__ORBIS__) || defined(__PROSPERO__)
		freq = (float)sceKernelGetTscFrequency();
#endif
		resetCount();
	}

	~PfxPerfCounter()
	{
		//printCount();
	}

	void countBegin(const char *name)
	{
		SCE_PFX_ASSERT(m_strCount < SCE_PFX_MAX_PERF_COUNT);
		strncpy(m_str[m_strCount],name,SCE_PFX_MAX_PERF_STR-1);
		m_str[m_strCount][SCE_PFX_MAX_PERF_STR-1] = 0x00;
		m_strCount++;
		count(m_count++);
	}
	
	void countEnd()
	{
		count(m_count++);
	}

	void resetCount()
	{
		m_strCount = 0;
		m_count = 0;
	}

	float getCountTime(int i)
	{
		return (float)(m_cnt[i+1]-m_cnt[i]) / freq * 1000.0f;
	}

	void printCount()
	{
		if(m_count%2 != 0) countEnd();
		SCE_PFX_PRINTF("*** PfxPerfCounter results ***\n");
		float total = 0.0f;
		for(int i=0;i+1<m_count;i+=2) {
			total += getCountTime(i);
		}
		for(int i=0;i+1<m_count;i+=2) {
			SCE_PFX_PRINTF(" -- %s %fms(%.2f%%)\n",m_str[i>>1],getCountTime(i),getCountTime(i)/total*100.0f);
		}
		SCE_PFX_PRINTF(" -- Total %fms\n",total);
	}
};

#else /* SCE_PFX_USE_PERFCOUNTER */

class SCE_PFX_API PfxPerfCounter
{
public:
	PfxPerfCounter() {}
	~PfxPerfCounter() {}
	void countBegin(const char *name) {(void) name;}
	void countEnd() {}
	void resetCount() {}
	float getCountTime(int i) {(void)i;return 0.0f;}
	void printCount() {}
};

#endif /* SCE_PFX_USE_PERFCOUNTER */

} //namespace pfxv4
} //namespace sce

///////////////////////////////////////////////////////////////////////////////

#include "../../util/pfx_static_array.h"

#define SCE_PFX_PERF_NONE 0xffffffff

namespace sce {
namespace pfxv4 {

#ifdef _WIN32
typedef LONGLONG PfxPerf;
#elif defined(__ORBIS__) || defined(__PROSPERO__)// [SMS_CHANGE] add Prospero support
typedef uint64_t PfxPerf;
#endif

template <size_t SIZE = 128>
class PfxPerfAnalyzer
{
private:
	PfxPerfAnalyzer();
	
	PfxUInt32 m_root;
	
	struct PerfInfo {
		char  title[SCE_PFX_MAX_PERF_STR];
		PfxPerf count;
		PfxFloat ratio;
		PfxUInt32 depth;
		PfxUInt32 parent,child,next;
		SCE_PFX_PADDING4

		PerfInfo() : count(0),ratio(0.0f),depth(0),parent(SCE_PFX_PERF_NONE),child(SCE_PFX_PERF_NONE),next(SCE_PFX_PERF_NONE)
		{
			title[0]='\0';
		}
	};
	
	SCE_PFX_PADDING12
	PfxStaticArray<PerfInfo,SIZE> m_perfPool;
	PfxStaticStack<PfxUInt32> m_perfStack;
	
	inline PfxPerf getPerfCount();
	
	inline void printResults(PfxUInt32 pid);
	
	inline void printResultsCsv(PfxUInt32 pid);

	inline void printResultsTag(PfxUInt32 pid);
	
	inline void gatherResults(PfxUInt32 pid,PfxPerf total);
	
public:
	static PfxPerfAnalyzer& getInstance() {
		static PfxPerfAnalyzer instance;
		return instance;
	}
	
	inline void push(const char *title);
	
	inline void pop();
	
	inline void clear();
	
	inline void print() {printResults(m_root);}

	inline void printCsv() {
		//printResultsTag(m_root);
		//SCE_PFX_PRINTF("\n");
		printResultsCsv(m_root);
		SCE_PFX_PRINTF("\n");
	}
	
	inline void gather() {gatherResults(m_root,0);}
};

template <size_t SIZE>
inline PfxPerfAnalyzer<SIZE>::PfxPerfAnalyzer()
{
	clear();
}

template <size_t SIZE>
inline void PfxPerfAnalyzer<SIZE>::clear()
{
	m_perfPool.clear();
	m_perfStack.clear();

	PerfInfo pinfo; // root
	m_root = m_perfPool.push(pinfo);
	m_perfStack.push(m_root);
}

template <size_t SIZE>
inline PfxPerf PfxPerfAnalyzer<SIZE>::getPerfCount()
{
	PfxPerf c;
#ifdef _WIN32
	QueryPerformanceCounter( (LARGE_INTEGER *)&c );
#elif defined(__ORBIS__) || defined(__PROSPERO__)
	c = sceKernelGetProcessTimeCounter();
#endif
	return c;
}

template <size_t SIZE>
inline void PfxPerfAnalyzer<SIZE>::push(const char *title)
{
	SCE_PFX_ALWAYS_ASSERT(m_perfPool.length() < SIZE);
	SCE_PFX_ALWAYS_ASSERT(!m_perfStack.empty());
	PerfInfo pinfo;
	strncpy(pinfo.title,title,SCE_PFX_MAX_PERF_STR-1);
	pinfo.title[SCE_PFX_MAX_PERF_STR-1] = '\0';
	PfxUInt32 pid = m_perfPool.push(pinfo);
	PfxUInt32 parent = m_perfStack.top();
	m_perfStack.push(pid);
	if(m_perfPool[parent].child != SCE_PFX_PERF_NONE) {
		PfxUInt32 iPerf = m_perfPool[parent].child;
		for(;m_perfPool[iPerf].next!=SCE_PFX_PERF_NONE;iPerf=m_perfPool[iPerf].next) {}
		m_perfPool[iPerf].next = pid;
	}
	else {
		m_perfPool[parent].child = pid;
	}
	m_perfPool[pid].parent = parent;
	m_perfPool[pid].depth = m_perfPool[parent].depth + 1;
	m_perfPool[pid].count = getPerfCount();
}

template <size_t SIZE>
inline void PfxPerfAnalyzer<SIZE>::pop()
{
	PfxPerf current = getPerfCount();
	PfxUInt32 pid = m_perfStack.top();
	m_perfStack.pop();
	m_perfPool[pid].count = current - m_perfPool[pid].count;
}

template <size_t SIZE>
inline void PfxPerfAnalyzer<SIZE>::printResults(PfxUInt32 pid)
{
	PfxFloat freq;

#ifdef _WIN32
	LARGE_INTEGER sPerfCountFreq;
	QueryPerformanceFrequency(&sPerfCountFreq);
	freq = (PfxFloat)sPerfCountFreq.QuadPart;
#elif defined(__ORBIS__) || defined(__PROSPERO__)
	freq = (PfxFloat)sceKernelGetTscFrequency();
#endif

	PerfInfo &info = m_perfPool[pid];
	char buf[256]={0};
	char *p = buf;
	char *e = buf + 255;

	if(pid > 0) {
		int t;
		for(t=1;t<(int)info.depth-1;t++) {
			if(p != e) {*(p++) = ' ';}
			if(p != e) {*(p++) = ' ';}
			if(p != e) {*(p++) = ' ';}
		}
		for(;t<(int)info.depth;t++) {
			if(p != e) {*(p++) = '+';}
			if(p != e) {*(p++) = '-';}
			if(p != e) {*(p++) = '-';}
		}
		if(info.ratio > 0.0f) {
			snprintf(p,e-p,"%s : %.2fms (%.1f%%)",
				info.title,(PfxFloat)(info.count / freq * 1000.0f),info.ratio*100.0f);
		}
		else {
			snprintf(p,e-p,"%s : %.2fms",
				info.title,(PfxFloat)(info.count / freq * 1000.0f));
		}
		SCE_PFX_PRINTF("%s\n",buf);
	}
		
	if(info.child != SCE_PFX_PERF_NONE) {
		printResults(info.child);
	}

	if(info.next != SCE_PFX_PERF_NONE) {
		printResults(info.next);
	}
}

template <size_t SIZE>
inline void PfxPerfAnalyzer<SIZE>::printResultsCsv(PfxUInt32 pid)
{
	PfxFloat freq;

#ifdef _WIN32
	LARGE_INTEGER sPerfCountFreq;
	QueryPerformanceFrequency(&sPerfCountFreq);
	freq = (PfxFloat)sPerfCountFreq.QuadPart;
#elif defined(__ORBIS__) || defined(__PROSPERO__)
	freq = (PfxFloat)sceKernelGetTscFrequency();
#endif

	PerfInfo &info = m_perfPool[pid];
	
	if(info.child == SCE_PFX_PERF_NONE) {
		if(pid > 0) {
			SCE_PFX_PRINTF("%f\t",(PfxFloat)(info.count / freq * 1000.0f));
			//SCE_PFX_PRINTF("%d,",info.count);
		}
	}
	
	if(info.child != SCE_PFX_PERF_NONE) {
		printResultsCsv(info.child);
	}

	if(info.next != SCE_PFX_PERF_NONE) {
		printResultsCsv(info.next);
	}
}

template <size_t SIZE>
inline void PfxPerfAnalyzer<SIZE>::printResultsTag(PfxUInt32 pid)
{
	PerfInfo &info = m_perfPool[pid];
	
	if(info.child == SCE_PFX_PERF_NONE) {
		if(pid > 0) {
			SCE_PFX_PRINTF("%s,",info.title);
		}
	}
	
	if(info.child != SCE_PFX_PERF_NONE) {
		printResultsTag(info.child);
	}

	if(info.next != SCE_PFX_PERF_NONE) {
		printResultsTag(info.next);
	}
}

template <size_t SIZE>
inline void PfxPerfAnalyzer<SIZE>::gatherResults(PfxUInt32 pid,PfxPerf total)
{
	PerfInfo &info = m_perfPool[pid];
	
	if(total > 0) {
		info.ratio = info.count/(PfxFloat)total;
	}
	else {
		info.ratio = 0.0f;
	}
	
	if(info.child != SCE_PFX_PERF_NONE) {
		gatherResults(info.child,info.count);
	}

	if(info.next != SCE_PFX_PERF_NONE) {
		gatherResults(info.next,total);
	}
}

} //namespace pfxv4
} //namespace sce

///////////////////////////////////////////////////////////////////////////////

#ifdef SCE_PFX_USE_BOOKMARK
	#if defined(__ORBIS__) || defined(__PROSPERO__)
		#define SCE_PFX_PUSH_MARKER_HUD(name,col)
		#define SCE_PFX_PUSH_MARKER(name) sceRazorCpuPushMarker(name,0,0);
		#define SCE_PFX_PUSH_MARKER_INF(name,id1,id2,id3) {char c[256];snprintf(c,256,"%s_%d_%d_%d",name,id1,id2,id3);sceRazorCpuPushMarker(c,0,0);}
		#define SCE_PFX_POP_MARKER() sceRazorCpuPopMarker();
	#else
		#define SCE_PFX_PUSH_MARKER_HUD(name,col)
		#define SCE_PFX_PUSH_MARKER(name)
		#define SCE_PFX_PUSH_MARKER_INF(name,id1,id2,id3)
		#define SCE_PFX_POP_MARKER()
	#endif
#else
	#define SCE_PFX_PUSH_MARKER_HUD(name,col)
	#define SCE_PFX_PUSH_MARKER(name)
	#define SCE_PFX_PUSH_MARKER_INF(name,id1,id2,id3)
	#define SCE_PFX_POP_MARKER()
#endif

#ifdef SCE_PFX_USE_PERFCOUNTER
	#define SCE_PFX_PUSH_PERF(title) PfxPerfAnalyzer<>::getInstance().push(title);
	#define SCE_PFX_POP_PERF() PfxPerfAnalyzer<>::getInstance().pop();
	#define SCE_PFX_PRINT_PERF() \
	{\
		PfxPerfAnalyzer<>::getInstance().gather();\
		PfxPerfAnalyzer<>::getInstance().printCsv();\
	}
	#define SCE_PFX_SYNC_PERF() PfxPerfAnalyzer<>::getInstance().clear();
#else
	#define SCE_PFX_PUSH_PERF(title)
	#define SCE_PFX_POP_PERF()
	#define SCE_PFX_PRINT_PERF()
	#define SCE_PFX_SYNC_PERF()
#endif

#endif // _SCE_PFX_PERF_COUNTER_H

