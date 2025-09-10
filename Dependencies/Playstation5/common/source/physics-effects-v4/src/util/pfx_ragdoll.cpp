/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                Copyright (C)  Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../include/physics_effects/util/pfx_ragdoll.h"
#include "../base_level/solver/pfx_constraint_row_solver.h"

#define SCE_PFX_MOTION_MASK_APPLYFORCE ((1<<kPfxMotionTypeFixed)|(1<<kPfxMotionTypeTrigger)|(1<<kPfxMotionTypeKeyframe))

namespace sce {
namespace pfxv4 {

// size of work buffer = sizeof(PfxUInt32) * number of pairs * 2
PfxUInt32 pfxSortRagdollJoints(RagdollJointPair*   ragdollJointPairs,
                               PfxUInt32           numRagdollJointPairs,
                               const PfxRigidBody* bodies,
                               void*               workBuffer,
                               PfxUInt32           workBytes)
{
    if( workBytes < sizeof( PfxUInt32 ) * numRagdollJointPairs * 2 )
        return SCE_PFX_ERR_OUT_OF_BUFFER;

    PfxFloat maxMass = 0.0f;
    int maxMassPairId = 0;
    int maxMassRigidBodyId = 0;
    for( PfxUInt32 i = 0; i < numRagdollJointPairs; i++ )
    {
        const RagdollJointPair& jointPair = ragdollJointPairs[ i ];

        const PfxRigidBody& bodyA = bodies[ jointPair.parentId ];
        const PfxRigidBody& bodyB = bodies[ jointPair.childId ];

        if( maxMass < bodyA.getMass() )
        {
            maxMass = bodyA.getMass();
            maxMassPairId = i;
            maxMassRigidBodyId = jointPair.parentId;
        }

        if( maxMass < bodyB.getMass() )
        {
            maxMass = bodyB.getMass();
            maxMassPairId = i;
            maxMassRigidBodyId = jointPair.childId;
        }
    }

    // Now we sort the array by levels so that the first level contains only the joint with the heaviest body (we assume
    // we don't have a tie with multiple bodies having the same heaviest mass), the N-th level contains all the joints
    // that have as a parent the bodies that are children in the (N-1)-th level and so forth
    struct Level
    {
        PfxUInt32* bodyId;
        PfxUInt32  numBodies;
    };

    Level level[ 2 ]; // double buffered
    level[ 0 ].bodyId = (PfxUInt32*)( (uintptr_t)workBuffer );
    level[ 1 ].bodyId = (PfxUInt32*)( (uintptr_t)workBuffer + sizeof( PfxUInt32 ) * numRagdollJointPairs );

    // we start with the heaviest body at the root of the tree
    level[ 0 ].bodyId[ 0 ] = maxMassRigidBodyId;
    level[ 0 ].numBodies = 1;
    level[ 1 ].numBodies = 0;

    unsigned int numJointsFinalized = 0;
    unsigned int parentLevelId = 0;

    while( numJointsFinalized < numRagdollJointPairs && level[ parentLevelId ].numBodies > 0 )
    {
        const Level& parentLevel = level[ parentLevelId ];
        Level& nextLevel         = level[ 1 - parentLevelId ];

        for( PfxUInt32 currentPairId = numJointsFinalized; currentPairId < numRagdollJointPairs; currentPairId++ )
        {
            RagdollJointPair currentPairCopy = ragdollJointPairs[ currentPairId ];

            for( PfxUInt32 i = 0; i < parentLevel.numBodies; i++ )
            {
                const int potentialParendBodyId = parentLevel.bodyId[ i ];

                if( potentialParendBodyId == currentPairCopy.parentId )
                {
                    // This joint is connected to the parent level. It needs to be in the next level
                    ragdollJointPairs[ currentPairId ]      = ragdollJointPairs[ numJointsFinalized ];
                    ragdollJointPairs[ numJointsFinalized ] = currentPairCopy;
                    numJointsFinalized++;

                    const int slotInNextLevel = nextLevel.numBodies++;
                    nextLevel.bodyId[ slotInNextLevel ] = currentPairCopy.childId;
                }
            }
        }

        level[ parentLevelId ].numBodies = 0;   // resets the parent level cause we're about to reuse it
        parentLevelId = 1 - parentLevelId;
    }

    return SCE_PFX_OK;
}

PfxFloat pfxCalculateRagdollAveragePoseDifference(
	RagdollJointPair *sortedRagdollJointPairs, PfxUInt32 numSortedRagdollJointPairs,
	PfxRigidState *states, PfxJoint *joints)
{
	PfxFloat totaldiff = 0.0f;
	
	PfxUInt32 numActiveConstraints = numSortedRagdollJointPairs;

	// First, collect differences between joint angles
	for (PfxUInt32 i = 0; i < numSortedRagdollJointPairs; i++) {
		RagdollJointPair &jointPair = sortedRagdollJointPairs[i];

		if( jointPair.scale == 0.0f )
			continue;

		PfxJoint &joint = joints[jointPair.jointId];
		PfxRigidState &parentState = states[jointPair.parentId];
		PfxRigidState &childState = states[jointPair.childId];

		PfxMatrix3 worldFrameA;
		worldFrameA = PfxMatrix3(parentState.getOrientation()) * joint.m_frameA;
		worldFrameA = worldFrameA * PfxMatrix3(joint.m_targetFrame);

		PfxQuat childOrientation = PfxQuat(worldFrameA * transpose(joint.m_frameB));

		PfxQuat rot = childOrientation * conj(childState.getOrientation());
		PfxVector3 axis;
		PfxFloat angle;
		pfxGetRotationAngleAndAxis(rot, angle, axis);
		totaldiff += angle * angle;
		numActiveConstraints++;
	}

	PfxFloat avgDiff = totaldiff / (PfxFloat)numActiveConstraints;

	return avgDiff;
}

void pfxRagdollPositionProjection(
	RagdollJointPair *sortedRagdollJointPairs, PfxUInt32 numSortedRagdollJointPairs,
	PfxRigidState *states, PfxJoint *joints, PfxFloat ratio )
{
	for (PfxUInt32 i = 0; i < numSortedRagdollJointPairs; i++) {
		RagdollJointPair &jointPair = sortedRagdollJointPairs[i];
		PfxJoint &joint = joints[jointPair.jointId];
		PfxRigidState &parentState = states[jointPair.parentId];
		PfxRigidState &childState = states[jointPair.childId];

		if( jointPair.scale == 0.0f )
			continue;

		if (joint.m_type == kPfxJointMotor) {
			// Position based method : It moves a child rigid body in a geometric way
			PfxLargePosition posA = parentState.getLargePosition();
			PfxLargePosition posB = childState.getLargePosition();
			posB.changeSegment(posA.segment);

			PfxVector3 anchorA = posA.offset + rotate(parentState.getOrientation(), joint.m_anchorA);

			PfxMatrix3 worldFrameA;
			worldFrameA = PfxMatrix3(parentState.getOrientation()) * joint.m_frameA;
			worldFrameA = worldFrameA * PfxMatrix3(joint.m_targetFrame);

			PfxQuat childOrientation = PfxQuat(worldFrameA * transpose(joint.m_frameB));
			PfxVector3 childPosition = anchorA - rotate(childOrientation, joint.m_anchorB);

			childPosition = posB.offset + ratio * jointPair.scale * (childPosition - posB.offset);
			childOrientation = slerp(ratio * jointPair.scale, childState.getOrientation(), childOrientation);

			posB.offset = childPosition;

			childState.setOrientation(PfxQuat(childOrientation));
			childState.setLargePosition(posB);
		}
	}
}

void pfxRagdollVelocityProjection(
	RagdollJointPair *sortedRagdollJointPairs, PfxUInt32 numSortedRagdollJointPairs,
	PfxRigidState *states, PfxJoint *joints, PfxFloat ratio, PfxFloat timeStep )
{
	for( PfxUInt32 i = 0; i < numSortedRagdollJointPairs; i++ ) {
		RagdollJointPair &jointPair = sortedRagdollJointPairs[ i ];
		PfxJoint &joint = joints[ jointPair.jointId ];
		PfxRigidState &parentState = states[ jointPair.parentId ];
		PfxRigidState &childState = states[ jointPair.childId ];

		if( jointPair.scale == 0.0f )
			continue;

		if( joint.m_type == kPfxJointMotor )
		{
			PfxVector3 linearVelocityA = parentState.getLinearVelocity();
			PfxVector3 angularVelocityA = parentState.getAngularVelocity();
			PfxVector3 linearVelocityB = childState.getLinearVelocity();
			PfxVector3 angularVelocityB = childState.getAngularVelocity();

			// Velocity based method : It changes child's velocity
			PfxMatrix3 ori(childState.getOrientation());

			PfxFloat massInvB = 1.0f;
			PfxMatrix3 inertiaInvB = ori * PfxMatrix3::scale(PfxVector3(1.0f)) * transpose(ori);

			PfxVector3 rA = rotate(parentState.getOrientation(), joint.m_anchorA);
			PfxVector3 rB = rotate(childState.getOrientation(), joint.m_anchorB);

			PfxVector3 vA = linearVelocityA + cross(angularVelocityA, rA);
			PfxVector3 vB = linearVelocityB + cross(angularVelocityB, rB);
			PfxVector3 vAB = vA - vB;

			PfxVector3 angAB = angularVelocityA - angularVelocityB;

			PfxVector3 distance = (parentState.getLargePosition() - childState.getLargePosition()).convertToVector3() + rA - rB;

			PfxMatrix3 worldFrameA, worldFrameB;
			worldFrameA = PfxMatrix3(parentState.getOrientation()) * joint.m_frameA;
			worldFrameB = PfxMatrix3(childState.getOrientation()) * joint.m_frameB;

			// Linear Constraint
			PfxMatrix3 K = PfxMatrix3::scale(PfxVector3(massInvB)) - crossMatrix(rB) * inertiaInvB * crossMatrix(rB);

			for (int c = 0; c < 3; c++) {
				PfxJointConstraint &jointConstraint = joint.m_constraints[c];

				PfxVector3 normal = worldFrameA[c];

				PfxFloat posErr = dot(distance, -normal);
				PfxFloat lowerLimit = -jointConstraint.m_maxImpulse;
				PfxFloat upperLimit = jointConstraint.m_maxImpulse;

				if (posErr < jointConstraint.m_movableLowerLimit) {
					posErr = posErr - jointConstraint.m_movableLowerLimit;
					posErr = SCE_PFX_MIN(0.0f, posErr);
					upperLimit = SCE_PFX_MIN(0.0f, upperLimit);
				}
				else { // posErr > movableUpperLimit
					posErr = posErr - jointConstraint.m_movableUpperLimit;
					posErr = SCE_PFX_MAX(0.0f, posErr);
					lowerLimit = SCE_PFX_MAX(0.0f, lowerLimit);
				}

				PfxFloat bias = pfxModifyLinearBias(fabsf(posErr), jointPair.scale * ratio);
				PfxFloat denom = dot(K*normal, normal);
				PfxFloat jacDiagInv = 1.0f / denom;

				PfxFloat impulse = -dot(vAB, normal);
				impulse -= (bias * (-posErr)) / timeStep;
				impulse *= jacDiagInv;

				impulse = SCE_PFX_CLAMP(impulse, lowerLimit, upperLimit);
				linearVelocityB -= impulse * massInvB * normal;
				angularVelocityB -= impulse * inertiaInvB * cross(rB, normal);
			}

			worldFrameA = worldFrameA * PfxMatrix3(joint.m_targetFrame);

			PfxVector3 axis[3];
			PfxFloat angle[3];
			pfxCalcJointAngleSwingTwist(worldFrameA, worldFrameB, angle, axis);

			// Angular Constraint
			for (int c = 3; c < 6; c++) {
				PfxJointConstraint &jointConstraint = joint.m_constraints[c];

				PfxVector3 normal = axis[c - 3];

				PfxFloat posErr = angle[c - 3];
				PfxFloat lowerLimit = -jointConstraint.m_maxImpulse;
				PfxFloat upperLimit = jointConstraint.m_maxImpulse;

				if (jointConstraint.m_lock != SCE_PFX_JOINT_LOCK_FIX) {
					PfxFloat diffLower = pfxModifyAngle(0.0f - posErr);
					PfxFloat diffUpper = pfxModifyAngle(0.0f - posErr);
					if (0.0f < diffLower) {
						posErr = -diffLower;
						posErr = SCE_PFX_MIN(0.0f, posErr);
						upperLimit = SCE_PFX_MIN(0.0f, upperLimit);
					}
					else {
						posErr = -diffUpper;
						posErr = SCE_PFX_MAX(0.0f, posErr);
						lowerLimit = SCE_PFX_MAX(0.0f, lowerLimit);
					}
				}

				PfxFloat bias = pfxModifyAngularBias(fabsf(posErr), jointPair.scale * ratio);

				PfxFloat denom = dot((inertiaInvB)*normal, normal);
				if (denom > 0.0f) {
					PfxFloat jacDiagInv = 1.0f / denom;
					PfxFloat impulse = -dot(angAB, normal); // velocity error
					impulse -= (bias * (-posErr)) / timeStep; // position error
					impulse *= jacDiagInv;
					impulse = SCE_PFX_CLAMP(impulse, lowerLimit, upperLimit);
					angularVelocityB -= impulse * inertiaInvB * normal;
				}
			}

			childState.setLinearVelocity(linearVelocityB);
			childState.setAngularVelocity(angularVelocityB);
		}
	}
}

void pfxRagdollVelocityBasedLinearProjection( const RagdollJointPair* sortedRagdollJointPairs,
											  PfxUInt32               numSortedRagdollJointPairs,
											  PfxRigidState*          states,
											  PfxRigidBody*           bodies,
											  PfxJoint*               joints )
{
#define USE_REAL_MASS 0
	for( PfxUInt32 i = 0; i < numSortedRagdollJointPairs; i++ )
	{
		const RagdollJointPair& jointPair = sortedRagdollJointPairs[ i ];
		const PfxJoint& joint = joints[ jointPair.jointId ];
		if( joint.m_type != kPfxJointMotor )
			continue;

		const PfxRigidState& parentState = states[ jointPair.parentId ];
		PfxRigidState& childState = states[ jointPair.childId ];
#if USE_REAL_MASS
		const PfxRigidBody& childBody = bodies[ jointPair.childId ];
#endif

		const PfxVector3 parentLinearVelocity  = parentState.getLinearVelocity();
		const PfxVector3 parentAngularVelocity = parentState.getAngularVelocity();
		PfxVector3 childLinearVelocity  = childState.getLinearVelocity();
		PfxVector3 childAngularVelocity = childState.getAngularVelocity();

		// Velocity based method : It changes child's velocity
		const PfxMatrix3 childOrientation( childState.getOrientation() );

#if USE_REAL_MASS
		const PfxFloat childInverseMass = childBody.getMassInv();
		const PfxMatrix3 childInvInertiaWorldSpace = childOrientation * childBody.getInertiaInv() * transpose( childOrientation );
#else
		const PfxFloat childInverseMass = 1.0f;
		const PfxMatrix3 childInvInertiaWorldSpace = childOrientation * PfxMatrix3::scale( PfxVector3( 1.0f ) ) * transpose( childOrientation );
#endif

		const PfxVector3 rA = rotate( parentState.getOrientation(), joint.m_anchorA );
		const PfxVector3 rB = rotate( childState.getOrientation(), joint.m_anchorB );

		const PfxVector3 vA = parentLinearVelocity + cross( parentAngularVelocity, rA );
		const PfxVector3 vB = childLinearVelocity + cross( childAngularVelocity, rB );
		const PfxVector3 vAB = vA - vB;

		const PfxMatrix3 worldFrameA = PfxMatrix3( parentState.getOrientation() ) * joint.m_frameA;

		// Linear Constraint
		const PfxMatrix3 K = PfxMatrix3::scale( PfxVector3( childInverseMass ) ) - 
							 crossMatrix( rB ) * childInvInertiaWorldSpace * crossMatrix( rB );

		for( int c = 0; c < 3; c++ )
		{
			const PfxVector3 normal = worldFrameA[ c ];

			PfxFloat denom = dot( K * normal, normal );
			PfxFloat impulse = -dot( vAB, normal ) / denom;

			childLinearVelocity -= impulse * childInverseMass * normal;
			childAngularVelocity -= impulse * childInvInertiaWorldSpace * cross( rB, normal );
		}

		childState.setLinearVelocity( childLinearVelocity );
		childState.setAngularVelocity( childAngularVelocity );
	}
}

PfxVector3 pfxComputeDeltaLinearVelocity(const PfxRigidState &state, const PfxRigidBody &body, const PfxVector3 &targetLinearVelocity)
{
	if (((1 << state.getMotionType())&SCE_PFX_MOTION_MASK_APPLYFORCE) || state.isAsleep()) return PfxVector3(0.0f);

	PfxVector3 linearVelocity = state.getLinearVelocity();

	PfxFloat massInv = body.getMassInv();

	PfxVector3 vAB = targetLinearVelocity - linearVelocity;

	PfxVector3 deltaLinearVelocity(0.0f);
	// Linear Constraint
	PfxFloat l = length(targetLinearVelocity);
	if (l > 1e-8f) {
		PfxVector3 normal = targetLinearVelocity / l;

#if 1
		PfxFloat denom = dot(massInv*normal, normal);               // denom = massInv
		PfxFloat jacDiagInv = 1.0f / denom;                         // jacDiagInv = 1.0f / massInv
		PfxFloat impulse = -dot(vAB, normal); // velocity error     // impulse = -speedAlongNormal
		impulse *= jacDiagInv;                                      // impulse = -speedAlongNormal / massInv
		deltaLinearVelocity -= impulse * massInv * normal;
#else
		deltaLinearVelocity += dot( vAB, normal ) * normal;	// [SMS_CHANGE]
#endif
	}

	return deltaLinearVelocity;
}

PfxVector3 pfxComputeDeltaLinearVelocityLimit(const PfxRigidState &state, const PfxRigidBody &body, const PfxVector3 &targetLinearVelocity)
{
	if (((1 << state.getMotionType())&SCE_PFX_MOTION_MASK_APPLYFORCE) || state.isAsleep()) return PfxVector3(0.0f);

	PfxVector3 linearVelocity = state.getLinearVelocity();

	PfxFloat massInv = body.getMassInv();

	PfxVector3 vAB = targetLinearVelocity - linearVelocity;

	PfxVector3 deltaLinearVelocity(0.0f);
	// Linear Constraint
#if 1
	PfxFloat l = length(targetLinearVelocity);
	if (l > 1e-8f) {
		PfxVector3 axis[3];
		axis[0] = targetLinearVelocity / l;
		pfxGetPlaneSpace(axis[0], axis[1], axis[2]);

		for (int i = 0; i < 3; i++) {
			PfxFloat denom = dot(massInv*axis[i], axis[i]);
			PfxFloat jacDiagInv = 1.0f / denom;
			PfxFloat impulse = -dot(vAB, axis[i]); // velocity error
			impulse *= jacDiagInv;
			deltaLinearVelocity -= impulse * massInv * axis[i];
		}
	}
#else
	deltaLinearVelocity += vAB; // [SMS_CHANGE]
#endif

	return deltaLinearVelocity;
}

PfxVector3 pfxComputeDeltaAngularVelocity(const PfxRigidState &state, const PfxRigidBody &body, const PfxVector3 &targetAngularVelocity)
{
	if (((1 << state.getMotionType())&SCE_PFX_MOTION_MASK_APPLYFORCE) || state.isAsleep()) return PfxVector3(0.0f);

	PfxVector3 angularVelocity = state.getAngularVelocity();

	PfxMatrix3 ori(state.getOrientation());
	PfxMatrix3 inertiaInv = ori * body.getInertiaInv() * transpose(ori);

	PfxVector3 angAB = targetAngularVelocity - angularVelocity;

	PfxVector3 deltaAngularVelocity(0.0f);

	// Angular Constraint
	PfxFloat l = length(targetAngularVelocity);
	if (l > 1e-8f) {
		PfxVector3 normal = targetAngularVelocity / l;

		PfxFloat denom = dot((inertiaInv)*normal, normal);
		PfxFloat jacDiagInv = 1.0f / denom;
		PfxFloat impulse = -dot(angAB, normal); // velocity error
		impulse *= jacDiagInv;
		deltaAngularVelocity -= impulse * inertiaInv * normal;
	}

	return deltaAngularVelocity;
}

PfxVector3 pfxComputeDeltaAngularVelocityLimit(const PfxRigidState &state, const PfxRigidBody &body, const PfxVector3 &targetAngularVelocity)
{
	if (((1 << state.getMotionType())&SCE_PFX_MOTION_MASK_APPLYFORCE) || state.isAsleep()) return PfxVector3(0.0f);

	PfxVector3 angularVelocity = state.getAngularVelocity();

	PfxMatrix3 ori(state.getOrientation());
	PfxMatrix3 inertiaInv = ori * body.getInertiaInv() * transpose(ori);

	PfxVector3 angAB = targetAngularVelocity - angularVelocity;

	PfxVector3 deltaAngularVelocity(0.0f);

	// Angular Constraint
	PfxFloat l = length(targetAngularVelocity);
	if (l > 1e-8f) {
		PfxVector3 axis[3];
		axis[0] = targetAngularVelocity / l;
		pfxGetPlaneSpace(axis[0], axis[1], axis[2]);

		for (int i = 0; i < 3; i++) {
			PfxFloat denom = dot((inertiaInv)*axis[i], axis[i]);
			PfxFloat jacDiagInv = 1.0f / denom;
			PfxFloat impulse = -dot(angAB, axis[i]); // velocity error
			impulse *= jacDiagInv;
			deltaAngularVelocity -= impulse * inertiaInv * axis[i];
		}
	}

	return deltaAngularVelocity;
}

} //namespace PhysicsEffects
} //namespace sce
