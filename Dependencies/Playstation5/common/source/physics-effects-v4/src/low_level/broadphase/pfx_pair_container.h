/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2022 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_PAIR_CONTAINER_H
#define _SCE_PFX_PAIR_CONTAINER_H

#include <thread>
#include <atomic>

#include "../../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../../include/physics_effects/base_level/broadphase/pfx_broadphase_pair.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Pool Buffer

template <class T>
class PfxPoolBuffer
{
friend class PfxPairContainer;

private:
	T *m_data;
	PfxUInt32 *m_reusableId;
	PfxUInt32 m_reusableIdCount;
	std::atomic<PfxUInt32> m_size;	// number of stored data
	std::atomic<PfxUInt32> m_length;	// length of consumed data
	PfxUInt32 m_capacity; // capacity of data

public:
	static inline PfxUInt32 queryBytes(PfxUInt32 maxData) {return sizeof(T)*maxData + sizeof(PfxUInt32)*maxData;}

	inline bool initialize(void *buffer, PfxUInt32 bytes, PfxUInt32 maxData);

	PfxUInt32 size() const {return m_size.load(std::memory_order_relaxed);}
	PfxUInt32 capacity() const {return m_capacity;}
	
	inline PfxUInt32 length() const { return m_length.load(std::memory_order_relaxed); }
	
	inline T& operator[](PfxUInt32 i);
	
	inline const T& operator[](PfxUInt32 i) const;
	
	inline void assign(PfxUInt32 num,const T &initData);
	
	inline void assign(PfxUInt32 num);
	
	inline bool empty() const {return m_size==0;}
	
	inline void clear();
	
	inline PfxUInt32 push(const T& data);
	
	inline bool remove(PfxUInt32 i);
};

template <class T>
inline bool PfxPoolBuffer<T>::initialize(void *buffer, PfxUInt32 bytes, PfxUInt32 maxData)
{
	if(bytes < queryBytes(maxData)) return false;

	m_capacity = maxData;

	uint8_t *p = (uint8_t*)buffer;
	m_data = (T*)p;
	p += sizeof(T) * m_capacity;
	m_reusableId = (PfxUInt32*)p;

	clear();

	return true;
}

template <class T>
inline T& PfxPoolBuffer<T>::operator[](PfxUInt32 i)
{
	SCE_PFX_ASSERT(i < m_length);
	return m_data[i];
}

template <class T>
inline const T& PfxPoolBuffer<T>::operator[](PfxUInt32 i) const
{
	SCE_PFX_ASSERT(i < m_length);
	return m_data[i];
}

template <class T>
inline void PfxPoolBuffer<T>::assign(PfxUInt32 num,const T &initData)
{
	clear();
	
	SCE_PFX_ASSERT(num <= m_capacity);
	
	for(PfxUInt32 i=0;i<num;i++) {
		push(initData);
	}
}
template <class T>
inline void PfxPoolBuffer<T>::assign(PfxUInt32 num)
{
	clear();

	SCE_PFX_ASSERT(num <= m_capacity);
	
	const T initData;

	for(PfxUInt32 i=0;i<num;i++) {
		push(initData);
	}
	
}

template <class T>
inline PfxUInt32 PfxPoolBuffer<T>::push(const T& data)
{
	SCE_PFX_ASSERT(m_size < m_capacity);
	
	PfxUInt32 id = (PfxUInt32)-1;
	if(m_reusableIdCount > 0) {
		id = m_reusableId[--m_reusableIdCount];
		m_data[id] = data;
		m_size.fetch_add(1, std::memory_order_acq_rel);
	}
	else if(m_length < m_capacity) {
		id = m_length;
		m_data[id] = data;
		m_size.fetch_add(1, std::memory_order_acq_rel);
	}

	if (id == m_length) {
		m_length.fetch_add(1, std::memory_order_acq_rel);
	}

	return id;
}

template <class T>
inline bool PfxPoolBuffer<T>::remove(PfxUInt32 i)
{
	SCE_PFX_ASSERT(i < m_length);
	
#ifdef _DEBUG
	for(PfxUInt32 j=0;j<m_reusableIdCount;j++) {
		if(m_reusableId[j] == i) {
			return false;
		}
	}
#endif

	m_reusableId[m_reusableIdCount++] = i;
	m_size--;
	
	if (i == m_length - 1) {
		m_length--;
	}

	memset(&m_data[i], 0xff, sizeof(T));

	return true;
}

