/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../base_level/solver/pfx_check_solver.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint.h"
#include "pfx_pair_graph.h"

namespace sce {
namespace pfxv4 {

struct SimpleMemoryStock
{
	void *m_buffer;
	PfxUInt64 m_bufferSize;
	PfxUInt64 m_usedBytes;

	PfxUInt64 m_usedByteStack[64];
	PfxUInt32 m_stackSize;

	inline void initialize(void *buffer, PfxUInt64 bufferSize){
		m_buffer = buffer;
		m_bufferSize = bufferSize;

		m_usedBytes = m_stackSize = 0lu;
	}

	template <typename T>
	T *alloc(PfxUInt32 arraySize){
		PfxUInt64 requiredSize = sizeof(T) * arraySize;
//		SCE_PFX_PRINTF("Available: %8d, stack %8d/%8d, allocate %6d = %4d * %4d\n",	m_bufferSize - m_usedBytes, m_usedBytes + requiredSize, m_bufferSize, requiredSize, sizeof(T), arraySize);
		
		if (m_usedBytes + requiredSize > m_bufferSize){
			SCE_PFX_ASSERT_MSG(false, "Memory not enough!!");
			return NULL;
		}

		T *ptr = (T*)(((char*)m_buffer) + m_usedBytes);
		m_usedByteStack[m_stackSize++] = m_usedBytes;
		m_usedBytes += requiredSize;
		return ptr;
	}

	void free(void *ptr){
//		SCE_PFX_PRINTF("Free, available %8d, stack %8d -> %8d/%8d\n", m_bufferSize - m_usedBytes, m_usedBytes, m_usedByteStack[m_stackSize - 1]);
		if (m_stackSize > 0)
			m_usedBytes = m_usedByteStack[--m_stackSize];

		SCE_PFX_ASSERT_MSG(ptr == (((char*)m_buffer) + m_usedBytes), "Warning!! Releasing wrong memory!!\n");
	}
};

#define SCE_PFX_PARALLEL_NUM_CONTACTS	100
#define SCE_PFX_PARALLEL_NUM_JOINTS		100

///////////////////////////////////////////////////////////////////////////////
// Split pairs

void printPairBatches(const PfxPairBatch *pairBatches,PfxUInt32 *pairIdBuffer,PfxUInt32 maxBatches,const PfxConstraintPair *pairs,PfxUInt32 numPairs,bool details=false)
{
	SCE_PFX_PRINTF("***** pair batches *****\n");
	for (PfxUInt32 b = 0; b < maxBatches; b++) {
		const PfxPairBatch *currBatch = pairBatches + b;
		SCE_PFX_PRINTF("phase %u batch %u pairs %u num %u\n",
			currBatch->phase,currBatch->batch,currBatch->startPairId,currBatch->numPairs);
		if(details) {
			for (PfxUInt32 i = 0; i<currBatch->numPairs; i++) {
				PfxUInt32 pairId = pairIdBuffer[currBatch->startPairId + i];
				const PfxConstraintPair &pair = pairs[pairId];
				PfxUInt32 rbA = pfxGetObjectIdA(pair);
				PfxUInt32 rbB = pfxGetObjectIdB(pair);
				(void)rbA;
				(void)rbB;
				SCE_PFX_PRINTF("pairId %8u rbA %8u rbB %8u\n",pairId,rbA,rbB);
			}
		}
	}
}

// New
void gatherContactPairsToSingleLevel(PfxUInt32 &numPairsLevel, PfxPairSimpleData pairDatem[], PfxConstraintPair pairs[], PfxUInt32 numPairs)
{
	numPairsLevel = 0u;
	for (PfxUInt32 i = 0; i < numPairs; ++i){
		PfxConstraintPair &pair = pairs[i];
		if (!pfxCheckSolver(pair)) {
			continue;
		}

		pairDatem[numPairsLevel].pairId = i;
		pairDatem[numPairsLevel].rbA = pfxGetObjectIdA(pair);
		pairDatem[numPairsLevel].rbB = pfxGetObjectIdB(pair);
		++numPairsLevel;
	}
}

void gatherJointPairsToSingleLevel(PfxUInt32 &numPairsLevel, PfxPairSimpleData pairDatem[], PfxConstraintPair pairs[], PfxJoint joints[], PfxUInt32 numPairs)
{
	numPairsLevel = 0u;
	for (PfxUInt32 i = 0; i < numPairs; i++) {
		PfxConstraintPair &pair = pairs[i];
		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);
		PfxJoint &joint = joints[pfxGetConstraintId(pair)];

		if (!pfxCheckSolver(pair)) {
			continue;
		}

		if (iA != iB) {
			pfxSetActive(pair, joint.m_active > 0);
		}
		else {
			pfxSetActive(pair, false);
		}

		if (!pfxGetActive(pair)){
			continue;
		}

		pairDatem[numPairsLevel].pairId = i;
		pairDatem[numPairsLevel].rbA = iA;
		pairDatem[numPairsLevel].rbB = iB;
		++numPairsLevel;
	}
}

// New
void gatherContactPairsToEachLevel(PfxUInt32 numPairsLevel[], PfxPairSimpleData *pairDatem[], const PfxConstraintPair pairs[], const PfxUInt32 numPairs,const PfxBool *isRigidBodyMovable )
{
	numPairsLevel[0] = 0;
	numPairsLevel[1] = 0;
	numPairsLevel[2] = 0;
	for (PfxUInt32 i = 0; i<numPairs; i++) {
		const PfxConstraintPair &pair = pairs[i];
		if (!pfxCheckSolver(pair)) {
			continue;
		}

		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);

		if( !isRigidBodyMovable[iA] && !isRigidBodyMovable[iB] ) {
			continue;
		}

		int q = pfxGetSolverQuality(pair);
		if (q == 0) {
			pairDatem[2][numPairsLevel[2]].pairId = i;
			pairDatem[2][numPairsLevel[2]].rbA = iA;
			pairDatem[2][numPairsLevel[2]].rbB = iB;
			++numPairsLevel[2];
		}
		else if (q == 1) {
			pairDatem[1][numPairsLevel[1]].pairId = pairDatem[2][numPairsLevel[2]].pairId = i;
			pairDatem[1][numPairsLevel[1]].rbA = pairDatem[2][numPairsLevel[2]].rbA = iA;
			pairDatem[1][numPairsLevel[1]].rbB = pairDatem[2][numPairsLevel[2]].rbB = iB;

			++numPairsLevel[1];
			++numPairsLevel[2];
		}
		else if (q == 2) {
			pairDatem[0][numPairsLevel[0]].pairId = pairDatem[1][numPairsLevel[1]].pairId = pairDatem[2][numPairsLevel[2]].pairId = i;
			pairDatem[0][numPairsLevel[0]].rbA = pairDatem[1][numPairsLevel[1]].rbA = pairDatem[2][numPairsLevel[2]].rbA = iA;
			pairDatem[0][numPairsLevel[0]].rbB = pairDatem[1][numPairsLevel[1]].rbB = pairDatem[2][numPairsLevel[2]].rbB = iB;
			++numPairsLevel[0];
			++numPairsLevel[1];
			++numPairsLevel[2];
		}
	}
}

