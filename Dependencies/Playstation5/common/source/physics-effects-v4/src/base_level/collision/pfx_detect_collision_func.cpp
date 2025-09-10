/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_shape.h"
#include "../../../include/physics_effects/base_level/collision/pfx_detect_collision_func.h"
#include "pfx_contact_box_box.h"
#include "pfx_contact_box_capsule.h"
#include "pfx_contact_box_sphere.h"
#include "pfx_contact_capsule_capsule.h"
#include "pfx_contact_capsule_sphere.h"
#include "pfx_contact_sphere_sphere.h"
#include "pfx_contact_large_tri_mesh.h"
#include "pfx_closest_large_tri_mesh.h"
#include "pfx_convex_mesh_impl.h"
#include "pfx_gjk_solver.h"
#include "pfx_gjk_support_func.h"
#include "pfx_intersect_common.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Collision Detection Function

void dcDummy(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
	(void)contacts;
	(void)shapeA,(void)offsetTransformA,(void)worldTransformA0,(void)worldTransformA1,(void)shapeIdA;
	(void)shapeB,(void)offsetTransformB,(void)worldTransformB0,(void)worldTransformB1,(void)shapeIdB;
	(void)contactThreshold;
}

void dcBoxBox(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
	(void)worldTransformA1,(void)worldTransformB1;

#ifdef ENABLE_MPR_FOR_PRIMITIVES
	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA, pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	PfxBox boxA = shapeA.getBox();
	PfxBox boxB = shapeB.getBox();

	PfxMpr<PfxBox, PfxBox> collider(&boxA, &boxB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if (err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO, shapeA.getUserData(), shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO, shapeB.getUserData(), shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA - wB, normal), normal, offsetTransformA*pA, offsetTransformB*pB, featureIdA, featureIdB, subA, subB);
	}
#else
	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxBox boxA = shapeA.getBox();
	PfxBox boxB = shapeB.getBox();

	boxA.m_half *= scaleA;
	boxB.m_half *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactBoxBox(nml, pA, pB, &boxA, pfxRemoveScale(worldTransformA0, scaleA), &boxB, pfxRemoveScale(worldTransformB0, scaleB));

	if(d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		contacts.addContactPoint(d, -nml, pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB, subA, subB);
	}
#endif
}

void dcBoxCapsule(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	(void)worldTransformA1,(void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxBox boxA = shapeA.getBox();
	PfxCapsule capsuleB = shapeB.getCapsule();

	boxA.m_half *= scaleA;
	capsuleB.m_halfLen *= scaleB;
	capsuleB.m_radius *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactBoxCapsule(nml, pA, pB, &boxA, pfxRemoveScale(worldTransformA0, scaleA), &capsuleB, pfxRemoveScale(worldTransformB0, scaleB));

	if(d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		contacts.addContactPoint(d, -nml, pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB, subA, subB);
	}
}

void dcBoxSphere(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{

	(void)worldTransformA1,(void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxBox boxA = shapeA.getBox();
	PfxSphere sphereB = shapeB.getSphere();

	boxA.m_half *= scaleA;
	sphereB.m_radius *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactBoxSphere(nml, pA, pB, &boxA, pfxRemoveScale(worldTransformA0, scaleA), &sphereB, pfxRemoveScale(worldTransformB0, scaleB));

	if(d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO, shapeA.getUserData(), shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO, shapeB.getUserData(), shapeIdB);
		contacts.addContactPoint(d, -nml, pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB, subA, subB);
	}
}

void dcCapsuleBox(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{

	(void)worldTransformA1,(void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxCapsule capsuleA = shapeA.getCapsule();
	PfxBox boxB = shapeB.getBox();

	capsuleA.m_radius *= scaleA;
	capsuleA.m_halfLen *= scaleA;
	boxB.m_half *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactBoxCapsule(nml,pB,pA,&boxB,pfxRemoveScale(worldTransformB0, scaleB),&capsuleA,pfxRemoveScale(worldTransformA0, scaleA));

	if(d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		contacts.addContactPoint(d,nml,pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB,subA,subB);
	}
}

void dcCapsuleCapsule(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{

	(void)worldTransformA1,(void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxCapsule capsuleA = shapeA.getCapsule();
	PfxCapsule capsuleB = shapeB.getCapsule();

	capsuleA.m_halfLen *= scaleA;
	capsuleA.m_radius *= scaleA;
	capsuleB.m_halfLen *= scaleB;
	capsuleB.m_radius *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactCapsuleCapsule(nml, pA, pB, &capsuleA, pfxRemoveScale(worldTransformA0, scaleA), &capsuleB, pfxRemoveScale(worldTransformB0, scaleB));

	if(d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		contacts.addContactPoint(d,-nml,pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB,subA,subB);
	}
}

void dcCapsuleSphere(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{

	(void)worldTransformA1,(void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxCapsule capsuleA = shapeA.getCapsule();
	PfxSphere sphereB = shapeB.getSphere();

	capsuleA.m_halfLen *= scaleA;
	capsuleA.m_radius *= scaleA;
	sphereB.m_radius *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactCapsuleSphere(nml,pA,pB,&capsuleA,pfxRemoveScale(worldTransformA0, scaleA),&sphereB,pfxRemoveScale(worldTransformB0, scaleB));

	if(d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		contacts.addContactPoint(d,-nml,pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB,subA,subB);
	}
}

void dcSphereBox(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{

	(void)worldTransformA1,(void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxSphere sphereA = shapeA.getSphere();
	PfxBox boxB = shapeB.getBox();

	sphereA.m_radius *= scaleA;
	boxB.m_half *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactBoxSphere(nml,pB,pA,&boxB,pfxRemoveScale(worldTransformB0, scaleB),&sphereA,pfxRemoveScale(worldTransformA0, scaleA));

	if(d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		contacts.addContactPoint(d,nml,pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB,subA,subB);
	}
}

void dcSphereCapsule(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{

	(void)worldTransformA1,(void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxSphere sphereA = shapeA.getSphere();
	PfxCapsule capsuleB = shapeB.getCapsule();

	sphereA.m_radius *= scaleA;
	capsuleB.m_halfLen *= scaleB;
	capsuleB.m_radius *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactCapsuleSphere(nml,pB,pA,&capsuleB,pfxRemoveScale(worldTransformB0, scaleB),&sphereA,pfxRemoveScale(worldTransformA0, scaleA));

	if(d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		contacts.addContactPoint(d,nml,pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB,subA,subB);
	}
}

void dcSphereSphere(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{

	(void)worldTransformA1,(void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxSphere sphereA = shapeA.getSphere();
	PfxSphere sphereB = shapeB.getSphere();

	sphereA.m_radius *= scaleA;
	sphereB.m_radius *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactSphereSphere(nml, pA, pB, &sphereA, pfxRemoveScale(worldTransformA0, scaleA), &sphereB, pfxRemoveScale(worldTransformB0, scaleB));

	if(d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		contacts.addContactPoint(d,-nml,pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB,subA,subB);
	}
}

void dcCylinderSphere(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	PfxCylinder cylinderA = shapeA.getCylinder();
	PfxSphere sphereB = shapeB.getSphere();

	PfxMpr<PfxCylinder, PfxSphere> collider(&cylinderA, &sphereB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA-wB,normal),normal,offsetTransformA*pA,offsetTransformB*pB,subA,subB);
	}
}

void dcCylinderBox(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	PfxCylinder cylinderA = shapeA.getCylinder();
	PfxBox boxB = shapeB.getBox();

	PfxMpr<PfxCylinder, PfxBox> collider(&cylinderA, &boxB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA-wB,normal),normal,offsetTransformA*pA,offsetTransformB*pB,subA,subB);
	}
}

void dcCylinderCapsule(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	PfxCylinder cylinderA = shapeA.getCylinder();
	PfxCapsule capsuleB = shapeB.getCapsule();

	PfxMpr<PfxCylinder, PfxCapsule> collider(&cylinderA, &capsuleB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA-wB,normal),normal,offsetTransformA*pA,offsetTransformB*pB,subA,subB);
	}
}

void dcCylinderCylinder(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	PfxCylinder cylinderA = shapeA.getCylinder();
	PfxCylinder cylinderB = shapeB.getCylinder();

	PfxMpr<PfxCylinder, PfxCylinder> collider(&cylinderA, &cylinderB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA-wB,normal),normal,offsetTransformA*pA,offsetTransformB*pB,subA,subB);
	}
}

void dcCylinderConvex(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	PfxCylinder cylinderA = shapeA.getCylinder();
	const PfxConvexMeshImpl *convexB = (PfxConvexMeshImpl*)shapeB.getConvexMesh();

	PfxMpr<PfxCylinder, PfxConvexMeshImpl> collider(&cylinderA, convexB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);
	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA-wB,normal),normal,offsetTransformA*pA,offsetTransformB*pB,subA,subB);
	}
}

void dcSphereCylinder(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	PfxSphere sphereA = shapeA.getSphere();
	PfxCylinder cylinderB = shapeB.getCylinder();

	PfxMpr<PfxSphere, PfxCylinder> collider(&sphereA, &cylinderB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA-wB,normal),normal,offsetTransformA*pA,offsetTransformB*pB,subA,subB);
	}
}

void dcBoxCylinder(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	PfxBox boxA = shapeA.getBox();
	PfxCylinder cylinderB = shapeB.getCylinder();

	PfxMpr<PfxBox, PfxCylinder> collider(&boxA, &cylinderB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA-wB,normal),normal,offsetTransformA*pA,offsetTransformB*pB,subA,subB);
	}
}

