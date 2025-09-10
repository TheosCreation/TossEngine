/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _PFX_USER_CUSTOM_STAGE_H
#define _PFX_USER_CUSTOM_STAGE_H

#include "pfx_simulation_stage.h"

namespace sce {
namespace pfxv4 {

class PfxUserCustomStage : public PfxSimulationStage {
private:
	static const PfxUInt32 numJobs = 32;

	struct JobArg {
		PfxUserCustomStage *stage;
		PfxInt32 dispatchId;
		PfxUInt32 numJobs;
		PfxInt32 (*userFunc)(PfxInt32 jobId, void *userData);
		void *userData;
	};

	static void job_userCustom(void *data, int uid);

public:

	PfxInt32 execUserCustomAsync(PfxUInt32 numJobs, PfxInt32 (*func)(PfxInt32 jobId, void *userData), void *userData);
};

} //namespace pfxv4
} //namespace sce

#endif // _PFX_USER_CUSTOM_STAGE_H

