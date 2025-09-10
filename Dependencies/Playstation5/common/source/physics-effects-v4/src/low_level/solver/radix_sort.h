/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
*                Copyright (C) 2020 Sony Interactive Entertainment Inc.
*                                                
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <atomic>
//#include <x86intrin.h>
#include "../task/pfx_job_system.h"

#ifdef _DEBUG
#define _CHECK_RESULT
#endif

#ifdef _DEBUG
#define ASSERT(cond) do { if(!(cond)) { printf("Assertion %s failed.\n", #cond);abort(); } } while(0)
#else //_DEBUG
#define ASSERT(cond) do { ((void)1); } while(0)
#endif //_DEBUG

#define UNUSED(var)  (void)var;

#ifdef _DEBUG
//#define ERROR_RETURN(cond, code, msg, ...) if(__builtin_expect(!(cond), 0)) { printf(msg, ##__VA_ARGS__); return (code); }
#define ERROR_RETURN(cond, code, msg, ...) if(__builtin_expect(!(cond), 0)) { abort(); }
#else //_DEBUG
#define ERROR_RETURN(cond, code, msg, ...) if(__builtin_expect(!(cond), 0)) { return (code); }
#endif //_DEBUG
template <typename _Data> class RadixSort
{
public:

	void setNumWorkers(int32_t numWorkers)
	{
		m_numWorkers = numWorkers;
		m_numJobsExtractKey = numWorkers;
		m_numJobsSortData = numWorkers;
		m_numJobsShuffle = numWorkers;
	}

	bool initialize(int32_t numElements, void *buff, int32_t bytes)
	{
		if (bytes < queryBytes(numElements, m_numWorkers)) return false;

		m_numElements = numElements;

		m_jobScanBase = m_jobScanBaseBuff;

		uint8_t *p = (uint8_t*)buff;

		m_scanTable = (ScanArray<kScanTableSize>*)p;
		p += sizeof(ScanArray<kScanTableSize>) * (m_numJobsShuffle + 1);
		m_frontKey = (uint32_t*)p;
		p += sizeof(uint32_t) * m_numElements;
		m_frontIndex = (uint32_t*)p;
		p += sizeof(uint32_t) * m_numElements;
		m_backKey = (uint32_t*)p;
		p += sizeof(uint32_t) * m_numElements;
		m_backIndex = (uint32_t*)p;
		m_checkLast = 0;

		return true;
	}

	void finalize()
	{
	}

	int32_t queryBytes(int32_t numElements, int32_t numWorkers)
	{
		int32_t bytes = 0;
		bytes += sizeof(ScanArray<kScanTableSize>) * (numWorkers + 1);
		bytes += sizeof(uint32_t) * numElements * 4;
		return bytes;
	}

	void flushAllBuffers()
	{
		execClflush(m_jobScanBase, sizeof(std::atomic<uint32_t>) * m_numJobsScan);
		execClflush(m_scanTable, sizeof(ScanArray<kScanTableSize>) * (m_numJobsShuffle + 1));
		execClflush(m_frontKey, sizeof(uint32_t) * m_numElements);
		execClflush(m_frontIndex, sizeof(uint32_t) * m_numElements);
		execClflush(m_backKey, sizeof(uint32_t) * m_numElements);
		execClflush(m_backIndex, sizeof(uint32_t) * m_numElements);
	}

	int32_t sort(sce::pfxv4::PfxJobSystem *jobSystem, _Data *input, _Data *output)
	{
		int32_t ret = SCE_OK;
		m_input = input;
		m_output = output;
		if ((m_numJobsExtractKey == 1) && (m_numJobsShuffle == 1) && (m_numJobsScan == 1) && (m_numJobsSortData == 1))
		{
			ret = jobSystem->dispatch(singleSort, this, 1, "singleSort");
			if (ret != SCE_OK) return ret;
		}
		else
		{
			ret = jobSystem->dispatch(extractKey, this, m_numJobsExtractKey, "extractKey");
			if (ret != SCE_OK) return ret;
			ret = jobSystem->barrier();
			if (ret != SCE_OK) return ret;
			for (int32_t keyBit = 0; keyBit < kKeySize; keyBit += kNumRadixBits)
			{
				ret = jobSystem->dispatch(accumulate, this, m_numJobsShuffle, "accumulate");
				if (ret != SCE_OK) return ret;
				ret = jobSystem->barrier();
				if (ret != SCE_OK) return ret;
				if (m_numJobsScan != 1)
				{
					ret = jobSystem->dispatch(verticalScan, this, m_numJobsScan, "verticalScan");
					if (ret != SCE_OK) return ret;
					ret = jobSystem->barrier();
					if (ret != SCE_OK) return ret;
					ret = jobSystem->dispatch(horizontalScan, this, m_numJobsScan, "holizontalScan");
					if (ret != SCE_OK) return ret;
					ret = jobSystem->barrier();
					if (ret != SCE_OK) return ret;
				}
				ret = jobSystem->dispatch(shuffle, this, m_numJobsShuffle, "shuffle");
				if (ret != SCE_OK) return ret;
				ret = jobSystem->barrier();
				if (ret != SCE_OK) return ret;
			}
			ret = jobSystem->barrier();
			if (ret != SCE_OK) return ret;
			ret = jobSystem->dispatch(sortData, this, m_numJobsSortData, "sortData");
			if (ret != SCE_OK) return ret;
		}
		return ret;
	}

	static void singleSort(void *arg, int32_t count) { ((RadixSort *)arg)->_singleSort(); }
	void _singleSort()
	{
		_extractKey(0);
		for (int32_t keyBit = 0; keyBit < kKeySize; keyBit += kNumRadixBits)
		{
			_accumulate(0);
			_shuffle(0);
		}
		_sortData(0);
	}

private:
	
	static const uint32_t kKeySize = 32;
	//static const uint32_t kNumRadixBits = 16;
	//static const uint32_t kScanTableSize = 64 * 1024;
	//static const uint32_t kNumRadixBits = 11;
	//static const uint32_t kScanTableSize = 2 * 1024;
	static const uint32_t kNumRadixBits = 8;
	static const uint32_t kScanTableSize = 256;
	//static const uint32_t kNumRadixBits = 4;
	//static const uint32_t kScanTableSize = 16;

	static const uint32_t kCacheLineSize = 64;
	static const uint32_t kSortPrefetchDistanceInByte = kCacheLineSize * 4;

	template <int _Size> class ScanArray
	{
	public:

		ScanArray()
		{
			clear(0, _Size);
		}

		void scan(uint32_t base, int32_t indexLo, int32_t indexHi)
		{
			uint32_t sum = base;
			for (int32_t i = indexLo; i < indexHi; ++i)
			{
				uint32_t oldSum = sum;
				sum += m_table[i];
				m_table[i] = oldSum;
			}
		}

		void accumulate(ScanArray &rhs, int32_t indexLo, int32_t indexHi)
		{
			for (int32_t i = indexLo; i < indexHi; ++i)
			{
				m_table[i] += rhs.m_table[i];
			}
		}

		void accumulate(uint32_t num, int index)
		{
			m_table[index] += num;
		}

		uint32_t getAndAdd(int32_t a, int32_t index)
		{
			uint32_t rv = m_table[index];
			m_table[index] = rv + a;
			return rv;
		}

		uint32_t getValue(int32_t index)
		{
			uint32_t rv;
			rv = m_table[index];
			return rv;
		}

		void setValue(int32_t index, uint32_t value)
		{
			m_table[index] = value;
		}

		void clear(int32_t indexLo, int32_t indexHi)
		{
			memset(m_table + indexLo, 0, sizeof(uint32_t) * (indexHi - indexLo));
		}

		uint32_t sum(int32_t indexLo, int32_t indexHi)
		{
			uint32_t rv = 0;
			for (int32_t i = indexLo; i < indexHi; ++i)
			{
				rv += m_table[i];
			}
			return rv;
		}

	private:

		uint32_t m_table[_Size];
	};

	_Data *m_input;
	_Data *m_output;
	int32_t m_numElements;
	int32_t m_numWorkers;
	int32_t m_numJobsExtractKey;
	int32_t m_numJobsShuffle;
	int32_t m_numJobsSortData;

	static const int32_t m_numJobsScan = 1;

	std::atomic<uint32_t> m_jobScanBaseBuff[m_numJobsScan];
	std::atomic<uint32_t> *m_jobScanBase;
	ScanArray<kScanTableSize> *m_scanTable;

	std::atomic<int32_t> m_checkLast;

	uint32_t *m_frontKey, *m_backKey;
	uint32_t *m_frontIndex, *m_backIndex;

	void split(int32_t size, int32_t count, int32_t index, uint32_t &first, uint32_t &end)
	{
		int32_t modulo = size % count;
		int32_t div = size / count;
		first = index * div + SCE_PFX_MIN( index, modulo );
		end = first + div + ( ( index < modulo ) ? 1 : 0 );
	}

	void execClflush(void *base, size_t size)
	{
		char *addr = (char *)base;
		for (size_t offset = 0; offset < size; offset += 64)
		{
			_mm_clflush(addr + offset);
		}
	}

	template <uint32_t _Size> void moveNt(void *dest, const void *src)
	{
		int size = _Size;
		while (size >= 16)
		{
			__m128i *dest128 = reinterpret_cast<__m128i *>(dest);
			const __m128i *src128 = reinterpret_cast<const __m128i *>(src);
			_mm_stream_si128(dest128, *src128);
			dest = dest128 + 1;
			src = src128 + 1;
			size -= 16;
		}
		if (size >= 8)
		{
			long long *dest64 = reinterpret_cast<long long *>(dest);
			const long long *src64 = reinterpret_cast<const long long *>(src);
			_mm_stream_si64(dest64, *src64);
			dest = dest64 + 1;
			src = src64 + 1;
			size -= 8;
		}
		if (size >= 4)
		{
			int32_t *dest32 = reinterpret_cast<int32_t *>(dest);
			const int32_t *src32 = reinterpret_cast<const int32_t *>(src);
			_mm_stream_si32(dest32, *src32);
		}
	}

	static void extractKey(void *arg, int32_t count) { ((RadixSort *)arg)->_extractKey(count); }
	inline void _extractKey(int32_t count)
	{
		uint32_t first, end;
		split(m_numElements, m_numJobsExtractKey, count, first, end);
		for (uint32_t i = first; i < end; ++i)
		{
			m_frontKey[i] = m_input[i].getKey();
			m_frontIndex[i] = i;
		}
	}

	static void accumulate(void *arg, int32_t count) { ((RadixSort *)arg)->_accumulate(count); }
	inline void _accumulate(int32_t count)
	{
		uint32_t first, end;
		m_scanTable[count + 1].clear(0, kScanTableSize);
		split(m_numElements, m_numJobsShuffle, count, first, end);
		for (int32_t i = first; i < end; ++i)
		{
			m_scanTable[count + 1].accumulate(1, m_frontKey[i] & (kScanTableSize - 1));
		}
		if (++m_checkLast == m_numJobsShuffle)
		{
			for (int i = 0; i < m_numJobsScan; ++i)
			{
				m_jobScanBase[i] = 0;
			}
			if (m_numJobsScan == 1)
			{
				_verticalScan(0);
				_horizontalScan(0);
			}
			m_checkLast = 0;
		}
	}

	static void verticalScan(void *arg, int32_t count) { ((RadixSort *)arg)->_verticalScan(count); }
	inline void _verticalScan(int32_t count)
	{
		uint32_t first, end;
		split(kScanTableSize, m_numJobsScan, count, first, end);
		m_scanTable[0].clear(first, end);
		for (int32_t j = 1; j < m_numJobsShuffle; ++j)
		{
			m_scanTable[j + 1].accumulate(m_scanTable[j], first, end);
		}
		uint32_t localBase = 0;
		if (m_numJobsScan > 1)
		{
			localBase = m_scanTable[m_numJobsShuffle].sum(first, end);
		}
		for (int32_t i = count + 1; i < m_numJobsScan; ++i)
		{
			m_jobScanBase[i] += localBase;
		}
	}

	static void horizontalScan(void *arg, int32_t count) { ((RadixSort *)arg)->_horizontalScan(count); }
	inline void _horizontalScan(int32_t count)
	{
		uint32_t first, end;
		split(kScanTableSize, m_numJobsScan, count, first, end);
		m_scanTable[m_numJobsShuffle].scan(m_jobScanBase[count], first, end);
	}

	static void shuffle(void *arg, int32_t count) { ((RadixSort *)arg)->_shuffle(count); }
	inline void _shuffle(int32_t count)
	{
		uint32_t first, end;
		split(m_numElements, m_numJobsShuffle, count, first, end);
		m_scanTable[count].accumulate(m_scanTable[m_numJobsShuffle], 0, kScanTableSize);
		for (int i = first; i < end; ++i)
		{
			uint32_t key = m_frontKey[i];
			uint32_t keyIndex = key & (kScanTableSize - 1);
			uint32_t outIndex = m_scanTable[count].getAndAdd(1, keyIndex);
			m_backKey[outIndex] = key >> kNumRadixBits;
			m_backIndex[outIndex] = m_frontIndex[i];
		}
		if (++m_checkLast == m_numJobsShuffle)
		{
			pfxSwap(m_frontKey, m_backKey);
			pfxSwap(m_frontIndex, m_backIndex);
			m_checkLast = 0;
		}
	}

	static void sortData(void *arg, int32_t count) { ((RadixSort *)arg)->_sortData(count); }
	inline void _sortData(int32_t count)
	{
		uint32_t first, end;
		split(m_numElements, m_numJobsSortData, count, first, end);
		uint32_t distance = kSortPrefetchDistanceInByte / sizeof(_Data);
		distance = (distance == 0) ? 1 : distance;
		int32_t mainEnd = (int32_t)end - (int32_t)distance;
		int32_t index;
		for (index = first; index < mainEnd; ++index)
		{
			_mm_prefetch((const char*)(m_input + m_frontIndex[index + distance]), _MM_HINT_T0);
			moveNt<sizeof(_Data)>(m_output + index, m_input + m_frontIndex[index]);
		}
		for (; index < end; ++index)
		{
			m_output[index] = m_input[m_frontIndex[index]];
		}
	}
};
