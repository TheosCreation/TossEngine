/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CONSTRAINT_ROW_SOLVER_H
#define _SCE_PFX_CONSTRAINT_ROW_SOLVER_H

#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/solver/pfx_constraint_row.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_constraint.h"

#define SCE_PFX_CONSTRAINT_BIAS_MODIFY	0.05f

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Constraint Row Solver

static SCE_PFX_FORCE_INLINE
PfxFloat pfxSolveLinearConstraintRow(PfxConstraintRow &constraint, PfxFloat lowerLimit, PfxFloat upperLimit,
	PfxVector3 &deltaLinearVelocityA,PfxVector3 &deltaAngularVelocityA,
	PfxFloat massInvA,const PfxMatrix3 &inertiaInvA,const PfxVector3 &rA,
	PfxVector3 &deltaLinearVelocityB,PfxVector3 &deltaAngularVelocityB,
	PfxFloat massInvB,const PfxMatrix3 &inertiaInvB,const PfxVector3 &rB)
{
	const PfxVector3 normal(constraint.getNormal());
	PfxVector3 dVA = deltaLinearVelocityA + cross(deltaAngularVelocityA,rA);
	PfxVector3 dVB = deltaLinearVelocityB + cross(deltaAngularVelocityB,rB);
	PfxFloat deltaImpulse = constraint.m_rhs - constraint.m_jacDiagInv * dot(normal,dVA-dVB);
	PfxFloat oldImpulse = constraint.m_accumImpulse;
	constraint.m_accumImpulse = SCE_PFX_CLAMP(oldImpulse + deltaImpulse, lowerLimit, upperLimit);
	PfxFloat applyImpulse = constraint.m_accumImpulse - oldImpulse;
	deltaLinearVelocityA += applyImpulse * massInvA * normal;
	deltaAngularVelocityA += applyImpulse * inertiaInvA * cross(rA,normal);
	deltaLinearVelocityB -= applyImpulse * massInvB * normal;
	deltaAngularVelocityB -= applyImpulse * inertiaInvB * cross(rB,normal);
	return deltaImpulse;
}

static SCE_PFX_FORCE_INLINE
PfxFloat pfxSolveAngularConstraintRow(PfxConstraintRow &constraint, PfxFloat lowerLimit, PfxFloat upperLimit,
	PfxVector3 &deltaAngularVelocityA,
	const PfxMatrix3 &inertiaInvA,const PfxVector3 &rA,
	PfxVector3 &deltaAngularVelocityB,
	const PfxMatrix3 &inertiaInvB,const PfxVector3 &rB)
{
	(void)rA,(void)rB;
	const PfxVector3 normal(constraint.getNormal());
	PfxFloat deltaImpulse = constraint.m_rhs - constraint.m_jacDiagInv * dot(normal,deltaAngularVelocityA-deltaAngularVelocityB);
	PfxFloat oldImpulse = constraint.m_accumImpulse;
	constraint.m_accumImpulse = SCE_PFX_CLAMP(oldImpulse + deltaImpulse, lowerLimit, upperLimit);
	PfxFloat applyImpulse = constraint.m_accumImpulse - oldImpulse;
	deltaAngularVelocityA += applyImpulse * inertiaInvA * normal;
	deltaAngularVelocityB -= applyImpulse * inertiaInvB * normal;
	return deltaImpulse;
}

///////////////////////////////////////////////////////////////////////////////
// Calc Joint Angle

static SCE_PFX_FORCE_INLINE
PfxFloat pfxModifyAngle(PfxFloat angle)
{
	if(angle > 2.0f * SCE_PFX_PI) angle -= 2.0f * SCE_PFX_PI;
	else if(angle < -2.0f * SCE_PFX_PI) angle += 2.0f * SCE_PFX_PI;
	return angle;
}

static SCE_PFX_FORCE_INLINE
PfxFloat pfxModifyLinearBias(PfxFloat error, PfxFloat bias)
{
#if 0
	#define SCE_PFX_CONSTRAINT_LINEAR_BIAS_THRESHOLD	0.05f

	if(error < SCE_PFX_CONSTRAINT_LINEAR_BIAS_THRESHOLD) {
		bias = bias * powf(error,0.2f);
	}
	return bias;
#else
	return bias * (error / (error + SCE_PFX_CONSTRAINT_BIAS_MODIFY));
	//return bias * ((error+0.1f) / (error+0.5f));
	//return bias * (0.7f * (error+0.1f) * expf(1.0f-0.7f*(error+0.1f)));
#endif
}

