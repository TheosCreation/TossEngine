/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/low_level/rigidbody/pfx_compound_container.h"
#include "../../../include/physics_effects/util/pfx_mass.h"
#include "pfx_compound_container_impl.h"

#include "../../../include/physics_effects/base_level/collision/pfx_contact_manifold.h"
#include "../../base_level/solver/pfx_check_solver.h"

namespace sce {
namespace pfxv4 {

inline void translateSegment(const PfxRigidState &srcState, PfxRigidState &dstState)
{
	PfxLargePosition srcPos = srcState.getLargePosition();
	PfxLargePosition dstPos = dstState.getLargePosition();
	dstPos.changeSegment(srcPos.segment);
	dstState.setLargePosition(dstPos);
}

PfxUInt32 pfxCompoundContainerQueryMem(const PfxCompoundContainerInitParam &param)
{
	return param.maxStoredRigidBodies * sizeof(PfxCompoundChild) + param.maxPairs * sizeof(PfxCompoundPair) + 16;
}

PfxInt32 pfxCompoundContainerInit(PfxCompoundContainer &compoundContainer,const PfxCompoundContainerInitParam &param,void *workBuff,PfxUInt32 workBytes)
{
	SCE_PFX_ASSERT(sizeof(PfxCompoundContainerImpl) <= sizeof(PfxCompoundContainer));
	
	if(workBuff == NULL) return SCE_PFX_ERR_INVALID_VALUE;
	if(workBytes < pfxCompoundContainerQueryMem(param)) return SCE_PFX_ERR_OUT_OF_BUFFER;
	
	PfxCompoundContainerImpl *compoundContainerImpl = (PfxCompoundContainerImpl*)&compoundContainer;

	compoundContainerImpl->m_flags = param.flags;
	compoundContainerImpl->m_compoundId = (PfxUInt32)-1;
	compoundContainerImpl->m_numCompoundPairs = 0;
	compoundContainerImpl->m_numChilds = 0;
	
	uintptr_t buff = SCE_PFX_PTR_ALIGN16(workBuff);
	
	compoundContainerImpl->m_maxChilds = param.maxStoredRigidBodies;
	compoundContainerImpl->m_compoundChilds = (PfxCompoundChild*)buff;
	
	compoundContainerImpl->m_maxCompoundPairs = param.maxPairs;
	compoundContainerImpl->m_compoundPairs = (PfxCompoundPair*)(buff + sizeof(PfxCompoundChild) * compoundContainerImpl->m_maxChilds);
	
	return SCE_PFX_OK;
}

PfxInt32 pfxCompoundContainerTerm(PfxCompoundContainer &compoundContainer)
{
	return SCE_PFX_OK;
}

PfxInt32 pfxCompoundContainerReset(PfxCompoundContainer &compoundContainer)
{
	PfxCompoundContainerImpl *compoundContainerImpl = (PfxCompoundContainerImpl*)&compoundContainer;
	
	compoundContainerImpl->m_compoundId = (PfxUInt32)-1;
	compoundContainerImpl->m_numCompoundPairs = 0;
	compoundContainerImpl->m_numChilds = 0;
	
	return SCE_PFX_OK;
}

PfxUInt32 pfxCompoundContainerGetNumRigidBodies(PfxCompoundContainer &compoundContainer)
{
	PfxCompoundContainerImpl *compoundContainerImpl = (PfxCompoundContainerImpl*)&compoundContainer;

	return compoundContainerImpl->m_numChilds;
}

PfxUInt32 pfxCompoundContainerGetRigidBodyId(PfxCompoundContainer &compoundContainer, PfxUInt32 childId)
{
	PfxCompoundContainerImpl *compoundContainerImpl = (PfxCompoundContainerImpl*)&compoundContainer;
	
	if (childId < compoundContainerImpl->m_numChilds) {
		return compoundContainerImpl->m_compoundChilds[childId].rigidBodyId;
	}
	
	return -1;
}

PfxUInt32 pfxCompoundContainerFindRigidBodyId(PfxCompoundContainer &compoundContainer, PfxUInt32 rigidBodyId)
{
	PfxCompoundContainerImpl *compoundContainerImpl = (PfxCompoundContainerImpl*)&compoundContainer;
	
	PfxUInt32 childId = (PfxUInt32)-1;
	
	compoundContainerImpl->find(rigidBodyId, childId);

	return childId;
}

PfxUInt32 pfxCompoundContainerGetOriginBody(PfxCompoundContainer &compoundContainer)
{
	PfxCompoundContainerImpl *compoundContainerImpl = (PfxCompoundContainerImpl*)&compoundContainer;

	return compoundContainerImpl->m_compoundId;
}

PfxInt32 pfxCompoundContainerSetOriginBody(PfxCompoundContainer &compoundContainer, PfxUInt32 rigidBodyId)
{
	PfxCompoundContainerImpl *compoundContainerImpl = (PfxCompoundContainerImpl*)&compoundContainer;

	if (compoundContainerImpl->find(rigidBodyId)) {
		return SCE_PFX_ERR_INVALID_RIGID_BODY_INDEX;
	}

	compoundContainerImpl->m_compoundId = rigidBodyId;

	return SCE_PFX_OK;
}

PfxInt32 pfxCompoundContainerAttachBody(PfxCompoundContainer &compoundContainer, PfxUInt32 rigidBodyId)
{
	PfxCompoundContainerImpl *compoundContainerImpl = (PfxCompoundContainerImpl*)&compoundContainer;
	
	if (compoundContainerImpl->m_compoundId == rigidBodyId || compoundContainerImpl->find(rigidBodyId)) {
		return SCE_PFX_ERR_INVALID_RIGID_BODY_INDEX;
	}
	
	if (compoundContainerImpl->m_numChilds >= compoundContainerImpl->m_maxChilds) {
		return SCE_PFX_ERR_OUT_OF_BUFFER;
	}
	
	PfxUInt32 childId = compoundContainerImpl->m_numChilds++;
	PfxCompoundChild &child = compoundContainerImpl->m_compoundChilds[childId];
	child.rigidBodyId = rigidBodyId;
	
	return SCE_PFX_OK;
}

PfxInt32 pfxCompoundContainerDetachBody(PfxCompoundContainer &compoundContainer, PfxUInt32 rigidBodyId)
{
	PfxCompoundContainerImpl *compoundContainerImpl = (PfxCompoundContainerImpl*)&compoundContainer;
	
	if (compoundContainerImpl->m_compoundId == rigidBodyId) {
		return SCE_PFX_ERR_INVALID_RIGID_BODY_INDEX;
	}
	
	PfxUInt32 removeChildId = (PfxUInt32)-1;
	for (PfxUInt32 i = 0; i < compoundContainerImpl->m_numChilds; i++) {
		if (compoundContainerImpl->m_compoundChilds[i].rigidBodyId == rigidBodyId) {
			removeChildId = i;
			break;
		}
	}
	if(removeChildId == (PfxUInt32)-1) return SCE_PFX_ERR_INVALID_RIGID_BODY_INDEX;
	
	// Swap childs
	PfxUInt32 last = --compoundContainerImpl->m_numChilds;
	compoundContainerImpl->m_compoundChilds[removeChildId] = compoundContainerImpl->m_compoundChilds[last];
	
	return SCE_PFX_OK;
}

PfxInt32 pfxCompoundContainerFinish(PfxCompoundContainer &compoundContainer, PfxRigidState *states, PfxRigidBody *bodies, PfxUInt32 numRigidBodies)
{
	PfxCompoundContainerImpl *compoundContainerImpl = (PfxCompoundContainerImpl*)&compoundContainer;
	
	if(compoundContainerImpl->m_compoundId >= numRigidBodies) return SCE_PFX_ERR_INVALID_RIGID_BODY_INDEX;
	
	PfxRigidState &comState = states[compoundContainerImpl->m_compoundId];
	PfxRigidBody &comBody = bodies[compoundContainerImpl->m_compoundId];
	
	comState.setContactFilterSelf(0);
	comState.setContactFilterTarget(0);
	
	PfxTransform3 comTr(comState.getOrientation(), comState.getPosition());
	PfxTransform3 comTrInv = orthoInverse(comTr);
	
	// 子の相対位置をセットする
	for (PfxUInt32 i = 0; i < compoundContainerImpl->m_numChilds; i++) {
		PfxUInt32 rigidBodyId = compoundContainerImpl->m_compoundChilds[i].rigidBodyId;
		if(rigidBodyId >= numRigidBodies) return SCE_PFX_ERR_INVALID_RIGID_BODY_INDEX;

		PfxRigidState &childState = states[rigidBodyId];

		translateSegment(comState, childState);
		
		PfxTransform3 childTr(childState.getOrientation(), childState.getPosition());
		PfxTransform3 childToComTr = comTrInv * childTr;

		PfxCompoundChild &child = compoundContainerImpl->m_compoundChilds[i];
		child.offsetPosition = childToComTr.getTranslation();
		child.offsetOrientation = PfxQuat(childToComTr.getUpper3x3());
		child.offsetOrientation = normalize(child.offsetOrientation); // ここの計算を繰り返すと発散することがあるため防ぐ
	}

	// 慣性テンソルの合成＆重心位置を求める
	if ((compoundContainerImpl->m_flags & SCE_PFX_USE_ORIGIN_CENTER_OF_MASS) == 0) {
		PfxVector3 comPos(0.0f); // new center of mass
		PfxFloat totalMass = 0.0f;
		PfxMatrix3 totalInertia(0.0f);
		for (PfxUInt32 i = 0; i < compoundContainerImpl->m_numChilds; i++) {
			PfxCompoundChild &child = compoundContainerImpl->m_compoundChilds[i];
			PfxRigidBody &childBody = bodies[child.rigidBodyId];
			PfxMatrix3 childInertia = pfxMassRotate(childBody.getInertia(), PfxMatrix3(child.offsetOrientation));
			childInertia = pfxMassTranslate(childBody.getMass(), childInertia, child.offsetPosition);
			totalInertia += childInertia;
			comPos += childBody.getMass() * child.offsetPosition;
			totalMass += childBody.getMass();
		}
		comPos /= totalMass;
		comBody.setInertia(totalInertia);
		comBody.setMass(totalMass);

		// 剛体毎の相対関係を保持する
		for (PfxUInt32 i = 0; i < compoundContainerImpl->m_numChilds; i++) {
			PfxCompoundChild &child = compoundContainerImpl->m_compoundChilds[i];
			child.offsetPosition -= comPos;
		}

		comState.setPosition(comState.getPosition() + rotate(comState.getOrientation(), comPos));
	}

	return SCE_PFX_OK;
}

PfxInt32 pfxCompoundContainerAttachBodyImmediately(PfxCompoundContainer &compoundContainer, PfxUInt32 rigidBodyId, PfxRigidState *states, PfxRigidBody *bodies, PfxUInt32 numRigidBodies)
{
	PfxCompoundContainerImpl *compoundContainerImpl = (PfxCompoundContainerImpl*)&compoundContainer;
	
	PfxInt32 ret = pfxCompoundContainerAttachBody(compoundContainer, rigidBodyId);
	if(ret != SCE_PFX_OK) return ret;
	
	PfxRigidState &comState = states[compoundContainerImpl->m_compoundId];
	PfxRigidBody &comBody = bodies[compoundContainerImpl->m_compoundId];
	
	PfxRigidState &childState = states[rigidBodyId];
	PfxRigidBody &childBody = bodies[rigidBodyId];
	
	translateSegment(comState, childState);
	
	PfxTransform3 comTr(comState.getOrientation(), comState.getPosition());
	PfxTransform3 childTr(childState.getOrientation(), childState.getPosition());
	PfxTransform3 childToComTr = orthoInverse(comTr) * childTr;
	
	PfxVector3 childPos = childToComTr.getTranslation();
	PfxQuat childOri(childToComTr.getUpper3x3());
	
	PfxVector3 comPos(0.0f);
	
	if ((compoundContainerImpl->m_flags & SCE_PFX_USE_ORIGIN_CENTER_OF_MASS) == 0) {
		// 重心位置を再計算
		PfxFloat totalMass = comBody.getMass();
		totalMass += childBody.getMass();
		comPos = childBody.getMass() * childPos / totalMass;
		comBody.setMass(totalMass);
		
		// 慣性テンソルの合成
		
		// 元の慣性テンソルを新しい重心に移動
		PfxMatrix3 totalInertia = comBody.getInertia();
		totalInertia = pfxMassTranslate(comBody.getMass(), totalInertia, -comPos);
		
		PfxMatrix3 childInertia = pfxMassRotate(childBody.getInertia(), PfxMatrix3(childOri));
		childInertia = pfxMassTranslate(childBody.getMass(), childInertia, childPos - comPos);
		
		totalInertia += childInertia;
		comBody.setInertia(totalInertia);
		
		// 他の剛体の相対関係を再計算する
		for (PfxUInt32 i = 0; i < compoundContainerImpl->m_numChilds - 1; i++) {
			PfxCompoundChild &child = compoundContainerImpl->m_compoundChilds[i];
			child.offsetPosition -= comPos;
		}
		
		comState.setPosition(comState.getPosition() + rotate(comState.getOrientation(), comPos));
	}
	
	// 追加剛体の相対関係を保持する
	{
		PfxCompoundChild &child = compoundContainerImpl->m_compoundChilds[compoundContainerImpl->m_numChilds - 1];
		child.offsetPosition = childPos - comPos;
		child.offsetOrientation = childOri;
	}
	
	return SCE_PFX_OK;
}

PfxInt32 pfxCompoundContainerDetachBodyImmediately(PfxCompoundContainer &compoundContainer, PfxUInt32 rigidBodyId, PfxRigidState *states, PfxRigidBody *bodies, PfxUInt32 numRigidBodies)
{
	PfxCompoundContainerImpl *compoundContainerImpl = (PfxCompoundContainerImpl*)&compoundContainer;
	
	PfxInt32 ret = pfxCompoundContainerDetachBody(compoundContainer, rigidBodyId);
	if(ret != SCE_PFX_OK) return ret;
	
	if ((compoundContainerImpl->m_flags & SCE_PFX_USE_ORIGIN_CENTER_OF_MASS) == 0) {
		PfxRigidState &comState = states[compoundContainerImpl->m_compoundId];
		PfxRigidBody &comBody = bodies[compoundContainerImpl->m_compoundId];
		
		// 重心位置を求める
		PfxFloat totalMass = 0.0f;
		PfxVector3 comPos(0.0f);
		for (PfxUInt32 i = 0; i < compoundContainerImpl->m_numChilds; i++) {
			PfxCompoundChild &child = compoundContainerImpl->m_compoundChilds[i];
			PfxRigidBody &childBody = bodies[child.rigidBodyId];
			comPos += childBody.getMass() * child.offsetPosition;
			totalMass += childBody.getMass();
		}
		comPos /= totalMass;
		comBody.setMass(totalMass);
		
		// 剛体毎の相対関係を再計算
		for (PfxUInt32 i = 0; i < compoundContainerImpl->m_numChilds; i++) {
			PfxCompoundChild &child = compoundContainerImpl->m_compoundChilds[i];
			child.offsetPosition -= comPos;
		}
		
		// 慣性テンソルの合成
		PfxMatrix3 totalInertia(0.0f);
		for (PfxUInt32 i = 0; i < compoundContainerImpl->m_numChilds; i++) {
			PfxCompoundChild &child = compoundContainerImpl->m_compoundChilds[i];
			PfxRigidState &childState = states[child.rigidBodyId];
			PfxRigidBody &childBody = bodies[child.rigidBodyId];
			PfxMatrix3 childInertia = pfxMassRotate(childBody.getInertia(), PfxMatrix3(childState.getOrientation()));
			childInertia = pfxMassTranslate(childBody.getMass(), childInertia, child.offsetPosition);
			totalInertia += childInertia;
		}
		comBody.setInertia(totalInertia);
		
		comState.setPosition(comState.getPosition() + rotate(comState.getOrientation(), comPos));
	}
	
	return SCE_PFX_OK;
}

// Internal functions

void pfxDistributeBodies(PfxCompoundContainerImpl *compoundContainerImpl, PfxRigidState *states)
{
	PfxRigidState &comState = states[compoundContainerImpl->m_compoundId];
	PfxLargePosition comPos = comState.getLargePosition();
	
	// 複合剛体の位置と速度から各子剛体の位置と速度を計算し、与える
	for (PfxUInt32 i = 0; i < compoundContainerImpl->m_numChilds; i++) {
		PfxCompoundChild &child = compoundContainerImpl->m_compoundChilds[i];

		PfxRigidState &childState = states[child.rigidBodyId];

		PfxVector3 r = rotate(comState.getOrientation(), child.offsetPosition);

		// 位置
		PfxLargePosition pos(comPos.segment, comPos.offset + r);
		childState.setLargePosition(pos);
		childState.setOrientation(comState.getOrientation() * child.offsetOrientation);

		// 速度
		childState.setLinearVelocity(comState.getLinearVelocity() + cross(r, comState.getAngularVelocity()));
		childState.setAngularVelocity(comState.getAngularVelocity());
	}
}

void pfxModifyPairsForCompound(PfxCompoundContainerImpl *compoundContainerImpl, 
	PfxRigidState *states,
	PfxConstraintPair *contactPairs, PfxUInt32 numContactPairs, PfxContactManager *contactManager, 
	PfxConstraintPair *jointPairs, PfxUInt32 numJointPairs, PfxJoint *joints)
{
	PfxRigidState &comState = states[compoundContainerImpl->m_compoundId];

	PfxUInt32 numCompoundPairs = 0;

	// 衝突ペアの剛体IDを複合剛体IDに付け替える
	for (PfxUInt32 i = 0; i < numContactPairs && numCompoundPairs < compoundContainerImpl->m_maxCompoundPairs; i++) {
		PfxConstraintPair &pair = contactPairs[i];
		if (!pfxCheckSolver(pair)) {
			continue;
		}

		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);

		PfxBool findA = compoundContainerImpl->find(iA);
		PfxBool findB = compoundContainerImpl->find(iB);

		if (findA && findB) {
			// 同一複合体に両方の剛体が含まれる場合は、衝突を無効にする
			PfxContactHolder &contact = contactManager->getContactHolder(pfxGetConstraintId(pair));
			contact.reset(iA, iB);
			pfxSetNumConstraints(pair, 0);
		}
		else if (findA) {
			pfxSetObjectIdA(pair, compoundContainerImpl->m_compoundId);
			PfxCompoundPair &cpair = compoundContainerImpl->m_compoundPairs[numCompoundPairs++];
			cpair.type = PfxCompoundPair::kContact;
			cpair.pairId = i;
			cpair.originalId = iA;
		}
		else if (findB) {
			pfxSetObjectIdB(pair, compoundContainerImpl->m_compoundId);
			PfxCompoundPair &cpair = compoundContainerImpl->m_compoundPairs[numCompoundPairs++];
			cpair.type = PfxCompoundPair::kContact;
			cpair.pairId = i;
			cpair.originalId = iB;
		}
	}

