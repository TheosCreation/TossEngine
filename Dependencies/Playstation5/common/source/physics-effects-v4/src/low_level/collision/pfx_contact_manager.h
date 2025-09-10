/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2022 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CONTACT_MANAGER_H_
#define _SCE_PFX_CONTACT_MANAGER_H_

#include "../../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../../include/physics_effects/base_level/collision/pfx_contact_manifold.h"
#include "../../util/pfx_heap_manager.h"
#include <thread>
#include <atomic>

//#define ENABLE_RING_BUFFER_TO_STORE_CONTACTS

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// PfxPoolArray (thread safe)

template <class T>
class PfxPoolBufferMT
{
friend class PfxContactManager;
private:
	PfxHeapManager *m_pool;
	T *m_data;
	PfxUInt32 *m_reusableId;
	PfxUInt32 m_reusableIdCount;
	std::atomic<PfxUInt32> m_size;	// number of stored data
	PfxUInt32 m_capacity;	// capcatity
	std::atomic<PfxUInt32> m_length;	// length of consumed data

	PfxUInt32 *m_poolTable;

	std::atomic <PfxInt8> m_lock;

	void lock()
	{
		int count = 0;
		while (m_lock.exchange(1, std::memory_order_acquire) == 1) {
			_mm_pause();
			if (((++count) % 100) == 0) {
				std::this_thread::yield();
			}
		}
	}

	void unlock()
	{
		m_lock.store(0, std::memory_order_release);
	}

public:
	static inline PfxUInt32 getBytes(PfxUInt32 maxData);

	inline void initialize(PfxHeapManager *pool, PfxUInt32 maxData);

	inline void finalize();

	PfxUInt32 size() const { return m_size.load(std::memory_order_relaxed); }
	PfxUInt32 capacity() const { return m_capacity; }

	inline PfxUInt32 length() const { return m_length.load(std::memory_order_relaxed); }

	inline T& operator[](PfxUInt32 i);

	inline const T& operator[](PfxUInt32 i) const;

	inline PfxUInt32 push(const T& data);

	inline bool remove(PfxUInt32 i);

	inline bool isRemoved(PfxUInt32 i) const;

	inline bool empty() const { return m_size == 0; }

	inline void clear();

	inline T* ptr() { return m_data; }
};

template <class T>
inline PfxUInt32 PfxPoolBufferMT<T>::getBytes(PfxUInt32 maxData)
{
	PfxUInt32 bytes = 0;
	bytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(T) * maxData);
	bytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxUInt32) * maxData);
	bytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxUInt32)*((maxData + 31) >> 5));
	return bytes;
}

template <class T>
inline void PfxPoolBufferMT<T>::initialize(PfxHeapManager *pool, PfxUInt32 maxData)
{
	m_pool = pool;
	m_capacity = maxData;
	m_data = (T*)m_pool->allocate(sizeof(T)*m_capacity);
	m_reusableId = (PfxUInt32*)m_pool->allocate(sizeof(PfxUInt32)*m_capacity);
	m_poolTable = (PfxUInt32*)m_pool->allocate(sizeof(PfxUInt32)*((m_capacity + 31) >> 5));
	m_lock = 0;
	clear();
}

template <class T>
inline void PfxPoolBufferMT<T>::finalize()
{
	m_pool->deallocate(m_poolTable);
	m_pool->deallocate(m_reusableId);
	m_pool->deallocate(m_data);
}

template <class T>
inline T& PfxPoolBufferMT<T>::operator[](PfxUInt32 i)
{
	SCE_PFX_ASSERT(i < m_capacity);
#ifdef SCE_PFX_POOL_ARRAY_ASSERT_REMOVED_DATA
	SCE_PFX_ASSERT((m_poolTable[i >> 5] & (1 << (i & 31)))>0);
#endif
	return m_data[i];
}

template <class T>
inline const T& PfxPoolBufferMT<T>::operator[](PfxUInt32 i) const
{
	SCE_PFX_ASSERT(i < m_capacity);
#ifdef SCE_PFX_POOL_ARRAY_ASSERT_REMOVED_DATA
	SCE_PFX_ASSERT((m_poolTable[i >> 5] & (1 << (i & 31)))>0);
#endif
	return m_data[i];
}

template <class T>
inline PfxUInt32 PfxPoolBufferMT<T>::push(const T& data)
{
	SCE_PFX_ASSERT(m_size < m_capacity);
	lock();
	PfxUInt32 id = (PfxUInt32)-1;
	if(m_reusableIdCount > 0) {
		id = m_reusableId[--m_reusableIdCount];
		m_data[id] = data;
		m_size++;
	}
	else if(m_length < m_capacity) {
		id = m_length;
		m_data[id] = data;
		m_size++;
	}

	if (id == m_length) {
		m_length++;
	}
	m_poolTable[id >> 5] |= (1 << (id & 31));

	m_data[id] = data;

	unlock();

	return id;
}

