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

namespace tinyply
{

static inline uint16_t swap_16(uint16_t value)
{
    return (uint16_t)((value >> 8) | (value << 8));
}

static inline uint32_t swap_32(uint32_t value)
{
    return (((value & 0x000000ff) << 24) |
            ((value & 0x0000ff00) <<  8) |
            ((value & 0x00ff0000) >>  8) |
            ((value & 0xff000000) >> 24));
}

static inline uint64_t swap_64(uint64_t value)
{
    return (((value & 0x00000000000000ffLL) << 56) |
            ((value & 0x000000000000ff00LL) << 40) |
            ((value & 0x0000000000ff0000LL) << 24) |
            ((value & 0x00000000ff000000LL) << 8)  |
            ((value & 0x000000ff00000000LL) >> 8)  |
            ((value & 0x0000ff0000000000LL) >> 24) |
            ((value & 0x00ff000000000000LL) >> 40) |
            ((value & 0xff00000000000000LL) >> 56));
}

struct DataCursor
{
    uint8_t * data;
    uint32_t offset;
};

typedef std::map<std::string, std::shared_ptr<DataCursor>> PropertyMapType;
    
class PlyProperty
{
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
    
    PlyProperty(std::istream& is);
    PlyProperty(Type type, const std::string & name) : propertyType(type), isList(false), name(name) {}
    PlyProperty(Type list_type, Type prop_type, const std::string & name, int stride) : listType(list_type), propertyType(prop_type), isList(true), name(name), stride(stride) {}
    
    const std::string & get_name() const { return name; }
    bool is_list() const { return isList; }
    Type get_list_type() const { return listType; }
    Type get_property_type() const { return propertyType; }
    
    int stride;
    
private:
    
    void parse_internal(std::istream & is);
    Type get_data_type(const std::string & string);
    