	// ジョイントペアの剛体IDを複合剛体IDに付け替える
	for (PfxUInt32 i = 0; i < numJointPairs && numCompoundPairs < compoundContainerImpl->m_maxCompoundPairs; i++) {
		PfxConstraintPair &pair = jointPairs[i];
		if (!pfxGetActive(pair)) {
			continue;
		}

		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);

		PfxBool findA = compoundContainerImpl->find(iA);
		PfxBool findB = compoundContainerImpl->find(iB);

		if (findA && findB) {
			// 同一複合体に両方の剛体が含まれる場合は、ジョイントを無効にする
			PfxJoint &joint = joints[pfxGetConstraintId(pair)];
			joint.m_active = 0;
			pfxSetActive(pair, false);
		}
		else if (findA) {
			pfxSetObjectIdA(pair, compoundContainerImpl->m_compoundId);
			PfxCompoundPair &cpair = compoundContainerImpl->m_compoundPairs[numCompoundPairs++];
			cpair.type = PfxCompoundPair::kJoint;
			cpair.pairId = i;
			cpair.originalId = iA;
		}
		else if (findB) {
			pfxSetObjectIdB(pair, compoundContainerImpl->m_compoundId);
			PfxCompoundPair &cpair = compoundContainerImpl->m_compoundPairs[numCompoundPairs++];
			cpair.type = PfxCompoundPair::kJoint;
			cpair.pairId = i;
			cpair.originalId = iB;
		}
	}