template <class T>
inline bool PfxPoolBufferMT<T>::remove(PfxUInt32 i)
{
	lock();
	if (i >= m_length || (m_poolTable[i >> 5] & (1 << (i & 31))) == 0) {
		unlock();
		return false;
	}

	m_reusableId[m_reusableIdCount++] = i;
	m_poolTable[i >> 5] &= ~(1u << (i & 31));
	m_size.fetch_add(-1, std::memory_order_acq_rel);

	if (i == m_length - 1) {
		m_length.fetch_add(-1, std::memory_order_acq_rel);
	}

	memset(&m_data[i], 0xff, sizeof(T));

	unlock();

	return true;
}

template <class T>
inline bool PfxPoolBufferMT<T>::isRemoved(PfxUInt32 i) const
{
	return (m_poolTable[i >> 5] & (1 << (i & 31))) == 0;
}

template <class T>
inline void PfxPoolBufferMT<T>::clear()
{
	m_reusableIdCount = 0;
	m_size = m_length = 0;
	memset(m_poolTable, 0, sizeof(PfxUInt32)*((m_capacity + 31) >> 5));
}

///////////////////////////////////////////////////////////////////////////////
// PfxPoolRing (lock free)

template <class T>
class PfxPoolRing
{
private:
	PfxUInt8 *m_used;
	T *m_data;

	PfxHeapManager *m_pool;

	PfxUInt32 m_capacity;
	std::atomic<PfxUInt32> m_size;	// number of stored data
	std::atomic<PfxUInt32> m_counter;

public:
	PfxPoolRing();
	~PfxPoolRing();

	static inline PfxUInt32 getBytes(PfxUInt32 maxData);

	inline void initialize(PfxHeapManager *pool, PfxUInt32 maxData);

	inline void finalize();

	inline PfxUInt32 size() const { return m_size; }
	inline PfxUInt32 capacity() const { return m_capacity; }
	inline PfxUInt32 length() const { return m_capacity; }

	inline T& operator[](PfxUInt32 i);

	inline const T& operator[](PfxUInt32 i) const;

	inline void assign(PfxUInt32 num, const T &initData);

	inline void assign(PfxUInt32 num);

	inline bool empty() const { return m_size == 0; }

	inline void clear();

	// thread safe methods

	inline PfxUInt32 push(const T& data);

	inline bool remove(PfxUInt32 i);
};

template <class T>
inline PfxUInt32 PfxPoolRing<T>::getBytes(PfxUInt32 maxData)
{
	return
		SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(T) * maxData) +
		SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxUInt8) * maxData);
}

template <class T>
inline void PfxPoolRing<T>::initialize(PfxHeapManager *pool, PfxUInt32 maxData)
{
	m_pool = pool;
	m_capacity = maxData;
	m_data = (T*)m_pool->allocate(sizeof(T)*m_capacity);
	m_used = (PfxUInt8*)m_pool->allocate(sizeof(PfxUInt8)*m_capacity);
	clear();
}

template <class T>
inline void PfxPoolRing<T>::finalize()
{
	m_pool->deallocate(m_used);
	m_pool->deallocate(m_data);
}

template <class T>
inline PfxPoolRing<T>::PfxPoolRing()
{
}

template <class T>
inline PfxPoolRing<T>::~PfxPoolRing()
{
}

template <class T>
inline T& PfxPoolRing<T>::operator[](PfxUInt32 i)
{
	SCE_PFX_ASSERT(i < m_capacity);
	return m_data[i];
}

template <class T>
inline const T& PfxPoolRing<T>::operator[](PfxUInt32 i) const
{
	SCE_PFX_ASSERT(i < m_capacity);
	return m_data[i];
}

template <class T>
inline void PfxPoolRing<T>::assign(PfxUInt32 num, const T &initData)
{
	clear();

	SCE_PFX_ASSERT(num <= m_capacity);

	for (PfxUInt32 i = 0; i<num; i++) {
		push(initData);
	}
}

template <class T>
inline void PfxPoolRing<T>::assign(PfxUInt32 num)
{
	clear();

	SCE_PFX_ASSERT(num <= m_capacity);

	const T initData;

	for (PfxUInt32 i = 0; i<num; i++) {
		push(initData);
	}

}

template <class T>
inline PfxUInt32 PfxPoolRing<T>::push(const T& data)
{
	SCE_PFX_ASSERT(m_size < m_capacity);

	// Increment this first so that the next push has to be failed if there is no room for new data.
	m_size++;

	PfxUInt32 exp = m_counter;
	PfxUInt32 val;
	do {
		do {
			val = (exp + 1) % m_capacity;
		} while (!m_counter.compare_exchange_strong(exp, val));
	} while (m_used[exp] != 0);

	m_used[exp] = 1;
	m_data[exp] = data;

	return exp;
}

