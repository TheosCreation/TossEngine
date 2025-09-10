/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_shape.h"
#include "../../../include/physics_effects/base_level/collision/pfx_intersect_ray_func.h"
#include "pfx_intersect_ray_box.h"
#include "pfx_intersect_ray_sphere.h"
#include "pfx_intersect_ray_capsule.h"
#include "pfx_intersect_ray_cylinder.h"
#include "pfx_intersect_ray_convex.h"
#include "pfx_intersect_ray_large_tri_mesh.h"
#include "pfx_convex_mesh_impl.h"
#include "pfx_large_tri_mesh_impl.h"
#include "pfx_intersect_common.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Ray Intersection Function Table

PfxBool intersectRayFuncDummy(
				const PfxRayInputInternal &ray,PfxRayOutputInternal &out,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	(void)ray,(void)out,(void)shape,(void)transform;
	return false;
}

PfxBool intersectRayFuncBox(
				const PfxRayInputInternal &ray,PfxRayOutputInternal &out,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	PfxBox box = shape.getBox();
	box.m_half *= shape.getScale();
	
	return pfxIntersectRayBox(ray, out, box, pfxRemoveScale(transform, shape.getScale()));
}

PfxBool intersectRayFuncSphere(
				const PfxRayInputInternal &ray,PfxRayOutputInternal &out,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	PfxSphere sphere = shape.getSphere();
	sphere.m_radius *= shape.getScale();

	return pfxIntersectRaySphere(ray, out, sphere, pfxRemoveScale(transform, shape.getScale()));
}

PfxBool intersectRayFuncCapsule(
				const PfxRayInputInternal &ray,PfxRayOutputInternal &out,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	PfxCapsule capsule = shape.getCapsule();
	capsule.m_halfLen *= shape.getScale();
	capsule.m_radius *= shape.getScale();

	return pfxIntersectRayCapsule(ray, out, capsule, pfxRemoveScale(transform, shape.getScale()));
}

PfxBool intersectRayFuncCylinder(
				const PfxRayInputInternal &ray,PfxRayOutputInternal &out,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	PfxCylinder cylinder = shape.getCylinder();
	cylinder.m_halfLen *= shape.getScale();
	cylinder.m_radius *= shape.getScale();

	return pfxIntersectRayCylinder(ray, out, cylinder, pfxRemoveScale(transform, shape.getScale()));
}

PfxBool intersectRayFuncConvex(
				const PfxRayInputInternal &ray,PfxRayOutputInternal &out,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	const PfxConvexMeshImpl *convexMesh = (PfxConvexMeshImpl*)shape.getConvexMesh();
	
	PfxBool ret = pfxIntersectRayConvex(ray,out,(const void*)convexMesh,transform);
	
	return ret;
}

PfxBool intersectRayFuncLargeTriMesh(
				const PfxRayInputInternal &ray,PfxRayOutputInternal &out,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape.getLargeTriMesh();
	
	PfxBool ret = pfxIntersectRayLargeTriMesh(ray,out,(const void*)largeMesh,transform, discardTriangleCallback, userDataForDiscardingTriangles);
	
	return ret;
}

PfxIntersectRayFunc funcTbl_intersectRay[kPfxShapeCount] = {
	intersectRayFuncSphere,
	intersectRayFuncBox,
	intersectRayFuncCapsule,
	intersectRayFuncCylinder,
	intersectRayFuncConvex,
	intersectRayFuncLargeTriMesh,
	intersectRayFuncDummy, // Reserved
	intersectRayFuncDummy, // Reserved
	intersectRayFuncDummy, // Reserved
	intersectRayFuncDummy, // Reserved
	intersectRayFuncDummy, // Reserved
};

///////////////////////////////////////////////////////////////////////////////
// Ray Intersection Function Table Interface

PfxIntersectRayFunc pfxGetIntersectRayFunc(PfxUInt8 shapeType)
{
	return funcTbl_intersectRay[shapeType];
}

PfxInt32 pfxSetIntersectRayFunc(PfxUInt8 shapeType,PfxIntersectRayFunc func)
{
	if(shapeType >= kPfxShapeCount) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	funcTbl_intersectRay[shapeType] = func;
	
	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