	// 衝突座標を複合剛体の座標系に変換する
	PfxTransform3 comTrInv = orthoInverse(PfxTransform3(comState.getOrientation(), comState.getPosition()));

	for (PfxUInt32 i = 0; i < numCompoundPairs; i++) {
		PfxCompoundPair &cpair = compoundContainerImpl->m_compoundPairs[i];

		// todo:毎回ここで計算するのは無駄
		PfxRigidState &childState = states[cpair.originalId];
		translateSegment(comState, childState); // 念のためセグメントを合わせておく
		PfxTransform3 tr(childState.getOrientation(), childState.getPosition());
		tr = comTrInv * tr;

		if (cpair.type == PfxCompoundPair::kContact) {
			PfxConstraintPair &pair = contactPairs[cpair.pairId];

			PfxContactHolder &contact = contactManager->getContactHolder(pfxGetConstraintId(pair));
			
			PfxContactManifold *contactManifold = contact.findFirstContactManifold();

			while (contactManifold) {
				for (PfxUInt32 j = 0; j < contactManifold->getNumContactPoints(); j++) {
					PfxContactPoint &cp = contactManifold->getContactPoint(j);

					if (pfxGetObjectIdA(pair) == compoundContainerImpl->m_compoundId) {
						cpair.contactInfo.localPoint[j][0] = cp.m_localPointA[0];
						cpair.contactInfo.localPoint[j][1] = cp.m_localPointA[1];
						cpair.contactInfo.localPoint[j][2] = cp.m_localPointA[2];

						PfxPoint3 cpoint = pfxReadPoint3(cp.m_localPointA);
						cpoint = tr * cpoint;
						cp.m_localPointA[0] = cpoint[0];
						cp.m_localPointA[1] = cpoint[1];
						cp.m_localPointA[2] = cpoint[2];
					}
					else if (pfxGetObjectIdB(pair) == compoundContainerImpl->m_compoundId) {
						cpair.contactInfo.localPoint[j][0] = cp.m_localPointB[0];
						cpair.contactInfo.localPoint[j][1] = cp.m_localPointB[1];
						cpair.contactInfo.localPoint[j][2] = cp.m_localPointB[2];

						PfxPoint3 cpoint = pfxReadPoint3(cp.m_localPointB);
						cpoint = tr * cpoint;
						cp.m_localPointB[0] = cpoint[0];
						cp.m_localPointB[1] = cpoint[1];
						cp.m_localPointB[2] = cpoint[2];
					}
				}

				contactManifold = contact.findNextContactManifold(contactManifold);
			}
		}
		else if (cpair.type == PfxCompoundPair::kJoint) {
			PfxConstraintPair &pair = jointPairs[cpair.pairId];

			PfxJoint &joint = joints[pfxGetConstraintId(pair)];

			if (pfxGetObjectIdA(pair) == compoundContainerImpl->m_compoundId) {
				cpair.jointInfo.anchor[0] = joint.m_anchorA[0];
				cpair.jointInfo.anchor[1] = joint.m_anchorA[1];
				cpair.jointInfo.anchor[2] = joint.m_anchorA[2];
				PfxQuat qframe(joint.m_frameA);
				cpair.jointInfo.frame[0] = qframe[0];
				cpair.jointInfo.frame[1] = qframe[1];
				cpair.jointInfo.frame[2] = qframe[2];
				cpair.jointInfo.frame[3] = qframe[3];

				joint.m_anchorA = PfxVector3(tr * PfxPoint3(joint.m_anchorA));
				joint.m_frameA = tr.getUpper3x3() * joint.m_frameA;
			}
			else if (pfxGetObjectIdB(pair) == compoundContainerImpl->m_compoundId) {
				cpair.jointInfo.anchor[0] = joint.m_anchorB[0];
				cpair.jointInfo.anchor[1] = joint.m_anchorB[1];
				cpair.jointInfo.anchor[2] = joint.m_anchorB[2];
				PfxQuat qframe(joint.m_frameB);
				cpair.jointInfo.frame[0] = qframe[0];
				cpair.jointInfo.frame[1] = qframe[1];
				cpair.jointInfo.frame[2] = qframe[2];
				cpair.jointInfo.frame[3] = qframe[3];

				joint.m_anchorB = PfxVector3(tr * PfxPoint3(joint.m_anchorB));
				joint.m_frameB = tr.getUpper3x3() * joint.m_frameB;
			}
		}
	}

