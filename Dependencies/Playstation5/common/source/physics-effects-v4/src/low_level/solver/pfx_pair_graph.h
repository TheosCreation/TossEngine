/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_PAIR_GRAPH_H
#define _SCE_PFX_PAIR_GRAPH_H

namespace sce {
namespace pfxv4 {

// Pair Batch
struct PfxPairBatch {
	PfxUInt8 phase;
	PfxUInt8 batch;
	SCE_PFX_PADDING2
	PfxUInt32 startPairId;
	PfxUInt32 numPairs;
};

struct PfxBatchInfo {
	PfxPairBatch *pairBatches;
	PfxUInt32 *pairIdBuffer;
	PfxUInt32 numValidPairs;
	PfxUInt32 maxBatches;
	PfxUInt32 maxPhases;
};

struct PfxPairSimpleData
{
	PfxUInt32 rbA;
	PfxUInt32 rbB;
	PfxUInt32 pairId;
};

struct PfxSplitPairsArg {
	void *workBuff;
	PfxBool makeConstraintDepth;
	PfxBool makeContactDepth;
	PfxBool makeJointDepth;
	PfxUInt32 workBytes;
	PfxUInt8 *constraintDepth;
	PfxBatchInfo *batchInfo;
	PfxConstraintPair *pairs;
	PfxUInt32 numPairs;
	PfxPairSimpleData *pairDatem;
	PfxUInt32 numPairDatem;
	PfxBool *isRigidBodyMovable;
	PfxBool *isRigidBodyJointRoot;
	PfxBool enableSortOfPairs;
	PfxUInt32 maxPairsPerBatch;
	PfxRigidState *states;
	PfxUInt32 numRigidBodies;
	PfxUInt32 numWorkThreads;
	PfxInt32 result;
};

void gatherContactPairsToSingleLevel(PfxUInt32 &numPairsLevel, PfxPairSimpleData pairDatem[], PfxConstraintPair pairs[], PfxUInt32 numPairs);
void gatherJointPairsToSingleLevel(PfxUInt32 &numPairsLevel, PfxPairSimpleData pairDatem[], PfxConstraintPair pairs[], PfxJoint joints[], PfxUInt32 numPairs);
void gatherContactPairsToEachLevel( PfxUInt32 numPairsLevel[], PfxPairSimpleData *pairDatem[], const PfxConstraintPair pairs[], const PfxUInt32 numPairs, const PfxBool *isRigidBodyMovable );
void gatherJointPairsToEachLevel( PfxUInt32 numPairsLevel[], PfxPairSimpleData *pairDatem[], PfxConstraintPair pairs[], const PfxJoint joints[], const PfxUInt32 numPairs, const PfxBool *isRigidBodyMovable );

void preProcessOfSplitContactPairs(PfxSplitPairsArg &arg);
void preProcessOfSplitJointPairs(PfxSplitPairsArg &arg);
PfxInt32 postProcessOfSplitPairs(PfxSplitPairsArg &arg);
void sortPairsByDepth(
	void *workBuff, PfxUInt32 workBytes,
	const PfxPairSimpleData pairDatem[], PfxUInt32 numPairDatem,
	const PfxUInt8 rigidBodyDepth[], PfxPairSimpleData outSortedPairDatem[]);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_PAIR_GRAPH_H

