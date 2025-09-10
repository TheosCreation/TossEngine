/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_PARTICLE_PHYSICS_H__
#define _SCE_PFX_PARTICLE_PHYSICS_H__

#include "../rigidbody/pfx_rigid_state.h"
#include "../collision/pfx_collidable.h"

namespace sce {
namespace pfxv4 {

//E Particle
//J パーティクル
struct SCE_PFX_ALIGNED(16) PfxParticle {
	PfxFloat m_mass;
	PfxFloat m_invMass;
	PfxFloat m_damping;
	PfxFloat m_radius;
	PfxVector3 m_curPosition;
	PfxVector3 m_prePosition;
	void *m_userData;

	void reset()
	{
		m_mass = 0.1f;
		m_invMass = 10.0f;
		m_damping = 1.0f;
		m_radius = 0.1f;
		m_userData = nullptr;
		m_curPosition = m_prePosition = PfxVector3::zero();
	}
};

//E Distance constraint
//J 距離拘束
struct PfxParticleDistanceConstraint {
	PfxUInt32 m_particleA;
	PfxUInt32 m_particleB;
	PfxFloat m_length;
	PfxFloat m_bias;
	PfxFloat m_damping;

	enum {kOneWayStretch, kOneWayCompress, kKeepLength} m_behavior;

	void reset()
	{
		m_particleA = 0;
		m_particleB = 0;
		m_length = 0.0f;
		m_bias = 0.2f;
		m_damping = 1.0f;
		m_behavior = kKeepLength;
	}
};

//E Angle constraint
//J 角度拘束
struct PfxParticleAngleConstraint {
	PfxUInt32 m_particleA;
	PfxUInt32 m_particleB;
	PfxUInt32 m_particleC;
	PfxFloat m_angle;
	PfxFloat m_bias;

	void reset()
	{
		m_particleA = 0;
		m_particleB = 0;
		m_particleC = 0;
		m_angle = 0.0f;
		m_bias = 0.2f;
	}
};

//E Attach to a rigid body constraint
//J 剛体との接続拘束
struct PfxParticleAttachConstraint {
	PfxUInt32 m_particleA;
	PfxUInt32 m_rigidBodyB;
	PfxFloat m_anchorB[3];
	PfxFloat m_bias;

	void reset()
	{
		m_particleA = 0;
		m_rigidBodyB = 0;
		m_anchorB[0] = 0.0f;
		m_anchorB[1] = 0.0f;
		m_anchorB[2] = 0.0f;
		m_bias = 0.2f;
	}
};

//E Attach constraint which returns force and torque to a rigid body
//J 剛体への反力を返す接続拘束
struct PfxParticlePushConstraint {
	PfxUInt32 m_particleA;
	PfxUInt32 m_rigidBodyB;
	PfxFloat m_anchorB[3];
	PfxFloat m_bias;
	PfxFloat m_stiffness;
	PfxFloat m_damping;