void gatherJointPairsToEachLevel(PfxUInt32 numPairsLevel[], PfxPairSimpleData *pairDatem[], PfxConstraintPair pairs[], const PfxJoint joints[], const PfxUInt32 numPairs, const PfxBool *isRigidBodyMovable )
{
	numPairsLevel[0] = 0;
	numPairsLevel[1] = 0;
	numPairsLevel[2] = 0;
	for (PfxUInt32 i = 0; i < numPairs; i++) {
		PfxConstraintPair &pair = pairs[i];
		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);
		const PfxJoint &joint = joints[pfxGetConstraintId(pair)];

		if (!pfxCheckSolver(pair)) {
			continue;
		}

		if( !isRigidBodyMovable[ iA ] && !isRigidBodyMovable[ iB ] ) {
			continue;
		}

		if (iA != iB) {
			pfxSetActive(pair, joint.m_active > 0);
		}
		else {
			pfxSetActive(pair, false);
		}

		if (!pfxGetActive(pair)) {
			continue;
		}

		int q = pfxGetSolverQuality(pair);
		if (q == 0) {
			pairDatem[2][numPairsLevel[2]].pairId = i;
			pairDatem[2][numPairsLevel[2]].rbA = iA;
			pairDatem[2][numPairsLevel[2]].rbB = iB;
			++numPairsLevel[2];
		}
		else if (q == 1) {
			pairDatem[1][numPairsLevel[1]].pairId = pairDatem[2][numPairsLevel[2]].pairId = i;
			pairDatem[1][numPairsLevel[1]].rbA = pairDatem[2][numPairsLevel[2]].rbA = iA;
			pairDatem[1][numPairsLevel[1]].rbB = pairDatem[2][numPairsLevel[2]].rbB = iB;
			
			++numPairsLevel[1];
			++numPairsLevel[2];
		}
		else if (q == 2) {
			pairDatem[0][numPairsLevel[0]].pairId = pairDatem[1][numPairsLevel[1]].pairId = pairDatem[2][numPairsLevel[2]].pairId = i;
			pairDatem[0][numPairsLevel[0]].rbA = pairDatem[1][numPairsLevel[1]].rbA = pairDatem[2][numPairsLevel[2]].rbA = iA;
			pairDatem[0][numPairsLevel[0]].rbB = pairDatem[1][numPairsLevel[1]].rbB = pairDatem[2][numPairsLevel[2]].rbB = iB;
			++numPairsLevel[0];
			++numPairsLevel[1];
			++numPairsLevel[2];
		}

	}
}

struct PfxRigidBodyLinkedListForBatchNode
{
	PfxInt32 head;
	PfxInt32 tail;
	PfxInt32 next;
	PfxInt32 prev;
};