void dcCapsuleCylinder(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	PfxCapsule capsuleA = shapeA.getCapsule();
	PfxCylinder cylinderB = shapeB.getCylinder();

	PfxMpr<PfxCapsule, PfxCylinder> collider(&capsuleA, &cylinderB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA-wB,normal),normal,offsetTransformA*pA,offsetTransformB*pB,subA,subB);
	}
}

void dcConvexCylinder(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	const PfxConvexMeshImpl *convexA = (PfxConvexMeshImpl*)shapeA.getConvexMesh();
	PfxCylinder cylinderB = shapeB.getCylinder();

	PfxMpr<PfxConvexMeshImpl, PfxCylinder> collider(convexA, &cylinderB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA-wB,normal),normal,offsetTransformA*pA,offsetTransformB*pB,subA,subB);
	}
}

void dcConvexSphere(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	const PfxConvexMeshImpl *convexA = (PfxConvexMeshImpl*)shapeA.getConvexMesh();
	PfxSphere sphereB = shapeB.getSphere();

	PfxMpr<PfxConvexMeshImpl, PfxSphere> collider(convexA, &sphereB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA - wB, normal), normal, offsetTransformA*pA, offsetTransformB*pB, featureIdA, featureIdB, subA, subB);
	}
}

void dcConvexBox(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	const PfxConvexMeshImpl *convexA = (PfxConvexMeshImpl*)shapeA.getConvexMesh();
	PfxBox boxB = shapeB.getBox();

	PfxMpr<PfxConvexMeshImpl, PfxBox> collider(convexA, &boxB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA - wB, normal), normal, offsetTransformA*pA, offsetTransformB*pB, featureIdA, featureIdB, subA, subB);
	}
}

void dcConvexCapsule(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	const PfxConvexMeshImpl *convexA = (PfxConvexMeshImpl*)shapeA.getConvexMesh();
	PfxCapsule capsuleB = shapeB.getCapsule();

	PfxMpr<PfxConvexMeshImpl, PfxCapsule> collider(convexA, &capsuleB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA - wB, normal), normal, offsetTransformA*pA, offsetTransformB*pB, subA, subB);
	}
}

void dcConvexConvex(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	const PfxConvexMeshImpl *convexA = (PfxConvexMeshImpl*)shapeA.getConvexMesh();
	const PfxConvexMeshImpl *convexB = (PfxConvexMeshImpl*)shapeB.getConvexMesh();

	PfxMpr<PfxConvexMeshImpl, PfxConvexMeshImpl> collider(convexA, convexB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(transpose(inverse(worldTransformA0.getUpper3x3())) * nml);
		contacts.addContactPoint(dot(wA - wB, normal), normal, offsetTransformA*pA, offsetTransformB*pB, featureIdA, featureIdB, subA, subB);
	}
}

void dcSphereConvex(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	PfxSphere sphereA = shapeA.getSphere();
	const PfxConvexMeshImpl *convexB = (PfxConvexMeshImpl*)shapeB.getConvexMesh();

	PfxMpr<PfxSphere, PfxConvexMeshImpl> collider(&sphereA, convexB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA - wB, normal), normal, offsetTransformA*pA, offsetTransformB*pB, featureIdA, featureIdB, subA, subB);
	}
}

void dcBoxConvex(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	PfxBox boxA = shapeA.getBox();
	const PfxConvexMeshImpl *convexB = (PfxConvexMeshImpl*)shapeB.getConvexMesh();

	PfxMpr<PfxBox, PfxConvexMeshImpl> collider(&boxA, convexB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA - wB, normal), normal, offsetTransformA*pA, offsetTransformB*pB, featureIdA, featureIdB, subA, subB);
	}
}

