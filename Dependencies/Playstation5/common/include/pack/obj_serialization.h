/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2024 Sony Interactive Entertainment Inc.
* 
*/

#ifndef PACK_FILE_OBJ_SERIALIZATION_DEFINED
#define PACK_FILE_OBJ_SERIALIZATION_DEFINED

#include <cstdint>
#include "pack_common.h"
#include <type_traits>

namespace PackFile {

inline size_t read_from_stream(Stream& input_stream, void* RESTRICT_PTR out_buffer, size_t buffer_size)
{
    return input_stream.read(out_buffer, buffer_size);
}

template<typename T>
size_t read_from_stream(Stream& is, T& out_v)
{
    return read_from_stream(is, &out_v, sizeof(out_v));
}
    
namespace Serialization {

struct RootTable
{
    uint32_t signature = 0;
    Array<uint16_t> element_names;
    Array<uint8_t>  elements;
};

struct ParsingContext
{
    uint32_t file_version;
    PackFile::Array<uint8_t> const& strings_table;
    RootTable const* root_table = nullptr;

    RootTable const* get_table(unsigned int table_idx) const
    {
        PACK_ASSERT(table_idx < tables_count);
        return tables + table_idx;
    }
    void set_tables(RootTable const* t, unsigned int t_count)
    {
        tables = t;
        root_table = t;
        tables_count = t_count;
    }

