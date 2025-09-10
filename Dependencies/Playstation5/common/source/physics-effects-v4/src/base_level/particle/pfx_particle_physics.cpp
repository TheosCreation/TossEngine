/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "../../../include/physics_effects/base_level/particle/pfx_particle_physics.h"
#include "../collision/pfx_gjk_solver.h"
#include "../collision/pfx_gjk_support_func.h"
#include "../collision/pfx_contact_large_tri_mesh.h"

namespace sce {
namespace pfxv4 {

static const float epsilon = 0.00001f;

void pfxIntegrateParticle(PfxParticle &particle, const PfxVector3 &force,const PfxFloat timeStep)
{
	PfxVector3 tmpX = particle.m_prePosition;
	particle.m_prePosition = particle.m_curPosition;
	
	PfxVector3 dx = (particle.m_curPosition - tmpX) * particle.m_damping;
	PfxVector3 dv = force * particle.m_invMass * timeStep * timeStep;
	
	particle.m_curPosition += dx + dv;
}

void pfxSolveParticleDistanceConstraint( const PfxParticleDistanceConstraint &constraint,PfxParticle &particleA,PfxParticle &particleB)
{
	if(particleA.m_invMass == 0.0f && particleB.m_invMass == 0.0f) return;
	
	PfxVector3 distance = particleA.m_curPosition - particleB.m_curPosition;
	PfxFloat lenSqr = lengthSqr(distance);
	if(lenSqr < epsilon) return;
	
	PfxFloat len = sqrtf(lenSqr);
	PfxFloat posErr = len - constraint.m_length;
	PfxVector3 dir = distance / len;
	
	PfxVector3 vA = particleA.m_curPosition - particleA.m_prePosition;
	PfxVector3 vB = particleB.m_curPosition - particleB.m_prePosition;
	PfxVector3 vAB = vA - vB;

	PfxFloat correction = -constraint.m_damping * dot( dir, vAB ) - constraint.m_bias * posErr;

	if( constraint.m_behavior == PfxParticleDistanceConstraint::kOneWayStretch ) {
		if( posErr > 0.0f ) correction = 0.0f;
	}
	else if( constraint.m_behavior == PfxParticleDistanceConstraint::kOneWayCompress ) {
		if( posErr < 0.0f ) correction = 0.0f;
	}

	PfxFloat K = 1.0f / (particleA.m_invMass + particleB.m_invMass);
	correction *= K;
	
	particleA.m_curPosition += correction * particleA.m_invMass * dir;
	particleB.m_curPosition -= correction * particleB.m_invMass * dir;
}

void pfxSolveParticleDistanceConstraintAlongDirection( const PfxParticleDistanceConstraint &constraint, PfxParticle &particleA, PfxParticle &particleB, const PfxVector3 &direction )
{
	if( particleA.m_invMass == 0.0f && particleB.m_invMass == 0.0f ) return;
	SCE_PFX_ASSERT( fabsf(lengthSqr( direction ) - 1.0f ) < 0.0001f);

	PfxVector3 distance = particleA.m_curPosition - particleB.m_curPosition;
	PfxFloat len = dot( distance, direction );
	if( len < epsilon ) return;

	PfxVector3 distanceProj = len * direction;
	PfxFloat posErr = len - constraint.m_length;
	PfxVector3 dir = distanceProj / len;

	PfxVector3 vA = particleA.m_curPosition - particleA.m_prePosition;
	PfxVector3 vB = particleB.m_curPosition - particleB.m_prePosition;
	PfxVector3 vAB = vA - vB;

	PfxFloat correction = -constraint.m_damping * dot( dir, vAB ) - constraint.m_bias * posErr;

	if( constraint.m_behavior == PfxParticleDistanceConstraint::kOneWayStretch ) {
		if( posErr > 0.0f ) correction = 0.0f;
	}
	else if( constraint.m_behavior == PfxParticleDistanceConstraint::kOneWayCompress ) {
		if( posErr < 0.0f ) correction = 0.0f;
	}

	PfxFloat K = 1.0f / ( particleA.m_invMass + particleB.m_invMass );
	correction *= K;

	particleA.m_curPosition += correction * particleA.m_invMass * dir;
	particleB.m_curPosition -= correction * particleB.m_invMass * dir;
}

void pfxSolveParticleAngleConstraint( const PfxParticleAngleConstraint &constraint,const PfxParticle &particleA,PfxParticle &particleB,PfxParticle &particleC)
{
	if(particleB.m_invMass == 0.0f && particleC.m_invMass == 0.0f) return;

	PfxVector3 dirBA = particleB.m_curPosition - particleA.m_curPosition;
	PfxFloat lenSqrBA = lengthSqr(dirBA);
	if(lenSqrBA < epsilon) return;
	
	PfxVector3 dirCB = particleC.m_curPosition - particleB.m_curPosition;
	PfxFloat lenSqrCB = lengthSqr(dirCB);
	if(lenSqrCB < epsilon) return;
	
	PfxFloat lenBA = sqrtf(lenSqrBA);
	PfxFloat lenCB = sqrtf(lenSqrCB);
	
	dirBA /= lenBA;
	dirCB /= lenCB;
	
	PfxFloat cosCurr = dot(dirBA,dirCB);
	PfxFloat cosLimit = cosf(constraint.m_angle);
	
	if(cosCurr < cosLimit) {
		PfxVector3 vtmp = cross(dirBA,dirCB);
		PfxFloat vtmpSqrLen = lengthSqr(vtmp);
		if(vtmpSqrLen > epsilon) {
			PfxVector3 rotAxis = vtmp/sqrtf(vtmpSqrLen);
			PfxVector3 distCB = particleC.m_curPosition - particleB.m_curPosition;
			PfxQuat rot = PfxQuat::rotation(constraint.m_angle,rotAxis);
			PfxVector3 correctPositionC = lenCB * rotate(rot,dirBA);
			correctPositionC = particleB.m_curPosition + lerp(constraint.m_bias,distCB,correctPositionC);
			particleC.m_curPosition = correctPositionC;
			
			if(particleB.m_invMass > 0.0f) {
				PfxQuat rotB = PfxQuat::rotation(dirCB,rotate(rot,dirBA));
				PfxVector3 correctPositionB = lenCB * rotate(rotB,-dirCB);
				correctPositionB = particleC.m_curPosition + lerp(constraint.m_bias,-distCB,correctPositionB);
				particleB.m_curPosition = correctPositionB;
			}
		}
	}
}

void pfxSolveParticleAttachConstraint( const PfxParticleAttachConstraint &constraint, PfxParticle &particleA, const PfxRigidState &stateB)
{
	PfxLargePosition pos = stateB.getLargePosition();
	pos.changeSegment(PfxSegment());

	PfxVector3 anchorB(constraint.m_anchorB[0], constraint.m_anchorB[1], constraint.m_anchorB[2]);
	PfxVector3 positionB = pos.offset + rotate(stateB.getOrientation(), anchorB);

	PfxVector3 distance = particleA.m_curPosition - positionB;
	PfxFloat lenSqr = lengthSqr(distance);

	if (lenSqr > 0.00001f) {
		PfxFloat len = sqrtf(lenSqr);
		PfxFloat posErr = len;
		PfxVector3 dir = distance / len;
		PfxFloat correction = -constraint.m_bias * posErr;
		particleA.m_curPosition += correction * dir;
	}
}

void pfxSolveParticlePushConstraint(const PfxParticlePushConstraint &constraint, PfxParticle &particleA, const PfxRigidState &stateB, const PfxFloat timeStep, PfxVector3 &force, PfxVector3 &torque)
{
	PfxLargePosition pos = stateB.getLargePosition();
	pos.changeSegment(PfxSegment());

	PfxVector3 rB = rotate(stateB.getOrientation(), pfxReadVector3(constraint.m_anchorB));
	PfxVector3 positionB = pos.offset + rB;

	PfxVector3 distance = particleA.m_curPosition - positionB;
	PfxFloat lenSqr = lengthSqr(distance);

	if (lenSqr > 0.00001f) {
		PfxFloat len = sqrtf(lenSqr);
		PfxFloat posErr = len;
		PfxVector3 dir = distance / len;
		PfxFloat correction = -constraint.m_bias * posErr;
		particleA.m_curPosition += correction * dir;

		force = constraint.m_stiffness * distance + constraint.m_damping * dot(dir, (particleA.m_curPosition - particleA.m_prePosition) / timeStep) * dir;
		torque = cross(rB, force);
	}
}

void pfxContactParticleSphere(PfxParticle &particle,const PfxVector3 &spherePosition,PfxFloat sphereRadius,PfxFloat bias)
{
	PfxVector3 distance = particle.m_curPosition - spherePosition;
	PfxFloat lenSqr = lengthSqr(distance);
	PfxFloat radius = sphereRadius + particle.m_radius;
	if(lenSqr > epsilon && lenSqr < radius * radius) {
		PfxFloat l = sqrtf(lenSqr);
		particle.m_curPosition += bias * (radius - l) * (distance / l);
	}
}

void pfxContactParticleCapsule(PfxParticle &particle,const PfxVector3 &capsulePosition1,const PfxVector3 &capsulePosition2,PfxFloat capsuleRadius,PfxFloat bias)
{
	PfxVector3 p = particle.m_curPosition;
	PfxVector3 direction = capsulePosition2 - capsulePosition1;
	PfxFloat t = dot(p - capsulePosition1,direction) / dot(direction,direction);
	t = SCE_PFX_CLAMP(t, 0.0f, 1.0f);
	PfxVector3 closestPosition = capsulePosition1 + t * direction;
	PfxVector3 distance = p - closestPosition;
	PfxFloat lenSqr = lengthSqr(distance);
	PfxFloat radius = capsuleRadius + particle.m_radius;
	if (lenSqr > epsilon && lenSqr < radius*radius) {
		PfxFloat l = sqrtf(lenSqr);
		particle.m_curPosition += bias * (radius - l) * (distance / l);
	}
}

bool pfxContactParticlePlane(PfxParticle &particle,const PfxVector3 &planeNormal,PfxFloat planeDistance,PfxFloat bias)
{
	PfxFloat distance = dot(particle.m_curPosition,planeNormal);
	PfxFloat targetDistance = planeDistance + particle.m_radius;
	if(distance < targetDistance) {
		particle.m_curPosition += bias * (targetDistance - distance) * planeNormal;
		return true;
	}
	return false;
}

void pfxContactParticleRoundBox(PfxParticle &particle,const PfxTransform3 &boxTransform,const PfxVector3 &boxExtent,PfxFloat margin,PfxFloat bias)
{
	PfxVector3 P(orthoInverse(boxTransform) * PfxPoint3(particle.m_curPosition)); // in the box local coordinates
	const PfxVector3 B = boxExtent;
	PfxVector3 absBoxP = absPerElem(P);
	PfxVector3 boxExtentM = boxExtent + PfxVector3(margin);
	
	// check voronoi region
	// todo : compress following process
	PfxVector3 src(0.0f);
	bool inVoronoi = false;
	
	// edge Z dir
	if(P[2]<B[2] && P[2]>-B[2]) {
		if(P[0]>B[0]) {
			if(P[1]>B[1]) {
				src = PfxVector3(B[0],B[1],P[2]);
				inVoronoi = true;
			}
			else if(P[1]<-B[1]) {
				src = PfxVector3(B[0],-B[1],P[2]);
				inVoronoi = true;
			}
		}
		else if(P[0]<-B[0]) {
			if(P[1]>B[1]) {
				src = PfxVector3(-B[0],B[1],P[2]);
				inVoronoi = true;
			}
			else if(P[1]<-B[1]) {
				src = PfxVector3(-B[0],-B[1],P[2]);
				inVoronoi = true;
			}
		}
	}

	// edge X dir
	else if(P[0]<B[0] && P[0]>-B[0]) {
		if(P[1]>B[1]) {
			if(P[2]>B[2]) {
				src = PfxVector3(P[0],B[1],B[2]);
				inVoronoi = true;
			}
			else if(P[2]<-B[2]) {
				src = PfxVector3(P[0],B[1],-B[2]);
				inVoronoi = true;
			}
		}
		else if(P[1]<-B[1]) {
			if(P[2]>B[2]) {
				src = PfxVector3(P[0],-B[1],B[2]);
				inVoronoi = true;
			}
			else if(P[2]<-B[2]) {
				src = PfxVector3(P[0],-B[1],-B[2]);
				inVoronoi = true;
			}
		}
	}

	// edge Y dir
	else if(P[1]<B[1] && P[1]>-B[1]) {
		if(P[0]>B[0]) {
			if(P[2]>B[2]) {
				src = PfxVector3(B[0],P[1],B[2]);
				inVoronoi = true;
			}
			else if(P[2]<-B[2] ) {
				src = PfxVector3(B[0],P[1],-B[2]);
				inVoronoi = true;
			}
		}
		else if(P[0]<-B[0]) {
			if(P[2]>B[2]) {
				src = PfxVector3(-B[0],P[1],B[2]);
				inVoronoi = true;
			}
			else if(P[2]<-B[2] ) {
				src = PfxVector3(-B[0],P[1],-B[2]);
				inVoronoi = true;
			}
		}
	}

	// points
	else if(P[0]>B[0]) {
		if(P[1]>B[1]) {
			if(P[2]>B[2]) {
				src = PfxVector3(B[0],B[1],B[2]);
				inVoronoi = true;
			}
			else if(P[2]<-B[2]) {
				src = PfxVector3(B[0],B[1],-B[2]);
				inVoronoi = true;
			}
		}
		else if(P[1]<-B[1]) {
			if(P[2]>B[2]) {
				src = PfxVector3(B[0],-B[1],B[2]);
				inVoronoi = true;
			}
			else if(P[2]<-B[2]) {
				src = PfxVector3(B[0],-B[1],-B[2]);
				inVoronoi = true;
			}
		}
	}
	else if(P[0]<-B[0]) {
		if(P[1]>B[1]) {
			if(P[2]>B[2]) {
				src = PfxVector3(-B[0],B[1],B[2]);
				inVoronoi = true;
			}
			else if(P[2]<-B[2]) {
				src = PfxVector3(-B[0],B[1],-B[2]);
				inVoronoi = true;
			}
		}
		else if(P[1]<-B[1]) {
			if(P[2]>B[2]) {
				src = PfxVector3(-B[0],-B[1],B[2]);
				inVoronoi = true;
			}
			else if(P[2]<-B[2]) {
				src = PfxVector3(-B[0],-B[1],-B[2]);
				inVoronoi = true;
			}
		}
	}

	if(inVoronoi) {
		if(lengthSqr(P - src) > margin * margin) return;
		PfxVector3 tgt = src + margin * normalize(P - src);
		particle.m_curPosition += bias * boxTransform.getUpper3x3() * (tgt - P);
	}
	else if(absBoxP[0] < boxExtentM[0] && absBoxP[1] < boxExtentM[1] && absBoxP[2] < boxExtentM[2]) {
		PfxVector3 diff = boxExtentM - absBoxP;
		int i = 0;
		if(diff[1] < diff[0]) {
			i = 1;
		}
		if(diff[2] < diff[i]) {
			i = 2;
		}
		PfxVector3 sign = divPerElem(P,absBoxP);
		PfxVector3 s = P;
		s[i] = sign[i] * boxExtentM[i];
		particle.m_curPosition += bias * boxTransform.getUpper3x3() * (s - P);
	}
}

void pfxContactParticleBox(PfxParticle &particle,const PfxTransform3 &boxTransform,const PfxVector3 &boxExtent,PfxFloat bias)
{
	pfxContactParticleRoundBox(particle, boxTransform, boxExtent, particle.m_radius, bias);
}

void pfxContactParticleConvexMesh(PfxParticle &particle,const PfxTransform3 &convexTransform,const PfxVector3 &scale,const PfxConvexMesh &convexMesh,PfxFloat bias)
{
	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA, pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;
	PfxVector3 cachedAxis(0.0f);

	PfxSphere sphere(particle.m_radius);
	const PfxConvexMeshImpl *convex = (PfxConvexMeshImpl*)&convexMesh;

	PfxTransform3 particleTransform(PfxMatrix3::identity(), particle.m_curPosition);

	PfxMpr<PfxSphere, PfxConvexMeshImpl> collider(&sphere, convex);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, particleTransform, convexTransform, cachedAxis, SCE_PFX_FLT_MAX);