	compoundContainerImpl->m_numCompoundPairs = numCompoundPairs;
}

struct SimpleMemoryStock
{
	void *m_buffer;
	PfxUInt64 m_bufferSize;
	PfxUInt64 m_usedBytes;

	PfxUInt64 m_usedByteStack[64];
	PfxUInt32 m_stackSize;

	inline void initialize(void *buffer, PfxUInt64 bufferSize){
		m_buffer = buffer;
		m_bufferSize = bufferSize;

		m_usedBytes = m_stackSize = 0lu;
	}

	template <typename T>
	T *alloc(PfxUInt32 arraySize){
		PfxUInt64 requiredSize = sizeof(T) * arraySize;
//		SCE_PFX_PRINTF("Available: %8d, stack %8d/%8d, allocate %6d = %4d * %4d\n",	m_bufferSize - m_usedBytes, m_usedBytes + requiredSize, m_bufferSize, requiredSize, sizeof(T), arraySize);

		if (m_usedBytes + requiredSize > m_bufferSize){
			SCE_PFX_ASSERT_MSG(false, "Memory not enough!!");
			return NULL;
		}

		T *ptr = (T*)(((char*)m_buffer) + m_usedBytes);
		m_usedByteStack[m_stackSize++] = m_usedBytes;
		m_usedBytes += requiredSize;
		return ptr;
	}