void dcCapsuleConvex(PfxContactCache &contacts,
			const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
			const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
			PfxFloat contactThreshold)
{
(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;
	PfxUInt32 featureIdA = 0, featureIdB = 0;

	PfxCapsule capsuleA = shapeA.getCapsule();
	const PfxConvexMeshImpl *convexB = (PfxConvexMeshImpl*)shapeB.getConvexMesh();

	PfxMpr<PfxCapsule, PfxConvexMeshImpl> collider(&capsuleA, convexB);

	PfxInt32 err = collider.collideRetry(d, nml, pA, pB, featureIdA, featureIdB, worldTransformA0, worldTransformB0, contacts.cachedAxis, SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d < contactThreshold) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);
		contacts.addContactPoint(dot(wA-wB,normal),normal,offsetTransformA*pA,offsetTransformB*pB,subA,subB);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Large Tri Mesh

void dcLargeMeshSphere(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	const PfxLargeTriMeshImpl *lmeshA = (PfxLargeTriMeshImpl*)shapeA.getLargeTriMesh();
	PfxSphere sphereB = shapeB.getSphere();

	PfxVector3 scaleA = shapeA.getScaleXyz();
	PfxBool flipTriangle = (scaleA[0] * scaleA[1] * scaleA[2]) < 0.0f;

	PfxContactCache localContacts;

	switch(lmeshA->getType()) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		pfxContactLargeTriMesh<PfxSphere, PfxExpandedTriMesh>(localContacts, flipTriangle, lmeshA, worldTransformA0, sphereB, worldTransformB0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		pfxContactLargeTriMeshBvh<PfxSphere, PfxQuantizedTriMeshBvh>(localContacts, flipTriangle, lmeshA, worldTransformA0, sphereB, worldTransformB0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		pfxContactLargeTriMeshBvh<PfxSphere, PfxCompressedTriMesh>(localContacts, flipTriangle, lmeshA, worldTransformA0, sphereB, worldTransformB0, contactThreshold);
		break;
	}

	PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subA = localContacts.getContactSubDataA(i);
		subA.setShapeId(shapeIdA);
		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			localContacts.getContactNormal(i),
			offsetTransformA * localContacts.getContactLocalPointA(i),
			offsetTransformB * localContacts.getContactLocalPointB(i),
			subA,subB);
	}
}

void dcLargeMeshBox(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	const PfxLargeTriMeshImpl *lmeshA = (PfxLargeTriMeshImpl*)shapeA.getLargeTriMesh();
	PfxBox boxB = shapeB.getBox();

	PfxVector3 scaleA = shapeA.getScaleXyz();
	PfxBool flipTriangle = (scaleA[0] * scaleA[1] * scaleA[2]) < 0.0f;

	PfxContactCache localContacts;

	switch(lmeshA->getType()) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		pfxContactLargeTriMesh<PfxBox, PfxExpandedTriMesh>(localContacts, flipTriangle, lmeshA, worldTransformA0, boxB, worldTransformB0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		pfxContactLargeTriMeshBvh<PfxBox, PfxQuantizedTriMeshBvh>(localContacts, flipTriangle, lmeshA, worldTransformA0, boxB, worldTransformB0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		pfxContactLargeTriMeshBvh<PfxBox, PfxCompressedTriMesh>(localContacts, flipTriangle, lmeshA, worldTransformA0, boxB, worldTransformB0, contactThreshold);
		break;
	}

	PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subA = localContacts.getContactSubDataA(i);
		subA.setShapeId(shapeIdA);
		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			localContacts.getContactNormal(i),
			offsetTransformA * localContacts.getContactLocalPointA(i),
			offsetTransformB * localContacts.getContactLocalPointB(i),
			localContacts.getFeatureIdA(i), localContacts.getFeatureIdB(i),
			subA,subB);
	}
}

void dcLargeMeshCapsule(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	const PfxLargeTriMeshImpl *lmeshA = (PfxLargeTriMeshImpl*)shapeA.getLargeTriMesh();
	PfxCapsule capsuleB = shapeB.getCapsule();

	PfxVector3 scaleA = shapeA.getScaleXyz();
	PfxBool flipTriangle = (scaleA[0] * scaleA[1] * scaleA[2]) < 0.0f;

	PfxContactCache localContacts;

	switch(lmeshA->getType()) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		pfxContactLargeTriMesh<PfxCapsule, PfxExpandedTriMesh>(localContacts, flipTriangle, lmeshA, worldTransformA0, capsuleB, worldTransformB0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		pfxContactLargeTriMeshBvh<PfxCapsule, PfxQuantizedTriMeshBvh>(localContacts, flipTriangle, lmeshA, worldTransformA0, capsuleB, worldTransformB0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		pfxContactLargeTriMeshBvh<PfxCapsule, PfxCompressedTriMesh>(localContacts, flipTriangle, lmeshA, worldTransformA0, capsuleB, worldTransformB0, contactThreshold);
		break;
	}

	PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subA = localContacts.getContactSubDataA(i);
		subA.setShapeId(shapeIdA);
		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			localContacts.getContactNormal(i),
			offsetTransformA * localContacts.getContactLocalPointA(i),
			offsetTransformB * localContacts.getContactLocalPointB(i),
			subA,subB);
	}
}

void dcLargeMeshCylinder(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	const PfxLargeTriMeshImpl *lmeshA = (PfxLargeTriMeshImpl*)shapeA.getLargeTriMesh();
	PfxCylinder cylinderB = shapeB.getCylinder();

	PfxVector3 scaleA = shapeA.getScaleXyz();
	PfxBool flipTriangle = (scaleA[0] * scaleA[1] * scaleA[2]) < 0.0f;

	PfxContactCache localContacts;

	switch(lmeshA->getType()) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		pfxContactLargeTriMesh<PfxCylinder, PfxExpandedTriMesh>(localContacts, flipTriangle, lmeshA, worldTransformA0, cylinderB, worldTransformB0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		pfxContactLargeTriMeshBvh<PfxCylinder, PfxQuantizedTriMeshBvh>(localContacts, flipTriangle, lmeshA, worldTransformA0, cylinderB, worldTransformB0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		pfxContactLargeTriMeshBvh<PfxCylinder, PfxCompressedTriMesh>(localContacts, flipTriangle, lmeshA, worldTransformA0, cylinderB, worldTransformB0, contactThreshold);
		break;
	}

	PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subA = localContacts.getContactSubDataA(i);
		subA.setShapeId(shapeIdA);
		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			localContacts.getContactNormal(i),
			offsetTransformA * localContacts.getContactLocalPointA(i),
			offsetTransformB * localContacts.getContactLocalPointB(i),
			subA,subB);
	}
}

void dcLargeMeshConvex(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	const PfxLargeTriMeshImpl *lmeshA = (PfxLargeTriMeshImpl*)shapeA.getLargeTriMesh();
	const PfxConvexMeshImpl *convexB = (PfxConvexMeshImpl*)shapeB.getConvexMesh();

	PfxVector3 scaleA = shapeA.getScaleXyz();
	PfxVector3 scaleB = shapeB.getScaleXyz();
	PfxBool flipTriangle = (scaleA[0] * scaleA[1] * scaleA[2] * scaleB[0] * scaleB[1] * scaleB[2]) < 0.0f;

	PfxContactCache localContacts;

	switch(lmeshA->getType()) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		pfxContactLargeTriMesh<PfxConvexMeshImpl, PfxExpandedTriMesh>(localContacts, flipTriangle, lmeshA, worldTransformA0, *convexB, worldTransformB0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		pfxContactLargeTriMeshBvh<PfxConvexMeshImpl, PfxQuantizedTriMeshBvh>(localContacts, flipTriangle, lmeshA, worldTransformA0, *convexB, worldTransformB0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		pfxContactLargeTriMeshBvh<PfxConvexMeshImpl, PfxCompressedTriMesh>(localContacts, flipTriangle, lmeshA, worldTransformA0, *convexB, worldTransformB0, contactThreshold);
		break;
	}

	PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subA = localContacts.getContactSubDataA(i);
		subA.setShapeId(shapeIdA);
		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			localContacts.getContactNormal(i),
			offsetTransformA * localContacts.getContactLocalPointA(i),
			offsetTransformB * localContacts.getContactLocalPointB(i),
			localContacts.getFeatureIdA(i), localContacts.getFeatureIdB(i),
			subA,subB);
	}
}

