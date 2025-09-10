/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2023 Sony Interactive Entertainment Inc.
* 
*/

#include "../../../include/pack/ext/ext_bvh.h"
#include "../../../include/pack/pack_common.h"

namespace PackFile
{

int load_bvh_section(void* user_data, SectionLoadParam& load_params)
{
    PackFile::BufferFile& stream = load_params.section_data;

    uint32_t bvh_count = 0;
    PackFile::read_from_stream(stream, bvh_count);

    PackFile::bvh_flags flags = {};
    if (load_params.pack_version >= MAKE_VERSION(0, 2, 6))
    {
        PackFile::read_from_stream(stream, flags);
    }
    BVHData& data = *static_cast<BVHData*>(user_data);

    data.decompression_mode = sce::Psr::DecompressionMode::kNonRefittable;
    if (flags & PackFile::bvh_flags::k_bvh_refittable)
        data.decompression_mode = sce::Psr::DecompressionMode::kRefittable;

    if (flags & PackFile::bvh_flags::k_bvh_compressed)
        data.compressed_bvh_descriptors = Array<sce::Psr::CompressedBottomLevelBvhDescriptor>{bvh_count};
    else
        data.descriptors = Array<sce::Psr::BottomLevelBvhDescriptor>{bvh_count};

    if (load_params.pack_version >= MAKE_VERSION(0, 4, 0))
    {
        data.bvh_offsets = Array<Serialization::DataOffset>{bvh_count};
        size_t read_size = sizeof(Serialization::DataOffset) * bvh_count;
        if (PackFile::read_from_stream(stream, data.bvh_offsets.data(), read_size) != read_size)
        {
            return SCE_PACK_ERROR_IO;
        }
        return SCE_OK;
    }

    int error_code = SCE_OK;
    for (uint32_t i = 0; i < bvh_count; ++i)
    {
        uint32_t bvh_size = 0;
        PackFile::read_from_stream(stream, bvh_size);

        void* bvh_memory = load_params.gpu_allocator.alloc(bvh_size, sce::Psr::kRequiredAlignment);
        if (bvh_memory == nullptr)
        {
            error_code = SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
            break;
        }
        if (PackFile::read_from_stream(stream, bvh_memory, bvh_size) != bvh_size)
        {
            error_code = SCE_PACK_ERROR_IO;
            break;
        }
        if (flags & PackFile::bvh_flags::k_bvh_compressed)
            data.compressed_bvh_descriptors[i] = static_cast<sce::Psr::CompressedBottomLevelBvhDescriptor>(bvh_memory);
        else
            data.descriptors[i] = static_cast<sce::Psr::BottomLevelBvhDescriptor>(bvh_memory);
    }


    // Clean up if we failed to read any bvh data.
    if (error_code != SCE_OK)
    {
        for (auto data : data.descriptors)
            load_params.gpu_allocator.dealloc(data);
        for (auto data : data.compressed_bvh_descriptors)
            load_params.gpu_allocator.dealloc(data);
    }
    return error_code;
}

void reset_data(BVHData& data)
{
    data.compressed_bvh_descriptors.clear();
    data.descriptors.clear();
    data.bvh_offsets.clear();
}

int patch_data(BVHData& data, Array<DataMapping> const& data_mapping)
{
    size_t bvh_count = data.bvh_offsets.size();
    if (bvh_count == 0)
    {
        // This can be an old Pack file
        // Or maybe there can be no bvh at all.
        return SCE_OK;
    }
    if (data_mapping.size() == 0)
        return SCE_PACK_ERROR_INVALID_PARAM;

    for (uint32_t i = 0; i < bvh_count; ++i)
    {
        void* ptr_data = get_ptr(data_mapping, data.bvh_offsets[i]);
        if (ptr_data == nullptr)
        {
            reset_data(data);
            return SCE_PACK_ERROR_IO;
        }
        if (data.compressed_bvh_descriptors.size() == bvh_count)
            data.compressed_bvh_descriptors[i] = static_cast<sce::Psr::CompressedBottomLevelBvhDescriptor>(ptr_data);
        else if (data.descriptors.size() == bvh_count)
            data.descriptors[i] = static_cast<sce::Psr::BottomLevelBvhDescriptor>(ptr_data);
        else
        {
            reset_data(data);
            return SCE_PACK_ERROR_IO;
        }
    }
    return SCE_OK;
}

}