	void free(void *ptr){
//		SCE_PFX_PRINTF("Free, available %8d, stack %8d -> %8d/%8d\n", m_bufferSize - m_usedBytes, m_usedBytes, m_usedByteStack[m_stackSize - 1]);
		if (m_stackSize > 0)
			m_usedBytes = m_usedByteStack[--m_stackSize];

		SCE_PFX_ASSERT_MSG(ptr == (((char*)m_buffer) + m_usedBytes), "Warning!! Releasing wrong memory!!\n");
	}
};

void buildPairListToWhichRigidBodyBelongs(const PfxPairSimpleData pairDatem[], PfxUInt32 numPairDatem, PfxUInt32 numRigidBodies, PfxUInt32 *startPosPairDatemToWhichRigidBodyBelongs, PfxUInt32 *pairDataIdsToWhichRigidBodyBelongs)
{
	PfxUInt32 *endPosPairDatemToWhichRigidBodyBelongs = startPosPairDatemToWhichRigidBodyBelongs + 1;
	PfxUInt32 *accumPosPairDatemToWhichRigidBodyBelongs = endPosPairDatemToWhichRigidBodyBelongs + 1;

	for (PfxUInt32 i = 0; i < numRigidBodies; ++i)	accumPosPairDatemToWhichRigidBodyBelongs[i] = 0u;

	for (PfxUInt32 i = 0; i < numPairDatem; ++i)
	{
		const PfxPairSimpleData *pairData = pairDatem + i;
		++accumPosPairDatemToWhichRigidBodyBelongs[pairData->rbA];
		++accumPosPairDatemToWhichRigidBodyBelongs[pairData->rbB];
	}

	startPosPairDatemToWhichRigidBodyBelongs[0] = endPosPairDatemToWhichRigidBodyBelongs[0] = 0u;
	for (PfxUInt32 i = 0; i < numRigidBodies; ++i)	accumPosPairDatemToWhichRigidBodyBelongs[i] += endPosPairDatemToWhichRigidBodyBelongs[i];

	for (PfxUInt32 i = 0; i < numPairDatem; ++i)
	{
		const PfxPairSimpleData *pairData = pairDatem + i;
		pairDataIdsToWhichRigidBodyBelongs[endPosPairDatemToWhichRigidBodyBelongs[pairData->rbA]++] = i;
		pairDataIdsToWhichRigidBodyBelongs[endPosPairDatemToWhichRigidBodyBelongs[pairData->rbB]++] = i;
	}
}

