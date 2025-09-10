/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_world_binary_serializer.h"
#include "pfx_binary_reader_writer.h"
#include "../../include/physics_effects/util/pfx_mesh_serializer.h"

namespace sce {
namespace pfxv4 {

PfxUInt32 PfxWorldBinarySerializer::getPossiblePointerCount()
{
    PfxInt32 maxCount = 0;

    for(PfxUInt32 i = 0;i<m_paramForWrite.numRigidBodies;i++) {
        PfxCollidable &collidable = m_paramForWrite.collidables[i];
        maxCount++; // count a shape buffer for this collidable
		for(PfxUInt32 j=0;j<collidable.getNumShapes();j++) {
			const PfxShape &shape = collidable.getShape(j);
            if(shape.getType() == kPfxShapeConvexMesh || shape.getType() == kPfxShapeLargeTriMesh) {
                maxCount++; // count a mesh
            }
        }

		if(collidable.getNumCoreSpheres() > 0) {
            maxCount++; // count a core sphere buffer for this collidable
        }
    }

    return maxCount;
}

void PfxWorldBinarySerializer::aggregateCollidableInfo(CollidableSerializeInfo &cinfo)
{
    cinfo.shapeCount = 0;
    cinfo.coreSphereCount = 0;
    cinfo.convexMeshCount = 0;
    cinfo.largeMeshCount = 0;
    cinfo.convexMeshBytes = 0;
    cinfo.largeMeshBytes = 0;

    for(PfxUInt32 i = 0;i<m_paramForWrite.numRigidBodies;i++) {
        PfxCollidable &collidable = m_paramForWrite.collidables[i];

        if(collidable.getNumShapes() > 0) {
            uintptr_t shapePointer = (uintptr_t)&collidable.getShape(0);
            PointerFix pfix;
            if(shapePointer && !cinfo.pointerTable->find(shapePointer, pfix)) {
                pfix.id = cinfo.shapeCount;
                cinfo.pointerTable->insert(shapePointer, pfix);
                cinfo.shapeCount += collidable.getNumShapes();
            } 
        }

		for(PfxUInt32 j=0;j<collidable.getNumShapes();j++) {
			const PfxShape &shape = collidable.getShape(j);
            if(shape.getType() == kPfxShapeConvexMesh) {
                uintptr_t meshPointer = (uintptr_t)shape.getConvexMesh();
                PointerFix pfix;
                if(meshPointer && !cinfo.pointerTable->find(meshPointer, pfix)) {
                    PfxUInt32 meshBytes = pfxQuerySerializedBytesOfConvexMesh(*shape.getConvexMesh());
                    pfix.id = cinfo.convexMeshBytes;
                    pfix.bytes = meshBytes;
                    cinfo.pointerTable->insert(meshPointer, pfix);
                    cinfo.convexMeshBytes += meshBytes;
                    cinfo.convexMeshCount++;
                } 
            }
            else if(shape.getType() == kPfxShapeLargeTriMesh) {
                uintptr_t meshPointer = (uintptr_t)shape.getLargeTriMesh();
                PointerFix pfix;
                if(meshPointer && !cinfo.pointerTable->find(meshPointer, pfix)) {
                    PfxUInt32 meshBytes = pfxQuerySerializedBytesOfLargeTriMesh(*shape.getLargeTriMesh());
                    pfix.id = cinfo.largeMeshBytes;
                    pfix.bytes = meshBytes;
                    cinfo.pointerTable->insert(meshPointer, pfix);
                    cinfo.largeMeshBytes += meshBytes;
                    cinfo.largeMeshCount++;
                } 
            }
        }

		if(collidable.getNumCoreSpheres() > 0) {
            uintptr_t coreSpherePointer = (uintptr_t)&collidable.getCoreSphere(0);
			PointerFix pfix;
            if(coreSpherePointer && !cinfo.pointerTable->find(coreSpherePointer, pfix)) {
                pfix.id = cinfo.coreSphereCount;
                cinfo.pointerTable->insert(coreSpherePointer, pfix);
                cinfo.coreSphereCount += collidable.getNumCoreSpheres();
            } 
        }
    }
}

PfxUInt32 PfxWorldBinarySerializer::querySerializeBytesOfRigidStates()
{
    return  PfxRigidState::bytesOfRigidState * m_paramForWrite.numRigidBodies;
}

PfxUInt32 PfxWorldBinarySerializer::querySerializeBytesOfRigidBodies()
{
    return  PfxRigidBody::bytesOfRigidBody * m_paramForWrite.numRigidBodies;
}

PfxUInt32 PfxWorldBinarySerializer::querySerializeBytesOfCollidables(PfxUInt8 *workBuffer, PfxUInt32 workBytes)
{
    PfxUInt32 maxPtrCount = getPossiblePointerCount();

    PfxHeapManager pool(workBuffer, workBytes);
    PfxPoolMap<uintptr_t, PointerFix> pointerTable;
    pointerTable.initialize(&pool, maxPtrCount);

    CollidableSerializeInfo cinfo;
    cinfo.pointerTable = &pointerTable;

    aggregateCollidableInfo(cinfo);

    pointerTable.finalize();

    PfxInt32 bytes = HEADER_SIZE;
    bytes += PfxCollidable::bytesOfCollidable * m_paramForWrite.numRigidBodies;
    bytes += PfxShape::bytesOfShape * cinfo.shapeCount;
    bytes += PfxCoreSphere::bytesOfCoreSphere * cinfo.coreSphereCount;
    bytes += cinfo.convexMeshBytes;
    bytes += cinfo.largeMeshBytes;

    return bytes;
}

PfxUInt32 PfxWorldBinarySerializer::querySerializeBytesOfJoints()
{
    return PfxJoint::bytesOfJoint * m_paramForWrite.numJoints;
}

PfxInt32 PfxWorldBinarySerializer::loadRigidStates(const PfxUInt8 *buffer, PfxUInt32 bytes)
{
	const PfxUInt8 *statePtr = buffer;

    for(PfxUInt32 i = 0;i<m_paramForRead.numRigidBodies;i++) {
        PfxRigidState &state = m_paramForRead.states[i];
        state.load(statePtr, PfxRigidState::bytesOfRigidState);
        statePtr += PfxRigidState::bytesOfRigidState;
    }
    
    return SCE_PFX_OK;
}

PfxInt32 PfxWorldBinarySerializer::loadRigidBodies(const PfxUInt8 *buffer, PfxUInt32 bytes)
{
	const PfxUInt8 *bodyPtr = buffer;

    for(PfxUInt32 i = 0;i<m_paramForRead.numRigidBodies;i++) {
        PfxRigidBody &body = m_paramForRead.bodies[i];
        body.load(bodyPtr, PfxRigidBody::bytesOfRigidBody);
        bodyPtr += PfxRigidBody::bytesOfRigidBody;
    }

    return SCE_PFX_OK;
}

PfxInt32 PfxWorldBinarySerializer::loadCollidables(const PfxUInt8 *buffer, PfxUInt32 bytes, PfxUInt8 *workBuffer, PfxUInt32 workBytes, PfxMeshCreationCallbacks &callbacks)
{
	const PfxUInt8 *p = buffer;

    PfxUInt32 shapeCount = 0;
    PfxUInt32 coreSphereCount = 0;
    PfxUInt32 convexMeshCount = 0;
    PfxUInt32 largeMeshCount = 0;
    PfxUInt32 convexMeshBytes = 0;
    PfxUInt32 largeMeshBytes = 0;

	readUInt32(&p, shapeCount);
	readUInt32(&p, coreSphereCount);
	readUInt32(&p, convexMeshCount);
	readUInt32(&p, largeMeshCount);
	readUInt32(&p, convexMeshBytes);
	readUInt32(&p, largeMeshBytes);

	m_paramForRead.numShapes = shapeCount;
	m_paramForRead.numCoreSpheres = coreSphereCount;

    if(m_paramForRead.numShapes > m_paramForRead.maxShapes ||
       m_paramForRead.numCoreSpheres > m_paramForRead.maxCoreSpheres) {
        return SCE_PFX_ERR_OUT_OF_BUFFER;
    }

    const PfxUInt8 *collidablePtr = buffer + HEADER_SIZE;
    const PfxUInt8 *shapePtr = collidablePtr + PfxCollidable::bytesOfCollidable * m_paramForRead.numRigidBodies;
    const PfxUInt8 *coreSpherePtr = shapePtr + PfxShape::bytesOfShape * shapeCount;
    const PfxUInt8 *convexMeshPtr = coreSpherePtr + PfxCoreSphere::bytesOfCoreSphere * coreSphereCount;
    const PfxUInt8 *largeTriMeshPtr = convexMeshPtr + convexMeshBytes;

    // load collidables
    for(PfxUInt32 i = 0;i<m_paramForRead.numRigidBodies;i++) {
        PfxCollidable &collidable = m_paramForRead.collidables[i];
        collidable.load(collidablePtr + PfxCollidable::bytesOfCollidable * i, PfxCollidable::bytesOfCollidable, m_paramForRead.shapes, m_paramForRead.coreSpheres);
    }

    // load shapes
    for(PfxUInt32 i = 0;i<m_paramForRead.numShapes;i++) {
        PfxShape &shape = m_paramForRead.shapes[i];
        shape.load(shapePtr + PfxShape::bytesOfShape * i, PfxShape::bytesOfShape);
    }

    // load core spheres
    for(PfxUInt32 i = 0;i<m_paramForRead.numCoreSpheres;i++) {
        PfxCoreSphere &coreSphere = m_paramForRead.coreSpheres[i];
        coreSphere.load(coreSpherePtr + PfxCoreSphere::bytesOfCoreSphere * i, PfxCoreSphere::bytesOfCoreSphere);
    }

    // restore meshes
	PfxUInt32 meshCount = convexMeshCount + largeMeshCount;
    if(workBytes < PfxPoolMap<uintptr_t, uintptr_t>::getBytes(meshCount)) return SCE_PFX_ERR_OUT_OF_BUFFER;
    PfxHeapManager pool(workBuffer, workBytes);
    PfxPoolMap<uintptr_t, uintptr_t> meshPointerMap;
    meshPointerMap.initialize(&pool, meshCount);

    for(PfxUInt32 i = 0;i<m_paramForRead.numShapes;i++) {
        PfxShape &shape = m_paramForRead.shapes[i];
        if(shape.getType() == kPfxShapeConvexMesh) {
            PfxUInt64 param = shape.getDataPtr64(0);
			const PfxUInt8 *meshBuff = convexMeshPtr + (param & 0xffffffff);
            PfxUInt32 meshBytes = param >> 32;

            uintptr_t restoredPtr = 0;
            if(!meshPointerMap.find((uintptr_t)meshBuff, restoredPtr)) {
                if(callbacks.restoreConvexMesh) {
                    restoredPtr = (uintptr_t)callbacks.restoreConvexMesh(meshBuff, meshBytes, callbacks.userDataForRestoreConvexMesh);
                }
                else {
                    return SCE_PFX_ERR_INVALID_VALUE;
                }
                meshPointerMap.insert((uintptr_t)meshBuff, (uintptr_t)restoredPtr);
                /*
                PfxInt32 allocBytes = pfxQueryBytesToRestoreConvexMesh(meshBuff, meshBytes);
                PfxUInt8 *allocBuff = (PfxUInt8*)SCE_PFX_UTIL_ALLOC(16, allocBytes);
                PfxInt32 ret = pfxRestoreConvexMesh(convexMesh, allocBuff, allocBytes, meshBuff, meshBytes);
                */
            }

            shape.setDataPtr64(0, (PfxUInt64)restoredPtr);
        }
        else if(shape.getType() == kPfxShapeLargeTriMesh) {
            PfxUInt64 param = shape.getDataPtr64(0);
			const PfxUInt8 *meshBuff = largeTriMeshPtr + (param & 0xffffffff);
            PfxUInt32 meshBytes = param >> 32;

            uintptr_t restoredPtr = 0;
            if(!meshPointerMap.find((uintptr_t)meshBuff, restoredPtr)) {
                if(callbacks.restoreLargeTriMesh) {
                    restoredPtr = (uintptr_t)callbacks.restoreLargeTriMesh(meshBuff, meshBytes, callbacks.userDataForRestoreLargeTriMesh);
                }
                else {
                    return SCE_PFX_ERR_INVALID_VALUE;
                }
                meshPointerMap.insert((uintptr_t)meshBuff, (uintptr_t)restoredPtr);
                /*
                PfxInt32 allocBytes = pfxQueryBytesToRestoreLargeTriMesh(meshBuff, meshBytes);
                PfxUInt8 *allocBuff = (PfxUInt8*)SCE_PFX_UTIL_ALLOC(16, allocBytes);
                PfxInt32 ret = pfxRestoreLargeTriMesh(largeMesh, allocBuff, allocBytes, meshBuff, meshBytes);
                */
            }

            shape.setDataPtr64(0, (PfxUInt64)restoredPtr);
        }
    }

    meshPointerMap.finalize();

    return SCE_PFX_OK;
}

PfxInt32 PfxWorldBinarySerializer::loadJoints(const PfxUInt8 *buffer, PfxUInt32 bytes)
{
	const PfxUInt8 *jointPtr = buffer;

	for (PfxUInt32 i = 0; i < m_paramForRead.numJoints; i++) {
		PfxJoint &joint = m_paramForRead.joints[i];
		joint.load(jointPtr, PfxJoint::bytesOfJoint);
		jointPtr += PfxJoint::bytesOfJoint;

		if (joint.m_active == 1) {
			pfxUpdateJointPairs(m_paramForRead.jointPairs[i], i, m_paramForRead.joints[i], m_paramForRead.states[joint.m_rigidBodyIdA], m_paramForRead.states[joint.m_rigidBodyIdB]);
		}
		else {
			memset( &m_paramForRead.jointPairs[ i ], 0, sizeof( PfxConstraintPair ) );
		}
	}

    return SCE_PFX_OK;
}

PfxInt32 PfxWorldBinarySerializer::queryHeaderFromSerializedWorld(PfxSerializedWorldHeader &worldHeader, const PfxUInt8 *buffer, PfxUInt32 bytes)
{
	if (!buffer || bytes < HEADER_SIZE) return SCE_PFX_ERR_INVALID_VALUE;

	const PfxUInt8 *p = buffer;

	// check the header information
	PfxUInt32 headerId;
	PfxUInt8 versionMajor;
	PfxUInt8 versionMinor;

	readUInt32(&p,headerId);
	readUInt8(&p,versionMajor);
	readUInt8(&p,versionMinor);

	if(headerId != HEADER_ID) {
		return SCE_PFX_ERR_SERIALIZE_INVALID_HEADER;
	}
	else if(versionMajor != VERSION_MAJOR || versionMinor != VERSION_MINOR) {
		return SCE_PFX_ERR_SERIALIZE_INVALID_VERSION;
	}

    PfxUInt32 rigidBodyCount = 0;
    PfxUInt32 jointCount = 0;
	PfxUInt32 rigidStatesBytes = 0;
	PfxUInt32 rigidBodiesBytes = 0;
	PfxUInt32 collidablesBytes = 0;
	PfxUInt32 jointsBytes = 0;

	readUInt32(&p, rigidBodyCount);
	readUInt32(&p, jointCount);
	readUInt32(&p, rigidStatesBytes);
	readUInt32(&p, rigidBodiesBytes);
	readUInt32(&p, collidablesBytes);
	readUInt32(&p, jointsBytes);

	if (bytes < HEADER_SIZE + rigidStatesBytes + rigidBodiesBytes + collidablesBytes + jointsBytes) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	PfxUInt32 padding = (PfxUInt32)(HEADER_SIZE - (p - buffer));
	p += padding;
	p += rigidStatesBytes;
	p += rigidBodiesBytes;

    PfxUInt32 shapeCount = 0;
    PfxUInt32 coreSphereCount = 0;
    PfxUInt32 convexMeshCount = 0;
    PfxUInt32 largeMeshCount = 0;
    PfxUInt32 convexMeshBytes = 0;
    PfxUInt32 largeMeshBytes = 0;

	readUInt32(&p, shapeCount);
	readUInt32(&p, coreSphereCount);
	readUInt32(&p, convexMeshCount);
	readUInt32(&p, largeMeshCount);
	readUInt32(&p, convexMeshBytes);
	readUInt32(&p, largeMeshBytes);

    PfxUInt32 meshCount = convexMeshCount + largeMeshCount;

	worldHeader.rigidBodyCount = rigidBodyCount;
	worldHeader.jointCount = jointCount;
	worldHeader.shapeCount = shapeCount;
	worldHeader.coreSphereCount = coreSphereCount;
	worldHeader.convexMeshCount = convexMeshCount;
	worldHeader.largeMeshCount = largeMeshCount;
	worldHeader.workBytes = PfxPoolMap<uintptr_t, uintptr_t>::getBytes(meshCount);

	return SCE_PFX_OK;
}

PfxInt32 PfxWorldBinarySerializer::readWorldBuffer(const PfxUInt8 *buffer, PfxUInt32 bytes, PfxUInt8 *workBuffer, PfxUInt32 workBytes, PfxMeshCreationCallbacks &callbacks)
{
	if (!buffer || bytes < HEADER_SIZE) return SCE_PFX_ERR_INVALID_VALUE;

	const PfxUInt8 *p = buffer;

	// check the header information
	PfxUInt32 headerId;
	PfxUInt8 versionMajor;
	PfxUInt8 versionMinor;

	readUInt32(&p,headerId);
	readUInt8(&p,versionMajor);
	readUInt8(&p,versionMinor);

	if(headerId != HEADER_ID) {
		return SCE_PFX_ERR_SERIALIZE_INVALID_HEADER;
	}
	else if(versionMajor != VERSION_MAJOR || versionMinor != VERSION_MINOR) {
		return SCE_PFX_ERR_SERIALIZE_INVALID_VERSION;
	}

    PfxUInt32 rigidBodyCount = 0;
    PfxUInt32 jointCount = 0;
	PfxUInt32 rigidStatesBytes = 0;
	PfxUInt32 rigidBodiesBytes = 0;
	PfxUInt32 collidablesBytes = 0;
	PfxUInt32 jointsBytes = 0;

	readUInt32(&p, rigidBodyCount);
	readUInt32(&p, jointCount);
	readUInt32(&p, rigidStatesBytes);
	readUInt32(&p, rigidBodiesBytes);
	readUInt32(&p, collidablesBytes);
	readUInt32(&p, jointsBytes);

	if (bytes < HEADER_SIZE + rigidStatesBytes + rigidBodiesBytes + collidablesBytes + jointsBytes) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	PfxUInt32 padding = (PfxUInt32)(HEADER_SIZE - (p - buffer));
	p += padding;

    // todo: support adding to an existing world
	m_paramForRead.numRigidBodies = rigidBodyCount;
	m_paramForRead.numJoints = jointCount;

    if(m_paramForRead.numRigidBodies > m_paramForRead.maxRigidBodies || 
       m_paramForRead.numJoints > m_paramForRead.maxJoints ) {
        return SCE_PFX_ERR_OUT_OF_BUFFER;
    }

	PfxInt32 ret;

    ret = loadRigidStates(p, rigidStatesBytes);
	if(ret != SCE_PFX_OK) return ret;
	p += rigidStatesBytes;

    ret = loadRigidBodies(p, rigidBodiesBytes);
	if(ret != SCE_PFX_OK) return ret;
	p += rigidBodiesBytes;

    ret = loadCollidables(p, collidablesBytes, workBuffer, workBytes, callbacks);
	if(ret != SCE_PFX_OK) return ret;
	p += collidablesBytes;

    ret = loadJoints(p, jointsBytes);
	if(ret != SCE_PFX_OK) return ret;

    return SCE_PFX_OK;
}

PfxInt32 PfxWorldBinarySerializer::saveRigidStates(PfxUInt8 *buffer, PfxUInt32 bytes)
{
    PfxUInt32 queryBytes = querySerializeBytesOfRigidStates();
    if(bytes < queryBytes) return SCE_PFX_ERR_OUT_OF_BUFFER;

    PfxUInt8 *statePtr = buffer;

    for(PfxUInt32 i = 0;i<m_paramForWrite.numRigidBodies;i++) {
        PfxRigidState &state = m_paramForWrite.states[i];
        state.save(statePtr, PfxRigidState::bytesOfRigidState);
        statePtr += PfxRigidState::bytesOfRigidState;
    }

    return SCE_PFX_OK;
}

PfxInt32 PfxWorldBinarySerializer::saveRigidBodies(PfxUInt8 *buffer, PfxUInt32 bytes)
{
    PfxUInt32 queryBytes = querySerializeBytesOfRigidBodies();
    if(bytes < queryBytes) return SCE_PFX_ERR_OUT_OF_BUFFER;

    PfxUInt8 *bodyPtr = buffer;

    for(PfxUInt32 i = 0;i<m_paramForWrite.numRigidBodies;i++) {
        PfxRigidBody &body = m_paramForWrite.bodies[i];
        body.save(bodyPtr, PfxRigidBody::bytesOfRigidBody);
        bodyPtr += PfxRigidBody::bytesOfRigidBody;
    }

    return SCE_PFX_OK;
}

PfxInt32 PfxWorldBinarySerializer::saveCollidables(PfxUInt8 *buffer, PfxUInt32 bytes, PfxUInt8 *workBuffer, PfxUInt32 workBytes)
{
    PfxUInt32 queryBytes = querySerializeBytesOfCollidables(workBuffer, workBytes);
    if(bytes < queryBytes) return SCE_PFX_ERR_OUT_OF_BUFFER;

    PfxUInt32 maxPtrCount = getPossiblePointerCount();

    PfxHeapManager pool(workBuffer, workBytes);
    PfxPoolMap<uintptr_t, PointerFix> pointerTable;
    pointerTable.initialize(&pool, maxPtrCount);

    CollidableSerializeInfo cinfo;
    cinfo.pointerTable = &pointerTable;

    aggregateCollidableInfo(cinfo);

    PfxUInt8 *p = buffer;

	writeUInt32(&p, cinfo.shapeCount);
	writeUInt32(&p, cinfo.coreSphereCount);
	writeUInt32(&p, cinfo.convexMeshCount);
	writeUInt32(&p, cinfo.largeMeshCount);
	writeUInt32(&p, cinfo.convexMeshBytes);
	writeUInt32(&p, cinfo.largeMeshBytes);

    PfxUInt8 *collidablePtr = buffer + HEADER_SIZE;
    PfxUInt8 *shapePtr = collidablePtr + PfxCollidable::bytesOfCollidable * m_paramForWrite.numRigidBodies;
    PfxUInt8 *coreSpherePtr = shapePtr + PfxShape::bytesOfShape * cinfo.shapeCount;
    PfxUInt8 *convexMeshPtr = coreSpherePtr + PfxCoreSphere::bytesOfCoreSphere * cinfo.coreSphereCount;
    PfxUInt8 *largeTriMeshPtr = convexMeshPtr + cinfo.convexMeshBytes;

    PfxUInt32 shapeCount = 0;
    PfxUInt32 coreSphereCount = 0;
    PfxUInt32 convexMeshBytes = 0;
    PfxUInt32 largeMeshBytes = 0;

    for(PfxUInt32 i = 0;i<m_paramForWrite.numRigidBodies;i++) {
        PfxCollidable &collidable = m_paramForWrite.collidables[i];
        PointerFix pfixShape;
        PointerFix pfixCoreSphere;

        if(collidable.getNumShapes() > 0) {
            uintptr_t shapePointer = (uintptr_t)&collidable.getShape(0);
            if(pointerTable.find(shapePointer, pfixShape) && pfixShape.id == shapeCount) {
                shapeCount += collidable.getNumShapes();
                for(PfxUInt32 j = 0;j<collidable.getNumShapes();j++) {
                    const PfxShape &shape = collidable.getShape(j);

                    PointerFix pfixMesh;
                    if(shape.getType() == kPfxShapeConvexMesh) {
                        uintptr_t meshPointer = (uintptr_t)shape.getConvexMesh();
                        if(pointerTable.find(meshPointer, pfixMesh) && pfixMesh.id == convexMeshBytes) {
                            // save a convex mesh
                            PfxInt32 ret = pfxSerializeConvexMesh(*shape.getConvexMesh(), convexMeshPtr + convexMeshBytes, pfixMesh.bytes);
							(void)ret;
							SCE_PFX_ASSERT(ret == SCE_PFX_OK);
                            convexMeshBytes += pfixMesh.bytes;
                        }
                    }
                    else if(shape.getType() == kPfxShapeLargeTriMesh) {
                        uintptr_t meshPointer = (uintptr_t)shape.getLargeTriMesh();
                        if(pointerTable.find(meshPointer, pfixMesh) && pfixMesh.id == largeMeshBytes) {
                            // save a large mesh
                            PfxInt32 ret = pfxSerializeLargeTriMesh(*shape.getLargeTriMesh(), largeTriMeshPtr + largeMeshBytes, pfixMesh.bytes);
							(void)ret;
							SCE_PFX_ASSERT(ret == SCE_PFX_OK);
                            largeMeshBytes += pfixMesh.bytes;
                        }
                    }

                    // save shape
                    shape.save(shapePtr, PfxShape::bytesOfShape, pfixMesh.id, pfixMesh.bytes);
                    shapePtr += PfxShape::bytesOfShape;
                }
            }
        }

        if(collidable.getNumCoreSpheres() > 0) {
            uintptr_t coreSpherePointer = (uintptr_t)&collidable.getCoreSphere(0);
            if(pointerTable.find(coreSpherePointer, pfixCoreSphere) && pfixCoreSphere.id == coreSphereCount) {
                coreSphereCount += collidable.getNumCoreSpheres();
                for(PfxUInt32 j = 0;j<collidable.getNumCoreSpheres();j++) {
                    const PfxCoreSphere &coreSphere = collidable.getCoreSphere(j);
                    // save core sphere
                    coreSphere.save(coreSpherePtr, PfxCoreSphere::bytesOfCoreSphere);
                    coreSpherePtr += PfxCoreSphere::bytesOfCoreSphere;
                }
            } 
        }

        // save collidable
        collidable.save(collidablePtr, PfxCollidable::bytesOfCollidable, pfixShape.id, pfixCoreSphere.id);
        collidablePtr += PfxCollidable::bytesOfCollidable;
    }

    pointerTable.finalize();

	SCE_PFX_ASSERT(largeTriMeshPtr + largeMeshBytes <= buffer + bytes );

    return SCE_PFX_OK;
}

PfxInt32 PfxWorldBinarySerializer::saveJoints(PfxUInt8 *buffer, PfxUInt32 bytes)
{
    PfxUInt32 queryBytes = querySerializeBytesOfJoints();
    if(bytes < queryBytes) return SCE_PFX_ERR_OUT_OF_BUFFER;

    PfxUInt8 *jointPtr = buffer;

    for(PfxUInt32 i = 0;i<m_paramForWrite.numJoints;i++) {
        PfxJoint &joint = m_paramForWrite.joints[i];
        joint.save(jointPtr, PfxJoint::bytesOfJoint);
        jointPtr += PfxJoint::bytesOfJoint;
    }

    return SCE_PFX_OK;
}

PfxUInt32 PfxWorldBinarySerializer::queryWorkBytesToSaveWorldBuffer()
{
    PfxUInt32 maxPtrCount = getPossiblePointerCount();
    return PfxPoolMap<uintptr_t, PointerFix>::getBytes(maxPtrCount);
}

PfxUInt32 PfxWorldBinarySerializer::querySerializeBytesOfWorldBuffer(PfxUInt8 *workBuffer, PfxUInt32 workBytes)
{
    PfxUInt32 bytes = HEADER_SIZE;
    bytes += querySerializeBytesOfRigidStates();
    bytes += querySerializeBytesOfRigidBodies();
    bytes += querySerializeBytesOfCollidables(workBuffer, workBytes);
    bytes += querySerializeBytesOfJoints();
    return bytes;
}

PfxInt32 PfxWorldBinarySerializer::writeWorldBuffer(PfxUInt8 *buffer, PfxUInt32 bytes, PfxUInt8 *workBuffer, PfxUInt32 workBytes)
{
	if(bytes < querySerializeBytesOfWorldBuffer(workBuffer, workBytes)) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxUInt8 *p = buffer;

	writeUInt32(&p, HEADER_ID);
	writeUInt8(&p, VERSION_MAJOR);
	writeUInt8(&p, VERSION_MINOR);

    // todo: support adding to an existing world
    PfxUInt32 rigidBodyCount = m_paramForWrite.numRigidBodies;
    PfxUInt32 jointCount = m_paramForWrite.numJoints;
	PfxUInt32 rigidStatesBytes = querySerializeBytesOfRigidStates();
	PfxUInt32 rigidBodiesBytes = querySerializeBytesOfRigidBodies();
	PfxUInt32 collidablesBytes = querySerializeBytesOfCollidables(workBuffer, workBytes);
	PfxUInt32 jointsBytes = querySerializeBytesOfJoints();

	writeUInt32(&p, rigidBodyCount);
	writeUInt32(&p, jointCount);
	writeUInt32(&p, rigidStatesBytes);
	writeUInt32(&p, rigidBodiesBytes);
	writeUInt32(&p, collidablesBytes);
	writeUInt32(&p, jointsBytes);

	PfxUInt32 padding = (PfxUInt32)(HEADER_SIZE - (p - buffer));
	p += padding;

    PfxInt32 ret;

    ret = saveRigidStates(p, rigidStatesBytes);
	if(ret != SCE_PFX_OK) return ret;
	p += rigidStatesBytes;

    ret = saveRigidBodies(p, rigidBodiesBytes);
	if(ret != SCE_PFX_OK) return ret;
	p += rigidBodiesBytes;

    ret = saveCollidables(p, collidablesBytes, workBuffer, workBytes);
	if(ret != SCE_PFX_OK) return ret;
	p += collidablesBytes;

    ret = saveJoints(p, jointsBytes);
	if(ret != SCE_PFX_OK) return ret;

    return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
