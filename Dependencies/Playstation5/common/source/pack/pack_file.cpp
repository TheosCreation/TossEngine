/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2023 Sony Interactive Entertainment Inc.
* 
*/

#include "../../include/pack/pack_file.h"
#include <stdlib.h> // aligned alloc
#include "../../include/pack/pack_common.h"
#include "../../include/pack/descriptor_table.h"

// Description:
// Serialization namespace that contains most of the functionality.
namespace PackFile
{
namespace Serialization
{
    using PackFile::Array;
    using File_t = FILE*;

    inline File_t open_file_for_read(char const* filename)
    {
        FILE* file = nullptr;
        if (fopen_s(&file, filename, "rb") != 0)
        {
            return nullptr;
        }
        return file;
    }

    inline void close_file(File_t h)
    {
        fclose(h);
    }

    inline bool is_valid(File_t h)
    {
        return h != nullptr;
    }

    inline size_t get_file_size(File_t h)
    {
        const size_t curr = ftell(h);
        PACK_VERIFY(fseek(h, 0, SEEK_END) == 0);
        const size_t file_size = ftell(h);
        PACK_VERIFY(fseek(h, curr, SEEK_SET) == 0);
        return file_size;
    }

    size_t read_from_file(File_t h, void* RESTRICT_PTR out_buffer, size_t buffer_size)
    {
        size_t to_read = buffer_size;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(out_buffer);
        while (to_read > 0)
        {
            size_t read = fread(ptr, 1, to_read, h);
            if (read == 0)
            {
                if (ferror (h))
                {
                    report_error("Error reading from file\n");
                }
                break;
            }
            ptr += read;
            to_read -= read;
        }
        return buffer_size - to_read;
    }

    int read(PackFile::BufferFile &file, PackFile::Header& header)
    {
        size_t dummy_size = 0;
        header.version = read_uint32(file, dummy_size);
        if (header.version > PackFile::k_version)
        {
            report_error("Unknown version\n");
            return SCE_PACK_ERROR_VERSION_MISMATCH;
        }
        header.buffer_count    = read_uint32(file, dummy_size);
        header.meshes_count    = read_uint32(file, dummy_size);
        header.materials_count = read_uint32(file, dummy_size);
        header.nodes_count     = read_uint32(file, dummy_size);
        header.texture_count   = read_uint32(file, dummy_size);
        PACK_ASSERT(dummy_size == sizeof(PackFile::Header));
        if (file.bad()) return SCE_PACK_ERROR_IO;
        return SCE_OK;
    }

    int32_t check_signature(PackFile::BufferFile &is, RootTable const& desc)
    {
        uint32_t obj_size = 0;
        if (desc.signature != 0x00)
        {
            size_t read_size = 0;
            if (read_uint32(is, read_size) != desc.signature)
            {
                report_error("Wrong signature\n");
                return SCE_PACK_ERROR_MALFORMED_FILE;
            }
            obj_size = read_uint32(is, read_size);
            if (read_size != (sizeof(uint32_t) * 2))
                return SCE_PACK_ERROR_IO;
        }
        return obj_size;
    }

#define READ_PREAMBLE  RootTable const& table = *ctx.root_table; \
    const int32_t obj_size = check_signature(stream, table); \
    if (obj_size < 0) return SCE_PACK_ERROR_MALFORMED_FILE; \
    size_t start_pos = stream.position()
#define READ_FOOTER if (obj_size > 0) { \
        size_t read_size = stream.position() - start_pos; \
        if (read_size > (size_t)obj_size) return SCE_PACK_ERROR_MALFORMED_FILE; \
        stream.skip(obj_size - read_size); \
        } \
        return SCE_OK

    static int read_object(PackFile::BufferFile &stream, PackFile::TextureRef& buffer, ParsingContext const& ctx)
    {
        READ_PREAMBLE;
        uint8_t const* fields = table.elements.data();
        for (size_t i = 0; i < table.element_names.size(); ++i)
        {
            int error_code = SCE_OK;
            switch((TextureRefFieldsName)table.element_names[i])
            {
            case TextureRefFieldsName::path:
                error_code = read_string(buffer.path, stream, fields, ctx.strings_table);
                break;
            }
            if (error_code != SCE_OK)
                return error_code;
            fields = get_next_field(fields);
        }
        READ_FOOTER;
    }

    static int read_object(PackFile::BufferFile &stream, PackFile::Buffer& buffer, ParsingContext const& ctx)
    {
        READ_PREAMBLE;
        uint8_t const* fields = table.elements.data();
        for (size_t i = 0; i < table.element_names.size(); ++i)
        {
            int error_code = SCE_OK;
            switch ((BufferFieldsName)table.element_names[i])
            {
            case BufferFieldsName::offset:
                error_code = read_scalar(buffer.offset, stream, fields);
                break;
            case BufferFieldsName::size:
                error_code = read_scalar(buffer.size, stream, fields);
                break;
            case BufferFieldsName::elem_count:
                error_code = read_scalar(buffer.elem_count, stream, fields);
                break;
            case BufferFieldsName::stride:
                error_code = read_scalar(buffer.stride, stream, fields);
                break;
            case BufferFieldsName::data_section:
                error_code = read_scalar(buffer.data_section, stream, fields);
                break;
            default:
                if (is_mandatory(*fields))
                    error_code = SCE_PACK_ERROR_UNKNOWN_MANDATORY_FIELD_FOUND;
                break;
            }
            if (error_code != SCE_OK)
            {
                return error_code;
            }
            fields = get_next_field(fields);
        }
        READ_FOOTER;
    }

    static int read_object(PackFile::BufferFile& stream, PackFile::VertexAttribute& attribute, ParsingContext const& ctx)
    {
        READ_PREAMBLE;
        uint8_t const* fields = table.elements.data();
        for (size_t i = 0; i < table.element_names.size(); ++i)
        {
            int error_code = SCE_OK;
            switch ((VertexAttributeFieldsName)table.element_names[i])
            {
            case VertexAttributeFieldsName::format:
                error_code = read_scalar(attribute.format, stream, fields);
                break;
            case VertexAttributeFieldsName::semantic:
                error_code = read_scalar(attribute.semantic, stream, fields);
                break;
            case VertexAttributeFieldsName::index:
                error_code = read_scalar(attribute.index, stream, fields);
                break;
            case VertexAttributeFieldsName::vertex_buffer_index:
                error_code = read_scalar(attribute.vertex_buffer_index, stream, fields);
                break;
            case VertexAttributeFieldsName::offset:
                error_code = read_scalar(attribute.offset, stream, fields);
                break;
            default:
                if (is_mandatory(*fields))
                    return SCE_PACK_ERROR_UNKNOWN_MANDATORY_FIELD_FOUND;
            }
            if (error_code != SCE_OK)
               return error_code;
            fields = get_next_field(fields);
        }

        READ_FOOTER;
    }