    RootTable const* tables = nullptr;
    size_t tables_count = 0;
};

enum Required : uint8_t
{
    k_mandatory = 0b00010000,
    k_optional  = 0b00000000,
};

// Bit Size 
//   8, 16, 32, 64
// Format
//  Unsigned
//  Signed
//  Float
//  Enum

enum class ScalarSize
{
    k_8bits  = 0b0000,
    k_16bits = 0b0100,
    k_32bits = 0b1000,
    k_64bits = 0b1100,
};

enum class ScalarFormat
{
    k_uint  = 0b0000,
    k_sint  = 0b0001,
    k_float = 0b0010,
    k_enum  = 0b0011,
};

static constexpr uint8_t k_scalar_size_mask = 0x0C;
static constexpr uint8_t k_scalar_format_mask = 0x03;
static constexpr uint8_t k_field_type_mask = 0xE0;

enum class FieldType : uint8_t
{
    k_null   = 0b00000000,
    k_scalar = 0b00100000,
    k_string = 0b01000000,
    k_vector = 0b01100000,
    k_struct = 0b10000000,
    k_table  = 0b10100000,
};

// Root table
//  object signature (32 bits)
//  table size (16 bits)
//  table element count (16 bits)

using ScalarType     = uint8_t;
using StringType     = uint8_t;
using VectorHeadType = uint8_t;
using StructType     = uint16_t;
using TableType      = uint16_t;

// NULL (8 bits)
// 000X 0000
// Represented with nothing, 0 byte

// Literal (8 bits)
// 001X | format (2bits) | size (2bits)
// size determinated by size
constexpr auto make_scalar_type(ScalarFormat f, ScalarSize s, Required required)
{
    ScalarType res = (ScalarType)(FieldType::k_scalar)| ((uint8_t)s) | (uint8_t)f;
    res |= (ScalarType)required;
    return res;
}

// String (8 bits)
// 010X 0000
//  String literal 
//    Storage - 16 bits string length, followed by length byte
// 010X 0001
//  String reference
//    Storage - 32 bits index in the string table
constexpr auto make_sting_type(Required required)
{
    StringType res = (StringType)(FieldType::k_string);
    res |= (uint8_t)required;
    return res;
}

constexpr auto make_sting_ref_type(Required required)
{
    StringType res = (StringType)(FieldType::k_string) | 1;
    res |= (uint8_t)required;
    return res;
}

// Vector
// 0011 0000 <type> (variable)
//  variable length vector
//  Storage 16 bits??? vector length, followed by length copy of <type>
// 0011 XXXX <type> (variable)
//  XXXX is a 4 bits number that represent the length - 1
//  Storage XXXX types
constexpr auto make_vector_head(Required required)
{
    VectorHeadType res = (VectorHeadType)(FieldType::k_vector);
    res |= (uint8_t)required;
    return res;
}

inline auto make_vector_head(size_t len, Required required)
{
    PACK_ASSERT(len < 16);
    VectorHeadType res = (VectorHeadType)(FieldType::k_vector) | (0xF & (len - 1));
    res |= (uint8_t)required;
    return res;
}

// 0100 <length / 4> (12 bits)
//  Raw structure, up to 16KiB, aligned to 4 byte.
//  storage length * 4 of data
inline auto make_struct_type(size_t len, Required required)
{
    PACK_ASSERT((len & 0x0003) == 0); // len must be aligned to 4
    PACK_ASSERT((len < 0x4000)); // len
    StructType res = ((StructType)FieldType::k_struct << 8) | (0xFFF & (len / 4));
    res |= (uint8_t)required << 8;
    return res;
}

// Table / Object / Map 
// 0101 <index> (12 bits)
//  index reference to the root table
//  storage size in byte (32 bits) followed by N (16 bits) offset where N is the number of entrance in the root table
inline auto make_table_type(uint16_t idx, Required required)
{
    PACK_ASSERT((idx & 0xF000) == 0);
    TableType res = ((TableType)FieldType::k_table << 8) | (0xFFF & idx);
    res |= (uint8_t)required << 8;
    return res;
}

template<typename T>
constexpr ScalarType same_type(uint8_t header)
{
    if (std::is_enum<T>::value)
    {
        ScalarSize size {};
        switch(sizeof(T)){
        case 1: size = ScalarSize::k_8bits;
        case 2: size = ScalarSize::k_16bits;
        case 4: size = ScalarSize::k_32bits;
        case 8: size = ScalarSize::k_64bits;
        default: return false;
        }
        ScalarType T_type = make_scalar_type(ScalarFormat::k_enum, size, k_optional);
        return ScalarType(header & 0xEF) == T_type;
    }
    return false;
};

template<> constexpr ScalarType same_type<uint8_t>(uint8_t header)
{
    constexpr ScalarType T_type = make_scalar_type(ScalarFormat::k_uint, ScalarSize::k_8bits, k_optional);
    return ScalarType(header & 0xEF) == T_type;
}
template<> constexpr ScalarType same_type<uint16_t>(uint8_t header)
{
    constexpr ScalarType T_type = make_scalar_type(ScalarFormat::k_uint, ScalarSize::k_16bits, k_optional);
    return ScalarType(header & 0xEF) == T_type;
}
template<> constexpr ScalarType same_type<uint32_t>(uint8_t header)
{
    constexpr ScalarType T_type = make_scalar_type(ScalarFormat::k_uint, ScalarSize::k_32bits, k_optional);
    return ScalarType(header & 0xEF) == T_type;
}
template<> constexpr ScalarType same_type<uint64_t>(uint8_t header)
{
    constexpr ScalarType T_type = make_scalar_type(ScalarFormat::k_uint, ScalarSize::k_64bits, k_optional);
    return ScalarType(header & 0xEF) == T_type;
}
template<> constexpr ScalarType same_type<int8_t>(uint8_t header)
{
    constexpr ScalarType T_type = make_scalar_type(ScalarFormat::k_sint, ScalarSize::k_8bits, k_optional);
    return ScalarType(header & 0xEF) == T_type;
}
template<> constexpr ScalarType same_type<int16_t>(uint8_t header)
{
    constexpr ScalarType T_type = make_scalar_type(ScalarFormat::k_sint, ScalarSize::k_16bits, k_optional);
    return ScalarType(header & 0xEF) == T_type;
}
template<> constexpr ScalarType same_type<int32_t>(uint8_t header)
{
    constexpr ScalarType T_type = make_scalar_type(ScalarFormat::k_sint, ScalarSize::k_32bits, k_optional);
    return ScalarType(header & 0xEF) == T_type;
}
template<> constexpr ScalarType same_type<int64_t>(uint8_t header)
{
    constexpr ScalarType T_type = make_scalar_type(ScalarFormat::k_sint, ScalarSize::k_64bits, k_optional);
    return ScalarType(header & 0xEF) == T_type;
}
//template<> constexpr FieldType get_type<half>() { return make_scalar_type(ScalarFormat::k_float, ScalarSize::k_16bits, k_optional); }
template<> constexpr ScalarType same_type<float>(uint8_t header)
{
    constexpr ScalarType T_type = make_scalar_type(ScalarFormat::k_float, ScalarSize::k_32bits, k_optional);
    return ScalarType(header & 0xEF) == T_type;
}
template<> constexpr ScalarType same_type<double>(uint8_t header)
{
    constexpr ScalarType T_type = make_scalar_type(ScalarFormat::k_float, ScalarSize::k_64bits, k_optional);
    return ScalarType(header & 0xEF) == T_type;
}

inline bool is_mandatory(uint8_t head)
{
    return (head & 0x10) != 0;
}

constexpr FieldType get_field_type(uint8_t head)
{
    return (FieldType)(head & k_field_type_mask);
}

inline uint32_t get_stuct_size(uint8_t const* fields)
{
    return ((*fields & 0x0F) << 8) | *(fields + 1) << 2;
}

inline uint32_t get_obj_table_idx(uint8_t const* fields)
{
    return ((*fields & 0x0F) << 8) | *(fields + 1);
}

inline uint8_t const* get_next_field(uint8_t const* fields)
{
    FieldType type = get_field_type(*fields);
    size_t size = 0;
    switch(type)
    {
    case FieldType::k_null:
        size = 1;
        break;
    case FieldType::k_scalar:
        size = 1;
        break;
    case FieldType::k_string:
        size = 1;
        break;
    case FieldType::k_vector:
        fields = get_next_field(fields + 1);
        break;
    case FieldType::k_struct:
        size = 2;
        break;
    case FieldType::k_table:
        size = 2;
        break;
    }
    return fields + size;
}

#if defined(_MSC_VER)
#define VS2019_STATIC_BUG static
#else
#define VS2019_STATIC_BUG
#endif

template<typename From, typename To>
struct Converter
{
    template<bool enable>
    static int read_T(To& result, PackFile::BufferFile& is);