static void countContactDepthInternal(SimpleMemoryStock &memStock, PfxUInt8 constraintDepth[], PfxPairSimpleData pairDatem[], PfxBool isRigidBodyMovable[], PfxUInt32 numValidPairs, PfxUInt32 numRigidBodies)
{
	PfxUInt32 *startPosRigidBodyRelativeToRigidBody = memStock.alloc<PfxUInt32>(numRigidBodies + 2);
	PfxUInt32 *endPosRigidBodyRelativeToRigidBody = startPosRigidBodyRelativeToRigidBody + 1;
	PfxUInt32 *accumPosRigidBodyRelativeToRigidBody = endPosRigidBodyRelativeToRigidBody + 1;
	PfxUInt32 *adjRigidBodyIds = memStock.alloc<PfxUInt32>(numValidPairs * 2);

	// Initialize the numbers
	for (PfxUInt32 i = 0; i < numRigidBodies; ++i)	accumPosRigidBodyRelativeToRigidBody[i] = 0u;

	// Count for # of pairs relative to each rigid body
	for (PfxUInt32 i = 0; i < numValidPairs; ++i){
		++accumPosRigidBodyRelativeToRigidBody[pairDatem[i].rbA];
		++accumPosRigidBodyRelativeToRigidBody[pairDatem[i].rbB];
	}

	// Calculate the starting position
	startPosRigidBodyRelativeToRigidBody[0] = endPosRigidBodyRelativeToRigidBody[0] = 0u;
	for (PfxUInt32 i = 0; i < numRigidBodies; ++i)	accumPosRigidBodyRelativeToRigidBody[i] += endPosRigidBodyRelativeToRigidBody[i];

	// Save the pairs
	for (PfxUInt32 i = 0; i < numValidPairs; ++i){
		PfxPairSimpleData *pairData = pairDatem + i;
		adjRigidBodyIds[endPosRigidBodyRelativeToRigidBody[pairData->rbA]++] = pairData->rbB;
		adjRigidBodyIds[endPosRigidBodyRelativeToRigidBody[pairData->rbB]++] = pairData->rbA;
	}

	PfxUInt32 *rigidBodyIdsList1 = memStock.alloc<PfxUInt32>(numValidPairs);
	PfxUInt32 *rigidBodyIdsList2 = memStock.alloc<PfxUInt32>(numValidPairs);
	PfxUInt32 *rigidBodyIdsInCurrIteration = rigidBodyIdsList1;
	PfxUInt32 *rigidBodyIdsInNextIteration = rigidBodyIdsList2;
	PfxUInt32 numRigidBodyIdsInCurrIteration = 0u;
	PfxUInt32 numRigidBodyIdsInNextIteration = 0u;

	// Initialize the depth and list
	for (PfxUInt32 i = 0; i < numRigidBodies; ++i){
		constraintDepth[i] = (PfxUInt8)0xFF;
		if (!isRigidBodyMovable[i]){
			rigidBodyIdsInCurrIteration[numRigidBodyIdsInCurrIteration++] = i;
		}
	}

	// Iterative breadth-first-search
	PfxUInt8 depth = (PfxUInt8)0u;
	PfxUInt8 nextDepth = (PfxUInt8)1u;
	while (numRigidBodyIdsInCurrIteration > 0 && depth < (PfxUInt8)0xFF){

		// Loop for all rigid bodies in current iteration
		for (PfxUInt32 i = 0; i < numRigidBodyIdsInCurrIteration; ++i){
			PfxUInt32 rbId = rigidBodyIdsInCurrIteration[i];
			constraintDepth[rbId] = depth;

			// Gather its neighbors which have not been visited for next iteration
			for (PfxUInt32 j = startPosRigidBodyRelativeToRigidBody[rbId]; j < endPosRigidBodyRelativeToRigidBody[rbId]; ++j){
				PfxUInt32 adjRbId = adjRigidBodyIds[j];

				if (constraintDepth[adjRbId] > nextDepth){
					constraintDepth[adjRbId] = nextDepth;
					rigidBodyIdsInNextIteration[numRigidBodyIdsInNextIteration++] = adjRbId;
				}
			}
		}

		// Swap the list pointers
		PfxUInt32 *tempListPtr = rigidBodyIdsInCurrIteration;
		rigidBodyIdsInCurrIteration = rigidBodyIdsInNextIteration;
		rigidBodyIdsInNextIteration = tempListPtr;

		numRigidBodyIdsInCurrIteration = numRigidBodyIdsInNextIteration;
		numRigidBodyIdsInNextIteration = 0u;

		// Increase the depth
		depth = nextDepth;
		++nextDepth;
	}

	// Release the lists
	memStock.free(rigidBodyIdsList2);
	memStock.free(rigidBodyIdsList1);

	memStock.free(adjRigidBodyIds);
	memStock.free(startPosRigidBodyRelativeToRigidBody);
}

void preProcessOfSplitContactPairs(PfxSplitPairsArg &arg)
{
	// Inititalize
	SimpleMemoryStock memStock;
	memStock.initialize(arg.workBuff, arg.workBytes);

	//----------------------------------------------------------------------------
	// STEP 2: count depth
	//----------------------------------------------------------------------------
	
	countContactDepthInternal(memStock, arg.constraintDepth, arg.pairDatem, arg.isRigidBodyMovable, arg.numPairDatem, arg.numRigidBodies);
}

void preProcessOfSplitJointPairs(PfxSplitPairsArg &arg)
{
	// Inititalize
	SimpleMemoryStock memStock;
	memStock.initialize(arg.workBuff, arg.workBytes);

	//----------------------------------------------------------------------------
	//  STEP 1: save the pairs' data 
	//----------------------------------------------------------------------------

	// Find the fixed rigid bodies
	for (PfxUInt32 i = 0; i < arg.numRigidBodies; ++i){
		arg.isRigidBodyJointRoot[i] = !arg.states[i].isJointRoot();
	}

	//----------------------------------------------------------------------------
	// STEP 2: count depth
	//----------------------------------------------------------------------------

	countContactDepthInternal(memStock, arg.constraintDepth, arg.pairDatem, arg.isRigidBodyJointRoot, arg.numPairDatem, arg.numRigidBodies);
}

