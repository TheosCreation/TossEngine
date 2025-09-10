/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_HEAP_MANAGER_H
#define _SCE_PFX_HEAP_MANAGER_H

#include "../../include/physics_effects/base_level/base/pfx_common.h"

//J プールされたメモリを管理するスタックのサイズ
//E Size of a stack which used to manage pool memory
#define SCE_PFX_HEAP_STACK_SIZE 64

#define SCE_PFX_MIN_ALLOC_SIZE 4

#define SCE_PFX_ALLOC_BYTES_ALIGN4(bytes)		SCE_PFX_MAX(4,SCE_PFX_BYTES_ALIGN4(bytes))
#define SCE_PFX_ALLOC_BYTES_ALIGN8(bytes)		SCE_PFX_MAX(8,SCE_PFX_BYTES_ALIGN8(bytes))
#define SCE_PFX_ALLOC_BYTES_ALIGN16(bytes)		SCE_PFX_MAX(16,SCE_PFX_BYTES_ALIGN16(bytes))
#define SCE_PFX_ALLOC_BYTES_ALIGN128(bytes)		SCE_PFX_MAX(128,SCE_PFX_BYTES_ALIGN128(bytes))

///////////////////////////////////////////////////////////////////////////////
// PfxHeapManager

//J ＜補足＞
//J メモリはスタックで管理されています。取得した順と逆に開放する必要があります。
//J メモリを一気に開放したい場合はclear()を呼び出してください。
//J 最小割り当てサイズはSCE_PFX_MIN_ALLOC_SIZEで定義されます。

//E <Notes>
//E Memory is managed as a stack, so deallocate() needs to be called in reverse order.
//E Use clear() to deallocate all allocated memory at once.
//E SCE_PFX_MIN_ALLOC_SIZE defines the smallest amount of buffer.

namespace sce {
namespace pfxv4 {

class SCE_PFX_API PfxHeapManager
{
private:
	PfxUInt8 *m_heap;
	PfxUInt32 m_poolStack[SCE_PFX_HEAP_STACK_SIZE];
	PfxInt32 m_heapBytes;
	PfxInt32 m_curStack;
	PfxInt32 m_rest;
	
	PfxUInt8 *getCurAddr()
	{
		return m_heap + m_poolStack[m_curStack];
	}
	
public:
	enum {ALIGN4=4,ALIGN8=8,ALIGN16=16,ALIGN128=128};
	
	PfxHeapManager() : m_heap(NULL),m_heapBytes(0) {}
	
	PfxHeapManager(PfxUInt8 *buf,PfxInt32 bytes)
	{
		m_heap = buf;
		m_heapBytes = bytes;
		clear();
	}
	
	~PfxHeapManager()
	{
	}
	
	void initialize(PfxUInt8 *buf,PfxInt32 bytes)
	{
		m_heap = buf;
		m_heapBytes = bytes;
		clear();
	}
	
	PfxInt32 getAllocated()
	{
		return (PfxInt32)(m_poolStack[m_curStack]);
	}
	
	PfxInt32 getRest()
	{
		return m_heapBytes-getAllocated();
	}

	void *allocate(size_t bytes,PfxInt32 alignment = ALIGN16)
	{
		SCE_PFX_ASSERT(m_curStack<SCE_PFX_HEAP_STACK_SIZE-1);

		bytes = SCE_PFX_MAX(bytes,SCE_PFX_MIN_ALLOC_SIZE);

		uintptr_t p = (uintptr_t)getCurAddr();
		
		uintptr_t tmp = (uintptr_t)alignment - 1;
		
		p = (p+tmp) & ~tmp;
		bytes = (bytes+tmp) & ~tmp;
		
		SCE_PFX_ASSERT_MSG((p + bytes) <= (uintptr_t)(m_heap + m_heapBytes),"Memory overflow");
		
		m_poolStack[m_curStack] = (PfxUInt32)(p - (uintptr_t)m_heap);
		m_poolStack[++m_curStack] = (PfxUInt32)((p + bytes) - (uintptr_t)m_heap);
		
		m_rest = getRest();
		
		return (void*)p;
	}

	template< typename T >
	T* allocateArray(PfxUInt32 numElements, PfxUInt32 alignment)
	{
		const size_t memoryAmount = numElements * sizeof(T);
		void* memory = allocate(memoryAmount, alignment);
		return reinterpret_cast<T*>(memory);
	}

	void deallocate(void *p)
	{
#if 0
		m_curStack--;
		uintptr_t addr = (uintptr_t)getCurAddr();
		SCE_PFX_ASSERT_MSG(addr == (uintptr_t)p, "Invalid pointer deallocation");
#else
		(void) p;
		m_curStack--;
#endif
	}
	
	void clear()
	{
		m_poolStack[0] = 0;
		m_curStack = 0;
		m_rest = 0;
	}

	void printStack()
	{
		SCE_PFX_PRINTF("memStack %d/%d\n",m_curStack,SCE_PFX_HEAP_STACK_SIZE);
	}
};

} //namespace pfxv4
} //namespace sce
#endif // _SCE_PFX_HEAP_MANAGER_H