template <class T>
inline void PfxPoolBuffer<T>::clear()
{
	m_reusableIdCount = 0;
	m_size = 0;
	m_length = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Pair Container

#define NODE_IS_NOT_ASSIGNED 0x07

class PfxPairContainer
{
public:
	struct PairNode
	{
		PfxUInt8 state : 3; // 7:not assigned 0:init 1:new 2:keep
		PfxUInt8 solverQuality : 2;
		PfxUInt8 numConstraints = 0;
		PfxUInt8 motionMaskA = 0;
		PfxUInt8 motionMaskB = 0;
		PfxUInt32 poolId = (PfxUInt32)-1; // an index in a pair pool
		PfxUInt16 rigidbodyIdA = 0xffff;
		PfxUInt16 rigidbodyIdB = 0xffff;
		PfxUInt32 data;			// ext data (contact index)

		PairNode()
		{
			state = NODE_IS_NOT_ASSIGNED;
			solverQuality = 0;
		}
	};
	
	struct BodyNode
	{
		PfxUInt32 pairId = (PfxUInt32)-1; // an index in a pair pool
		PfxUInt32 next = (PfxUInt32)-1, prev = (PfxUInt32)-1; // linked body nodes
		PfxUInt16 rigidbodyId = 0xffff;
	};

	struct BodyHead
	{
		PfxUInt32 headId; // an index of a body node
		PfxUInt16 numLinks;
	};

	enum eResult {
		kSuccess = 0,
		kInvalid,
		kAlreadyExists,
		kOverflowOfPairs,
		kOverflowOfBodies,
		kFailedToLockPair,
		kFailedToRemovePair,
	};

private:
	PfxUInt32 m_maxPairs;
	PfxUInt32 m_maxRigidBodies;

	PfxPoolBuffer<PairNode> m_pairPool;
	PfxPoolBuffer<BodyNode> m_bodyPool;
	BodyHead *m_bodyHeads;
	
	std::atomic<PfxInt8> *m_bodyLock;
	std::atomic<PfxInt8> m_resourceLock;

	inline bool tryLock(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB);
	inline void unlock(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB);
	
	inline void lockResource();
	inline void unlockResource();

	eResult connect(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB, PfxUInt32 &pairId);
	void disconnect(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB);

public:

	void clear();

	void verify();

	void print();
	
	PfxUInt32 getMaxPairs() const {return m_maxPairs;}
	PfxUInt32 getMaxRigidBodies() const {return m_maxRigidBodies;}

	PfxUInt32 getPairLength() const { return m_pairPool.length(); }
	const PairNode &getPairNode(PfxUInt32 i) const { return m_pairPool[i]; }
	PairNode &getPairNode(PfxUInt32 i) { return m_pairPool[i]; }

	PfxUInt32 getNumPairs() const { return m_pairPool.size(); }

	PfxUInt32 querySerializeBytes();
	PfxInt32 save(PfxUInt8 *buffer, PfxUInt32 bytes);
	PfxInt32 load(const PfxUInt8 *buffer, PfxUInt32 bytes);

	static PfxUInt32 queryBytes(PfxUInt32 maxPairs, PfxUInt32 maxRigidBodies);

	bool initialize(PfxUInt32 maxPairs, PfxUInt32 maxRigidBodies, void *buffer, PfxUInt32 bytes);

	// kOverflowOfBodies : overflow of bodies
	// kOverflowOfPairs : overflow of pairs
	// kFailedToLockPair : failed to lock a pair
	// kSuccess : success (inserted)
	// kAlreadyExists : already exists
	eResult tryInsertPair(const PfxBroadphasePair &pair, PfxUInt32 &pairId);

	// kFailedToRemovePair : failed to remove a pair
	// kFailedToLockPair : failed to lock a pair
	// kSuccess : success (removed)
	eResult tryRemovePair(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB);

	// kInvalid : failed to set data
	// kSuccess : success
	eResult setPairData(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB, PfxUInt32 data);
	inline eResult setPairData(PfxUInt32 pairId, PfxUInt32 data);

	// kInvalid : failed to get data
	// kSuccess : success
	eResult getPairData(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB, PfxUInt32 &data);
	inline eResult getPairData(PfxUInt32 pairId, PfxUInt32 &data);

	const BodyNode *getFirstRigidBody(PfxUInt16 rigidbodyId) const;
	const BodyNode *getNextRigidBody(const BodyNode *node) const;
};

inline bool PfxPairContainer::tryLock(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB)
{
	PfxInt32 lockA, lockB;
	lockA = m_bodyLock[rigidbodyIdA].exchange(1, std::memory_order_acquire);
	
	if (lockA == 0) {
		lockB = m_bodyLock[rigidbodyIdB].exchange(1, std::memory_order_acquire);
		
		if (lockB == 0) {
			return true;
		}
		else {
			m_bodyLock[rigidbodyIdA].store(0, std::memory_order_release);
		}
	}

	return false;
}

inline void PfxPairContainer::unlock(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB)
{
	m_bodyLock[rigidbodyIdA].store(0, std::memory_order_release);
	m_bodyLock[rigidbodyIdB].store(0, std::memory_order_release);
}

inline void PfxPairContainer::lockResource()
{
	while (m_resourceLock.exchange(1, std::memory_order_acquire) == 1) {
	}
}

inline void PfxPairContainer::unlockResource()
{
	m_resourceLock.store(0, std::memory_order_release);
}

PfxPairContainer::eResult PfxPairContainer::setPairData(PfxUInt32 pairId, PfxUInt32 data)
{
	if(pairId < m_pairPool.length()) {
		m_pairPool[pairId].data = data;
		return kSuccess;
	}
	else {
		return kInvalid;
	}
}

PfxPairContainer::eResult PfxPairContainer::getPairData(PfxUInt32 pairId, PfxUInt32 &data)
{
	if(pairId < m_pairPool.length()) {
		data = m_pairPool[pairId].data;
		return kSuccess;
	}
	else{
		return kInvalid;
	}
}

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_PAIR_CONTAINER_H

