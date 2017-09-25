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

	template<typename T> T endian_swap(const T & v) { return v; }
	template<> inline uint16_t endian_swap(const uint16_t & v) { return (v << 8) | (v >> 8); }
	template<> inline uint32_t endian_swap(const uint32_t & v) { return (v << 24) | ((v << 8) & 0x00ff0000) | ((v >> 8) & 0x0000ff00) | (v >> 24); }
	template<> inline uint64_t endian_swap(const uint64_t & v)
	{
		return (((v & 0x00000000000000ffLL) << 56) |
			((v & 0x000000000000ff00LL) << 40) |
			((v & 0x0000000000ff0000LL) << 24) |
			((v & 0x00000000ff000000LL) << 8) |
			((v & 0x000000ff00000000LL) >> 8) |
			((v & 0x0000ff0000000000LL) >> 24) |
			((v & 0x00ff000000000000LL) >> 40) |
			((v & 0xff00000000000000LL) >> 56));
	}
	template<> inline int16_t endian_swap(const int16_t & v) { uint16_t r = endian_swap(*(uint16_t*)&v); return *(int16_t*)&r; }
	template<> inline int32_t endian_swap(const int32_t & v) { uint32_t r = endian_swap(*(uint32_t*)&v); return *(int32_t*)&r; }
	template<> inline int64_t endian_swap(const int64_t & v) { uint64_t r = endian_swap(*(uint64_t*)&v); return *(int64_t*)&r; }
	inline float endian_swap_float(const uint32_t & v) { union {float f; uint32_t i;}; i = endian_swap(v); return f; }
	inline double endian_swap_double(const uint64_t & v) { union {double d; uint64_t i;}; i = endian_swap(v); return d; }

	class PlyProperty
	{
		void parse_internal(std::istream & is);
	public:

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

		PlyProperty(std::istream & is);
		PlyProperty(Type type, const std::string & _name) : propertyType(type), isList(false), name(_name) {}
		PlyProperty(Type list_type, Type prop_type, const std::string & _name, int list_count) : listType(list_type), propertyType(prop_type), isList(true), name(_name), listCount(list_count) {}

		Type listType, propertyType;
		bool isList;
		std::string name;
		int listCount = 0;
	};

    struct ParsedData
    {
        std::vector<uint8_t> data;
        size_t byteOffset;
        size_t sizeBytes;
        size_t count;
        bool valid;
    };

    inline std::string make_key(const std::string & a, const std::string & b)
    {
        return (a + "-" + b);
    }

	template<typename T>
	void ply_cast(void * dest, const char * src, bool be)
	{
		*(static_cast<T *>(dest)) = (be) ? endian_swap(*(reinterpret_cast<const T *>(src))) : *(reinterpret_cast<const T *>(src));
	}

	template<typename T>
	void ply_cast_float(void * dest, const char * src, bool be)
	{
		*(static_cast<T *>(dest)) = (be) ? endian_swap_float(*(reinterpret_cast<const uint32_t *>(src))) : *(reinterpret_cast<const T *>(src));
	}

	template<typename T>
	void ply_cast_double(void * dest, const char * src, bool be)
	{
		*(static_cast<T *>(dest)) = (be) ? endian_swap_double(*(reinterpret_cast<const uint64_t *>(src))) : *(reinterpret_cast<const T *>(src));
	}

	template<typename T>
	T ply_read_ascii(std::istream & is)
	{
		T data;
		is >> data;
		return data;
	}

	template<typename T>
	void ply_cast_ascii(void * dest, std::istream & is)
	{
		*(static_cast<T *>(dest)) = ply_read_ascii<T>(is);
	}

	struct PropertyInfo { int stride; std::string str; };
	static std::map<PlyProperty::Type, PropertyInfo> PropertyTable
	{
		{ PlyProperty::Type::INT8,{ 1, "char" } },
		{ PlyProperty::Type::UINT8,{ 1, "uchar" } },
		{ PlyProperty::Type::INT16,{ 2, "short" } },
		{ PlyProperty::Type::UINT16,{ 2, "ushort" } },
		{ PlyProperty::Type::INT32,{ 4, "int" } },
		{ PlyProperty::Type::UINT32,{ 4, "uint" } },
		{ PlyProperty::Type::FLOAT32,{ 4, "float" } },
		{ PlyProperty::Type::FLOAT64,{ 8, "double" } },
		{ PlyProperty::Type::INVALID,{ 0, "INVALID" } }
	};

	inline PlyProperty::Type property_type_from_string(const std::string & t)
	{
		if (t == "int8" || t == "char")             return PlyProperty::Type::INT8;
		else if (t == "uint8" || t == "uchar")      return PlyProperty::Type::UINT8;
		else if (t == "int16" || t == "short")      return PlyProperty::Type::INT16;
		else if (t == "uint16" || t == "ushort")    return PlyProperty::Type::UINT16;
		else if (t == "int32" || t == "int")        return PlyProperty::Type::INT32;
		else if (t == "uint32" || t == "uint")      return PlyProperty::Type::UINT32;
		else if (t == "float32" || t == "float")    return PlyProperty::Type::FLOAT32;
		else if (t == "float64" || t == "double")   return PlyProperty::Type::FLOAT64;
		return PlyProperty::Type::INVALID;
	}

	class PlyElement
	{
		void parse_internal(std::istream & is);
	public:
		PlyElement(std::istream & istream);
		PlyElement(const std::string & _name, size_t count) : name(_name), size(count) {}
		std::string name;
		size_t size;
		std::vector<PlyProperty> properties;
	};

	inline int find_element(const std::string & key, const std::vector<PlyElement> & list)
	{
        for (size_t i = 0; i < list.size(); i++) if (list[i].name == key) return i;
		return -1;
	}

    inline int find_property(const std::string & key, const std::vector<PlyProperty> & list)
    {
        for (size_t i = 0; i < list.size(); ++i) if (list[i].name == key) return i;
        return -1;
    }

	class PlyFile
	{

        std::istream & is;

	public:

		PlyFile(std::istream & is);

		void read();
		void write(std::ostream & os, bool isBinary);

		std::vector<PlyElement> & get_elements() { return elements; }

		std::vector<std::string> comments;
		std::vector<std::string> objInfo;

        // Returns the size (in bytes)
        std::shared_ptr<ParsedData> request_properties_from_element(const std::string & elementKey, const std::initializer_list<std::string> propertyKeys)
		{
            // All requested properties in the userDataTable share the same cursor (thrown into the same flat array)
            std::shared_ptr<ParsedData> cursor = std::make_shared<ParsedData>();
            cursor->byteOffset = 0;
            cursor->sizeBytes = 0;
            cursor->valid = false;

            if (get_elements().size() == 0) throw std::runtime_error("parsed header had no elements defined. malformed file?");
            if (!propertyKeys.size()) throw std::invalid_argument("`propertyKeys` argument is empty");
            if (elementKey.size() == 0) throw std::invalid_argument("`elementKey` argument is empty");

            const int elementIndex = find_element(elementKey, get_elements());

            // Sanity check if the user requested element is in the pre-parsed header
			if (elementIndex >= 0)
			{
                // We found the element
                const PlyElement & element = get_elements()[elementIndex];

                cursor->count = element.size;

                // Find each of the keys
                for (auto key : propertyKeys)
                {
                    const int propertyIndex = find_property(key, element.properties);

                    if (propertyIndex >= 0)
                    {
                        // We found the property
                        const PlyProperty & property = element.properties[propertyIndex];

                        auto result = userDataTable.insert(std::pair<std::string, std::shared_ptr<ParsedData>>(make_key(element.name, property.name), cursor));
                        if (result.second == false) throw std::invalid_argument("element-property key has already been requested: " + make_key(element.name, property.name));

                        cursor->valid = true;
                    }
                    else throw std::invalid_argument("one of the property keys was not found in the header: " + key);
                }
			}
			else throw std::invalid_argument("`elementKey` was not found in the header");

            return cursor;
		}

        /*
		template<typename T>
		void add_properties_to_element(const std::string & elementKey, const std::vector<std::string> & propertyKeys, std::vector<T> & source, const int listCount = 1, const PlyProperty::Type listType = PlyProperty::Type::INVALID)
		{
			auto cursor = std::make_shared<DataCursor>();
			cursor->offset = 0;
			cursor->vector = &source;
			cursor->data = reinterpret_cast<uint8_t *>(source.data());

			auto create_property_on_element = [&](PlyElement & e)
			{
				for (auto key : propertyKeys)
				{
					PlyProperty::Type t = property_type_for_type(source);
					PlyProperty newProp = (listType == PlyProperty::Type::INVALID) ? PlyProperty(t, key) : PlyProperty(listType, t, key, listCount);
					userDataTable.insert(std::pair<std::string, std::shared_ptr<DataCursor>>(make_key(e.name, key), cursor));
					e.properties.push_back(newProp);
				}
			};

			int idx = find_element(elementKey, elements);
			if (idx >= 0)
			{
				PlyElement & e = elements[idx];
				create_property_on_element(e);
			}
			else
			{
				PlyElement newElement = (listCount == 1) ? PlyElement(elementKey, source.size() / propertyKeys.size()) : PlyElement(elementKey, source.size() / listCount);
				create_property_on_element(newElement);
				elements.push_back(newElement);
			}
		}
        */

	private:

		size_t skip_property_binary(const PlyProperty & property, std::istream & is);
		size_t skip_property_ascii(const PlyProperty & property, std::istream & is);

		size_t read_property_binary(const PlyProperty & p, void * dest, size_t & destOffset, std::istream & is);
        size_t read_property_ascii(const PlyProperty & p, void * dest, size_t & destOffset, std::istream & is);
		void write_property_ascii(PlyProperty::Type t, std::ostream & os, uint8_t * src, size_t & srcOffset);
		void write_property_binary(PlyProperty::Type t, std::ostream & os, uint8_t * src, size_t & srcOffset);

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

		std::map<std::string, std::shared_ptr<ParsedData>> userDataTable;

		std::vector<PlyElement> elements;
	};

} // namesapce tinyply

#endif // tinyply_h

