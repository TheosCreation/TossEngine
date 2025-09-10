/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2023 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../base_level/broadphase/pfx_check_collidable.h"
#include "../task/pfx_job_system.h"
#include "../rigidbody/pfx_context.h"
#include "../task/pfx_atomic.h"
#include "pfx_broadphase_stage.h"

namespace sce {
namespace pfxv4 {

extern PfxInt32 gSegmentWidth;
extern PfxFloat gSegmentWidthInv;

void calculateAabb( const PfxRigidState &state, const PfxCollidable &collidable, const PfxFloat timeStep, PfxVector3& center, PfxVector3& extent )
{
	if( collidable.isContinuous() ) {
		const PfxTransform3 tA0( state.getOrientation(), state.getPosition() );
		const PfxTransform3 tA1 = pfxIntegrateTransform( timeStep, tA0, state.getLinearVelocity(), state.getAngularVelocity() );
		PfxVector3 center0, extent0;
		PfxVector3 center1, extent1;

		collidable.calcAabb( state.getOrientation(), center0, extent0 );
		center0 += state.getPosition();

		collidable.calcAabb( PfxQuat( tA1.getUpper3x3() ), center1, extent1 );
		center1 += tA1.getTranslation();

		const PfxVector3 extraRotationDistance( length( state.getAngularVelocity() ) * length( extent0 ) * timeStep );

		const PfxVector3 b0 = center0 - extent0 - extraRotationDistance;
		const PfxVector3 b1 = center0 + extent0 + extraRotationDistance;
		const PfxVector3 b2 = center1 - extent0 - extraRotationDistance;
		const PfxVector3 b3 = center1 + extent0 + extraRotationDistance;

		PfxVector3 minAabb( SCE_PFX_FLT_MAX ), maxAabb( -SCE_PFX_FLT_MAX );
		minAabb = minPerElem( b0, b2 );
		maxAabb = maxPerElem( b1, b3 );
		center = ( maxAabb + minAabb ) * 0.5f;
		extent = ( maxAabb - minAabb ) * 0.5f;
	}
	else {
		collidable.calcAabb( state.getOrientation(), center, extent );
		center += state.getPosition();
	}
}

void PfxBroadphaseStage::job_updateBvh(void *data, int uid)
{
	UpdateBvhJobArg *arg = (UpdateBvhJobArg*)data;

	PfxProgressiveBvh &bvh = *arg->bvh;

	PfxLargePosition minWld(arg->stage->shared.worldMin);
	PfxLargePosition maxWld(arg->stage->shared.worldMax);

	PfxInt32 total = arg->total;
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 num = SCE_PFX_MAX(0, SCE_PFX_MIN(batch, (total - start)));
	PfxInt32 offset = (1 << bvh.m_layers) - 1;

	for (int i = start; i < start + num; i++) {
		PfxProgressiveBvh::PfxBvNode &leaf = bvh.m_bvNodes[offset + i];

		if( leaf.valid == 0 ) continue;

		if (arg->resetShiftFlag) {
			leaf.shift = 0;
		}

		PfxRigidState &state = arg->stage->shared.states[leaf.proxyId];
		PfxCollidable &collidable = arg->stage->shared.collidables[leaf.proxyId];

		if (!state.getIsInitialized()) {
			continue;
		}

		PfxSegment segment = state.getSegment();
		PfxVector3 center, extent;

		calculateAabb( state, collidable, arg->stage->shared.timeStep, center, extent );

		PfxSegment segMaxCheck = segment - maxWld.segment;
		PfxSegment segMinCheck = minWld.segment - segment;
		PfxVector3 maxCheck = PfxVector3((PfxFloat)segMaxCheck.x, (PfxFloat)segMaxCheck.y, (PfxFloat)segMaxCheck.z) - (maxWld.offset - (center - extent)) * gSegmentWidthInv;
		PfxVector3 minCheck = PfxVector3((PfxFloat)segMinCheck.x, (PfxFloat)segMinCheck.y, (PfxFloat)segMinCheck.z) - ((center + extent) - minWld.offset) * gSegmentWidthInv;

		if (maxElem(maxCheck) > 0.0f || maxElem(minCheck) > 0.0f) {
			// Out of world behavior
			if (arg->outOfWorldCallback) {
				arg->outOfWorldCallback(leaf.proxyId, arg->userDataForOutOfWorldCallback);
			}
			if (arg->fixOutOfWorldBody) {
				state.setMotionType(kPfxMotionTypeFixed);
			}
			state.setProxyShiftFlag(true);
			continue;
		}

		if (bvh.update(leaf.proxyId,
			state.getContactFilterSelf(),
			state.getContactFilterTarget(),
			state.getMotionMask(),
			state.getSolverQuality(),
			state.getCollisionIgnoreGroup(0),
			state.getCollisionIgnoreGroup(1),
			PfxLargePosition(segment, center),
			extent,
			false) == PfxProgressiveBvh::kUpdateBvhAABBChanged ) {
			state.setProxyShiftFlag(true);
		}
	}
}

void PfxBroadphaseStage::job_generateRandomSwap(void *data, int uid)
{
	UpdateBvhJobArg *arg = (UpdateBvhJobArg*)data;

	PfxProgressiveBvh &bvh = *arg->bvh;

	std::uniform_int_distribution<> rnd(0, arg->num);

	for (int i = 0; i < arg->total * 2; i++) {
		bool retry;
		PfxUInt32 swap;
		do {
			retry = false;
			swap = rnd(bvh.m_rand); // swap children of this pairs
			for (int j = 0; j < i; j++) {
				if (arg->stage->swapBodies[j] == swap) {
					retry = true;
				}
			}
		} while (retry);
		arg->stage->swapBodies[i] = swap;
	}
}

void PfxBroadphaseStage::job_swapBvh(void *data, int uid)
{
	UpdateBvhJobArg *arg = (UpdateBvhJobArg*)data;

	PfxProgressiveBvh &bvh = *arg->bvh;

	// 適当な剛体を入れ替える
	int layer = bvh.m_layers - 1;
	int offset = (1 << layer) - 1;

	PfxInt32 total = arg->total;
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 num = SCE_PFX_MAX(0, SCE_PFX_MIN(batch, (total - start)));

	for (int i = start; i < start + num; i++) {
		PfxUInt32 parentIdA = offset + arg->stage->swapBodies[i * 2];
		PfxUInt32 parentIdB = offset + arg->stage->swapBodies[i * 2 + 1];

		PfxProgressiveBvh::PfxBvNode &parentA = bvh.m_bvNodes[parentIdA];
		PfxProgressiveBvh::PfxBvNode &parentB = bvh.m_bvNodes[parentIdB];

		if (parentA.valid == 0 || parentB.valid == 0) continue;

		PfxProgressiveBvh::PfxBvNode* nodes[4];
		nodes[0] = &bvh.m_bvNodes[parentA.childL];
		nodes[1] = &bvh.m_bvNodes[parentA.childR];
		nodes[2] = &bvh.m_bvNodes[parentB.childL];
		nodes[3] = &bvh.m_bvNodes[parentB.childR];

		PfxBv bv[4];
		bv[0] = nodes[0]->getBv();
		bv[1] = nodes[1]->getBv();
		bv[2] = nodes[2]->getBv();
		bv[3] = nodes[3]->getBv();

		// [0,1] + [2,3]
		PfxFloat bvA = pfxCalcCombinedVolume(bv[0], bv[1]) + pfxCalcCombinedVolume(bv[2], bv[3]);

		// [0,3] + [1,2]
		PfxFloat bvB = pfxCalcCombinedVolume(bv[0], bv[3]) + pfxCalcCombinedVolume(bv[1], bv[2]);

		// [0,2] + [1,3]
		PfxFloat bvC = pfxCalcCombinedVolume(bv[0], bv[2]) + pfxCalcCombinedVolume(bv[1], bv[3]);

		// 入れ替え
		int pattern = 0;
		if (bvA < bvB) {
			if (bvA < bvC) {
				// A
			}
			else {
				// C
				pattern = 2;
			}
		}
		else {
			if (bvB < bvC) {
				// B
				pattern = 1;
			}
			else {
				// C
				pattern = 2;
			}
		}

		if (pattern == 1) {
			// 1,3を交換
			pfxSwap(parentA.childR, parentB.childR);
			pfxSwap(nodes[1], nodes[3]);
			nodes[1]->parent = parentIdA;
			nodes[3]->parent = parentIdB;
		}
		else if (pattern == 2) {
			// 1,2を交換
			pfxSwap(parentA.childR, parentB.childL);
			pfxSwap(nodes[1], nodes[2]);
			nodes[1]->parent = parentIdA;
			nodes[2]->parent = parentIdB;
		}

		parentA.shift = nodes[0]->shift | nodes[1]->shift;
		parentB.shift = nodes[2]->shift | nodes[3]->shift;
		parentA.valid = nodes[0]->valid | nodes[1]->valid;
		parentB.valid = nodes[2]->valid | nodes[3]->valid;
	}
}

void PfxBroadphaseStage::job_refineBvh(void *data, int uid)
{
	UpdateBvhJobArg *arg = (UpdateBvhJobArg*)data;

	PfxProgressiveBvh &bvh = *arg->bvh;

	PfxInt32 total = 1 << arg->layer;
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 num = SCE_PFX_MAX(0, SCE_PFX_MIN(batch, (total - start)));

	int offset = start + total - 1;

	for (PfxUInt32 i = 0; i < num; i++) {
		PfxProgressiveBvh::PfxBvNode &grandParent = bvh.m_bvNodes[offset + i];
		PfxProgressiveBvh::PfxBvNode &parentL = bvh.m_bvNodes[grandParent.childL];
		PfxProgressiveBvh::PfxBvNode &parentR = bvh.m_bvNodes[grandParent.childR];

		PfxProgressiveBvh::PfxBvNode* nodes[4];
		nodes[0] = &bvh.m_bvNodes[parentL.childL];
		nodes[1] = &bvh.m_bvNodes[parentL.childR];
		nodes[2] = &bvh.m_bvNodes[parentR.childL];
		nodes[3] = &bvh.m_bvNodes[parentR.childR];

		PfxBv bv[4];
		bv[0] = nodes[0]->getBv();
		bv[1] = nodes[1]->getBv();
		bv[2] = nodes[2]->getBv();
		bv[3] = nodes[3]->getBv();

		if (parentL.valid == 1 && parentR.valid == 1) {
			// [0,1] + [2,3]
			PfxFloat bvA = pfxCalcCombinedVolume(bv[0], bv[1]) + pfxCalcCombinedVolume(bv[2], bv[3]);

			// [0,3] + [1,2]
			PfxFloat bvB = pfxCalcCombinedVolume(bv[0], bv[3]) + pfxCalcCombinedVolume(bv[1], bv[2]);

			// [0,2] + [1,3]
			PfxFloat bvC = pfxCalcCombinedVolume(bv[0], bv[2]) + pfxCalcCombinedVolume(bv[1], bv[3]);

			// 入れ替え
			int pattern = 0;
			if (bvA < bvB) {
				if (bvA < bvC) {
					// A
				}
				else {
					// C
					pattern = 2;
				}
			}
			else {
				if (bvB < bvC) {
					// B
					pattern = 1;
				}
				else {
					// C
					pattern = 2;
				}
			}

			if (pattern == 1) {
				// 1,3を交換
				pfxSwap(parentL.childR, parentR.childR);
				pfxSwap(nodes[1], nodes[3]);
				nodes[1]->parent = grandParent.childL;
				nodes[3]->parent = grandParent.childR;
				pfxSwap(bv[1], bv[3]);
			}
			else if (pattern == 2) {
				// 1,2を交換
				pfxSwap(parentL.childR, parentR.childL);
				pfxSwap(nodes[1], nodes[2]);
				nodes[1]->parent = grandParent.childL;
				nodes[2]->parent = grandParent.childR;
				pfxSwap(bv[1], bv[2]);
			}
		}

		parentL.shift = nodes[0]->shift | nodes[1]->shift;
		parentR.shift = nodes[2]->shift | nodes[3]->shift;
		parentL.valid = nodes[0]->valid | nodes[1]->valid;
		parentR.valid = nodes[2]->valid | nodes[3]->valid;

		// 親のBVを更新
		if (nodes[0]->valid == 1 && nodes[1]->valid == 1) {
			parentL.setBv(pfxMergeBv(bv[0], bv[1]));
		}
		else if (nodes[0]->valid == 1 && nodes[1]->valid == 0) {
			parentL.setBv(bv[0]);
		}
		else if (nodes[0]->valid == 0 && nodes[1]->valid == 1) {
			parentL.setBv(bv[1]);
		}

		if (nodes[2]->valid == 1 && nodes[3]->valid == 1) {
			parentR.setBv(pfxMergeBv(bv[2], bv[3]));
		}
		else if (nodes[2]->valid == 1 && nodes[3]->valid == 0) {
			parentR.setBv(bv[2]);
		}
		else if (nodes[2]->valid == 0 && nodes[3]->valid == 1) {
			parentR.setBv(bv[3]);
		}
	}
}

void PfxBroadphaseStage::job_refineBvhTop(void *data, int uid)
{
	UpdateBvhJobArg *arg = (UpdateBvhJobArg*)data;

	PfxProgressiveBvh &bvh = *arg->bvh;

	PfxProgressiveBvh::PfxBvNode &root = bvh.m_bvNodes[0];

	PfxProgressiveBvh::PfxBvNode* nodes[2];
	nodes[0] = &bvh.m_bvNodes[1];
	nodes[1] = &bvh.m_bvNodes[2];

	if (nodes[0]->shift == 0 && nodes[1]->shift == 0) {
		root.shift = 0;
	}
	else {
		root.shift = 1;
	}

	// 親のBVを更新
	root.setBv(pfxMergeBv(nodes[0]->getBv(), nodes[1]->getBv()));
}

void PfxBroadphaseStage::job_setSkipPointersTop(void *data, int uid)
{
	UpdateBvhJobArg *arg = (UpdateBvhJobArg*)data;

	PfxProgressiveBvh &bvh = *arg->bvh;

	PfxProgressiveBvh::PfxBvNode &root = bvh.m_bvNodes[0];

	root.destination = 0xffff;

	bvh.m_bvNodes[root.childL].destination = root.childR;
	bvh.m_bvNodes[root.childR].destination = root.destination;
}

void PfxBroadphaseStage::job_setSkipPointers(void *data, int uid)
{
	UpdateBvhJobArg *arg = (UpdateBvhJobArg*)data;

	PfxProgressiveBvh &bvh = *arg->bvh;

	PfxInt32 total = 1 << arg->layer;
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 num = SCE_PFX_MAX(0, SCE_PFX_MIN(batch, (total - start)));

	int offset = start + total - 1;

	for (PfxUInt32 i = 0; i < num; i++) {
		PfxProgressiveBvh::PfxBvNode &parent = bvh.m_bvNodes[offset + i];
		PfxUInt16 destL = parent.childR;
		PfxUInt16 destR = parent.destination;

		bvh.m_bvNodes[parent.childL].destination = destL;
		bvh.m_bvNodes[parent.childR].destination = destR;
	}
}

void PfxBroadphaseStage::job_findOverlapPairsTopS(void *data, int uid)
{
	FindPairsJobArg *arg = (FindPairsJobArg*)data;

	const PfxProgressiveBvh &bvh = *arg->bvhA;

	const PfxProgressiveBvh::PfxBvNode &root = bvh.m_bvNodes[0];

	const PfxProgressiveBvh::PfxBvNode* nodes[2];
	nodes[0] = &bvh.m_bvNodes[1];
	nodes[1] = &bvh.m_bvNodes[2];

	arg->stage->numTmpPairs[0] = 0;
	arg->stage->numTmpPairs[1] = 0;
	arg->stage->numTmpPairs[2] = 0;

	if (root.shift == 1 && pfxTestBv(nodes[0]->getBv(), nodes[1]->getBv())) {
		arg->stage->numTmpPairs[0] = 1;
		arg->stage->tmpPairs[0][0] = root.childL;
		arg->stage->tmpPairs[0][1] = root.childR;
	}
}

void PfxBroadphaseStage::job_findOverlapPairsSelfS(void *data, int uid)
{
	FindPairsJobArg *arg = (FindPairsJobArg*)data;

	PfxContext *context = (PfxContext*)arg->stage->getRigidBodyContext();

	const PfxProgressiveBvh &bvh = *arg->bvhA;

	int src = arg->flip;
	int dst = (src + 1) % 3;

	arg->stage->numTmpPairs[(dst + 1) % 3] = 0; // ここで次の保存先のペア数をリセット

	PfxInt32 total = 1 << arg->layer;
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 end = SCE_PFX_MIN(start + batch, total);

	int num = 1 << arg->layer;
	int offset = num - 1;

	const int maxPairs = arg->stage->shared.maxPairs;
	const int maxWork = 100;
	PfxUInt16 workIdx[maxWork];

	int workNum = 0;

	for (int i = offset + start; i < offset + end; i++) {
		const PfxProgressiveBvh::PfxBvNode &parent = bvh.m_bvNodes[i];

		auto overlap = [&](PfxUInt16 childA, PfxUInt16 childB)
		{
			const PfxProgressiveBvh::PfxBvNode &nodeA = bvh.m_bvNodes[childA];
			const PfxProgressiveBvh::PfxBvNode &nodeB = bvh.m_bvNodes[childB];
			if (nodeA.valid == 1 && nodeB.valid == 1) {
				if (pfxTestBv(nodeA.getBv(), nodeB.getBv())) {
					workIdx[workNum++] = childA;
					workIdx[workNum++] = childB;
					if (workNum == maxWork) {
						if (arg->stage->numTmpPairs[dst] + workNum / 2 > maxPairs) {
							return false;
						}
						PfxUInt32 dstId = arg->stage->numTmpPairs[dst].fetch_add(workNum / 2);
						for (int k = 0; k < workNum; k++) {
							arg->stage->tmpPairs[dst][dstId * 2 + k] = workIdx[k];
						}
						workNum = 0;
					}
				}
			}
			return true;
		};

		if (parent.shift == 1) {
			if (!overlap(parent.childL, parent.childR)) break;
		}
	}

	if (workNum > 0) {
		if (arg->stage->numTmpPairs[dst] + workNum / 2 > maxPairs) {
			context->appendError(PfxContext::kStageFindOverlapPairs, arg->dispatchId, SCE_PFX_ERR_OUT_OF_MAX_PAIRS);
			return;
		}
		PfxUInt32 dstId = arg->stage->numTmpPairs[dst].fetch_add(workNum / 2);
		for (int k = 0; k < workNum; k++) {
			arg->stage->tmpPairs[dst][dstId * 2 + k] = workIdx[k];
		}
	}
}

void PfxBroadphaseStage::job_findOverlapPairsInterS(void *data, int uid)
{
	FindPairsJobArg *arg = (FindPairsJobArg*)data;

	PfxContext *context = (PfxContext*)arg->stage->getRigidBodyContext();

	const PfxProgressiveBvh &bvh = *arg->bvhA;

	int src = arg->flip;
	int dst = (src + 1) % 3;

	PfxInt32 total = arg->stage->numTmpPairs[src];
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 end = SCE_PFX_MIN(start + batch, total);

	const int maxPairs = arg->stage->shared.maxPairs;
	const int maxWork = 100;
	PfxUInt16 workIdx[maxWork];

	int workNum = 0;

	for (int i = start; i < end; i++) {
		PfxUInt16 parentId[2];
		parentId[0] = arg->stage->tmpPairs[src][i * 2];
		parentId[1] = arg->stage->tmpPairs[src][i * 2 + 1];

		const PfxProgressiveBvh::PfxBvNode &parentA = bvh.m_bvNodes[parentId[0]];
		const PfxProgressiveBvh::PfxBvNode &parentB = bvh.m_bvNodes[parentId[1]];

		auto overlap = [&](PfxUInt16 childA, PfxUInt16 childB)
		{
			const PfxProgressiveBvh::PfxBvNode &nodeA = bvh.m_bvNodes[childA];
			const PfxProgressiveBvh::PfxBvNode &nodeB = bvh.m_bvNodes[childB];
			if (nodeA.valid == 1 && nodeB.valid == 1) {
				if (pfxTestBv(nodeA.getBv(), nodeB.getBv())) {
					workIdx[workNum++] = childA;
					workIdx[workNum++] = childB;
					if (workNum == maxWork) {
						if (arg->stage->numTmpPairs[dst] + workNum / 2 > maxPairs) {
							return false;
						}
						PfxUInt32 dstId = arg->stage->numTmpPairs[dst].fetch_add(workNum / 2);
						for (int k = 0; k < workNum; k++) {
							arg->stage->tmpPairs[dst][dstId * 2 + k] = workIdx[k];
						}
						workNum = 0;
					}
				}
			}
			return true;
		};

		// 全組み合わせのBV交差チェック
		if (parentA.shift == 1 || parentB.shift == 1) {
			if (!overlap(parentA.childL, parentB.childL)) break;
			if (!overlap(parentA.childL, parentB.childR)) break;
			if (!overlap(parentA.childR, parentB.childL)) break;
			if (!overlap(parentA.childR, parentB.childR)) break;
		}
	}

	if (workNum > 0) {
		if (arg->stage->numTmpPairs[dst] + workNum / 2 > maxPairs) {
			context->appendError(PfxContext::kStageFindOverlapPairs, arg->dispatchId, SCE_PFX_ERR_OUT_OF_MAX_PAIRS);
			return;
		}
		PfxUInt32 dstId = arg->stage->numTmpPairs[dst].fetch_add(workNum / 2);
		for (int k = 0; k < workNum; k++) {
			arg->stage->tmpPairs[dst][dstId * 2 + k] = workIdx[k];
		}
	}
}

void PfxBroadphaseStage::job_findOverlapPairsBottomS(void *data, int uid)
{
	FindPairsJobArg *arg = (FindPairsJobArg*)data;

	PfxContext *context = (PfxContext*)arg->stage->getRigidBodyContext();

	const PfxProgressiveBvh &bvh = *arg->bvhA;

	int src = arg->flip;

	PfxUInt32 total = arg->stage->numTmpPairs[src];
	PfxUInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxUInt32 start = batch * uid;
	PfxUInt32 end = SCE_PFX_MIN(start + batch, total);

	const int maxPairs = arg->stage->shared.maxPairs;
	const int maxLocalPairs = 10;
	PfxBroadphasePair localPairs[maxLocalPairs];

	int workNum = 0;

	for (int i = start; i < end; i++) {
		PfxUInt16 leafId[2];
		leafId[0] = arg->stage->tmpPairs[src][i * 2];
		leafId[1] = arg->stage->tmpPairs[src][i * 2 + 1];

		const PfxProgressiveBvh::PfxBvNode &nodeA = bvh.m_bvNodes[leafId[0]];
		const PfxProgressiveBvh::PfxBvNode &nodeB = bvh.m_bvNodes[leafId[1]];

		const PfxProgressiveBvh::PfxLeafAttrib &attribA = bvh.m_leafAttrib[nodeA.proxyId];
		const PfxProgressiveBvh::PfxLeafAttrib &attribB = bvh.m_leafAttrib[nodeB.proxyId];

		//if (debugPrint && nodeA.proxyId == nodeB.proxyId) {
		//	bvh.verify();
		//	SCE_PFX_PRINTF("parent %d %d leaf %d %d proxyId %d\n", nodeA.parent, nodeB.parent, leafId[0], leafId[1], nodeA.proxyId);
		//}
		SCE_PFX_ASSERT(nodeA.proxyId != nodeB.proxyId);

		if (pfxCheckCollidableInBroadphase(
			attribA.filterSelf, attribA.filterTarget, attribA.motionMask, attribA.ignoreGroup[0], attribA.ignoreGroup[1],
			attribB.filterSelf, attribB.filterTarget, attribB.motionMask, attribB.ignoreGroup[0], attribB.ignoreGroup[1])) {

			PfxBroadphasePair &pair = localPairs[workNum++];
			pfxSetActive(pair, true);
			pfxSetSolverQuality(pair, SCE_PFX_MAX(attribA.solverQuality, attribB.solverQuality));
			if (nodeA.proxyId < nodeB.proxyId) {
				pfxSetMotionMaskA(pair, attribA.motionMask);
				pfxSetMotionMaskB(pair, attribB.motionMask);
				pfxSetKey(pair, pfxCreateUniqueKey(nodeA.proxyId, nodeB.proxyId));
			}
			else {
				pfxSetMotionMaskA(pair, attribB.motionMask);
				pfxSetMotionMaskB(pair, attribA.motionMask);
				pfxSetKey(pair, pfxCreateUniqueKey(nodeB.proxyId, nodeA.proxyId));
			}

			if (workNum == maxLocalPairs) {
				if (*arg->numOutPairs + workNum > maxPairs) {
					break;
				}
				PfxInt32 pairId = pfxAtomicAdd(arg->numOutPairs, workNum);
				for (int k = 0; k < workNum; k++) {
					arg->outPairs[pairId + k] = localPairs[k];
				}
				workNum = 0;
			}
		}
	}

	if (workNum > 0) {
		if (*arg->numOutPairs + workNum > maxPairs) {
			context->appendError(PfxContext::kStageFindOverlapPairs, arg->dispatchId, SCE_PFX_ERR_OUT_OF_MAX_PAIRS);
			return;
		}
		PfxInt32 pairId = pfxAtomicAdd(arg->numOutPairs, workNum);
		for (int k = 0; k < workNum; k++) {
			arg->outPairs[pairId + k] = localPairs[k];
		}
	}
}

void PfxBroadphaseStage::job_findOverlapPairsTopX(void *data, int uid)
{
	FindPairsJobArg *arg = (FindPairsJobArg*)data;

	arg->stage->numTmpPairs[0] = 0;
	arg->stage->numTmpPairs[1] = 0;
	arg->stage->numTmpPairs[2] = 0;

	const PfxProgressiveBvh &bvhA = *arg->bvhA;
	const PfxProgressiveBvh &bvhB = *arg->bvhB;

	const PfxProgressiveBvh::PfxBvNode &rootA = bvhA.m_bvNodes[0];
	const PfxProgressiveBvh::PfxBvNode &rootB = bvhB.m_bvNodes[0];

	auto overlap = [&](PfxUInt16 childA, PfxUInt16 childB)
	{
		const PfxProgressiveBvh::PfxBvNode &nodeA = bvhA.m_bvNodes[childA];
		const PfxProgressiveBvh::PfxBvNode &nodeB = bvhB.m_bvNodes[childB];
		if (nodeA.valid == 1 && nodeB.valid == 1) {
			if (pfxTestBv(nodeA.getBv(), nodeB.getBv())) {
				PfxUInt32 dstId = arg->stage->numTmpPairs[0]++;
				arg->stage->tmpPairs[0][dstId * 2] = childA;
				arg->stage->tmpPairs[0][dstId * 2 + 1] = childB;
			}
		}
	};

	overlap(rootA.childL, rootB.childL);
	overlap(rootA.childL, rootB.childR);
	overlap(rootA.childR, rootB.childL);
	overlap(rootA.childR, rootB.childR);
}

void PfxBroadphaseStage::job_findOverlapPairsInterX(void *data, int uid)
{
	FindPairsJobArg *arg = (FindPairsJobArg*)data;

	PfxContext *context = (PfxContext*)arg->stage->getRigidBodyContext();

	const PfxProgressiveBvh &bvhA = *arg->bvhA;
	const PfxProgressiveBvh &bvhB = *arg->bvhB;

	int src = arg->flip;
	int dst = (src + 1) % 3;

	arg->stage->numTmpPairs[(dst + 1) % 3] = 0; // ここで次の保存先のペア数をリセット

	PfxInt32 total = arg->stage->numTmpPairs[src];
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 end = SCE_PFX_MIN(start + batch, total);

	//if(frame % 100 == 0) SCE_PFX_PRINTF("inter layer %d total %d start %d num %d\n",arg->layer, total, start, end - start);

	const int maxPairs = arg->stage->shared.maxPairs;
	const int maxWork = 100;
	PfxUInt16 workIdx[maxWork];

	int workNum = 0;

	for (int i = start; i < end; i++) {
		PfxUInt16 parentId[2];
		parentId[0] = arg->stage->tmpPairs[src][i * 2];
		parentId[1] = arg->stage->tmpPairs[src][i * 2 + 1];

		const PfxProgressiveBvh::PfxBvNode &parentA = bvhA.m_bvNodes[parentId[0]];
		const PfxProgressiveBvh::PfxBvNode &parentB = bvhB.m_bvNodes[parentId[1]];

		auto overlap = [&](PfxUInt16 childA, PfxUInt16 childB)
		{
			const PfxProgressiveBvh::PfxBvNode &nodeA = bvhA.m_bvNodes[childA];
			const PfxProgressiveBvh::PfxBvNode &nodeB = bvhB.m_bvNodes[childB];
			if (nodeA.valid == 1 && nodeB.valid == 1) {
				if (pfxTestBv(nodeA.getBv(), nodeB.getBv())) {
					workIdx[workNum++] = childA;
					workIdx[workNum++] = childB;
					if (workNum == maxWork) {
						if (arg->stage->numTmpPairs[dst] + workNum / 2 > maxPairs) {
							return false;
						}
						PfxUInt32 dstId = arg->stage->numTmpPairs[dst].fetch_add(workNum / 2);
						for (int k = 0; k < workNum; k++) {
							arg->stage->tmpPairs[dst][dstId * 2 + k] = workIdx[k];
						}
						workNum = 0;
					}
				}
			}
			return true;
		};

		auto isBottomA = [&]()
		{
			return parentA.childL == 0xffff;
		};

		auto isBottomB = [&]()
		{
			return parentB.childL == 0xffff;
		};

		// 全組み合わせのBV交差チェック
		if (parentA.shift == 1 || parentB.shift == 1) {
			if (isBottomA()) {
				if (!overlap(parentId[0], parentB.childL)) break;
				if (!overlap(parentId[0], parentB.childR)) break;
			}
			else if (isBottomB()) {
				if (!overlap(parentA.childL, parentId[1])) break;
				if (!overlap(parentA.childR, parentId[1])) break;
			}
			else {
				if (!overlap(parentA.childL, parentB.childL)) break;
				if (!overlap(parentA.childL, parentB.childR)) break;
				if (!overlap(parentA.childR, parentB.childL)) break;
				if (!overlap(parentA.childR, parentB.childR)) break;
			}
		}
	}

	if (workNum > 0) {
		if (arg->stage->numTmpPairs[dst] + workNum / 2 > maxPairs) {
			context->appendError(PfxContext::kStageFindOverlapPairs, arg->dispatchId, SCE_PFX_ERR_OUT_OF_MAX_PAIRS);
			return;
		}
		PfxUInt32 dstId = arg->stage->numTmpPairs[dst].fetch_add(workNum / 2);
		for (int k = 0; k < workNum; k++) {
			arg->stage->tmpPairs[dst][dstId * 2 + k] = workIdx[k];
		}
	}
}

void PfxBroadphaseStage::job_findOverlapPairsBottomX(void *data, int uid)
{
	FindPairsJobArg *arg = (FindPairsJobArg*)data;

	PfxContext *context = (PfxContext*)arg->stage->getRigidBodyContext();

	const PfxProgressiveBvh &bvhA = *arg->bvhA;
	const PfxProgressiveBvh &bvhB = *arg->bvhB;

	int src = arg->flip;

	PfxUInt32 total = arg->stage->numTmpPairs[src];
	PfxUInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxUInt32 start = batch * uid;
	PfxUInt32 end = SCE_PFX_MIN(start + batch, total);

	const int maxPairs = arg->stage->shared.maxPairs;
	const int maxLocalPairs = 10;
	PfxBroadphasePair localPairs[maxLocalPairs];
	int workNum = 0;

	for (int i = start; i < end; i++) {
		PfxUInt16 leafId[2];
		leafId[0] = arg->stage->tmpPairs[src][i * 2];
		leafId[1] = arg->stage->tmpPairs[src][i * 2 + 1];

		const PfxProgressiveBvh::PfxBvNode &nodeA = bvhA.m_bvNodes[leafId[0]];
		const PfxProgressiveBvh::PfxBvNode &nodeB = bvhB.m_bvNodes[leafId[1]];

		const PfxProgressiveBvh::PfxLeafAttrib &attribA = bvhA.m_leafAttrib[nodeA.proxyId];
		const PfxProgressiveBvh::PfxLeafAttrib &attribB = bvhB.m_leafAttrib[nodeB.proxyId];

		if( nodeA.proxyId == nodeB.proxyId ) continue;

		if (pfxCheckCollidableInBroadphase(
			attribA.filterSelf, attribA.filterTarget, attribA.motionMask, attribA.ignoreGroup[0], attribA.ignoreGroup[1],
			attribB.filterSelf, attribB.filterTarget, attribB.motionMask, attribB.ignoreGroup[0], attribB.ignoreGroup[1])) {

			PfxBroadphasePair &pair = localPairs[workNum++];
			pfxSetActive(pair, true);
			pfxSetSolverQuality(pair, SCE_PFX_MAX(attribA.solverQuality, attribB.solverQuality));
			if (nodeA.proxyId < nodeB.proxyId) {
				pfxSetMotionMaskA(pair, attribA.motionMask);
				pfxSetMotionMaskB(pair, attribB.motionMask);
				pfxSetKey(pair, pfxCreateUniqueKey(nodeA.proxyId, nodeB.proxyId));
			}
			else {
				pfxSetMotionMaskA(pair, attribB.motionMask);
				pfxSetMotionMaskB(pair, attribA.motionMask);
				pfxSetKey(pair, pfxCreateUniqueKey(nodeB.proxyId, nodeA.proxyId));
			}

			if (workNum == maxLocalPairs) {
				if (sceAtomicLoad32(arg->numOutPairs) + workNum > maxPairs) {
					std::atomic_thread_fence(std::memory_order_acquire);
					break;
				}
				PfxInt32 pairId = pfxAtomicAdd(arg->numOutPairs, workNum);
				for (int k = 0; k < workNum; k++) {
					arg->outPairs[pairId + k] = localPairs[k];
				}
				workNum = 0;
			}
		}
	}

	if (workNum > 0) {
		if (sceAtomicLoad32(arg->numOutPairs) + workNum > maxPairs) {
			std::atomic_thread_fence(std::memory_order_acquire);
			context->appendError(PfxContext::kStageFindOverlapPairs, arg->dispatchId, SCE_PFX_ERR_OUT_OF_MAX_PAIRS);
			return;
		}
		PfxInt32 pairId = pfxAtomicAdd(arg->numOutPairs, workNum);
		for (int k = 0; k < workNum; k++) {
			arg->outPairs[pairId + k] = localPairs[k];
		}
	}
}

PfxInt32 PfxBroadphaseStage::updateBvh(PfxProgressiveBvh &bvh, PfxBool fixOutOfWorldBody, PfxBool resetShiftFlag, void(*outOfWorldCallback)(PfxUInt32, void *), void *userDataForOutOfWorldCallback)
{
	int numArgs = 4 + bvh.m_layers * 2;
	UpdateBvhJobArg *jobArgs = getRigidBodyContext()->allocate<UpdateBvhJobArg>(numArgs);
	if (!jobArgs) { return SCE_PFX_ERR_OUT_OF_BUFFER; }

	PfxInt32 dispatchId = getRigidBodyContext()->getDispatchId();
	PfxInt32 ret = SCE_OK;

	int njob = 0;

	{
		// Update BVH (args : 1)
		{
			PfxInt32 numJobs_ = getJobSystem()->getNumWorkerThreads();
			UpdateBvhJobArg &arg = jobArgs[njob++];
			arg.stage = this;
			arg.dispatchId = dispatchId;
			arg.bvh = &bvh;
			arg.total = bvh.m_numProxies;
			arg.numJobs = numJobs_;
			arg.fixOutOfWorldBody = fixOutOfWorldBody;
			arg.resetShiftFlag = resetShiftFlag;
			arg.outOfWorldCallback = outOfWorldCallback;
			arg.userDataForOutOfWorldCallback = userDataForOutOfWorldCallback;

			ret = getJobSystem()->dispatch(job_updateBvh, &arg, numJobs_, "update bvh");
			if (ret != SCE_OK) return ret;
		}

		ret = getJobSystem()->barrier();
		if (ret != SCE_OK) return ret;

		// Random Swap (args : 2)
		{
			PfxUInt32 numSwapBodies = SCE_PFX_MIN(bvh.m_numProxies * 0.1f, maxSwapBodies) / 2;
			if (numSwapBodies == 0 && bvh.m_numProxies > 2) numSwapBodies = 1;

			{
				UpdateBvhJobArg &arg = jobArgs[njob++];
				int n = (bvh.m_numProxies + 1) / 2;
				arg.stage = this;
				arg.dispatchId = dispatchId;
				arg.bvh = &bvh;
				arg.num = SCE_PFX_MAX(0, n - 1);
				arg.total = numSwapBodies;

				ret = getJobSystem()->dispatch(job_generateRandomSwap, &arg, "random swap");
				if (ret != SCE_OK) return ret;
			}

			ret = getJobSystem()->barrier();
			if (ret != SCE_OK) return ret;

			{
				PfxInt32 numJobs_ = getJobSystem()->getNumWorkerThreads() * 2;
				UpdateBvhJobArg &arg = jobArgs[njob++];
				arg.stage = this;
				arg.dispatchId = dispatchId;
				arg.bvh = &bvh;
				arg.total = numSwapBodies;
				arg.numJobs = numJobs_;

				ret = getJobSystem()->dispatch(job_swapBvh, &arg, numJobs_, "swap nodes");
				if (ret != SCE_OK) return ret;
			}
		}

		ret = getJobSystem()->barrier();
		if (ret != SCE_OK) return ret;

		// Refine BVH (args : number of layers)
		{
			int layer = bvh.m_layers - 2;
			for (; layer >= 0; layer--) {
				PfxInt32 numJobs_ = getJobSystem()->getNumWorkerThreads() * 2;
				{
					UpdateBvhJobArg &arg = jobArgs[njob++];
					arg.stage = this;
					arg.dispatchId = dispatchId;
					arg.bvh = &bvh;
					arg.layer = layer;
					arg.numJobs = numJobs_;

					ret = getJobSystem()->dispatch(job_refineBvh, &arg, numJobs_, "refine bvh");
					if (ret != SCE_OK) return ret;
				}

				ret = getJobSystem()->barrier();
				if (ret != SCE_OK) return ret;
			}

			{
				UpdateBvhJobArg &arg = jobArgs[njob++];
				arg.stage = this;
				arg.dispatchId = dispatchId;
				arg.bvh = &bvh;
				arg.layer = 1;
				arg.start = 0;
				arg.num = 2;

				ret = getJobSystem()->dispatch(job_refineBvhTop, &arg, "refine bvh");
				if (ret != SCE_OK) return ret;

				ret = getJobSystem()->barrier();
				if (ret != SCE_OK) return ret;
			}
		}

		// Set skip pointers (args : number of layers)
		{
			{
				UpdateBvhJobArg &arg = jobArgs[njob++];
				arg.stage = this;
				arg.dispatchId = dispatchId;
				arg.bvh = &bvh;
				arg.layer = 0;
				arg.start = 0;
				arg.num = 1;

				ret = getJobSystem()->dispatch(job_setSkipPointersTop, &arg, "set skip");
				if (ret != SCE_OK) return ret;

				ret = getJobSystem()->barrier();
				if (ret != SCE_OK) return ret;
			}

			for (int layer = 1; layer <= bvh.m_layers - 1; layer++) {

				PfxUInt32 numJobs_ = getJobSystem()->getNumWorkerThreads();
				{
					UpdateBvhJobArg &arg = jobArgs[njob++];
					arg.stage = this;
					arg.dispatchId = dispatchId;
					arg.bvh = &bvh;
					arg.layer = layer;
					arg.numJobs = numJobs_;

					ret = getJobSystem()->dispatch(job_setSkipPointers, &arg, numJobs_, "set skip");
					if (ret != SCE_OK) return ret;
				}

				ret = getJobSystem()->barrier();
				if (ret != SCE_OK) return ret;
			}
		}
	}

	//getJobSystem()->wait();
	//bvh.verify(true);

	return SCE_PFX_OK;
}

PfxInt32 PfxBroadphaseStage::findOverlapPairsS(const PfxProgressiveBvh &bvh, PfxBroadphasePair *outPairs, PfxInt32 *numOutPairs)
{
	if (shared.maxPairs == 0) return SCE_PFX_OK; // do nothing

	int flip = 0;

	int numArgs = 2 + bvh.m_layers * 2;
	FindPairsJobArg *jobArgs = getRigidBodyContext()->allocate<FindPairsJobArg>(numArgs);
	if (!jobArgs) { return SCE_PFX_ERR_OUT_OF_BUFFER; }

	PfxInt32 ret = SCE_OK;
	PfxInt32 dispatchId = getRigidBodyContext()->getDispatchId();

	int njob = 0;

	// Find Overlap Pairs (args : 1)
	{
		FindPairsJobArg &arg = jobArgs[njob++];
		arg.stage = this;
		arg.dispatchId = dispatchId;
		arg.bvhA = &bvh;
		arg.layer = 0;
		arg.start = 0;
		arg.num = 2;

		//SCE_PFX_PRINTF(" - start %d num %d\n", arg.start * 4, arg.num * 4);

		ret = getJobSystem()->dispatch(job_findOverlapPairsTopS, &arg, "find pairs top");
		if (ret != SCE_OK) return ret;
	}

	ret = getJobSystem()->barrier();
	if (ret != SCE_OK) return ret;

	for (int layer = 1; layer <= bvh.m_layers - 1; layer++) {
		//SCE_PFX_PRINTF("refine layer %d num %d\n", layer, num);
		PfxUInt32 numJobs_ = getJobSystem()->getNumWorkerThreads();

		// Self (args : 1 * layers)
		{
			FindPairsJobArg &arg = jobArgs[njob++];
			arg.stage = this;
			arg.dispatchId = dispatchId;
			arg.bvhA = &bvh;
			arg.numJobs = numJobs_;
			arg.layer = layer;
			arg.flip = flip;

			//SCE_PFX_PRINTF(" - start %d num %d\n", arg.start * 4, arg.num * 4);

			ret = getJobSystem()->dispatch(job_findOverlapPairsSelfS, &arg, numJobs_, "find pairs self");
			if (ret != SCE_OK) return ret;
		}

		// Inter (args : 1 * layers)
		// 検出されたペア数はdispath時にわからないので、まずはジョブをキックしておいて
		// ジョブ開始時に検出数を計算する
		{
			FindPairsJobArg &arg = jobArgs[njob++];
			arg.stage = this;
			arg.dispatchId = dispatchId;
			arg.bvhA = &bvh;
			arg.numJobs = numJobs_;
			arg.layer = layer;
			arg.flip = flip;

			//SCE_PFX_PRINTF(" - start %d num %d\n", arg.start * 4, arg.num * 4);

			ret = getJobSystem()->dispatch(job_findOverlapPairsInterS, &arg, numJobs_, "find pairs inter");
			if (ret != SCE_OK) return ret;
		}

		flip = (flip + 1) % 3;

		ret = getJobSystem()->barrier();
		if (ret != SCE_OK) return ret;
	}

	// Bottom (args : 1)
	{
		PfxInt32 numJobs_ = getJobSystem()->getNumWorkerThreads() * 2;
		{
			FindPairsJobArg &arg = jobArgs[njob++];
			arg.stage = this;
			arg.dispatchId = dispatchId;
			arg.bvhA = &bvh;
			arg.numJobs = numJobs_;
			arg.layer = bvh.m_layers;
			arg.flip = flip;
			arg.outPairs = outPairs;
			arg.numOutPairs = numOutPairs;

			//SCE_PFX_PRINTF(" - start %d num %d\n", arg.start * 4, arg.num * 4);

			ret = getJobSystem()->dispatch(job_findOverlapPairsBottomS, &arg, numJobs_, "find pairs bottom");
			if (ret != SCE_OK) return ret;
		}
	}

	ret = getJobSystem()->barrier();
	if (ret != SCE_OK) return ret;

	return SCE_PFX_OK;
}

PfxInt32 PfxBroadphaseStage::findOverlapPairsX(const PfxProgressiveBvh &bvhA, const PfxProgressiveBvh &bvhB, PfxBroadphasePair *outPairs, PfxInt32 *numOutPairs)
{
	if (shared.maxPairs == 0) return SCE_PFX_OK; // do nothing

	int flip = 0;

	int numArgs = 2 + SCE_PFX_MAX(bvhA.m_layers, bvhB.m_layers);
	FindPairsJobArg *jobArgs = getRigidBodyContext()->allocate<FindPairsJobArg>(numArgs);
	if (!jobArgs) { return SCE_PFX_ERR_OUT_OF_BUFFER; }

	PfxInt32 ret = SCE_OK;
	PfxInt32 dispatchId = getRigidBodyContext()->getDispatchId();

	int njob = 0;

	// Find Overlap Pairs (args : 1)
	{
		FindPairsJobArg &arg = jobArgs[njob++];
		arg.stage = this;
		arg.dispatchId = dispatchId;
		arg.bvhA = &bvhA;
		arg.bvhB = &bvhB;
		arg.layer = 0;
		arg.start = 0;
		arg.num = 2;

		//SCE_PFX_PRINTF(" - start %d num %d\n", arg.start * 4, arg.num * 4);

		ret = getJobSystem()->dispatch(job_findOverlapPairsTopX, &arg, "find pairs top");
		if (ret != SCE_OK) return ret;
	}

	ret = getJobSystem()->barrier();
	if (ret != SCE_OK) return ret;

	// とりあえずツリーの深さは同じとする
	int layers = SCE_PFX_MAX(bvhA.m_layers, bvhB.m_layers);
	for (int layer = 1; layer <= layers - 1; layer++) {
		//SCE_PFX_PRINTF("refine layer %d num %d\n", layer, num);
		PfxUInt32 numJobs_ = getJobSystem()->getNumWorkerThreads();

		// Inter (args : 1 * layers)
		// 検出されたペア数はdispath時にわからないので、まずはジョブをキックしておいて
		// ジョブ開始時に検出数を計算する
		{
			FindPairsJobArg &arg = jobArgs[njob++];
			arg.stage = this;
			arg.dispatchId = dispatchId;
			arg.bvhA = &bvhA;
			arg.bvhB = &bvhB;
			arg.numJobs = numJobs_;
			arg.layer = layer;
			arg.flip = flip;

			//SCE_PFX_PRINTF(" - start %d num %d\n", arg.start * 4, arg.num * 4);

			ret = getJobSystem()->dispatch(job_findOverlapPairsInterX, &arg, numJobs_, "find pairs inter");
			if (ret != SCE_OK) return ret;
		}

		flip = (flip + 1) % 3;

		ret = getJobSystem()->barrier();
		if (ret != SCE_OK) return ret;
	}

	// Bottom (args : 1)
	{
		PfxInt32 numJobs_ = getJobSystem()->getNumWorkerThreads() * 2;
		{
			FindPairsJobArg &arg = jobArgs[njob++];
			arg.stage = this;
			arg.dispatchId = dispatchId;
			arg.bvhA = &bvhA;
			arg.bvhB = &bvhB;
			arg.numJobs = numJobs_;
			arg.layer = bvhA.m_layers;
			arg.flip = flip;
			arg.outPairs = outPairs;
			arg.numOutPairs = numOutPairs;

			//SCE_PFX_PRINTF(" - start %d num %d\n", arg.start * 4, arg.num * 4);

			ret = getJobSystem()->dispatch(job_findOverlapPairsBottomX, &arg, numJobs_, "find pairs bottom");
			if (ret != SCE_OK) return ret;
		}
	}

	ret = getJobSystem()->barrier();
	if (ret != SCE_OK) return ret;

	return SCE_PFX_OK;
}

PfxInt32 PfxBroadphaseStage::findOverlapPairs(const PfxProgressiveBvh &bvhA, const PfxProgressiveBvh &bvhB, PfxBroadphasePair *outPairs, PfxInt32 *numOutPairs)
{
	//SCE_PFX_PRINTF("testFindOverlapPairsS seq:%d bvh:%d\n", i, i);
	if (&bvhA == &bvhB) {
		return findOverlapPairsS(bvhA, outPairs, numOutPairs);
	}
	else {
		return findOverlapPairsX(bvhA, bvhB, outPairs, numOutPairs);
	}
}

void PfxBroadphaseStage::printOverlapPairs(const PfxBroadphasePair *outPairs, const PfxInt32 numOutPairs)
{
	for (int i = 0; i < numOutPairs; i++) {
		const PfxBroadphasePair &pair = outPairs[i];
		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);
		SCE_PFX_PRINTF("pair %d RB %d %d\n", i, iA, iB);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Refine Pairs

#define SCE_PFX_SKIP_COLLISION_THRESHOLD 0.5f

PfxBool PfxBroadphaseStage::updateContactManifolds(
	PfxContactManager *contactManager, PfxUInt32 contactId,
	const PfxRigidState &stateA, const PfxCollidable &collA,
	const PfxRigidState &stateB, const PfxCollidable &collB)
{
	PfxFloat enegyA = lengthSqr(stateA.getLinearVelocity()) + lengthSqr(stateA.getAngularVelocity());
	PfxFloat enegyB = lengthSqr(stateB.getLinearVelocity()) + lengthSqr(stateB.getAngularVelocity());

	PfxContactHolder &contactHolder = contactManager->getContactHolder(contactId);

	PfxLargePosition lposA = stateA.getLargePosition();
	PfxLargePosition lposB = stateB.getLargePosition();
	lposB.changeSegment(lposA.segment);

	PfxTransform3 tA0(stateA.getOrientation(), lposA.offset);
	PfxTransform3 tB0(stateB.getOrientation(), lposB.offset);

	auto checkAabb = [](const PfxVector3 &aabbMinA, const PfxVector3 &aabbMaxA, const PfxVector3 &aabbMinB, const PfxVector3 &aabbMaxB) {
#ifdef PFX_ENABLE_AVX
		const __m128 vAabbMinA = sce_vectormath_asm128(aabbMinA.get128());
		const __m128 vAabbMaxA = sce_vectormath_asm128(aabbMaxA.get128());
		const __m128 vAabbMinB = sce_vectormath_asm128(aabbMinB.get128());
		const __m128 vAabbMaxB = sce_vectormath_asm128(aabbMaxB.get128());
		int test1 = _mm_movemask_ps(_mm_cmplt_ps(vAabbMaxA, vAabbMinB));
		int test2 = _mm_movemask_ps(_mm_cmpgt_ps(vAabbMinA, vAabbMaxB));
		if (((test1 | test2) & 0x07) != 0) return false;
#else
		if (aabbMaxA[0] < aabbMinB[0] || aabbMinA[0] > aabbMaxB[0]) return false;
		if (aabbMaxA[1] < aabbMinB[1] || aabbMinA[1] > aabbMaxB[1]) return false;
		if (aabbMaxA[2] < aabbMinB[2] || aabbMinA[2] > aabbMaxB[2]) return false;
#endif
		return true;
	};

	for (PfxUInt32 j = 0; j < collA.getNumShapes(); j++) {
		const PfxShape &shapeA = collA.getShape(j);

		PfxVector3 aabbMinA, aabbMaxA, centerA, extentA;
		shapeA.getAabb(aabbMinA, aabbMaxA);
		centerA = (aabbMaxA + aabbMinA) * 0.5f;
		extentA = (aabbMaxA - aabbMinA) * 0.5f;
		centerA = tA0.getTranslation() + tA0.getUpper3x3() * centerA;
		extentA = absPerElem(tA0.getUpper3x3()) * extentA;
		aabbMinA = centerA - extentA;
		aabbMaxA = centerA + extentA;

		for (PfxUInt32 k = 0; k < collB.getNumShapes(); k++) {
			const PfxShape &shapeB = collB.getShape(k);

			if (j == 0 && k == 0) continue;

			PfxVector3 aabbMinB, aabbMaxB, centerB, extentB;
			shapeB.getAabb(aabbMinB, aabbMaxB);
			centerB = (aabbMaxB + aabbMinB) * 0.5f;
			extentB = (aabbMaxB - aabbMinB) * 0.5f;
			centerB = tB0.getTranslation() + tB0.getUpper3x3() * centerB;
			extentB = absPerElem(tB0.getUpper3x3()) * extentB;
			aabbMinB = centerB - extentB;
			aabbMaxB = centerB + extentB;

			if (pfxCheckContactFilter(
				shapeA.getContactFilterSelf(), shapeA.getContactFilterTarget(),
				shapeB.getContactFilterSelf(), shapeB.getContactFilterTarget()) &&
				checkAabb(aabbMinA, aabbMaxA, aabbMinB, aabbMaxB)) {

				// create a new contact manifold
				PfxUInt16 shapeId = (k << 8) | j;

				// A contact manifold with shapeId=0 always exists
				PfxContactManifold *contactManifold = contactManager->insertContactManifoldToContactHolder(contactHolder, shapeId);
				if (contactManifold == nullptr) {
					return false;
				}

				if (enegyA > SCE_PFX_SKIP_COLLISION_THRESHOLD || enegyB > SCE_PFX_SKIP_COLLISION_THRESHOLD) {
					contactManifold->cachedAxis = PfxVector3::zero();
				}
			}
		}
	}

	return true;
}

void PfxBroadphaseStage::job_resetPairState(void *data, int uid)
{
	RefinePairsJobArg *arg = (RefinePairsJobArg*)data;

	arg->stage->pairLen = 0;

	PfxUInt32 total = arg->pairContainer->getPairLength();
	PfxUInt32 batch = (total + arg->num - 1) / arg->num;
	PfxUInt32 start = uid * batch;
	PfxUInt32 end = SCE_PFX_MIN(start + batch, total);

	for (PfxUInt32 i = start; i < end; i++) {
		PfxPairContainer::PairNode &pair = arg->pairContainer->getPairNode(i);
		if (pair.state != NODE_IS_NOT_ASSIGNED) pair.state = 0;
	}
}

void PfxBroadphaseStage::job_AddPairs(void *data, int uid)
{
	RefinePairsJobArg *arg = (RefinePairsJobArg*)data;

	PfxContext *context = (PfxContext*)arg->stage->getRigidBodyContext();

	PfxUInt32 total = *arg->numOutPairs;
	PfxUInt32 batch = (total + arg->num - 1) / arg->num;
	PfxUInt32 start = uid * batch;
	PfxUInt32 end = SCE_PFX_MIN(start + batch, total);

	PfxUInt32 swap = 0;
	PfxUInt32 *work[2] = { arg->stage->workBuff1 + start,  arg->stage->workBuff2 + start };
	PfxUInt32 numWork[2] = { 0, 0 };

	for (PfxUInt32 i = start; i < end; i++) {
		const PfxBroadphasePair &pair = arg->outPairs[i];
		PfxUInt32 pairId;
		PfxPairContainer::eResult result = arg->pairContainer->tryInsertPair(pair, pairId);

		if (result == PfxPairContainer::kFailedToLockPair && numWork[0] < arg->stage->shared.maxPairs) {
			work[0][numWork[0]++] = i;
		}
		else if (result == PfxPairContainer::kOverflowOfBodies) {
			context->appendError(PfxContext::kStageRefinePairs, arg->dispatchId, SCE_PFX_ERR_OUT_OF_MAX_BODIES);
			break;
		}
		else if (result == PfxPairContainer::kOverflowOfPairs) {
			context->appendError(PfxContext::kStageRefinePairs, arg->dispatchId, SCE_PFX_ERR_OUT_OF_MAX_PAIRS);
			break;
		}
	}

	while (numWork[swap] > 0) {
		for (PfxUInt32 i = 0; i < numWork[swap]; i++) {
			const PfxBroadphasePair &pair = arg->outPairs[work[swap][i]];
			PfxUInt32 pairId;
			PfxPairContainer::eResult result = arg->pairContainer->tryInsertPair(pair, pairId);

			if (result == PfxPairContainer::kFailedToLockPair) {
				work[1 - swap][numWork[1 - swap]++] = work[swap][i];
			}
			else if (result == PfxPairContainer::kOverflowOfBodies) {
				context->appendError(PfxContext::kStageRefinePairs, arg->dispatchId, SCE_PFX_ERR_OUT_OF_MAX_BODIES);
				break;
			}
			else if (result == PfxPairContainer::kOverflowOfPairs) {
				context->appendError(PfxContext::kStageRefinePairs, arg->dispatchId, SCE_PFX_ERR_OUT_OF_MAX_PAIRS);
				break;
			}
		}

		numWork[swap] = 0;
		swap = 1 - swap;
	}

	// it returns the total number of pairs
	PfxUInt32 numSrc = arg->pairContainer->getPairLength();
	PfxUInt32 numDst = arg->stage->pairLen.load(std::memory_order_acquire);

	bool ret = false;
	while (!ret && numSrc > numDst) {
		ret = arg->stage->pairLen.compare_exchange_strong(numDst, numSrc);
	}
}

void PfxBroadphaseStage::job_removeUnusedPairs(void *data, int uid)
{
	RefinePairsJobArg *arg = (RefinePairsJobArg*)data;

	PfxContext *context = (PfxContext*)arg->stage->getRigidBodyContext();

	PfxUInt32 total = arg->stage->pairLen;
	PfxUInt32 batch = (total + arg->num - 1) / arg->num;
	PfxUInt32 start = uid * batch;
	PfxUInt32 end = SCE_PFX_MIN(start + batch, total);

	PfxUInt32 swap = 0;
	PfxUInt32 *work[2] = { arg->stage->workBuff1 + start,  arg->stage->workBuff2 + start };
	PfxUInt32 numWork[2] = { 0, 0 };

	auto newPair = [&](PfxUInt32 &contactId, PfxUInt32 iA, PfxUInt32 iB) {
		//nnew++;
		PfxInt32 ret = arg->contactManager->createContactHolder(contactId, iA, iB);
		if (ret != SCE_PFX_OK) { return ret; }

		// wake up sleeping bodies
		if ((arg->dontWakeUp) == 0) {
			PfxRigidState &stateA = arg->stage->shared.states[iA];
			PfxRigidState &stateB = arg->stage->shared.states[iB];
			if (stateA.isAsleep() && stateB.getMotionType() != kPfxMotionTypeFixed) {
				stateA.wakeup();
			}
			if (stateB.isAsleep() && stateA.getMotionType() != kPfxMotionTypeFixed) {
				stateB.wakeup();
			}
		}

		return ret;
	};

	auto removePair = [&](PfxUInt32 contactId, PfxUInt32 iA, PfxUInt32 iB) {
		//nrem++;
		PfxInt32 ret = arg->contactManager->removeContactHolder(contactId);
		if (ret != SCE_PFX_OK) { return ret; }

		// todo : wake up sleeping bodies
		if ((arg->dontWakeUp) == 0) {
			PfxRigidState &stateA = arg->stage->shared.states[iA];
			PfxRigidState &stateB = arg->stage->shared.states[iB];
			if (stateA.isAsleep() && stateB.getMotionType() != kPfxMotionTypeFixed) {
				stateA.wakeup();
			}
			if (stateB.isAsleep() && stateA.getMotionType() != kPfxMotionTypeFixed) {
				stateB.wakeup();
			}
		}
		return ret;
	};

	auto keepPair = [&](PfxPairContainer::PairNode &pair) {
		//nkeep++;
		PfxContactHolder &contactHolder = arg->contactManager->getContactHolder(pair.data);
		PfxRigidState &stateA = arg->stage->shared.states[pair.rigidbodyIdA];
		PfxRigidState &stateB = arg->stage->shared.states[pair.rigidbodyIdB];
		// update the latest information
		pair.motionMaskA = stateA.getMotionMask();
		pair.motionMaskB = stateB.getMotionMask();
		pair.solverQuality = SCE_PFX_MAX(stateA.getSolverQuality(), stateB.getSolverQuality());
		//SCE_PFX_PRINTF(" - refresh contact RB %d %d contactId %d \n", contact.getRigidBodyIdA(), contact.getRigidBodyIdB(), contactId);
		SCE_PFX_ASSERT(pair.rigidbodyIdA == contactHolder.getRigidBodyIdA());
		SCE_PFX_ASSERT(pair.rigidbodyIdB == contactHolder.getRigidBodyIdB());
		if (stateA.getProxyShiftFlag() || stateB.getProxyShiftFlag() || contactHolder.getDuration() < 60 ||
			stateA.getMotionType() == kPfxMotionTypeTrigger || stateB.getMotionType() == kPfxMotionTypeTrigger) {
			PfxLargePosition posA = stateA.getLargePosition();
			PfxLargePosition posB = stateB.getLargePosition();
			posB.changeSegment(posA.segment);
			contactHolder.refresh(
				posA.offset, stateA.getOrientation(),
				posB.offset, stateB.getOrientation());
		}
		else {
			contactHolder.refresh();
		}
	};

	for (PfxUInt32 i = start; i < end; i++) {
		PfxPairContainer::PairNode pair = arg->pairContainer->getPairNode(i);
		if (pair.poolId == (PfxUInt32)-1) continue;
		if (pair.state == 0) { // A pair wasn't found. It needs to be checked.
			if (!(arg->stage->shared.states[pair.rigidbodyIdA].getProxyShiftFlag() || arg->stage->shared.states[pair.rigidbodyIdB].getProxyShiftFlag())) {
				// A pair wasn't detected, because both rigid bodies kept static
				arg->pairContainer->getPairNode(i).state = 2;
				keepPair(arg->pairContainer->getPairNode(i));
			}
			else {
				PfxPairContainer::eResult result = arg->pairContainer->tryRemovePair(pair.rigidbodyIdA, pair.rigidbodyIdB);
				if (result == PfxPairContainer::kSuccess) { // removed
					PfxInt32 ret = removePair(pair.data, pair.rigidbodyIdA, pair.rigidbodyIdB);
					if (ret != SCE_PFX_OK) {
						context->appendError(PfxContext::kStageRefinePairs, arg->dispatchId, ret);
						return;
					}
				}
				else if (result == PfxPairContainer::kFailedToLockPair) {
					work[0][numWork[0]++] = i;
				}
				else if (result == PfxPairContainer::kFailedToRemovePair) {
					context->appendError(PfxContext::kStageRefinePairs, arg->dispatchId, SCE_PFX_ERR_OUT_OF_RANGE);
				}
			}
		}
		else if (pair.state == 1) {
			PfxUInt32 contactId;
			PfxInt32 ret = newPair(contactId, pair.rigidbodyIdA, pair.rigidbodyIdB);
			if (ret != SCE_PFX_OK) {
				context->appendError(PfxContext::kStageRefinePairs, arg->dispatchId, ret);
				return;
			}
#ifdef SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS
			updateContactManifolds(arg->contactManager, contactId,
				getPhysicsBuffer()->states[pair.rigidbodyIdA], getPhysicsBuffer()->collidables[pair.rigidbodyIdA],
				getPhysicsBuffer()->states[pair.rigidbodyIdB], getPhysicsBuffer()->collidables[pair.rigidbodyIdB]);
#endif
			arg->pairContainer->setPairData(pair.poolId, contactId);
			//SCE_PFX_PRINTF("new pair %d %d contactId %d\n", pair.rigidbodyIdA, pair.rigidbodyIdB, contactId);
		}
		else if (pair.state == 2) {
			PfxUInt32 contactId;
			if (arg->pairContainer->getPairData(pair.poolId, contactId) == PfxPairContainer::kSuccess) {
				keepPair(arg->pairContainer->getPairNode(i));
#ifdef SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS
				updateContactManifolds(arg->contactManager, contactId,
					getPhysicsBuffer()->states[pair.rigidbodyIdA], getPhysicsBuffer()->collidables[pair.rigidbodyIdA],
					getPhysicsBuffer()->states[pair.rigidbodyIdB], getPhysicsBuffer()->collidables[pair.rigidbodyIdB]);
#endif
			}
			//SCE_PFX_PRINTF("keep pair %d %d contactId %d\n", pair.rigidbodyIdA, pair.rigidbodyIdB, contactId);
		}
	}

	while (numWork[swap] > 0) {
		for (PfxUInt32 i = 0; i < numWork[swap]; i++) {
			PfxPairContainer::PairNode pair = arg->pairContainer->getPairNode(work[swap][i]);
			if (pair.poolId == (PfxUInt32)-1) continue;
			PfxPairContainer::eResult result = arg->pairContainer->tryRemovePair(pair.rigidbodyIdA, pair.rigidbodyIdB);
			if (result == PfxPairContainer::kSuccess) { // removed
				PfxInt32 ret = removePair(pair.data, pair.rigidbodyIdA, pair.rigidbodyIdB);
				if (ret != SCE_PFX_OK) {
					context->appendError(PfxContext::kStageRefinePairs, arg->dispatchId, ret);
					return;
				}
			}
			else if (result == PfxPairContainer::kFailedToLockPair) {
				work[1 - swap][numWork[1 - swap]++] = work[swap][i];
			}
			else if (result == PfxPairContainer::kFailedToRemovePair) {
				context->appendError(PfxContext::kStageRefinePairs, arg->dispatchId, SCE_PFX_ERR_OUT_OF_RANGE);
			}
		}

		numWork[swap] = 0;
		swap = 1 - swap;
	}
}

PfxInt32 PfxBroadphaseStage::refinePairs(PfxContactComplex &contactComplex, const PfxBroadphasePair *outPairs, const PfxInt32 *numOutPairs, PfxBool dontWakeUp)
{
	PfxContactManager *contactManager = (PfxContactManager*)&contactComplex.contactManager;
	PfxPairContainer *pairContainer = (PfxPairContainer*)&contactComplex.pairContainer;

	int numArgs = 3;
	RefinePairsJobArg *jobArgs = getRigidBodyContext()->allocate<RefinePairsJobArg>(numArgs);
	if (!jobArgs) { return SCE_PFX_ERR_OUT_OF_BUFFER; }

	PfxInt32 ret = SCE_OK;
	PfxInt32 dispatchId = getRigidBodyContext()->getDispatchId();

	int njob = 0;

	// Reset pair state (args : 1)
	{
		PfxInt32 numJobs_ = getJobSystem()->getNumWorkerThreads() * 2;
		RefinePairsJobArg &arg = jobArgs[njob++];
		arg.stage = this;
		arg.dispatchId = dispatchId;
		arg.contactManager = contactManager;
		arg.pairContainer = pairContainer;
		arg.num = numJobs_;

		ret = getJobSystem()->dispatch(job_resetPairState, &arg, numJobs_, "reset");
		if (ret != SCE_OK) return ret;
	}

	ret = getJobSystem()->barrier();
	if (ret != SCE_OK) return ret;

	// Construct pair graph (args : 1)
	{
		PfxUInt32 numJobs_ = getJobSystem()->getNumWorkerThreads() * 4;
		RefinePairsJobArg &arg = jobArgs[njob++];
		arg.stage = this;
		arg.dispatchId = dispatchId;
		arg.contactManager = contactManager;
		arg.pairContainer = pairContainer;
		arg.num = numJobs_;
		arg.outPairs = outPairs;
		arg.numOutPairs = numOutPairs;

		ret = getJobSystem()->dispatch(job_AddPairs, &arg, numJobs_, "add");
		if (ret != SCE_OK) return ret;
	}

	ret = getJobSystem()->barrier();
	if (ret != SCE_OK) return ret;

	// Remove unused pairs (args : 1)
	{
		PfxUInt32 numJobs_ = getJobSystem()->getNumWorkerThreads() * 4;
		RefinePairsJobArg &arg = jobArgs[njob++];
		arg.stage = this;
		arg.dispatchId = dispatchId;
		arg.contactManager = contactManager;
		arg.pairContainer = pairContainer;
		arg.num = numJobs_;
		arg.dontWakeUp = dontWakeUp;

		ret = getJobSystem()->dispatch(job_removeUnusedPairs, &arg, numJobs_, "remove");
		if (ret != SCE_OK) return ret;
	}

	ret = getJobSystem()->barrier();
	if (ret != SCE_OK) return ret;

	//getPairContainer()->verify();
	//getPairContainer()->print();
	//contactManager->print();
	//SCE_PFX_PRINTF("------ end of job queue ------\n");

	return SCE_PFX_OK;
}

PfxInt32 PfxBroadphaseStage::preparePipelineForUpdateBvh(const PfxLargePosition &worldMin, const PfxLargePosition &worldMax, PfxFloat timeStep,
	PfxRigidState *states, PfxCollidable *collidables, PfxUInt32 numRigidBodies)
{
	if (timeStep <= 0.0f || !states || !collidables) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	shared.timeStep = timeStep;
	shared.worldMin = worldMin;
	shared.worldMax = worldMax;
	shared.states = states;
	shared.collidables = collidables;
	shared.numRigidBodies = numRigidBodies;

	swapBodies = getRigidBodyContext()->allocate<PfxUInt32>(numRigidBodies);
	if (!swapBodies) return SCE_PFX_ERR_OUT_OF_BUFFER;

	return SCE_PFX_OK;
}

PfxInt32 PfxBroadphaseStage::preparePipelineForFindPairs(PfxUInt32 maxContacts)
{
	shared.maxPairs = maxContacts;

	tmpPairs[0] = getRigidBodyContext()->allocate<PfxUInt16>(maxContacts * 2);
	if (!tmpPairs[0]) return SCE_PFX_ERR_OUT_OF_BUFFER;

	tmpPairs[1] = getRigidBodyContext()->allocate<PfxUInt16>(maxContacts * 2);
	if (!tmpPairs[1]) return SCE_PFX_ERR_OUT_OF_BUFFER;

	tmpPairs[2] = getRigidBodyContext()->allocate<PfxUInt16>(maxContacts * 2);
	if (!tmpPairs[2]) return SCE_PFX_ERR_OUT_OF_BUFFER;

	return SCE_PFX_OK;
}

PfxInt32 PfxBroadphaseStage::preparePipelineForRefinePairs(PfxRigidState *states, PfxUInt32 maxContacts)
{
	if (!states) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	shared.states = states;
	shared.maxPairs = maxContacts;

	workBuff1 = getRigidBodyContext()->allocate<PfxUInt32>(maxContacts);
	if (!workBuff1) return SCE_PFX_ERR_OUT_OF_BUFFER;

	workBuff2 = getRigidBodyContext()->allocate<PfxUInt32>(maxContacts);
	if (!workBuff2) return SCE_PFX_ERR_OUT_OF_BUFFER;

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
