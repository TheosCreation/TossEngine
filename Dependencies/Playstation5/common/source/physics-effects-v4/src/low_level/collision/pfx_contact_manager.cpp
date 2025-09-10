/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_contact_manager.h"
#include "../../util/pfx_binary_reader_writer.h"

namespace sce {
namespace pfxv4 {

PfxInt32 PfxContactManager::initialize(PfxUInt32 maxContacts, void *workBuff, PfxUInt32 workBytes)
{
	if (workBytes < queryBytes(maxContacts)) {
		return SCE_PFX_ERR_OUT_OF_BUFFER;
	}

	m_pool.initialize((PfxUInt8*)workBuff, workBytes);
	m_maxContacts = maxContacts;
	m_contactManifolds.initialize(&m_pool, maxContacts);
	m_contactHolders.initialize(&m_pool, maxContacts);
	PfxUInt32 additionalBytes = workBytes - queryBytes(maxContacts);
	m_maxClosestPoints = additionalBytes / sizeof(PfxClosestPoint);
	if (m_maxClosestPoints > 0) {
		m_closestPoints = (PfxClosestPoint*)m_pool.allocate(sizeof(PfxClosestPoint) * m_maxClosestPoints);
	}
	else {
		m_closestPoints = NULL;
	}

	clear();

	return SCE_PFX_OK;
}

void PfxContactManager::finalize()
{
	m_pool.clear();
}

void PfxContactManager::clear()
{
	m_numClosestPoints = 0;
	m_contactManifolds.clear();
	m_contactHolders.clear();
}

PfxInt32 PfxContactManager::createContactManifold(PfxUInt32 &contactManifoldId, PfxUInt8 shapeIdA, PfxUInt8 shapeIdB)
{
	if (m_contactManifolds.size() >= m_maxContacts /*|| (m_contactPoints.size() + 4) > m_maxContacts * 4*/) {
		return SCE_PFX_ERR_OUT_OF_BUFFER;
	}

	PfxContactManifold contactManifold;
	contactManifold.reset(shapeIdA, shapeIdB);

	contactManifold.m_contactPoints[0].reset();
	contactManifold.m_contactPoints[1].reset();
	contactManifold.m_contactPoints[2].reset();
	contactManifold.m_contactPoints[3].reset();

	contactManifoldId = m_contactManifolds.push(contactManifold);

	return SCE_PFX_OK;
}

PfxInt32 PfxContactManager::removeContactManifold(PfxUInt32 contactManifoldId)
{
	m_contactManifolds.remove(contactManifoldId);
	return SCE_PFX_OK;
}

PfxInt32 PfxContactManager::createContactHolder(PfxUInt32 &contactId, PfxUInt32 iA, PfxUInt32 iB)
{
	if (m_contactHolders.size() >= m_maxContacts) {
		return SCE_PFX_ERR_OUT_OF_MAX_CONTACTS;
	}

	PfxUInt32 contactManifoldId = 0;
	PfxInt32 err = createContactManifold(contactManifoldId, 0, 0);
	if (err != SCE_PFX_OK) return err;

	PfxContactHolder contactHolder;
	contactHolder.reset(iA, iB);
	contactHolder.m_contactManifolds = &m_contactManifolds[contactManifoldId];
	contactId = m_contactHolders.push(contactHolder);

	return SCE_PFX_OK;
}

PfxInt32 PfxContactManager::removeContactHolder(PfxUInt32 contactId)
{
	if (contactId >= m_contactHolders.length()) return SCE_PFX_ERR_OUT_OF_RANGE;
	PfxContactHolder &contactHolder = m_contactHolders[contactId];

	// traverse and release all contact manifolds
	PfxContactManifold *cm = contactHolder.m_contactManifolds;
	while (cm) {
		PfxContactManifold *tmp = cm;
		cm = cm->m_next;
		removeContactManifold((PfxUInt32)(tmp - &m_contactManifolds[0]));
	}

	m_contactHolders.remove(contactId);
	return SCE_PFX_OK;
}

PfxInt32 PfxContactManager::clearContactHolder(PfxUInt32 contactId)
{
	PfxContactHolder &contactHolder = m_contactHolders[contactId];

	// traverse and release all contact manifolds except for the first one
	PfxContactManifold *cm = contactHolder.m_contactManifolds->m_next;
	while (cm) {
		PfxContactManifold *tmp = cm;
		cm = cm->m_next;
		removeContactManifold((PfxUInt32)(tmp - &m_contactManifolds[0]));
	}

	contactHolder.m_contactManifolds->cachedAxis = PfxVector3::zero();
	contactHolder.m_contactManifolds->m_numContactPoints = 0;
	contactHolder.m_contactManifolds->m_duration = 0;
	contactHolder.m_contactManifolds->m_shapeId = 0;
	contactHolder.m_contactManifolds->m_numClosestPoints = 0;
	contactHolder.m_contactManifolds->m_closestPoints = nullptr;
	contactHolder.m_contactManifolds->m_next = nullptr;

	return SCE_PFX_OK;
}

PfxClosestPoint *PfxContactManager::createClosestPoints(int num)
{
	if (m_numClosestPoints + num > m_maxClosestPoints) {
		return NULL;
	}

	PfxUInt32 offset = m_numClosestPoints.fetch_add(num);

	PfxClosestPoint *p = m_closestPoints + offset;

	return p;
}

void PfxContactManager::clearAllClosestPoints()
{
	m_numClosestPoints = 0;
}

PfxContactManifold *PfxContactManager::insertContactManifoldToContactHolder(PfxContactHolder &contactHolder, PfxUInt16 shapeId)
{
	if (shapeId == 0) return contactHolder.m_contactManifolds;

	// todo:max check
	PfxContactManifold *prev = contactHolder.m_contactManifolds;
	PfxContactManifold *curr = prev;
	while (curr) {
		if (shapeId == curr->m_shapeId) {
			return curr;
		}
		else if (shapeId < curr->m_shapeId) {
			PfxUInt32 contactManifoldId = 0;
			PfxInt32 err = createContactManifold(contactManifoldId, shapeId & 0xFF, shapeId >> 8);
			if (err != SCE_PFX_OK) return nullptr;
			m_contactManifolds[contactManifoldId].m_next = curr;
			prev->m_next = &m_contactManifolds[contactManifoldId];
			return &m_contactManifolds[contactManifoldId];
		}
		prev = curr;
		curr = curr->m_next;
	}

	// insert at the end
	{
		PfxUInt32 contactManifoldId = 0;
		PfxInt32 err = createContactManifold(contactManifoldId, shapeId & 0xFF, shapeId >> 8);
		if (err != SCE_PFX_OK) return nullptr;
		prev->m_next = &m_contactManifolds[contactManifoldId];
		return &m_contactManifolds[contactManifoldId];
	}
}

PfxBool PfxContactManager::removeContactManifoldFromContactHolder(PfxContactHolder &contactHolder, PfxUInt16 shapeId)
{
	if (shapeId == 0) return false;

	PfxContactManifold *prev = contactHolder.m_contactManifolds;
	PfxContactManifold *curr = prev;
	while (curr) {
		if (shapeId == curr->m_shapeId) {
			prev->m_next = curr->m_next;
			removeContactManifold((PfxUInt32)(curr - &m_contactManifolds[0]));
			return true;
		}
		prev = curr;
		curr = curr->m_next;
	}

	return false;
}

PfxBool PfxContactManager::removeEmptyContactManifoldFromContactHolder(PfxContactHolder &contactHolder)
{
	PfxContactManifold *prev = contactHolder.m_contactManifolds;
	PfxContactManifold *curr = contactHolder.m_contactManifolds->m_next;
	while (curr) {
		if (curr->isEmpty()) {
			prev->m_next = curr->m_next;
			int ret = removeContactManifold((PfxUInt32)(curr - &m_contactManifolds[0]));
			if (ret != SCE_PFX_OK) return false;
			curr = prev->m_next;
		}
		else {
			prev = curr;
			curr = curr->m_next;
		}
	}

	return true;
}

void PfxContactManager::print()
{
	SCE_PFX_PRINTF("contact manifolds %d/%d contact holders %d/%d\n",
		m_contactManifolds.size(), m_contactManifolds.capacity(),
		m_contactHolders.size(), m_contactHolders.capacity());
}

PfxUInt32 PfxContactManager::querySerializeBytes()
{
	PfxUInt32 bytes = 0;

	// closest buffer
	bytes += 12;
	bytes += sizeof(PfxClosestPoint)*m_numClosestPoints;

	// contact holder buffer
	bytes += 24;
	bytes += sizeof(PfxContactHolder)*m_contactHolders.m_length;
	bytes += sizeof(PfxUInt32)*m_contactHolders.m_reusableIdCount;
	bytes += sizeof(PfxUInt32)*((m_contactHolders.m_capacity + 31) >> 5);

	// contact manifold buffer
	bytes += 24;
	bytes += sizeof(PfxContactManifold)*m_contactManifolds.m_length;
	bytes += sizeof(PfxUInt32)*m_contactManifolds.m_reusableIdCount;
	bytes += sizeof(PfxUInt32)*((m_contactManifolds.m_capacity + 31) >> 5);

	return bytes;
}

PfxInt32 PfxContactManager::save(PfxUInt8 *buffer, PfxUInt32 bytes)
{
	PfxUInt32 checkBytes = querySerializeBytes();
	if (bytes < checkBytes) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxUInt8 *p = buffer;

	// closest points buffer
	writeUInt32(&p, m_numClosestPoints);
	writeUInt64(&p, (uintptr_t)m_closestPoints);
	writeUInt8Array(&p, (PfxUInt8*)m_closestPoints, sizeof(PfxClosestPoint)*m_numClosestPoints);

	// contact holder buffer
	writeUInt32(&p, m_contactHolders.m_reusableIdCount);
	writeUInt32(&p, m_contactHolders.m_size);
	writeUInt32(&p, m_contactHolders.m_capacity);
	writeUInt32(&p, m_contactHolders.m_length);
	writeUInt64(&p, (uintptr_t)m_contactHolders.m_data);
	writeUInt8Array(&p, (PfxUInt8*)m_contactHolders.m_data, sizeof(PfxContactHolder)*m_contactHolders.m_length);
	writeUInt8Array(&p, (PfxUInt8*)m_contactHolders.m_reusableId, sizeof(PfxUInt32)*m_contactHolders.m_reusableIdCount);
	writeUInt8Array(&p, (PfxUInt8*)m_contactHolders.m_poolTable, sizeof(PfxUInt32)*((m_contactHolders.m_capacity + 31) >> 5));

	// contact manifold buffer
	writeUInt32(&p, m_contactManifolds.m_reusableIdCount);
	writeUInt32(&p, m_contactManifolds.m_size);
	writeUInt32(&p, m_contactManifolds.m_capacity);
	writeUInt32(&p, m_contactManifolds.m_length);
	writeUInt64(&p, (uintptr_t)m_contactManifolds.m_data);
	writeUInt8Array(&p, (PfxUInt8*)m_contactManifolds.m_data, sizeof(PfxContactManifold)*m_contactManifolds.m_length);
	writeUInt8Array(&p, (PfxUInt8*)m_contactManifolds.m_reusableId, sizeof(PfxUInt32)*m_contactManifolds.m_reusableIdCount);
	writeUInt8Array(&p, (PfxUInt8*)m_contactManifolds.m_poolTable, sizeof(PfxUInt32)*((m_contactManifolds.m_capacity + 31) >> 5));

	return SCE_PFX_OK;
}

PfxInt32 PfxContactManager::load(const PfxUInt8 *buffer, PfxUInt32 bytes)
{
	clear();

	const PfxUInt8 *p = buffer;

	// closest points buffer
	PfxUInt32 numClosestPoints = 0;
	PfxUInt64 closestPointsPointerOffset = 0;
	readUInt32(&p, numClosestPoints);
	readUInt64(&p, closestPointsPointerOffset);
	if (numClosestPoints > m_maxClosestPoints) {
		return SCE_PFX_ERR_OUT_OF_BUFFER;
	}
	readUInt8Array(&p, (PfxUInt8*)m_closestPoints, sizeof(PfxClosestPoint)*numClosestPoints);
	m_numClosestPoints = numClosestPoints;

	// contact holder buffer
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
		if (capacity != m_contactHolders.m_capacity) {
			return SCE_PFX_ERR_OUT_OF_BUFFER;
		}

		m_contactHolders.clear();
		m_contactHolders.m_reusableIdCount = reusableIdCount;
		m_contactHolders.m_size = size;
		m_contactHolders.m_length = length;

		readUInt64(&p, pointerOffset);
		readUInt8Array(&p, (PfxUInt8*)m_contactHolders.m_data, sizeof(PfxContactHolder)*m_contactHolders.m_length);
		readUInt8Array(&p, (PfxUInt8*)m_contactHolders.m_reusableId, sizeof(PfxUInt32)*m_contactHolders.m_reusableIdCount);
		readUInt8Array(&p, (PfxUInt8*)m_contactHolders.m_poolTable, sizeof(PfxUInt32)*((m_contactHolders.m_capacity + 31) >> 5));
	}

