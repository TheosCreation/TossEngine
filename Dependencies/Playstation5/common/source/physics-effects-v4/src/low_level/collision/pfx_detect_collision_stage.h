/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _PFX_DETECT_COLLISION_STAGE_H
#define _PFX_DETECT_COLLISION_STAGE_H

#include "../../../include/physics_effects/base_level/rigidbody/pfx_rigid_state.h"
#include "../../../include/physics_effects/base_level/collision/pfx_collidable.h"
#include "../rigidbody/pfx_simulation_stage.h"
#include "pfx_contact_complex.h"

namespace sce {
namespace pfxv4 {

class PfxDetectCollisionStage : public PfxSimulationStage {
private:
	static const PfxUInt32 numJobs = 32;

	struct JobArg {
		PfxDetectCollisionStage *stage;
		PfxInt32 dispatchId;
		PfxPairContainer *pairContainer;
		PfxContactManager *contactManager;
		PfxUInt32 numJobs;
	};

	PfxBool pfxPreCheckCcd(const PfxCoreSphere &coreSphereA,const PfxShape &shapeB,
		const PfxVector3 &posA0,const PfxVector3 &posA1,
		const PfxTransform3 &trB0,const PfxTransform3 &trB1);

	static void job_detectCollision(void *data, int uid);

public:
	struct DetectCollisionSharedArg {
		PfxRigidState *states;
		PfxCollidable *collidables;
		PfxFloat timeStep;
	} shared;

	PfxInt32 preparePipeline(PfxFloat timeStep, PfxRigidState *states, PfxCollidable *collidables);

	PfxInt32 dispatchDetectCollision(PfxContactComplex &contactComplex);
};

} //namespace pfxv4
} //namespace sce

#endif // _PFX_DETECT_COLLISION_STAGE_H