    static int read_object(PackFile::BufferFile& stream, PackFile::Mesh& mesh, ParsingContext const& ctx)
    {
        static_assert(decltype(test_read_object<PackFile::VertexAttribute>(0))::value, "There is not Read declared for VertexAttribute");

        READ_PREAMBLE;

        uint8_t vx_attributes_count = 0;
        uint8_t vx_buffers_count = 0;
        uint8_t const* fields = table.elements.data();
        for (size_t i = 0; i < table.element_names.size(); ++i)
        {
            int error_code = SCE_OK;
            switch ((MeshFieldsName)table.element_names[i])
            {
            case MeshFieldsName::name:
                error_code = read_string(mesh.name, stream, fields, ctx.strings_table);
                break;
            case MeshFieldsName::bounding_box:
                error_code = read_struct(mesh.bounding_box, stream, fields);
                break;
            case MeshFieldsName::vertex_count:
                error_code = read_scalar(mesh.vertex_count, stream, fields);
                break;
            case MeshFieldsName::index_count:
                error_code = read_scalar(mesh.index_count, stream, fields);
                break;
            case MeshFieldsName::index_elem_size:
                error_code = read_scalar(mesh.index_elem_size, stream, fields);
                break;
            case MeshFieldsName::index_buffer:
                error_code = read_scalar(mesh.index_buffer, stream, fields);
                break;
            case MeshFieldsName::primitive_type:
                error_code = read_scalar(mesh.primitive_type, stream, fields);
                break;
            case MeshFieldsName::skeleton_index:
                error_code = read_scalar(mesh.skeleton_index, stream, fields);
                break;
            case MeshFieldsName::vertex_buffers:
                error_code = read_vector(mesh.vertex_buffers, stream, fields, ctx);
                break;
            case MeshFieldsName::attributes:
                error_code = read_vector(mesh.attributes, stream, fields, ctx);
                break;
            case MeshFieldsName::vertex_offset:
                error_code = read_scalar(mesh.vertex_offset, stream, fields);
                break;
            case MeshFieldsName::index_offset:
                error_code = read_scalar(mesh.index_offset, stream, fields);
                break;
            // Legacy attribute
            case MeshFieldsName::vertex_buffers_count:
                error_code = read_scalar(vx_buffers_count, stream, fields);
                mesh.vertex_buffers = PackFile::Array<uint32_t>(vx_buffers_count);
                break;
            case MeshFieldsName::vertex_buffers_data:
                error_code = read_vector_elements(mesh.vertex_buffers.data(), vx_buffers_count, stream, fields, ctx);
                break;
            case MeshFieldsName::vertex_attributes_count:
                error_code = read_scalar(vx_attributes_count, stream, fields);
                mesh.attributes = PackFile::Array<PackFile::VertexAttribute>(vx_attributes_count);
                break;
            case MeshFieldsName::vertex_attributes_data:
                error_code = read_vector_elements(mesh.attributes.data(), vx_attributes_count, stream, fields, ctx);
                break;
            default:
                if (is_mandatory(*fields))
                    return SCE_PACK_ERROR_UNKNOWN_MANDATORY_FIELD_FOUND;
            }
            if (error_code != SCE_OK)
               return error_code;
            fields = get_next_field(fields);
        }

        READ_FOOTER;
    }

    static int read_object(PackFile::BufferFile& stream, PackFile::MaterialProperty& property, ParsingContext const& ctx)
    {
        READ_PREAMBLE;
        uint8_t const* fields = table.elements.data();
        for (size_t i = 0; i < table.element_names.size(); ++i)
        {
            int error_code = SCE_OK;
            switch ((MaterialPropertyFieldsName)table.element_names[i])
            {
            case MaterialPropertyFieldsName::semantic:
                error_code = read_scalar(property.semantic, stream, fields);
                break;
            case MaterialPropertyFieldsName::type:
                error_code = read_scalar(property.type.rep, stream, fields);
                break;
            default:
                if (is_mandatory(*fields))
                    return SCE_PACK_ERROR_UNKNOWN_MANDATORY_FIELD_FOUND;
            }
            if (error_code != SCE_OK)
               return error_code;
            fields = get_next_field(fields);
        }
        READ_FOOTER;
    }

    static int size_multiplier(PackFile::MaterialPropertyType::Multiplicity mult)
    {
        switch (mult)
        {
        case PackFile::MaterialPropertyType::e_mpt_scalar: return 1;
        case PackFile::MaterialPropertyType::e_mpt_vec2: return 2;
        case PackFile::MaterialPropertyType::e_mpt_vec3: return 3;
        case PackFile::MaterialPropertyType::e_mpt_vec4: return 4;
        case PackFile::MaterialPropertyType::e_mpt_mat2x2: return 4;
        case PackFile::MaterialPropertyType::e_mpt_mat2x3: return 6;
        case PackFile::MaterialPropertyType::e_mpt_mat2x4: return 8;
        case PackFile::MaterialPropertyType::e_mpt_mat3x2: return 6;
        case PackFile::MaterialPropertyType::e_mpt_mat3x3: return 9;
        case PackFile::MaterialPropertyType::e_mpt_mat3x4: return 12;
        case PackFile::MaterialPropertyType::e_mpt_mat4x2: return 8;
        case PackFile::MaterialPropertyType::e_mpt_mat4x3: return 12;
        case PackFile::MaterialPropertyType::e_mpt_mat4x4: return 16;
        }
        return 1;
    }

    static size_t size_from_type(PackFile::MaterialPropertyType type)
    {
        PackFile::MaterialPropertyType::DataType data_type = PackFile::MaterialPropertyType::get_data_type(type);
        if (data_type == PackFile::MaterialPropertyType::e_mpt_texture)
        {
            return sizeof(PackFile::TextureProperties);
        }
        const int multiplier = size_multiplier(PackFile::MaterialPropertyType::get_multiplicity(type));
        switch(data_type)
        {
        case PackFile::MaterialPropertyType::e_mpt_char:   return multiplier * sizeof(uint8_t);
        case PackFile::MaterialPropertyType::e_mpt_short:  return multiplier * sizeof(uint16_t);
        case PackFile::MaterialPropertyType::e_mpt_int:    return multiplier * sizeof(int32_t);
        case PackFile::MaterialPropertyType::e_mpt_uchar:  return multiplier * sizeof(uint8_t);
        case PackFile::MaterialPropertyType::e_mpt_ushort: return multiplier * sizeof(uint16_t);
        case PackFile::MaterialPropertyType::e_mpt_uint:   return multiplier * sizeof(uint32_t);
        case PackFile::MaterialPropertyType::e_mpt_half:   return multiplier * sizeof(short);
        case PackFile::MaterialPropertyType::e_mpt_float:  return multiplier * sizeof(float);
        default : return 0;
        }
    }

