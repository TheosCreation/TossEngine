/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2021 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_PROGRESSIVE_BVH_H_
#define _SCE_PFX_PROGRESSIVE_BVH_H_

#include "../../../include/physics_effects/base_level/base/pfx_common.h"
#include "pfx_pair_container.h"
#include "../../../src/low_level/broadphase/pfx_bounding_volume.h"
#include "pfx_proxy_container_callback.h"
#include <random>

namespace sce {
namespace pfxv4 {

// Initially it starts with the defined size of a tree
// If the number of proxies exceeds its size, it expands tree until it meets its 
// maximum capacity by adding a layer and set true to m_requestReconstruction.
// In broadphase update, it reconstructs a tree if m_requestReconstruction is true.

struct PfxProgressiveBvh {
	struct SCE_PFX_ALIGNED(16) PfxBvNode {
		PfxBv bv;
		PfxUInt16 parent = 0xffff, childL = 0xffff, childR = 0xffff;
		PfxUInt16 proxyId = 0xffff;
		PfxUInt16 destination = 0xffff;
		PfxUInt8 valid = 0;
		PfxUInt8 shift = 0;

		void setAabb(const PfxLargePosition &vmin, const PfxLargePosition &vmax)
		{
			bv.vmin = vmin;
			bv.vmax = vmax;
		}

		void getAabb(PfxLargePosition &vmin, PfxLargePosition &vmax) const
		{
			vmin = bv.vmin;
			vmax = bv.vmax;
		}

		const PfxBv &getBv() const
		{
			return bv;
		}

		void setBv(const PfxBv &bv_)
		{
			bv = bv_;
		}

		bool isLeaf() const
		{
			return childL == 0xffff && childR == 0xffff;
		}
	};

	struct PfxLeafAttrib {
		PfxUInt8 motionMask;
		PfxUInt8 solverQuality;
		PfxUInt16 ignoreGroup[2];
		PfxUInt32 filterSelf;
		PfxUInt32 filterTarget;
	};

	PfxUInt32 m_layers; // m_layers + 1 = depth
	PfxUInt32 m_maxProxies; // biggest index
	PfxUInt32 m_numProxies = 0; // stored proxies
	PfxUInt32 m_capacity; // how many proxies can it store ?

	PfxUInt32 m_numLeaves;
	PfxUInt16 m_numPoolNodes;

	PfxUInt16 *m_poolNodeIds;
	PfxUInt16 *m_proxyTable;
	PfxBvNode *m_bvNodes;
	PfxLeafAttrib *m_leafAttrib;

	std::minstd_rand m_rand;

	static PfxUInt32 queryBytes(int capacity, int maxProxies);

	bool initialize(int capacity, int maxProxies, void *buffer, int bufferBytes);

	bool empty() const { return m_numProxies == 0; }

	bool clear();

	bool expandTree();

	bool submit(PfxUInt16 proxyId, PfxUInt32 filterSelf, PfxUInt32 filterTarget, PfxUInt8 motionMask, PfxUInt8 solverQuality, PfxUInt16 ignoreGroup0, PfxUInt16 ignoreGroup1, const PfxLargePosition &center, const PfxVector3 &extent );

	bool remove(PfxUInt16 proxyId);

	void constructInitialTree();

	void setDestination();

	enum eUpdateBvh {
		kUpdateBvhProxyNotExist,	// The specified proxy doesn't exist
		kUpdateBvhAABBChanged,		// The proxy updated correctly and AABB was expanded.
		kUpdateBvhOk,				// The proxy updated correctly. 
	};

	eUpdateBvh update(PfxUInt16 proxyId, PfxUInt32 filterSelf, PfxUInt32 filterTarget, PfxUInt8 motionMask, PfxUInt8 solverQuality, PfxUInt16 ignoreGroup0, PfxUInt16 ignoreGroup1, const PfxLargePosition &center, const PfxVector3 &extent, bool updateParents );

	void verify( bool displayOnlyErrors = false) const;

	void print() const;

	PfxUInt32 getCapacity() const { return m_capacity; }

	PfxUInt32 getNumProxies() const { return m_numProxies; }

	PfxUInt32 getMaxProxies() const { return m_maxProxies; }

	bool getBv(PfxUInt32 proxyId, PfxLargePosition &bvMin, PfxLargePosition &bvMax) const;

	void traverse(pfxTraverseProxyContainerCallback callback, void *userData) const;

	void traverseSphereOverlap(pfxTraverseProxyContainerCallback callback, const PfxLargePosition &sphereCenter, const PfxFloat sphereRadius, void *userData) const;

	void traverseProxyOverlap(pfxTraverseProxyContainerCallback callback, const PfxBv &bv, void *userData) const;

	void traverseRayOverlap(pfxTraverseProxyContainerCallback callback, const PfxRayInput &ray, PfxFloat radius, void *userData) const;

	void traverseRayClosest(pfxTraverseProxyContainerCallbackForRayClipping callback, const PfxRayInput &ray, PfxFloat radius, void *userData) const;

	PfxUInt32 querySerializeBytes();

	PfxInt32 save(PfxUInt8 *buffer, PfxUInt32 bytes);

	PfxInt32 load(const PfxUInt8 *buffer, PfxUInt32 bytes);
};

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_PROGRESSIVE_BVH_H_ */
