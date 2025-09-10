/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_ONEWAY_ALLOCATOR_H
#define _SCE_PFX_ONEWAY_ALLOCATOR_H

#include "../../include/physics_effects/base_level/base/pfx_common.h"

///////////////////////////////////////////////////////////////////////////////
// PfxOneWayAllocator

namespace sce {
namespace pfxv4 {

class PfxOneWayAllocator
{
public:
	PfxUInt8 *m_poolBuffer;
	PfxUInt8 *m_current;
	PfxInt32 m_poolBytes;
	
	PfxOneWayAllocator() : m_poolBuffer(NULL),m_poolBytes(0) {}
	
	PfxOneWayAllocator(PfxUInt8 *buf,PfxInt32 bytes)
	{
		m_poolBuffer = buf;
		m_poolBytes = bytes;
		clear();
	}
	
	~PfxOneWayAllocator()
	{
	}
	
	void initialize(PfxUInt8 *buf,PfxInt32 bytes)
	{
		m_poolBuffer = buf;
		m_poolBytes = bytes;
		clear();
	}
	
	PfxInt64 getAllocated()
	{
		return m_current - m_poolBuffer;
	}
	
	PfxInt64 getRest()
	{
		return m_poolBuffer + m_poolBytes - m_poolBuffer;
	}

	void *allocate(size_t bytes,PfxInt32 alignment=16)
	{
		SCE_PFX_ASSERT(alignment>0);
		
		bytes = SCE_PFX_MAX(bytes,alignment); // minmum alloc size

		uintptr_t p = (uintptr_t)m_current;
		
		uintptr_t align = alignment - 1;
		
		p = (p+align) & ~align;
		bytes = (bytes+align) & ~align;
		
		if((p + bytes) > (uintptr_t)(m_poolBuffer + m_poolBytes)) {
			return NULL;
		}
		
		m_current = (PfxUInt8*)(p + bytes);
		
		return (void*)p;
	}
	
	void clear()
	{
		m_current = m_poolBuffer;
	}
};

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_ONEWAY_ALLOCATOR_H