static SCE_PFX_FORCE_INLINE
PfxFloat pfxModifyAngularBias(PfxFloat error, PfxFloat bias)
{
#if 0
	#define SCE_PFX_CONSTRAINT_ANGULAR_BIAS_THRESHOLD	0.25f

	if(error < SCE_PFX_CONSTRAINT_ANGULAR_BIAS_THRESHOLD) {
		bias = bias * powf(error,0.2f);
	}
	return bias;
#else
	return bias * (error / (error + SCE_PFX_CONSTRAINT_BIAS_MODIFY));
#endif
}

static SCE_PFX_FORCE_INLINE
void pfxCalcJointAngleHinge(PfxMatrix3 &worldFrameA,PfxMatrix3 &worldFrameB,PfxFloat *angle,PfxVector3 *axis)
{
	// フレームA座標系への変換マトリクス
	PfxMatrix3 frameBA = transpose(worldFrameA) * worldFrameB;

	// スイング回転を算出
	PfxQuat swing,qBA(frameBA);
	swing = PfxQuat::rotation(PfxVector3(1.0f,0.0f,0.0f),frameBA.getCol0());
	
	// 回転軸の角度を算出
	PfxVector3 by = frameBA.getCol1();
	angle[0] = atan2f(dot(by,PfxVector3(0.0f,0.0f,1.0f)),dot(by,PfxVector3(0.0f,1.0f,0.0f)));
	
	// スイング軸と回転角度を算出
	pfxGetRotationAngleAndAxis(normalize(swing),angle[1],axis[1]);

	if(angle[1] < 0.00001f) {
		axis[1] = PfxVector3(0.0f,1.0f,0.0f);
	}

	axis[0] = worldFrameB.getCol0(); // axis
	axis[1] = worldFrameA * axis[1]; // swing
	axis[2] = cross(axis[0],axis[1]); // stabilize
	angle[2] = 0.0f;
}

static SCE_PFX_FORCE_INLINE
void pfxCalcJointAngleSwingTwist(PfxMatrix3 &worldFrameA,PfxMatrix3 &worldFrameB,PfxFloat *angle,PfxVector3 *axis)
{
	// フレームA座標系への変換マトリクス
	PfxMatrix3 frameBA = transpose(worldFrameA) * worldFrameB;

	// クォータニオン回転をtwistとswingに分離
	PfxQuat swing,twist,qBA(frameBA);
	swing = PfxQuat::rotation(PfxVector3(1.0f,0.0f,0.0f),frameBA.getCol0());
	twist = qBA * conj(swing);

	if(dot(twist,PfxQuat::rotationX(0.0f)) < 0.0f) {
		twist = -twist;
	}

	// それぞれの回転軸と回転角度を算出
	pfxGetRotationAngleAndAxis(normalize(twist),angle[0],axis[0]);
	pfxGetRotationAngleAndAxis(normalize(swing),angle[1],axis[1]);

	if(angle[1] < 0.00001f) {
		axis[1] = PfxVector3(0.0f,1.0f,0.0f);
	}

	// twistの軸方向のチェック
	if(dot(axis[0],frameBA.getCol0()) < 0.0f) {
		angle[0] = -angle[0];
	}

	axis[0] = worldFrameB.getCol0();
	axis[1] = worldFrameA * axis[1];
	axis[2] = cross(axis[0],axis[1]);
	angle[2] = 0.0f;
}