	void reset()
	{
		m_particleA = 0;
		m_rigidBodyB = 0;
		m_anchorB[0] = 0.0f;
		m_anchorB[1] = 0.0f;
		m_anchorB[2] = 0.0f;
		m_bias = 0.2f;
		m_stiffness = 300.0f;
		m_damping = 0.2f;
	}
};

enum ePfxParticleConstraintType {
	kPfxParticleConstraintDistance = 0,
	kPfxParticleConstraintAngle,
	kPfxParticleConstraintAttach,
	kPfxParticleConstraintPush,
	kPfxParticleConstraintCount
};

//E Particle constraint
//J パーティクル拘束
struct PfxParticleConstraint {
	ePfxParticleConstraintType type;
	union {
		PfxParticleDistanceConstraint	distanceConstraint;
		PfxParticleAngleConstraint		angleConstraint;
		PfxParticleAttachConstraint		attachConstraint;
		PfxParticlePushConstraint		pushConstraint;
	};
};

//E Contact Information
//J 衝突情報
struct PfxParticleContactInfo {
	PfxVector3 pointOnShape;
	PfxVector3 normal;
	PfxFloat depth;
};

//J インテグレート関数
//E Integrate functions
void pfxIntegrateParticle(PfxParticle &particle, const PfxVector3 &force, const PfxFloat timeStep);

//J 拘束ソルバー関数
//E Constraint solver functions

//J 距離拘束はＡＢ間の距離を一定に保つようにパーティクルＡ，Ｂの位置を更新します。
//E Distance constraint works as it updates the position of particle A and B so that they are fixed in the specified distance.
void pfxSolveParticleDistanceConstraint( const PfxParticleDistanceConstraint &constraint,PfxParticle &particleA,PfxParticle &particleB);

//J ＡＢ間の距離を特定の方向上に一定に保つようにパーティクルＡ，Ｂの位置を更新します。
//E It updates the position of particle A and B so that they are fixed in the specified distance along the specificed direction.
void pfxSolveParticleDistanceConstraintAlongDirection( const PfxParticleDistanceConstraint &constraint, PfxParticle &particleA, PfxParticle &particleB, const PfxVector3 &direction );

//J 角度拘束はベクトルＡＢとベクトルＢＣとの成す角が指定値を超えないようにパーティクルＣの位置を更新します。
//E Angular constraint works as it updates the position of particle C so that the angle between vector AB and vector BC doesn't exceed the specified angle.
void pfxSolveParticleAngleConstraint( const PfxParticleAngleConstraint &constraint,const PfxParticle &particleA,PfxParticle &particleB,PfxParticle &particleC);

//J 剛体Ｂの相対的な位置にパーティクルＡを接続します。この関数はパーティクルＡの位置を剛体Ｂの相対位置へ動かします。
//E Attach particle A on the relative position of rigid body B. This function change partice A's position to the relative position on the rigid body B.
void pfxSolveParticleAttachConstraint( const PfxParticleAttachConstraint &constraint, PfxParticle &particleA, const PfxRigidState &stateB);

//J 剛体Ｂの相対的な位置にパーティクルＡを接続します。この関数はパーティクルＡの位置を動かし、剛体Ｂへ与える力とトルクを返します。
//E Attach particle A on the relative position of rigid body B. This function change partice A's position and returns force and torque to move rigid body B.
void pfxSolveParticlePushConstraint( const PfxParticlePushConstraint &constraint, PfxParticle &particleA, const PfxRigidState &stateB, const PfxFloat timeStep, PfxVector3 &force, PfxVector3 &torque);

//J 衝突反発関数
//E Contact response functions
void pfxContactParticleSphere(PfxParticle &particle,const PfxVector3 &spherePosition,PfxFloat sphereRadius,PfxFloat bias);
void pfxContactParticleCapsule(PfxParticle &particle,const PfxVector3 &capsulePosition1,const PfxVector3 &capsulePosition2,PfxFloat capsuleRadius,PfxFloat bias);
bool pfxContactParticlePlane(PfxParticle &particle,const PfxVector3 &planeNormal,PfxFloat planeDistance,PfxFloat bias);
void pfxContactParticleBox(PfxParticle &particle,const PfxTransform3 &boxTransform,const PfxVector3 &boxExtent,PfxFloat bias);
void pfxContactParticleConvexMesh(PfxParticle &particle,const PfxTransform3 &convexTransform,const PfxVector3 &scale,const PfxConvexMesh &convexMesh,PfxFloat bias);
void pfxContactParticleLargeTriMesh(PfxParticle &particle,const PfxTransform3 &largeMeshTransform,const PfxVector3 &scale,const PfxLargeTriMesh &largeMesh,PfxFloat bias);

//J 衝突情報取得
//E Functions to get a contact information
PfxBool pfxGetContactInfoParticleSphere( PfxParticleContactInfo &contact, const PfxParticle &particle, const PfxVector3 &spherePosition, PfxFloat sphereRadius );
PfxBool pfxGetContactInfoParticleCapsule( PfxParticleContactInfo &contact, const PfxParticle &particle, const PfxVector3 &capsulePosition1, const PfxVector3 &capsulePosition2, PfxFloat capsuleRadius );
PfxBool pfxGetContactInfoParticlePlane( PfxParticleContactInfo &contact, const PfxParticle &particle, const PfxVector3 &planeNormal, PfxFloat planeDistance );

} // namespace pfxv4
} // namespace sce

#endif /* _SCE_PFX_PARTICLE_PHYSICS_H__ */