void dcSphereLargeMesh(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	const PfxLargeTriMeshImpl *lmeshB = (PfxLargeTriMeshImpl*)shapeB.getLargeTriMesh();
	PfxSphere sphereA = shapeA.getSphere();

	PfxVector3 scaleB = shapeB.getScaleXyz();
	PfxBool flipTriangle = (scaleB[0] * scaleB[1] * scaleB[2]) < 0.0f;

	PfxContactCache localContacts;

	switch(lmeshB->getType()) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		pfxContactLargeTriMesh<PfxSphere, PfxExpandedTriMesh>(localContacts, flipTriangle, lmeshB, worldTransformB0, sphereA, worldTransformA0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		pfxContactLargeTriMeshBvh<PfxSphere, PfxQuantizedTriMeshBvh>(localContacts, flipTriangle, lmeshB, worldTransformB0, sphereA,worldTransformA0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		pfxContactLargeTriMeshBvh<PfxSphere, PfxCompressedTriMesh>(localContacts, flipTriangle, lmeshB, worldTransformB0, sphereA,worldTransformA0, contactThreshold);
		break;
	}

	PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subB = localContacts.getContactSubDataA(i);
		subB.setShapeId(shapeIdB);
		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			-localContacts.getContactNormal(i),
			offsetTransformA * localContacts.getContactLocalPointB(i),
			offsetTransformB * localContacts.getContactLocalPointA(i),
			subA,subB);
	}
}

void dcBoxLargeMesh(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	const PfxLargeTriMeshImpl *lmeshB = (PfxLargeTriMeshImpl*)shapeB.getLargeTriMesh();
	PfxBox boxA = shapeA.getBox();

	PfxVector3 scaleB = shapeB.getScaleXyz();
	PfxBool flipTriangle = (scaleB[0] * scaleB[1] * scaleB[2]) < 0.0f;

	PfxContactCache localContacts;

	switch(lmeshB->getType()) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		pfxContactLargeTriMesh<PfxBox, PfxExpandedTriMesh>(localContacts, flipTriangle, lmeshB, worldTransformB0, boxA, worldTransformA0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		pfxContactLargeTriMeshBvh<PfxBox, PfxQuantizedTriMeshBvh>(localContacts, flipTriangle, lmeshB, worldTransformB0, boxA,worldTransformA0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		pfxContactLargeTriMeshBvh<PfxBox, PfxCompressedTriMesh>(localContacts, flipTriangle, lmeshB, worldTransformB0, boxA,worldTransformA0, contactThreshold);
		break;
	}

	PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subB = localContacts.getContactSubDataA(i);
		subB.setShapeId(shapeIdB);
		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			-localContacts.getContactNormal(i),
			offsetTransformA * localContacts.getContactLocalPointB(i),
			offsetTransformB * localContacts.getContactLocalPointA(i),
			localContacts.getFeatureIdB(i), localContacts.getFeatureIdA(i),
			subA,subB);
	}
}

void dcCapsuleLargeMesh(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	const PfxLargeTriMeshImpl *lmeshB = (PfxLargeTriMeshImpl*)shapeB.getLargeTriMesh();
	PfxCapsule capsuleA = shapeA.getCapsule();

	PfxVector3 scaleB = shapeB.getScaleXyz();
	PfxBool flipTriangle = (scaleB[0] * scaleB[1] * scaleB[2]) < 0.0f;

	PfxContactCache localContacts;

	switch(lmeshB->getType()) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		pfxContactLargeTriMesh<PfxCapsule, PfxExpandedTriMesh>(localContacts, flipTriangle, lmeshB, worldTransformB0, capsuleA, worldTransformA0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		pfxContactLargeTriMeshBvh<PfxCapsule, PfxQuantizedTriMeshBvh>(localContacts, flipTriangle, lmeshB, worldTransformB0, capsuleA,worldTransformA0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		pfxContactLargeTriMeshBvh<PfxCapsule, PfxCompressedTriMesh>(localContacts, flipTriangle, lmeshB, worldTransformB0, capsuleA,worldTransformA0, contactThreshold);
		break;
	}

	PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subB = localContacts.getContactSubDataA(i);
		subB.setShapeId(shapeIdB);
		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			-localContacts.getContactNormal(i),
			offsetTransformA * localContacts.getContactLocalPointB(i),
			offsetTransformB * localContacts.getContactLocalPointA(i),
			subA,subB);
	}
}

void dcCylinderLargeMesh(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	const PfxLargeTriMeshImpl *lmeshB = (PfxLargeTriMeshImpl*)shapeB.getLargeTriMesh();
	PfxCylinder cylinderA = shapeA.getCylinder();

	PfxVector3 scaleB = shapeB.getScaleXyz();
	PfxBool flipTriangle = (scaleB[0] * scaleB[1] * scaleB[2]) < 0.0f;

	PfxContactCache localContacts;

	switch(lmeshB->getType()) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		pfxContactLargeTriMesh<PfxCylinder, PfxExpandedTriMesh>(localContacts, flipTriangle, lmeshB, worldTransformB0, cylinderA, worldTransformA0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		pfxContactLargeTriMeshBvh<PfxCylinder, PfxQuantizedTriMeshBvh>(localContacts, flipTriangle, lmeshB, worldTransformB0, cylinderA,worldTransformA0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		pfxContactLargeTriMeshBvh<PfxCylinder, PfxCompressedTriMesh>(localContacts, flipTriangle, lmeshB, worldTransformB0, cylinderA,worldTransformA0, contactThreshold);
		break;
	}

	PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subB = localContacts.getContactSubDataA(i);
		subB.setShapeId(shapeIdB);
		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			-localContacts.getContactNormal(i),
			offsetTransformA * localContacts.getContactLocalPointB(i),
			offsetTransformB * localContacts.getContactLocalPointA(i),
			subA,subB);
	}
}

