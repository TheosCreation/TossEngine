/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_detect_collision_stage.h"
#include "../task/pfx_job_system.h"
#include "../rigidbody/pfx_context.h"
#include "../../../src/base_level/broadphase/pfx_check_collidable.h"
#include "../../../include/physics_effects/base_level/collision/pfx_contact_cache.h"
#include "../../../include/physics_effects/base_level/collision/pfx_detect_collision_func.h"

#define SCE_PFX_CONTACT_THRESHOLD 0.0f
#define SCE_PFX_CCD_LINEAR_THRESHOLD 0.25f
#define SCE_PFX_CCD_ANGULAR_THRESHOLD 0.05f

#define SCE_PFX_ENABLE_SKIP_COLLISION
#define SCE_PFX_SKIP_COLLISION_THRESHOLD 0.5f

namespace sce {
namespace pfxv4 {

PfxBool PfxDetectCollisionStage::pfxPreCheckCcd(const PfxCoreSphere &coreSphereA,const PfxShape &shapeB,
	const PfxVector3 &posA0,const PfxVector3 &posA1,
	const PfxTransform3 &trB0,const PfxTransform3 &trB1)
{
	// calc A's ccd sphere
	PfxVector3 centerA = 0.5f * (posA0 + posA1);
	PfxFloat lenSqrA = lengthSqr(0.5f * (posA1 - posA0));
	PfxFloat radiusA = (lenSqrA < 0.00001f ? 0.0f : sqrtf(lenSqrA)) + coreSphereA.getRadius();

	// calc B's sphere
	PfxVector3 aabbMinB,aabbMaxB;
	shapeB.getAabb(aabbMinB,aabbMaxB);

	PfxVector3 centerB_ = 0.5f*(aabbMaxB + aabbMinB);
	PfxVector3 extentB_ = 0.5f*(aabbMaxB - aabbMinB);

	PfxVector3 centerB0 = trB0.getTranslation() + trB0.getUpper3x3() * centerB_; // shape center in the world coordinates
	PfxVector3 centerB1 = trB1.getTranslation() + trB1.getUpper3x3() * centerB_; // shape center in the world coordinates
	PfxVector3 centerB = 0.5f * (centerB0 + centerB1);
	PfxFloat lenSqrB = lengthSqr(0.5f * (centerB0 - centerB1));
	PfxFloat radiusB = (lenSqrB < 0.00001f ? 0.0f : sqrtf(lenSqrB)) + length(extentB_);

	// Skip CCD
#if 1
	PfxFloat radius = radiusA + radiusB;
	if(lengthSqr(centerA - centerB) > radius * radius) return false;
#else
	PfxVector3 s;
	pfxClosestPointLine(centerB,posA0,posA1-posA0,s);
	PfxFloat radius = coreSphereA.getRadius() + radiusB;
	if(lengthSqr(s - centerB) > radius * radius) return false;
#endif

	return true;
}

void PfxDetectCollisionStage::job_detectCollision(void *data, int uid)
{
	JobArg *arg = (JobArg*)data;

	PfxContext *context = (PfxContext*)arg->stage->getRigidBodyContext();

	PfxUInt32 total = arg->pairContainer->getPairLength();
	PfxUInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxUInt32 start = uid * batch;
	PfxUInt32 end = SCE_PFX_MIN(start + batch, total);

	auto filterPair = [](const PfxPairContainer::PairNode &pair)
	{
		PfxUInt32 motionTypeA = (pair.motionMaskA)&SCE_PFX_MOTION_MASK_TYPE;
		PfxUInt32 motionTypeB = (pair.motionMaskB)&SCE_PFX_MOTION_MASK_TYPE;
		PfxUInt32 sleepA = (pair.motionMaskA)&SCE_PFX_MOTION_MASK_SLEEPING;
		PfxUInt32 sleepB = (pair.motionMaskB)&SCE_PFX_MOTION_MASK_SLEEPING;

		return
			pfxCheckCollidableTable((ePfxMotionType)motionTypeA,(ePfxMotionType)motionTypeB) && // モーションタイプ別衝突判定テーブル
			!((sleepA != 0 && sleepB != 0) || (sleepA != 0 && motionTypeB == kPfxMotionTypeFixed) || (sleepB != 0 && motionTypeA == kPfxMotionTypeFixed)); // スリープ時のチェック
	};

	for (PfxUInt32 i = start; i < end; i++) {
		PfxPairContainer::PairNode &pair = arg->pairContainer->getPairNode(i);
		if (pair.state == NODE_IS_NOT_ASSIGNED) continue;

		if(!filterPair(pair)) {
			continue;
		}

		SCE_PFX_ASSERT( pair.data < arg->contactManager->getContactHolderCapacity() );

		PfxContactHolder &contactHolder = arg->contactManager->getContactHolder(pair.data);

		PfxRigidState &stateA = arg->stage->shared.states[pair.rigidbodyIdA];
		PfxRigidState &stateB = arg->stage->shared.states[pair.rigidbodyIdB];
		PfxCollidable &collA = arg->stage->shared.collidables[pair.rigidbodyIdA];
		PfxCollidable &collB = arg->stage->shared.collidables[pair.rigidbodyIdB];

		PfxFloat enegyA = lengthSqr(stateA.getLinearVelocity()) + lengthSqr(stateA.getAngularVelocity());
		PfxFloat enegyB = lengthSqr(stateB.getLinearVelocity()) + lengthSqr(stateB.getAngularVelocity());

		// If both rigid bodies' shift flags are 0, it means their nodes remain unchanged in BVH.
#ifdef SCE_PFX_ENABLE_SKIP_COLLISION
		if (stateA.getAllowSkippingNarrowPhaseDetection() && stateB.getAllowSkippingNarrowPhaseDetection() && // [SMS_CHANGE]
			!stateA.getProxyShiftFlag() && !stateB.getProxyShiftFlag() &&
			stateA.getMotionType() != kPfxMotionTypeTrigger && stateB.getMotionType() != kPfxMotionTypeTrigger &&
			enegyA < SCE_PFX_SKIP_COLLISION_THRESHOLD && enegyB < SCE_PFX_SKIP_COLLISION_THRESHOLD) {
			// If there isn't any movement, the previous contact manifold is used in this frame again.
			pair.numConstraints = SCE_PFX_MIN(0xFF, contactHolder.getTotalContactAndClosestPoints());
			contactHolder.setNarrowPhaseDetctionSkipped( true );// [SMS_CHANGE]
			continue;
		}
		else {
			contactHolder.setNarrowPhaseDetctionSkipped( false );// [SMS_CHANGE]
		}
#endif

		PfxLargePosition lposA = stateA.getLargePosition();
		PfxLargePosition lposB = stateB.getLargePosition();
		lposB.changeSegment(lposA.segment);

		PfxVector3 linearVelocityA = stateA.getLinearVelocity();
		PfxVector3 angularVelocityA = stateA.getAngularVelocity();
		PfxVector3 linearVelocityB = stateB.getLinearVelocity();
		PfxVector3 angularVelocityB = stateB.getAngularVelocity();

		PfxFloat linVelSqr = lengthSqr(linearVelocityA - linearVelocityB);
		PfxFloat angVelSqr = lengthSqr(angularVelocityA - angularVelocityB);

		PfxBool ccdEnable = (collA.isContinuous() || collB.isContinuous()) &&
			(linVelSqr > SCE_PFX_CCD_LINEAR_THRESHOLD || angVelSqr > SCE_PFX_CCD_ANGULAR_THRESHOLD);

		PfxTransform3 tA0(stateA.getOrientation(), lposA.offset);
		PfxTransform3 tA1 = ccdEnable?pfxIntegrateTransform(arg->stage->shared.timeStep,tA0,linearVelocityA,angularVelocityA):tA0;

		PfxTransform3 tB0(stateB.getOrientation(), lposB.offset);
		PfxTransform3 tB1 = ccdEnable?pfxIntegrateTransform(arg->stage->shared.timeStep,tB0,linearVelocityB,angularVelocityB):tB0;

		// Collision detection for shapes
		PfxContactManifold *contactManifold = contactHolder.findFirstContactManifold();

		#ifdef SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS

		while(contactManifold) {
			PfxContactCache contactCache;
			contactCache.reset();

			PfxUInt8 shapeIdA = contactManifold->getShapeIdA();
			PfxUInt8 shapeIdB = contactManifold->getShapeIdB();

			const PfxShape &shapeA = collA.getShape(shapeIdA);
			const PfxShape &shapeB = collB.getShape(shapeIdB);

			if (pfxCheckContactFilter(
				shapeA.getContactFilterSelf(), shapeA.getContactFilterTarget(),
				shapeB.getContactFilterSelf(), shapeB.getContactFilterTarget())) {

				PfxTransform3 offsetTrA = appendScale(shapeA.getOffsetTransform(), shapeA.getScaleXyz());
				PfxTransform3 worldTrA0 = tA0 * offsetTrA;
				PfxTransform3 worldTrA1 = tA1 * offsetTrA;

				PfxTransform3 offsetTrB = appendScale(shapeB.getOffsetTransform(), shapeB.getScaleXyz());
				PfxTransform3 worldTrB0 = tB0 * offsetTrB;
				PfxTransform3 worldTrB1 = tB1 * offsetTrB;

				if (shapeA.getType() == kPfxShapeCoreSphere && ccdEnable &&
					!arg->stage->pfxPreCheckCcd(shapeA.getCoreSphere(), shapeB, worldTrA0.getTranslation(), worldTrA1.getTranslation(), tB0, tB1)) {
					continue;
				}
				else if (shapeB.getType() == kPfxShapeCoreSphere && ccdEnable &&
					!arg->stage->pfxPreCheckCcd(shapeB.getCoreSphere(), shapeA, worldTrB0.getTranslation(), worldTrB1.getTranslation(), tA0, tA1)) {
					continue;
				}

				pfxGetDetectCollisionFunc(shapeA.getType(), shapeB.getType())(
					contactCache,
					shapeA, offsetTrA, worldTrA0, worldTrA0, shapeIdA,
					shapeB, offsetTrB, worldTrB0, worldTrB0, shapeIdB,
					SCE_PFX_CONTACT_THRESHOLD);

				contactManifold->cachedAxis = contactCache.cachedAxis;

				for (PfxUInt32 j = 0; j < contactCache.getNumContactPoints(); j++) {
					const PfxCachedContactPoint &cp = contactCache.getContactPoint(j);

					contactManifold->addContactPoint(
						cp.m_distance,
						cp.m_normal,
						cp.m_localPointA,
						cp.m_localPointB,
						cp.m_featureIdA,
						cp.m_featureIdB,
						cp.m_subDataA,
						cp.m_subDataB);
				}
			}

			contactManifold = contactHolder.findNextContactManifold(contactManifold);
		}

		contactManager->removeEmptyContactManifoldFromContactHolder(contactHolder);

		#else // SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS

		PfxContactCache contactCache;
		contactCache.reset();

		if (enegyA < SCE_PFX_SKIP_COLLISION_THRESHOLD && enegyB < SCE_PFX_SKIP_COLLISION_THRESHOLD) {
			contactCache.cachedAxis = contactManifold->cachedAxis;
		}
		else {
			contactCache.cachedAxis = PfxVector3::zero();
		}

		for(PfxUInt32 j=0;j<collA.getNumShapes();j++) {
			const PfxShape &shapeA = collA.getShape(j);

			if (shapeA.getType() == kPfxShapeCoreSphere && !ccdEnable) continue;

			PfxTransform3 offsetTrA = appendScale(shapeA.getOffsetTransform(),shapeA.getScaleXyz());
			PfxTransform3 worldTrA0 = tA0 * offsetTrA;
			PfxTransform3 worldTrA1 = tA1 * offsetTrA;

			for(PfxUInt32 k=0;k<collB.getNumShapes();k++) {
				const PfxShape &shapeB = collB.getShape(k);

				if (shapeB.getType() == kPfxShapeCoreSphere && !ccdEnable) continue;

				PfxTransform3 offsetTrB = appendScale(shapeB.getOffsetTransform(), shapeB.getScaleXyz());
				PfxTransform3 worldTrB0 = tB0 * offsetTrB;
				PfxTransform3 worldTrB1 = tB1 * offsetTrB;

				if (shapeA.getType() == kPfxShapeCoreSphere && ccdEnable &&
					!arg->stage->pfxPreCheckCcd(shapeA.getCoreSphere(), shapeB, worldTrA0.getTranslation(), worldTrA1.getTranslation(), tB0, tB1)) {
					continue;
				}
				else if (shapeB.getType() == kPfxShapeCoreSphere && ccdEnable &&
					!arg->stage->pfxPreCheckCcd(shapeB.getCoreSphere(), shapeA, worldTrB0.getTranslation(), worldTrB1.getTranslation(), tA0, tA1)) {
					continue;
				}

				if(pfxCheckContactFilter(
					shapeA.getContactFilterSelf(),shapeA.getContactFilterTarget(),
					shapeB.getContactFilterSelf(),shapeB.getContactFilterTarget())) {

					pfxGetDetectCollisionFunc(shapeA.getType(),shapeB.getType())(
						contactCache,
						shapeA, offsetTrA, worldTrA0, worldTrA1, j,
						shapeB, offsetTrB, worldTrB0, worldTrB1, k,
						SCE_PFX_CONTACT_THRESHOLD);
				}
			}
		}

		contactManifold->cachedAxis = contactCache.cachedAxis;

		for(PfxUInt32 j=0;j<contactCache.getNumContactPoints();j++) {
			const PfxCachedContactPoint &cp = contactCache.getContactPoint(j);

			contactManifold->addContactPoint(
				cp.m_distance,
				cp.m_normal,
				cp.m_localPointA,
				cp.m_localPointB,
				cp.m_subDataA,
				cp.m_subDataB);
		}

		#endif // SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS
		
		// Continuous collision detection
		if (ccdEnable) {
			for(PfxUInt32 j=0;j<collA.getNumCoreSpheres();j++) {
				const PfxShape shapeA(collA.getCoreSphere(j));
				
				PfxTransform3 offsetTrA = PfxTransform3::translation(shapeA.getOffsetPosition());
				PfxTransform3 worldTrA0 = tA0 * offsetTrA;
				PfxTransform3 worldTrA1 = tA1 * offsetTrA;

				for(PfxUInt32 k=0;k<collB.getNumShapes();k++) {
					const PfxShape &shapeB = collB.getShape(k);

					if(pfxCheckContactFilter(
						shapeA.getContactFilterSelf(), shapeA.getContactFilterTarget(),
						shapeB.getContactFilterSelf(), shapeB.getContactFilterTarget())) {
						PfxTransform3 offsetTrB = appendScale(shapeB.getOffsetTransform(),shapeB.getScaleXyz());
						PfxTransform3 worldTrB0 = tB0 * offsetTrB;
						PfxTransform3 worldTrB1 = tB1 * offsetTrB;

						if(!arg->stage->pfxPreCheckCcd(collA.getCoreSphere(j),shapeB,worldTrA0.getTranslation(),worldTrA1.getTranslation(),tB0,tB1)) {
							continue;
						}

						pfxGetDetectCollisionFunc(kPfxShapeCoreSphere,shapeB.getType())(
							contactCache,
							shapeA,offsetTrA,worldTrA0,worldTrA1,j,
							shapeB,offsetTrB,worldTrB0,worldTrB1,k,
							SCE_PFX_CONTACT_THRESHOLD);
					}
				}
			}

			for(PfxUInt32 j=0;j<collB.getNumCoreSpheres();j++) {
				const PfxShape shapeB(collB.getCoreSphere(j));

				PfxTransform3 offsetTrB = PfxTransform3::translation(shapeB.getOffsetPosition());
				PfxTransform3 worldTrB0 = tB0 * offsetTrB;
				PfxTransform3 worldTrB1 = tB1 * offsetTrB;

				for(PfxUInt32 k=0;k<collA.getNumShapes();k++) {
					const PfxShape &shapeA = collA.getShape(k);

					if(pfxCheckContactFilter(
						shapeB.getContactFilterSelf(), shapeB.getContactFilterTarget(),
						shapeA.getContactFilterSelf(), shapeA.getContactFilterTarget())) {
						PfxTransform3 offsetTrA = appendScale(shapeA.getOffsetTransform(),shapeA.getScaleXyz());
						PfxTransform3 worldTrA0 = tA0 * offsetTrA;
						PfxTransform3 worldTrA1 = tA1 * offsetTrA;

						if(!arg->stage->pfxPreCheckCcd(collB.getCoreSphere(j),shapeA,worldTrB0.getTranslation(),worldTrB1.getTranslation(),tA0,tA1)) {
							continue;
						}

						pfxGetDetectCollisionFunc(shapeA.getType(),kPfxShapeCoreSphere)(
							contactCache,
							shapeA,offsetTrA,worldTrA0,worldTrA1,j,
							shapeB,offsetTrB,worldTrB0,worldTrB1,k,
							SCE_PFX_CONTACT_THRESHOLD);
					}
				}
			}

			// insert closest points into the first contact manifold
			// Only the first contact manifold holds closest points 
			contactManifold = contactHolder.findFirstContactManifold();
			contactManifold->setClosestPointBuffer(NULL,0);
			int nClosests = SCE_PFX_MIN(contactCache.getNumClosestPoints(), 0xFF);
			if(nClosests > 0) {
				// if both are active and one has a core sphere, it always enables CCD
				if (SCE_PFX_MOTION_MASK_CAN_SLEEP(stateA.getMotionType()) && SCE_PFX_MOTION_MASK_CAN_SLEEP(stateB.getMotionType())) {
					if(collA.isContinuous()) stateA.setClosestHit(true); // It doesn't need atomic operation, because it just changes a single bit to 1.
					if(collB.isContinuous()) stateB.setClosestHit(true);
				}

				PfxClosestPoint *closestPoints = arg->contactManager->createClosestPoints(nClosests);
				if(closestPoints) {
					contactManifold->setClosestPointBuffer(closestPoints,nClosests);

					for(int j=0;j<nClosests;j++) {
						const PfxCachedContactPoint &cp = contactCache.getClosestPoint(j);

						if (cp.m_subDataA.getCcdPriority() & 0x08) {
							stateB.setClosestHit(true);
						}
						if (cp.m_subDataB.getCcdPriority() & 0x08) {
							stateA.setClosestHit(true);
						}

						contactManifold->addClosestPoint(
							cp.m_distance,
							cp.m_normal,
							cp.m_localPointA,
							cp.m_localPointB,
							cp.m_subDataA,
							cp.m_subDataB);
					}
				}
				else {
					context->appendError(PfxContext::kStageDetectCollision, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER);
				}
			}
		}

		pair.numConstraints = SCE_PFX_MIN(0xFF, contactHolder.getTotalContactAndClosestPoints());
	}
}

PfxInt32 PfxDetectCollisionStage::dispatchDetectCollision(PfxContactComplex &contactComplex)
{
	PfxContactManager *contactManager = (PfxContactManager*)&contactComplex.contactManager;
	PfxPairContainer *pairContainer = (PfxPairContainer*)&contactComplex.pairContainer;

	contactManager->clearAllClosestPoints();

	JobArg *jobArg = getRigidBodyContext()->allocate<JobArg>(1);
	if(!jobArg) { return SCE_PFX_ERR_OUT_OF_BUFFER; }

	PfxInt32 ret = SCE_OK;
	PfxInt32 dispatchId = getRigidBodyContext()->getDispatchId();

	JobArg &arg = *jobArg;
	arg.stage = this;
	arg.dispatchId = dispatchId;
	arg.contactManager = contactManager;
	arg.pairContainer = pairContainer;
	arg.numJobs = numJobs;

	// collision (args : 1)
	ret = getJobSystem()->dispatch(job_detectCollision, &arg, numJobs, "detect collision");
	if(ret != SCE_OK) return ret;

	ret = getJobSystem()->barrier();
	if(ret != SCE_OK) return ret;

	return SCE_PFX_OK;
}

PfxInt32 PfxDetectCollisionStage::preparePipeline(PfxFloat timeStep, PfxRigidState *states, PfxCollidable *collidables)
{
	if (timeStep <= 0.0f || !states || !collidables) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	shared.timeStep = timeStep;
	shared.states = states;
	shared.collidables = collidables;

	return SCE_PFX_OK;
}

void PfxContactComplex::print()
{
	for (PfxUInt32 i = 0; i < pairContainer.getPairLength(); i++) {
		PfxPairContainer::PairNode &pair = pairContainer.getPairNode(i);
		if (pair.state == NODE_IS_NOT_ASSIGNED) continue;

		PfxContactHolder &contactHolder = contactManager.getContactHolder(pair.data);
		PfxContactManifold *contactManifold = contactHolder.findFirstContactManifold();

		PfxUInt32 numContactPoints = contactManifold->getNumContactPoints();
		PfxUInt32 numClosestPoints = contactManifold->getNumClosestPoints();

		if (numContactPoints > 0 || numClosestPoints > 0) {
			float maxDepth = 0.0f;
			for (PfxUInt32 j = 0; j < numContactPoints; j++) {
				if (contactManifold->getContactPoint(j).m_distance < maxDepth) {
					maxDepth = contactManifold->getContactPoint(j).m_distance;
				}
			}

			SCE_PFX_PRINTF("pair %d %d num contacts %d max depth %f closest %d\n", pair.rigidbodyIdA, pair.rigidbodyIdB, numContactPoints, maxDepth, numClosestPoints);
		}
	}
}

} //namespace pfxv4
} //namespace sce