    Type listType, propertyType;
    bool isList;
    std::string name;
};

template<typename T>
void ply_cast(void * dest, const uint8_t * src)
{
    *(static_cast<T *>(dest)) = *(reinterpret_cast<const T *>(src));
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
    
template <typename T>
inline PlyProperty::Type property_type_for_type(std::vector<T> & theType)
{
    if (std::is_same<T, int8_t>::value)     return PlyProperty::Type::INT8;
    if (std::is_same<T, uint8_t>::value)    return PlyProperty::Type::UINT8;
    if (std::is_same<T, int16_t>::value)    return PlyProperty::Type::INT16;
    if (std::is_same<T, uint16_t>::value)   return PlyProperty::Type::UINT16;
    if (std::is_same<T, int32_t>::value)    return PlyProperty::Type::INT32;
    if (std::is_same<T, uint32_t>::value)   return PlyProperty::Type::UINT32;
    if (std::is_same<T, float>::value)      return PlyProperty::Type::FLOAT32;
    if (std::is_same<T, double>::value)     return PlyProperty::Type::FLOAT64;
    else return PlyProperty::Type::INVALID;
}
    
inline int stride_for_property(PlyProperty::Type t)
{
    switch(t)
    {
        case PlyProperty::Type::INT8:       return 1;
        case PlyProperty::Type::UINT8:      return 1;
        case PlyProperty::Type::INT16:      return 2;
        case PlyProperty::Type::UINT16:     return 2;
        case PlyProperty::Type::INT32:      return 4;
        case PlyProperty::Type::UINT32:     return 4;
        case PlyProperty::Type::FLOAT32:    return 4;
        case PlyProperty::Type::FLOAT64:    return 8;
        default: return 0;
    }
}
    
inline std::string property_type_as_string(PlyProperty::Type t)
{
    switch(t)
    {
        case PlyProperty::Type::INT8:       return "char";
        case PlyProperty::Type::UINT8:      return "uchar";
        case PlyProperty::Type::INT16:      return "short";
        case PlyProperty::Type::UINT16:     return "ushort";
        case PlyProperty::Type::INT32:      return "int";
        case PlyProperty::Type::UINT32:     return "uint";
        case PlyProperty::Type::FLOAT32:    return "float";
        case PlyProperty::Type::FLOAT64:    return "double";
        default: return "";
    }
}
    
inline void read_property(PlyProperty::Type t, void * dest, uint32_t & destOffset, const uint8_t * src, uint32_t & srcOffset)
{
    switch (t)
    {
        case PlyProperty::Type::INT8:       ply_cast<int8_t>(dest, src + srcOffset);    break;
        case PlyProperty::Type::UINT8:      ply_cast<uint8_t>(dest, src + srcOffset);   break;
        case PlyProperty::Type::INT16:      ply_cast<uint8_t>(dest, src+ srcOffset);    break;
        case PlyProperty::Type::UINT16:     ply_cast<uint16_t>(dest, src + srcOffset);  break;
        case PlyProperty::Type::INT32:      ply_cast<int32_t>(dest, src + srcOffset);   break;
        case PlyProperty::Type::UINT32:     ply_cast<uint32_t>(dest, src + srcOffset);  break;
        case PlyProperty::Type::FLOAT32:    ply_cast<float>(dest, src + srcOffset);     break;
        case PlyProperty::Type::FLOAT64:    ply_cast<double>(dest, src + srcOffset);    break;
        case PlyProperty::Type::INVALID:    throw std::invalid_argument("invalid ply property");
    }
    destOffset += stride_for_property(t);
    srcOffset += stride_for_property(t);
}

inline void read_property(PlyProperty::Type t, void * dest, uint32_t & destOffset, std::istream & is)
{
    switch (t)
    {
        case PlyProperty::Type::INT8:       *((int8_t *)dest) = ply_read_ascii<int32_t>(is);        break;
        case PlyProperty::Type::UINT8:      *((uint8_t *)dest) = ply_read_ascii<uint32_t>(is);      break;
        case PlyProperty::Type::INT16:      ply_cast_ascii<int16_t>(dest, is);                      break;
        case PlyProperty::Type::UINT16:     ply_cast_ascii<uint16_t>(dest, is);                     break;
        case PlyProperty::Type::INT32:      ply_cast_ascii<int32_t>(dest, is);                      break;
        case PlyProperty::Type::UINT32:     ply_cast_ascii<uint32_t>(dest, is);                     break;
        case PlyProperty::Type::FLOAT32:    ply_cast_ascii<float>(dest, is);                        break;
        case PlyProperty::Type::FLOAT64:    ply_cast_ascii<double>(dest, is);                       break;
        case PlyProperty::Type::INVALID:    throw std::invalid_argument("invalid ply property");
    }
    destOffset += stride_for_property(t);
}
    
inline void write_property_ascii(PlyProperty::Type t, std::ostringstream & os, uint8_t * src, uint32_t & srcOffset)
{
    switch (t)
    {
        case PlyProperty::Type::INT8:       os << *reinterpret_cast<int8_t*>(src);      break;
        case PlyProperty::Type::UINT8:      os << *reinterpret_cast<uint8_t*>(src);     break;
        case PlyProperty::Type::INT16:      os << *reinterpret_cast<int16_t*>(src);     break;
        case PlyProperty::Type::UINT16:     os << *reinterpret_cast<uint16_t*>(src);    break;
        case PlyProperty::Type::INT32:      os << *reinterpret_cast<int32_t*>(src);     break;
        case PlyProperty::Type::UINT32:     os << *reinterpret_cast<uint32_t*>(src);    break;
        case PlyProperty::Type::FLOAT32:    os << *reinterpret_cast<float*>(src);       break;
        case PlyProperty::Type::FLOAT64:    os << *reinterpret_cast<double*>(src);      break;
        case PlyProperty::Type::INVALID:    throw std::invalid_argument("invalid ply property");
    }
    os << " ";
    srcOffset += stride_for_property(t);
}
    
inline void write_property_binary(PlyProperty::Type t, std::ostringstream & os, uint8_t * src, uint32_t & srcOffset)
{
    os.write(reinterpret_cast<const char *>(src), stride_for_property(t));
    srcOffset += stride_for_property(t);
}

inline void skip_property(uint32_t & fileOffset, const PlyProperty & property, const uint8_t * src)
{
    if (property.is_list())
    {
        uint32_t listSize = 0;
        uint32_t dummyCount = 0;
        read_property(property.get_list_type(), &listSize, dummyCount, src, fileOffset);
        for (int i = 0; i < listSize; ++i) fileOffset += stride_for_property(property.get_property_type());
    } fileOffset += stride_for_property(property.get_property_type());
}
    
inline void skip_property(std::istream & is, const PlyProperty & property)
{
    std::string skip;
    if (property.is_list())
    {
        int listSize;
        is >> listSize;
        for (int i = 0; i < listSize; ++i) is >> skip;
    }
    else is >> skip;
}

class PlyElement
{
public:
    PlyElement(std::istream & istream);
    PlyElement(const std::string & name, int count) : name(name), size(count) {}
    const std::string & get_name() const { return name; }
    int get_element_count() const { return size; }
    std::vector<PlyProperty> & get_properties() { return properties; }
private:
    void parse_internal(std::istream & istream);
    std::string name;
    int size;
    std::vector<PlyProperty> properties;
};
    
inline int find_element(const std::string key, std::vector<PlyElement> & list)
{
    for (int i = 0; i < list.size(); ++i)
    {
        if (list[i].get_name() == key) return i;
    }
    return -1;
}
    
class PlyFile
{
public:
    PlyFile() {}
    PlyFile(std::istream & is);
    ~PlyFile();

    void parse(std::istream & is, const std::vector<uint8_t> & buffer);
    
    void write(std::ostringstream & os);
    void write(std::ostringstream & os, bool binary);
    
    std::vector<std::string> comments;
    std::vector<std::string> objInfo;
    
    template<typename T>
    int request_properties_from_element(std::string elementKey, std::vector<std::string> propertyKeys, std::vector<T> & source, uint32_t listCount = 1)
    {
        if (get_elements().size() == 0)
            return 0;
        
        if (find_element(elementKey, get_elements()) >= 0)
        {
            if (std::find(requestedElements.begin(), requestedElements.end(), elementKey) == requestedElements.end())
            {
                requestedElements.push_back(elementKey);
            }
        }
        else throw std::invalid_argument("requested unknown element: " + elementKey);

        auto instance_counter = [&](const std::string & prop)
        {
            for (auto e : get_elements())
                for (auto p : e.get_properties())
                    if (p.get_name() == prop) return e.get_element_count();
            return 0;
        };
        
        // Properties in the requestTable share the same cursor
        auto cursor = std::make_shared<DataCursor>();
        std::vector<uint32_t> instanceCounts;
                   
        for (auto key : propertyKeys)
        {
            if (int instanceCount = instance_counter(key))
            {
                instanceCounts.push_back(instanceCount);
                auto result = userDataMap.insert(std::pair<std::string, std::shared_ptr<DataCursor>>(key, cursor));
                if (result.second == false)
                    throw std::runtime_error("property has already been requested: " + key);
            }
            else throw std::invalid_argument("requested unknown property: " + key);
        }
        
        uint32_t totalInstanceSize = [&]() { uint32_t t = 0; for (auto c : instanceCounts) { t += c; } return t; }();
        
        if ((totalInstanceSize / propertyKeys.size()) == instanceCounts[0])
        {
            source.resize(totalInstanceSize * listCount);
            cursor->data = reinterpret_cast<uint8_t*>(source.data());
            cursor->offset = 0;
        }
        else
        {
            throw std::runtime_error("count mismatch for requested properties");
        }

        return totalInstanceSize;
    }
    
    template<typename T>
    void add_properties_to_element(std::string elementKey, std::vector<std::string> propertyKeys, std::vector<T> & source, PlyProperty::Type listType = PlyProperty::Type::INVALID, int listCount = 1)
    {
        auto cursor = std::make_shared<DataCursor>();
        cursor->data = reinterpret_cast<uint8_t *>(source.data());
        cursor->offset = 0;
        
        auto create_property_on_element = [&](PlyElement & ele)
        {
            for (auto key : propertyKeys)
            {
                PlyProperty::Type t = property_type_for_type(source);
                PlyProperty newProp = (listType == PlyProperty::Type::INVALID) ? PlyProperty(t, key) : PlyProperty(listType, t, key, listCount);
                userDataMap.insert(std::pair<std::string, std::shared_ptr<DataCursor>>(key, cursor));
                ele.get_properties().push_back(newProp);
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
            PlyElement newElement = listCount == 1 ? PlyElement(elementKey, (int) source.size() / (int) propertyKeys.size()) : PlyElement(elementKey, (int) source.size() / listCount);
            create_property_on_element(newElement);
            elements.push_back(newElement);
        }
    }
    
    std::vector<PlyElement> & get_elements() { return elements; }
    
private:
        
    bool parse_header(std::istream & is);
    void parse_data_binary(std::istream & is, const std::vector<uint8_t> & buffer);
    void parse_data_ascii(std::istream & is, const std::vector<uint8_t> & buffer);
    
    void read_header_format(std::istream & is);
    void read_header_element(std::istream & is);
    void read_header_property(std::istream & is);
    void read_header_text(std::string line, std::istream & is, std::vector<std::string> place, int erase = 0);
    
    void write_header(std::ostringstream & os);
    
    std::vector<PlyElement> elements;
    bool isBinary = false;
    bool isBigEndian = false;
    PropertyMapType userDataMap;
    std::vector<std::string> requestedElements;
};

} // tinyply

#endif // tinyply_h
