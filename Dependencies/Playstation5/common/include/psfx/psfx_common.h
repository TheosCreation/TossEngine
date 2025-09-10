/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#include <cstdint>
#include <agc.h>

namespace psfx
{

typedef sce::Agc::CxClipControl::ClipSpace ClipControlClipSpace;

struct MemoryRequirement
{
   sce::Agc::SizeAlign mem_size;
};

struct MemoryPool
{
   void* mem_ptr;
};

struct Rectangle
{
   uint16_t left;
   uint16_t top;
   uint16_t right;
   uint16_t bottom;
};

}