    static int read_object(PackFile::BufferFile& stream, PackFile::Material& material, ParsingContext const& ctx)
    {
        static_assert(decltype(test_read_object<PackFile::MaterialProperty>(0))::value, "There is not Read declared for MaterialProperty");
        READ_PREAMBLE;

        uint16_t material_data_size = 0;
        uint16_t properties_count = 0;

        uint8_t const* fields = table.elements.data();
        for (size_t i = 0; i < table.element_names.size(); ++i)
        {
            int error_code = SCE_OK;
            switch ((MaterialFieldsName)table.element_names[i])
            {
            case MaterialFieldsName::name:
                error_code = read_string(material.name, stream, fields, ctx.strings_table);
                break;
            case MaterialFieldsName::model:
                error_code = read_scalar(material.model, stream, fields);
                break;
            case MaterialFieldsName::model_version:
                error_code = read_scalar(material.model_version, stream, fields);
                break;
            case MaterialFieldsName::flags:
                error_code = read_scalar(material.flags, stream, fields);
                break;
            case MaterialFieldsName::data_blob:
                error_code = read_vector(material.data_blob, stream, fields, ctx);
                break;
            case MaterialFieldsName::properties:
                error_code = read_vector(material.properties, stream, fields, ctx);
                break;
            // These are to support legacy serialization
            case MaterialFieldsName::data_blob_size:
                error_code = read_scalar(material_data_size, stream, fields);
                material.data_blob = Array<uint8_t>(material_data_size);
                break;
            case MaterialFieldsName::data_blob_data:
                error_code = read_vector_elements(material.data_blob.data(), material_data_size, stream, fields, ctx);
                break;
            case MaterialFieldsName::properties_count:
                error_code = read_scalar(properties_count, stream, fields);
                material.properties = Array<PackFile::MaterialProperty>(properties_count);
                break;
            case MaterialFieldsName::properties_data:
                error_code = read_vector_elements(material.properties.data(), properties_count, stream, fields, ctx);
                break;
            default:
                if (is_mandatory(*fields))
                    return SCE_PACK_ERROR_UNKNOWN_MANDATORY_FIELD_FOUND;
            }
            if (error_code != SCE_OK)
               return error_code;
            fields = get_next_field(fields);
        }
        uint8_t const* data_ptr = material.data_blob.data();
        for (uint32_t px = 0; px < material.properties.size(); ++px)
        {
            material.properties[px].value_ptr = data_ptr;
            size_t data_size = size_from_type(material.properties[px].type);
            data_ptr += data_size;
        }

        READ_FOOTER;
    }

    //
    struct MeshInstance
    {
        uint16_t mesh_idx;
        uint16_t material_idx;
    };
    static int read_object(PackFile::BufferFile& stream, PackFile::Node& node, ParsingContext const& ctx)
    {
        READ_PREAMBLE;
        uint8_t const* fields = table.elements.data();
        Array<MeshInstance> mesh_instances;
        for (size_t i = 0; i < table.element_names.size(); ++i)
        {
            int error_code = SCE_OK;
            switch ((NodeFieldsName)table.element_names[i])
            {
            case NodeFieldsName::name:
                error_code = read_string(node.name, stream, fields, ctx.strings_table);
                break;
            case NodeFieldsName::parent_node_index:
                error_code = read_scalar(node.parent_node_index, stream, fields);
                break;
            case NodeFieldsName::matrix:
                error_code = read_struct(node.matrix, stream, fields);
                break;
            case NodeFieldsName::meshes_instance:
                error_code = read_vector(mesh_instances, stream, fields, ctx);
                break;
            default:
                if (is_mandatory(*fields))
                    return SCE_PACK_ERROR_UNKNOWN_MANDATORY_FIELD_FOUND;
            }
            if (error_code != SCE_OK)
               return error_code;
            fields = get_next_field(fields);
        }
        const uint32_t mesh_count = mesh_instances.size();
        if (mesh_count > 0)
        {
            node.meshes = PackFile::Array<uint16_t>(mesh_count);
            node.material_index = PackFile::Array<uint16_t>(mesh_count);
            if (node.meshes == nullptr || node.material_index == nullptr)
            {
                report_error("*** OUT OF MEMORY ***\n");
                return SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
            }
            for (uint16_t i = 0; i < mesh_count; ++i)
            {
                node.meshes[i] = mesh_instances[i].mesh_idx;
                node.material_index[i] = mesh_instances[i].material_idx;
            }
        }
        READ_FOOTER;
    }

    static int read_object(PackFile::BufferFile& stream, PackFile::Skeleton& output, ParsingContext const& ctx)
    {
        READ_PREAMBLE;

        uint16_t bone_count = 0;
        uint8_t const* fields = table.elements.data();
        for (size_t i = 0; i < table.element_names.size(); ++i)
        {
            int error_code = SCE_OK;
            switch ((SkeletonFieldsName)table.element_names[i])
            {
            case SkeletonFieldsName::name:
                error_code = read_string(output.name, stream, fields, ctx.strings_table);
                break;
            // Legacy
            case SkeletonFieldsName::bone_count:
                error_code = read_scalar(bone_count, stream, fields);
                output.node_index = PackFile::Array<uint16_t>(bone_count);
                output.bind_pose = PackFile::Array<PackFile::Float4x4>(bone_count);
                if ((output.node_index == nullptr) || (output.bind_pose == nullptr))
                {
                    report_error("*** OUT OF MEMORY ***\n");
                    return SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
                }
                break;
            case SkeletonFieldsName::node_index_data:
                error_code = read_vector_elements(output.node_index.data(), bone_count, stream, fields, ctx);
                break;
            case SkeletonFieldsName::bind_pose_data:
                error_code = read_vector_elements(output.bind_pose.data(), bone_count, stream, fields, ctx);
                break;
            // End lecagy
            case SkeletonFieldsName::node_index:
                error_code = read_vector(output.node_index, stream, fields, ctx);
                break;
            case SkeletonFieldsName::bind_pose:
                error_code = read_vector(output.bind_pose, stream, fields, ctx);
                break;
            default:
                error_code = -1;
                break;
            }
            if (error_code != SCE_OK)
               return error_code;
            fields = get_next_field(fields);
        }
        READ_FOOTER;
    }

    static int read_object(PackFile::BufferFile& stream, PackFile::DataMapping& output, ParsingContext const& ctx)
    {
        READ_PREAMBLE;
        uint8_t const* fields = table.elements.data();
        for (size_t i = 0; i < table.element_names.size(); ++i)
        {
            int error_code = SCE_OK;
            switch ((DataMappingName)table.element_names[i])
            {
            case DataMappingName::offset:
                error_code = read_scalar(output.data_offset, stream, fields);
                break;
            case DataMappingName::size:
                error_code = read_scalar(output.size, stream, fields);
                break;
            case DataMappingName::mapping_flags:
                error_code = read_scalar(output.mapping_flags, stream, fields);
                break;
            }
            if (error_code != SCE_OK)
                return error_code;
            fields = get_next_field(fields);
        }
        READ_FOOTER;
    }

    // An error code indicating if the array elements were loaded successfully or not.
    template<typename T>
    int allocateAndRead(PackFile::BufferFile &file, PackFile::Array<T> &output_array, uint32_t count, ParsingContext const& ctx)
    {
        output_array = PackFile::Array<T>(count);
        if (count > 0 && output_array == nullptr)
        {
            report_error("*** OUT OF MEMORY ***\n");
            return SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
        }

        PACK_ASSERT(ctx.root_table != nullptr);
        PACK_ASSERT(ctx.tables_count > 0);

        for (auto& elem : output_array)
        {
            int result = read_object(file, elem, ctx);
            if (result != SCE_OK)
            {
                return result;
            }
        }
        if (file.bad()) return SCE_PACK_ERROR_IO;
        return SCE_OK;
    }

