/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2023 Sony Interactive Entertainment Inc.
* 
*/

#ifndef PACK_FILE_AGC_UTILS_H
#define PACK_FILE_AGC_UTILS_H

#include <agc.h>
#include "pack_file.h"

namespace PackFile
{

sce::Agc::Core::VertexAttribute::Format to_agc_vertex_format(PackFile::VertexAttribFormat format);
sce::Agc::UcPrimitiveType::Type to_agc_primitive_type(PackFile::PrimitiveType primitive_type);

}

#endif