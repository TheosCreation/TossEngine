/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_POOL_ARRAY_H
#define _SCE_PFX_POOL_ARRAY_H

#include "../../include/physics_effects/base_level/base/pfx_common.h"
#include "pfx_heap_manager.h"

namespace sce{
namespace pfxv4{

///////////////////////////////////////////////////////////////////////////////
// PfxPoolQueue

template <class T>
class PfxPoolQueue
{
private:
	PfxHeapManager *m_pool;
	PfxUInt32 m_numData;
	PfxUInt32 m_maxData;
	PfxUInt32 m_head;
	PfxUInt32 m_tail;
	T *m_data;
	
public:
	static inline PfxUInt32 getBytes(PfxUInt32 maxData);
	
	inline void initialize(PfxHeapManager *pool,PfxUInt32 maxData);
	
	inline void finalize();
	
	PfxUInt32 size() const {return m_numData;}
	
	inline PfxUInt32 push(const T& data);
	
	inline void pop();
	
	inline T& front();
	
	inline const T& front() const;
	
	inline bool empty() const {return m_numData==0;}
	
	inline void clear() {m_numData=0;m_head=0;m_tail=0;}
};

///////////////////////////////////////////////////////////////////////////////
// PfxPoolPriorityQueue

template <class T>
class PfxPoolPriorityQueue
{
private:
	PfxHeapManager *m_pool;
	PfxUInt32 m_numData;
	PfxUInt32 m_maxData;
	T *m_data;
	
	void upHeap(PfxUInt32 i);
	void downHeap(PfxUInt32 i);
	
public:
	static inline PfxUInt32 getBytes(PfxUInt32 maxData);
	
	inline void initialize(PfxHeapManager *pool,PfxUInt32 maxData);
	
	inline void finalize();
	
	PfxUInt32 size() const {return m_numData-1;}
	
	inline PfxUInt32 push(const T& data);
	
	inline void pop();
	
	inline T& front();
	
	inline const T& front() const;
	
	inline bool empty() const {return m_numData==1;}
	
	inline void clear() {m_numData=1;}
};

///////////////////////////////////////////////////////////////////////////////
// PfxPoolStack

template <class T>
class PfxPoolStack
{
private:
	PfxHeapManager *m_pool;
	PfxUInt32 m_numData;
	PfxUInt32 m_maxData;

	T *m_data;

public:
	static inline PfxUInt32 getBytes(PfxUInt32 maxData);
	
	inline void initialize(PfxHeapManager *pool,PfxUInt32 maxData);
	
	inline void finalize();
	
	PfxUInt32 size() const {return m_numData;}
	
	inline PfxUInt32 push(const T& data);
	
	inline void pop();
	
	inline T& top();
	inline const T& top() const;
	
	inline bool empty() const {return m_numData==0;}
	
	inline void clear() {m_numData = 0;}
};

///////////////////////////////////////////////////////////////////////////////
// PfxPoolArray

template <class T>
class PfxPoolArray
{
private:
	PfxHeapManager *m_pool;
	PfxUInt32 m_numData;
	PfxUInt32 m_maxData;
	PfxUInt32 m_dataLength;
	T *m_data;
	
	PfxUInt32 *m_poolTable;

	PfxPoolStack<PfxUInt32> m_poolId;
	
public:
	static inline PfxUInt32 getBytes(PfxUInt32 maxData);
	
	inline void initialize(PfxHeapManager *pool,PfxUInt32 maxData);
	
	inline void finalize();
	
	PfxUInt32 size() const {return m_numData;}
	PfxUInt32 capacity() const {return m_maxData;}
	
	inline PfxUInt32 length() const {return m_dataLength;}
	
	inline T& operator[](PfxUInt32 i);
	
	inline const T& operator[](PfxUInt32 i) const;
	
	inline void assign(PfxUInt32 num,const T &initData);

	inline void assign(PfxUInt32 num);
	
	inline PfxUInt32 push(const T& data);
	
	inline bool remove(PfxUInt32 i);

	inline bool isRemoved(PfxUInt32 i) const;

	inline bool empty() const {return m_numData==0;}
	
	inline void clear();

	inline T* ptr() {return m_data;}
};

///////////////////////////////////////////////////////////////////////////////
// PfxPoolMap

template <class KEY, class T>
class PfxPoolMap
{
private:
	PfxHeapManager *m_pool;
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
	static inline PfxUInt32 getBytes(PfxUInt32 maxData);

	inline void initialize(PfxHeapManager *pool,PfxUInt32 maxData);
	
	inline void finalize();
	
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

} // namespace pfxv4
} // namespace sce

#include "pfx_pool_array_implementation.h"

#endif // _SCE_PFX_POOL_ARRAY_H