	if (err == kPfxGjkResultOk && d < 0.0f) {
		PfxPoint3 wA = particleTransform * pA;
		PfxPoint3 wB = convexTransform * pB;
		PfxVector3 normal = nml;
		PfxFloat distance = dot(wB - wA, normal);
		particle.m_curPosition += bias * distance * normal;
	}
}

void pfxContactParticleLargeTriMesh(PfxParticle &particle,const PfxTransform3 &largeMeshTransform,const PfxVector3 &scale,const PfxLargeTriMesh &largeMesh,PfxFloat bias)
{
	PfxSphere sphere(particle.m_radius);
	const PfxLargeTriMeshImpl *lmesh = (PfxLargeTriMeshImpl*)&largeMesh;

	PfxTransform3 particleTransform(PfxMatrix3::identity(), particle.m_curPosition);

	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;

	PfxContactCache localContacts;

	switch (lmesh->getType()) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		pfxContactLargeTriMesh<PfxSphere, PfxExpandedTriMesh>(localContacts, flipTriangle, lmesh, largeMeshTransform, sphere, particleTransform, 0.0f);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		pfxContactLargeTriMeshBvh<PfxSphere, PfxQuantizedTriMeshBvh>(localContacts, flipTriangle, lmesh, largeMeshTransform, sphere, particleTransform, 0.0f);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		pfxContactLargeTriMeshBvh<PfxSphere, PfxCompressedTriMesh>(localContacts, flipTriangle, lmesh, largeMeshTransform, sphere, particleTransform, 0.0f);
		break;
	}

