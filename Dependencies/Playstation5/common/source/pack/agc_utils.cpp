/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2023 Sony Interactive Entertainment Inc.
* 
*/

#include <agc.h>
#include "..\..\include\pack\pack_file.h"

namespace PackFile
{

sce::Agc::Core::VertexAttribute::Format to_agc_vertex_format(PackFile::VertexAttribFormat format)
{
    using sce::Agc::Core::VertexAttribute;
    switch(format)
    {
    case e_float16_2:           return VertexAttribute::Format::k16_16Float;
    case e_float16_4:           return VertexAttribute::Format::k16_16_16_16Float;
    case e_float32:             return VertexAttribute::Format::k32Float;
    case e_float32_2:           return VertexAttribute::Format::k32_32Float;
    case e_float32_3:           return VertexAttribute::Format::k32_32_32Float;
    case e_float32_4:           return VertexAttribute::Format::k32_32_32_32Float;
    case e_s_int8_4:            return VertexAttribute::Format::k8_8_8_8SInt;
    case e_s_int16_2:           return VertexAttribute::Format::k16_16SInt;
    case e_s_int16_4:           return VertexAttribute::Format::k16_16_16_16SInt;
    case e_s_int32:             return VertexAttribute::Format::k32SInt;
    case e_s_int32_2:           return VertexAttribute::Format::k32_32SInt;
    case e_s_int32_3:           return VertexAttribute::Format::k32_32_32SInt;
    case e_s_int32_4:           return VertexAttribute::Format::k32_32_32_32SInt;
    case e_u_int8_4:            return VertexAttribute::Format::k8_8_8_8UInt;
    case e_u_int16_2:           return VertexAttribute::Format::k16_16UInt;
    case e_u_int16_4:           return VertexAttribute::Format::k16_16_16_16UInt;
    case e_u_int32:             return VertexAttribute::Format::k32UInt;
    case e_u_int32_2:           return VertexAttribute::Format::k32_32UInt;
    case e_u_int32_3:           return VertexAttribute::Format::k32_32_32UInt;
    case e_u_int32_4:           return VertexAttribute::Format::k32_32_32_32UInt;
    case e_sn_int8_4:           return VertexAttribute::Format::k8_8_8_8SNorm;
    case e_sn_int16_2:          return VertexAttribute::Format::k16_16SNorm;
    case e_sn_int16_4:          return VertexAttribute::Format::k16_16_16_16SNorm;
    case e_un_int8_4:           return VertexAttribute::Format::k8_8_8_8UNorm;
    case e_un_int16_2:          return VertexAttribute::Format::k16_16UNorm;
    case e_un_int16_4:          return VertexAttribute::Format::k16_16_16_16UNorm;
    case e_float11_11_10:       return VertexAttribute::Format::k11_11_10Float;
    case e_sn_int11_11_10:      return VertexAttribute::Format::k11_11_10SNorm;
    case e_un_int11_11_10:      return VertexAttribute::Format::k11_11_10UNorm;
    case e_sn_int10_10_10_2:    return VertexAttribute::Format::k10_10_10_2SNorm;
    case e_un_int10_10_10_2:    return VertexAttribute::Format::k10_10_10_2UNorm;
    case e_none:                return VertexAttribute::Format::kVertexBuffer;
    }
    return VertexAttribute::Format::kVertexBuffer;
}

sce::Agc::UcPrimitiveType::Type to_agc_primitive_type(PackFile::PrimitiveType primitive_type)
{
    using sce::Agc::UcPrimitiveType;
    switch(primitive_type)
    {
    case PrimitiveType::e_none:                 return UcPrimitiveType::Type::kNone;
    case PrimitiveType::e_point_list:           return UcPrimitiveType::Type::kPointList;
    case PrimitiveType::e_line_list:            return UcPrimitiveType::Type::kLineList;
    case PrimitiveType::e_line_strip:           return UcPrimitiveType::Type::kLineStrip;
    case PrimitiveType::e_tri_list:             return UcPrimitiveType::Type::kTriList;
    case PrimitiveType::e_tri_fan:              return UcPrimitiveType::Type::kTriFan;
    case PrimitiveType::e_tri_strip:            return UcPrimitiveType::Type::kTriStrip;
    case PrimitiveType::e_patch:                return UcPrimitiveType::Type::kPatch;
    case PrimitiveType::e_line_list_adjacency:  return UcPrimitiveType::Type::kLineListAdjacency;
    case PrimitiveType::e_line_strip_adjacency: return UcPrimitiveType::Type::kLineStripAdjacency;
    case PrimitiveType::e_tri_list_adjacency:   return UcPrimitiveType::Type::kTriListAdjacency;
    case PrimitiveType::e_tri_strip_adjacency:  return UcPrimitiveType::Type::kTriStripAdjacency;
    case PrimitiveType::e_rect_list:            return UcPrimitiveType::Type::kRectList;
    case PrimitiveType::e_line_loop:            return UcPrimitiveType::Type::kLineLoop;
    case PrimitiveType::e_polygon:              return UcPrimitiveType::Type::kPolygon;
    }
    return UcPrimitiveType::Type::kNone;
}

sce::Agc::IndexSize to_agc_index_elem_size(uint8_t elem_size_bytes)
{
    using sce::Agc::IndexSize;
    switch (elem_size_bytes) {
        case 8:     return IndexSize::k8;
        case 16:    return IndexSize::k16;
        case 32:    return IndexSize::k32;
    }
    return IndexSize::k32;
}

}
