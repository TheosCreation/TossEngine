/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_ATOMIC_SYNC_H
#define _SCE_PFX_ATOMIC_SYNC_H

#include "pfx_atomic.h"

#define SCE_PFX_DEFAULT_WAIT_SYNC_COUNT 4000

namespace sce{
namespace PhysicsEffects{

template<int WAIT = SCE_PFX_DEFAULT_WAIT_SYNC_COUNT>
class PfxAtomicSyncEvent
{
private:
	volatile int e;

public:
	void wait(volatile int mask)
	{
		int count = 0;
		while(mask != pfxAtomicCompareAndSwap(&e,mask,mask)) { // while(e!=1)
			if(((++count) % WAIT) == 0) {
				pfxYieldThread();
			}
		}
	}
	
	void signal(volatile int mask)
	{
		pfxAtomicOr(&e,mask);
	}
	
	void reset()
	{
		e = 0;
	}
};

class PfxAtomicSyncBarrier
{
private:
	PfxAtomicSyncEvent<SCE_PFX_DEFAULT_WAIT_SYNC_COUNT> events[2];
	int maxCount;
	volatile int step;
	volatile int counter;
	
public:
	void init(int n)
	{
		maxCount = n;
		events[0].reset();
		events[1].reset();
		step = 0;
		counter = n;
	}
	
	void reset(int n)
	{
		maxCount = n;
		events[0].reset();
		events[1].reset();
		step = 0;
		counter = n;
	}
	
	void release()
	{
	}
	
	void sync()
	{
		int step_ = step;
		int counter_ = pfxAtomicDecrement(&counter);
		if(counter_ == 1) {
			//printf("last step %d counter %d\n",step_,counter);
			counter = maxCount;
			step = 1-step_;
			events[1-step_].reset();
			events[step_].signal(1);
		}
		else {
			//printf("cont step %d counter %d\n",step_,counter);
			events[step_].wait(1);
		}
	}
};

} //namespace PhysicsEffects
} //namespace sce

#endif // _SCE_PFX_ATOMIC_SYNC_H
