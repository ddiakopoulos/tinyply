// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.
// Authored in 2015 by Dimitri Diakopoulos (http://www.dimitridiakopoulos.com)
// https://github.com/ddiakopoulos/tinyply
// Version 2.1

#ifndef tinyply_h
#define tinyply_h

#include <vector>
#include <string>
#include <stdint.h>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <map>

namespace tinyply
{

    enum class Type : uint8_t
    {
        INVALID,
        INT8,
        UINT8,
        INT16,
        UINT16,
        INT32,
        UINT32,
        FLOAT32,
        FLOAT64
    };

    struct PropertyInfo
    {
        int stride;
        std::string str;
    };

    static std::map<Type, PropertyInfo> PropertyTable
    {
        { Type::INT8,{ 1, "char" } },
        { Type::UINT8,{ 1, "uchar" } },
        { Type::INT16,{ 2, "short" } },
        { Type::UINT16,{ 2, "ushort" } },
        { Type::INT32,{ 4, "int" } },
        { Type::UINT32,{ 4, "uint" } },
        { Type::FLOAT32,{ 4, "float" } },
        { Type::FLOAT64,{ 8, "double" } },
        { Type::INVALID,{ 0, "INVALID" } }
    };

    class Buffer
    {
        uint8_t * alias{ nullptr };
        struct delete_array { void operator()(uint8_t * p) { delete[] p; } };
        std::unique_ptr<uint8_t, decltype(Buffer::delete_array())> data;
        size_t size;
    public:
        Buffer() {};
        Buffer(const size_t size) : data(new uint8_t[size], delete_array()), size(size) { alias = data.get(); } // allocating
        Buffer(uint8_t * ptr) { alias = ptr; } // non-allocating, todo: set size?
        uint8_t * get() { return alias; }
        size_t size_bytes() const { return size; }
    };

    struct PlyData
    {
        Type t;
        size_t count;
        Buffer buffer;
        bool isList;
    };

    struct PlyProperty
    {
        PlyProperty(std::istream & is);
        PlyProperty(Type type, std::string & _name) : name(_name), propertyType(type) {}
        PlyProperty(Type list_type, Type prop_type, std::string & _name, size_t list_count) 
            : name(_name), propertyType(prop_type), isList(true), listType(list_type), listCount(list_count) {}
        std::string name;
        Type propertyType;
        bool isList{ false };
        Type listType{ Type::INVALID };
        size_t listCount{ 0 };
    };

    struct PlyElement
    {
        PlyElement(std::istream & istream);
        PlyElement(const std::string & _name, size_t count) : name(_name), size(count) {}
        std::string name;
        size_t size;
        std::vector<PlyProperty> properties;
    };

    struct PlyFile
    {
        struct PlyFileImpl;
        std::unique_ptr<PlyFileImpl> impl;

        PlyFile();
        ~PlyFile();

        /*
         * The ply format requires an ascii header. This can be used to determine at 
         * runtime which properties or elements exist in the file. Limited validation of the 
         * header is performed; it is assumed the header correctly reflects the contents of the 
         * payload. This function may throw. Returns true on success, false on failure. 
         */ 
        bool parse_header(std::istream & is);

        /* 
         * In the general case where |fixed_list_size| is zero, `read` performs a two-pass 
         * parse to support variable length lists. The first pass determines the size of memory
         * to allocate, while the second pass performs the read. The most general use of the 
         * ply format is storing triangle meshes. When this fact is known a-priori, we can pass 
         * an expected list length that will apply to *all* list elements in a file (e.g. 3). Doing 
         * so results in an up-front memory allocation and a single-pass import, generally a ~2x speedup. 
         */
        void read(std::istream & is, uint32_t fixed_list_size = 0);

        /* 
         * `write` performs no validation and assumes that the data passed into 
         * `add_properties_to_element` is well-formed. 
         */
        void write(std::ostream & os, bool isBinary);

        std::vector<PlyElement> get_elements() const;
        std::vector<std::string> get_info() const;
        std::vector<std::string> & get_comments();

        std::shared_ptr<PlyData> request_properties_from_element(const std::string & elementKey, 
            const std::initializer_list<std::string> propertyKeys);

        void add_properties_to_element(const std::string & elementKey, 
            const std::initializer_list<std::string> propertyKeys, 
            const Type type, 
            const size_t count, 
            uint8_t * data, 
            const Type listType, 
            const size_t listCount);
    };

} // namespace tinyply

#endif // tinyply_h
