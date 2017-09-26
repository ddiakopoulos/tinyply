// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.

// Authored in 2015 by Dimitri Diakopoulos (http://www.dimitridiakopoulos.com)
// https://github.com/ddiakopoulos/tinyply

#include "tinyply.h"

using namespace tinyply;
using namespace std;

//////////////////
// Endian Utils //
//////////////////

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
inline float endian_swap_float(const uint32_t & v) { union { float f; uint32_t i; }; i = endian_swap(v); return f; }
inline double endian_swap_double(const uint64_t & v) { union { double d; uint64_t i; }; i = endian_swap(v); return d; }

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


//////////////////
// PLY Property //
//////////////////

struct PlyCursor
{
    size_t byteOffset;
    size_t sizeBytes;
};

PlyProperty::PlyProperty(std::istream & is) : isList(false)
{
    parse_internal(is);
}

void PlyProperty::parse_internal(std::istream & is)
{
    string type;
    is >> type;
    if (type == "list")
    {
        string countType;
        is >> countType >> type;
        listType = property_type_from_string(countType);
        isList = true;
    }
    propertyType = property_type_from_string(type);
    is >> name;
}

/////////////////
// PLY Element //
/////////////////

PlyElement::PlyElement(std::istream & is)
{
    parse_internal(is);
}

void PlyElement::parse_internal(std::istream & is)
{
    is >> name >> size;
}

///////////
// Utils //
///////////

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

size_t find_element(const std::string & key, const std::vector<PlyElement> & list)
{
    for (size_t i = 0; i < list.size(); i++) if (list[i].name == key) return i;
    return -1;
}

size_t find_property(const std::string & key, const std::vector<PlyProperty> & list)
{
    for (size_t i = 0; i < list.size(); ++i) if (list[i].name == key) return i;
    return -1;
}

//////////////
// PLY File //
//////////////

PlyFile::PlyFile(std::istream & is) : is(is)
{
    if (!parse_header(is))
    {
        throw std::runtime_error("file is not ply or encounted junk in header");
    }
}

bool PlyFile::parse_header(std::istream & is)
{
    std::string line;
    while (std::getline(is, line))
    {
        std::istringstream ls(line);
        std::string token;
        ls >> token;
        if (token == "ply" || token == "PLY" || token == "")
        {
            continue;
        }
        else if (token == "comment")    read_header_text(line, ls, comments, 8);
        else if (token == "format")     read_header_format(ls);
        else if (token == "element")    read_header_element(ls);
        else if (token == "property")   read_header_property(ls);
        else if (token == "obj_info")   read_header_text(line, ls, objInfo, 9);
        else if (token == "end_header") break;
        else return false;
    }
    return true;
}

void PlyFile::read_header_text(std::string line, std::istream & is, std::vector<std::string>& place, int erase)
{
    place.push_back((erase > 0) ? line.erase(0, erase) : line);
}

void PlyFile::read_header_format(std::istream & is)
{
    std::string s;
    (is >> s);
	if (s == "binary_little_endian") isBinary = true;
	else if (s == "binary_big_endian") isBinary = isBigEndian = true;
}

void PlyFile::read_header_element(std::istream & is)
{
    get_elements().emplace_back(is);
}

void PlyFile::read_header_property(std::istream & is)
{
    get_elements().back().properties.emplace_back(is);
}

size_t PlyFile::skip_property_binary(const PlyProperty & p, std::istream & is)
{
    static std::vector<char> skip(PropertyTable[p.propertyType].stride);
    if (p.isList)
    {
		size_t listSize = 0;
		size_t dummyCount = 0;
        read_property_binary(p, &listSize, dummyCount, is);
        for (size_t i = 0; i < listSize; ++i) is.read(skip.data(), PropertyTable[p.propertyType].stride);
        return listSize * PropertyTable[p.propertyType].stride; // in bytes
    }
    else
    {
        is.read(skip.data(), PropertyTable[p.propertyType].stride);
        return PropertyTable[p.propertyType].stride;
    }
}

size_t PlyFile::skip_property_ascii(const PlyProperty & p, std::istream & is)
{
    std::string skip;
    if (p.isList)
    {
        size_t listSize = 0;
        size_t dummyCount = 0;
        read_property_ascii(p, &listSize, dummyCount, is);
        for (size_t i = 0; i < listSize; ++i) is >> skip;
        return listSize * PropertyTable[p.propertyType].stride; // in bytes
    }
    else
    {
        is >> skip;
        return PropertyTable[p.propertyType].stride;
    }
}

