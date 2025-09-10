/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2023 Sony Interactive Entertainment Inc.
* 
*/

#ifndef PACK_FILE_EXT_EDGE_ANIM_H
#define PACK_FILE_EXT_EDGE_ANIM_H

#include "../pack_file.h"
#include <edgeanim.h>

// Edge anim for pack

namespace  PackFile
{

struct EdgeData
{
    Array<EdgeAnimSkeleton*> skeletons;
    Array<uint32_t> anim_to_skeleton;
    Array<EdgeAnimAnimation*> animations;

    Array<Serialization::DataOffset> buffers;
};

static const uint32_t k_edge_header   = "EDGE"_FourCC;
int load_edge_section(void* user_data, SectionLoadParam& load_params);
int patch_data(EdgeData& data, Array<DataMapping> const& data_mapping);

}

#endif