template <class T>
inline bool PfxPoolRing<T>::remove(PfxUInt32 i)
{
	SCE_PFX_ASSERT(i < m_capacity);

	memset(&m_data[i], 0xff, sizeof(T));
	m_used[i] = 0;

	// Decrement this at the end to avoid to be written by other push
	m_size--;

	return true;
}

template <class T>
inline void PfxPoolRing<T>::clear()
{
	m_size = 0;
	m_counter = 0;
	memset(m_used, 0, sizeof(PfxUInt8) * m_capacity);
}

class PfxContactManager {
private:
	PfxHeapManager m_pool;
	PfxUInt32 m_maxContacts;
	PfxUInt32 m_maxClosestPoints;
	std::atomic<PfxUInt32> m_numClosestPoints;
#ifdef ENABLE_RING_BUFFER_TO_STORE_CONTACTS
	PfxPoolRing<PfxContactManifold> m_contactManifolds;
	PfxPoolRing<PfxContactHolder> m_contactHolders;
#else
	PfxPoolBufferMT<PfxContactManifold> m_contactManifolds;
	PfxPoolBufferMT<PfxContactHolder> m_contactHolders;
#endif
	PfxClosestPoint *m_closestPoints;

public:
	static inline PfxUInt32 queryBytes(PfxUInt32 maxContacts);

	PfxUInt32 getNumContactPoints() const {return m_contactHolders.size();}

	PfxUInt32 getNumClosestPoints() const {return m_numClosestPoints;}

	PfxUInt32 getMaxContactPoints() const {return m_maxContacts;}

	PfxUInt32 getMaxClosestPoints() const {return m_maxClosestPoints;}

	PfxInt32 initialize(PfxUInt32 maxContacts,void *workBuff,PfxUInt32 workBytes);

	void finalize();

	void clear();

	PfxContactHolder &getContactHolder(PfxUInt32 contactId) {return m_contactHolders[contactId];}
	const PfxContactHolder &getContactHolder(PfxUInt32 contactId) const { return m_contactHolders[contactId]; }
	PfxUInt32 getContactHolderCapacity() const {return m_contactHolders.capacity();}

	// Create a contact holder
	PfxInt32 createContactHolder(PfxUInt32 &contactId,PfxUInt32 iA,PfxUInt32 iB);

	// Remove a contact holder
	PfxInt32 removeContactHolder(PfxUInt32 contactId);

	// Return to an initial state
	PfxInt32 clearContactHolder(PfxUInt32 contactId);

	// Create a contact manifold
	PfxInt32 createContactManifold(PfxUInt32 &contactManifoldId, PfxUInt8 shapeIdA, PfxUInt8 shapeIdB);

	// Remove a contact manifold
	PfxInt32 removeContactManifold(PfxUInt32 contactManifoldId);

	// 最近接点を取得する（失敗した場合はNULLを返す）
	PfxClosestPoint *createClosestPoints(int num);

	// Insert a contact manifold to a contact holder
	PfxContactManifold *insertContactManifoldToContactHolder(PfxContactHolder &contactHolder, PfxUInt16 shapeId);

	// Remove a contact manifold from a contact holder
	PfxBool removeContactManifoldFromContactHolder(PfxContactHolder &contactHolder, PfxUInt16 shapeId);

	// Remove contact manifolds from a contact holder if they are empty
	PfxBool removeEmptyContactManifoldFromContactHolder(PfxContactHolder &contactHolder);

	// 最近接点をクリアする
	void clearAllClosestPoints();

	// Print buffer usage
	void print();

	PfxUInt32 querySerializeBytes();
	PfxInt32 save(PfxUInt8 *buffer, PfxUInt32 bytes);
	PfxInt32 load(const PfxUInt8 *buffer, PfxUInt32 bytes);
};

inline PfxUInt32 PfxContactManager::queryBytes(PfxUInt32 maxContacts)
{
	PfxUInt32 bytes = 0;
#ifdef ENABLE_RING_BUFFER_TO_STORE_CONTACTS
	bytes += PfxPoolRing<PfxContactManifold>::getBytes(maxContacts);
	bytes += PfxPoolRing<PfxContactPoint>::getBytes(maxContacts * 4);
	bytes += PfxPoolRing<PfxContactHolder>::getBytes(maxContacts);
#else
	bytes += PfxPoolBufferMT<PfxContactManifold>::getBytes(maxContacts);
	bytes += PfxPoolBufferMT<PfxContactPoint>::getBytes(maxContacts * 4);
	bytes += PfxPoolBufferMT<PfxContactHolder>::getBytes(maxContacts);
#endif
	return bytes;
}

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_CONTACT_MANAGER_H_ */
