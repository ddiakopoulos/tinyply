// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.

// Authored in 2015 by Dimitri Diakopoulos (http://www.dimitridiakopoulos.com)
// https://github.com/ddiakopoulos/tinyply

#ifndef tinyply_h
#define tinyply_h

#include <vector>
#include <algorithm>
#include <string>
#include <stdint.h>
#include <map>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <memory>
#include <functional>
#include <cstring>

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

    struct PlyData
    {
        Type t;
        size_t count;
        std::vector<uint8_t> data;
    };

    struct PropertyInfo 
    { 
        int stride; std::string str; 
    };

    static const std::map<Type, PropertyInfo> PropertyTable
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

    inline Type property_type_from_string(const std::string & t)
    {
        if (t == "int8" || t == "char")             return Type::INT8;
        else if (t == "uint8" || t == "uchar")      return Type::UINT8;
        else if (t == "int16" || t == "short")      return Type::INT16;
        else if (t == "uint16" || t == "ushort")    return Type::UINT16;
        else if (t == "int32" || t == "int")        return Type::INT32;
        else if (t == "uint32" || t == "uint")      return Type::UINT32;
        else if (t == "float32" || t == "float")    return Type::FLOAT32;
        else if (t == "float64" || t == "double")   return Type::FLOAT64;
        return Type::INVALID;
    }

	struct PlyProperty
	{
		PlyProperty(std::istream & is);
		PlyProperty(Type type, const std::string & _name) : propertyType(type), isList(false), name(_name) {}
		PlyProperty(Type list_type, Type prop_type, const std::string & _name, int list_count) : listType(list_type), propertyType(prop_type), isList(true), name(_name), listCount(list_count) {}
        std::string name;
        Type listType;
        Type propertyType;
		bool isList;
        int listCount;
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

        void parse_header(std::istream & is);

		void read();
		void write(std::ostream & os, bool isBinary);

        std::vector<PlyElement> & get_elements();
        std::vector<std::string> get_comments();
        std::vector<std::string> get_info();

        std::shared_ptr<PlyData> request_properties_from_element(const std::string & elementKey, const std::initializer_list<std::string> propertyKeys);
        void add_properties_to_element(const std::string & elementKey, const std::vector<std::string> & propertyKeys, std::vector<uint8_t> & source, const int listCount = 1, const Type listType = Type::INVALID);
	};

} // namesapce tinyply

#endif // tinyply_h

