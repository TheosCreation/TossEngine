/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2022 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_pair_container.h"
#include "../../../src/util/pfx_binary_reader_writer.h"

namespace sce {
namespace pfxv4 {

PfxPairContainer::eResult PfxPairContainer::connect(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB, PfxUInt32 &pairId_)
{
	if (m_bodyHeads[rigidbodyIdA].numLinks < m_bodyHeads[rigidbodyIdB].numLinks) {
		PfxUInt32 poolId = m_bodyHeads[rigidbodyIdA].headId;
		while (poolId != (PfxUInt32)-1) {
			BodyNode &bodyNode = m_bodyPool[poolId];
			if (bodyNode.rigidbodyId == rigidbodyIdB) {
				if (m_pairPool[bodyNode.pairId].data == (PfxUInt32)-1) {
					m_pairPool[bodyNode.pairId].state = 1;
				}
				else {
					m_pairPool[bodyNode.pairId].state = 2;
				}
				pairId_ = bodyNode.pairId;
				return kAlreadyExists; // already exists
			}
			poolId = bodyNode.next;
		}
	}
	else {
		PfxUInt32 poolId = m_bodyHeads[rigidbodyIdB].headId;
		while (poolId != (PfxUInt32)-1) {
			BodyNode &bodyNode = m_bodyPool[poolId];
			if (bodyNode.rigidbodyId == rigidbodyIdA) {
				if (m_pairPool[bodyNode.pairId].data == (PfxUInt32)-1) {
					m_pairPool[bodyNode.pairId].state = 1;
				}
				else {
					m_pairPool[bodyNode.pairId].state = 2;
				}
				pairId_ = bodyNode.pairId;
				return kAlreadyExists; // already exists
			}
			poolId = bodyNode.next;
		}
	}

	lockResource();
	PfxUInt32 pairId = m_pairPool.push(PairNode());
	PfxUInt32 poolIdA = m_bodyPool.push(BodyNode());
	PfxUInt32 poolIdB = m_bodyPool.push(BodyNode());
	unlockResource();

	BodyNode &bodyNodeA = m_bodyPool[poolIdA];
	BodyNode &bodyNodeB = m_bodyPool[poolIdB];

	bodyNodeA.pairId = pairId;
	bodyNodeA.rigidbodyId = rigidbodyIdB;

	bodyNodeB.pairId = pairId;
	bodyNodeB.rigidbodyId = rigidbodyIdA;

	m_pairPool[pairId].poolId = pairId;
	m_pairPool[pairId].state = 1;
	m_pairPool[pairId].data = (PfxUInt32)-1;
	m_pairPool[pairId].rigidbodyIdA = rigidbodyIdA;
	m_pairPool[pairId].rigidbodyIdB = rigidbodyIdB;
	pairId_ = pairId;

	// connect A
	if (m_bodyHeads[rigidbodyIdA].headId == (PfxUInt32)-1) {
		m_bodyHeads[rigidbodyIdA].headId = poolIdA;
		m_bodyHeads[rigidbodyIdA].numLinks++;
	}
	else {
		PfxUInt32 tmp = m_bodyHeads[rigidbodyIdA].headId;
		m_bodyHeads[rigidbodyIdA].headId = m_bodyPool[tmp].prev = poolIdA;
		m_bodyHeads[rigidbodyIdA].numLinks++;
		bodyNodeA.next = tmp;
	}

	// connect B
	if (m_bodyHeads[rigidbodyIdB].headId == (PfxUInt32)-1) {
		m_bodyHeads[rigidbodyIdB].headId = poolIdB;
		m_bodyHeads[rigidbodyIdB].numLinks++;
	}
	else {
		PfxUInt32 tmp = m_bodyHeads[rigidbodyIdB].headId;
		m_bodyHeads[rigidbodyIdB].headId = m_bodyPool[tmp].prev = poolIdB;
		m_bodyHeads[rigidbodyIdB].numLinks++;
		bodyNodeB.next = tmp;
	}

	return kSuccess; // added
}

void PfxPairContainer::disconnect(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB)
{
	// disconnect B from A
	PfxUInt32 poolIdA = m_bodyHeads[rigidbodyIdA].headId;
	while (poolIdA != (PfxUInt32)-1) {
		BodyNode &bodyNode = m_bodyPool[poolIdA];

		if (bodyNode.rigidbodyId == rigidbodyIdB) {
			if (bodyNode.prev == (PfxUInt32)-1 && bodyNode.next != (PfxUInt32)-1) {
				m_bodyHeads[rigidbodyIdA].headId = bodyNode.next;
				m_bodyPool[bodyNode.next].prev = (PfxUInt32)-1;
			}
			else if (bodyNode.prev != (PfxUInt32)-1 && bodyNode.next == (PfxUInt32)-1) {
				m_bodyPool[bodyNode.prev].next = (PfxUInt32)-1;
			}
			else if (bodyNode.prev != (PfxUInt32)-1 && bodyNode.next != (PfxUInt32)-1) {
				m_bodyPool[bodyNode.prev].next = bodyNode.next;
				m_bodyPool[bodyNode.next].prev = bodyNode.prev;
			}
			else {
				m_bodyHeads[rigidbodyIdA].headId = (PfxUInt32)-1;
			}
			m_bodyHeads[rigidbodyIdA].numLinks--;
			m_pairPool[bodyNode.pairId].state = NODE_IS_NOT_ASSIGNED;
			lockResource();
			m_pairPool.remove(bodyNode.pairId);
			m_bodyPool.remove(poolIdA);
			unlockResource();
			break;
		}
		poolIdA = bodyNode.next;
	}

	// disconnect A from B
	PfxUInt32 poolIdB = m_bodyHeads[rigidbodyIdB].headId;
	while (poolIdB != (PfxUInt32)-1) {
		BodyNode &bodyNode = m_bodyPool[poolIdB];

		if (bodyNode.rigidbodyId == rigidbodyIdA) {
			if (bodyNode.prev == (PfxUInt32)-1 && bodyNode.next != (PfxUInt32)-1) {
				m_bodyHeads[rigidbodyIdB].headId = bodyNode.next;
				m_bodyPool[bodyNode.next].prev = (PfxUInt32)-1;
			}
			else if (bodyNode.prev != (PfxUInt32)-1 && bodyNode.next == (PfxUInt32)-1) {
				m_bodyPool[bodyNode.prev].next = (PfxUInt32)-1;
			}
			else if (bodyNode.prev != (PfxUInt32)-1 && bodyNode.next != (PfxUInt32)-1) {
				m_bodyPool[bodyNode.prev].next = bodyNode.next;
				m_bodyPool[bodyNode.next].prev = bodyNode.prev;
			}
			else {
				m_bodyHeads[rigidbodyIdB].headId = (PfxUInt32)-1;
			}
			m_bodyHeads[rigidbodyIdB].numLinks--;
			lockResource();
			m_bodyPool.remove(poolIdB);
			unlockResource();
			break;
		}
		poolIdB = bodyNode.next;
	}
}

PfxUInt32 PfxPairContainer::queryBytes(PfxUInt32 maxPairs, PfxUInt32 maxRigidBodies)
{
	PfxUInt32 bytes = 0;
	bytes += PfxPoolBuffer<PairNode>::queryBytes(maxPairs);
	bytes += PfxPoolBuffer<BodyNode>::queryBytes(maxPairs * 2);
	bytes += sizeof(BodyHead) * maxRigidBodies;
	bytes += sizeof(std::atomic<PfxInt8>) * maxRigidBodies;
	return bytes;
}

bool PfxPairContainer::initialize(PfxUInt32 maxPairs, PfxUInt32 maxRigidBodies, void *buffer, PfxUInt32 bytes)
{
	if(bytes < queryBytes(maxPairs, maxRigidBodies)) return false;

	uint8_t *p = (uint8_t*)buffer;
	PfxUInt32 sz1 = m_pairPool.queryBytes(maxPairs);
	m_pairPool.initialize(p, sz1, maxPairs);
	p += sz1;
	PfxUInt32 sz2 = m_bodyPool.queryBytes(maxPairs * 2);
	m_bodyPool.initialize(p, sz2, maxPairs * 2);
	p += sz2;
	m_bodyHeads = (BodyHead*)p;
	p += sizeof(BodyHead) * maxRigidBodies;
	m_bodyLock = (std::atomic<PfxInt8>*)p;

	m_maxPairs = maxPairs;
	m_maxRigidBodies = maxRigidBodies;

	clear();
	
	return true;
}

void PfxPairContainer::clear()
{
	m_pairPool.clear();
	m_bodyPool.clear();

	for (int i = 0; i < m_maxRigidBodies; i++) {
		m_bodyLock[i] = 0;
		m_bodyHeads[i].headId = (PfxUInt32)-1;
		m_bodyHeads[i].numLinks = 0;
	}

	m_resourceLock = 0;
}

void PfxPairContainer::verify()
{
	for (PfxUInt32 i = 0; i < m_pairPool.length(); i++) {
		if (m_pairPool[i].poolId != (PfxUInt32)-1) {
			PfxUInt32 rbA = m_pairPool[i].rigidbodyIdA;
			PfxUInt32 rbB = m_pairPool[i].rigidbodyIdB;
			PfxBool rbA_exist = false, rbB_exist = false;

			if (m_bodyHeads[rbA].headId != (PfxUInt32)-1) {
				PfxUInt32 poolId = m_bodyHeads[rbA].headId;
				while (poolId != (PfxUInt32)-1) {
					BodyNode &bodyNode = m_bodyPool[poolId];
					if (bodyNode.rigidbodyId == rbB) {
						rbA_exist = true;
						break;
					}
					poolId = bodyNode.next;
				}
			}
			else {
				SCE_PFX_PRINTF("can't find bodyHead %d\n", rbA);
			}

			if (m_bodyHeads[rbB].headId != (PfxUInt32)-1) {
				PfxUInt32 poolId = m_bodyHeads[rbB].headId;
				while (poolId != (PfxUInt32)-1) {
					BodyNode &bodyNode = m_bodyPool[poolId];
					if (bodyNode.rigidbodyId == rbA) {
						rbB_exist = true;
						break;
					}
					poolId = bodyNode.next;
				}

			}
			else {
				SCE_PFX_PRINTF("can't find bodyHead %d\n", rbB);
			}
			
			if (!rbA_exist) {
				SCE_PFX_PRINTF("can't find pair %d %d in bodyList %d\n", rbA, rbB, rbA);
			}
			if (!rbB_exist) {
				SCE_PFX_PRINTF("can't find pair %d %d in bodyList %d\n", rbA, rbB, rbB);
			}

			if (m_pairPool[i].state == 2 && m_pairPool[i].data == (PfxUInt32)-1) {
				SCE_PFX_PRINTF("Existing pair %d %d doesn't have valid contactId\n", rbA, rbB);
			}
		}
	}
	SCE_PFX_PRINTF("finish verify\n");
}

void PfxPairContainer::print()
{
	for (PfxUInt32 i = 0; i < m_maxRigidBodies; i++) {
		if (m_bodyHeads[i].headId != (PfxUInt32)-1) {
			SCE_PFX_PRINTF("RB%d links %d : ",i, m_bodyHeads[i].numLinks);
			PfxUInt32 poolId = m_bodyHeads[i].headId;
			while (poolId != (PfxUInt32)-1) {
				BodyNode &bodyNode = m_bodyPool[poolId];
				SCE_PFX_PRINTF(" %d", bodyNode.rigidbodyId);
				poolId = bodyNode.next;
			}
			SCE_PFX_PRINTF("\n");
		}
	}

	for (PfxUInt32 i = 0; i < m_pairPool.length(); i++) {
		if (m_pairPool[i].state != NODE_IS_NOT_ASSIGNED) {
			SCE_PFX_PRINTF("pairId %d state %d data %d RB %d %d\n", i, m_pairPool[i].state, m_pairPool[i].data, m_pairPool[i].rigidbodyIdA, m_pairPool[i].rigidbodyIdB);
		}
	}
}

PfxPairContainer::eResult PfxPairContainer::tryInsertPair(const PfxBroadphasePair &pair, PfxUInt32 &pairId)
{
	PfxUInt32 rigidbodyIdA, rigidbodyIdB;
	pfxDecodeKey(pfxGetKey(pair), rigidbodyIdA, rigidbodyIdB);

	SCE_PFX_ASSERT(rigidbodyIdA != rigidbodyIdB);

	if(m_pairPool.size() == m_pairPool.capacity())
	{
		std::atomic_thread_fence(std::memory_order_acquire);
		return kOverflowOfPairs; // overflow
	}
	if(rigidbodyIdA >= m_maxRigidBodies || rigidbodyIdB >= m_maxRigidBodies) return kOverflowOfBodies;

	if (tryLock(rigidbodyIdA, rigidbodyIdB)) {
		eResult result = connect(rigidbodyIdA, rigidbodyIdB, pairId);

		m_pairPool[pairId].solverQuality = pfxGetSolverQuality(pair);
		m_pairPool[pairId].motionMaskA = pfxGetMotionMaskA(pair);
		m_pairPool[pairId].motionMaskB = pfxGetMotionMaskB(pair);

		unlock(rigidbodyIdA, rigidbodyIdB);

		return result;
	}

	return kFailedToLockPair;
}

PfxPairContainer::eResult PfxPairContainer::tryRemovePair(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB)
{
	if(rigidbodyIdA >= m_maxRigidBodies || rigidbodyIdB >= m_maxRigidBodies) return kOverflowOfBodies;

	if (tryLock(rigidbodyIdA, rigidbodyIdB)) {
		disconnect(rigidbodyIdA, rigidbodyIdB);

		unlock(rigidbodyIdA, rigidbodyIdB);

		return kSuccess;
	}

	return kFailedToLockPair;
}

PfxPairContainer::eResult PfxPairContainer::setPairData(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB, PfxUInt32 data)
{
	if(rigidbodyIdA >= m_maxRigidBodies || rigidbodyIdB >= m_maxRigidBodies) return kInvalid;

	if (m_bodyHeads[rigidbodyIdA].numLinks < m_bodyHeads[rigidbodyIdB].numLinks) {
		PfxUInt32 poolId = m_bodyHeads[rigidbodyIdA].headId;
		while (poolId != (PfxUInt32)-1) {
			BodyNode &bodyNode = m_bodyPool[poolId];
			if (bodyNode.rigidbodyId == rigidbodyIdB) {
				m_pairPool[bodyNode.pairId].data = data;
				return kSuccess;
			}
			poolId = bodyNode.next;
		}
	}
	else {
		PfxUInt32 poolId = m_bodyHeads[rigidbodyIdB].headId;
		while (poolId != (PfxUInt32)-1) {
			BodyNode &bodyNode = m_bodyPool[poolId];
			if (bodyNode.rigidbodyId == rigidbodyIdA) {
				m_pairPool[bodyNode.pairId].data = data;
				return kSuccess;
			}
			poolId = bodyNode.next;
		}
	}
	return kInvalid;
}

PfxPairContainer::eResult PfxPairContainer::getPairData(PfxUInt16 rigidbodyIdA, PfxUInt16 rigidbodyIdB, PfxUInt32 &data)
{
	if(rigidbodyIdA >= m_maxRigidBodies || rigidbodyIdB >= m_maxRigidBodies) return kInvalid;

	if (m_bodyHeads[rigidbodyIdA].numLinks < m_bodyHeads[rigidbodyIdB].numLinks) {
		PfxUInt32 poolId = m_bodyHeads[rigidbodyIdA].headId;
		while (poolId != (PfxUInt32)-1) {
			BodyNode &bodyNode = m_bodyPool[poolId];
			if (bodyNode.rigidbodyId == rigidbodyIdB) {
				data = m_pairPool[bodyNode.pairId].data;
				return kSuccess;
			}
			poolId = bodyNode.next;
		}
	}
	else {
		PfxUInt32 poolId = m_bodyHeads[rigidbodyIdB].headId;
		while (poolId != (PfxUInt32)-1) {
			BodyNode &bodyNode = m_bodyPool[poolId];
			if (bodyNode.rigidbodyId == rigidbodyIdA) {
				data = m_pairPool[bodyNode.pairId].data;
				return kSuccess;
			}
			poolId = bodyNode.next;
		}
	}
	return kInvalid;
}

const PfxPairContainer::BodyNode *PfxPairContainer::getFirstRigidBody(PfxUInt16 rigidbodyId) const
{
	if(rigidbodyId >= m_maxRigidBodies) return nullptr;
	PfxUInt32 poolId = m_bodyHeads[rigidbodyId].headId;
	if (poolId != (PfxUInt32)-1) {
		return &m_bodyPool[poolId];
	}
	return nullptr;
}

const PfxPairContainer::BodyNode *PfxPairContainer::getNextRigidBody(const BodyNode *node) const
{
	if (!node) return nullptr;

	PfxUInt32 poolId = node->next;
	if (poolId != (PfxUInt32)-1) {
		return &m_bodyPool[poolId];
	}
	return nullptr;
}

PfxUInt32 PfxPairContainer::querySerializeBytes()
{
	PfxUInt32 bytes = 0;

	// pair nodes
	bytes += 24;
	bytes += sizeof(PairNode)*m_pairPool.m_length;
	bytes += sizeof(PfxUInt32)*m_pairPool.m_reusableIdCount;

	// body nodes
	bytes += 24;
	bytes += sizeof(BodyNode)*m_bodyPool.m_length;
	bytes += sizeof(PfxUInt32)*m_bodyPool.m_reusableIdCount;

	// body heads
	bytes += 4;
	bytes += sizeof(BodyHead)*m_maxRigidBodies;

	return bytes;
}

PfxInt32 PfxPairContainer::save(PfxUInt8 *buffer, PfxUInt32 bytes)
{
	PfxUInt32 checkBytes = querySerializeBytes();
	if (bytes < checkBytes) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxUInt8 *p = buffer;

	// pair nodes
	writeUInt32(&p, m_pairPool.m_reusableIdCount);
	writeUInt32(&p, m_pairPool.m_size);
	writeUInt32(&p, m_pairPool.capacity());
	writeUInt32(&p, m_pairPool.m_length);
	writeUInt64(&p, (uintptr_t)m_pairPool.m_data);
	writeUInt8Array(&p, (PfxUInt8*)m_pairPool.m_data, sizeof(PairNode)*m_pairPool.m_length);
	writeUInt8Array(&p, (PfxUInt8*)m_pairPool.m_reusableId, sizeof(PfxUInt32)*m_pairPool.m_reusableIdCount);

	// body nodes
	writeUInt32(&p, m_bodyPool.m_reusableIdCount);
	writeUInt32(&p, m_bodyPool.m_size);
	writeUInt32(&p, m_bodyPool.capacity());
	writeUInt32(&p, m_bodyPool.m_length);
	writeUInt64(&p, (uintptr_t)m_bodyPool.m_data);
	writeUInt8Array(&p, (PfxUInt8*)m_bodyPool.m_data, sizeof(BodyNode)*m_bodyPool.m_length);
	writeUInt8Array(&p, (PfxUInt8*)m_bodyPool.m_reusableId, sizeof(PfxUInt32)*m_bodyPool.m_reusableIdCount);

	// body heads
	writeUInt32(&p, m_maxRigidBodies);
	writeUInt8Array(&p, (PfxUInt8*)m_bodyHeads, sizeof(BodyHead)*m_maxRigidBodies);

	return SCE_PFX_OK;
}

PfxInt32 PfxPairContainer::load(const PfxUInt8 *buffer, PfxUInt32 bytes)
{
	clear();

	const PfxUInt8 *p = buffer;

	// pair nodes
	{
		PfxUInt32 reusableIdCount = 0;
		PfxUInt32 size = 0;
		PfxUInt32 capacity = 0;
		PfxUInt32 length = 0;
		PfxUInt64 pointerOffset = 0;

		readUInt32(&p, reusableIdCount);
		readUInt32(&p, size);
		readUInt32(&p, capacity);
		readUInt32(&p, length);
		if(capacity != m_pairPool.capacity()) {
			return SCE_PFX_ERR_OUT_OF_BUFFER;
		}

		m_pairPool.clear();
		m_pairPool.m_reusableIdCount = reusableIdCount;
		m_pairPool.m_size = size;
		m_pairPool.m_length = length;

		readUInt64(&p, pointerOffset);
		readUInt8Array(&p, (PfxUInt8*)m_pairPool.m_data, sizeof(PairNode)*m_pairPool.m_length);
		readUInt8Array(&p, (PfxUInt8*)m_pairPool.m_reusableId, sizeof(PfxUInt32)*m_pairPool.m_reusableIdCount);
	}

	// body nodes
	{
		PfxUInt32 reusableIdCount = 0;
		PfxUInt32 size = 0;
		PfxUInt32 capacity = 0;
		PfxUInt32 length = 0;
		PfxUInt64 pointerOffset = 0;

		readUInt32(&p, reusableIdCount);
		readUInt32(&p, size);
		readUInt32(&p, capacity);
		readUInt32(&p, length);
		if(capacity != m_bodyPool.capacity()) {
			return SCE_PFX_ERR_OUT_OF_BUFFER;
		}

		m_bodyPool.clear();
		m_bodyPool.m_reusableIdCount = reusableIdCount;
		m_bodyPool.m_size = size;
		m_bodyPool.m_length = length;

		readUInt64(&p, pointerOffset);
		readUInt8Array(&p, (PfxUInt8*)m_bodyPool.m_data, sizeof(BodyNode)*m_bodyPool.m_length);
		readUInt8Array(&p, (PfxUInt8*)m_bodyPool.m_reusableId, sizeof(PfxUInt32)*m_bodyPool.m_reusableIdCount);
	}

	// body heads
	{
		PfxUInt32 numBodies = 0;
		readUInt32(&p, numBodies);

		if(numBodies > m_maxRigidBodies) {
			return SCE_PFX_ERR_OUT_OF_BUFFER;
		}

		readUInt8Array(&p, (PfxUInt8*)m_bodyHeads, sizeof(BodyHead)*numBodies);
	}

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce

