/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _PFX_GATHER_SOLVER_PAIRS_STAGE_H
#define _PFX_GATHER_SOLVER_PAIRS_STAGE_H

#include "../rigidbody/pfx_simulation_stage.h"
#include "../collision/pfx_contact_complex.h"
#include "radix_sort.h"

namespace sce {
namespace pfxv4 {

class PfxGatherSolverPairsStage : public PfxSimulationStage {
private:
	static const PfxUInt32 numJobs = 32;

	struct GatherJobArg {
		PfxGatherSolverPairsStage *stage;
		PfxPairContainer *pairContainer;
		PfxUInt32 numJobs;
	} gatherJobArg;

	struct InitSortJobArg {
		PfxGatherSolverPairsStage *stage;
		int32_t radixSortBytes = 0;
		void *radixSortBuff = nullptr;
		PfxInt32 *numOutPairs;
	} initSortJobArg;

	struct RadixData : public PfxBroadphasePair {
		uint32_t getKey() { return pfxGetKey(*this); }
		bool operator < (RadixData value) { return getKey() < value.getKey(); }
	};

	RadixSort<RadixData> sorter;

	std::atomic<PfxInt32> numWorkPairs;

	PfxBroadphasePair *workPairs;
	PfxUInt32 radixSortBytes;
	PfxUInt8 *sortWorkBuff; // <- MAX_CONTACTS, numThreads

	static void job_gatherSolverPairs(void *data, int uid);
	static void job_initSort(void *data, int uid);

public:
	PfxInt32 preparePipeline(PfxUInt32 maxContacts);
	
	PfxInt32 dispatchGatherSolverPairs(PfxContactComplex &contactComplex, PfxBroadphasePair *outPairs, PfxInt32 *numOutPairs);
};

} //namespace pfxv4
} //namespace sce

#endif // _PFX_GATHER_SOLVER_PAIRS_STAGE_H

