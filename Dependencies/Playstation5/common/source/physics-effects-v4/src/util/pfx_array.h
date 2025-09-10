/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_ARRAY_H
#define _SCE_PFX_ARRAY_H

#include "../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../include/physics_effects/util/pfx_util_common.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Basic container class

///////////////////////////////////////////////////////////////////////////////
// PfxArray

/*
	Variable array
*/

template <class T>
class PfxArray
{
private:
	PfxUInt32 m_numData;
	PfxUInt32 m_maxData;
	SCE_PFX_PADDING(1,8)
	T SCE_PFX_ALIGNED(16) *m_data;
	SCE_PFX_PADDING(2,12)
	
public:
	PfxArray() : m_numData(0),m_maxData(100)
	{
		m_data = (T*)SCE_PFX_UTIL_ALLOC(16,sizeof(T)*m_maxData);
	}
	
	PfxArray(PfxUInt32 maxData) : m_numData(0),m_maxData(maxData)
	{
		m_data = (T*)SCE_PFX_UTIL_ALLOC(16,sizeof(T)*m_maxData);
	}
	
	~PfxArray()
	{
		SCE_PFX_UTIL_FREE(m_data);
	}
	
	PfxUInt32 size() const {return m_numData;}

	PfxUInt32 capacity() const {return m_maxData;}
	
	inline T& operator[](PfxUInt32 i);
	
	inline const T& operator[](PfxUInt32 i) const;
	
	inline const PfxArray& operator=(const PfxArray &array);
	
	inline void assign(PfxUInt32 num,const T &initData);
	
	inline void assign(PfxUInt32 num);
	
	inline PfxUInt32 push(const T& data);
	
	inline bool remove(PfxUInt32 i);
	
	inline bool empty() const {return m_numData==0;}
	
	inline void clear() {m_numData = 0;}

	inline T* ptr(){return m_data;}
};

///////////////////////////////////////////////////////////////////////////////
// PfxQueue

/*
	Queue (static size)
*/

template <class T>
class PfxQueue
{
private:
	PfxUInt32 m_numData;
	PfxUInt32 m_maxData;
	PfxUInt32 m_head;
	PfxUInt32 m_tail;
	T SCE_PFX_ALIGNED(16) *m_data;
	SCE_PFX_PADDING(1,12)
	
	PfxQueue() {}
	
public:
	
	PfxQueue(PfxUInt32 maxData) : m_numData(0),m_maxData(maxData),m_head(0),m_tail(0)
	{
		m_data = (T*)SCE_PFX_UTIL_ALLOC(16,sizeof(T)*m_maxData);
	}
	
	~PfxQueue()
	{
		SCE_PFX_UTIL_FREE(m_data);
	}
	
	PfxUInt32 size() const {return m_numData;}
	
	inline PfxUInt32 push(const T& data);
	
	inline void pop();
	
	inline T& front();
	
	inline const T& front() const;
	
	inline bool empty() const {return m_numData==0;}
	
	inline void clear() {m_numData=0;m_head=0;m_tail=0;}

	inline T* ptr(){return m_data;}
};

///////////////////////////////////////////////////////////////////////////////
// PfxPriorityQueue

/*
	Priority Queue (static size)
	* comparison function
	　PfxBool operator > (const T &data1,const T &data2);
*/

template <class T>
class PfxPriorityQueue
{
private:
	PfxUInt32 m_numData;
	PfxUInt32 m_maxData;
	T SCE_PFX_ALIGNED(16) *m_data;
	SCE_PFX_PADDING(1,12)
	
	PfxPriorityQueue() {}
	
	void upHeap(PfxUInt32 i);
	void downHeap(PfxUInt32 i);
	
public:
	PfxPriorityQueue(PfxUInt32 maxData) : m_numData(1),m_maxData(maxData+1)
	{
		m_data = (T*)SCE_PFX_UTIL_ALLOC(16,sizeof(T)*m_maxData);
	}
	
	~PfxPriorityQueue()
	{
		SCE_PFX_UTIL_FREE(m_data);
	}
	
	PfxUInt32 size() const {return m_numData-1;}
	
	inline PfxUInt32 push(const T& data);
	
	inline void pop();
	
	inline T& front();
	
	inline const T& front() const;
	
	inline bool empty() const {return m_numData==1;}
	
	inline void clear() {m_numData=1;}
};

///////////////////////////////////////////////////////////////////////////////
// PfxStack

/*
	Stack (static size)
*/

template <class T>
class PfxStack
{
private:
	PfxUInt32 m_numData;
	PfxUInt32 m_maxData;
	T SCE_PFX_ALIGNED(16) *m_data;
	
	PfxStack() {}
	
public:

	PfxStack(PfxUInt32 maxData) : m_numData(0),m_maxData(maxData)
	{
		m_data = (T*)SCE_PFX_UTIL_ALLOC(16,sizeof(T)*m_maxData);
	}
	
	~PfxStack()
	{
		SCE_PFX_UTIL_FREE(m_data);
	}
	
	PfxUInt32 size() const {return m_numData;}
	
	inline PfxUInt32 push(const T& data);
	
	inline void pop();
	
	inline T& top();
	
	inline const T& top() const;
	
	inline bool empty() const {return m_numData==0;}
	
	inline void clear() {m_numData = 0;}
};

///////////////////////////////////////////////////////////////////////////////
// PfxMap

template <class KEY, class T>
class PfxMap
{
private:
	PfxUInt32 m_numData;
	PfxUInt32 m_maxData;

	struct PfxMapNode {
		KEY key;
		T *data;
		PfxMapNode *prev;
		PfxMapNode *next;
		SCE_PFX_PADDING(1,4)
	};

	PfxMapNode *m_nodes;
	PfxMapNode **m_headers;
	T *m_data;

	inline PfxUInt32 getHashId(const KEY& key);
	
public:
	PfxMap();
	PfxMap(PfxUInt32 maxData);
	~PfxMap();
	
	PfxUInt32 size() const {return m_numData;}
	
	PfxUInt32 capacity() const {return m_maxData;}
	
	inline bool insert(const KEY& key, const T& data);
	
	inline bool find(const KEY& key,T& data) const;
	
	inline bool find(const KEY& key,T& data);
	
	inline bool erase(const KEY& key);
	
	inline bool empty() const {return m_numData==0;}
	
	inline void clear();

	inline T* ptr(){return m_data;}
};

} //namespace pfxv4
} //namespace sce

#include "pfx_array_implementation.h"

#endif // _SCE_PFX_ARRAY_H