void dcConvexLargeMesh(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	const PfxLargeTriMeshImpl *lmeshB = (PfxLargeTriMeshImpl*)shapeB.getLargeTriMesh();
	const PfxConvexMeshImpl *convexA = (PfxConvexMeshImpl*)shapeA.getConvexMesh();

	PfxVector3 scaleA = shapeA.getScaleXyz();
	PfxVector3 scaleB = shapeB.getScaleXyz();
	PfxBool flipTriangle = (scaleA[0] * scaleA[1] * scaleA[2] * scaleB[0] * scaleB[1] * scaleB[2]) < 0.0f;

	PfxContactCache localContacts;

	switch(lmeshB->getType()) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		pfxContactLargeTriMesh<PfxConvexMeshImpl, PfxExpandedTriMesh>(localContacts, flipTriangle, lmeshB, worldTransformB0, *convexA, worldTransformA0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		pfxContactLargeTriMeshBvh<PfxConvexMeshImpl, PfxQuantizedTriMeshBvh>(localContacts, flipTriangle, lmeshB, worldTransformB0, *convexA,worldTransformA0, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		pfxContactLargeTriMeshBvh<PfxConvexMeshImpl, PfxCompressedTriMesh>(localContacts, flipTriangle, lmeshB, worldTransformB0, *convexA,worldTransformA0, contactThreshold);
		break;
	}

	PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
	for(PfxUInt32 i=0;i<localContacts.getNumContactPoints();i++) {
		PfxSubData subB = localContacts.getContactSubDataA(i);
		subB.setShapeId(shapeIdB);
		contacts.addContactPoint(
			localContacts.getContactDistance(i),
			-localContacts.getContactNormal(i),
			offsetTransformA * localContacts.getContactLocalPointB(i),
			offsetTransformB * localContacts.getContactLocalPointA(i),
			localContacts.getFeatureIdB(i), localContacts.getFeatureIdA(i),
			subA,subB);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Closest Distance Function

static PfxQuat calcCapsuleOrientatioin(const PfxVector3 &capP0, const PfxVector3 &capP1)
{
	PfxVector3 dir = capP1 - capP0;
	PfxFloat len = length(dir);
	if (len < SCE_PFX_INTERSECT_COMMON_EPSILON) return PfxQuat::identity();

	return PfxQuat::rotation(PfxVector3::xAxis(), dir / len);
}

void dtCoreSphereSphere(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
 (void)contactThreshold; (void)worldTransformA1; (void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxSphere sphereA = shapeA.getSphere();
	PfxSphere sphereB = shapeB.getSphere();

	sphereA.m_radius *= scaleA;
	sphereB.m_radius *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactSphereSphere(nml,pA,pB,&sphereA,pfxRemoveScale(worldTransformA0, scaleA),&sphereB,pfxRemoveScale(worldTransformB0, scaleB));

	if(d > 0.0f) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);

		PfxUInt8 ccdPriority = 0;

		// Check if this triangle is in a range of a moving-sphere line
		PfxVector3 capP0 = worldTransformA0.getTranslation();
		PfxVector3 capP1 = worldTransformA1.getTranslation();
		PfxCapsule capsule(0.5f * length(capP0 - capP1), sphereA.m_radius);
		PfxTransform3 capsuleTransform(calcCapsuleOrientatioin(capP0, capP1), 0.5f * (capP0 + capP1));
		PfxVector3 nml_;
		PfxPoint3 pA_,pB_;
		PfxFloat dd = pfxContactCapsuleSphere(nml_, pA_, pB_, &capsule, capsuleTransform, &sphereB, pfxRemoveScale(worldTransformB0, scaleB));
		if(dd < 0.0f) {
			ccdPriority = 1 << 3;
		}

		// Check an angle between the sphere direction and the closest direction (normal)
		PfxVector3 sphereDirection = normalize(capP1 - capP0);
		PfxFloat dirAngle = SCE_PFX_MAX(dot(nml, sphereDirection), 0.0f);
		PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
		ccdPriority = ccdPriority | priority;
		subB.setCcdPriority(ccdPriority);

		contacts.addClosestPoint(d,-nml,pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB,subA,subB);
	}
}

void dtSphereCoreSphere( PfxContactCache &contacts,
	const PfxShape &shapeA, const PfxTransform3 &offsetTransformA, const PfxTransform3 &worldTransformA0, const PfxTransform3 &worldTransformA1, int shapeIdA,
	const PfxShape &shapeB, const PfxTransform3 &offsetTransformB, const PfxTransform3 &worldTransformB0, const PfxTransform3 &worldTransformB1, int shapeIdB,
	PfxFloat contactThreshold)
{
 (void)contactThreshold; (void)worldTransformA1; (void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxSphere sphereA = shapeA.getSphere();
	PfxSphere sphereB = shapeB.getSphere();

	sphereA.m_radius *= scaleA;
	sphereB.m_radius *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA, pB;

	PfxFloat d = pfxContactSphereSphere(nml, pA, pB, &sphereA, pfxRemoveScale(worldTransformA0, scaleA), &sphereB, pfxRemoveScale(worldTransformB0, scaleB));

	if (d > 0.0f) {
		PfxSubData subA(PfxSubData::SHAPE_INFO, shapeA.getUserData(), shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO, shapeB.getUserData(), shapeIdB);

		PfxUInt8 ccdPriority = 0;

		// Check if this triangle is in a range of a moving-sphere line
		PfxVector3 capP0 = worldTransformB0.getTranslation();
		PfxVector3 capP1 = worldTransformB1.getTranslation();
		PfxCapsule capsule(0.5f * length(capP0 - capP1), sphereB.m_radius);
		PfxTransform3 capsuleTransform(calcCapsuleOrientatioin(capP0, capP1), 0.5f * (capP0 + capP1));
		PfxVector3 nml_;
		PfxPoint3 pA_, pB_;
		PfxFloat dd = pfxContactCapsuleSphere(nml_, pA_, pB_, &capsule, capsuleTransform, &sphereA, pfxRemoveScale(worldTransformA0, scaleA));
		if (dd < 0.0f) {
			ccdPriority = 1 << 3;
		}

		// Check an angle between the sphere direction and the closest direction (normal)
		PfxVector3 sphereDirection = normalize(capP1 - capP0);
		PfxFloat dirAngle = SCE_PFX_MAX(dot(-nml, sphereDirection), 0.0f);
		PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
		ccdPriority = ccdPriority | priority;
		subA.setCcdPriority(ccdPriority);

		contacts.addClosestPoint(d, -nml, pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB, subA, subB);
	}
}

void dtSphereBox(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
 (void)contactThreshold; (void)worldTransformA1; (void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxSphere sphereA = shapeA.getSphere();
	PfxBox boxB = shapeB.getBox();

	sphereA.m_radius *= scaleA;
	boxB.m_half *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactBoxSphere(nml,pB,pA,&boxB,pfxRemoveScale(worldTransformB0, scaleB),&sphereA,pfxRemoveScale(worldTransformA0, scaleA));

	if(d > 0.0f) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		
		PfxUInt8 ccdPriority = 0;
		
		// Check if this triangle is in a range of a moving-sphere line
		PfxVector3 capP0 = worldTransformA0.getTranslation();
		PfxVector3 capP1 = worldTransformA1.getTranslation();
		PfxCapsule capsule(0.5f * length(capP0 - capP1), sphereA.m_radius);
		PfxTransform3 capsuleTransform(calcCapsuleOrientatioin(capP0, capP1), 0.5f * (capP0 + capP1));
		PfxVector3 nml_;
		PfxPoint3 pA_,pB_;
		PfxFloat dd = pfxContactBoxCapsule(nml_, pA_, pB_, &boxB, pfxRemoveScale(worldTransformB0, scaleB), &capsule, capsuleTransform);
		if(dd < 0.0f) {
			ccdPriority = 1 << 3;
		}

		// Check an angle between the sphere direction and the closest direction (normal)
		PfxVector3 sphereDirection = normalize(capP1 - capP0);
		PfxFloat dirAngle = SCE_PFX_MAX(dot(-nml, sphereDirection), 0.0f);
		PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
		ccdPriority = ccdPriority | priority;
		subB.setCcdPriority(ccdPriority);
		
		contacts.addClosestPoint(d,nml,pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB,subA,subB);
	}
}

void dtBoxSphere(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
 (void)contactThreshold; (void)worldTransformA1; (void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxBox boxA = shapeA.getBox();
	PfxSphere sphereB = shapeB.getSphere();

	boxA.m_half *= scaleA;
	sphereB.m_radius *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactBoxSphere(nml,pA,pB,&boxA,pfxRemoveScale(worldTransformA0, scaleA),&sphereB,pfxRemoveScale(worldTransformB0, scaleB));
	if(d > 0.0f) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		
		PfxUInt8 ccdPriority = 0;
		
		// Check if this triangle is in a range of a moving-sphere line
		PfxVector3 capP0 = worldTransformB0.getTranslation();
		PfxVector3 capP1 = worldTransformB1.getTranslation();
		PfxCapsule capsule(0.5f * length(capP0 - capP1), sphereB.m_radius);
		PfxTransform3 capsuleTransform(calcCapsuleOrientatioin(capP0, capP1), 0.5f * (capP0 + capP1));
		PfxVector3 nml_;
		PfxPoint3 pA_, pB_;
		PfxFloat dd = pfxContactBoxCapsule(nml_, pA_, pB_, &boxA, pfxRemoveScale(worldTransformA0, scaleA), &capsule, capsuleTransform);
		if (dd < 0.0f) {
			ccdPriority = 1 << 3;
		}
		
		// Check an angle between the sphere direction and the closest direction (normal)
		PfxVector3 sphereDirection = normalize(capP1 - capP0);
		PfxFloat dirAngle = SCE_PFX_MAX(dot(-nml, sphereDirection), 0.0f);
		PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
		ccdPriority = ccdPriority | priority;
		subA.setCcdPriority(ccdPriority);
		
		contacts.addClosestPoint(d,-nml,pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB,subA,subB);
	}
}

void dtSphereCapsule(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
 (void)contactThreshold; (void)worldTransformA1; (void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxSphere sphereA = shapeA.getSphere();
	PfxCapsule capsuleB = shapeB.getCapsule();

	sphereA.m_radius *= scaleA;
	capsuleB.m_halfLen *= scaleB;
	capsuleB.m_radius *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactCapsuleSphere(nml,pB,pA,&capsuleB,pfxRemoveScale(worldTransformB0, scaleB),&sphereA,pfxRemoveScale(worldTransformA0, scaleA));
	if(d > 0.0f) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		
		PfxUInt8 ccdPriority = 0;
		
		// Check if this triangle is in a range of a moving-sphere line
		PfxVector3 capP0 = worldTransformA0.getTranslation();
		PfxVector3 capP1 = worldTransformA1.getTranslation();
		PfxCapsule capsule(0.5f * length(capP0 - capP1), sphereA.m_radius);
		PfxTransform3 capsuleTransform(calcCapsuleOrientatioin(capP0, capP1), 0.5f * (capP0 + capP1));
		PfxVector3 nml_;
		PfxPoint3 pA_,pB_;
		PfxFloat dd = pfxContactCapsuleCapsule(nml_, pA_, pB_, &capsuleB, pfxRemoveScale(worldTransformB0, scaleB), &capsule, capsuleTransform);
		if(dd < 0.0f) {
			ccdPriority = 1 << 3;
		}
		
		// Check an angle between the sphere direction and the closest direction (normal)
		PfxVector3 sphereDirection = normalize(capP1 - capP0);
		PfxFloat dirAngle = SCE_PFX_MAX(dot(-nml, sphereDirection), 0.0f);
		PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
		ccdPriority = ccdPriority | priority;
		subB.setCcdPriority(ccdPriority);

		contacts.addClosestPoint(d,nml,pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB,subA,subB);
	}
}

void dtCapsuleSphere(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
 (void)contactThreshold; (void)worldTransformA1; (void)worldTransformB1;

	PfxFloat scaleA = shapeA.getScale();
	PfxFloat scaleB = shapeB.getScale();

	PfxCapsule capsuleA = shapeA.getCapsule();
	PfxSphere sphereB = shapeB.getSphere();

	capsuleA.m_halfLen *= scaleA;
	capsuleA.m_radius *= scaleA;
	sphereB.m_radius *= scaleB;

	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxFloat d = pfxContactCapsuleSphere(nml,pA,pB,&capsuleA,pfxRemoveScale(worldTransformA0, scaleA),&sphereB,pfxRemoveScale(worldTransformB0, scaleB));
	if(d > 0.0f) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		
		PfxUInt8 ccdPriority = 0;
		
		// Check if this triangle is in a range of a moving-sphere line
		PfxVector3 capP0 = worldTransformB0.getTranslation();
		PfxVector3 capP1 = worldTransformB1.getTranslation();
		PfxCapsule capsule(0.5f * length(capP0 - capP1), sphereB.m_radius);
		PfxTransform3 capsuleTransform(calcCapsuleOrientatioin(capP0, capP1), 0.5f * (capP0 + capP1));
		PfxVector3 nml_;
		PfxPoint3 pA_, pB_;
		PfxFloat dd = pfxContactCapsuleCapsule(nml_, pA_, pB_, &capsuleA, pfxRemoveScale(worldTransformA0, scaleA), &capsule, capsuleTransform);
		if (dd < 0.0f) {
			ccdPriority = 1 << 3;
		}

		// Check an angle between the sphere direction and the closest direction (normal)
		PfxVector3 sphereDirection = normalize(capP1 - capP0);
		PfxFloat dirAngle = SCE_PFX_MAX(dot(-nml, sphereDirection), 0.0f);
		PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
		ccdPriority = ccdPriority | priority;
		subA.setCcdPriority(ccdPriority);

		contacts.addClosestPoint(d,-nml,pfxRemoveScale(offsetTransformA, scaleA)*pA, pfxRemoveScale(offsetTransformB, scaleB)*pB,subA,subB);
	}
}

void dtSphereCylinder(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	(void)contactThreshold;(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxSphere sphereA = shapeA.getSphere();
	PfxCylinder cylinderB = shapeB.getCylinder();

	PfxGjk<PfxSphere, PfxCylinder> collider(&sphereA,&cylinderB);

	PfxInt32 err = collider.closest(d,nml,pA,pB,worldTransformA0,worldTransformB0,SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk &&  d > 0.0f && d < SCE_PFX_FLT_MAX) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		
		PfxUInt8 ccdPriority = 0;
		
		// Check if this triangle is in a range of a moving-sphere line
		PfxVector3 capP0 = worldTransformA0.getTranslation();
		PfxVector3 capP1 = worldTransformA1.getTranslation();
		PfxCapsule capsule(0.5f * length(capP0 - capP1), sphereA.m_radius);
		PfxTransform3 capsuleTransform(calcCapsuleOrientatioin(capP0, capP1), 0.5f * (capP0 + capP1));

		PfxGjk<PfxCapsule, PfxCylinder> collider_sub(&capsule, &cylinderB);

		PfxVector3 nml_;
		PfxPoint3 pA_,pB_;
		PfxFloat dd;
		err = collider_sub.closest(dd,nml_,pA_,pB_,capsuleTransform,worldTransformB0,SCE_PFX_FLT_MAX);
		if(err == kPfxGjkResultIntersect) {
			ccdPriority = 1 << 3;
		}

		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);

		// Check an angle between the sphere direction and the closest direction (normal)
		PfxVector3 sphereDirection = normalize(capP1 - capP0);
		PfxFloat dirAngle = SCE_PFX_MAX(dot(-normal, sphereDirection), 0.0f);
		PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
		ccdPriority = ccdPriority | priority;
		subB.setCcdPriority(ccdPriority);

		PfxPoint3 wA = worldTransformA0*pA;
		PfxPoint3 wB = worldTransformB0*pB;
		contacts.addClosestPoint(dot(wA-wB,normal),normal,offsetTransformA*pA,offsetTransformB*pB,subA,subB);
	}
}

void dtSphereConvex(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	(void)contactThreshold;(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxSphere sphereA = shapeA.getSphere();
	const PfxConvexMeshImpl *convexB = (PfxConvexMeshImpl*)shapeB.getConvexMesh();

	PfxGjk<PfxSphere, PfxConvexMeshImpl> collider(&sphereA,convexB);

	PfxInt32 err = collider.closest(d,nml,pA,pB,worldTransformA0,worldTransformB0,SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d > 0.0f && d < SCE_PFX_FLT_MAX) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		
		PfxUInt8 ccdPriority = 0;
		
		// Check if this triangle is in a range of a moving-sphere line
		PfxVector3 capP0 = worldTransformA0.getTranslation();
		PfxVector3 capP1 = worldTransformA1.getTranslation();
		PfxCapsule capsule(0.5f * length(capP0 - capP1), sphereA.m_radius);
		PfxTransform3 capsuleTransform(calcCapsuleOrientatioin(capP0, capP1), 0.5f * (capP0 + capP1));

		PfxGjk<PfxCapsule, PfxConvexMeshImpl> collider_sub(&capsule, convexB);

		PfxVector3 nml_;
		PfxPoint3 pA_,pB_;
		PfxFloat dd;
		err = collider_sub.closest(dd,nml_,pA_,pB_,capsuleTransform,worldTransformB0,SCE_PFX_FLT_MAX);
		if(err == kPfxGjkResultIntersect) {
			ccdPriority = 1 << 3;
		}

		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);

		// Check an angle between the sphere direction and the closest direction (normal)
		PfxVector3 sphereDirection = normalize(capP1 - capP0);
		PfxFloat dirAngle = SCE_PFX_MAX(dot(-normal, sphereDirection), 0.0f);
		PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
		ccdPriority = ccdPriority | priority;
		subB.setCcdPriority(ccdPriority);
		
		// 近接点が球上に配置されるように補正する
		pA = PfxPoint3(sphereA.m_radius * -nml);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		contacts.addClosestPoint(dot(wA-wB,normal),normal,offsetTransformA*pA,offsetTransformB*pB,subA,subB);
	}
}

