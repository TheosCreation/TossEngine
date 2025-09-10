/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_debug_render_impl.h"
#include "../base_level/collision/pfx_intersect_common.h"
#include "../src/base_level/collision/pfx_convex_mesh_impl.h"

#include <random>

namespace sce {
namespace pfxv4 {

PfxUInt32 PfxDebugRenderImpl::getBytes()
{
	PfxUInt32 bytes = 16;
	bytes += PfxHashArray<PfxUInt8>::getBytes(SCE_PFX_DRENDER_VISIBLE_NUM); // SCE_PFX_DRENDER_VISIBLE_NUM
	bytes += sizeof(PfxVector3) * (kPfxDebugRenderColorCount + SCE_PFX_DRENDER_RANDOM_COLOR_NUM);// colors
	return bytes;
}

void PfxDebugRenderImpl::initialize(
		pfxDebugRenderPointFunc pointFunc,
		pfxDebugRenderLineFunc lineFunc,
		pfxDebugRenderArcFunc arcFunc,
		pfxDebugRenderAabbFunc aabbFunc,
		pfxDebugRenderBoxFunc boxFunc,
		void *workBuff,PfxUInt32 workBytes)
{
	m_pool.initialize((PfxUInt8*)workBuff,workBytes);
	m_visibleFlags.initialize(&m_pool,SCE_PFX_DRENDER_VISIBLE_NUM);
	m_colors = (PfxVector3*)m_pool.allocate(sizeof(PfxVector3) * (kPfxDebugRenderColorCount + SCE_PFX_DRENDER_RANDOM_COLOR_NUM));
	
	m_scale = 1.0f;
	
	m_normalLength = 0.5f;
	
	m_numPlanes = 0;
	
	m_renderPointFunc = pointFunc;
	m_renderLineFunc  = lineFunc;
	m_renderArcFunc   = arcFunc;
	m_renderAabbFunc  = aabbFunc;
	m_renderBoxFunc   = boxFunc;
	
	m_colors[kPfxDebugRenderColorAabb] = PfxVector3(0.0f,0.0f,1.0f);
	m_colors[kPfxDebugRenderColorSimulationIsland] = PfxVector3(0.0f,1.0f,0.0f);
	m_colors[kPfxDebugRenderColorContactPointA] = PfxVector3(1.0f,0.0f,0.0f);
	m_colors[kPfxDebugRenderColorContactPointB] = PfxVector3(0.0f,0.8f,0.5f);
	m_colors[kPfxDebugRenderColorContactNormal] = PfxVector3(0.0f,0.0f,0.8f);
	m_colors[kPfxDebugRenderColorClosest] = PfxVector3(0.0f,0.8f,1.0f);
	m_colors[kPfxDebugRenderColorClosestHits] = PfxVector3(1.0f, 0.8f, 0.0f);
	m_colors[kPfxDebugRenderColorMeshIsland] = PfxVector3::xAxis();
	m_colors[kPfxDebugRenderColorMeshEdge] = PfxVector3(0.2f,0.2f,0.2f);
	m_colors[kPfxDebugRenderColorMeshEdgeConvex] = PfxVector3(1.0f,1.0f,0.0f);
	m_colors[kPfxDebugRenderColorFacetEdge] = PfxVector3(1.0f,0.0f,0.0f);
	m_colors[kPfxDebugRenderColorFacetAabb] = PfxVector3(0.4f,0.4f,0.4f);
	m_colors[kPfxDebugRenderColorFacetNormal] = PfxVector3(0.3f,0.9f,0.3f);
	m_colors[kPfxDebugRenderColorFacetNormal2] = PfxVector3(0.0f,0.0f,0.8f);
	m_colors[kPfxDebugRenderColorJointAxisX] = PfxVector3(1.0f,0.0f,0.0f);
	m_colors[kPfxDebugRenderColorJointAxisY] = PfxVector3(0.0f,1.0f,0.0f);
	m_colors[kPfxDebugRenderColorJointAxisZ] = PfxVector3(0.0f,0.0f,1.0f);
	m_colors[kPfxDebugRenderColorJointAnchorA] = PfxVector3(1.0f,0.0f,0.0f);
	m_colors[kPfxDebugRenderColorJointAnchorB] = PfxVector3(0.0f,1.0f,0.0f);
	m_colors[kPfxDebugRenderColorJointArc1] = PfxVector3(1.0f,0.0f,0.0f);
	m_colors[kPfxDebugRenderColorJointArc2] = PfxVector3(0.0f,1.0f,0.0f);
	m_colors[kPfxDebugRenderColorPrimitive] = PfxVector3(0.0f, 1.0f, 0.0f);
	m_colors[kPfxDebugRenderColorCoreSphere] = PfxVector3(0.0f, 0.0f, 1.0f);

	std::minstd_rand rand(9999);
	std::uniform_int_distribution<> randGen(0, 1000);

	for(unsigned int i=kPfxDebugRenderColorCount;i<kPfxDebugRenderColorCount + SCE_PFX_DRENDER_RANDOM_COLOR_NUM;i++){
		m_colors[i][0] = 0.25f + 0.75f * randGen(rand) / 1000.0f;
		m_colors[i][1] = 0.25f + 0.75f * randGen(rand) / 1000.0f;
		m_colors[i][2] = 0.25f + 0.75f * randGen(rand) / 1000.0f;
	}
}

void PfxDebugRenderImpl::finalize()
{
}

void PfxDebugRenderImpl::clear()
{
	m_segment = PfxSegment();
	m_numPlanes = 0;
	m_visibleFlags.clear();
}

void PfxDebugRenderImpl::setFrustum(const PfxVector4 *planes, int numPlanes)
{
	int n = SCE_PFX_MIN(numPlanes,SCE_PFX_DRENDER_FRUSTUM_PLANES);
	for(int i=0;i<n;i++) {
		m_planes[i] = planes[i];
	}
	m_numPlanes = n;
}

void PfxDebugRenderImpl::setSegment(const PfxSegment &origin)
{
	m_segment = origin;
}

void PfxDebugRenderImpl::renderLocalAxis(const PfxRigidState *states,const PfxUInt32 numRigidbodies)
{
	SCE_PFX_ASSERT_MSG(m_renderLineFunc,"PfxDebugRenderImpl: render Line function is not set.\n");

	for(PfxUInt32 i=0;i<numRigidbodies;i++) {
		if(isVisible(i)){
			const PfxRigidState &state = states[i];

			PfxLargePosition lpos = state.getLargePosition();
			lpos.changeSegment(m_segment);
			
			PfxMatrix3 ori(state.getOrientation());
			ori *= m_scale;
			
			if(isInsideFrustum(lpos.offset,m_scale)) {
				m_renderLineFunc(lpos.offset,lpos.offset+ori.getCol0(),m_colors[kPfxDebugRenderColorJointAxisX]);
				m_renderLineFunc(lpos.offset,lpos.offset+ori.getCol1(),m_colors[kPfxDebugRenderColorJointAxisY]);
				m_renderLineFunc(lpos.offset,lpos.offset+ori.getCol2(),m_colors[kPfxDebugRenderColorJointAxisZ]);
			}
		}
	}
}

void PfxDebugRenderImpl::renderAabb(const PfxRigidState *states,const PfxCollidable *collidables,const PfxUInt32 numRigidbodies)
{
	SCE_PFX_ASSERT_MSG(m_renderAabbFunc,"PfxDebugRenderImpl: render Aabb function is not set.\n");

	for(PfxUInt32 i=0;i<numRigidbodies;i++) {
		if(isVisible(i)){
			const PfxRigidState &state = states[i];
			const PfxCollidable &coll = collidables[i];

			PfxLargePosition lpos = state.getLargePosition();
			lpos.changeSegment(m_segment);
			
			PfxMatrix3 ori(state.getOrientation());
			PfxVector3 center = lpos.offset + ori * coll.getCenter();
			PfxVector3 half   = absPerElem(ori) * coll.getHalf();
			
			if(isInsideFrustum(center,length(half))) {
				m_renderAabbFunc(center,half,m_colors[kPfxDebugRenderColorAabb]);
			}
		}
	}
}

PfxBool renderAabbCallback(PfxUInt32 proxyId,
	const PfxLargePosition &aabbMin,const PfxLargePosition &aabbMax,void *userData)
{
	PfxDebugRenderImpl *impl = (PfxDebugRenderImpl*)userData;
	
	if(!impl->isVisible(proxyId)) return true;
	
	PfxLargePosition aabbMin_ = aabbMin;
	PfxLargePosition aabbMax_ = aabbMax;
	aabbMin_.changeSegment(impl->m_segment);
	aabbMax_.changeSegment(impl->m_segment);
	PfxVector3 center = (aabbMax_.offset + aabbMin_.offset) * 0.5f;
	PfxVector3 half   = (aabbMax_.offset - aabbMin_.offset) * 0.5f;
	if(impl->isInsideFrustum(center,length(half))) {
		impl->m_renderAabbFunc(center,half,impl->m_colors[kPfxDebugRenderColorAabb]);
	}
	return true;
}

void PfxDebugRenderImpl::renderAabb(const PfxBroadphaseProxyContainer &broadphaseProxyContainer)
{
	SCE_PFX_ASSERT_MSG(m_renderAabbFunc,"PfxDebugRenderImpl: render Aabb function is not set.\n");
	
	pfxBroadphaseProxyContainerTraverse(broadphaseProxyContainer,renderAabbCallback,this);
}

struct PfxDebugRenderIslandAabbParam {
	PfxLargePosition islandAabbMin;
	PfxLargePosition islandAabbMax;
	const PfxRigidState *states;
	const PfxCollidable *collidables;
};

struct PfxDebugRenderContactCallbackParam {
	PfxDebugRenderImpl *debugRender;
	const PfxRigidState *states;
};

static PfxBool traverseContactsCallback(PfxContactHolder &contact,PfxUInt32 rigidbodyIdA,PfxUInt32 rigidbodyIdB,void *userData)
{
	PfxDebugRenderContactCallbackParam *param = (PfxDebugRenderContactCallbackParam*)userData;
	
	const PfxRigidState &stateA = param->states[rigidbodyIdA];
	const PfxRigidState &stateB = param->states[rigidbodyIdB];
	
	PfxLargePosition lposA = stateA.getLargePosition();
	PfxLargePosition lposB = stateB.getLargePosition();
	lposA.changeSegment(param->debugRender->m_segment);
	lposB.changeSegment(param->debugRender->m_segment);
	
	PfxContactManifold *contactManifold = contact.findFirstContactManifold();

	while(contactManifold) {
		for (PfxUInt32 j = 0; j < contactManifold->getNumContactPoints(); j++) {
			const PfxContactPoint &cp = contactManifold->getContactPoint(j);
			PfxVector3 pointNormal = cp.getNormal();
			PfxVector3 pointA = lposA.offset + rotate(stateA.getOrientation(), pfxReadVector3(cp.m_localPointA));
			PfxVector3 pointB = lposB.offset + rotate(stateB.getOrientation(), pfxReadVector3(cp.m_localPointB));

			if (param->debugRender->isVisible(rigidbodyIdA) && param->debugRender->isInsideFrustum(pointA, 0.5f)) {
				param->debugRender->m_renderPointFunc(pointA, param->debugRender->m_colors[kPfxDebugRenderColorContactPointA]);
				param->debugRender->m_renderLineFunc(pointA, pointA + param->debugRender->m_scale * pointNormal, param->debugRender->m_colors[kPfxDebugRenderColorContactPointA]);
			}
			if (param->debugRender->isVisible(rigidbodyIdB) && param->debugRender->isInsideFrustum(pointB, 0.5f)) {
				param->debugRender->m_renderPointFunc(pointB, param->debugRender->m_colors[kPfxDebugRenderColorContactPointB]);
				param->debugRender->m_renderLineFunc(pointB, pointB - param->debugRender->m_scale * pointNormal, param->debugRender->m_colors[kPfxDebugRenderColorContactPointB]);
			}
		}

		for (PfxUInt32 j = 0; j<contactManifold->getNumClosestPoints(); j++) {
			const PfxClosestPoint &cp = contactManifold->getClosestPoint(j);
			PfxVector3 pointA = lposA.offset + rotate(stateA.getOrientation(), pfxReadVector3(cp.m_localPointA));
			PfxVector3 pointB = lposB.offset + rotate(stateB.getOrientation(), pfxReadVector3(cp.m_localPointB));

			if ((param->debugRender->isVisible(rigidbodyIdA) || param->debugRender->isVisible(rigidbodyIdB)) &&
				(param->debugRender->isInsideFrustum(pointA, 0.5f) || param->debugRender->isInsideFrustum(pointB, 0.5f))) {
				//param->debugRender->m_renderLineFunc(pointA,pointA+param->debugRender->m_scale * pointNormal,param->debugRender->m_colors[kPfxDebugRenderColorFacetNormal]);
				//param->debugRender->m_renderLineFunc(pointB,pointB-param->debugRender->m_scale * pointNormal,param->debugRender->m_colors[kPfxDebugRenderColorFacetNormal]);

				if ((cp.m_subDataA.getCcdPriority() & 0x08) || (cp.m_subDataB.getCcdPriority() & 0x08)) {
					param->debugRender->m_renderLineFunc(pointA, pointB, param->debugRender->m_colors[kPfxDebugRenderColorClosestHits]);
				}
				else {
					param->debugRender->m_renderLineFunc(pointA, pointB, param->debugRender->m_colors[kPfxDebugRenderColorClosest]);
				}
			}
		}

		contactManifold = contact.findNextContactManifold(contactManifold);
	}
	
	return true;
}

void PfxDebugRenderImpl::renderContact(const PfxContactContainer &contactContainer,const PfxRigidState *states)
{
	SCE_PFX_ASSERT_MSG(m_renderPointFunc,"PfxDebugRenderImpl: render Point function is not set.\n");
	SCE_PFX_ASSERT_MSG(m_renderLineFunc,"PfxDebugRenderImpl: render Line function is not set.\n");
	
	PfxDebugRenderContactCallbackParam param;
	param.debugRender = this;
	param.states = states;
	
	pfxContactContainerTraverse(contactContainer,traverseContactsCallback,&param);
}

void PfxDebugRenderImpl::renderJoint(const PfxJoint *joints,const PfxRigidState *states,const PfxUInt32 numJoints, const PfxUInt32 filter)
{
	SCE_PFX_ASSERT_MSG(m_renderPointFunc,"PfxDebugRenderImpl: render Point function is not set.\n");
	SCE_PFX_ASSERT_MSG(m_renderLineFunc,"PfxDebugRenderImpl: render Line function is not set.\n");
	SCE_PFX_ASSERT_MSG(m_renderArcFunc,"PfxDebugRenderImpl: render Arc function is not set.\n");

	float lower1, upper1, lower2, upper2, upper3;
	PfxVector3 anchorA, anchorB;
	PfxMatrix3 frameA, frameB;
	PfxVector3 vlow, vup, endpt;

	//joint
	for(PfxUInt32 i=0;i<numJoints;i++) {
		PfxJoint joint = joints[i];
		if(!joint.m_active) continue;
		if( ( filter & ( 1 << joint.m_type ) ) == 0 )continue;

		const PfxUInt32 idA = joint.m_rigidBodyIdA;
		const PfxUInt32 idB = joint.m_rigidBodyIdB;

		if(!isVisible(idA) || !isVisible(idB)){
			continue;
		}

		const PfxRigidState &stateA = states[idA];
		const PfxRigidState &stateB = states[idB];

		PfxLargePosition lposA = stateA.getLargePosition();
		PfxLargePosition lposB = stateB.getLargePosition();
		lposA.changeSegment(m_segment);
		lposB.changeSegment(m_segment);

		PfxMatrix3 wRotA(stateA.getOrientation());
		PfxMatrix3 wRotB(stateB.getOrientation());
		PfxVector3 wAnchorA = lposA.offset + wRotA * joint.m_anchorA;
		PfxVector3 wAnchorB = lposB.offset + wRotB * joint.m_anchorB;
		PfxMatrix3 wFrameA = wRotA * joint.m_frameA;
		PfxMatrix3 wFrameB = wRotB * joint.m_frameB;
		PfxMatrix4 wTransA(wFrameA,wAnchorA);
		PfxMatrix4 wTransB(wFrameB,wAnchorB);

		if(joint.m_type == kPfxJointMotor) {
			wFrameA = wRotA * joint.m_frameA * PfxMatrix3(joint.m_targetFrame);
		}
		
		if(!(isInsideFrustum(wAnchorA,1.0f) && isInsideFrustum(wAnchorB,1.0f))) {
			continue;
		}
		
		//point
		m_renderPointFunc(wAnchorA,m_colors[kPfxDebugRenderColorJointAnchorA]);
		m_renderPointFunc(wAnchorB,m_colors[kPfxDebugRenderColorJointAnchorB]);
		
		//axis
		if( joint.m_type != kPfxJointSwingTwist ) {
			m_renderLineFunc( wAnchorA, wAnchorA + m_scale * wFrameA.getCol0(), m_colors[ kPfxDebugRenderColorJointAxisX ] );
			m_renderLineFunc( wAnchorA, wAnchorA + m_scale * wFrameA.getCol1(), m_colors[ kPfxDebugRenderColorJointAxisY ] );
			m_renderLineFunc( wAnchorA, wAnchorA + m_scale * wFrameA.getCol2(), m_colors[ kPfxDebugRenderColorJointAxisZ ] );
			m_renderLineFunc( wAnchorB, wAnchorB + m_scale * wFrameB.getCol0(), m_colors[ kPfxDebugRenderColorJointAxisX ] );
			m_renderLineFunc( wAnchorB, wAnchorB + m_scale * wFrameB.getCol1(), m_colors[ kPfxDebugRenderColorJointAxisY ] );
			m_renderLineFunc( wAnchorB, wAnchorB + m_scale * wFrameB.getCol2(), m_colors[ kPfxDebugRenderColorJointAxisZ ] );
		}

		float radius1 = 1.0f * m_scale;
		float radius2 = 0.5f * m_scale;
		
		switch(joint.m_type){
			case kPfxJointBall:
			case kPfxJointFix:
			case kPfxJointMotor:
			break;
			case kPfxJointHinge:
				lower1 = joint.m_constraints[3].m_movableLowerLimit;
				upper1 = joint.m_constraints[3].m_movableUpperLimit;
				m_renderArcFunc(wAnchorA,wFrameA.getCol0(),wFrameA.getCol2(),radius1,lower1,upper1,m_colors[kPfxDebugRenderColorJointArc1]);
				break;
			case kPfxJointSlider:
				lower1 = joint.m_constraints[0].m_movableLowerLimit;
				upper1 = joint.m_constraints[0].m_movableUpperLimit;
				vlow = wAnchorA + wRotA * joint.m_frameA * PfxVector3(lower1, 0.0, 0.0);
				vup  = wAnchorA + wRotA * joint.m_frameA * PfxVector3(upper1, 0.0, 0.0);
				m_renderLineFunc(vlow, vup, PfxVector3(1.0,0.0,0.0));
				endpt = wFrameA.getCol2()*0.2*m_scale;
				m_renderLineFunc(vlow+endpt,vlow-endpt, PfxVector3(0.5,0.0,0.8));
				m_renderLineFunc(vup+endpt, vup-endpt,  PfxVector3(0.5,0.0,0.8));			
				break;
			case kPfxJointSwingTwist:	
				lower1 = joint.m_constraints[ 3 ].m_movableLowerLimit;
				upper1 = joint.m_constraints[ 3 ].m_movableUpperLimit;
				lower2 = joint.m_constraints[ 4 ].m_movableLowerLimit;
				upper2 = joint.m_constraints[ 4 ].m_movableUpperLimit;
				m_renderArcFunc( wAnchorA, wFrameA.getCol0(), wFrameA.getCol2(), radius2, lower1, upper1, m_colors[ kPfxDebugRenderColorJointArc1 ] ); //twist
				m_renderArcFunc( wAnchorA, wFrameA.getCol2(), wFrameA.getCol0(), radius1, lower2, upper2, m_colors[ kPfxDebugRenderColorJointArc2 ] ); //swing(+)
				m_renderArcFunc( wAnchorA, wFrameA.getCol2(), wFrameA.getCol0(), radius1, -lower2, -upper2, m_colors[ kPfxDebugRenderColorJointArc2 ] ); //swing(-)
				m_renderArcFunc( wAnchorA, wFrameA.getCol1(), wFrameA.getCol0(), radius1, lower2, upper2, m_colors[ kPfxDebugRenderColorJointArc2 ] ); //swing(+)
				m_renderArcFunc( wAnchorA, wFrameA.getCol1(), wFrameA.getCol0(), radius1, -lower2, -upper2, m_colors[ kPfxDebugRenderColorJointArc2 ] ); //swing(-)
				m_renderLineFunc( wAnchorB, wAnchorB + m_scale * wFrameB.getCol0(), m_colors[ kPfxDebugRenderColorJointAxisX ] );
				break;
			case kPfxJointUniversal:
				lower1 = joint.m_constraints[4].m_movableLowerLimit;
				upper1 = joint.m_constraints[4].m_movableUpperLimit;
				lower2 = joint.m_constraints[5].m_movableLowerLimit;
				upper2 = joint.m_constraints[5].m_movableUpperLimit;
				m_renderArcFunc(wAnchorA,wFrameA.getCol2(),wFrameA.getCol0(),radius1,lower1,upper1,m_colors[kPfxDebugRenderColorJointArc1]);
				m_renderArcFunc(wAnchorA,wFrameA.getCol1(),wFrameA.getCol0(),radius1,lower2,upper2,m_colors[kPfxDebugRenderColorJointArc2]);
			break;

			case kPfxJointShoulder:
				lower1 = joint.m_constraints[3].m_movableLowerLimit;
				upper1 = joint.m_constraints[3].m_movableUpperLimit;
				upper2 = joint.m_constraints[4].m_movableUpperLimit;
				upper3 = joint.m_constraints[5].m_movableUpperLimit;
				m_renderArcFunc(wAnchorA,wFrameA.getCol0(),wFrameA.getCol2(),radius2,  lower1, upper1,m_colors[kPfxDebugRenderColorJointArc1]); //twist
				m_renderArcFunc(wAnchorA,wFrameA.getCol2(),wFrameA.getCol0(),radius1,  -upper2, upper2,m_colors[kPfxDebugRenderColorJointArc2]); //swing(+)
				m_renderArcFunc(wAnchorA,wFrameA.getCol1(),wFrameA.getCol0(),radius1,  -upper3, upper3,m_colors[kPfxDebugRenderColorJointArc2]); //swing(+)
			break;

			case kPfxJointDistance:
			{
				PfxVector3 dir = wAnchorB - wAnchorA;
				if (lengthSqr(dir) > 0.00001f) {
					PfxVector3 limitPos = wAnchorA + joint.m_constraints[0].m_movableUpperLimit * normalize(dir);
					m_renderLineFunc(wAnchorA, limitPos, PfxVector3(1.0, 0.0, 0.0));
					PfxVector3 tmpX = wFrameA.getCol0()*0.2f*m_scale;
					PfxVector3 tmpY = wFrameA.getCol1()*0.2f*m_scale;
					PfxVector3 tmpZ = wFrameA.getCol2()*0.2f*m_scale;
					m_renderLineFunc(limitPos + tmpX, limitPos - tmpX, PfxVector3(0.5, 0.0, 0.8));
					m_renderLineFunc(limitPos + tmpY, limitPos - tmpY, PfxVector3(0.5, 0.0, 0.8));
					m_renderLineFunc(limitPos + tmpZ, limitPos - tmpZ, PfxVector3(0.5, 0.0, 0.8));
				}
			}
				break;

			default:
				//SCE_PFX_PRINTF("PfxDebugRenderImpl: joint type error\n");
			break;
		}
	}
}

PfxBool PfxDebugRenderImpl::renderLargeMeshIsland(const PfxLargeTriMesh *mesh,PfxUInt32 islandId,const PfxTransform3 &shapeTransform,const PfxVector3 &color)
{
	SCE_PFX_ASSERT_MSG(m_renderBoxFunc,"PfxDebugRenderImpl: render Box function is not set.\n");

	PfxVector3 aabbMin,aabbMax;
	
	pfxLargeTriMeshGetIslandAabb(*mesh,islandId,aabbMin,aabbMax);
	
	PfxVector3 aabbCenter = (aabbMin + aabbMax) * 0.5f;
	PfxVector3 aabbHalf   = (aabbMax - aabbMin) * 0.5f;
	
	PfxVector3 wCenter = shapeTransform.getTranslation() + shapeTransform.getUpper3x3() * aabbCenter;
	PfxVector3 wHalf = shapeTransform.getUpper3x3() * aabbHalf;
	
	if(isInsideFrustum(wCenter,length(wHalf))) {
		m_renderBoxFunc(shapeTransform * PfxTransform3::translation(aabbCenter),aabbHalf,color);
		return true;
	}
	return false;
}

void PfxDebugRenderImpl::renderLargeMeshFacet(const PfxLargeTriMesh *mesh, PfxUInt32 islandId, PfxUInt32 facetId, const PfxTransform3 &shapeTransform, const PfxUInt32 flag, const PfxVector3 &color)
{
	(void) mesh;
	SCE_PFX_ASSERT_MSG(m_renderLineFunc,"PfxDebugRenderImpl: render Line function is not set.\n");
	SCE_PFX_ASSERT_MSG(m_renderBoxFunc,"PfxDebugRenderImpl: render Box function is not set.\n");

	if(flag==SCE_PFX_DRENDER_MESH_FLG_NONE) return;
	
	const PfxLargeTriMeshFacet facetInfo = pfxLargeTriMeshGetFacetInIslands(*mesh,islandId,facetId);
	
	PfxVector3 pnts[6];
	pnts[0] = pfxLargeTriMeshGetVertexInIslands(*mesh,islandId,facetInfo.vertIds[0]);
	pnts[1] = pfxLargeTriMeshGetVertexInIslands(*mesh,islandId,facetInfo.vertIds[1]);
	pnts[2] = pfxLargeTriMeshGetVertexInIslands(*mesh,islandId,facetInfo.vertIds[2]);
	
	pnts[3] = pnts[0]-facetInfo.thickness*facetInfo.normal;
	pnts[4] = pnts[1]-facetInfo.thickness*facetInfo.normal;
	pnts[5] = pnts[2]-facetInfo.thickness*facetInfo.normal;

	PfxVector3 aabbMin = minPerElem(pnts[5],minPerElem(pnts[4],minPerElem(pnts[3],minPerElem(pnts[2],minPerElem(pnts[1],pnts[0])))));
	PfxVector3 aabbMax = maxPerElem(pnts[5],maxPerElem(pnts[4],maxPerElem(pnts[3],maxPerElem(pnts[2],maxPerElem(pnts[1],pnts[0])))));
	
	if((SCE_PFX_DRENDER_MESH_FLG_EDGE | SCE_PFX_DRENDER_MESH_FLG_THICKNESS)& flag) {
		if(facetInfo.edgeTilt[0] > 0.0f || facetInfo.edgeTilt[1] > 0.0f || facetInfo.edgeTilt[2] > 0.0f) {
			PfxVector3 sideNml[3];
			sideNml[0] = normalize(cross((pnts[1] - pnts[0]),facetInfo.normal));
			sideNml[1] = normalize(cross((pnts[2] - pnts[1]),facetInfo.normal));
			sideNml[2] = normalize(cross((pnts[0] - pnts[2]),facetInfo.normal));
			sideNml[0] = cosf(facetInfo.edgeTilt[0])*sideNml[0] - sinf(facetInfo.edgeTilt[0])*facetInfo.normal;
			sideNml[1] = cosf(facetInfo.edgeTilt[1])*sideNml[1] - sinf(facetInfo.edgeTilt[1])*facetInfo.normal;
			sideNml[2] = cosf(facetInfo.edgeTilt[2])*sideNml[2] - sinf(facetInfo.edgeTilt[2])*facetInfo.normal;
			PfxMatrix3 mtx(0.0f);
			PfxVector3 vec(0.0f);
			mtx.setRow(0,sideNml[0]);
			mtx.setRow(1,sideNml[1]);
			mtx.setRow(2,sideNml[2]);
			vec[0] = dot(pnts[0],sideNml[0]);
			vec[1] = dot(pnts[1],sideNml[1]);
			vec[2] = dot(pnts[2],sideNml[2]);
			PfxVector3 intersection = inverse(mtx) * vec;
			float dist = -dot(intersection-pnts[0],facetInfo.normal);
			if(facetInfo.thickness < dist) {
				float t = facetInfo.thickness / dist;
				pnts[3] = pnts[0]+t*(intersection-pnts[0]);
				pnts[4] = pnts[1]+t*(intersection-pnts[1]);
				pnts[5] = pnts[2]+t*(intersection-pnts[2]);
			} else {
				pnts[3] = pnts[4] = pnts[5] = intersection;
			}
		}
	}
	
	for(int m=0;m<6;m++) {
		pnts[m] = PfxVector3(shapeTransform * PfxPoint3(pnts[m]));
	}
	
	PfxVector3 wnormal = normalize(transpose(inverse(shapeTransform.getUpper3x3())) * facetInfo.normal);
	
	if(!isInsideFrustum(pnts[0],pnts[1],pnts[2],wnormal)) {
		return;
	}
	
	if(SCE_PFX_DRENDER_MESH_FLG_FACET_AABB & flag) {
		PfxVector3 cnt = 0.5f * (aabbMax + aabbMin);
		PfxVector3 hlf = 0.5f * (aabbMax - aabbMin);
		m_renderBoxFunc(shapeTransform * PfxTransform3::translation(cnt),hlf,m_colors[kPfxDebugRenderColorFacetAabb]);
	}
	
	if((SCE_PFX_DRENDER_MESH_FLG_EDGE | SCE_PFX_DRENDER_MESH_FLG_THICKNESS)& flag) {

//		if((facetInfo.edgeType&0x03) == SCE_PFX_EDGE_CONVEX) m_renderLineFunc(pnts[0],pnts[1],color);
//		if(((facetInfo.edgeType>>2)&0x03) == SCE_PFX_EDGE_CONVEX) m_renderLineFunc(pnts[1],pnts[2],color);
//		if(((facetInfo.edgeType>>4)&0x03) == SCE_PFX_EDGE_CONVEX) m_renderLineFunc(pnts[2],pnts[0],color);

		m_renderLineFunc(pnts[0],pnts[1],color);
		m_renderLineFunc(pnts[1],pnts[2],color);
		m_renderLineFunc(pnts[2],pnts[0],color);
		
		if(SCE_PFX_DRENDER_MESH_FLG_THICKNESS & flag) {
			m_renderLineFunc(pnts[3],pnts[4],0.2f*color);
			m_renderLineFunc(pnts[4],pnts[5],0.2f*color);
			m_renderLineFunc(pnts[5],pnts[3],0.2f*color);
			m_renderLineFunc(pnts[0],pnts[3],0.5f*color);
			m_renderLineFunc(pnts[1],pnts[4],0.5f*color);
			m_renderLineFunc(pnts[2],pnts[5],0.5f*color);
		}
	}
	
	if(SCE_PFX_DRENDER_MESH_FLG_NORMAL & flag) {
		PfxVector3 p1((pnts[0] + pnts[1] + pnts[2])/3.0f);
		m_renderLineFunc(p1,p1+m_normalLength*wnormal,m_colors[kPfxDebugRenderColorFacetNormal]);
	}
}

void PfxDebugRenderImpl::renderLargeMesh(const PfxRigidState *states,const PfxCollidable *collidables,const PfxUInt32 numRigidbodies,const PfxUInt32 flag)
{
	if(flag==SCE_PFX_DRENDER_MESH_FLG_NONE) return;

	for(PfxUInt32 i=0;i<numRigidbodies;i++) {
		if(isVisible(i)){
			const PfxRigidState &state = states[i];
			const PfxCollidable &coll = collidables[i];
			
			PfxLargePosition lpos = state.getLargePosition();
			lpos.changeSegment(m_segment);
			
			if(!isInsideFrustum(lpos.offset+coll.getCenter(),length(coll.getHalf()))) {
				continue;
			}
			
			PfxTransform3 bodyTransform(state.getOrientation(),lpos.offset);
			
			for(PfxUInt32 j=0;j<coll.getNumShapes();j++) {
				const PfxShape &shape = coll.getShape(j);
				if (shape.getType() == kPfxShapeLargeTriMesh) {
					const PfxLargeTriMesh *mesh = shape.getLargeTriMesh();
					PfxTransform3 shapeTransform = bodyTransform * shape.getOffsetTransform() * PfxTransform3::scale(shape.getScaleXyz());
					for(PfxUInt32 l=0;l<pfxLargeTriMeshGetNumIslands(*mesh);l++) {
						PfxBool islandRendered = true;
						
						if(flag  & SCE_PFX_DRENDER_MESH_FLG_ISLAND) {
							islandRendered = renderLargeMeshIsland(mesh,l,shapeTransform,m_colors[kPfxDebugRenderColorMeshIsland]);
						}
						
						if(!islandRendered) continue;
						
						PfxVector3 col = 0.5f * m_colors[kPfxDebugRenderColorCount + (l%SCE_PFX_DRENDER_RANDOM_COLOR_NUM)];

						for(PfxUInt32 f=0;f<pfxLargeTriMeshGetNumFacetsInIslands(*mesh,l);f++) {
							renderLargeMeshFacet(mesh, l, f, shapeTransform, flag, col);
						}
					}
				}
			}
		}
	}
}

void PfxDebugRenderImpl::renderPrimitives(const PfxRigidState *states, const PfxCollidable *collidables, const PfxUInt32 numRigidbodies)
{
	for (PfxUInt32 i = 0; i<numRigidbodies; i++) {
		if (isVisible(i)) {
			const PfxRigidState &state = states[i];
			const PfxCollidable &coll = collidables[i];

			PfxLargePosition lpos = state.getLargePosition();
			lpos.changeSegment(m_segment);

			if (!isInsideFrustum(lpos.offset + coll.getCenter(), length(coll.getHalf()))) {
				continue;
			}

			PfxTransform3 bodyTransform(state.getOrientation(), lpos.offset);

			for (PfxUInt32 j = 0; j < coll.getNumShapes(); j++) {
				const PfxShape &shape = coll.getShape(j);
				PfxTransform3 shapeTransform = bodyTransform * shape.getOffsetTransform() * PfxTransform3::scale(shape.getScaleXyz());
				PfxVector3 col = m_colors[kPfxDebugRenderColorPrimitive];

				switch (shape.getType()) {
				case kPfxShapeSphere:
				case kPfxShapeCoreSphere:
					renderSphere(shapeTransform, col, shape.getSphere().m_radius);
					break;

				case kPfxShapeBox:
					renderBox(shapeTransform, col, shape.getBox().m_half);
					break;

				case kPfxShapeCapsule:
					renderCapsule(shapeTransform, col, shape.getCapsule().m_radius, shape.getCapsule().m_halfLen, true);
					break;

				case kPfxShapeCylinder:
					renderCapsule(shapeTransform, col, shape.getCylinder().m_radius, shape.getCylinder().m_halfLen, false);
					break;

				case kPfxShapeConvexMesh:
					renderConvexMesh(shapeTransform, col, shape.getConvexMesh());
				break;

				default:
					break;
				}
			}

			for (PfxUInt32 j = 0; j < coll.getNumCoreSpheres(); j++) {
				const PfxCoreSphere &coreSphere = coll.getCoreSphere(j);
				PfxTransform3 shapeTransform = bodyTransform * PfxTransform3::translation(coreSphere.getOffsetPosition());
				PfxVector3 col = m_colors[kPfxDebugRenderColorCoreSphere];
				renderSphere(shapeTransform, col, coreSphere.getRadius());
			}
		}
	}
}

void PfxDebugRenderImpl::renderSphere(const PfxTransform3 &transform, const PfxVector3 &color, PfxFloat radius)
{
	PfxVector3 axisX = transform.getUpper3x3().getCol0();
	PfxVector3 axisY = transform.getUpper3x3().getCol1();
	m_renderArcFunc(transform.getTranslation(), axisY, axisX, radius, 0.0f, SCE_PFX_PI*2.0f, color);
	m_renderArcFunc(transform.getTranslation(), axisX, axisY, radius, 0.0f, SCE_PFX_PI*2.0f, color);
}

void PfxDebugRenderImpl::renderBox(const PfxTransform3 &transform, const PfxVector3 &color, PfxVector3 halfExtent)
{
	m_renderBoxFunc(transform, halfExtent, color);
}

void PfxDebugRenderImpl::renderCapsule(const PfxTransform3 &transform, const PfxVector3 &color, PfxFloat radius, PfxFloat halfLength, PfxBool cap)
{
	PfxVector3 leftCenter(transform * PfxPoint3(-halfLength, 0.0f, 0.0f));
	PfxVector3 rightCenter(transform * PfxPoint3(halfLength, 0.0f, 0.0f));
	PfxVector3 center(transform.getTranslation());
	PfxVector3 axisX = transform.getUpper3x3().getCol0();
	PfxVector3 axisY = transform.getUpper3x3().getCol1();
	PfxVector3 axisZ = transform.getUpper3x3().getCol2();

	const float pi = 3.14f;

	// body
	m_renderArcFunc(leftCenter, axisX, axisY, radius, -pi, pi, color);
	m_renderArcFunc(rightCenter, axisX, axisY, radius, -pi, pi, color);
	m_renderArcFunc(center, axisX, axisY, radius, -pi, pi, color);

	if (cap) {
		// left cap
		m_renderArcFunc(leftCenter, axisY, axisZ, radius, -pi, 0.0f, color);
		m_renderArcFunc(leftCenter, axisZ, axisY, radius, 0.0f, pi, color);

		// right cap
		m_renderArcFunc(rightCenter, axisY, axisZ, radius, 0.0f, pi, color);
		m_renderArcFunc(rightCenter, axisZ, axisY, radius, -pi, 0.0f, color);
	}

	const PfxVector3 points[8] = {
		leftCenter + radius * axisY,
		rightCenter + radius * axisY,
		leftCenter - radius * axisY,
		rightCenter - radius * axisY,
		leftCenter + radius * axisZ,
		rightCenter + radius * axisZ,
		leftCenter - radius * axisZ,
		rightCenter - radius * axisZ,
	};

	const unsigned short indices[] = {
		0, 1, 2, 3, 4, 5, 6, 7
	};

	for (int i = 0; i<8; i += 2) {
		m_renderLineFunc(PfxVector3(points[indices[i]]), PfxVector3(points[indices[i + 1]]), color);
	}
}

void PfxDebugRenderImpl::renderConvexMesh(const PfxTransform3 &transform, const PfxVector3 &color, const PfxConvexMesh *convexMesh)
{
	const PfxConvexMeshImpl *convexMeshImpl = (PfxConvexMeshImpl*)convexMesh;

	for (int i = 0; i < (int)convexMeshImpl->m_numFacets; i++) {
		int vId0 = convexMeshImpl->m_indices[i * 3];
		int vId1 = convexMeshImpl->m_indices[i * 3 + 1];
		int vId2 = convexMeshImpl->m_indices[i * 3 + 2];
		PfxVector3 v0(transform * pfxReadPoint3(&convexMeshImpl->m_verts[vId0 * 3]));
		PfxVector3 v1(transform * pfxReadPoint3(&convexMeshImpl->m_verts[vId1 * 3]));
		PfxVector3 v2(transform * pfxReadPoint3(&convexMeshImpl->m_verts[vId2 * 3]));
		m_renderLineFunc(v0, v1, color);
		m_renderLineFunc(v1, v2, color);
		m_renderLineFunc(v2, v0, color);
	}
}

void PfxDebugRenderImpl::enableVisible(const PfxUInt32 rigidbodyId) 
{
	PfxUInt8 flag;
	if(m_visibleFlags.find(rigidbodyId,flag)) {
		m_visibleFlags.erase(rigidbodyId);
	}
}

void PfxDebugRenderImpl::disableVisible(const PfxUInt32 rigidbodyId)
{
	m_visibleFlags.insert(rigidbodyId,SCE_PFX_DRENDER_INVISIBLE);
}

PfxBool PfxDebugRenderImpl::isVisible(const PfxUInt32 rigidbodyId) const 
{
	PfxUInt8 flag = 0;
	m_visibleFlags.find(rigidbodyId,flag);
	return (flag & SCE_PFX_DRENDER_INVISIBLE) == 0;
}

PfxBool PfxDebugRenderImpl::isInsideFrustum(const PfxVector3 &center,const PfxFloat &radius)
{
	switch(m_numPlanes) {
		case 0:
		break;
		
		case 1:
		{
			const PfxVector3 testCenter = m_planes[0].getXYZ();
			const PfxFloat testRadius = m_planes[0].getW();
			const PfxFloat distanceSqr = lengthSqr(testCenter - center);
			const PfxFloat r2 = testRadius + radius;
			if(distanceSqr > r2*r2) return false;
		}
		break;
		
		default:
		for(PfxUInt32 i=0; i<m_numPlanes; i++) {
			const PfxVector3 normal = m_planes[i].getXYZ();
			const PfxFloat distance = m_planes[i].getW() - dot(normal, center);
			/*  Frustum definition
				|                |
				|-->  Inside  <--|
				|                |
			 */
			if(distance < -radius) {
				return false;
			}
		}
		break;
	}
	
	return true;
}

PfxBool PfxDebugRenderImpl::isInsideFrustum(const PfxVector3 &p0,const PfxVector3 &p1,const PfxVector3 &p2,const PfxVector3 &normal)
{
	switch(m_numPlanes) {
		case 0:
		break;
		
		case 1:
		{
			const PfxVector3 testCenter = m_planes[0].getXYZ();
			const PfxFloat testRadius = m_planes[0].getW();
			
			PfxVector3 s;
			pfxClosestPointTriangle(testCenter,PfxTriangle(p0,p1,p2),s);
			
			if(lengthSqr(testCenter-s) >= testRadius * testRadius) return false;
		}
		break;
		
		default:
		/*  Frustum definition
			|                |
			|-->  Inside  <--|
			|                |
		 */
		for(PfxUInt32 i=0; i<m_numPlanes; i++) {
			const PfxVector3 pnormal = m_planes[i].getXYZ();
			const PfxFloat distance0 = m_planes[i].getW() - dot(pnormal, p0);
			const PfxFloat distance1 = m_planes[i].getW() - dot(pnormal, p1);
			const PfxFloat distance2 = m_planes[i].getW() - dot(pnormal, p2);
			if(distance0 < 0 || distance1 < 0 || distance2 < 0) {
				return false;
			}
		}
		break;
	}
	
	return true;
}

PfxFloat PfxDebugRenderImpl::getScale() const
{
	return m_scale;
}

void PfxDebugRenderImpl::setScale(PfxFloat scale)
{
	m_scale = scale;
}

} //namespace pfxv4
} //namespace sce
