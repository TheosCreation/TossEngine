/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_DEBUG_RENDER_IMPL_H
#define _SCE_PFX_DEBUG_RENDER_IMPL_H

#include "../../include/physics_effects/util/pfx_debug_render.h"
#include "pfx_hash_array.h"

namespace sce {
namespace pfxv4 {

#define SCE_PFX_DRENDER_VISIBLE                 0x00
#define SCE_PFX_DRENDER_INVISIBLE               0x01

#define SCE_PFX_DRENDER_RANDOM_COLOR_NUM        128
#define SCE_PFX_DRENDER_VISIBLE_NUM				1000
#define SCE_PFX_DRENDER_FRUSTUM_PLANES			6

enum {
	kPfxDebugRenderColorAabb = 0,
	kPfxDebugRenderColorSimulationIsland,
	kPfxDebugRenderColorContactPointA,
	kPfxDebugRenderColorContactPointB,
	kPfxDebugRenderColorContactNormal,
	kPfxDebugRenderColorClosest,
	kPfxDebugRenderColorClosestHits,
	kPfxDebugRenderColorMeshIsland,
	kPfxDebugRenderColorMeshEdge,
	kPfxDebugRenderColorMeshEdgeConvex,
	kPfxDebugRenderColorFacetEdge,
	kPfxDebugRenderColorFacetAabb,
	kPfxDebugRenderColorFacetNormal,
	kPfxDebugRenderColorFacetNormal2,
	kPfxDebugRenderColorJointAxisX,
	kPfxDebugRenderColorJointAxisY,
	kPfxDebugRenderColorJointAxisZ,
	kPfxDebugRenderColorJointAnchorA,
	kPfxDebugRenderColorJointAnchorB,
	kPfxDebugRenderColorJointArc1,
	kPfxDebugRenderColorJointArc2,
	kPfxDebugRenderColorPrimitive,
	kPfxDebugRenderColorCoreSphere,
	kPfxDebugRenderColorCount
};

class SCE_PFX_ALIGNED(16) PfxDebugRenderImpl
{
public:
	PfxHeapManager m_pool;
	
	PfxHashArray<PfxUInt8> m_visibleFlags;
	
	pfxDebugRenderPointFunc m_renderPointFunc;
	pfxDebugRenderLineFunc  m_renderLineFunc;
	pfxDebugRenderArcFunc   m_renderArcFunc;
	pfxDebugRenderAabbFunc  m_renderAabbFunc;
	pfxDebugRenderBoxFunc   m_renderBoxFunc;
	
	PfxVector3 *m_colors;
	PfxFloat m_scale;
	PfxFloat m_normalLength;
	PfxSegment m_segment;
	PfxVector4 m_planes[SCE_PFX_DRENDER_FRUSTUM_PLANES];
	PfxUInt32 m_numPlanes;
	
	static PfxUInt32 getBytes();
	
	void initialize(
		pfxDebugRenderPointFunc pointFunc,
		pfxDebugRenderLineFunc lineFunc,
		pfxDebugRenderArcFunc arcFunc,
		pfxDebugRenderAabbFunc aabbFunc,
		pfxDebugRenderBoxFunc boxFunc,
		void *workBuff,PfxUInt32 workBytes);
	
	void finalize();
	
	void clear();
	
	//view area setup
	void setSegment(const PfxSegment &origin);
	void setFrustum(const PfxVector4 *planes, int numPlanes);
	
	//render
	void renderAabb(const PfxRigidState *states,const PfxCollidable *collidables,const PfxUInt32 numRigidbodies);
	void renderAabb(const PfxBroadphaseProxyContainer &broadphaseProxyContainer);
	void renderLocalAxis(const PfxRigidState *states,const PfxUInt32 numRigidbodies);
	void renderContact(const PfxContactContainer &contactContainer, const PfxRigidState *states);
	void renderJoint(const PfxJoint *joints,const PfxRigidState *states,const PfxUInt32 numJoints,const PfxUInt32 filter );
	void renderLargeMesh(const PfxRigidState *states,const PfxCollidable *collidables,const PfxUInt32 numRigidbodies,const PfxUInt32 flag);
	void renderPrimitives(const PfxRigidState *states, const PfxCollidable *collidables, const PfxUInt32 numRigidbodies);

	void enableVisible(const PfxUInt32 rigidbodyId);
	void disableVisible(const PfxUInt32 rigidbodyId);
	PfxBool isVisible(const PfxUInt32 rigidbodyId) const;
	
	PfxBool isInsideFrustum(const PfxVector3 &center,const PfxFloat &radius);
	PfxBool isInsideFrustum(const PfxVector3 &p0,const PfxVector3 &p1,const PfxVector3 &p2,const PfxVector3 &normal);
	
	PfxFloat getScale() const;
	void setScale(PfxFloat scale);
	
protected:
	PfxBool renderLargeMeshIsland(const PfxLargeTriMesh *mesh,PfxUInt32 islandId,const PfxTransform3 &shapeTransform,const PfxVector3 &color);
    void renderLargeMeshFacet(const PfxLargeTriMesh *mesh, PfxUInt32 islandId, PfxUInt32 facetId, const PfxTransform3 &shapeTransform, const PfxUInt32 flag, const PfxVector3 &color);
	void renderSphere(const PfxTransform3 &transform, const PfxVector3 &color, PfxFloat radius);
	void renderBox(const PfxTransform3 &transform, const PfxVector3 &color, PfxVector3 halfExtent);
	void renderCapsule(const PfxTransform3 &transform, const PfxVector3 &color, PfxFloat radius, PfxFloat halfLength, PfxBool cap = true);
	void renderConvexMesh(const PfxTransform3 &transform, const PfxVector3 &color, const PfxConvexMesh *convexMesh);
};

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_DEBUG_RENDER_IMPL_H