void dtCylinderSphere(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	(void)contactThreshold;(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;

	PfxCylinder cylinderA = shapeA.getCylinder();
	PfxSphere sphereB = shapeB.getSphere();

	PfxGjk<PfxCylinder, PfxSphere> collider(&cylinderA,&sphereB);

	PfxInt32 err = collider.closest(d,nml,pA,pB,worldTransformA0,worldTransformB0,SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk &&  d > 0.0f && d < SCE_PFX_FLT_MAX) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
		
		PfxUInt8 ccdPriority = 0;
		
		// Check if this triangle is in a range of a moving-sphere line
		PfxVector3 capP0 = worldTransformB0.getTranslation();
		PfxVector3 capP1 = worldTransformB1.getTranslation();
		PfxCapsule capsule(0.5f * length(capP0 - capP1), sphereB.m_radius);
		PfxTransform3 capsuleTransform(calcCapsuleOrientatioin(capP0, capP1), 0.5f * (capP0 + capP1));

		PfxGjk<PfxCylinder, PfxCapsule> collider_sub(&cylinderA, &capsule);

		PfxVector3 nml_;
		PfxPoint3 pA_,pB_;
		PfxFloat dd;
		err = collider_sub.closest(dd,nml_,pA_,pB_,worldTransformA0,capsuleTransform,SCE_PFX_FLT_MAX);
		if (err == kPfxGjkResultIntersect || fabsf(dd) < SCE_PFX_INTERSECT_COMMON_EPSILON) {
			ccdPriority = 1 << 3;
		}

		PfxVector3 normal = normalize(worldTransformA0.getUpper3x3() * nml);

		// Check an angle between the sphere direction and the closest direction (normal)
		PfxVector3 sphereDirection = normalize(capP1 - capP0);
		PfxFloat dirAngle = SCE_PFX_MAX(dot(normal, sphereDirection), 0.0f);
		PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
		ccdPriority = ccdPriority | priority;
		subA.setCcdPriority(ccdPriority);

		PfxPoint3 wA = worldTransformA0*pA;
		PfxPoint3 wB = worldTransformB0*pB;
		contacts.addClosestPoint(dot(wA-wB,normal), normal, offsetTransformA*pA, offsetTransformB*pB, subA, subB);
	}
}


