/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_shape.h"
#include "../../../include/physics_effects/base_level/collision/pfx_intersect_moving_sphere_func.h"
#include "pfx_intersect_moving_sphere_sphere.h"
#include "pfx_intersect_moving_sphere_box.h"
#include "pfx_intersect_moving_sphere_capsule.h"
#include "pfx_intersect_moving_sphere_cylinder.h"
#include "pfx_intersect_moving_sphere_convex.h"
#include "pfx_intersect_moving_sphere_large_tri_mesh.h"
#include "pfx_convex_mesh_impl.h"
#include "pfx_large_tri_mesh_impl.h"
#include "pfx_intersect_common.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Moving Sphere Intersection Function Table

PfxBool intersectMovingSphereFuncDummy(
				const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	(void)sphereIn,(void)sphereOut,(void)shape,(void)transform;
	return false;
}

PfxBool intersectMovingSphereFuncBox(
				const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	PfxBox box = shape.getBox();
	box.m_half *= shape.getScale();
	
	return pfxIntersectMovingSphereBox(sphereIn, sphereOut, box, pfxRemoveScale(transform, shape.getScale()));
}

PfxBool intersectMovingSphereFuncSphere(
				const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	PfxSphere sphere = shape.getSphere();
	sphere.m_radius *= shape.getScale();

	return pfxIntersectMovingSphereSphere(sphereIn, sphereOut, sphere, pfxRemoveScale(transform, shape.getScale()));
}

PfxBool intersectMovingSphereFuncCapsule(
				const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	PfxCapsule capsule = shape.getCapsule();
	capsule.m_halfLen *= shape.getScale();
	capsule.m_radius *= shape.getScale();

	return pfxIntersectMovingSphereCapsule(sphereIn, sphereOut, capsule, pfxRemoveScale(transform, shape.getScale()));
}

PfxBool intersectMovingSphereFuncCylinder(
				const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	PfxCylinder cylinder = shape.getCylinder();
	cylinder.m_halfLen *= shape.getScale();
	cylinder.m_radius *= shape.getScale();

	return pfxIntersectMovingSphereCylinder(sphereIn, sphereOut, cylinder, pfxRemoveScale(transform, shape.getScale()));
}

PfxBool intersectMovingSphereFuncConvex(
				const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxConvexMeshImpl *convexMesh = (PfxConvexMeshImpl*)shape.getConvexMesh();
	
	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
	
	PfxBool ret = pfxIntersectMovingSphereConvex(sphereIn,sphereOut,(const void*)convexMesh,transform,flipTriangle);
	
	return ret;
}

PfxBool intersectMovingSphereFuncLargeTriMesh(
				const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape.getLargeTriMesh();
	
	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
	PfxFloat radiusLocal = sphereIn.m_radius / minElem(absPerElem(scale));

	PfxBool ret = pfxIntersectMovingSphereLargeTriMesh(sphereIn,sphereOut,(const void*)largeMesh,transform,flipTriangle,radiusLocal, discardTriangleCallback, userDataForDiscardingTriangle);
	
	return ret;
}

PfxIntersectMovingSphereFunc funcTbl_intersectMovingSphere[kPfxShapeCount] = {
	intersectMovingSphereFuncSphere,
	intersectMovingSphereFuncBox,
	intersectMovingSphereFuncCapsule,
	intersectMovingSphereFuncCylinder,
	intersectMovingSphereFuncConvex,
	intersectMovingSphereFuncLargeTriMesh,
	intersectMovingSphereFuncDummy, // Reserved
	intersectMovingSphereFuncDummy, // Reserved
	intersectMovingSphereFuncDummy, // Reserved
	intersectMovingSphereFuncDummy, // Reserved
	intersectMovingSphereFuncDummy, // Reserved
};

///////////////////////////////////////////////////////////////////////////////
// Moving Sphere Intersection Function Table Interface

PfxIntersectMovingSphereFunc pfxGetIntersectMovingSphereFunc(PfxUInt8 shapeType)
{
	return funcTbl_intersectMovingSphere[shapeType];
}

PfxInt32 pfxSetIntersectSphereFunc(PfxUInt8 shapeType,PfxIntersectMovingSphereFunc func)
{
	if(shapeType >= kPfxShapeCount) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	funcTbl_intersectMovingSphere[shapeType] = func;
	
	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
