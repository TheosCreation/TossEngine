/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../include/physics_effects/util/pfx_debug_render.h"
#include "pfx_debug_render_impl.h"

namespace sce {
namespace pfxv4 {

PfxUInt32 pfxDebugRenderQueryMem()
{
	return PfxDebugRenderImpl::getBytes();
}

PfxInt32 pfxDebugRenderInit(PfxDebugRender &debugRender,const PfxDebugRenderInitParam &param,void *workBuff,PfxUInt32 workBytes)
{
	SCE_PFX_ASSERT(sizeof(PfxDebugRenderImpl) <= sizeof(PfxDebugRender));

	if(workBytes < PfxDebugRenderImpl::getBytes()) {
		return SCE_PFX_ERR_OUT_OF_BUFFER;
	}
	
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;
	
	debugRenderImpl->initialize(
		param.pointFunc,
		param.lineFunc,
		param.arcFunc,
		param.aabbFunc,
		param.boxFunc,
		workBuff,workBytes);
	
	return SCE_PFX_OK;
}

PfxInt32 pfxDebugRenderTerm(PfxDebugRender &debugRender)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;
	
	debugRenderImpl->finalize();
	
	return SCE_PFX_OK;
}

PfxInt32 pfxDebugRenderClear(PfxDebugRender &debugRender)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;
	
	debugRenderImpl->clear();
	
	return SCE_PFX_OK;
}

void pfxDebugRenderEnableVisible(PfxDebugRender &debugRender,PfxUInt32 rigidbodyId)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;
	
	debugRenderImpl->enableVisible(rigidbodyId);
}

void pfxDebugRenderDisableVisible(PfxDebugRender &debugRender,PfxUInt32 rigidbodyId)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;
	
	debugRenderImpl->disableVisible(rigidbodyId);
}

void pfxDebugRenderSetScale(PfxDebugRender &debugRender,PfxFloat scale)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;
	
	debugRenderImpl->setScale(scale);
}

PfxFloat pfxDebugRenderGetScale(PfxDebugRender &debugRender)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;
	
	return debugRenderImpl->getScale();
}

void pfxDebugRenderSetSegment(PfxDebugRender &debugRender,const PfxSegment &segment)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;
	
	debugRenderImpl->setSegment(segment);
}

void pfxDebugRenderSetFrustum(PfxDebugRender &debugRender,const PfxVector4 *planes, PfxUInt32 numPlanes)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;

	debugRenderImpl->setFrustum(planes,numPlanes);
}

void pfxDebugRenderRenderAabb(PfxDebugRender &debugRender,const PfxBroadphaseProxyContainer &broadphaseProxyContainer)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;
	
	debugRenderImpl->renderAabb(broadphaseProxyContainer);
}

void pfxDebugRenderRenderLocalAxis(PfxDebugRender &debugRender,const PfxRigidState *states,PfxUInt32 numRigidbodies)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;
	
	debugRenderImpl->renderLocalAxis(states,numRigidbodies);
}

void pfxDebugRenderRenderContact(PfxDebugRender &debugRender,const PfxContactContainer &contactContainer,const PfxRigidState *states)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;
	
	debugRenderImpl->renderContact(contactContainer,states);
}

void pfxDebugRenderRenderJoint(PfxDebugRender &debugRender,const PfxRigidState *states,const PfxJoint *joints,PfxUInt32 numJoints, PfxUInt32 filter)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;
	
	debugRenderImpl->renderJoint(joints,states,numJoints, filter );
}

void pfxDebugRenderRenderLargeMesh(PfxDebugRender &debugRender,const PfxRigidState *states,const PfxCollidable *collidables,PfxUInt32 numRigidbodies,PfxUInt32 flag)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;
	
	debugRenderImpl->renderLargeMesh(states,collidables,numRigidbodies,flag);
}

void pfxDebugRenderRenderPrimitives(PfxDebugRender &debugRender, const PfxRigidState *states, const PfxCollidable *collidables, PfxUInt32 numRigidbodies)
{
	PfxDebugRenderImpl *debugRenderImpl = (PfxDebugRenderImpl*)&debugRender;

	debugRenderImpl->renderPrimitives(states, collidables, numRigidbodies);
}

} //namespace pfxv4
} //namespace sce