    static void* default_alloc(size_t size, size_t align, void*, uint32_t)
    {
#if _MSC_VER
       return _aligned_malloc(size, align);
#else // assuming posix
       return aligned_alloc(align, size);
#endif
    }

    static void default_free(void* ptr, void*, uint32_t)
    {
#if _MSC_VER
       _aligned_free(ptr);
#else
       free(ptr);
#endif
    }

    struct DeallocateOnFailure
    {
    public:
        DeallocateOnFailure(PackFile::Allocator const* d, void* p)
           : allocator { d }
           , ptr{ p }
        {
        }
        ~DeallocateOnFailure()
        {
           if (failure && (allocator != nullptr))
               allocator->dealloc(ptr);
        }
        bool failure = true;
    private:
        PackFile::Allocator const* allocator;
        void* ptr = nullptr;
    };

    struct CloseFileOnDestruction
    {
       CloseFileOnDestruction(File_t f)
           : handle {f}
       {
       }
       ~CloseFileOnDestruction() { close_file(handle); }
     private:
       File_t handle;
    };


    int check_pack(PackFile::Stream& is, PackFile::Serialization::Preamble& out_preamble, size_t file_size)
    {
        PackFile::Serialization::Preamble preamble;
        // Preamble
        if (read_from_stream(is, &preamble, sizeof(PackFile::Serialization::Preamble)) != sizeof(PackFile::Serialization::Preamble))
        {
            report_error("Error reading from file.\n");
            return SCE_PACK_ERROR_IO;
        }

        if ((preamble.signature[0] != PackFile::k_signature_0)
         || (preamble.signature[1] != PackFile::k_signature_1))
        {
            report_error("Invalid/corrupted file\n");
            return SCE_PACK_ERROR_MALFORMED_FILE;
        }
        if (preamble.gpu_data_size != 0)
        {
            uint64_t min_offset = sizeof(PackFile::Serialization::Preamble);
            if (preamble.version > PackFile::MAKE_VERSION(0, 2, 3))
            {
                min_offset += sizeof(PackFile::Serialization::Section) * preamble.section_count;
            }
            if (preamble.gpu_data_offset < min_offset)
            {
                report_error("Gpu offset is wrong as it's pointing to the metadata.\n");
                return SCE_PACK_ERROR_BUFFER_TOO_SMALL;
            }
            if ((preamble.gpu_data_offset + preamble.gpu_data_size) > file_size)
            {
                report_error("Gpu size is too big and is beyond the end of the file.\n");
                return SCE_PACK_ERROR_BUFFER_TOO_SMALL;
            }
        }
        out_preamble = preamble;
        if (is.bad()) return SCE_PACK_ERROR_IO;
        return SCE_OK;
    }

    int read_section(PackFile::Stream& stream, PackFile::Array<uint8_t> &dst, PackFile::Serialization::Section const& section)
    {
        if (!stream.seek(section.offset, PackFile::Stream::SeekFrom::k_begin))
        {
            report_error("Error reading from file.\n");
            return SCE_PACK_ERROR_IO;
        }
        dst = PackFile::Array<uint8_t>(section.size);
        if (dst == nullptr)
        {
            report_error("Out of memory.\n");
            return SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
        }
        if (read_from_stream(stream, dst.data(), section.size) != section.size)
        {
            report_error("Error reading from file.\n");
            return SCE_PACK_ERROR_IO;
        }
        return SCE_OK;
    }

    int load_root_tables(PackFile::BufferFile& is, RootTable* root_table, uint16_t table_count)
    {
        for (uint16_t i = 0; i < table_count; ++i)
        {
            read_from_stream(is, root_table[i].signature);

            uint16_t elements_count = 0;
            read_from_stream(is, elements_count);
            Array<RootTable> root_tables;
            root_table[i].element_names = Array<uint16_t>(elements_count);
            if (root_table[i].element_names == nullptr)
            {
                report_error("Out of memory.\n");
                return SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
            }
            is.read(root_table[i].element_names.data(), sizeof(uint16_t) * elements_count);

            uint16_t table_size = 0;
            read_from_stream(is, table_size);
            root_table[i].elements = Array<uint8_t>(table_size);
            if (root_table[i].elements == nullptr)
            {
                report_error("Out of memory.\n");
                return SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
            }
            is.read(root_table[i].elements.data(), table_size);
        }
        return SCE_OK;
    }

#define READ_ROOT_TABLE(is, tables)  uint16_t table_count = 0; \
    read_from_stream(is, table_count); \
    tables = Array<RootTable>(table_count); \
    const int code = load_root_tables(is, tables.data(), table_count); \
    if (code != SCE_OK) return code; \
    context.set_tables(tables.data(), table_count);

    int load_buffers(void* user_data, PackFile::SectionLoadParam& params)
    {
        ParsingContext context = {
            params.pack_version,
            params.string_table
        };
        Array<RootTable> root_tables;
        DescriptorTable legacy_buffer_table;
        if (context.file_version < PackFile::MAKE_VERSION(0, 3, 0))
        {
            root_tables = Array<RootTable>(1);
            legacy_buffer_table.add_scalar((uint16_t)BufferFieldsName::offset, ScalarFormat::k_uint, ScalarSize::k_64bits, k_mandatory)
                .add_scalar((uint16_t)BufferFieldsName::size,       ScalarFormat::k_uint, ScalarSize::k_64bits, k_mandatory)
                .add_scalar((uint16_t)BufferFieldsName::elem_count, ScalarFormat::k_uint, ScalarSize::k_32bits, k_mandatory)
                .add_scalar((uint16_t)BufferFieldsName::stride,     ScalarFormat::k_uint, ScalarSize::k_32bits, k_mandatory);

            if (context.file_version < PackFile::MAKE_VERSION(0, 2, 3))
                legacy_buffer_table.set_root_table(root_tables[0], PackFile::k_buffer_header);
            else
                legacy_buffer_table.set_root_table(root_tables[0]);
            context.set_tables(root_tables.data(), 1);
        }
        else
        {
            READ_ROOT_TABLE(params.section_data, root_tables);
        }
        return allocateAndRead(params.section_data, *(PackFile::Array<PackFile::Buffer>*)(user_data), params.elements_count, context);
    }

