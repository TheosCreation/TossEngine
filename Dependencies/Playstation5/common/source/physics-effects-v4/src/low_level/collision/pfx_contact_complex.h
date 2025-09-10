/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _PFX_CONTACT_COMPLEX_H
#define _PFX_CONTACT_COMPLEX_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../../src/low_level/collision/pfx_contact_manager.h"
#include "../broadphase/pfx_pair_container.h"
#include "pfx_contact_manager.h"

namespace sce {
namespace pfxv4 {

struct PfxContactComplex {
	PfxPairContainer pairContainer;
	PfxContactManager contactManager;

	void print();
};

} //namespace pfxv4
} //namespace sce

#endif // _PFX_CONTACT_COMPLEX_H