	// contact manifold buffer
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
		if (capacity != m_contactManifolds.m_capacity) {
			return SCE_PFX_ERR_OUT_OF_BUFFER;
		}

		m_contactManifolds.clear();
		m_contactManifolds.m_reusableIdCount = reusableIdCount;
		m_contactManifolds.m_size = size;
		m_contactManifolds.m_length = length;

		readUInt64(&p, pointerOffset);
		readUInt8Array(&p, (PfxUInt8*)m_contactManifolds.m_data, sizeof(PfxContactManifold)*m_contactManifolds.m_length);
		readUInt8Array(&p, (PfxUInt8*)m_contactManifolds.m_reusableId, sizeof(PfxUInt32)*m_contactManifolds.m_reusableIdCount);
		readUInt8Array(&p, (PfxUInt8*)m_contactManifolds.m_poolTable, sizeof(PfxUInt32)*((m_contactManifolds.m_capacity + 31) >> 5));

		auto modifyPointer = [&](PfxContactManifold *p)
		{
			return p == nullptr ? p : (m_contactManifolds.m_data + (p - (PfxContactManifold*)pointerOffset));
		};

		// modify contact manifold's pointers stored in contact holders
		for (PfxUInt32 i = 0; i < m_contactHolders.length(); i++) {
			if (!m_contactHolders.isRemoved(i)) {
				PfxContactHolder &contactHolder = m_contactHolders[i];

				contactHolder.m_contactManifolds = modifyPointer(contactHolder.m_contactManifolds);
				PfxContactManifold *contactManifold = contactHolder.m_contactManifolds;
				while (contactManifold) {
					if (contactManifold->m_closestPoints != nullptr) {
						contactManifold->m_closestPoints = (m_closestPoints + (contactManifold->m_closestPoints - (PfxClosestPoint*)closestPointsPointerOffset));
					}
					contactManifold->m_next = modifyPointer(contactManifold->m_next);
					contactManifold = contactManifold->m_next;
				}
			}
		}
	}

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