    int load_meshes(void* user_data, PackFile::SectionLoadParam& params)
    {
        ParsingContext context = {
            params.pack_version,
            params.string_table
        };

        Array<RootTable> root_tables;
        DescriptorTable legacy_mesh_table;
        DescriptorTable legacy_vertex_attribute_table;
        if (params.pack_version < PackFile::MAKE_VERSION(0, 3, 0))
        {
            root_tables = Array<RootTable>(2);
            legacy_mesh_table.add_struct<PackFile::BoundingBox>((uint16_t)MeshFieldsName::bounding_box, k_mandatory)
                .add_scalar((uint16_t)MeshFieldsName::vertex_count, ScalarFormat::k_uint, ScalarSize::k_32bits, k_mandatory)
                .add_scalar((uint16_t)MeshFieldsName::index_count, ScalarFormat::k_uint, ScalarSize::k_32bits, k_mandatory)
                .add_scalar((uint16_t)MeshFieldsName::index_elem_size, ScalarFormat::k_uint, ScalarSize::k_8bits, k_mandatory)
                .add_scalar((uint16_t)MeshFieldsName::primitive_type, ScalarFormat::k_enum, ScalarSize::k_8bits, k_mandatory)
                .add_scalar((uint16_t)MeshFieldsName::vertex_buffers_count, ScalarFormat::k_uint, ScalarSize::k_8bits, k_mandatory)
                .add_scalar((uint16_t)MeshFieldsName::vertex_attributes_count, ScalarFormat::k_uint, ScalarSize::k_8bits, k_mandatory)
                .add_vector((uint16_t)MeshFieldsName::vertex_buffers_data, k_mandatory).of_scalar(ScalarFormat::k_uint, ScalarSize::k_32bits)
                .add_scalar((uint16_t)MeshFieldsName::index_buffer, ScalarFormat::k_uint, ScalarSize::k_32bits, k_mandatory)
                .add_vector((uint16_t)MeshFieldsName::vertex_attributes_data, k_mandatory).of_table(1);
            if (params.pack_version < PackFile::MAKE_VERSION(0, 2, 4))
                legacy_mesh_table.add_string((uint16_t)MeshFieldsName::name, k_optional);
            else
                legacy_mesh_table.add_sting_ref((uint16_t)MeshFieldsName::name, k_optional);

            if (params.pack_version > PackFile::MAKE_VERSION(0, 2, 4))
                legacy_mesh_table.add_scalar((uint16_t)MeshFieldsName::skeleton_index, ScalarFormat::k_uint, ScalarSize::k_16bits, k_optional);
            legacy_mesh_table.set_root_table(root_tables[0], PackFile::k_mesh_header);

            legacy_vertex_attribute_table
                .add_scalar((uint16_t)VertexAttributeFieldsName::format,              ScalarFormat::k_enum, ScalarSize::k_8bits, k_mandatory)
                .add_scalar((uint16_t)VertexAttributeFieldsName::semantic,            ScalarFormat::k_enum, ScalarSize::k_8bits, k_mandatory)
                .add_scalar((uint16_t)VertexAttributeFieldsName::index,               ScalarFormat::k_uint, ScalarSize::k_8bits, k_mandatory)
                .add_scalar((uint16_t)VertexAttributeFieldsName::vertex_buffer_index, ScalarFormat::k_uint, ScalarSize::k_8bits, k_mandatory)
                .add_scalar((uint16_t)VertexAttributeFieldsName::offset,              ScalarFormat::k_uint, ScalarSize::k_16bits, k_mandatory);

            legacy_vertex_attribute_table.set_root_table(root_tables[1]);
            context.set_tables(root_tables.data(), 2);
        }
        else
        {
            READ_ROOT_TABLE(params.section_data, root_tables);
        }
        return allocateAndRead(params.section_data, *(PackFile::Array<PackFile::Mesh>*)(user_data), params.elements_count, context);
    }

    int load_materials(void* user_data, PackFile::SectionLoadParam& params)
    {
        ParsingContext context = {
            params.pack_version,
            params.string_table
        };

        Array<RootTable> root_tables;
        DescriptorTable legacy_material_table;
        DescriptorTable legacy_material_property_table;
        if (params.pack_version < PackFile::MAKE_VERSION(0, 3, 0))
        {
            root_tables = Array<RootTable>(2);
            legacy_material_table.add_scalar((uint16_t)MaterialFieldsName::model, ScalarFormat::k_enum, ScalarSize::k_16bits, k_mandatory)
                .add_scalar((uint16_t)MaterialFieldsName::model_version, ScalarFormat::k_uint, ScalarSize::k_16bits, k_optional)
                .add_scalar((uint16_t)MaterialFieldsName::flags, ScalarFormat::k_uint, ScalarSize::k_16bits, k_optional)

                .add_scalar((uint16_t)MaterialFieldsName::data_blob_size, ScalarFormat::k_uint, ScalarSize::k_16bits, k_optional)
                .add_scalar((uint16_t)MaterialFieldsName::properties_count, ScalarFormat::k_uint, ScalarSize::k_16bits, k_optional)
                .add_vector((uint16_t)MaterialFieldsName::data_blob_data, k_mandatory).of_scalar(ScalarFormat::k_uint, ScalarSize::k_8bits)
                .add_vector((uint16_t)MaterialFieldsName::properties_data, k_mandatory).of_table(1);
            if (params.pack_version < PackFile::MAKE_VERSION(0, 2, 4))
                legacy_material_table.add_string((uint16_t)MaterialFieldsName::name, k_optional);
            else
                legacy_material_table.add_sting_ref((uint16_t)MaterialFieldsName::name, k_optional);
            legacy_material_table.set_root_table(root_tables[0], PackFile::k_material_header);

            legacy_material_property_table.add_scalar((uint16_t)MaterialPropertyFieldsName::semantic, ScalarFormat::k_enum, ScalarSize::k_16bits, k_mandatory)
                .add_scalar((uint16_t)MaterialPropertyFieldsName::type, ScalarFormat::k_uint, ScalarSize::k_16bits, k_mandatory)
                .set_root_table(root_tables[1]);
            context.set_tables(root_tables.data(), 2);
        }
        else
        {
            READ_ROOT_TABLE(params.section_data, root_tables);
        }
        return allocateAndRead(params.section_data, *(PackFile::Array<PackFile::Material>*)(user_data), params.elements_count, context);
    }

    int load_nodes(void* user_data, PackFile::SectionLoadParam& params)
    {
        ParsingContext context = {
            params.pack_version,
            params.string_table
        };

        Array<RootTable> root_tables;
        DescriptorTable legacy_node_table;
        if (params.pack_version < PackFile::MAKE_VERSION(0, 3, 0))
        {
            root_tables = Array<RootTable>(1);
            if (params.pack_version < PackFile::MAKE_VERSION(0, 2, 4))
                legacy_node_table.add_string((uint16_t)NodeFieldsName::name, k_optional);
            else
                legacy_node_table.add_sting_ref((uint16_t)NodeFieldsName::name, k_optional);
            legacy_node_table.add_struct<PackFile::Float4x4>((uint16_t)NodeFieldsName::matrix, k_optional)
                .add_vector((uint16_t)NodeFieldsName::meshes_instance, k_mandatory).of_struct<MeshInstance>()
                .add_scalar((uint16_t)NodeFieldsName::parent_node_index, ScalarFormat::k_sint, ScalarSize::k_16bits, k_optional)
                .set_root_table(root_tables[0], PackFile::k_node_header);

            context.set_tables(root_tables.data(), 1);
        }
        else
        {
            READ_ROOT_TABLE(params.section_data, root_tables);
        }

        return allocateAndRead(params.section_data, *(PackFile::Array<PackFile::Node>*)(user_data), params.elements_count, context);
    }