PfxInt32 postProcessOfSplitPairs(PfxSplitPairsArg &arg)
{
	// Initialize
	SimpleMemoryStock memStock;
	memStock.initialize(arg.workBuff, arg.workBytes);

	if (arg.makeContactDepth)		preProcessOfSplitContactPairs(arg);
	else if (arg.makeJointDepth)	preProcessOfSplitJointPairs(arg);

	PfxUInt32 numValidPairs = arg.numPairDatem;

	PfxBatchInfo *batchInfo = arg.batchInfo;
	batchInfo->numValidPairs = numValidPairs;

	PfxPairSimpleData *pairDatem = arg.pairDatem;
	PfxBool *isRigidBodyMovable = arg.isRigidBodyMovable;

	//----------------------------------------------------------------------------
	// STEP 3: build/refine the batch
	//----------------------------------------------------------------------------

	// By new method
	//	{
	// Variables for "merge rb"
	PfxUInt32 *numPairsRequiredToSolveRigidBody = memStock.alloc<PfxUInt32>(arg.numRigidBodies);
	PfxUInt32 *unappliedPairs1 = memStock.alloc<PfxUInt32>(numValidPairs);
	PfxUInt32 *unappliedPairs2 = memStock.alloc<PfxUInt32>(numValidPairs);
	PfxUInt32 *rigidBodyIdToWhichPairBelongs = memStock.alloc<PfxUInt32>(arg.numPairs);
	PfxUInt32 *startPosPairsBelongToRigidBody = memStock.alloc<PfxUInt32>(arg.numRigidBodies + 2);
	PfxUInt32 *endPosPairsBelongToRigidBody = startPosPairsBelongToRigidBody + 1;
	PfxUInt32 *accumPosPairsBelongToRigidBody = endPosPairsBelongToRigidBody + 1;
	PfxRigidBodyLinkedListForBatchNode *rbBatchNodes = memStock.alloc<PfxRigidBodyLinkedListForBatchNode>(arg.numRigidBodies);
	PfxUInt32 *currUnappliedPairs = unappliedPairs1;
	PfxUInt32 *nextUnappliedPairs = unappliedPairs2;
	PfxUInt32 numCurrUnappliedPairs = 0u;
	PfxUInt32 numNextUnappliedPairs = 0u;

	// Variables for "gather pairs"
	PfxUInt32 *pairIdsBelongToRigidBody = memStock.alloc<PfxUInt32>(numValidPairs);

	// Variables for "gather heads"
	PfxInt32 *heads = memStock.alloc<PfxInt32>(arg.numRigidBodies);

	// Variables for "sort lists"
	PfxUInt32 *accumPosForLists = memStock.alloc<PfxUInt32>(SCE_PFX_MAX(numValidPairs + 2, arg.maxPairsPerBatch + 2));
	PfxUInt32 *endPosForLists = accumPosForLists + 1;
	PfxUInt32 *startPosForLists = endPosForLists + 1;
	PfxInt32 *sortedListHeads = memStock.alloc<PfxInt32>(arg.numRigidBodies);

	// Variables for "save to batch"
	PfxUInt32 *duplicateStartPosForLists = memStock.alloc<PfxUInt32>(SCE_PFX_MAX(numValidPairs, arg.maxPairsPerBatch));

	// Initialize the pair list
	numCurrUnappliedPairs = numValidPairs;

	//----------------------------------------------------------------------------
	// STEP 3-a: shuffle the pairs
	//----------------------------------------------------------------------------

	PfxUInt32 *depthOfPairs, *sortedPairs = nullptr;

	
	if (arg.enableSortOfPairs)
	{
		// Variables for sorting the unapplied pairs, to which will be referred later
		depthOfPairs = memStock.alloc<PfxUInt32>(numValidPairs);
		sortedPairs = memStock.alloc<PfxUInt32>(numValidPairs);

		// Variables for sorting the unapplied pairs, temporarily use
		PfxUInt32 *startPosForUnappliedPairs = memStock.alloc<PfxUInt32>(515);
		PfxUInt32 *endPosForUnappliedPairs = startPosForUnappliedPairs + 1;
		PfxUInt32 *accumPosForUnappliedPairs = endPosForUnappliedPairs + 1;

		PfxUInt32 *startPosSortedPairs = memStock.alloc<PfxUInt32>(515);
		PfxUInt32 *endPosSortedPairs = startPosSortedPairs + 1;
		PfxUInt32 *accumPosSortedPairs = endPosSortedPairs + 1;

		// Initialize
		for (PfxUInt32 i = 0; i < 515; ++i)	startPosSortedPairs[i] = startPosForUnappliedPairs[i] = 0u;

		// Sum the # of pairs according to the depth
		for (PfxUInt32 i = 0; i < numValidPairs; ++i){
			PfxPairSimpleData *pairData = pairDatem + i;
			PfxUInt32 depthA = arg.constraintDepth[pairData->rbA];
			PfxUInt32 depthB = arg.constraintDepth[pairData->rbB];

			depthOfPairs[i] = depthA + depthB;
			++accumPosForUnappliedPairs[depthOfPairs[i]];
			++accumPosSortedPairs[depthOfPairs[i]];
		}

		// Accumlate
		PfxUInt32 maxNumPairsInOneDepth = 0u;
		for (PfxUInt32 i = 0; i < 512; ++i)	maxNumPairsInOneDepth = SCE_PFX_MAX(maxNumPairsInOneDepth, accumPosForUnappliedPairs[i]);
		for (PfxUInt32 i = 0; i < 512; ++i)	accumPosSortedPairs[i] += endPosSortedPairs[i];

		// Assign the pairs
		for (PfxUInt32 i = 0; i < numValidPairs; ++i)		sortedPairs[endPosSortedPairs[depthOfPairs[i]]++] = i;

		// Sort the depth according to the # of pairs 
		PfxUInt32 *accumNumOfPairsInDepth = memStock.alloc<PfxUInt32>(maxNumPairsInOneDepth + 2);
		PfxUInt32 *endNumOfPairsInDepth = accumNumOfPairsInDepth + 1;
		//PfxUInt32 *startNumOfPairsInDepth = endNumOfPairsInDepth + 1;
		PfxUInt32 *insertIndex = memStock.alloc<PfxUInt32>(512);
		PfxUInt32 *numInsertedPairs = memStock.alloc<PfxUInt32>(512);

		for (PfxUInt32 i = 0; i < maxNumPairsInOneDepth + 2; ++i)	accumNumOfPairsInDepth[i] = 0u;

		for (PfxUInt32 i = 0; i < 512; ++i)		++accumNumOfPairsInDepth[accumPosForUnappliedPairs[i]];

		for (PfxInt32 i = maxNumPairsInOneDepth; i > 0; --i)	accumNumOfPairsInDepth[i] += endNumOfPairsInDepth[i];

		// Save the depth according to the # of pairs
		PfxUInt32 *sortedDepth = memStock.alloc<PfxUInt32>(512);
		for (PfxUInt32 i = 0; i < 512; ++i)		sortedDepth[endNumOfPairsInDepth[accumPosForUnappliedPairs[i]]++] = i;

		PfxUInt32 startIndex = 0u;
		for (PfxUInt32 i = 0; i < 512; ++i){
			PfxUInt32 currDepth = sortedDepth[i];

			numInsertedPairs[currDepth] = 0u;
			if (accumPosForUnappliedPairs[currDepth] > 0)	insertIndex[currDepth] = startIndex++;
		}

		for (PfxUInt32 i = 0; i < numValidPairs; ++i){
			PfxUInt32 pairDepth = depthOfPairs[i];

			currUnappliedPairs[insertIndex[pairDepth]] = i;
			insertIndex[pairDepth] += endNumOfPairsInDepth[++numInsertedPairs[pairDepth]];
		}

		// Insert the pairs according to the depth
		for (PfxUInt32 i = 0; i < 512; ++i)	accumPosForUnappliedPairs[i] += endPosForUnappliedPairs[i];

		for (PfxUInt32 i = 0; i < numValidPairs; ++i)
			currUnappliedPairs[endPosForUnappliedPairs[depthOfPairs[i]]++] = i;

		// Release the memory space
		memStock.free(sortedDepth);
		memStock.free(numInsertedPairs);
		memStock.free(insertIndex);
		memStock.free(accumNumOfPairsInDepth);
		memStock.free(startPosSortedPairs);
		memStock.free(startPosForUnappliedPairs);
	}
	else // if (arg.enableStabilization)
	{
		//----------------------------------------------------------------------------
		// STEP 3-b: if the depth is not built, then simply copy the pair ids
		//----------------------------------------------------------------------------

		for (PfxUInt32 i = 0; i < numValidPairs; ++i)				currUnappliedPairs[i] = i;
	} // if (arg.enableStabilization)

	for (PfxUInt32 i = 0; i < numValidPairs; ++i)				rigidBodyIdToWhichPairBelongs[pairDatem[i].pairId] = arg.numRigidBodies;

	//----------------------------------------------------------------------------
	// STEP 3-c: initialize the pairs contacting with fixed rigid bodies
	//----------------------------------------------------------------------------

	// Initialize the batchInfo
	batchInfo->maxBatches = 0u;
	batchInfo->maxPhases = 0u;
	PfxUInt32 numBatchInCurrPhase = 0u;
	PfxUInt32 *currPairIdBuffer = batchInfo->pairIdBuffer;

	// Initialize the numbers
	for (PfxUInt32 i = 0; i < arg.numRigidBodies; ++i)			accumPosPairsBelongToRigidBody[i] = numPairsRequiredToSolveRigidBody[i] = 0u;

	// Initialize the pairs and rigid bodies that directly contacting fixed rigid bodies
	for (PfxUInt32 i = 0; i < numValidPairs; ++i){
		PfxPairSimpleData * pairData = pairDatem + i;
		PfxUInt32 rbA = pairData->rbA;
		PfxUInt32 rbB = pairData->rbB;

		if (isRigidBodyMovable[rbA] && !isRigidBodyMovable[rbB]){
			++numPairsRequiredToSolveRigidBody[rbA];
			++accumPosPairsBelongToRigidBody[rbA];
			rigidBodyIdToWhichPairBelongs[pairData->pairId] = rbA;
		}
		else if (!isRigidBodyMovable[rbA] && isRigidBodyMovable[rbB]){
			++numPairsRequiredToSolveRigidBody[rbB];
			++accumPosPairsBelongToRigidBody[rbB];
			rigidBodyIdToWhichPairBelongs[pairData->pairId] = rbB;
		}
	}

	//----------------------------------------------------------------------------
	// STEP 3-d: start iteratively creation of batches
	//			 the process:
	//				 1. Gather the connecting pairs into groups
	//				 2. Sort the groups
	//				 3. First, put the groups with # of pairs = maxPairsPerBatch
	//				 4. Second, insert the groups with # of pairs from 
	//					maxPairsPerBatch - 1 to maxPairsPerBatch/2 
	//				 5. Third, insert the rest groups
	//----------------------------------------------------------------------------

	while (numCurrUnappliedPairs > 0)
	{
		// Re-calculate the maximum # of pairs in each batch for each phase
		PfxUInt32 maxPairsPerBatchInCurrPhase = SCE_PFX_CLAMP(numCurrUnappliedPairs / (arg.numWorkThreads * 4u), 8u, 128u);

		// Initialize the list
		for (PfxUInt32 i = 0; i < arg.numRigidBodies; ++i){
			rbBatchNodes[i].head = rbBatchNodes[i].tail = i;
			rbBatchNodes[i].prev = rbBatchNodes[i].next = -1;
		}

		//----------------------------------------------------------------------------
		// STEP 3-d-1: gather the connecting pairs into groups
		//----------------------------------------------------------------------------

		// Gather the pairs
		for (PfxUInt32 i = 0; i < numCurrUnappliedPairs; ++i){
			PfxPairSimpleData * pairData = pairDatem + currUnappliedPairs[i];
			PfxUInt32 rbA = pairData->rbA;
			PfxUInt32 rbB = pairData->rbB;

			if (!isRigidBodyMovable[rbA] || !isRigidBodyMovable[rbB])
				continue;

			PfxUInt32 headRbA = rbA;
			PfxUInt32 headRbB = rbB;
			PfxRigidBodyLinkedListForBatchNode *headRbBatchNodeA = rbBatchNodes + headRbA;
			PfxRigidBodyLinkedListForBatchNode *headRbBatchNodeB = rbBatchNodes + headRbB;

			// Move to the head node of the linked lists to which A and B belong
			while (headRbBatchNodeA->prev != -1){
				PfxRigidBodyLinkedListForBatchNode *tempRbBatchNode = headRbBatchNodeA;
				headRbA = headRbBatchNodeA->head;
				headRbBatchNodeA = rbBatchNodes + headRbA;
				tempRbBatchNode->head = headRbBatchNodeA->head;
			}

			while (headRbBatchNodeB->prev != -1){
				PfxRigidBodyLinkedListForBatchNode *tempRbBatchNode = headRbBatchNodeB;
				headRbB = headRbBatchNodeB->head;
				headRbBatchNodeB = rbBatchNodes + headRbB;
				tempRbBatchNode->head = headRbBatchNodeB->head;
			}

			if (headRbA != headRbB){

				// Attach B to A if the size of the merged linked list is less than the limitation
				PfxUInt32 tempTotalPairs = numPairsRequiredToSolveRigidBody[headRbA] + numPairsRequiredToSolveRigidBody[headRbB] + 1;
				if (tempTotalPairs <= maxPairsPerBatchInCurrPhase){
					rbBatchNodes[headRbBatchNodeA->tail].next = headRbB;
					headRbBatchNodeB->prev = headRbBatchNodeA->tail;

					headRbBatchNodeB->head = headRbA;
					headRbBatchNodeA->tail = headRbBatchNodeB->tail;

					numPairsRequiredToSolveRigidBody[headRbA] = tempTotalPairs;
				}

				// Leave this pair to next iteration
				else
				{
					// Leave this pair to next iterations
					nextUnappliedPairs[numNextUnappliedPairs++] = currUnappliedPairs[i];

					// Reset the rigid body to which current pair belongs
					rigidBodyIdToWhichPairBelongs[pairData->pairId] = arg.numRigidBodies;

					// Skip the assignment of this pair
					continue;
				}
			}

			// if (headRbA != headRbB): if rigid body A and B are already in the same linked list, than simply increase the number
			else{
				++numPairsRequiredToSolveRigidBody[headRbA];
			}

			// Assign this pair to rbA
			rigidBodyIdToWhichPairBelongs[pairData->pairId] = headRbA;
			++accumPosPairsBelongToRigidBody[headRbA];
		}

		//----------------------------------------------------------------------------
		// STEP 3-d-2: sort the gathered groups
		//----------------------------------------------------------------------------

		// Accumulate the position
		startPosPairsBelongToRigidBody[0] = endPosPairsBelongToRigidBody[0] = 0u;
		for (PfxUInt32 i = 0; i < arg.numRigidBodies; ++i)		accumPosPairsBelongToRigidBody[i] += endPosPairsBelongToRigidBody[i];

		// Assign to the position
		for (PfxUInt32 i = 0; i < numCurrUnappliedPairs; ++i){
			PfxPairSimpleData *pairData = pairDatem + currUnappliedPairs[i];
			PfxUInt32 rbId = rigidBodyIdToWhichPairBelongs[pairData->pairId];
			if (rbId < arg.numRigidBodies){
				pairIdsBelongToRigidBody[endPosPairsBelongToRigidBody[rbId]++] = pairData->pairId;
			}
		}

		PfxUInt32 numHeads = 0u;
		PfxUInt32 maxNumPairsInOneHead = maxPairsPerBatchInCurrPhase;
		for (PfxInt32 i = 0; i < arg.numRigidBodies; ++i)
			if (rbBatchNodes[i].prev == -1 && numPairsRequiredToSolveRigidBody[i] > 0){
				heads[numHeads++] = i;
				maxNumPairsInOneHead = SCE_PFX_MAX(maxNumPairsInOneHead, numPairsRequiredToSolveRigidBody[i]);
			}

		// Initialize the numbers
		for (PfxUInt32 i = 0; i <= maxNumPairsInOneHead; ++i)	accumPosForLists[i] = 0u;

		// Count the sizes
		for (PfxUInt32 i = 0; i < numHeads; ++i)				++accumPosForLists[numPairsRequiredToSolveRigidBody[heads[i]]];

		// Set the start position
		startPosForLists[maxNumPairsInOneHead] = endPosForLists[maxNumPairsInOneHead] = 0;
		for (PfxUInt32 i = maxNumPairsInOneHead; i > 0; --i)	accumPosForLists[i] += endPosForLists[i];

		// Assign the lists
		for (PfxUInt32 i = 0; i < numHeads; ++i)				sortedListHeads[endPosForLists[numPairsRequiredToSolveRigidBody[heads[i]]]++] = heads[i];

		//----------------------------------------------------------------------------
		// STEP 3-d-3: put the groups with # of pairs >= maxPairsPerBatch
		//----------------------------------------------------------------------------

		// First build those already larger than or equal to the max pairs per batch
		for (PfxUInt32 i = 0; i < endPosForLists[maxPairsPerBatchInCurrPhase]; ++i){

			PfxPairBatch *currPairBatch = batchInfo->pairBatches + batchInfo->maxBatches++;
			currPairBatch->batch = numBatchInCurrPhase++;
			currPairBatch->phase = batchInfo->maxPhases;
			currPairBatch->startPairId = currPairIdBuffer - batchInfo->pairIdBuffer;
			currPairBatch->numPairs = 0u;

			// Add the pairs into the buffer
			for (PfxInt32 rbId = sortedListHeads[i]; rbId >= 0; rbId = rbBatchNodes[rbId].next){
				for (PfxUInt32 j = startPosPairsBelongToRigidBody[rbId]; j < endPosPairsBelongToRigidBody[rbId]; ++j)
					currPairIdBuffer[currPairBatch->numPairs++] = pairIdsBelongToRigidBody[j];
			}

			// Move the id buffer ptr forward
			currPairIdBuffer += currPairBatch->numPairs;
		}

		// Copy the end of pairs
		for (PfxInt32 i = 0; i <= maxNumPairsInOneHead; ++i)
			duplicateStartPosForLists[i] = startPosForLists[i];

		//----------------------------------------------------------------------------
		// STEP 3-d-4: insert the groups with # of pairs from 
		//			   maxPairsPerBatch - 1 to maxPairsPerBatch/2 to 1
		//----------------------------------------------------------------------------

		// Start from the half of max pairs
		PfxUInt32 halfMaxPairsPerBatch = (maxPairsPerBatchInCurrPhase >> 1) + 1;

		// For heads that include more than half of max pairs limitation
		for (PfxUInt32 i = startPosForLists[maxPairsPerBatchInCurrPhase - 1]; i < endPosForLists[halfMaxPairsPerBatch]; ++i){

			PfxPairBatch *currPairBatch = batchInfo->pairBatches + batchInfo->maxBatches++;
			currPairBatch->batch = numBatchInCurrPhase++;
			currPairBatch->phase = batchInfo->maxPhases;
			currPairBatch->startPairId = currPairIdBuffer - batchInfo->pairIdBuffer;
			currPairBatch->numPairs = 0u;

			PfxInt32 headId = sortedListHeads[i];

			// Add the pairs into buffer
			for (PfxInt32 rbId = headId; rbId >= 0; rbId = rbBatchNodes[rbId].next){
				for (PfxUInt32 j = startPosPairsBelongToRigidBody[rbId]; j < endPosPairsBelongToRigidBody[rbId]; ++j){
					currPairIdBuffer[currPairBatch->numPairs++] = pairIdsBelongToRigidBody[j];
				}
			}

			// Start from (max # of pairs - numPairs) to full
			for (PfxInt32 remainingSpace = (PfxInt32)maxPairsPerBatchInCurrPhase - (PfxInt32)currPairBatch->numPairs; remainingSpace > 0 && duplicateStartPosForLists[remainingSpace] < numHeads;){

				if (duplicateStartPosForLists[remainingSpace] >= endPosForLists[remainingSpace]){
					--remainingSpace;
				}
				else
				{
					PfxInt32 mergeWithHead = sortedListHeads[duplicateStartPosForLists[remainingSpace]++];
					for (PfxInt32 rbId = mergeWithHead; rbId >= 0; rbId = rbBatchNodes[rbId].next){
						for (PfxUInt32 j = startPosPairsBelongToRigidBody[rbId]; j < endPosPairsBelongToRigidBody[rbId]; ++j)
							currPairIdBuffer[currPairBatch->numPairs++] = pairIdsBelongToRigidBody[j];
					}
					remainingSpace = SCE_PFX_MIN(remainingSpace, (PfxInt32)maxPairsPerBatchInCurrPhase - (PfxInt32)currPairBatch->numPairs);
				}
			}

			// Move the id buffer ptr forward
			currPairIdBuffer += currPairBatch->numPairs;
		}

		//----------------------------------------------------------------------------
		// STEP 3-d-5: insert the rest groups
		//----------------------------------------------------------------------------

		// Initialize the batch
		{
			PfxPairBatch *currPairBatch = batchInfo->pairBatches + batchInfo->maxBatches;
			currPairBatch->batch = numBatchInCurrPhase;
			currPairBatch->phase = batchInfo->maxPhases;
			currPairBatch->startPairId = currPairIdBuffer - batchInfo->pairIdBuffer;
			currPairBatch->numPairs = 0u;
		}

		// Continue to merge the rest heads
		for (PfxUInt32 currSize = halfMaxPairsPerBatch - 1; currSize > 0; --currSize){
			for (; duplicateStartPosForLists[currSize] < endPosForLists[currSize];){
				PfxInt32 headId = sortedListHeads[duplicateStartPosForLists[currSize]++];

				// Find the batch
				PfxPairBatch *currPairBatch = batchInfo->pairBatches + batchInfo->maxBatches;
				if (currPairBatch->numPairs + currSize > maxPairsPerBatchInCurrPhase){
					currPairIdBuffer += currPairBatch->numPairs;

					currPairBatch = batchInfo->pairBatches + (++batchInfo->maxBatches);
					currPairBatch->batch = ++numBatchInCurrPhase;
					currPairBatch->phase = batchInfo->maxPhases;
					currPairBatch->startPairId = currPairIdBuffer - batchInfo->pairIdBuffer;
					currPairBatch->numPairs = 0u;
				}

				// Add the pairs into buffer
				for (PfxInt32 rbId = headId; rbId >= 0; rbId = rbBatchNodes[rbId].next){
					for (PfxUInt32 j = startPosPairsBelongToRigidBody[rbId]; j < endPosPairsBelongToRigidBody[rbId]; ++j)
						currPairIdBuffer[currPairBatch->numPairs++] = pairIdsBelongToRigidBody[j];
				}
			}
		}

		//----------------------------------------------------------------------------
		// STEP 3-e: initialize for next iteration
		//----------------------------------------------------------------------------

		if (numNextUnappliedPairs > 0)
		{
			// Reset the numbers
			for (PfxUInt32 i = 0; i < arg.numRigidBodies; ++i)			accumPosPairsBelongToRigidBody[i] = numPairsRequiredToSolveRigidBody[i] = 0u;

			// Swap the lists
			PfxUInt32 *tempPairArray = currUnappliedPairs;
			currUnappliedPairs = nextUnappliedPairs;
			nextUnappliedPairs = tempPairArray;

			// Set the size of lists
			numCurrUnappliedPairs = numNextUnappliedPairs;
			numNextUnappliedPairs = 0u;

			// Move the id buffer pointer forward if the last batch is not empty
			PfxPairBatch *currPairBatch = batchInfo->pairBatches + batchInfo->maxBatches;
			if (currPairBatch->numPairs > 0u){
				currPairIdBuffer += currPairBatch->numPairs;
				++batchInfo->maxBatches;
			}

			// Increase the phase and reset the batch number
			++batchInfo->maxPhases;
			numBatchInCurrPhase = 0u;
		}
		else
			break;
	}

	if (batchInfo->pairBatches[batchInfo->maxBatches].numPairs > 0u && numValidPairs > 0){
		++batchInfo->maxBatches;
	}
	//	}

	if (arg.enableSortOfPairs)
	{
		//----------------------------------------------------------------------------
		// STEP 4: re-arrange the pairs
		//----------------------------------------------------------------------------

		PfxUInt32 *batchIdToWhichPairBelongs = memStock.alloc<PfxUInt32>(arg.numPairs);
		currPairIdBuffer = batchInfo->pairIdBuffer;

		for (PfxUInt32 i = 0; i < batchInfo->maxBatches; ++i){
			PfxPairBatch *pairBatch = batchInfo->pairBatches + i;
			for (PfxUInt32 j = 0; j < pairBatch->numPairs; ++j)
				batchIdToWhichPairBelongs[*(currPairIdBuffer++)] = i;
		}

		// validate batch Id
#if 0
		for( PfxUInt32 i = 0; i < numValidPairs; ++i ){
			PfxPairSimpleData *pairData = pairDatem + sortedPairs[ i ];
			SCE_PFX_ASSERT( batchIdToWhichPairBelongs[ pairData->pairId ] < batchInfo->maxBatches );
		}
#endif

		for (PfxUInt32 i = 0; i < batchInfo->maxBatches; ++i)	batchInfo->pairBatches[i].numPairs = 0u;

		for (PfxUInt32 i = 0; i < numValidPairs; ++i){
			PfxPairSimpleData *pairData = pairDatem + sortedPairs[i];
			PfxPairBatch *pairBatch = batchInfo->pairBatches + batchIdToWhichPairBelongs[pairData->pairId];
			batchInfo->pairIdBuffer[pairBatch->startPairId + pairBatch->numPairs++] = pairData->pairId;
		}
	}

	batchInfo->numValidPairs = numValidPairs;

	return batchInfo->numValidPairs;
}

