/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2022 Sony Interactive Entertainment Inc.
* 
*/

#include "../../include/pack/pack_common.h"

namespace PackFile
{
namespace Serialization
{
// Description:
// Read a string from the specified file.
// Arguments:
// is - The file from which to read the string.
// string - The string into which to read.
// Returns:
// The error code.
int read(PackFile::BufferFile &is, PackFile::String &string, size_t& read_size)
{
    uint8_t const* start = is.cursor;
    const size_t temp_size = read_size;
    uint32_t string_length = read_uint16(is, read_size);
    if (temp_size == read_size)
        return SCE_PACK_ERROR_IO;
    string = PackFile::String(size_t(string_length + 1));
    if (string == nullptr)
        return SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
    
    read_size += PackFile::read_from_stream(is, string.data(), string_length);
    string[string_length] = '\0';

    size_t read_byte = is.cursor - start;
    if (read_byte != (string_length + sizeof(uint16_t)))
        return SCE_PACK_ERROR_IO;
    return SCE_OK;
}

int read_string(String& result, PackFile::BufferFile &is, uint8_t const* fields, PackFile::Array<uint8_t> const& strings_table)
{
    uint8_t head_desc = *fields;
    FieldType data_type = get_field_type(head_desc);
    if (data_type == FieldType::k_string)
    {
        if (head_desc & 0xF)
        {
            char const* table_ptr = (char const* )strings_table.data();
            size_t read_size = 0;
            uint32_t string_id = read_uint32(is, read_size);
            if (table_ptr != nullptr)
            {
                size_t table_size = strings_table.size();
                PACK_ASSERT(string_id < table_size);
                size_t len = strnlen_s(table_ptr + string_id, table_size);
                PACK_ASSERT(string_id + len <= table_size);
                result = PackFile::String(len);
                strncpy_s(result.data(), result.size(), table_ptr + string_id, len);
            }
            return SCE_OK;
        } else
        {
            size_t read_size = 0;
            return read(is, result, read_size);
        }
    }
    return SCE_PACK_ERROR_NOT_CONVERTIBLE;
}

}
}