	for (PfxUInt32 i = 0; i < localContacts.getNumContactPoints(); i++) {
		PfxFloat distance = -localContacts.getContactDistance(i);
		PfxVector3 normal = -localContacts.getContactNormal(i);
		particle.m_curPosition += bias * distance * normal;
	}
}

PfxBool pfxGetContactInfoParticleSphere( PfxParticleContactInfo &contact, const PfxParticle &particle, const PfxVector3 &spherePosition, PfxFloat sphereRadius )
{
	PfxVector3 distance = particle.m_curPosition - spherePosition;
	PfxFloat lenSqr = lengthSqr( distance );
	PfxFloat radius = sphereRadius + particle.m_radius;
	if( lenSqr > epsilon && lenSqr < radius * radius ) {
		PfxFloat l = sqrtf( lenSqr );
		PfxVector3 normal = distance / l;
		contact.pointOnShape = spherePosition + sphereRadius * normal;
		contact.depth = l - radius;
		contact.normal = normal;
		return true;
	}
	return false;
}

PfxBool pfxGetContactInfoParticleCapsule( PfxParticleContactInfo &contact, const PfxParticle &particle, const PfxVector3 &capsulePosition1, const PfxVector3 &capsulePosition2, PfxFloat capsuleRadius )
{
	PfxVector3 p = particle.m_curPosition;
	PfxVector3 direction = capsulePosition2 - capsulePosition1;
	PfxFloat t = dot( p - capsulePosition1, direction ) / dot( direction, direction );
	t = SCE_PFX_CLAMP( t, 0.0f, 1.0f );
	PfxVector3 closestPosition = capsulePosition1 + t * direction;
	PfxVector3 distance = p - closestPosition;
	PfxFloat lenSqr = lengthSqr( distance );
	PfxFloat radius = capsuleRadius + particle.m_radius;
	if( lenSqr > epsilon && lenSqr < radius*radius ) {
		PfxFloat l = sqrtf( lenSqr );
		PfxVector3 normal = distance / l;
		contact.pointOnShape = closestPosition + capsuleRadius * normal;
		contact.depth = l - radius;
		contact.normal = normal;
		return true;
	}
	return false;
}

PfxBool pfxGetContactInfoParticlePlane( PfxParticleContactInfo &contact, const PfxParticle &particle, const PfxVector3 &planeNormal, PfxFloat planeDistance )
{
	PfxFloat distance = dot( particle.m_curPosition, planeNormal );
	PfxFloat targetDistance = planeDistance + particle.m_radius;
	if( distance < targetDistance ) {
		contact.pointOnShape = particle.m_curPosition + ( planeDistance - distance ) * planeNormal;
		contact.depth = distance - targetDistance;
		contact.normal = planeNormal;
		return true;
	}
	return false;
}

} // namespace pfxv4
} // namespace sce

