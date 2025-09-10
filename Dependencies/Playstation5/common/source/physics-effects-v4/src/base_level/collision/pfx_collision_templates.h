/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_COLLISION_TEMPLATES_H
#define _SCE_PFX_COLLISION_TEMPLATES_H

namespace sce {
namespace pfxv4 {

template<typename PfxShapeType, typename PfxLargeTriMeshIslandType>
PfxInt32 pfxContactTriMesh(
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA, const PfxLargeTriMeshIslandType *meshA, bool flipTriangle,
	const PfxShapeType &shapeB,
	const PfxTransform3 &transformB,
	PfxFloat distanceThreshold)
{
	// A specific function for each combination should be implemented
}

template<typename PfxShapeType, typename PfxLargeTriMeshIslandType>
PfxInt32 pfxClosestTriMesh(
	PfxContactCache &contacts,
	const PfxLargeTriMeshImpl *largeMeshA,const PfxLargeTriMeshIslandType *meshA,bool flipTriangle,
	const PfxShapeType &shapeB,
	const PfxTransform3 &transformB0, const PfxTransform3 &transformB1,
	const PfxVector3 &bsphereCenter,PfxFloat bsphereRadius,
	PfxFloat distanceThreshold )
{
	// A specific function for each combination should be implemented
}

} //namespace pfxv4
} //namespace sce


#endif // _SCE_PFX_COLLISION_TEMPLATES_H
