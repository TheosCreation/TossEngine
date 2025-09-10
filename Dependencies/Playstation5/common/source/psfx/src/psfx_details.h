/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#include <agc.h>
#include <tuple>
#include <shader.h>
#ifdef _DEBUG
#include <libdbg.h>
#else
#define SCE_DBG_LOG_ERROR(x)
#endif // !_DEBUG


#include "psfx_common.h"

namespace psfx
{

namespace details
{

constexpr size_t align_to(size_t v, size_t alignment)
{
   const size_t align_mask = alignment - 1;
   return (v + align_mask) & (~align_mask);
}

inline uint8_t* align_to(uint8_t* v, size_t alignment)
{
   const size_t align_mask = alignment - 1;
   const uintptr_t aligned_v = ((uintptr_t)v + align_mask) & (~align_mask);
   return (uint8_t*)aligned_v;
}

template<typename T>
constexpr T max_of(T a, T b)
{
   return a > b ? a : b;
}

template<typename T>
constexpr T min_of(T a, T b)
{
   return a < b ? a : b;
}

constexpr uint32_t round_div(uint32_t d, uint32_t x)
{
   return (d + (x-1)) / x;
}
constexpr int32_t round(float x)
{
   if (x < 0.0)
      return (int)(x - 0.5);
   else
      return (int)(x + 0.5);
}

template<typename T>
void set_to_zero(T& x)
{
   memset(&x, 0, sizeof(x));
}

std::pair<float,float> compute_coefficent_for_linear_depth(float near_plane, float far_plane, ClipControlClipSpace clip_space, bool infinite_far, bool reverse_z);

// Shader
typedef sce::Agc::Shader ComputeShader;

inline void set_compute_shader(sce::Agc::CommandBuffer* buff, sce::Agc::Shader* sh)
{
   if (sh->m_numShRegisters)
      buff->setShRegistersDirect(sh->m_shRegisters, sh->m_numShRegisters);
}

inline SceError set_srt(sce::Agc::CommandBuffer* buff, sce::Agc::Shader* sh, void* srt)
{
   sce::Agc::RegisterRange location = sce::Agc::Core::getResourceUserDataRange(sh, sce::Agc::UserDataLayout::DirectResourceType::kShaderResourceTable);
   uint16_t size = location.size();
   if (size)
   {
      return sce::Agc::bindUserData( nullptr, buff, sh->m_type, location.m_start, srt, size );
   }
   return -1;
}

template <typename T>
inline T* alloc_top_down(sce::Agc::TwoSidedAllocator* alloc, size_t count = 1, size_t alignment = sce::Agc::Alignment::kBuffer)
{
   return (T*)alloc->allocateTopDown(sizeof(T) * count, alignment);
}

inline void dispatch_indirect(sce::Agc::DrawCommandBuffer* dcb, const volatile sce::Agc::DispatchIndirectArgs* args, sce::Agc::DispatchModifier modifier)
{
   dcb->setBaseDispatchIndirectArgs(args);
   dcb->dispatchIndirect(0, modifier);
}

inline void dispatch_indirect(sce::Agc::AsyncCommandBuffer* acb, const volatile sce::Agc::DispatchIndirectArgs* args, sce::Agc::DispatchModifier modifier)
{
   acb->dispatchIndirect(args, modifier);
}

// Helper wrapper for condExec to predicate a series of commands.
// The specified function is called twice - first to measure the required size of the commands (so
// we can reserve space to ensure that the condExec and predicated packets will be in the same CB),
// then to record the actual commands.
template <typename Cb, typename Func>
inline void cond_exec(Cb* cb, uint32_t* command, Func func)
{
   uint32_t bytes = 0;
   Cb measure_cb;
   measure_cb.init(nullptr, 0, sce::Agc::Core::MeasureCallbackStatic<256>, &bytes);

   measure_cb.condExec(reinterpret_cast<sce::Agc::CondExecCommand*>(command), 0);
   const uint32_t cond_dwords = bytes / sizeof(uint32_t);
   func(&measure_cb);
   const uint32_t dwords = bytes / sizeof(uint32_t);

   cb->reserveSpaceInDwords(dwords);

   cb->condExec(reinterpret_cast<sce::Agc::CondExecCommand*>(command), dwords - cond_dwords);
   func(cb);
}

// Create CS and update memory pointers
sce::Agc::Shader* make_cs(sce::Agc::ResourceRegistration::OwnerHandle owner, const char* resource_name, char* header, const char* text );

// Immediate sync point - drains compute and flushes writes for subsequent shader reads.
void cs_sync(sce::Agc::DrawCommandBuffer* dcb);
void cs_sync(sce::Agc::AsyncCommandBuffer* dcb);

// Stall the PFP to ensure that it does not prefetch stale indirect dispatch arguments before the
// correct values are written.
static inline void stall_pfp(sce::Agc::DrawCommandBuffer* dcb)
{
   dcb->stallCommandBufferParser();
}

static inline void stall_pfp(sce::Agc::AsyncCommandBuffer* dcb)
{
   // Async compute has no PFP.
}

// Split sync point - flush writes after all prior work has completed, and wait at a later point.
struct SyncResult
{
   sce::Agc::Label* label;
   sce::Agc::Core::SyncCacheOp deferred_cache_ops;
};

SyncResult write_cs_sync(sce::Agc::DrawCommandBuffer* dcb);
SyncResult write_cs_sync(sce::Agc::AsyncCommandBuffer* dcb);
void wait_cs_sync(sce::Agc::DrawCommandBuffer* dcb, SyncResult result);
void wait_cs_sync(sce::Agc::AsyncCommandBuffer* dcb, SyncResult result);

// Write a label at end of pipe.
void write_label(sce::Agc::DrawCommandBuffer* dcb, volatile sce::Agc::Label* label);
void write_label(sce::Agc::AsyncCommandBuffer* dcb, volatile sce::Agc::Label* label);

// Tx chain
void create_descriptor_chain( sce::Agc::Core::TextureSpec prototype, sce::Agc::Core::TextureSpec* tx_specs, uint32_t count);
void make_tx_descriptors(uint32_t width, uint32_t height, uint32_t slices, sce::Agc::Core::DataFormat format, sce::Agc::Core::TextureSpec* tx_specs, uint32_t count, bool use_dcc = false);
void make_tx_mip_descriptors(uint32_t width, uint32_t height, uint32_t mips, uint32_t slices, sce::Agc::Core::DataFormat format, sce::Agc::Core::TextureSpec& tx_spec, bool use_dcc = false);
void tx_memory_usage(sce::Agc::Core::TextureSpec* tx_specs, uint32_t count, sce::Agc::SizeAlign& out_size);
void init_tx_chain(sce::Agc::ResourceRegistration::OwnerHandle owner, const char* resource_name, sce::Agc::Core::Texture* tx, sce::Agc::Core::TextureSpec* specs, uint32_t count, uint8_t*& memory);

// Resizes textures in place. Size must be less than or equal to the size the texture was originally created with.
void resize_tx(uint32_t width, uint32_t height, uint32_t mips, sce::Agc::Core::Texture& tx);
void resize_tx_chain(uint32_t width, uint32_t height, sce::Agc::Core::Texture* tx, uint32_t count);

void clear_texture(
   sce::Agc::CommandBuffer* cb,
   const sce::Agc::Core::Texture& texture,
   const sce::Agc::Core::Encoder::EncoderValue& value = sce::Agc::Core::Encoder::encode({ 0 }));

constexpr uint32_t k_CPU_alignment = 8;

struct ArenaAllocator
{
   ArenaAllocator(void* ptr)
      : arena {(uint8_t*)ptr }
   {
   }

   void* alloc(uint32_t size, uint32_t align)
   {
      arena = details::align_to(arena, align);
      void* ptr = arena;
      arena += size;
      return ptr;
   }

   // One is size, align, the other align count... >_>
   template<typename T>
   T* alloc(uint32_t align = k_CPU_alignment, uint32_t count = 1)
   {
      return static_cast<T*>(alloc(sizeof(T) * count, align));
   }

   uint8_t* arena;
};

template<typename T> class MarkerAutoScope
{
public:
   MarkerAutoScope(T* a, const char* marker)
   {
      cb = a;
      cb->pushMarker(marker);
   }
   ~MarkerAutoScope()
   {
      cb->popMarker();
   }
private:
   T* cb;
};

}

}