static SCE_PFX_FORCE_INLINE
void pfxCalcJointAngleShoulder(PfxMatrix3 &worldFrameA,PfxMatrix3 &worldFrameB,PfxFloat *angle,PfxVector3 *axis)
{
	// フレームA座標系への変換マトリクス
	PfxMatrix3 frameBA = transpose(worldFrameA) * worldFrameB;

	// クォータニオン回転をtwistとswingに分離
	PfxQuat swing, twist, qBA(frameBA);
	swing = PfxQuat::rotation(PfxVector3::xAxis(), frameBA.getCol0());
	twist = qBA * conj(swing);

	if (dot(twist, PfxQuat::rotationX(0.0f)) < 0.0f) {
		twist = -twist;
	}

	// それぞれの回転軸と回転角度を算出
	pfxGetRotationAngleAndAxis(normalize(twist), angle[0], axis[0]);
	pfxGetRotationAngleAndAxis(normalize(swing), angle[1], axis[1]);

	if (angle[1] < 0.00001f) {
		axis[1] = PfxVector3::yAxis();
	}

	// twistの軸方向のチェック
	if (dot(axis[0], frameBA.getCol0()) < 0.0f) {
		angle[0] = -angle[0];
	}

	// swing2の計算
	PfxVector3 bx = frameBA.getCol0();
	if (fabsf(bx[2]) > 0.00001f) {
		bx[2] = 0.0f;
		PfxVector3 bxAxy = normalize(bx); // AのXY平面上に投影
		PfxQuat swing2 = PfxQuat::rotation(bxAxy, frameBA.getCol0());
		pfxGetRotationAngleAndAxis(normalize(swing2), angle[2], axis[2]);
		axis[2] = worldFrameA * axis[2];
	}
	else {
		axis[2] = cross(axis[0], axis[1]);
		angle[2] = 0.0f;
	}
	
	axis[0] = worldFrameB.getCol0();
	axis[1] = worldFrameA * axis[1];
}

static SCE_PFX_FORCE_INLINE
void pfxCalcJointAngleUniversal(PfxMatrix3 &worldFrameA,PfxMatrix3 &worldFrameB,PfxFloat *angle,PfxVector3 *axis)
{
	// フレームA座標系への変換マトリクス
	PfxMatrix3 frameBA = transpose(worldFrameA) * worldFrameB;

#if 0 // calc angle by quaternion decomposition
	// クォータニオン回転をtwistとswingに分離
	PfxQuat swing,swing1,swing2,twist,qBA(frameBA);
	PfxVector3 Pxy(frameBA.getCol0());
	Pxy[2] = 0.0f;
	PfxFloat l = length(Pxy);
	if(l > 1e-5f) {
		swing1 = PfxQuat::rotation(PfxVector3(1.0f,0.0f,0.0f),Pxy / l);
	}
	else {
		swing1 = PfxQuat::identity();
	}
	swing = PfxQuat::rotation(PfxVector3(1.0f,0.0f,0.0f),frameBA.getCol0());
	swing2 = swing * conj(swing1);
	twist = conj(swing) * qBA;

	if(dot(twist,PfxQuat::rotationX(0.0f)) < 0.0f) {
		twist = -twist;
	}

	pfxGetRotationAngleAndAxis(normalize(twist),angle[0],axis[0]);
	pfxGetRotationAngleAndAxis(normalize(swing1),angle[1],axis[1]);
	pfxGetRotationAngleAndAxis(normalize(swing2),angle[2],axis[2]);

	if(axis[1].getZ() < 0.0f) {
		axis[1] = -axis[1];
		angle[1] = -angle[1];
	}

	PfxVector3 chkY = cross(PfxVector3(0.0f,0.0f,1.0f),frameBA.getCol0());
	if(dot(chkY,axis[2]) < 0.0f) {
		axis[2] = -axis[2];
		angle[2] = -angle[2];
	}

	// twistの軸方向のチェック
	if(dot(axis[0],frameBA.getCol0()) < 0.0f) {
		angle[0] = -angle[0];
	}

	axis[0] = worldFrameB.getCol0();
	axis[1] = worldFrameA * axis[1];
	axis[2] = worldFrameA * axis[2];
#else  // calc angle by atan2
	PfxMatrix3 frameAB = transpose(worldFrameB) * worldFrameA;

	// Axis0の角度を算出
	{
		PfxVector3 ay = frameAB.getCol1();
		angle[0] = -atan2f(ay[2], ay[1]);
		axis[0] = worldFrameB.getCol0();
	}

	// Axis1の角度を算出
	{
		PfxVector3 bx = frameBA.getCol0();

		// BXをAのXZ平面に投影した軸を求める
		PfxVector3 bX_Axz = bx;
		bX_Axz[1] = 0.0f;

		PfxFloat len_bX_Axz = length(bX_Axz);
		if (len_bX_Axz > 0.00001f) {
			angle[1] = atan2f(bx[1], len_bX_Axz);
			axis[1] = normalize(cross(worldFrameB.getCol0(), worldFrameA.getCol1()));
		}
		else {
			angle[1] = 0.0f;
			axis[1] = worldFrameA.getCol2();
		}
	}

	// Axis2の角度を算出
	{
		PfxVector3 az = frameAB.getCol2();
		angle[2] = -atan2f(az[0], az[2]);
		axis[2] = worldFrameB.getCol1();
	}

#endif
}