void pfxModifyPairsForCompound(PfxCompoundContainerImpl *compoundContainerImpl,
	void *workBuff, PfxUInt32 workBytes,
	PfxRigidState *states, PfxUInt32 numRigidBodies,
	PfxPairSimpleData *contactPairDatem, PfxConstraintPair *contactPairs, PfxUInt32 numContactPairDatem, PfxContactManager *contactManager,
	PfxPairSimpleData *jointPairDatem, PfxConstraintPair *jointPairs, PfxUInt32 numJointPairDatem, PfxJoint *joints)
{
	SimpleMemoryStock memStock;
	memStock.initialize(workBuff, workBytes);
	
	PfxUInt32 *startPosPairDatemToWhichRigidBodyBelongs = memStock.alloc<PfxUInt32>(numRigidBodies + 2);
	PfxUInt32 *endPosPairDatemToWhichRigidBodyBelongs = startPosPairDatemToWhichRigidBodyBelongs + 1;
	PfxUInt32 *contactPairDataIdsToWhichRigidBodyBelongs = memStock.alloc<PfxUInt32>(numContactPairDatem);
	buildPairListToWhichRigidBodyBelongs(contactPairDatem, numContactPairDatem, numRigidBodies, startPosPairDatemToWhichRigidBodyBelongs, contactPairDataIdsToWhichRigidBodyBelongs);

	PfxRigidState &comState = states[compoundContainerImpl->m_compoundId];

	PfxUInt32 numCompoundPairs = 0;

	// 衝突ペアの剛体IDを複合剛体IDに付け替える
	for (PfxUInt32 i = 0; i < compoundContainerImpl->m_numChilds; i++) {
		PfxCompoundChild &child = compoundContainerImpl->m_compoundChilds[i];

		for (PfxUInt32 j = startPosPairDatemToWhichRigidBodyBelongs[child.rigidBodyId]; j < endPosPairDatemToWhichRigidBodyBelongs[child.rigidBodyId]; ++j){
			PfxPairSimpleData *pairData = contactPairDatem + contactPairDataIdsToWhichRigidBodyBelongs[j];
			PfxUInt32 pairId = pairData->pairId;
			PfxConstraintPair &pair = contactPairs[pairId];

			PfxUInt32 iA = pfxGetObjectIdA(pair);
			PfxUInt32 iB = pfxGetObjectIdB(pair);

			if (iA == child.rigidBodyId) {
				if (compoundContainerImpl->find(iB)) {
					// 同一複合体に両方の剛体が含まれる場合は、衝突を無効にする
					contactManager->clearContactHolder(pfxGetConstraintId(pair));
					pfxSetNumConstraints(pair, 0);
				}
				else {
					pfxSetObjectIdA(pair, compoundContainerImpl->m_compoundId);
					PfxCompoundPair &cpair = compoundContainerImpl->m_compoundPairs[numCompoundPairs++];
					cpair.type = PfxCompoundPair::kContact;
					cpair.pairId = pairId;
					cpair.originalId = iA;
				}
			}
			else if (iB == child.rigidBodyId) {
				if (compoundContainerImpl->find(iA)) {
					// 同一複合体に両方の剛体が含まれる場合は、衝突を無効にする
					contactManager->clearContactHolder(pfxGetConstraintId(pair));
					pfxSetNumConstraints(pair, 0);
				}
				else {
					pfxSetObjectIdB(pair, compoundContainerImpl->m_compoundId);
					PfxCompoundPair &cpair = compoundContainerImpl->m_compoundPairs[numCompoundPairs++];
					cpair.type = PfxCompoundPair::kContact;
					cpair.pairId = pairId;
					cpair.originalId = iB;
				}
			}
		}
	}

	memStock.free(contactPairDataIdsToWhichRigidBodyBelongs);
	PfxUInt32 *jointPairDataIdsToWhichRigidBodyBelongs = memStock.alloc<PfxUInt32>(numJointPairDatem);
	buildPairListToWhichRigidBodyBelongs(jointPairDatem, numJointPairDatem, numRigidBodies, startPosPairDatemToWhichRigidBodyBelongs, jointPairDataIdsToWhichRigidBodyBelongs);

	// ジョイントペアの剛体IDを複合剛体IDに付け替える
	for (PfxUInt32 i = 0; i < compoundContainerImpl->m_numChilds; i++) {
		PfxCompoundChild &child = compoundContainerImpl->m_compoundChilds[i];

		for (PfxUInt32 j = startPosPairDatemToWhichRigidBodyBelongs[child.rigidBodyId]; j < endPosPairDatemToWhichRigidBodyBelongs[child.rigidBodyId]; ++j)
		{
			PfxPairSimpleData *pairData = jointPairDatem + jointPairDataIdsToWhichRigidBodyBelongs[j];
			PfxUInt32 pairId = pairData->pairId;
			PfxConstraintPair &pair = jointPairs[pairId];

			if (!pfxGetActive(pair)) {
				continue;
			}

			PfxUInt32 iA = pfxGetObjectIdA(pair);
			PfxUInt32 iB = pfxGetObjectIdB(pair);

			if (iA == child.rigidBodyId) {
				if (compoundContainerImpl->find(iB)) {
					// 同一複合体に両方の剛体が含まれる場合は、ジョイントを無効にする
					PfxJoint &joint = joints[pfxGetConstraintId(pair)];
					joint.m_active = 0;
					pfxSetActive(pair, false);
				}
				else {
					pfxSetObjectIdA(pair, compoundContainerImpl->m_compoundId);
					PfxCompoundPair &cpair = compoundContainerImpl->m_compoundPairs[numCompoundPairs++];
					cpair.type = PfxCompoundPair::kJoint;
					cpair.pairId = pairId;
					cpair.originalId = iA;
				}
			}
			else if (iB == child.rigidBodyId) {
				if (compoundContainerImpl->find(iA)) {
					// 同一複合体に両方の剛体が含まれる場合は、ジョイントを無効にする
					PfxJoint &joint = joints[pfxGetConstraintId(pair)];
					joint.m_active = 0;
					pfxSetActive(pair, false);
				}
				else {
					pfxSetObjectIdB(pair, compoundContainerImpl->m_compoundId);
					PfxCompoundPair &cpair = compoundContainerImpl->m_compoundPairs[numCompoundPairs++];
					cpair.type = PfxCompoundPair::kJoint;
					cpair.pairId = pairId;
					cpair.originalId = iB;
				}
			}
		}
	}

	memStock.free(jointPairDataIdsToWhichRigidBodyBelongs);
	memStock.free(startPosPairDatemToWhichRigidBodyBelongs);

	// 衝突座標を複合剛体の座標系に変換する
	PfxTransform3 comTrInv = orthoInverse(PfxTransform3(comState.getOrientation(), comState.getPosition()));

	for (PfxUInt32 i = 0; i < numCompoundPairs; i++) {
		PfxCompoundPair &cpair = compoundContainerImpl->m_compoundPairs[i];

		// todo:毎回ここで計算するのは無駄
		PfxRigidState &childState = states[cpair.originalId];
		translateSegment(comState, childState); // 念のためセグメントを合わせておく
		PfxTransform3 tr(childState.getOrientation(), childState.getPosition());
		tr = comTrInv * tr;

		if (cpair.type == PfxCompoundPair::kContact) {
			PfxConstraintPair &pair = contactPairs[cpair.pairId];

			PfxContactHolder &contact = contactManager->getContactHolder(pfxGetConstraintId(pair));

			PfxContactManifold *contactManifold = contact.findFirstContactManifold();

			while (contactManifold) {
				for (PfxUInt32 j = 0; j < contactManifold->getNumContactPoints(); j++) {
					PfxContactPoint &cp = contactManifold->getContactPoint(j);

					if (pfxGetObjectIdA(pair) == compoundContainerImpl->m_compoundId) {
						cpair.contactInfo.localPoint[j][0] = cp.m_localPointA[0];
						cpair.contactInfo.localPoint[j][1] = cp.m_localPointA[1];
						cpair.contactInfo.localPoint[j][2] = cp.m_localPointA[2];

						PfxPoint3 cpoint = pfxReadPoint3(cp.m_localPointA);
						cpoint = tr * cpoint;
						cp.m_localPointA[0] = cpoint[0];
						cp.m_localPointA[1] = cpoint[1];
						cp.m_localPointA[2] = cpoint[2];
					}
					else if (pfxGetObjectIdB(pair) == compoundContainerImpl->m_compoundId) {
						cpair.contactInfo.localPoint[j][0] = cp.m_localPointB[0];
						cpair.contactInfo.localPoint[j][1] = cp.m_localPointB[1];
						cpair.contactInfo.localPoint[j][2] = cp.m_localPointB[2];

						PfxPoint3 cpoint = pfxReadPoint3(cp.m_localPointB);
						cpoint = tr * cpoint;
						cp.m_localPointB[0] = cpoint[0];
						cp.m_localPointB[1] = cpoint[1];
						cp.m_localPointB[2] = cpoint[2];
					}
				}
				contactManifold = contact.findNextContactManifold(contactManifold);
			}
		}
		else if (cpair.type == PfxCompoundPair::kJoint) {
			PfxConstraintPair &pair = jointPairs[cpair.pairId];

			PfxJoint &joint = joints[pfxGetConstraintId(pair)];

			if (pfxGetObjectIdA(pair) == compoundContainerImpl->m_compoundId) {
				cpair.jointInfo.anchor[0] = joint.m_anchorA[0];
				cpair.jointInfo.anchor[1] = joint.m_anchorA[1];
				cpair.jointInfo.anchor[2] = joint.m_anchorA[2];
				PfxQuat qframe(joint.m_frameA);
				cpair.jointInfo.frame[0] = qframe[0];
				cpair.jointInfo.frame[1] = qframe[1];
				cpair.jointInfo.frame[2] = qframe[2];
				cpair.jointInfo.frame[3] = qframe[3];
				joint.m_anchorA = PfxVector3(tr * PfxPoint3(joint.m_anchorA));
				joint.m_frameA = tr.getUpper3x3() * joint.m_frameA;
			}
			else if (pfxGetObjectIdB(pair) == compoundContainerImpl->m_compoundId) {
				cpair.jointInfo.anchor[0] = joint.m_anchorB[0];
				cpair.jointInfo.anchor[1] = joint.m_anchorB[1];
				cpair.jointInfo.anchor[2] = joint.m_anchorB[2];
				PfxQuat qframe(joint.m_frameB);
				cpair.jointInfo.frame[0] = qframe[0];
				cpair.jointInfo.frame[1] = qframe[1];
				cpair.jointInfo.frame[2] = qframe[2];
				cpair.jointInfo.frame[3] = qframe[3];

				joint.m_anchorB = PfxVector3(tr * PfxPoint3(joint.m_anchorB));
				joint.m_frameB = tr.getUpper3x3() * joint.m_frameB;
			}
		}
	}

	compoundContainerImpl->m_numCompoundPairs = numCompoundPairs;/**/
}