    int load_textures(void* user_data, PackFile::SectionLoadParam& params)
    {
        ParsingContext context = {
            params.pack_version,
            params.string_table
        };

        Array<RootTable> root_tables;
        DescriptorTable legacy_texture_table;
        if (params.pack_version < PackFile::MAKE_VERSION(0, 3, 0))
        {
            root_tables = Array<RootTable>(1);
            legacy_texture_table.add_string((uint16_t)TextureRefFieldsName::path, k_mandatory)
                .set_root_table(root_tables[0]);
            context.set_tables(root_tables.data(), 1);
        }
        else
        {
            READ_ROOT_TABLE(params.section_data, root_tables);
        }

        return allocateAndRead(params.section_data, *(PackFile::Array<PackFile::TextureRef>*)(user_data), params.elements_count, context);
    }

    int load_skeletons(void* user_data, PackFile::SectionLoadParam& params)
    {
        ParsingContext context = {
            params.pack_version,
            params.string_table
        };

        Array<RootTable> root_tables;
        DescriptorTable legacy_skeleton_table;
        if (params.pack_version < PackFile::MAKE_VERSION(0, 3, 0))
        {
            root_tables = Array<RootTable>(1);
            if (params.pack_version < PackFile::MAKE_VERSION(0, 2, 4))
                legacy_skeleton_table.add_string((uint16_t)SkeletonFieldsName::name, k_optional);
            else
                legacy_skeleton_table.add_sting_ref((uint16_t)SkeletonFieldsName::name, k_optional);
            legacy_skeleton_table
                .add_scalar((uint16_t)SkeletonFieldsName::bone_count, ScalarFormat::k_uint, ScalarSize::k_16bits, k_mandatory)
                .add_vector((uint16_t)SkeletonFieldsName::node_index_data, k_mandatory).of_scalar(ScalarFormat::k_uint, ScalarSize::k_16bits)
                .add_vector((uint16_t)SkeletonFieldsName::bind_pose_data, k_mandatory).of_struct<PackFile::Float4x4>();

            legacy_skeleton_table.set_root_table(root_tables[0], PackFile::k_skeleton_header);
            context.set_tables(root_tables.data(), 1);
        }
        else
        {
            READ_ROOT_TABLE(params.section_data, root_tables);
        }

        return allocateAndRead(params.section_data, *(PackFile::Array<PackFile::Skeleton>*)(user_data), params.elements_count, context);
    }

    int load_data_mapping(void* user_data, PackFile::SectionLoadParam& params)
    {
        ParsingContext context = {
            params.pack_version,
            params.string_table
        };
        Array<RootTable> root_tables;
        READ_ROOT_TABLE(params.section_data, root_tables);

        return allocateAndRead(params.section_data, *(PackFile::Array<PackFile::DataMapping>*)(user_data), params.elements_count, context);
    }

    // Before the sections description was stored in the file, the sections were hard-coded in this immutable order.
    // This function is to support legacy files.
    int load_metadata_0_2_3(PackFile::BufferFile& buffer_file, PackFile::Package &out_pack, Allocator const& cpu_allocator, Allocator const& gpu_allocator)
    {
        // Skip signature that is already checked in check_pack function
        // Skip gpu offset as we read the gpu buffer in the callee
        size_t to_skip = sizeof(uint32_t) * 2 + sizeof(PackFile::OffsetSize);
        buffer_file.skip(to_skip);
        int result = read(buffer_file, out_pack.header);
        if (result != SCE_OK)
        {
            return result;
        }
        if (out_pack.header.version >= PackFile::MAKE_VERSION(0, 2, 4))
        {
            report_error("Unsupported version");
            return SCE_PACK_ERROR_VERSION_MISMATCH;
        }
        Array<uint8_t> empty_string_table;
        SectionLoadParam spl {
            (uint32_t)0x00, buffer_file, 0,
            cpu_allocator, gpu_allocator,
            empty_string_table, out_pack.header.version
        };

        spl.section_header = PackFile::k_buffer_header;
        spl.elements_count = out_pack.header.buffer_count;
        result = load_buffers((void*)(&out_pack.buffers), spl);
        if (result != SCE_OK)
            return result;

        spl.section_header = PackFile::k_mesh_header;
        spl.elements_count = out_pack.header.meshes_count;
        result = load_meshes((void*)(&out_pack.meshes), spl);
        if (result != SCE_OK)
            return result;

        spl.section_header = PackFile::k_material_header;
        spl.elements_count = out_pack.header.materials_count;
        result = load_materials((void*)(&out_pack.materials), spl);
        if (result != SCE_OK)
            return result;

        spl.section_header = PackFile::k_node_header;
        spl.elements_count = out_pack.header.nodes_count;
        result = load_nodes((void*)(&out_pack.nodes), spl);
        if (result != SCE_OK)
            return result;

        spl.section_header = PackFile::k_texture_header;
        spl.elements_count = out_pack.header.texture_count;
        result = load_textures((void*)(&out_pack.textures), spl);
        if (result != SCE_OK)
            return result;
        return SCE_OK;
    }

    struct LoaderMap
    {
      struct LoaderEntry
      {
          PackFile::SectionLoad load;
          void* data;
      };
      LoaderMap(size_t count)
          : keys(count)
          , loaders(count)
      {
      }
      void set_loader(uint32_t key, PackFile::SectionLoad func, void* data)
      {
          size_t insert_point = count;
          LoaderEntry new_entry = { func, data };
          for (size_t i = 0; i < count; i++)
          {
              if (keys[i] == key)
              {
                  loaders[i] = new_entry;
                  return;
              }
              if (keys[i] > key)
              {
                  insert_point = i;
                  break;
              }
          }
          if (count == keys.getCount())
              return;
          count++;
          for (size_t i = count; i > insert_point + 1; i--)
          {
              keys[i-1]    = keys[i - 2];
              loaders[i-1] = loaders[i - 2];
          }
          keys[insert_point] = key;
          loaders[insert_point] = LoaderEntry{func, data};
      }
      LoaderEntry const* get_loader(uint32_t key) const
      {
          size_t low = 0;
          size_t high = count;
          while (low < high)
          {
              // binary search
              size_t pivot = (high + low) / 2;
              uint32_t found = keys[pivot];
              if (found == key)
                  return &loaders[pivot];
              if (key < found)
                  high = pivot;
              else
                  low = pivot + 1;
          }
          return nullptr;
      }
      size_t count = 0;
      PackFile::Array<uint32_t>  keys;
      PackFile::Array<LoaderEntry>  loaders;
    };

    void check_allocator(PackFile::Allocator& allocator)
    {
        if (allocator.alloc_cb == nullptr)
        {
            PACK_ASSERT(allocator.dealloc_cb == nullptr);
            allocator.alloc_cb = default_alloc;
            allocator.dealloc_cb = default_free;
        }
        else
        {
            PACK_ASSERT(allocator.dealloc_cb != nullptr);
        }
    }

