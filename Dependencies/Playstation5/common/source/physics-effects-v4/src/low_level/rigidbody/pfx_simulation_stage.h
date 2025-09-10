/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _PFX_SIMULATION_STAGE_H
#define _PFX_SIMULATION_STAGE_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"

namespace sce {
namespace pfxv4 {

class PfxJobSystem;
class PfxContext;

class PfxSimulationStage {
private:
	PfxJobSystem *m_jobSystem = nullptr;
	PfxContext *m_context = nullptr;

public:
	void initialize(PfxJobSystem *jb, PfxContext *ctx) {m_jobSystem = jb; m_context = ctx;}

	PfxJobSystem *getJobSystem() {return m_jobSystem;}

	void setJobSystem( PfxJobSystem *jb ) { m_jobSystem = jb; }

	PfxContext *getRigidBodyContext() {return m_context;}
};

} //namespace pfxv4
} //namespace sce

#endif // _PFX_SIMULATION_STAGE_H