void sortPairsByDepth(
	void *workBuff, PfxUInt32 workBytes, 
	const PfxPairSimpleData pairDatem[], PfxUInt32 numPairDatem, 
	const PfxUInt8 rigidBodyDepth[], PfxPairSimpleData outSortedPairDatem[])
{
	SimpleMemoryStock memStock;
	memStock.initialize(workBuff, workBytes);

	// Variables for sorting the unapplied pairs, to which will be referred later
	PfxUInt32 *depthOfPairs = memStock.alloc<PfxUInt32>(numPairDatem);

	// Variables for sorting the unapplied pairs, temporarily use
	PfxUInt32 *startPosSortedPairs = memStock.alloc<PfxUInt32>(515);
	PfxUInt32 *endPosSortedPairs = startPosSortedPairs + 1;
	PfxUInt32 *accumPosSortedPairs = endPosSortedPairs + 1;

	// Initialize
	for (PfxUInt32 i = 0; i < 515; ++i)	startPosSortedPairs[i] = 0u;

	// Sum the # of pairs according to the depth
	for (PfxUInt32 i = 0; i < numPairDatem; ++i){
		const PfxPairSimpleData *pairData = pairDatem + i;
		PfxUInt32 depthA = rigidBodyDepth[pairData->rbA];
		PfxUInt32 depthB = rigidBodyDepth[pairData->rbB];

		depthOfPairs[i] = depthA + depthB;
		++accumPosSortedPairs[depthOfPairs[i]];
	}

	// Accumulate
	for (PfxUInt32 i = 0; i < 512; ++i)	accumPosSortedPairs[i] += endPosSortedPairs[i];

	// Assign the pairs
	for (PfxUInt32 i = 0; i < numPairDatem; ++i)	outSortedPairDatem[endPosSortedPairs[depthOfPairs[i]]++] = pairDatem[i];
}

} //namespace pfxv4
} //namespace sce++

