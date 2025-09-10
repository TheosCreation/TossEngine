/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _PFX_ISLAND_CREATION_STAGE_H
#define _PFX_ISLAND_CREATION_STAGE_H

#include "../rigidbody/pfx_simulation_stage.h"
#include "../../../include/physics_effects/base_level/rigidbody/pfx_rigid_state.h"
#include "../../../include/physics_effects/base_level/solver/pfx_constraint_pair.h"

namespace sce {
namespace pfxv4 {

class PfxIslandCreationStage : public PfxSimulationStage {
private:
	static const PfxUInt32 numJobs = 32;

	struct JobArg {
		PfxIslandCreationStage *stage;
		PfxConstraintPair *pairs;
		PfxInt32 *numPairsPtr;
		PfxUInt32 numJobs;
	} jobArgs[4];

	struct IslandCreationSharedArg {
		PfxRigidState *states;
		PfxInt32 *roots;
		PfxUInt32 numRigidBodies;
	} shared;

	static void job_resetIslands(void *data, int uid);
	static void job_updateIslands2(void *data, int uid);
	static void job_compressRoute(void *data, int uid);

	PfxUInt8 *isFixed;

public:
	PfxInt32 preparePipeline(PfxInt32 *roots, PfxRigidState *states, PfxUInt32 numRigidBodies);

	PfxInt32 dispatchCreateIslands(
		PfxConstraintPair *contactPairs, PfxInt32 *numContactPairsPtr,
		PfxConstraintPair *jointPairs, PfxInt32 *numJointPairsPtr);
};

} //namespace pfxv4
} //namespace sce

#endif // _PFX_ISLAND_CREATION_STAGE_H