size_t PlyFile::read_property_binary(const PlyProperty & p, void * dest, size_t & destOffset, std::istream & is)
{
    const Type t = (p.isList) ? p.listType : p.propertyType;

    destOffset += PropertyTable[t].stride;

    std::vector<char> src(PropertyTable[t].stride);
    is.read(src.data(), PropertyTable[t].stride);

    switch (t)
    {
        case Type::INT8:       ply_cast<int8_t>(dest, src.data(), isBigEndian);        break;
        case Type::UINT8:      ply_cast<uint8_t>(dest, src.data(), isBigEndian);       break;
        case Type::INT16:      ply_cast<int16_t>(dest, src.data(), isBigEndian);       break;
        case Type::UINT16:     ply_cast<uint16_t>(dest, src.data(), isBigEndian);      break;
        case Type::INT32:      ply_cast<int32_t>(dest, src.data(), isBigEndian);       break;
        case Type::UINT32:     ply_cast<uint32_t>(dest, src.data(), isBigEndian);      break;
        case Type::FLOAT32:    ply_cast_float<float>(dest, src.data(), isBigEndian);   break;
        case Type::FLOAT64:    ply_cast_double<double>(dest, src.data(), isBigEndian); break;
        case Type::INVALID:    throw std::invalid_argument("invalid ply property");
    }

    return PropertyTable[t].stride;
}

size_t PlyFile::read_property_ascii(const PlyProperty & p, void * dest, size_t & destOffset, std::istream & is)
{         
    const Type t = (p.isList) ? p.listType : p.propertyType;

    destOffset += PropertyTable[t].stride;

    switch (t)
    {
        case Type::INT8:       *((int8_t *)dest) = ply_read_ascii<int32_t>(is);        break;
        case Type::UINT8:      *((uint8_t *)dest) = ply_read_ascii<uint32_t>(is);      break;
        case Type::INT16:      ply_cast_ascii<int16_t>(dest, is);                      break;
        case Type::UINT16:     ply_cast_ascii<uint16_t>(dest, is);                     break;
        case Type::INT32:      ply_cast_ascii<int32_t>(dest, is);                      break;
        case Type::UINT32:     ply_cast_ascii<uint32_t>(dest, is);                     break;
        case Type::FLOAT32:    ply_cast_ascii<float>(dest, is);                        break;
        case Type::FLOAT64:    ply_cast_ascii<double>(dest, is);                       break;
        case Type::INVALID:    throw std::invalid_argument("invalid ply property");
    }
    return PropertyTable[t].stride;
}

void PlyFile::write_property_ascii(Type t, std::ostream & os, uint8_t * src, size_t & srcOffset)
{
    switch (t)
    {
        case Type::INT8:       os << static_cast<int32_t>(*reinterpret_cast<int8_t*>(src));    break;
        case Type::UINT8:      os << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(src));  break;
        case Type::INT16:      os << *reinterpret_cast<int16_t*>(src);     break;
        case Type::UINT16:     os << *reinterpret_cast<uint16_t*>(src);    break;
        case Type::INT32:      os << *reinterpret_cast<int32_t*>(src);     break;
        case Type::UINT32:     os << *reinterpret_cast<uint32_t*>(src);    break;
        case Type::FLOAT32:    os << *reinterpret_cast<float*>(src);       break;
        case Type::FLOAT64:    os << *reinterpret_cast<double*>(src);      break;
        case Type::INVALID:    throw std::invalid_argument("invalid ply property");
    }
    os << " ";
    srcOffset += PropertyTable[t].stride;
}

void PlyFile::write_property_binary(Type t, std::ostream & os, uint8_t * src, size_t & srcOffset)
{
    os.write((char *)src, PropertyTable[t].stride);
    srcOffset += PropertyTable[t].stride;
}

void PlyFile::read()
{
    // Parse but only get the data size
    read_internal(true);

    std::vector<std::shared_ptr<PlyData>> uniqueCursors;
    for (auto & key : userDataTable) uniqueCursors.push_back(key.second);

    // Since group-requested properties share the same cursor, we need to find unique cursors so we only allocate once
    std::sort(uniqueCursors.begin(), uniqueCursors.end());
    uniqueCursors.erase(std::unique(uniqueCursors.begin(), uniqueCursors.end()), uniqueCursors.end());
    for (auto & cursor : uniqueCursors) cursor->data.resize(cursor->sizeBytes);

    // Populate the data
    read_internal(false);
}

void PlyFile::write(std::ostream & os, bool _isBinary)
{
    if (_isBinary) write_binary_internal(os);
    else write_ascii_internal(os);
}

void PlyFile::write_binary_internal(std::ostream & os)
{
    isBinary = true;
    write_header(os);

    for (auto & e : elements)
    {
        for (size_t i = 0; i < e.size; ++i)
        {
            for (auto & p : e.properties)
            {
                auto & cursor = userDataTable[make_key(e.name, p.name)];
                if (p.isList)
                {
                    uint8_t listSize[4] = {0, 0, 0, 0};
                    memcpy(listSize, &p.listCount, sizeof(uint32_t));
					size_t dummyCount = 0;
                    write_property_binary(p.listType, os, listSize, dummyCount);
                    for (int j = 0; j < p.listCount; ++j)
                    {
                        write_property_binary(p.propertyType, os, (cursor->data.data() + cursor->byteOffset), cursor->byteOffset);
                    }
                }
                else
                {
                    write_property_binary(p.propertyType, os, (cursor->data.data() + cursor->byteOffset), cursor->byteOffset);
                }
            }
        }
    }
}

