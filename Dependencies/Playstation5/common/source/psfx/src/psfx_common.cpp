/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#include "psfx_details.h"


#include <cstdio> // snprintf

namespace psfx
{

namespace details
{

void cs_sync(sce::Agc::DrawCommandBuffer* dcb)
{
   sce::Agc::Core::gpuSyncEvent(dcb, sce::Agc::Core::SyncWaitMode::kDrainCompute, sce::Agc::Core::SyncCacheOp::kFlushShaderWriteForShaderRead);
}

void cs_sync(sce::Agc::AsyncCommandBuffer* dcb)
{
   sce::Agc::Core::gpuSyncEvent(dcb, sce::Agc::Core::AsyncSyncWaitMode::kDrainCompute, sce::Agc::Core::AsyncSyncCacheOp::kFlushShaderWriteForShaderRead);
}

SyncResult write_cs_sync(sce::Agc::DrawCommandBuffer* dcb)
{
   SyncResult result;
   
   result.label = reinterpret_cast<sce::Agc::Label*>(dcb->allocateTopDown(sizeof(sce::Agc::Label), sce::Agc::Alignment::kLabel));
   result.label->m_value = 0;

   sce::Agc::Core::GpuSyncPostProducerResult agc_result = sce::Agc::Core::gpuSyncPostProducer(dcb,
      sce::Agc::Core::SyncCacheOp::kFlushShaderWriteForShaderRead,
      result.label, 1);
   result.deferred_cache_ops = agc_result.m_deferredCacheOps;

   return result;
}

SyncResult write_cs_sync(sce::Agc::AsyncCommandBuffer* dcb)
{
   SyncResult result;

   result.label = reinterpret_cast<sce::Agc::Label*>(dcb->allocateTopDown(sizeof(sce::Agc::Label), sce::Agc::Alignment::kLabel));
   result.label->m_value = 0;

   sce::Agc::Core::GpuSyncPostProducerResult agc_result = sce::Agc::Core::gpuSyncPostProducer(dcb,
      sce::Agc::Core::SyncCacheOp::kFlushShaderWriteForShaderRead,
      result.label, 1);
   result.deferred_cache_ops = agc_result.m_deferredCacheOps;

   return result;
}

void wait_cs_sync(sce::Agc::DrawCommandBuffer* dcb, SyncResult result)
{
   sce::Agc::Core::gpuSyncPreConsumer(dcb, result.deferred_cache_ops, result.label, 1);
}

void wait_cs_sync(sce::Agc::AsyncCommandBuffer* dcb, SyncResult result)
{
   sce::Agc::Core::gpuSyncPreConsumer(dcb, result.deferred_cache_ops, result.label, 1);
}

void write_label(sce::Agc::DrawCommandBuffer* dcb, volatile sce::Agc::Label* label)
{
   sce::Agc::Core::gpuSyncEvent(dcb, sce::Agc::Core::SyncWaitMode::kAsynchronous, sce::Agc::Core::SyncCacheOp::kNone, sce::Agc::Core::SyncLabelVisibility::kGpu,
      label, 1);
}

void write_label(sce::Agc::AsyncCommandBuffer* dcb, volatile sce::Agc::Label* label)
{
   sce::Agc::Core::gpuSyncEvent(dcb, sce::Agc::Core::AsyncSyncWaitMode::kAsynchronous, sce::Agc::Core::AsyncSyncCacheOp::kNone, sce::Agc::Core::SyncLabelVisibility::kGpu,
      label, 1);
}

// Texture chain
void create_descriptor_chain(sce::Agc::Core::TextureSpec prototype, sce::Agc::Core::TextureSpec* tx_specs, uint32_t count)
{
   for (uint32_t i = 0; i < count; ++i)
   {
      tx_specs[i] = prototype;
      prototype.m_width  = details::max_of(1u, (prototype.m_width + 1) >> 1);
      prototype.m_height = details::max_of(1u, (prototype.m_height + 1) >> 1);
   }
}

void make_tx_descriptors(uint32_t width, uint32_t height, uint32_t slices, sce::Agc::Core::DataFormat format, sce::Agc::Core::TextureSpec* tx_specs, uint32_t count, bool use_dcc)
{
   sce::Agc::Core::TextureSpec desc;
   desc.init();
   
   desc.m_width  = width;
   desc.m_height = height;
   desc.m_numSlices = slices;
   desc.m_format = format;

   if (slices == 1)
   {
      desc.setType(sce::Agc::Core::Texture::Type::k2d);
   }
   else
   {
      desc.setType(sce::Agc::Core::Texture::Type::k2dArray);
   }

   desc.setIsWriteable(true);
   desc.setTileMode(sce::Agc::Core::Texture::TileMode::kRenderTarget);

   if (use_dcc)
   {
      sce::Agc::Core::translate(&desc.m_compression, width, format.m_format, sce::Agc::Core::DccCompatibility::kTextureReadWrite);
   }

   create_descriptor_chain(desc, tx_specs, count);
}

void make_tx_mip_descriptors(uint32_t width, uint32_t height, uint32_t mips, uint32_t slices, sce::Agc::Core::DataFormat format, sce::Agc::Core::TextureSpec& tx_spec, bool use_dcc)
{
   if (mips == 0)
   {
      mips = 1;
      uint32_t mip_width = width;
      uint32_t mip_height = height;
      while (mip_width > 1 || mip_height > 1)
      {
         if (mip_width > 1)  mip_width >>= 1;
         if (mip_height > 1) mip_height >>= 1;
         mips++;
      }
   }

   tx_spec.init();

   tx_spec.m_width = width;
   tx_spec.m_height = height;
   tx_spec.m_numMips = mips;
   tx_spec.m_numSlices = slices;
   tx_spec.m_format = format;

   tx_spec.setIsWriteable(true);
   tx_spec.setTileMode(sce::Agc::Core::Texture::TileMode::kRenderTarget);
   tx_spec.setType(slices == 1 ? sce::Agc::Core::Texture::Type::k2d : sce::Agc::Core::Texture::Type::k2dArray);

   if (use_dcc)
   {
      sce::Agc::Core::translate(&tx_spec.m_compression, width, format.m_format, sce::Agc::Core::DccCompatibility::kTextureReadWrite);
   }
}

void tx_memory_usage(sce::Agc::Core::TextureSpec* tx_specs, uint32_t count, sce::Agc::SizeAlign& out_size)
{
   for (int i = 0; i < count; ++i)
   {
      auto tx_size = sce::Agc::Core::getSize(&tx_specs[i]);
      out_size.m_align = details::max_of(tx_size.m_align, out_size.m_align);
      tx_size.m_size = tx_size.m_size + out_size.m_align; //overestimate
      out_size.m_size += tx_size.m_size;

      auto dcc_size = sce::Agc::Core::getSize(&tx_specs[i], sce::Agc::Core::TextureComponent::kMetadata);
      if (dcc_size.m_size != 0)
      {
         out_size.m_align = details::max_of(dcc_size.m_align, out_size.m_align);
         dcc_size.m_size = dcc_size.m_size + out_size.m_align;
         out_size.m_size += dcc_size.m_size;
      }
   }
}

void init_tx_chain( sce::Agc::ResourceRegistration::OwnerHandle owner, const char* resource_name, sce::Agc::Core::Texture* tx, sce::Agc::Core::TextureSpec* specs, uint32_t count, uint8_t*& memory )
{
   char temp_name[256];
   for (int i = 0; i < count; ++i)
   {
      specs[i].m_allowNullptr = 1;
      sce::Agc::Core::initialize(&tx[i], &specs[i]);
      auto tx_size = sce::Agc::Core::getSize(&specs[i]);
      memory = details::align_to(memory, tx_size.m_align);

      tx[i].setDataAddress(memory);

      sce::Agc::ResourceRegistration::ResourceHandle handle;
      if (count > 1)
      {
         snprintf(temp_name, 256, "%s-%d", resource_name, i);
      }
      else
      {
         snprintf(temp_name, 256, "%s", resource_name);
      }
      
      if (owner != sce::Agc::ResourceRegistration::kInvalidOwnerHandle)
         sce::Agc::ResourceRegistration::registerResource(&handle, owner, memory, tx_size.m_size, temp_name, sce::Agc::ResourceRegistration::ResourceType::kTextureBaseAddress , 0);
      
      memory += tx_size.m_size;

      auto dcc_size = sce::Agc::Core::getSize(&specs[i], sce::Agc::Core::TextureComponent::kMetadata);
      if (dcc_size.m_size != 0)
      {
         memory = details::align_to(memory, dcc_size.m_align);
         tx[i].setMetadataAddress(memory);

         // Initialize DCC metadata to clear to avoid any issues caused by invalid metadata.
         memset(memory, 0, dcc_size.m_size);

         memory += dcc_size.m_size;
      }
   }
}

void resize_tx(uint32_t width, uint32_t height, uint32_t mips, sce::Agc::Core::Texture& tx)
{
   if (mips == 0)
   {
      mips = 1;
      uint32_t mip_width = width;
      uint32_t mip_height = height;
      while (mip_width > 1 || mip_height > 1)
      {
         if (mip_width > 1)  mip_width >>= 1;
         if (mip_height > 1) mip_height >>= 1;
         mips++;
      }
   }

   sce::Agc::Core::TextureSpec spec;
   sce::Agc::Core::translate(&spec, &tx);

   spec.setWidth(width);
   spec.setHeight(height);
   spec.setNumMips(mips);

   // Keep same data addresses, should be big enough. Not moving anything avoids having to
   // re-register resources.

   sce::Agc::Core::initialize(&tx, &spec);
}

void resize_tx_chain(uint32_t width, uint32_t height, sce::Agc::Core::Texture* tx, uint32_t count)
{
   for (uint32_t i = 0; i < count; ++i)
   {
      resize_tx(width, height, 1, tx[i]);
      width = details::max_of(1u, (width + 1) >> 1);
      height = details::max_of(1u, (height + 1) >> 1);
   }
}

void clear_texture(sce::Agc::CommandBuffer* cb, const sce::Agc::Core::Texture& texture, const sce::Agc::Core::Encoder::EncoderValue& value)
{
   sce::Agc::CxRenderTarget rt;
   SceError ret = sce::Agc::Core::translate(&rt, &texture, sce::Agc::Core::MaintainCompression::kEnable);
   assert(ret == SCE_OK);
   (void)ret;

   sce::Agc::Toolkit::Result result = sce::Agc::Toolkit::clearRenderTargetCs(
      cb,
      &rt,
      sce::Agc::Toolkit::RenderTargetClearOp::kAuto,
      sce::Agc::Core::DccCompatibility::kTextureReadWrite,
      0, 2047,
      texture.getBaseMipLevel(), texture.getLastMipLevel());

   // Caller is expected to sync, just needs CS stall/flush.
   (void)result;
}

std::pair<float, float> compute_coefficent_for_linear_depth(float near_plane, float far_plane, ClipControlClipSpace clip_space, bool infinite_far, bool reverse_z)
{
   float A, B;
   if (clip_space == sce::Agc::CxClipControl::ClipSpace::kDX)
   {
      if (infinite_far)
      {
         A = -1.f;
         B = -near_plane;
      }
      else
      {
         A = far_plane / (near_plane - far_plane);
         B = (near_plane * far_plane) / (near_plane - far_plane);

      }
      if (reverse_z)
      {
         A = -A - 1;
         B = -B;
      }
   }
   else // sce::Agc::CxClipControl::ClipSpace::kOGL
   {
      if (infinite_far)
      {
         A = -1.f;
         B = -2.f* near_plane;
      }
      else
      {
         A = (far_plane + near_plane) / (near_plane - far_plane);
         B = 2.f*(near_plane * far_plane) / (near_plane - far_plane);

      }
      if (reverse_z)
      {
         A = -A;
         B = -B;
      }
   }
   return std::make_pair(A, B);
}

}

}
