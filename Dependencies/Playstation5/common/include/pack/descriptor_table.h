/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#ifndef PACK_FILE_DESCRIPTOR_TABLES_DEFINED
#define PACK_FILE_DESCRIPTOR_TABLES_DEFINED

#include "obj_serialization.h"

namespace PackFile {
namespace Serialization {

struct VectorTable;
struct DescriptorTable
{
    Array<uint16_t> names;
    Array<uint8_t> fields;

    DescriptorTable& add_scalar(uint16_t name, ScalarFormat f, ScalarSize s, Required required)
    {
        names.push_back(name);
        fields.push_back(make_scalar_type(f, s, required));
        return *this;
    }
    DescriptorTable& add_enum(uint16_t name, ScalarSize s, Required required)
    {
        names.push_back(name);
        fields.push_back(make_scalar_type(ScalarFormat::k_enum, s, required));
        return *this;
    }
    DescriptorTable& add_string(uint16_t name, Required required)
    {
        names.push_back(name);
        fields.push_back(make_sting_type(required));
        return *this;
    }
    DescriptorTable& add_sting_ref(uint16_t name, Required required)
    {
        names.push_back(name);
        fields.push_back(make_sting_ref_type(required));
        return *this;
    }
    VectorTable add_vector(uint16_t name, Required required);
    VectorTable add_vector(uint16_t name, size_t len, Required required);

    template<typename T>
    DescriptorTable& add_struct(uint16_t name, Required required)
    {
        names.push_back(name);
        auto type = make_struct_type(sizeof(T), required);
        fields.push_back(0xFF & (type >> 8));
        fields.push_back(0xFF & type);
        return *this;
    }

    DescriptorTable& add_table_type(uint16_t name, uint16_t index, Required required)
    {
        names.push_back(name);
        auto type = make_table_type(index, required);
        fields.push_back(0xFF & (type >> 8));
        fields.push_back(0xFF & type);
        return *this;
    }

    void set_root_table(RootTable& result, uint32_t signature = 0)
    {
        result.signature      = signature;
        result.element_names  = std::move(names);
        result.elements       = std::move(fields);
    }
};

struct VectorTable
{
    DescriptorTable* table = nullptr;
    DescriptorTable& of_enum(ScalarSize s)
    {
        table->fields.push_back(make_scalar_type(ScalarFormat::k_enum, s, k_optional));
        return *table;
    }
    DescriptorTable& of_scalar(ScalarFormat f, ScalarSize s)
    {
        table->fields.push_back(make_scalar_type(f, s, k_optional));
        return *table;
    }
    DescriptorTable& of_string()
    {
        table->fields.push_back(make_sting_type(k_optional));
        return *table;
    }
    DescriptorTable& of_string_reference()
    {
        table->fields.push_back(make_sting_type(k_optional));
        return *table;
    }
    VectorTable& of_vector()
    {
        table->fields.push_back(make_vector_head(k_optional));
        return *this;
    }
    VectorTable& of_vector(size_t len)
    {
        table->fields.push_back(make_vector_head(len, k_optional));
        return *this;
    }
    template<typename T>
    DescriptorTable& of_struct()
    {
        auto type = make_struct_type(sizeof(T), k_optional);
        table->fields.push_back(0xFF & (type >> 8));
        table->fields.push_back(0xFF & type);
        return *table;
    }

    DescriptorTable& of_table(uint16_t index)
    {
        auto type = make_table_type(index, k_optional);
        table->fields.push_back(0xFF & (type >> 8));
        table->fields.push_back(0xFF & type);
        return *table;
    }
};

inline VectorTable DescriptorTable::add_vector(uint16_t name, Required required)
{
    names.push_back(name);
    fields.push_back(make_vector_head(required));
    return VectorTable{ this };
}
inline VectorTable DescriptorTable::add_vector(uint16_t name, size_t len, Required required)
{
    names.push_back(name);
    fields.push_back(make_vector_head(len, required));
    return VectorTable{ this };
}

} } // namespace

#endif // PACK_FILE_DESCRIPTOR_TABLES_DEFINED