void pfxRestorePairsFromCompound(PfxCompoundContainerImpl *compoundContainerImpl,
	PfxConstraintPair *contactPairs, PfxUInt32 numContactPairs, PfxContactManager *contactManager,
	PfxConstraintPair *jointPairs, PfxUInt32 numJointPairs, PfxJoint *joints)
{
	// ペアの剛体IDを元に戻す
	// 衝突座標を元に戻す
	for (PfxUInt32 i = 0; i < compoundContainerImpl->m_numCompoundPairs; i++) {
		PfxCompoundPair &cpair = compoundContainerImpl->m_compoundPairs[i];

		if (cpair.type == PfxCompoundPair::kContact) {
			PfxConstraintPair &pair = contactPairs[cpair.pairId];

			bool isA = true;

			if (pfxGetObjectIdA(pair) == compoundContainerImpl->m_compoundId) {
				pfxSetObjectIdA(pair, cpair.originalId); // atomicに置き換える
				isA = true;
			}
			else if (pfxGetObjectIdB(pair) == compoundContainerImpl->m_compoundId) {
				pfxSetObjectIdB(pair, cpair.originalId); // atomicに置き換える
				isA = false;
			}

			PfxContactHolder &contact = contactManager->getContactHolder(pfxGetConstraintId(pair));

			PfxContactManifold *contactManifold = contact.findFirstContactManifold();

			while (contactManifold) {
				for (PfxUInt32 j = 0; j < contactManifold->getNumContactPoints(); j++) {
					PfxContactPoint &cp = contactManifold->getContactPoint(j);

					if (isA) {
						cp.m_localPointA[0] = cpair.contactInfo.localPoint[j][0];
						cp.m_localPointA[1] = cpair.contactInfo.localPoint[j][1];
						cp.m_localPointA[2] = cpair.contactInfo.localPoint[j][2];
					}
					else {
						cp.m_localPointB[0] = cpair.contactInfo.localPoint[j][0];
						cp.m_localPointB[1] = cpair.contactInfo.localPoint[j][1];
						cp.m_localPointB[2] = cpair.contactInfo.localPoint[j][2];
					}
				}
				contactManifold = contact.findNextContactManifold(contactManifold);
			}
		}
		else if (cpair.type == PfxCompoundPair::kJoint) {
			PfxConstraintPair &pair = jointPairs[cpair.pairId];

			PfxJoint &joint = joints[pfxGetConstraintId(pair)];

			if (pfxGetObjectIdA(pair) == compoundContainerImpl->m_compoundId) {
				pfxSetObjectIdA(pair, cpair.originalId); // atomicに置き換える
				joint.m_anchorA[0] = cpair.jointInfo.anchor[0];
				joint.m_anchorA[1] = cpair.jointInfo.anchor[1];
				joint.m_anchorA[2] = cpair.jointInfo.anchor[2];
				joint.m_frameA = PfxMatrix3(normalize(PfxQuat(cpair.jointInfo.frame[0], cpair.jointInfo.frame[1], cpair.jointInfo.frame[2], cpair.jointInfo.frame[3])));
			}
			else if (pfxGetObjectIdB(pair) == compoundContainerImpl->m_compoundId) {
				pfxSetObjectIdB(pair, cpair.originalId); // atomicに置き換える
				joint.m_anchorB[0] = cpair.jointInfo.anchor[0];
				joint.m_anchorB[1] = cpair.jointInfo.anchor[1];
				joint.m_anchorB[2] = cpair.jointInfo.anchor[2];
				joint.m_frameB = PfxMatrix3(normalize(PfxQuat(cpair.jointInfo.frame[0], cpair.jointInfo.frame[1], cpair.jointInfo.frame[2], cpair.jointInfo.frame[3])));
			}
		}
	}
}

