// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.

// Authored in 2015 by Dimitri Diakopoulos (http://www.dimitridiakopoulos.com)
// https://github.com/ddiakopoulos/tinyply

#ifndef tinyply_h
#define tinyply_h

#include <vector>
#include <string>
#include <stdint.h>
#include <sstream>
#include <memory>

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
        uint8_t * buffer;
    };

	struct PlyProperty
	{
		PlyProperty(std::istream & is);
		PlyProperty(Type type, std::string & _name) : propertyType(type), name(_name) {}
		PlyProperty(Type list_type, Type prop_type, std::string & _name, int list_count) : listType(list_type), propertyType(prop_type), isList(true), name(_name), listCount(list_count) {}
        std::string name;
        Type propertyType;
        bool isList{ false };
        Type listType{ Type::INVALID };
        int listCount{ 0 };
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

        bool parse_header(std::istream & is);

		void read(std::istream & is);

		void write(std::ostream & os, bool isBinary);

        std::vector<PlyElement> get_elements() const;
        std::vector<std::string> get_comments() const;
        std::vector<std::string> get_info() const;

        std::shared_ptr<PlyData> request_properties_from_element(const std::string & elementKey, const std::initializer_list<std::string> propertyKeys);
        void add_properties_to_element(const std::string & elementKey, const std::initializer_list<std::string> propertyKeys, const Type type, std::vector<uint8_t> & source, const int listCount, const Type listType);
	};

} // namesapce tinyply

#endif // tinyply_h
