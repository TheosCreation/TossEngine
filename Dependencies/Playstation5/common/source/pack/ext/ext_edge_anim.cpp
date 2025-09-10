/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2023 Sony Interactive Entertainment Inc.
* 
*/

#include "../../../include/pack/ext/ext_edge_anim.h"
#include "../../../include/pack/error_codes.h"
#include "../../../include/pack/pack_common.h"

namespace PackFile
{

static void clear_data(EdgeData& data)
{
    data.skeletons.clear();
    data.anim_to_skeleton.clear();
    data.animations.clear();
    data.buffers.clear();
}

int load_edge_section(void* user_data, SectionLoadParam& load_params)
{
    PackFile::BufferFile& stream = load_params.section_data;

    EdgeData* out_data = (EdgeData*)user_data;
    uint32_t skele_count = 0; 
    uint32_t anim_count = 0;
    read_from_stream(stream, skele_count);
    read_from_stream(stream, anim_count);

    out_data->skeletons = Array<EdgeAnimSkeleton*>(skele_count);
    out_data->animations = Array<EdgeAnimAnimation*>(anim_count);
    out_data->anim_to_skeleton = Array<uint32_t>(anim_count);

    if (load_params.pack_version >= MAKE_VERSION(0, 4, 0))
    {
        const size_t buffer_count = skele_count + anim_count;
        out_data->buffers = Array<Serialization::DataOffset>{buffer_count};
        read_from_stream(stream, out_data->buffers.data(), sizeof(Serialization::DataOffset) * buffer_count);
        read_from_stream(stream, out_data->anim_to_skeleton.data(), sizeof(uint32_t) * anim_count);
        if (stream.bad())
            return SCE_PACK_ERROR_IO;
        return SCE_OK;
    }

    int error = SCE_OK;
    for (uint32_t s_idx = 0; s_idx < skele_count; ++s_idx)
    {
        uint32_t size = 0;
        read_from_stream(stream, size);
        out_data->skeletons[s_idx] = (EdgeAnimSkeleton*)load_params.cpu_allocator.alloc(size, EDGE_SIMD_ALIGNMENT);
        if (out_data->skeletons[s_idx] == nullptr)
        {
            for (uint32_t j = 0; j < s_idx; ++j)
                load_params.cpu_allocator.dealloc(out_data->skeletons[j]);
            error = SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
            break;
        }
        read_from_stream(stream, out_data->skeletons[s_idx], size);
    }
    if (error)
        return error;
    for (uint32_t a_idx = 0; a_idx < anim_count; ++a_idx)
    {
        uint32_t size = 0;
        read_from_stream(stream, size);
        uint32_t skeleton_index = 0;
        read_from_stream(stream, skeleton_index);
        out_data->anim_to_skeleton[a_idx] = skeleton_index;
        out_data->animations[a_idx] = (EdgeAnimAnimation*)load_params.cpu_allocator.alloc(size, EDGE_SIMD_ALIGNMENT);
        if (out_data->animations[a_idx] == nullptr)
        {
            for (uint32_t j = 0; j < a_idx; ++j)
                load_params.cpu_allocator.dealloc(out_data->animations[j]);
            error = SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
            break;
        }
        read_from_stream(stream, out_data->animations[a_idx], size);
    }
    return error;
}

int patch_data(EdgeData& data, Array<DataMapping> const& data_mapping)
{
    size_t buffer_count = data.buffers.size();
    if (buffer_count == 0)
    {
        // This can be an old Pack file
        // Or maybe there can be no edge data at all.
        return SCE_OK;
    }
    if (data_mapping.size() == 0)
        return SCE_PACK_ERROR_INVALID_PARAM;

    size_t skele_count = data.skeletons.size();
    size_t anim_count  = data.animations.size();
    if (buffer_count < (skele_count + anim_count))
    {
        clear_data(data);
        return SCE_PACK_ERROR_IO;
    }

    for (uint32_t s_idx = 0; s_idx < skele_count; ++s_idx)
    {
        data.skeletons[s_idx] = static_cast<EdgeAnimSkeleton*>(get_ptr(data_mapping, data.buffers[s_idx]));
        if (data.skeletons[s_idx] == nullptr)
        {
            clear_data(data);
            return SCE_PACK_ERROR_IO;
        }
    }
    for (uint32_t a_idx = 0; a_idx < anim_count; ++a_idx)
    {
        data.animations[a_idx] = static_cast<EdgeAnimAnimation*>(get_ptr(data_mapping, data.buffers[skele_count + a_idx]));
        if (data.animations[a_idx] == nullptr)
        {
            clear_data(data);
            return SCE_PACK_ERROR_IO;
        }
    }
    return SCE_OK;
}

}
