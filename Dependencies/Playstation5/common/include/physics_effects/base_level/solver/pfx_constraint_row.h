/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CONSTRAINT_ROW_H
#define _SCE_PFX_CONSTRAINT_ROW_H

#include "../base/pfx_common.h"

#define SCE_PFX_CONSTRAINT_STABILIZE_FACTOR	1e-3f

namespace sce {
namespace pfxv4 {

struct SCE_PFX_API PfxConstraintRow {
	PfxFloat m_normal[3];
	PfxFloat m_rhs;
	PfxFloat m_jacDiagInv;
	PfxFloat m_accumImpulse;

	void copyNormal(const PfxConstraintRow &row)
	{
		m_normal[0] = row.m_normal[0];
		m_normal[1] = row.m_normal[1];
		m_normal[2] = row.m_normal[2];
	}

	void setNormal(const PfxVector3 &n)
	{
		m_normal[0] = n[0];
		m_normal[1] = n[1];
		m_normal[2] = n[2];
	}

	PfxVector3 getNormal() const 
	{
		return PfxVector3(m_normal[0], m_normal[1], m_normal[2]);
	}
};

} //namespace pfxv4
} //namespace sce
#endif // _SCE_PFX_CONSTRAINT_ROW_H