   struct FileStream : public PackFile::Stream
   {
        FileStream(File_t f)
           : file { f }
           , file_size { get_file_size(file) }
        {
        }

        // Receive the file from external source, not this class responsibility to close it.
        ~FileStream() = default;

        bool skip(size_t offset) override
        {
            if (bad()) return false;
            const int ret = fseek(file, (long int)offset, SEEK_CUR);
            error = ret != 0;
            return ret == 0;
        }

        size_t read(void* RESTRICT_PTR buffer, size_t size) override
        {
            if (bad()) return 0;
            const size_t read_data = read_from_file(file, buffer, size);
            error = read_data < size;
            return read_data;
        }

        bool seek(int64_t offset, SeekFrom start) override
        {
            if (bad()) return false;
            int ret = 0;
            switch (start)
            {
            case PackFile::Stream::SeekFrom::k_begin:
               ret = fseek(file, (long int)offset, SEEK_SET);
               break;
            case PackFile::Stream::SeekFrom::k_current:
               ret = fseek(file, (long int)offset, SEEK_CUR);
               break;
            case PackFile::Stream::SeekFrom::k_end:
               ret = fseek(file, (long int)offset, SEEK_END);
               break;
            }
            error = ret !=0;
            return ret == 0;
        }

        size_t position() const override
        {
            long result = ftell(file);
            if (result < 0)
            {
                error = true;
                return 0;
            }
            return size_t(result);
        }

        size_t size() const override
        {
            return file_size;
        }

        File_t file = nullptr;
        size_t file_size = 0;
   };

    int load_section(Stream& stream, SectionLoad load_func, void* load_data, Section const& section, SectionLoadParam const& load_param_template)
    {
        Array<uint8_t> section_data;
        int result = read_section(stream, section_data, section);
        if (result != SCE_OK)
        {
            report_error("Error while reading a section.\n");
            return result;
        }
        BufferFile data_file { section_data };
        SectionLoadParam load_param {
            section.signature, data_file, section.element_count,
            load_param_template.cpu_allocator, load_param_template.gpu_allocator,
            load_param_template.string_table, load_param_template.pack_version
        };
        result = load_func(load_data, load_param);
        if (result != SCE_OK)
        {
            report_error("Loading a section.\n");
            return result;
        }
        return SCE_OK;
    }

    int resolve_mapping_data(Array<DataMapping>& data_mapping, Stream& stream, Allocator const& cpu_allocator, Allocator const& gpu_allocator)
    {
        for (auto& m : data_mapping)
        {
            if (!stream.seek(m.data_offset, Stream::SeekFrom::k_begin))
            {
                report_error("Error reading from file.\n");
                return SCE_PACK_ERROR_IO;
            }
            void* data_ptr = nullptr;
            if (m.mapping_flags == DataMapping::cpu_rw)
            {
                data_ptr = cpu_allocator.alloc(m.size, 0x10000);
            }
            else
            {
                data_ptr = gpu_allocator.alloc(m.size, 0x10000);
            }
            if (read_from_stream(stream, data_ptr, m.size) != m.size)
            {
                report_error("Error reading from file.\n");
                return SCE_PACK_ERROR_IO;
            }
            m.data_offset = reinterpret_cast<uint64_t>(data_ptr);
        }
        return SCE_OK;
    }

    int read_gpu_data(void** out_ptr, Stream& stream, size_t data_size, size_t data_offset, Allocator const& gpu_allocator)
    {
        // GPU Data
        if (!stream.seek(data_offset, Stream::SeekFrom::k_begin))
        {
            report_error("Error reading from file.\n");
            return SCE_PACK_ERROR_IO;
        }
        void *temp_ptr = gpu_allocator.alloc(data_size, 256);
        if (temp_ptr == nullptr)
        {
            report_error("Out of memory.\n");
            return SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
        }
        if (read_from_stream(stream, temp_ptr, data_size) != data_size)
        {
            gpu_allocator.dealloc(temp_ptr);
            report_error("Error reading from file.\n");
            return SCE_PACK_ERROR_IO;
        }
        *out_ptr = temp_ptr;
        return SCE_OK;
    }


    void* get_ptr(Array<DataMapping> const& data_mapping, DataOffset const& entry)
    {
        if (entry.mapping_idx > data_mapping.size())
            return nullptr;
        DataMapping const& mapping = data_mapping[entry.mapping_idx];
        if ((entry.offset + entry.size) > mapping.size)
            return nullptr;
        uint8_t* ptr_data = reinterpret_cast<uint8_t*>(mapping.data_offset);
        return ptr_data + entry.offset;
    }
} } // namespace PackFile::Serialization


/*
   0            32           64           96           128
   | ---------- + ---------- + ---------- + ---------- |
   | Signature0 | Signature1 | gpu data offset         |
   | gpu data size           | version    |section_count|
   | ---------- + ---------- + ---------- + ---------- |
   | BUFF       | count      | offset     | size       |
   | MESH       | count      | offset     | size       |
   | MATE       | count      | offset     | size       |
   | NODE       | count      | offset     | size       |
   | ....                                              |
   | --------------------------------------------------|
   | GPU DATA...                                       |
   |    ...                                            |
   |                                                   |
   | --------------------------------------------------|
   | Section data...                                   |
   |    ...
*/

// Description:
// The namespace containing the pack file loading interface.
namespace PackFile
{
    static_assert(sizeof(Header) == 24, "Header not of expected size");
    static_assert(sizeof(MaterialPropertyType) == 2, "MaterialPropertyType not of expected size");
    static_assert(sizeof(VertexAttribute) == 6, "VertexAttribute not of expected size");
    static_assert(sizeof(BoundingBox) == 24, "BoundingBox not of expected size");

    int peek_gpu_data_offset(void const* metadata_buffer, size_t buffer_size, OffsetSize& out_data)
    {
        BufferFile stream { (uint8_t const*)metadata_buffer, (uint8_t const*)metadata_buffer + buffer_size };
        Serialization::Preamble preamble;
        int result = check_pack(stream, preamble, size_t(~0));
        if (result != SCE_OK)
        {
            return result;
        }
        out_data = { preamble.gpu_data_offset, preamble.gpu_data_size };
        return SCE_OK;
    };