void pfxPrintCompoundContainer(PfxCompoundContainerImpl *compoundContainerImpl, PfxRigidState *states, PfxRigidBody *bodies)
{
	PfxRigidState &comState = states[compoundContainerImpl->m_compoundId];
	PfxTransform3 comTr(comState.getOrientation(), comState.getPosition());

	PfxVector3 worldComPos = comState.getPosition();
	(void)worldComPos;
	
	SCE_PFX_PRINTF("%d worldComPos %.2f %.2f %.2f\n", compoundContainerImpl->m_compoundId, (float)worldComPos[0], (float)worldComPos[1], (float)worldComPos[2]);

	for (PfxUInt32 i = 0; i < compoundContainerImpl->m_numChilds; i++) {
		PfxCompoundChild &child = compoundContainerImpl->m_compoundChilds[i];

		PfxVector3 localChildPos = child.offsetPosition;
		PfxVector3 worldChildPos(comTr * PfxPoint3(localChildPos));
		SCE_PFX_PRINTF(" - %d childPos local %.2f %.2f %.2f world %.2f %.2f %.2f\n", child.rigidBodyId,
			(float)localChildPos[0], (float)localChildPos[1], (float)localChildPos[2],
			(float)worldChildPos[0], (float)worldChildPos[1], (float)worldChildPos[2]);
	}
}

} //namespace pfxv4
} //namespace sce
