/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2023 Sony Interactive Entertainment Inc.
* 
*/

#ifndef PACK_FILE_EXT_BVH_H
#define PACK_FILE_EXT_BVH_H

#include "../pack_file.h"
#include <Psr/psr.h>

// BVH for pack

namespace  PackFile
{

enum bvh_flags
{
    k_bvh_compressed = 1,
    k_bvh_refittable = 1 << 1
};

struct BVHData
{
    Array<sce::Psr::BottomLevelBvhDescriptor> descriptors;
    Array<sce::Psr::CompressedBottomLevelBvhDescriptor> compressed_bvh_descriptors;

    sce::Psr::DecompressionMode decompression_mode;

    Array<Serialization::DataOffset> bvh_offsets;
};

static const uint32_t k_bvh_header = "BVH_"_FourCC;
int load_bvh_section(void* user_data, SectionLoadParam& load_params);
int patch_data(BVHData& data, Array<DataMapping> const& data_mapping);

}

#endif