///////////////////////////////////////////////////////////////////////////////
// Calc Joint Limit

static SCE_PFX_FORCE_INLINE
void pfxCalcLinearLimit(
	const PfxJointConstraint &jointConstraint,
	PfxFloat &posErr,PfxFloat &velocityAmp,PfxFloat &lowerLimit,PfxFloat &upperLimit)
{
	switch(jointConstraint.m_lock) {
		case SCE_PFX_JOINT_LOCK_FREE:
		posErr = 0.0f;
		velocityAmp *= jointConstraint.m_damping;
		break;
		
		case SCE_PFX_JOINT_LOCK_LIMIT:
		if(posErr >= jointConstraint.m_movableLowerLimit && posErr <= jointConstraint.m_movableUpperLimit) {
			posErr = 0.0f;
			velocityAmp *= jointConstraint.m_damping;
		}
		else {
			if(posErr < jointConstraint.m_movableLowerLimit) {
				posErr = posErr - jointConstraint.m_movableLowerLimit;
				posErr = SCE_PFX_MIN(0.0f,posErr+SCE_PFX_JOINT_LINEAR_SLOP);
				upperLimit = SCE_PFX_MIN(0.0f,upperLimit);
				velocityAmp = 1.0f;
			}
			else { // posErr > movableUpperLimit
				posErr = posErr - jointConstraint.m_movableUpperLimit;
				posErr = SCE_PFX_MAX(0.0f,posErr-SCE_PFX_JOINT_LINEAR_SLOP);
				lowerLimit = SCE_PFX_MAX(0.0f,lowerLimit);
				velocityAmp = 1.0f;
			}
		}
		break;
		
		default: // SCE_PFX_JOINT_LOCK_FIX
		break;
	}
}

static SCE_PFX_FORCE_INLINE
void pfxCalcAngularLimit(
	const PfxJointConstraint &jointConstraint,
	PfxFloat &posErr,PfxFloat &velocityAmp,PfxFloat &lowerLimit,PfxFloat &upperLimit)
{
	switch(jointConstraint.m_lock) {
		case SCE_PFX_JOINT_LOCK_FREE:
		posErr = 0.0f;
		velocityAmp *= jointConstraint.m_damping;
		break;
		
		case SCE_PFX_JOINT_LOCK_LIMIT:
		if(posErr >= jointConstraint.m_movableLowerLimit && posErr <= jointConstraint.m_movableUpperLimit) {
			posErr = 0.0f;
			velocityAmp *= jointConstraint.m_damping;
		}
		else {
			// 回転の連続性が失われないように、角度の差分が-π～πを超えないように調整
			PfxFloat diffLower = pfxModifyAngle(jointConstraint.m_movableLowerLimit - posErr);
			PfxFloat diffUpper = pfxModifyAngle(jointConstraint.m_movableUpperLimit - posErr);
			if(0.0f >= diffLower && 0.0f <= diffUpper) {
				posErr = 0.0f;
				velocityAmp *= jointConstraint.m_damping;
			}
			else {
				if(0.0f < diffLower) {
					posErr = -diffLower;
					posErr = SCE_PFX_MIN(0.0f,posErr+SCE_PFX_JOINT_ANGULAR_SLOP);
					upperLimit = SCE_PFX_MIN(0.0f,upperLimit);
					velocityAmp = 1.0f;
				}
				else {
					posErr = -diffUpper;
					posErr = SCE_PFX_MAX(0.0f,posErr-SCE_PFX_JOINT_ANGULAR_SLOP);
					lowerLimit = SCE_PFX_MAX(0.0f,lowerLimit);
					velocityAmp = 1.0f;
				}
			}
		}
		break;
		
		default: // SCE_PFX_JOINT_LOCK_FIX
		break;
	}
}

