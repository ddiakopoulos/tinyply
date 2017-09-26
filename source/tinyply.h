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
    struct PlyData
    {
        std::vector<uint8_t> data;
        size_t count;
    };

    struct PlyCursor
    {
        size_t byteOffset;
        size_t sizeBytes;
    };

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

	struct PlyProperty
	{

		PlyProperty(std::istream & is);
		PlyProperty(Type type, const std::string & _name) : propertyType(type), isList(false), name(_name) {}
		PlyProperty(Type list_type, Type prop_type, const std::string & _name, int list_count) : listType(list_type), propertyType(prop_type), isList(true), name(_name), listCount(list_count) {}

		Type listType, propertyType;
		bool isList;
		std::string name;
		int listCount = 0;
	};

	struct PlyElement
	{
		PlyElement(std::istream & istream);
		PlyElement(const std::string & _name, size_t count) : name(_name), size(count) {}
		std::string name;
		size_t size;
		std::vector<PlyProperty> properties;
	};

	class PlyFile
	{
        std::istream & is;

        struct PlyFileImpl;
        std::unique_ptr<PlyFileImpl> impl;

	public:

		PlyFile(std::istream & is);

		void read();
		void write(std::ostream & os, bool isBinary);

        std::vector<PlyElement> & get_elements();

		std::vector<std::string> comments;
		std::vector<std::string> objInfo;

        std::shared_ptr<PlyData> request_properties_from_element(const std::string & elementKey, const std::initializer_list<std::string> propertyKeys);

        void add_properties_to_element(const std::string & elementKey, const std::vector<std::string> & propertyKeys, std::vector<uint8_t> & source, const int listCount = 1, const Type listType = Type::INVALID);

	private:

		size_t skip_property_binary(const PlyProperty & property, std::istream & is);
		size_t skip_property_ascii(const PlyProperty & property, std::istream & is);

		size_t read_property_binary(const PlyProperty & p, void * dest, size_t & destOffset, std::istream & is);
        size_t read_property_ascii(const PlyProperty & p, void * dest, size_t & destOffset, std::istream & is);

		void write_property_ascii(Type t, std::ostream & os, uint8_t * src, size_t & srcOffset);
		void write_property_binary(Type t, std::ostream & os, uint8_t * src, size_t & srcOffset);

		bool parse_header(std::istream & is);
		void write_header(std::ostream & os);

		void read_header_format(std::istream & is);
		void read_header_element(std::istream & is);
		void read_header_property(std::istream & is);
		void read_header_text(std::string line, std::istream & is, std::vector<std::string> & place, int erase = 0);

		void read_internal(bool firstPass = false);

		void write_ascii_internal(std::ostream & os);
		void write_binary_internal(std::ostream & os);

		bool isBinary = false;
		bool isBigEndian = false;

		std::map<std::string, std::shared_ptr<PlyData>> userDataTable;

		std::vector<PlyElement> elements;
	};

} // namesapce tinyply

#endif // tinyply_h