    // Load a pack package from a memory buffer.
    // Return SCE_OK on success, and error otherwise.
    //   Check error_codes.h for the error's description
    int load_package(Stream& stream, Package &out_pack, LoaderOptions const& options)
    {
        size_t file_size = stream.size();
        Serialization::Preamble preamble;
        int result = check_pack(stream, preamble, file_size);
        if (result != SCE_OK)
            return result;

        Serialization::check_allocator(const_cast<PackFile::Allocator&>(options.cpu_allocator));
        Serialization::check_allocator(const_cast<PackFile::Allocator&>(options.gpu_allocator));

        // Read GPU data if present
        void* gpu_data = nullptr;
        if (preamble.gpu_data_size > 0 && (preamble.version < MAKE_VERSION(0, 4, 0)))
        {
            result = Serialization::read_gpu_data(&out_pack.gpu_data, stream, preamble.gpu_data_size, preamble.gpu_data_offset, options.gpu_allocator);
            if (result != SCE_OK)
                return result;
            out_pack.gpu_data_size = preamble.gpu_data_size;
        }

        Serialization::DeallocateOnFailure deallocate  ( &options.gpu_allocator, gpu_data );

        memset(&out_pack.header, 0, sizeof(out_pack.header));
        out_pack.header.version = preamble.version;
        
        if (preamble.version < MAKE_VERSION(0, 2, 4))
        {
            // Metadata
            stream.seek(0, Stream::SeekFrom::k_begin);
            Array<uint8_t> metadata_buffer(preamble.gpu_data_offset);
            if (metadata_buffer == nullptr)
            {
                report_error("Out of memory.\n");
                return SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
            }
            if (read_from_stream(stream, metadata_buffer.data(), preamble.gpu_data_offset) != preamble.gpu_data_offset)
            {
                report_error("Error reading from file.\n");
                return SCE_PACK_ERROR_IO;
            }
            PackFile::BufferFile buffer_file { metadata_buffer };
            int result = Serialization::load_metadata_0_2_3(buffer_file, out_pack, options.cpu_allocator, options.gpu_allocator);
            if (result != SCE_OK)
            {
                return result;
            }
            deallocate.failure = result != SCE_OK;
            return result;
        }
        // Version >= 0.2.4
        if (!stream.seek(sizeof(Serialization::Preamble), Stream::SeekFrom::k_begin))
        {
           report_error("Error reading from file.\n");
           return SCE_PACK_ERROR_IO;
        }

        Array<Serialization::Section> sections(preamble.section_count);
        if (sections == nullptr)
        {
            report_error("Out of memory.\n");
            return SCE_PACK_ERROR_INSUFFICIENT_MEMORY;
        }

        read_from_stream(stream, sections.data(), sizeof(Serialization::Section) * preamble.section_count);
        Array<uint8_t> strings_table_section;
        // Find string and data mapping sector
        if (options.resolve_names)
        {
            for (uint32_t s = 0; s < preamble.section_count; ++s)
            {
                if (sections[s].signature == k_string_header)
                {
                    int result = read_section(stream, strings_table_section, sections[s]);
                    if (result !=  SCE_OK)
                        return result;
                    break;
                }
             }
        }
        BufferFile empty_stream(nullptr, nullptr);
        SectionLoadParam load_param {
            0x00,               // These three parameters
            empty_stream,       // are set by the load_section function
            0x00,               //
            options.cpu_allocator,
            options.gpu_allocator,
            strings_table_section,
            out_pack.header.version
        };
        const size_t fixed_sections_count = 7;
        Serialization::LoaderMap loaders (fixed_sections_count + options.custom_section_loaders.getCount());
        // String and mapping are already loaded
        loaders.set_loader(k_buffer_header,   &Serialization::load_buffers,   &out_pack.buffers);
        loaders.set_loader(k_mesh_header,     &Serialization::load_meshes,    &out_pack.meshes);
        loaders.set_loader(k_material_header, &Serialization::load_materials, &out_pack.materials);
        loaders.set_loader(k_node_header,     &Serialization::load_nodes,     &out_pack.nodes);
        loaders.set_loader(k_texture_header,  &Serialization::load_textures,  &out_pack.textures);
        loaders.set_loader(k_skeleton_header, &Serialization::load_skeletons, &out_pack.skeletons);
        loaders.set_loader(k_data_mapping_header, &Serialization::load_data_mapping, &out_pack.data_mapping);
        PACK_ASSERT(loaders.count == fixed_sections_count); // If this fire is because you forget to increment the count above

        // Add the extra loader
        for (auto& l : options.custom_section_loaders)
            loaders.set_loader(l.section_header, l.loader, l.loader_data);
        for (uint32_t s = 0; s < preamble.section_count; ++s)
        {
            if (sections[s].signature == k_string_header)
            {
               continue;
            }
            auto loader = loaders.get_loader(sections[s].signature);
            if (loader != nullptr)
            {
                int result = Serialization::load_section(stream, loader->load, loader->data, sections[s], load_param);
                if (result != SCE_OK)
                    return result;
            }
            else if (options.report_warning)
               report_warning("Found Unknown section.\n");
        }
        out_pack.header.buffer_count    = (uint32_t)out_pack.buffers.getCount();
        out_pack.header.meshes_count    = (uint32_t)out_pack.meshes.getCount();
        out_pack.header.materials_count = (uint32_t)out_pack.materials.getCount();
        out_pack.header.nodes_count     = (uint32_t)out_pack.nodes.getCount();
        out_pack.header.texture_count   = (uint32_t)out_pack.textures.getCount();

        if (preamble.gpu_data_size > 0)
        {
            if (out_pack.data_mapping.size() > 0)
            {
                for (auto& m : out_pack.data_mapping)
                {
                    m.data_offset += preamble.gpu_data_offset;
                }
                result = Serialization::resolve_mapping_data(out_pack.data_mapping, stream, options.cpu_allocator, options.gpu_allocator);
                if (result != SCE_OK)
                    return result;
                out_pack.gpu_data = reinterpret_cast<void*>(out_pack.data_mapping[0].data_offset);
                out_pack.gpu_data_size = out_pack.data_mapping[0].size;
            }
            else
            {
                result = Serialization::read_gpu_data(&out_pack.gpu_data, stream, preamble.gpu_data_size, preamble.gpu_data_offset, options.gpu_allocator);
                if (result != SCE_OK)
                    return result;
                out_pack.gpu_data_size = preamble.gpu_data_size;
            }
        }

        deallocate.failure = false;
        return SCE_OK;
    }

   // Load a package from a file.
   // Return SCE_OK on success, and error otherwise.
   //   Check error_codes.h for the error's description
   int load_package(Serialization::File_t file_handle, Package& out_pack, LoaderOptions const& options)
   {
       if (!Serialization::is_valid(file_handle))
       {
           report_error("Invalid parameter, FILE must be valid.\n");
           return SCE_PACK_ERROR_INVALID_PARAM;
       }
       PackFile::Serialization::FileStream fs { file_handle };
       return load_package(fs, out_pack, options);
   }


    // Load a package from a named file.
    // Return SCE_OK on success, and error otherwise.
    //   Check error_codes.h for the error's description
    int load_package(const char *filename, Package &out_pack, LoaderOptions const& options)
    {
        if (filename == nullptr)
        {
            return SCE_PACK_ERROR_INVALID_PARAM;
        }
        Serialization::File_t file = Serialization::open_file_for_read(filename);
        if (!Serialization::is_valid(file))
        {
            report_error("Unable to open file.\n");
            return SCE_PACK_ERROR_IO;
        }

        Serialization::CloseFileOnDestruction file_closer(file);
        return load_package(file, out_pack, options);
    }
}