void PlyFile::write_ascii_internal(std::ostream & os)
{
    write_header(os);

    for (auto & e : elements)
    {
        for (size_t i = 0; i < e.size; ++i)
        {
            for (auto & p : e.properties)
            {
                auto & cursor = userDataTable[make_key(e.name, p.name)];
                if (p.isList)
                {
                    os << p.listCount << " ";
                    for (int j = 0; j < p.listCount; ++j)
                    {
                        write_property_ascii(p.propertyType, os, (cursor->data.data() + cursor->byteOffset), cursor->byteOffset);
                    }
                }
                else
                {
                    write_property_ascii(p.propertyType, os, (cursor->data.data() + cursor->byteOffset), cursor->byteOffset);
                }
            }
            os << "\n";
        }
    }
}

void PlyFile::write_header(std::ostream & os)
{
    const std::locale & fixLoc = std::locale("C");
    os.imbue(fixLoc);

    os << "ply\n";
    if (isBinary) os << ((isBigEndian) ? "format binary_big_endian 1.0" : "format binary_little_endian 1.0") << "\n";
    else os << "format ascii 1.0\n";

    for (const auto & comment : comments) os << "comment " << comment << "\n";

    for (auto & e : elements)
    {
        os << "element " << e.name << " " << e.size << "\n";
        for (const auto & p : e.properties)
        {
            if (p.isList)
            {
                os << "property list " << PropertyTable[p.listType].str << " "
                << PropertyTable[p.propertyType].str << " " << p.name << "\n";
            }
            else
            {
                os << "property " << PropertyTable[p.propertyType].str << " " << p.name << "\n";
            }
        }
    }
    os << "end_header\n";
}

// Returns the size (in bytes)
std::shared_ptr<PlyData> PlyFile::request_properties_from_element(const std::string & elementKey, const std::initializer_list<std::string> propertyKeys)
{
    // All requested properties in the userDataTable share the same cursor (thrown into the same flat array)
    std::shared_ptr<PlyData> cursor = std::make_shared<PlyData>();
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

                auto result = userDataTable.insert(std::pair<std::string, std::shared_ptr<PlyData>>(make_key(element.name, property.name), cursor));
                if (result.second == false) throw std::invalid_argument("element-property key has already been requested: " + make_key(element.name, property.name));

                cursor->valid = true;
            }
            else throw std::invalid_argument("one of the property keys was not found in the header: " + key);
        }
    }
    else throw std::invalid_argument("the element key was not found in the header: " + elementKey);

    return cursor;
}

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

void PlyFile::read_internal(bool firstPass)
{
    std::function<size_t(const PlyProperty & p, void * dest, size_t & destOffset, std::istream & is)> read;
    std::function<size_t(const PlyProperty & p, std::istream & is)> skip;
    
    const auto start = is.tellg();

    if (isBinary)
    {
        read = [&](const PlyProperty & p, void * dest, size_t & destOffset, std::istream & _is) { return read_property_binary(p, dest, destOffset, _is); };
        skip = [&](const PlyProperty & p, std::istream & _is) { return skip_property_binary(p, _is); };
    }
    else
    {
        read = [&](const PlyProperty & p, void * dest, size_t & destOffset, std::istream & _is) { return read_property_ascii(p, dest, destOffset, _is); };
        skip = [&](const PlyProperty & p, std::istream & _is) { return skip_property_ascii(p, _is); };
    }

    for (auto & element : get_elements())
    {
        for (size_t count = 0; count < element.size; ++count)
        {
            for (auto & property : element.properties)
            {
                auto cursorIt = userDataTable.find(make_key(element.name, property.name));
                if (cursorIt != userDataTable.end())
                {
                    auto & cursor = cursorIt->second;
                    if (!firstPass)
                    {
                        if (property.isList)
                        {
                            size_t listSize = 0;
                            size_t dummyCount = 0;
                            read(property, &listSize, dummyCount, is);
                            for (size_t i = 0; i < listSize; ++i)
                            {
                                read(property, (cursor->data.data() + cursor->byteOffset), cursor->byteOffset, is);
                            }
                        }
                        else
                        {
                            read(property, (cursor->data.data() + cursor->byteOffset), cursor->byteOffset, is);
                        }
                    }
                    else
                    {
                        cursor->sizeBytes += skip(property, is);
                    }
                }
                else
                {
                    skip(property, is);
                }
            }
        }
    }

    // Reset istream reader to the beginning
    if (firstPass) is.seekg(start, is.beg);
}