void dtConvexSphere(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	(void)contactThreshold;(void)worldTransformA1;(void)worldTransformB1;

	PfxFloat d = SCE_PFX_FLT_MAX;
	PfxVector3 nml;
	PfxPoint3 pA,pB;

	const PfxConvexMeshImpl *convexA = (PfxConvexMeshImpl*)shapeA.getConvexMesh();
	PfxSphere sphereB = shapeB.getSphere();

	PfxGjk<PfxSphere, PfxConvexMeshImpl> collider(&sphereB, convexA);

	PfxInt32 err = collider.closest(d,nml,pB,pA,worldTransformB0,worldTransformA0,SCE_PFX_FLT_MAX);

	if(err == kPfxGjkResultOk && d > 0.0f && d < SCE_PFX_FLT_MAX) {
		PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
		PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);

		PfxUInt8 ccdPriority = 0;

		// Check if this triangle is in a range of a moving-sphere line
		PfxVector3 capP0 = worldTransformB0.getTranslation();
		PfxVector3 capP1 = worldTransformB1.getTranslation();
		PfxCapsule capsule(0.5f * length(capP0 - capP1), sphereB.m_radius);
		PfxTransform3 capsuleTransform(calcCapsuleOrientatioin(capP0, capP1), 0.5f * (capP0 + capP1));

		PfxGjk<PfxConvexMeshImpl, PfxCapsule> collider_sub(convexA, &capsule);

		PfxVector3 nml_;
		PfxPoint3 pA_,pB_;
		PfxFloat dd;
		err = collider_sub.closest(dd,nml_,pA_,pB_,worldTransformA0,capsuleTransform,SCE_PFX_FLT_MAX);
		if(err == kPfxGjkResultIntersect) {
			ccdPriority = 1 << 3;
		}

		PfxVector3 normal = -normalize(worldTransformB0.getUpper3x3() * nml);

		// Check an angle between the sphere direction and the closest direction (normal)
		PfxVector3 sphereDirection = normalize(capP1 - capP0);
		PfxFloat dirAngle = SCE_PFX_MAX(dot(normal, sphereDirection), 0.0f);
		PfxUInt8 priority = (PfxUInt8)roundf(7.0f * dirAngle);
		ccdPriority = ccdPriority | priority;
		subA.setCcdPriority(ccdPriority);

		// 近接点が球上に配置されるように補正する
		pB = PfxPoint3(sphereB.m_radius * -nml);
		PfxPoint3 wA = worldTransformA0 * pA;
		PfxPoint3 wB = worldTransformB0 * pB;
		contacts.addClosestPoint(dot(wA-wB,normal),normal,offsetTransformA*pA,offsetTransformB*pB,subA,subB);
	}
}

