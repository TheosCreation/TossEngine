/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_collidable.h"
#include "../src/util/pfx_binary_reader_writer.h"

namespace sce {
namespace pfxv4 {

void PfxCollidable::finish()
{
	if(m_numShapes == 0) return;

	// compute AABB
	PfxVector3 halfMax(-SCE_PFX_FLT_MAX),halfMin(SCE_PFX_FLT_MAX);

	for(PfxUInt32 i=0;i<m_numShapes;i++) {
		const PfxShape &shape = m_shapes[i];

		if( (shape.getType() == kPfxShapeConvexMesh && !shape.isMeshReady()) ||
			(shape.getType() == kPfxShapeLargeTriMesh && !shape.isMeshReady()) ) {
			continue;
		}

		if (shape.getType() == kPfxShapeCoreSphere) {
			m_useCcd = true;
		}

		PfxVector3 aabbMin,aabbMax;
		shape.getAabb(aabbMin,aabbMax);
		halfMax = maxPerElem(halfMax,aabbMax);
		halfMin = minPerElem(halfMin,aabbMin);
	}

	for(PfxUInt32 i=0;i<m_numCoreSpheres;i++) {
		const PfxCoreSphere &coreSphere = m_coreSpheres[i];
		m_useCcd = true;
		PfxVector3 aabbMin,aabbMax;
		coreSphere.getAabb(aabbMin,aabbMax);
		halfMax = maxPerElem(halfMax,aabbMax);
		halfMin = minPerElem(halfMin,aabbMin);
	}

	PfxVector3 allCenter = ( halfMin + halfMax ) * 0.5f;
	PfxVector3 allHalf = ( halfMax - halfMin ) * 0.5f;
	m_center[0] = allCenter[0];
	m_center[1] = allCenter[1];
	m_center[2] = allCenter[2];
	m_half[0] = allHalf[0];
	m_half[1] = allHalf[1];
	m_half[2] = allHalf[2];
}

void PfxCollidable::calcAabb(const PfxQuat &offsetOrientation, PfxVector3 &center, PfxVector3 &extent) const
{
	if (m_numShapes == 0) return;

	// compute AABB
	PfxVector3 halfMax(-SCE_PFX_FLT_MAX), halfMin(SCE_PFX_FLT_MAX);

	for (PfxUInt32 i = 0; i<m_numShapes; i++) {
		PfxShape shape = m_shapes[i];

		if ((shape.getType() == kPfxShapeConvexMesh && !shape.isMeshReady()) ||
			(shape.getType() == kPfxShapeLargeTriMesh && !shape.isMeshReady())) {
			continue;
		}

		PfxVector3 pos = shape.getOffsetPosition();
		PfxQuat ori = shape.getOffsetOrientation();

		pos = rotate(offsetOrientation, pos);
		ori = offsetOrientation * ori;

		shape.setOffsetPosition(pos);
		shape.setOffsetOrientation(ori);

		PfxVector3 aabbMin, aabbMax;
		shape.getAabb(aabbMin, aabbMax);
		halfMax = maxPerElem(halfMax, aabbMax);
		halfMin = minPerElem(halfMin, aabbMin);
	}

	for (PfxUInt32 i = 0; i<m_numCoreSpheres; i++) {
		const PfxCoreSphere &coreSphere = m_coreSpheres[i];
		PfxVector3 aabbMin, aabbMax;
		coreSphere.getAabb(aabbMin, aabbMax);
		halfMax = maxPerElem(halfMax, aabbMax);
		halfMin = minPerElem(halfMin, aabbMin);
	}

	center = (halfMin + halfMax) * 0.5f;
	extent = (halfMax - halfMin) * 0.5f;
}

void PfxCollidable::save(PfxUInt8 *pout, PfxUInt32 bytes, PfxUInt32 shapeId, PfxUInt32 coreSphereId) const
{
	SCE_PFX_ASSERT(bytes == bytesOfCollidable);

	PfxUInt8 *p = pout;
	writeUInt32(&p, shapeId);
	writeUInt32(&p, coreSphereId);
	writeUInt8(&p, m_numShapes);
	writeUInt8(&p, m_numCoreSpheres);
	writeFloat32Array(&p, m_center, 3);
	writeFloat32Array(&p, m_half, 3);
	writeUInt32(&p, m_useCcd ? 1 : 0);
}

void PfxCollidable::load(const PfxUInt8 *pout, PfxUInt32 bytes, PfxShape *shapeBasePtr, PfxCoreSphere *coreSphereBasePtr)
{
	SCE_PFX_ASSERT(bytes == bytesOfCollidable);
	const PfxUInt8 *p = pout;

	PfxUInt32 shapeId, coreSphereId;

	readUInt32(&p, shapeId);
	readUInt32(&p, coreSphereId);
	readUInt8(&p, m_numShapes);
	readUInt8(&p, m_numCoreSpheres);
	readFloat32Array(&p, m_center, 3);
	readFloat32Array(&p, m_half, 3);
	PfxUInt32 useCcd = 0;
	readUInt32(&p, useCcd);
	m_useCcd = useCcd == 1;

	m_shapes = shapeBasePtr + shapeId;
	m_coreSpheres = coreSphereBasePtr + coreSphereId;
	m_maxShapes = m_numShapes;
	m_maxCoreSpheres = m_numCoreSpheres;
}

} //namespace pfxv4
} //namespace sce
