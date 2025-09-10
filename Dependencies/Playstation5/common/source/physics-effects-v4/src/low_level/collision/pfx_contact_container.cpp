/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/low_level/collision/pfx_contact_container.h"
#include "../../../src/util/pfx_binary_reader_writer.h"
#include "../broadphase/pfx_pair_container.h"
#include "pfx_contact_complex.h"
#include "pfx_contact_manager.h"

#define SCE_PFX_CONTACT_CONTAINER_HEADER_SIZE 64

namespace sce {
namespace pfxv4 {

PfxUInt32 pfxContactContainerQueryMem(PfxUInt32 maxContacts,PfxUInt32 maxRigidBodies,PfxUInt32 additionalBytes)
{
	PfxUInt32 bytes = 0;
	bytes += PfxPairContainer::queryBytes(maxContacts, maxRigidBodies);
	bytes += PfxContactManager::queryBytes(maxContacts);
	bytes += additionalBytes;
	bytes += 16; // アライメント分
	return bytes;
}

PfxInt32 pfxContactContainerInit(PfxContactContainer &contactContainer,PfxUInt32 maxContacts,PfxUInt32 maxRigidBodies, void *workBuff,PfxUInt32 workBytes)
{
	SCE_PFX_ASSERT(sizeof(PfxContactComplex) <= sizeof(PfxContactContainer));
	
	PfxContactComplex *contactComplex = (PfxContactComplex*)&contactContainer;

	if(workBytes < pfxContactContainerQueryMem(maxContacts,maxRigidBodies,0)) {
		return SCE_PFX_ERR_OUT_OF_BUFFER;
	}

	PfxInt32 ret = SCE_PFX_OK;

	PfxUInt8 *p = (PfxUInt8*)workBuff;
	PfxUInt32 pairContainerBytes = PfxPairContainer::queryBytes(maxContacts, maxRigidBodies);

	PfxPairContainer &pairContainer = contactComplex->pairContainer;
	pairContainer.initialize(maxContacts, maxRigidBodies, p, pairContainerBytes);
	p += pairContainerBytes;

	PfxContactManager &contactManager = contactComplex->contactManager;
	ret = contactManager.initialize(maxContacts,p,((uintptr_t)workBuff + workBytes) - (uintptr_t)p);
	if(ret != SCE_PFX_OK) {
		return ret;
	}

	return SCE_PFX_OK;
}

PfxInt32 pfxContactContainerTerm(PfxContactContainer &contactContainer)
{
	PfxContactManager &contactManager = ((PfxContactComplex*)&contactContainer)->contactManager;
	contactManager.finalize();

	return SCE_PFX_OK;
}

PfxInt32 pfxContactContainerClear(PfxContactContainer &contactContainer)
{
	PfxContactManager &contactManager = ((PfxContactComplex*)&contactContainer)->contactManager;
	contactManager.clear();

	PfxPairContainer &pairContainer = ((PfxContactComplex*)&contactContainer)->pairContainer;
	pairContainer.clear();

	return SCE_PFX_OK;
}

PfxUInt32 pfxContactContainerGetNumContactPairs(const PfxContactContainer &contactContainer)
{
	PfxPairContainer &pairContainer = ((PfxContactComplex*)&contactContainer)->pairContainer;
	return pairContainer.getNumPairs();
}

PfxContactHolder *pfxContactContainerGetContactHolder(PfxContactContainer &contactContainer,PfxUInt32 rigidbodyIdA, PfxUInt32 rigidbodyIdB)
{
	PfxContactManager &contactManager = ((PfxContactComplex*)&contactContainer)->contactManager;
	PfxPairContainer &pairContainer = ((PfxContactComplex*)&contactContainer)->pairContainer;

	PfxUInt32 contactId = 0;
	if(PfxPairContainer::kSuccess != pairContainer.getPairData(rigidbodyIdA, rigidbodyIdB, contactId)) {
		return nullptr;
	}

	if (contactId < contactManager.getContactHolderCapacity()) {
		return &(contactManager.getContactHolder(contactId));
	}
	
	return nullptr;
}

const PfxContactHolder *pfxContactContainerGetContactHolder(const PfxContactContainer &contactContainer, PfxUInt32 rigidbodyIdA, PfxUInt32 rigidbodyIdB)
{
	PfxContactManager &contactManager = ((PfxContactComplex*)&contactContainer)->contactManager;
	PfxPairContainer &pairContainer = ((PfxContactComplex*)&contactContainer)->pairContainer;

	PfxUInt32 contactId = 0;
	if (PfxPairContainer::kSuccess != pairContainer.getPairData(rigidbodyIdA, rigidbodyIdB, contactId)) {
		return nullptr;
	}

	if (contactId < contactManager.getContactHolderCapacity()) {
		return &(contactManager.getContactHolder(contactId));
	}

	return nullptr;
}

PfxInt32 pfxContactContainerRemoveContact(const PfxContactContainer &contactContainer, PfxUInt32 rigidbodyIdA, PfxUInt32 rigidbodyIdB)
{
	PfxContactManager &contactManager = ((PfxContactComplex*)&contactContainer)->contactManager;
	PfxPairContainer &pairContainer = ((PfxContactComplex*)&contactContainer)->pairContainer;

	PfxUInt32 contactId = 0;
	if (PfxPairContainer::kSuccess == pairContainer.getPairData(rigidbodyIdA, rigidbodyIdB, contactId)) {
		PfxPairContainer::eResult result = PfxPairContainer::kSuccess;
		do {
			result = pairContainer.tryRemovePair(rigidbodyIdA, rigidbodyIdB);
		} while (result == PfxPairContainer::kFailedToLockPair);

		if (result == PfxPairContainer::kSuccess) {
			return contactManager.removeContactHolder(contactId);
		}
	}

	return SCE_PFX_ERR_INVALID_VALUE;
}

PfxInt32 pfxContactContainerRemoveAllRelatedContacts(const PfxContactContainer &contactContainer, PfxUInt32 rigidbodyId)
{
	PfxPairContainer &pairContainer = ((PfxContactComplex*)&contactContainer)->pairContainer;

	PfxUInt32 numRemoveIds = 0;
	PfxUInt32 removeIds[10] = { 0 };
	
	const PfxPairContainer::BodyNode *node = nullptr;
	do {
		numRemoveIds = 0;
		node = pairContainer.getFirstRigidBody(rigidbodyId);
		while (node && numRemoveIds < 10) {
			removeIds[numRemoveIds++] = node->rigidbodyId;
			node = pairContainer.getNextRigidBody(node);
		}

		for (int i = 0; i < numRemoveIds; i++) {
			PfxInt32 ret = pfxContactContainerRemoveContact(contactContainer, rigidbodyId, removeIds[i]);
			if (ret != SCE_PFX_OK) return ret;
		}
	} while (node);

	return SCE_PFX_OK;
}

void pfxContactContainerTraverse(const PfxContactContainer &contactContainer,pfxContactContainerTraverseCallback callback,void *userData)
{
	PfxContactManager &contactManager = ((PfxContactComplex*)&contactContainer)->contactManager;
	PfxPairContainer &pairContainer = ((PfxContactComplex*)&contactContainer)->pairContainer;

	PfxUInt32 numPairs = pairContainer.getPairLength();
	for(PfxUInt32 i=0;i<numPairs;i++) {
		const PfxPairContainer::PairNode &pair = pairContainer.getPairNode(i);
		if (pair.state == NODE_IS_NOT_ASSIGNED) continue;
		SCE_PFX_ASSERT(pair.data < pairContainer.getMaxPairs());
		SCE_PFX_ASSERT(pair.rigidbodyIdA < pairContainer.getMaxRigidBodies());
		SCE_PFX_ASSERT(pair.rigidbodyIdB < pairContainer.getMaxRigidBodies());

		PfxUInt32 iContact = pair.data;
		PfxUInt32 iA = pair.rigidbodyIdA;
		PfxUInt32 iB = pair.rigidbodyIdB;
		PfxContactHolder &contact = contactManager.getContactHolder(iContact);
		PfxBool ret = callback(contact,iA,iB,userData);
		if(!ret) break;
	}
}

void pfxContactContainerTraverse(const PfxContactContainer &contactContainer,PfxUInt32 rigidbodyId,pfxContactContainerTraverseCallback callback,void *userData)
{
	PfxContactManager &contactManager = ((PfxContactComplex*)&contactContainer)->contactManager;
	PfxPairContainer &pairContainer = ((PfxContactComplex*)&contactContainer)->pairContainer;

	const PfxPairContainer::BodyNode *node = pairContainer.getFirstRigidBody(rigidbodyId);
	while (node) {
		const PfxPairContainer::PairNode &pair = pairContainer.getPairNode(node->pairId);
		if (pair.state == NODE_IS_NOT_ASSIGNED) continue;
		SCE_PFX_ASSERT(pair.data < pairContainer.getMaxPairs());
		SCE_PFX_ASSERT(pair.rigidbodyIdA < pairContainer.getMaxRigidBodies());
		SCE_PFX_ASSERT(pair.rigidbodyIdB < pairContainer.getMaxRigidBodies());

		PfxUInt32 iContact = pair.data;
		PfxUInt32 iA = pair.rigidbodyIdA;
		PfxUInt32 iB = pair.rigidbodyIdB;
		PfxContactHolder &contact = contactManager.getContactHolder(iContact);

		PfxBool ret = callback(contact, iA, iB, userData);
		if (!ret) break;

		node = pairContainer.getNextRigidBody(node);
	}
}

PfxUInt32 pfxContactContainerQuerySerializeBytes(const PfxContactContainer &contactContainer)
{
	PfxContactManager &contactManager = ((PfxContactComplex*)&contactContainer)->contactManager;
	PfxPairContainer &pairContainer = ((PfxContactComplex*)&contactContainer)->pairContainer;
	
	return SCE_PFX_CONTACT_CONTAINER_HEADER_SIZE + 
		contactManager.querySerializeBytes() + 
		pairContainer.querySerializeBytes();
}

PfxInt32 pfxContactContainerWrite(const PfxContactContainer &contactContainer, PfxUInt8 *buffer, PfxUInt32 bytes)
{
	PfxContactManager &contactManager = ((PfxContactComplex*)&contactContainer)->contactManager;
	PfxPairContainer &pairContainer = ((PfxContactComplex*)&contactContainer)->pairContainer;

	if(!buffer) return SCE_PFX_ERR_INVALID_VALUE;
	if(bytes < pfxContactContainerQuerySerializeBytes(contactContainer)) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxUInt32 contactManagerBytes = contactManager.querySerializeBytes();
	PfxUInt32 pairContainerBytes = pairContainer.querySerializeBytes();
	PfxUInt32 contactCount = contactManager.getMaxContactPoints();
	PfxUInt32 pairCount = pairContainer.getMaxPairs();
	PfxUInt32 rigidBodyCount = pairContainer.getMaxRigidBodies();

	PfxUInt8 *p = buffer;

	writeUInt32(&p, contactManagerBytes);
	writeUInt32(&p, pairContainerBytes);
	writeUInt32(&p, contactCount);
	writeUInt32(&p, pairCount);
	writeUInt32(&p, rigidBodyCount);

	PfxUInt32 padding = (PfxUInt32)(SCE_PFX_CONTACT_CONTAINER_HEADER_SIZE - (p - buffer));
	p += padding;
	
	PfxInt32 ret = contactManager.save(p, contactManagerBytes);
	if(ret != SCE_PFX_OK) return ret;
	p += contactManagerBytes;

	ret = pairContainer.save(p, pairContainerBytes);
	if(ret != SCE_PFX_OK) return ret;

	return SCE_PFX_OK;
}

PfxInt32 pfxContactContainerRead(PfxContactContainer &contactContainer, const PfxUInt8 *buffer, PfxUInt32 bytes)
{
	PfxContactManager &contactManager = ((PfxContactComplex*)&contactContainer)->contactManager;
	PfxPairContainer &pairContainer = ((PfxContactComplex*)&contactContainer)->pairContainer;
	
	if (!buffer || bytes < SCE_PFX_CONTACT_CONTAINER_HEADER_SIZE) return SCE_PFX_ERR_INVALID_VALUE;

	const PfxUInt8 *p = buffer;

	PfxUInt32 contactManagerBytes = 0;
	PfxUInt32 pairContainerBytes = 0;
	PfxUInt32 contactCount = 0;
	PfxUInt32 pairCount = 0;
	PfxUInt32 rigidBodyCount = 0;

	readUInt32(&p, contactManagerBytes);
	readUInt32(&p, pairContainerBytes);
	readUInt32(&p, contactCount);
	readUInt32(&p, pairCount);
	readUInt32(&p, rigidBodyCount);

	if (bytes < contactManagerBytes + pairContainerBytes + SCE_PFX_CONTACT_CONTAINER_HEADER_SIZE) return SCE_PFX_ERR_INVALID_VALUE;

	PfxUInt32 padding = (PfxUInt32)(SCE_PFX_CONTACT_CONTAINER_HEADER_SIZE - (p - buffer));
	p += padding;

	PfxInt32 ret = contactManager.load(p, contactManagerBytes);
	if(ret != SCE_PFX_OK) return ret;
	p+= contactManagerBytes;

	ret = pairContainer.load(p, pairContainerBytes);
	if(ret != SCE_PFX_OK) return ret;
	p += pairContainerBytes;

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