void dtLargeMeshSphere(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	(void)contactThreshold;
	(void)shapeIdA,(void)shapeIdB;

	const PfxLargeTriMeshImpl *lmeshA = (PfxLargeTriMeshImpl*)shapeA.getLargeTriMesh();
	PfxSphere sphereB = shapeB.getSphere();

	PfxVector3 scaleA = shapeA.getScaleXyz();
	PfxBool flipTriangle = (scaleA[0] * scaleA[1] * scaleA[2]) < 0.0f;

	PfxContactCache localContacts;

	switch(lmeshA->getType()) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		pfxClosestLargeTriMesh<PfxSphere, PfxExpandedTriMesh>(localContacts, flipTriangle, lmeshA,worldTransformA0, sphereB, worldTransformB0, worldTransformB1, contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		pfxClosestLargeTriMeshBvh<PfxSphere, PfxQuantizedTriMeshBvh>(localContacts, flipTriangle, lmeshA, worldTransformA0, sphereB, worldTransformB0, worldTransformB1,contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		pfxClosestLargeTriMeshBvh<PfxSphere, PfxCompressedTriMesh>(localContacts, flipTriangle, lmeshA, worldTransformA0, sphereB, worldTransformB0, worldTransformB1,contactThreshold);
		break;
	}

	PfxSubData subB(PfxSubData::SHAPE_INFO,shapeB.getUserData(),shapeIdB);
	for(int i=0;i<localContacts.getNumClosestPoints();i++) {
		PfxSubData subA = localContacts.getClosestSubDataA(i);
		subA.setShapeId(shapeIdA);

		contacts.addClosestPoint(
			localContacts.getClosestDistance(i),
			localContacts.getClosestNormal(i),
			offsetTransformA * localContacts.getClosestLocalPointA(i),
			offsetTransformB * localContacts.getClosestLocalPointB(i),
			subA,subB);
	}
}

void dtSphereLargeMesh(PfxContactCache &contacts,
				const PfxShape &shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape &shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold)
{
	(void)contactThreshold;
	(void)shapeIdA,(void)shapeIdB;

	const PfxLargeTriMeshImpl *lmeshB = (PfxLargeTriMeshImpl*)shapeB.getLargeTriMesh();
	PfxSphere sphereA = shapeA.getSphere();

	PfxVector3 scaleB = shapeB.getScaleXyz();
	PfxBool flipTriangle = (scaleB[0] * scaleB[1] * scaleB[2]) < 0.0f;

	PfxContactCache localContacts;

	switch(lmeshB->getType()) {
		case SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY:
		pfxClosestLargeTriMesh<PfxSphere, PfxExpandedTriMesh>(localContacts, flipTriangle, lmeshB, worldTransformB0, sphereA,worldTransformA0,worldTransformA1,contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH:
		pfxClosestLargeTriMeshBvh<PfxSphere, PfxQuantizedTriMeshBvh>(localContacts, flipTriangle, lmeshB, worldTransformB0, sphereA,worldTransformA0,worldTransformA1,contactThreshold);
		break;

		case SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION:
		pfxClosestLargeTriMeshBvh<PfxSphere, PfxCompressedTriMesh>(localContacts, flipTriangle, lmeshB, worldTransformB0, sphereA,worldTransformA0,worldTransformA1,contactThreshold);
		break;
	}

	PfxSubData subA(PfxSubData::SHAPE_INFO,shapeA.getUserData(),shapeIdA);
	for(int i=0;i<localContacts.getNumClosestPoints();i++) {
		PfxSubData subB = localContacts.getClosestSubDataA(i);
		subB.setShapeId(shapeIdB);

		contacts.addClosestPoint(
			localContacts.getClosestDistance(i),
			-localContacts.getClosestNormal(i),
			offsetTransformA * localContacts.getClosestLocalPointB(i),
			offsetTransformB * localContacts.getClosestLocalPointA(i),
			subA,subB);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Collision Detection Function Table

pfx_detect_collision_func funcTbl_dc[kPfxShapeCount][kPfxShapeCount] = {
					/* Sphere,Box,Capsule,Cylinder,Convex,LargeMesh,CoreSphere,Reserved,Reserved,Reserved,Reserved,Reserved */
/* Sphere */		{dcSphereSphere,dcSphereBox,dcSphereCapsule,dcSphereCylinder,dcSphereConvex,dcSphereLargeMesh,dtSphereCoreSphere,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy},
/* Box */			{dcBoxSphere,dcBoxBox,dcBoxCapsule,dcBoxCylinder,dcBoxConvex,dcBoxLargeMesh,dtBoxSphere,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy},
/* Capsule */		{dcCapsuleSphere,dcCapsuleBox,dcCapsuleCapsule,dcCapsuleCylinder,dcCapsuleConvex,dcCapsuleLargeMesh,dtCapsuleSphere,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy},
/* Cylinder */		{dcCylinderSphere,dcCylinderBox,dcCylinderCapsule,dcCylinderCylinder,dcCylinderConvex,dcCylinderLargeMesh,dtCylinderSphere,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy},
/* Convex */		{dcConvexSphere,dcConvexBox,dcConvexCapsule,dcConvexCylinder,dcConvexConvex,dcConvexLargeMesh,dtConvexSphere,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy},
/* LargeMesh */		{dcLargeMeshSphere,dcLargeMeshBox,dcLargeMeshCapsule,dcLargeMeshCylinder,dcLargeMeshConvex,dcDummy,dtLargeMeshSphere,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy},
/* CoreSphere */	{dtCoreSphereSphere,dtSphereBox,dtSphereCapsule,dtSphereCylinder,dtSphereConvex,dtSphereLargeMesh,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy},
/* Reserved */		{dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy},
/* Reserved */		{dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy},
/* Reserved */		{dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy},
/* Reserved */		{dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy},
/* Reserved */		{dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy,dcDummy},
};

///////////////////////////////////////////////////////////////////////////////
// Collision Detection Function Table Interface

pfx_detect_collision_func pfxGetDetectCollisionFunc(PfxUInt8 shapeTypeA,PfxUInt8 shapeTypeB)
{
	SCE_PFX_ASSERT(shapeTypeA<kPfxShapeCount);
	SCE_PFX_ASSERT(shapeTypeB<kPfxShapeCount);
	return funcTbl_dc[shapeTypeA][shapeTypeB];
}

int pfxSetDetectCollisionFunc(PfxUInt8 shapeTypeA,PfxUInt8 shapeTypeB,pfx_detect_collision_func func)
{
	if(shapeTypeA >= kPfxShapeCount || shapeTypeB >= kPfxShapeCount) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}

	funcTbl_dc[shapeTypeA][shapeTypeB] = func;

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