    template<>
    VS2019_STATIC_BUG int read_T<true>(To&, PackFile::BufferFile&)
    {
        // Some cases can't be converted at all
        // Eg. float to scoped enum
        return SCE_PACK_ERROR_NOT_CONVERTIBLE;
    }

    template<>
    VS2019_STATIC_BUG int read_T<false>(To& result, PackFile::BufferFile& is)
    {
        From k;
        read_from_stream(is, k);
        result = static_cast<To>(k);
        if (result == static_cast<To>(k)) return SCE_OK;
        return SCE_PACK_ERROR_NOT_CONVERTIBLE;
    }
};

#undef VS2019_STATIC_BUG

template<typename T>
int convert_to(T& result, PackFile::BufferFile &is, ScalarType entry_type)
{
    // Is not worth to convert string in integer
    // And is impossible for table, vector and structures
    if (get_field_type(entry_type) != FieldType::k_scalar) return SCE_PACK_ERROR_NOT_CONVERTIBLE;
    constexpr bool is_enum = std::is_enum<T>::value;
    ScalarSize   size   = (ScalarSize)(entry_type & k_scalar_size_mask);
    ScalarFormat format = (ScalarFormat)(entry_type & k_scalar_format_mask);
    switch(size)
    {
    case ScalarSize::k_8bits:
        switch (format)
        {
        case ScalarFormat::k_enum:
        case ScalarFormat::k_uint: return Converter<uint8_t, T>::template read_T<false>(result, is);
        case ScalarFormat::k_sint: return Converter<int8_t, T>::template read_T<false>(result, is);
        case ScalarFormat::k_float: return SCE_PACK_ERROR_NOT_CONVERTIBLE; // 8bit float don't exist
        }
        break;
    case ScalarSize::k_16bits:
        switch (format)
        {
        case ScalarFormat::k_enum:
        case ScalarFormat::k_uint: return Converter<uint16_t, T>::template read_T<false>(result, is);
        case ScalarFormat::k_sint: return Converter<int16_t, T>::template read_T<false>(result, is);
        case ScalarFormat::k_float: return SCE_PACK_ERROR_NOT_CONVERTIBLE; // not supported (yet)
        }
        break;
    case ScalarSize::k_32bits:
        switch (format)
        {
        case ScalarFormat::k_enum:
        case ScalarFormat::k_uint: return Converter<uint32_t, T>::template read_T<false>(result, is);
        case ScalarFormat::k_sint: return Converter<int32_t, T>::template read_T<false>(result, is);
        case ScalarFormat::k_float: return Converter<float, T>::template read_T<is_enum>(result, is);
        }
        break;
    case ScalarSize::k_64bits:
        switch (format)
        {
        case ScalarFormat::k_enum: // 64bit enum?
        case ScalarFormat::k_uint: return Converter<uint64_t, T>::template read_T<false>(result, is);
        case ScalarFormat::k_sint: return Converter<int64_t, T>::template read_T<false>(result, is);
        case ScalarFormat::k_float: return Converter<double, T>::template read_T<is_enum>(result, is);
        }
        break;
    default:
        break;
    }
    return SCE_PACK_ERROR_NOT_CONVERTIBLE;
}

template<typename T>
uint32_t read_scalar(T& result, PackFile::BufferFile &is, uint8_t const* fields)
{
    const ScalarType table_type = *(ScalarType*)fields;
    if (same_type<T>(table_type))
    {
        read_from_stream(is, result);
        return SCE_OK;
    }
    // Not same type, try to convert
    return convert_to(result, is, table_type);
}

int read_string(String& result, PackFile::BufferFile &is, uint8_t const* field_type, PackFile::Array<uint8_t> const& strings_table);

template<typename T>
int read_struct(T& result, PackFile::BufferFile& is, uint8_t const* fields)
{
    uint8_t head_desc = *fields;
    FieldType serialized_type = get_field_type(head_desc);
    if (serialized_type != FieldType::k_struct) return SCE_PACK_ERROR_NOT_CONVERTIBLE;

    uint32_t serialized_size = get_stuct_size(fields);
    if (sizeof(T) != serialized_size)  return SCE_PACK_ERROR_NOT_CONVERTIBLE;
    read_from_stream(is, result);
    return SCE_OK;
}

template<typename T, size_t size>
int read_vector(T* result, PackFile::BufferFile& is, uint8_t const* fields, ParsingContext const& ctx);

template<typename T>
int read_vector(Array<T>& result, PackFile::BufferFile& is, uint8_t const* fields, ParsingContext const& ctx);

namespace
{

template<typename>
struct sfinae_true : std::true_type{};

template<typename T>
auto test_read_object(int)
  -> sfinae_true< decltype( read_object(std::declval<PackFile::BufferFile&>(), std::declval<T&>(), std::declval<ParsingContext const&>()) )>;

template<typename>
std::false_type test_read_object(long);

template<class T>
struct has_read_object : decltype(test_read_object<T>(0)){};

template <class T, template <class...> class U>
struct is_PackArray : std::false_type {};

template <template <class...> class T, class... Args>
struct is_PackArray<T<Args...>, T> : std::true_type {};

template<typename T>
typename std::enable_if_t<std::is_arithmetic<T>::value || std::is_enum<T>::value, uint32_t> static_dispatch(T& result, PackFile::BufferFile& is, uint8_t const* fields, ParsingContext const&)
{
    return read_scalar(result, is, fields);
}

template<typename T>
typename std::enable_if_t<has_read_object<T>::value, uint32_t> static_dispatch(T& result, PackFile::BufferFile& is, uint8_t const* fields, ParsingContext const& ctx)
{
    ParsingContext temp_ctx = ctx;
    const unsigned int table_idx = get_obj_table_idx(fields);
    temp_ctx.root_table = ctx.get_table(table_idx);
    return read_object(is, result, temp_ctx);
}

template<typename T>
typename std::enable_if_t<std::is_same<T, PackFile::String>::value, uint32_t> static_dispatch(T& result, PackFile::BufferFile& is, uint8_t const* fields, ParsingContext const& ctx)
{
    return read_string(result, is, fields, ctx.strings_table);
}

template<typename T>
typename std::enable_if_t<is_PackArray<T, PackFile::Array>::value, uint32_t> static_dispatch(T& result, PackFile::BufferFile& is, uint8_t const* fields, ParsingContext const&)
{
    return read_vector(result, is, fields);
}

template<typename T>
typename std::enable_if_t<std::is_array<T>::value, uint32_t> static_dispatch(T& result, PackFile::BufferFile& is, uint8_t const* fields, ParsingContext const&)
{
    return read_vector<T, std::extent<T>::value>(result, is, fields);
}

template<typename T>
typename std::enable_if_t<std::is_class<T>::value && !has_read_object<T>::value, uint32_t> static_dispatch(T& result, PackFile::BufferFile& is, uint8_t const* fields, ParsingContext const&)
{
    return read_struct(result, is, fields);
}

}

template<typename T>
int read_vector_elements(T* result, uint32_t buff_size, PackFile::BufferFile& is, uint8_t const* fields, ParsingContext const& ctx)
{
    uint8_t const* element_fields = fields + 1;
    // TODO: if the type is POD and it matched the field descriptor we can do a memcopy
    for (uint32_t i = 0; i < buff_size; ++i)
    {
        int error_code = static_dispatch(result[i], is, element_fields, ctx);
        if (error_code != SCE_OK)
            return error_code;
    }
    return SCE_OK;
}

// Fixed size array
template<typename T, size_t size>
int read_vector(T* result, PackFile::BufferFile& is, uint8_t const* fields, ParsingContext const& ctx)
{
    PACK_ASSERT(fields != nullptr);
    uint8_t vector_desc = *fields;
    FieldType vector_type = get_field_type(vector_desc);
    if (vector_type != FieldType::k_vector) return SCE_PACK_ERROR_NOT_CONVERTIBLE;

    uint32_t len = (vector_desc & 0x0F);
    if (len == 0)
    {
        uint32_t read_byte = 0;
        uint16_t vec_size = 0;
        is.read(&vec_size, read_byte);
        len = vec_size;
    }
    if (len != size)
    {
        return -1; // wrong size?
    }
    return read_vector_elements(result, len, is, fields, ctx);
}

// Variable size array
template<typename T>
int read_vector(Array<T>& result, PackFile::BufferFile& is, uint8_t const* fields, ParsingContext const& ctx)
{
    PACK_ASSERT(fields != nullptr);
    uint8_t vector_desc = *fields;
    FieldType vector_type = get_field_type(vector_desc);
    if (vector_type != FieldType::k_vector) return SCE_PACK_ERROR_NOT_CONVERTIBLE;

    uint32_t len = (vector_desc & 0x0F);
    if (len  == 0)
    {
        uint16_t vec_size = 0;
        PackFile::read_from_stream(is, vec_size);
        len = vec_size;
    }
    if (len == 0) return SCE_OK; // vector of 0 length
    result = Array<T>(len);
    if (result == nullptr)
    {
        report_error("*** OUT OF MEMORY ***\n");
        return SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
    }
    return read_vector_elements(result.data(), len, is, fields, ctx);
}

inline uint8_t read_uint8(PackFile::Stream& is, size_t& out_size)
{
    uint8_t v;
    out_size += PackFile::read_from_stream(is, v);
    return v;
}

inline uint16_t read_uint16(PackFile::Stream& is, size_t& out_size)
{
    uint16_t v;
    out_size += PackFile::read_from_stream(is, v);
    return v;
}

inline uint32_t read_uint32(PackFile::Stream& is, size_t& out_size)
{
    uint32_t v;
    out_size += PackFile::read_from_stream(is, v);
    return v;
}

inline uint64_t read_uint64(PackFile::Stream& is, size_t& out_size)
{
    uint64_t v;
    out_size += PackFile::read_from_stream(is, v);
    return v;
}

//////////////////////

// Field name for template.
// Remeber to always add new field at the end of the template

// Field name for Buffer
enum class BufferFieldsName : uint16_t
{
    offset,
    size,
    elem_count,
    stride,
    data_section,
};

enum class MeshFieldsName : uint16_t
{
    bounding_box, // 6 float
    attributes,
    vertex_buffers,
    index_buffer,
    vertex_count,
    index_count,
    index_elem_size,
    primitive_type,
    skeleton_index,
    name,
    vertex_buffers_count,    // Legacy support
    vertex_attributes_count, //
    vertex_buffers_data,     //
    vertex_attributes_data,  //
    vertex_offset,
    index_offset,
};

enum class VertexAttributeFieldsName : uint16_t
{
    format,
    semantic,
    index,
    vertex_buffer_index,
    offset,
};

enum class MaterialPropertyFieldsName : uint16_t
{
    semantic,
    type,
};

enum class MaterialFieldsName : uint16_t
{
    model,
    model_version,
    flags,
    data_blob_size,   // These are for legacy version
    data_blob_data,   //
    properties_count, //
    properties_data,  //
    data_blob,
    properties,
    name
};

enum class TextureFieldsName : uint16_t
{
    tx_idx,
    uv_idx,
    uv_matrix, // 6 values
};

enum class NodeFieldsName : uint16_t
{
    name,
    parent_node_index,
    matrix,
    meshes_instance, // Contains mesh and material index
    // meshes,
    // material_indices,
};

enum class SkeletonFieldsName : uint16_t
{
    name,
    bone_count,      // Legacy
    node_index_data, //
    bind_pose_data,  //
    node_index,
    bind_pose,
};

enum class TextureRefFieldsName : uint16_t
{
    path,
};

enum class DataMappingName : uint16_t
{
    offset,
    size,
    mapping_flags,
};

} }

#endif // PACK_FILE_OBJ_SERIALIZATION_DEFINED