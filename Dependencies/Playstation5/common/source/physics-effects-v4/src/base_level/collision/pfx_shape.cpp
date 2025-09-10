/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_shape.h"
#include "../../../include/physics_effects/base_level/collision/pfx_convex_mesh.h"
#include "../../../include/physics_effects/base_level/collision/pfx_large_tri_mesh.h"
#include "../src/util/pfx_binary_reader_writer.h"

#include "pfx_convex_mesh_impl.h"
#include "pfx_large_tri_mesh_impl.h"

namespace sce {
namespace pfxv4 {

void pfxGetShapeAabbDummy(const PfxShape &shape,PfxVector3 &aabbMin,PfxVector3 &aabbMax)
{
	(void)shape,(void)aabbMin,(void)aabbMax;
}

void pfxGetShapeAabbSphere(const PfxShape &shape,PfxVector3 &aabbMin,PfxVector3 &aabbMax)
{
	aabbMin = shape.getOffsetPosition() - PfxVector3(shape.getScale() * shape.getSphere().m_radius);
	aabbMax = shape.getOffsetPosition() + PfxVector3(shape.getScale() * shape.getSphere().m_radius);
}

void pfxGetShapeAabbBox(const PfxShape &shape,PfxVector3 &aabbMin,PfxVector3 &aabbMax)
{
	PfxVector3 boxSize = absPerElem(PfxMatrix3(shape.getOffsetOrientation())) * shape.getScale() * shape.getBox().m_half;
	aabbMin = shape.getOffsetPosition() - boxSize;
	aabbMax = shape.getOffsetPosition() + boxSize;
}

void pfxGetShapeAabbCapsule(const PfxShape &shape,PfxVector3 &aabbMin,PfxVector3 &aabbMax)
{
	PfxVector3 dir = rotate(shape.getOffsetOrientation(),PfxVector3(1.0f,0.0f,0.0f));
	PfxVector3 capSize = absPerElem(dir) * shape.getScale() * shape.getCapsule().m_halfLen +
		PfxVector3(shape.getScale() * shape.getCapsule().m_radius);
	aabbMin = shape.getOffsetPosition() - capSize;
	aabbMax = shape.getOffsetPosition() + capSize;
}

void pfxGetShapeAabbCylinder(const PfxShape &shape,PfxVector3 &aabbMin,PfxVector3 &aabbMax)
{
	PfxVector3 capSize = absPerElem(PfxMatrix3(shape.getOffsetOrientation())) * 
		shape.getScale() * PfxVector3(shape.getCylinder().m_halfLen,shape.getCylinder().m_radius,shape.getCylinder().m_radius);
	aabbMin = shape.getOffsetPosition() - capSize;
	aabbMax = shape.getOffsetPosition() + capSize;
}

// 0: Calculate AABB from OBB
// 1: Calculate AABB from Vertices
#define CONVEX_MESH_AABB_UPDATE_METHOD 1
#define LARGE_TRI_MESSH_AABB_UPDATE_METHOD 0

void pfxGetShapeAabbConvexMesh(const PfxShape &shape,PfxVector3 &aabbMin,PfxVector3 &aabbMax)
{
	SCE_PFX_ASSERT(sizeof(PfxConvexMeshImpl) <= sizeof(PfxConvexMesh));
	const PfxConvexMeshImpl *convex = (PfxConvexMeshImpl*)shape.getConvexMesh();
	SCE_PFX_ASSERT(convex);
#if CONVEX_MESH_AABB_UPDATE_METHOD == 0
	PfxVector3 half = absPerElem(PfxMatrix3(shape.getOffsetOrientation())) * mulPerElem(absPerElem(shape.getScaleXyz()), pfxReadVector3(convex->m_half));
	aabbMin = shape.getOffsetPosition() - half;
	aabbMax = shape.getOffsetPosition() + half;
#elif CONVEX_MESH_AABB_UPDATE_METHOD == 1
	PfxVector3 aabbMin_( SCE_PFX_FLT_MAX);
	PfxVector3 aabbMax_(-SCE_PFX_FLT_MAX);
	for (int i = 0; i < convex->m_numVerts; i++) {
		PfxVector3 v = pfxReadVector3(convex->m_verts + i * 3);
		v = rotate(shape.getOffsetOrientation(), mulPerElem(shape.getScaleXyz(), v));
		aabbMin_ = minPerElem(aabbMin_, v);
		aabbMax_ = maxPerElem(aabbMax_, v);
	}
	aabbMin = shape.getOffsetPosition() + aabbMin_;
	aabbMax = shape.getOffsetPosition() + aabbMax_;
#endif
}

void pfxGetShapeAabbLargeTriMesh(const PfxShape &shape,PfxVector3 &aabbMin,PfxVector3 &aabbMax)
{
	SCE_PFX_ASSERT(sizeof(PfxLargeTriMeshImpl) <= sizeof(PfxLargeTriMesh));
	const PfxLargeTriMeshImpl *largemesh = (PfxLargeTriMeshImpl*)shape.getLargeTriMesh();
	SCE_PFX_ASSERT(largemesh);
#if LARGE_TRI_MESSH_AABB_UPDATE_METHOD == 0
	PfxMatrix3 rot(shape.getOffsetOrientation());
	PfxVector3 half = absPerElem(rot) *  mulPerElem(absPerElem(shape.getScaleXyz()),largemesh->getHalf());
	PfxVector3 offset = rot * mulPerElem(shape.getScaleXyz(),largemesh->getOffset()) + shape.getOffsetPosition();
	aabbMin = offset - half;
	aabbMax = offset + half;
#elif LARGE_TRI_MESSH_AABB_UPDATE_METHOD == 1
	auto trans = [&](const PfxVector3 &v)
	{
		return shape.getOffsetPosition() + rotate(shape.getOffsetOrientation(), mulPerElem(shape.getScaleXyz(), v));
	};

	PfxVector3 aabbMin_(SCE_PFX_FLT_MAX);
	PfxVector3 aabbMax_(-SCE_PFX_FLT_MAX);
	for (int i = 0; i < largemesh->m_numIslands; i++) {
		if(largemesh->m_type == SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY) {
			const PfxExpandedTriMesh *island = ((PfxExpandedTriMesh*)largemesh->m_islands) + i;
			for (int j = 0; j < island->m_numVerts; j++) {
				PfxVector3 vertex = trans(island->m_verts[j]);
				aabbMin_ = minPerElem(aabbMin_, vertex);
				aabbMax_ = maxPerElem(aabbMax_, vertex);
			}
		}
		else if(largemesh->m_type == SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH) {
			const PfxQuantizedTriMeshBvh *island = ((PfxQuantizedTriMeshBvh*)largemesh->m_islands) + i;
			for (int j = 0; j < island->m_numVerts; j++) {
				PfxVector3 vertex = trans(largemesh->decodePosition(island->m_verts[j]));
				aabbMin_ = minPerElem(aabbMin_, vertex);
				aabbMax_ = maxPerElem(aabbMax_, vertex);
			}
		}
		else if (largemesh->m_type == SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION) {
			const PfxCompressedTriMesh *island = ((PfxCompressedTriMesh*)largemesh->m_islands) + i;
			for (int j = 0; j < island->m_numVerts; j++) {
				PfxQuantize3 *qvert = (PfxQuantize3*)largemesh->m_vertexBuffer + island->m_verts + j;
				PfxVector3 vertex = trans(largemesh->decodePosition(*qvert));
				aabbMin_ = minPerElem(aabbMin_, vertex);
				aabbMax_ = maxPerElem(aabbMax_, vertex);
			}
		}
	}
	aabbMin = aabbMin_;
	aabbMax = aabbMax_;
#endif
}

PfxFuncGetShapeAabb pfxFuncGetShapeAabb[kPfxShapeCount] = {
	pfxGetShapeAabbSphere,
	pfxGetShapeAabbBox,
	pfxGetShapeAabbCapsule,
	pfxGetShapeAabbCylinder,
	pfxGetShapeAabbConvexMesh,
	pfxGetShapeAabbLargeTriMesh,
	pfxGetShapeAabbSphere,
	pfxGetShapeAabbDummy,
	pfxGetShapeAabbDummy,
	pfxGetShapeAabbDummy,
	pfxGetShapeAabbDummy,
	pfxGetShapeAabbDummy,
};

void PfxShape::getAabb(PfxVector3 &aabbMin,PfxVector3 &aabbMax) const
{
	return pfxFuncGetShapeAabb[m_type](*this,aabbMin,aabbMax);
}

int pfxSetGetShapeAabbFunc(PfxUInt8 shapeType,PfxFuncGetShapeAabb func)
{
	if(shapeType >= kPfxShapeCount) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	pfxFuncGetShapeAabb[shapeType] = func;
	
	return SCE_PFX_OK;
}

void PfxShape::save(PfxUInt8 *pout, PfxUInt32 bytes, PfxUInt32 meshId, PfxUInt32 meshBytes) const
{
	SCE_PFX_ASSERT(bytes == bytesOfShape);

	PfxUInt8 *p = pout;
	writeUInt8(&p, m_type);
	writeFloat32Array(&p, m_scale, 3);
	writeFloat32Array(&p, m_offsetPosition, 3);
	writeFloat32Array(&p, m_offsetOrientation, 4);
	writeUInt32(&p, m_contactFilterSelf);
	writeUInt32(&p, m_contactFilterTarget);
	writeUInt32(&p, m_userData);
	if(m_type == kPfxShapeConvexMesh || m_type == kPfxShapeLargeTriMesh) {
		writeUInt32(&p, meshId);
		writeUInt32(&p, meshBytes);
	}
	else {
		writeFloat32Array(&p, m_vecDataF, 3);
	}
}

void PfxShape::load(const PfxUInt8 *pout, PfxUInt32 bytes)
{
	SCE_PFX_ASSERT(bytes == bytesOfShape);

	const PfxUInt8 *p = pout;
	readUInt8(&p, m_type);
	readFloat32Array(&p, m_scale, 3);
	readFloat32Array(&p, m_offsetPosition, 3);
	readFloat32Array(&p, m_offsetOrientation, 4);
	readUInt32(&p, m_contactFilterSelf);
	readUInt32(&p, m_contactFilterTarget);
	readUInt32(&p, m_userData);
	if(m_type == kPfxShapeConvexMesh || m_type == kPfxShapeLargeTriMesh) {
		PfxUInt32 meshId, meshBytes;
		readUInt32(&p, meshId);
		readUInt32(&p, meshBytes);

		PfxUInt64 param = ((PfxUInt64)meshBytes << 32) | (PfxUInt64)meshId;
		m_vecDataPtr[0] = param;
	}
	else {
		readFloat32Array(&p, m_vecDataF, 3);
	}
}

} //namespace pfxv4
} //namespace sce