// for position correction
static SCE_PFX_FORCE_INLINE
void pfxCalcLinearLimit(const PfxJointConstraint &jointConstraint,PfxFloat &posErr)
{
	switch (jointConstraint.m_lock) {
	case SCE_PFX_JOINT_LOCK_FREE:
		posErr = 0.0f;
		break;

	case SCE_PFX_JOINT_LOCK_LIMIT:
		if (posErr >= jointConstraint.m_movableLowerLimit && posErr <= jointConstraint.m_movableUpperLimit) {
			posErr = 0.0f;
		}
		else {
			if (posErr < jointConstraint.m_movableLowerLimit) {
				posErr = posErr - jointConstraint.m_movableLowerLimit;
				posErr = SCE_PFX_MIN(0.0f, posErr + SCE_PFX_JOINT_LINEAR_SLOP);
			}
			else { // posErr > movableUpperLimit
				posErr = posErr - jointConstraint.m_movableUpperLimit;
				posErr = SCE_PFX_MAX(0.0f, posErr - SCE_PFX_JOINT_LINEAR_SLOP);
			}
		}
		break;

	default: // SCE_PFX_JOINT_LOCK_FIX
		break;
	}
}

// for position correction
static SCE_PFX_FORCE_INLINE
void pfxCalcAngularLimit(const PfxJointConstraint &jointConstraint,PfxFloat &posErr)
{
	switch (jointConstraint.m_lock) {
	case SCE_PFX_JOINT_LOCK_FREE:
		posErr = 0.0f;
		break;

	case SCE_PFX_JOINT_LOCK_LIMIT:
		if (posErr >= jointConstraint.m_movableLowerLimit && posErr <= jointConstraint.m_movableUpperLimit) {
			posErr = 0.0f;
		}
		else {
			// 回転の連続性が失われないように、角度の差分が-π～πを超えないように調整
			PfxFloat diffLower = pfxModifyAngle(jointConstraint.m_movableLowerLimit - posErr);
			PfxFloat diffUpper = pfxModifyAngle(jointConstraint.m_movableUpperLimit - posErr);
			if (0.0f >= diffLower && 0.0f <= diffUpper) {
				posErr = 0.0f;
			}
			else {
				if (0.0f < diffLower) {
					posErr = -diffLower;
					posErr = SCE_PFX_MIN(0.0f, posErr + SCE_PFX_JOINT_ANGULAR_SLOP);
				}
				else {
					posErr = -diffUpper;
					posErr = SCE_PFX_MAX(0.0f, posErr - SCE_PFX_JOINT_ANGULAR_SLOP);
				}
			}
		}
		break;

	default: // SCE_PFX_JOINT_LOCK_FIX
		break;
	}
}

inline PfxQuat pfxAddRotation(PfxQuat &orientation, PfxVector3 &rotation)
{
	PfxQuat dAng = PfxQuat(rotation, 0) * orientation * 0.5f;
	return normalize(orientation + dAng);
}

inline PfxBool isWarmStaringOk(const PfxRigidState &stateA, const PfxRigidState &stateB)
{
	PfxFloat linVelSqrA = lengthSqr(stateA.getLinearVelocity());
	PfxFloat angVelSqrA = lengthSqr(stateA.getAngularVelocity());
	PfxFloat linVelSqrB = lengthSqr(stateB.getLinearVelocity());
	PfxFloat angVelSqrB = lengthSqr(stateB.getAngularVelocity());

	const PfxFloat checkValue = 0.1f;
	return linVelSqrA < checkValue && angVelSqrA < checkValue && linVelSqrB < checkValue && angVelSqrB < checkValue;
}

inline PfxBool isWarmStaringOk(const PfxVector3 &linVel, const PfxVector3 &angVel)
{
	PfxFloat linVelSqr = lengthSqr(linVel);
	PfxFloat angVelSqr = lengthSqr(angVel);

	const PfxFloat checkValue = 0.1f;
	return linVelSqr < checkValue && angVelSqr < checkValue;
}

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_CONSTRAINT_ROW_SOLVER_H
