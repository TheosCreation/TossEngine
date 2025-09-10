/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2021 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_progressive_bvh.h"
#include "../../../src/util/pfx_binary_reader_writer.h"

#define INITIAL_CAPACITY 32

namespace sce {
namespace pfxv4 {

PfxUInt32 PfxProgressiveBvh::queryBytes(int capacity, int maxProxies)
{
	capacity = SCE_PFX_MAX(INITIAL_CAPACITY, capacity);

	int depth = ceilf(log2f(capacity)) + 1;
	int maxNodes = (1 << depth) - 1;

	PfxUInt32 bytes = 0;
	bytes += sizeof(PfxUInt16) * maxProxies; // 
	bytes += sizeof(PfxUInt16) * maxProxies; // 
	bytes += sizeof(PfxBvNode) * maxNodes; // 
	bytes += sizeof(PfxLeafAttrib) * maxProxies; // 
	return bytes;
}

bool PfxProgressiveBvh::initialize(int capacity, int maxProxies, void *buffer, int bufferBytes)
{
	if (bufferBytes < queryBytes(capacity, maxProxies)) return false;
	if( !SCE_PFX_PTR_IS_ALIGNED16( buffer ) ) return false;

	m_capacity = SCE_PFX_MAX(INITIAL_CAPACITY, capacity);
	m_maxProxies = maxProxies;

	// calculate maximum capacities
	int layers = ceilf(log2f(m_capacity));
	int depth = layers + 1;
	int maxNodes = (1 << depth) - 1;
	m_capacity = 1 << layers;

	uint8_t *p = (uint8_t*)buffer;
	m_bvNodes = ( PfxBvNode* )p;
	p += sizeof( PfxBvNode ) * maxNodes;
	m_poolNodeIds = (PfxUInt16*)p;
	p += sizeof(PfxUInt16) * m_maxProxies;
	m_proxyTable = (PfxUInt16*)p;
	p += sizeof(PfxUInt16) * m_maxProxies;
	m_leafAttrib = (PfxLeafAttrib*)p;

	return clear();
}

bool PfxProgressiveBvh::clear()
{
	m_layers = ceilf(log2f(INITIAL_CAPACITY));
	m_numProxies = 0;
	m_numPoolNodes = 0;
	m_numLeaves = 1 << m_layers;

	m_rand.seed(9999 + m_capacity);

	for (int i = 0; i < m_maxProxies; i++) {
		m_proxyTable[i] = 0xffff;
	}

	{
		int startId = 0, num = 1 << m_layers;
		startId = num - 1;

		int j = startId;
		for (; j < startId + num; j++) {
			m_bvNodes[j] = PfxBvNode();
			m_bvNodes[j].setAabb(PfxLargePosition(-9999.0f), PfxLargePosition(9999.0f));
		}
	}

	constructInitialTree();

	return true;
}

bool PfxProgressiveBvh::expandTree()
{
	int nextLeaves = m_numLeaves << 1;

	if (nextLeaves > m_capacity) return false;

	// copy nodes in a upper layer to lower
	int upperLayer = m_layers;
	int numNodesInUpperLayer = 1 << upperLayer;
	int startIdInUpperLayer = numNodesInUpperLayer - 1;
	int lowerLayer = m_layers + 1;
	int numNodesInLowerLayer = 1 << lowerLayer;
	int startIdInLowerLayer = numNodesInLowerLayer - 1;

	int i = 0;
	for (; i < numNodesInUpperLayer; i++) {
		int upperNodeId = startIdInUpperLayer + i;
		int lowerNodeId = startIdInLowerLayer + i;
		m_bvNodes[lowerNodeId] = m_bvNodes[upperNodeId];
		m_bvNodes[lowerNodeId].parent = 0xffff;
		m_bvNodes[lowerNodeId].destination = 0xffff;
		m_bvNodes[lowerNodeId].shift = 0;
		if (m_bvNodes[upperNodeId].proxyId != 0xffff) {
			m_proxyTable[m_bvNodes[upperNodeId].proxyId] = lowerNodeId;
		}
	}

	for (; i < numNodesInLowerLayer; i++) {
		int lowerNodeId = startIdInLowerLayer + i;
		m_bvNodes[lowerNodeId] = PfxBvNode();
		m_bvNodes[lowerNodeId].setAabb(PfxLargePosition(-9999.0f), PfxLargePosition(9999.0f));
	}

	for (int j = 0; j < m_numPoolNodes; j++) {
		m_poolNodeIds[j] = startIdInLowerLayer + m_poolNodeIds[j] - startIdInUpperLayer;
	}

	m_layers++;
	m_numLeaves = nextLeaves;
	
	// reconstruct tree
	for( int layer = m_layers; layer > 0; layer-- ) {
		PfxInt32 total = ( 1 << layer ) / 2;

		int offset = ( 1 << layer ) - 1;

		for( PfxUInt32 i = 0; i < total; i++ ) {
			PfxUInt32 childL = offset + i * 2;
			PfxUInt32 childR = offset + i * 2 + 1;
			PfxUInt32 parent = ( childL - 1 ) >> 1;
			m_bvNodes[ childL ].parent = parent;
			m_bvNodes[ childR ].parent = parent;
			m_bvNodes[ parent ] = PfxProgressiveBvh::PfxBvNode();
			m_bvNodes[ parent ].setBv( pfxMergeBv( m_bvNodes[ childL ].bv, m_bvNodes[ childR ].bv ) );
			m_bvNodes[ parent ].valid = m_bvNodes[ childL ].valid | m_bvNodes[ childR ].valid;
			m_bvNodes[ parent ].shift = 1;
			m_bvNodes[ parent ].childL = childL;
			m_bvNodes[ parent ].childR = childR;
		}
	}
	
	setDestination();

	return true;
}

bool PfxProgressiveBvh::submit(PfxUInt16 proxyId, PfxUInt32 filterSelf, PfxUInt32 filterTarget, PfxUInt8 motionMask, PfxUInt8 solverQuality, PfxUInt16 ignoreGroup0, PfxUInt16 ignoreGroup1, const PfxLargePosition &center, const PfxVector3 &extent )
{
	if (proxyId >= m_maxProxies) return false;

	if (m_proxyTable[proxyId] == 0xffff) {
		if (m_numPoolNodes > 0) {
			m_numPoolNodes--;
			m_proxyTable[proxyId] = m_poolNodeIds[m_numPoolNodes];
		}
		else {
			if (m_numProxies >= m_capacity) { return false; }
			if (m_numProxies >= m_numLeaves) {
				if (!expandTree()) {
					return false;
				}
			}

			int startId = (1 << m_layers) - 1;
			m_proxyTable[proxyId] = startId + m_numProxies;
			m_numProxies++;
		}
	}

	PfxBvNode &leaf = m_bvNodes[m_proxyTable[proxyId]];


	PfxLargePosition vmin( center.segment, center.offset - extent );
	PfxLargePosition vmax( center.segment, center.offset + extent );

	leaf.bv = PfxBv( vmin, vmax );
	leaf.proxyId = proxyId;
	leaf.valid = 1;
	leaf.shift = 1;

	PfxLeafAttrib &attrib = m_leafAttrib[proxyId];

	attrib.filterSelf = filterSelf;
	attrib.filterTarget = filterTarget;
	attrib.ignoreGroup[0] = ignoreGroup0;
	attrib.ignoreGroup[1] = ignoreGroup1;
	attrib.motionMask = motionMask;
	attrib.solverQuality = solverQuality;

	// update AABBs
	PfxBvNode *parent = &leaf;
	while( parent->parent != 0xffff ) {
		parent = &m_bvNodes[ parent->parent ];
		parent->setBv( pfxMergeBv( m_bvNodes[ parent->childL ].bv, m_bvNodes[ parent->childR ].bv ) );
		parent->valid = m_bvNodes[ parent->childL ].valid | m_bvNodes[ parent->childR ].valid;
		parent->shift = 1;
	}

	return true;
}

bool PfxProgressiveBvh::remove(PfxUInt16 proxyId)
{
	// 'Update container' need to be called after removing proxies so that their destination will be set correctly
	if (proxyId >= m_maxProxies) {
		return false;
	}

	if (m_proxyTable[proxyId] != 0xffff) {
		PfxBvNode *leaf = &m_bvNodes[m_proxyTable[proxyId]];
		leaf->valid = 0;
		leaf->shift = 1;
		leaf->setAabb(PfxLargePosition(-9999.0f), PfxLargePosition(9999.0f));

		m_poolNodeIds[m_numPoolNodes++] = m_proxyTable[proxyId];
		m_proxyTable[proxyId] = 0xffff;
	}
	else {
		return false;
	}

	return true;
}

void PfxProgressiveBvh::constructInitialTree()
{
	for (int layer = m_layers; layer > 0; layer--) {
		int startId = 0, num = 1 << layer;
		startId = num - 1;

		for (int i = 0; i < num; i += 2) {
			PfxUInt32 childL = startId + i;
			PfxUInt32 childR = startId + i + 1;
			PfxUInt32 parent = (childL - 1) >> 1;

			m_bvNodes[childL].parent = parent;
			m_bvNodes[childR].parent = parent;
			m_bvNodes[parent] = PfxBvNode();
			m_bvNodes[parent].setAabb( PfxLargePosition( -9999.0f ), PfxLargePosition( 9999.0f ) );
			m_bvNodes[parent].valid = m_bvNodes[childL].valid | m_bvNodes[childR].valid;
			m_bvNodes[parent].shift = 1;
			m_bvNodes[parent].childL = childL;
			m_bvNodes[parent].childR = childR;
		}
	}

	setDestination();
}

void PfxProgressiveBvh::setDestination()
{
	PfxProgressiveBvh::PfxBvNode &root = m_bvNodes[ 0 ];
	root.destination = 0xffff;
	m_bvNodes[ root.childL ].destination = root.childR;
	m_bvNodes[ root.childR ].destination = root.destination;
	for( int layer = 1; layer <= m_layers - 1; layer++ ) {
		int startId = 0, num = 1 << layer;
		startId = num - 1;
		for( int i = 0; i < num; i++ ) {
			PfxProgressiveBvh::PfxBvNode &parent = m_bvNodes[ startId + i ];
			PfxUInt16 destL = parent.childR;
			PfxUInt16 destR = parent.destination;
			m_bvNodes[ parent.childL ].destination = destL;
			m_bvNodes[ parent.childR ].destination = destR;
		}
	}
}
PfxProgressiveBvh::eUpdateBvh PfxProgressiveBvh::update( PfxUInt16 proxyId, PfxUInt32 filterSelf, PfxUInt32 filterTarget, PfxUInt8 motionMask, PfxUInt8 solverQuality, PfxUInt16 ignoreGroup0, PfxUInt16 ignoreGroup1, const PfxLargePosition& center, const PfxVector3& extent, bool updateParents )
{
	if( m_proxyTable[ proxyId ] == 0xffff ) return kUpdateBvhProxyNotExist;

	PfxLargePosition vmin( center.segment, center.offset - extent );
	PfxLargePosition vmax( center.segment, center.offset + extent );
	PfxBv bv( vmin, vmax );

	PfxBvNode& leaf = m_bvNodes[ m_proxyTable[ proxyId ] ];

	PfxLeafAttrib& attrib = m_leafAttrib[ proxyId ];

	attrib.filterSelf = filterSelf;
	attrib.filterTarget = filterTarget;
	attrib.ignoreGroup[ 0 ] = ignoreGroup0;
	attrib.ignoreGroup[ 1 ] = ignoreGroup1;
	attrib.motionMask = motionMask;
	attrib.solverQuality = solverQuality;

	const PfxVector3 expand( SCE_PFX_BV_EXPAND ); // Add some extra volume for stabilization

	PfxVector3 previousExtent = leaf.bv.getExtent().convertToVector3();

	// if the previous BV includes the new BV and the gap of BVs is not so big, we don't need to update bvh. 
	if( pfxContain( leaf.bv, bv ) && lengthSqr( previousExtent ) < lengthSqr( 5.0f * extent ) ) {
		return kUpdateBvhOk;
	}
	else {
		leaf.bv = bv;
		leaf.bv.vmin.offset -= expand;
		leaf.bv.vmax.offset += expand;
		leaf.shift = 1;
	}

	// update AABBs
	if( updateParents )
	{
		PfxBvNode* parent = &leaf;
		while( parent->parent != 0xffff ) {
			parent = &m_bvNodes[ parent->parent ];
			parent->setBv( pfxMergeBv( m_bvNodes[ parent->childL ].bv, m_bvNodes[ parent->childR ].bv ) );
			parent->valid = m_bvNodes[ parent->childL ].valid | m_bvNodes[ parent->childR ].valid;
			parent->shift = 1;
		}
	}

	return kUpdateBvhAABBChanged;
}

void PfxProgressiveBvh::verify( bool displayOnlyErrors ) const
{
	for (int layer = 0; layer <= m_layers; layer++) {
		int num = 1 << layer;
		int start = num - 1;

		if( !displayOnlyErrors ) {
			SCE_PFX_PRINTF( "=== layer %d start %d num %d ===\n", layer, start, num );
		}

		for (int i = start; i < start + num; i++) {
			PfxBvNode &node = m_bvNodes[i];
			PfxBvNode &childL = m_bvNodes[node.childL];
			PfxBvNode &childR = m_bvNodes[node.childR];

			PfxBv bv = node.getBv();
			bv.vmin.changeSegment(PfxSegment());
			bv.vmax.changeSegment(PfxSegment());

			if( !displayOnlyErrors ) {
				SCE_PFX_PRINTF( "id %d RB %d valid %d shift %d dest %d parent %d child %d %d BV [%f,%f,%f] - [%f,%f,%f]\n", i,
					node.proxyId, node.valid, node.shift, node.destination, node.parent, node.childL, node.childR,
						( float )bv.vmin.offset[ 0 ], ( float )bv.vmin.offset[ 1 ], ( float )bv.vmin.offset[ 2 ],
							( float )bv.vmax.offset[ 0 ], ( float )bv.vmax.offset[ 1 ], ( float )bv.vmax.offset[ 2 ] );
			}

			if (layer < m_layers) {
				if (childL.valid) {
					PfxBv bvL = childL.getBv();
					bvL.vmin.changeSegment(PfxSegment());
					bvL.vmax.changeSegment(PfxSegment());
					if (!pfxTestBv(bvL, bv)) {
						SCE_PFX_PRINTF("ERROR!! L parent %d child %d %d BV [%f,%f,%f] - [%f,%f,%f]\n",
							childL.parent, childL.childL, childL.childR,
							(float)bvL.vmin.offset[0], (float)bvL.vmin.offset[1], (float)bvL.vmin.offset[2],
							(float)bvL.vmax.offset[0], (float)bvL.vmax.offset[1], (float)bvL.vmax.offset[2]);
					}
				}

				if (childR.valid) {
					PfxBv bvR = childR.getBv();
					bvR.vmin.changeSegment(PfxSegment());
					bvR.vmax.changeSegment(PfxSegment());
					if (!pfxTestBv(bvR, bv)) {
						SCE_PFX_PRINTF("ERROR!! R parent %d child %d %d BV [%f,%f,%f] - [%f,%f,%f]\n",
							childR.parent, childR.childL, childR.childR,
							(float)bvR.vmin.offset[0], (float)bvR.vmin.offset[1], (float)bvR.vmin.offset[2],
							(float)bvR.vmax.offset[0], (float)bvR.vmax.offset[1], (float)bvR.vmax.offset[2]);
					}
				}
			}
		}
	}
	
	// traversal check
	PfxUInt32 nodeId = m_bvNodes[ 0 ].childL;
	PfxUInt32 count = 0;
	while( nodeId != 0xffff ) {
		const PfxProgressiveBvh::PfxBvNode &node = m_bvNodes[ nodeId ];
		if( node.valid == 1 ) {
			if( node.isLeaf() ) {
				if( node.proxyId != 0xffff ) {
					count++;
				}
				nodeId = node.destination;
			}
			else {
				nodeId = node.childL;
			}
		}
		else {
			nodeId = node.destination;
		}
	}

	if(count != m_numProxies - m_numPoolNodes) {
		SCE_PFX_PRINTF( "ERROR!! traversal result %d proxies in BVH %d \n", count, m_numProxies - m_numPoolNodes );
	}
}

void PfxProgressiveBvh::print() const
{
	for (int layer = m_layers; layer >= 0; layer--) {
		int num = 1 << layer;
		int start = num - 1;

		SCE_PFX_PRINTF("=== layer %d start %d num %d ===\n", layer, start, num);

		for (int i = start; i < start + num; i++) {
			PfxBvNode &node = m_bvNodes[i];
			PfxBv bv = node.getBv();
			bv.vmin.changeSegment(PfxSegment());
			bv.vmax.changeSegment(PfxSegment());
			SCE_PFX_PRINTF("id %d RB %d dest %d parent %d child %d %d BV [%f,%f,%f] - [%f,%f,%f]\n", i,
				node.proxyId, node.destination, node.parent, node.childL, node.childR,
				(float)bv.vmin.offset[0], (float)bv.vmin.offset[1], (float)bv.vmin.offset[2],
				(float)bv.vmax.offset[0], (float)bv.vmax.offset[1], (float)bv.vmax.offset[2]);
		}
	}
}

bool PfxProgressiveBvh::getBv(PfxUInt32 proxyId, PfxLargePosition &bvMin, PfxLargePosition &bvMax) const
{
	if (m_proxyTable[proxyId] == 0xffff) return false;

	PfxBvNode &leaf = m_bvNodes[m_proxyTable[proxyId]];

	bvMin = leaf.bv.vmin;
	bvMax = leaf.bv.vmax;

	return true;
}

void PfxProgressiveBvh::traverse(pfxTraverseProxyContainerCallback callback, void *userData) const
{
	if (!callback || empty()) {
		return;
	}

	PfxUInt32 nodeId = m_bvNodes[0].childL;
	while (nodeId != 0xffff) {
		const PfxProgressiveBvh::PfxBvNode &node = m_bvNodes[nodeId];
		if (node.valid == 1) {
			if (node.isLeaf()) {
				if (node.proxyId != 0xffff) {
					if (!callback(node.proxyId, node.bv, userData)) {
						break;
					}
				}
				nodeId = node.destination;
			}
			else {
				nodeId = node.childL;
			}
		}
		else {
			nodeId = node.destination;
		}
	}
}

void PfxProgressiveBvh::traverseSphereOverlap(pfxTraverseProxyContainerCallback callback, const PfxLargePosition &sphereCenter, const PfxFloat sphereRadius, void *userData) const
{
	if (!callback || empty()) {
		return;
	}

	PfxUInt32 nodeId = m_bvNodes[0].childL;
	while (nodeId != 0xffff) {
		const PfxProgressiveBvh::PfxBvNode &node = m_bvNodes[nodeId];
		if (node.valid == 1 && pfxTestBvSphere(node.getBv(), sphereCenter, sphereRadius)) {
			if (node.isLeaf()) {
				if (node.proxyId != 0xffff) {
					if (!callback(node.proxyId, node.bv, userData)) {
						break;
					}
				}
				nodeId = node.destination;
			}
			else {
				nodeId = node.childL;
			}
		}
		else {
			nodeId = node.destination;
		}
	}
}

void PfxProgressiveBvh::traverseProxyOverlap(pfxTraverseProxyContainerCallback callback, const PfxBv &bv, void *userData) const
{
	if (!callback || empty()) {
		return;
	}

	PfxUInt32 nodeId = m_bvNodes[0].childL;
	while (nodeId != 0xffff) {
		const PfxProgressiveBvh::PfxBvNode &node = m_bvNodes[nodeId];
		if (node.valid == 1 && pfxTestBv(bv, node.getBv())) {
			if (node.isLeaf()) {
				if (node.proxyId != 0xffff) {
					if (!callback(node.proxyId, node.bv, userData)) {
						break;
					}
				}
				nodeId = node.destination;
			}
			else {
				nodeId = node.childL;
			}
		}
		else {
			nodeId = node.destination;
		}
	}
}

void PfxProgressiveBvh::traverseRayOverlap(pfxTraverseProxyContainerCallback callback, const PfxRayInput &ray, PfxFloat radius, void *userData) const
{
	if (!callback || empty()) {
		return;
	}

	PfxUInt32 nodeId = m_bvNodes[0].childL;
	while (nodeId != 0xffff) {
		const PfxProgressiveBvh::PfxBvNode &node = m_bvNodes[nodeId];
		PfxBv bv = node.getBv();
		bv.vmin.offset -= PfxVector3(radius);
		bv.vmax.offset += PfxVector3(radius);
		if (node.valid == 1 && pfxTestBvRay(bv, ray)) {
			if (node.isLeaf()) {
				if (node.proxyId != 0xffff) {
					if (!callback(node.proxyId, node.bv, userData)) {
						break;
					}
				}
				nodeId = node.destination;
			}
			else {
				nodeId = node.childL;
			}
		}
		else {
			nodeId = node.destination;
		}
	}
}

void PfxProgressiveBvh::traverseRayClosest(pfxTraverseProxyContainerCallbackForRayClipping callback, const PfxRayInput &ray, PfxFloat radius, void *userData) const
{
	if (!callback || empty()) {
		return;
	}

	PfxRayInput clipRay = ray;
	PfxFloat tmin = 1.0f;

	PfxUInt32 nodeId = m_bvNodes[0].childL;
	while (nodeId != 0xffff) {
		const PfxProgressiveBvh::PfxBvNode &node = m_bvNodes[nodeId];
		PfxBv bv = node.getBv();
		bv.vmin.offset -= PfxVector3(radius);
		bv.vmax.offset += PfxVector3(radius);
		if (node.valid == 1 && pfxTestBvRay(bv, clipRay)) {
			if (node.isLeaf()) {
				if (node.proxyId != 0xffff) {
					if (!callback(node.proxyId, node.bv, clipRay, tmin, userData)) {
						break;
					}
				}
				nodeId = node.destination;
			}
			else {
				nodeId = node.childL;
			}
		}
		else {
			nodeId = node.destination;
		}
	}
}

PfxUInt32 PfxProgressiveBvh::querySerializeBytes()
{
	PfxUInt32 bytes = 0;

	PfxUInt32 numNodes = (1 << (m_layers + 1)) - 1;

	bytes += 18;
	bytes += sizeof(PfxUInt16) * m_numPoolNodes;
	bytes += sizeof(PfxUInt16) * m_maxProxies;
	bytes += sizeof(PfxBvNode) * numNodes;
	bytes += sizeof(PfxLeafAttrib) * m_maxProxies;

	return bytes;
}

PfxInt32 PfxProgressiveBvh::save(PfxUInt8 *buffer, PfxUInt32 bytes)
{
	PfxUInt32 checkBytes = querySerializeBytes();
	if (bytes < checkBytes) return SCE_PFX_ERR_OUT_OF_BUFFER;

	PfxUInt8 *p = buffer;

	writeUInt32(&p, m_layers);
	writeUInt32(&p, m_maxProxies);
	writeUInt32(&p, m_numProxies);
	writeUInt32(&p, m_capacity);
	writeUInt16(&p, m_numPoolNodes);

	writeUInt16Array(&p, m_poolNodeIds, m_numPoolNodes);
	writeUInt16Array(&p, m_proxyTable, m_maxProxies);
	PfxUInt32 numNodes = (1 << (m_layers + 1)) - 1;
	writeUInt8Array(&p, (PfxUInt8*)m_bvNodes, sizeof(PfxBvNode) * numNodes);
	writeUInt8Array(&p, (PfxUInt8*)m_leafAttrib, sizeof(PfxLeafAttrib) * m_maxProxies);

	return SCE_PFX_OK;
}

PfxInt32 PfxProgressiveBvh::load(const PfxUInt8 *buffer, PfxUInt32 bytes)
{
	const PfxUInt8 *p = buffer;

	PfxUInt32 layers = 0;
	PfxUInt32 maxProxies = 0;
	PfxUInt32 numProxies = 0;
	PfxUInt32 capacity = 0;
	PfxUInt16 numPoolNodes = 0;

	readUInt32(&p, layers);
	readUInt32(&p, maxProxies);
	readUInt32(&p, numProxies);
	readUInt32(&p, capacity);
	readUInt16(&p, numPoolNodes);

	if (maxProxies > m_maxProxies || capacity > m_capacity) {
		return SCE_PFX_ERR_OUT_OF_BUFFER;
	}

	m_layers = layers;
	m_maxProxies = maxProxies;
	m_numProxies = numProxies;
	m_capacity = capacity;
	m_numPoolNodes = numPoolNodes;

	readUInt16Array(&p, m_poolNodeIds, m_numPoolNodes);
	readUInt16Array(&p, m_proxyTable, m_maxProxies);
	PfxUInt32 numNodes = (1 << (m_layers + 1)) - 1;
	readUInt8Array(&p, (PfxUInt8*)m_bvNodes, sizeof(PfxBvNode) * numNodes);
	readUInt8Array(&p, (PfxUInt8*)m_leafAttrib, sizeof(PfxLeafAttrib) * m_maxProxies);

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
