// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.

// Authored in 2015 by Dimitri Diakopoulos (http://www.dimitridiakopoulos.com)
// https://github.com/ddiakopoulos/tinyply

#include "tinyply.h"

using namespace tinyply;
using namespace std;

//////////////////
// PLY Property //
//////////////////

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
    const PlyProperty::Type t = (p.isList) ? p.listType : p.propertyType;

    destOffset += PropertyTable[t].stride;

    std::vector<char> src(PropertyTable[t].stride);
    is.read(src.data(), PropertyTable[t].stride);

    switch (t)
    {
        case PlyProperty::Type::INT8:       ply_cast<int8_t>(dest, src.data(), isBigEndian);        break;
        case PlyProperty::Type::UINT8:      ply_cast<uint8_t>(dest, src.data(), isBigEndian);       break;
        case PlyProperty::Type::INT16:      ply_cast<int16_t>(dest, src.data(), isBigEndian);       break;
        case PlyProperty::Type::UINT16:     ply_cast<uint16_t>(dest, src.data(), isBigEndian);      break;
        case PlyProperty::Type::INT32:      ply_cast<int32_t>(dest, src.data(), isBigEndian);       break;
        case PlyProperty::Type::UINT32:     ply_cast<uint32_t>(dest, src.data(), isBigEndian);      break;
        case PlyProperty::Type::FLOAT32:    ply_cast_float<float>(dest, src.data(), isBigEndian);   break;
        case PlyProperty::Type::FLOAT64:    ply_cast_double<double>(dest, src.data(), isBigEndian); break;
        case PlyProperty::Type::INVALID:    throw std::invalid_argument("invalid ply property");
    }

    return PropertyTable[t].stride;
}

size_t PlyFile::read_property_ascii(const PlyProperty & p, void * dest, size_t & destOffset, std::istream & is)
{
    const PlyProperty::Type t = (p.isList) ? p.listType : p.propertyType;

    destOffset += PropertyTable[t].stride;

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
    return PropertyTable[t].stride;
}

void PlyFile::write_property_ascii(PlyProperty::Type t, std::ostream & os, uint8_t * src, size_t & srcOffset)
{
    switch (t)
    {
        case PlyProperty::Type::INT8:       os << static_cast<int32_t>(*reinterpret_cast<int8_t*>(src));    break;
        case PlyProperty::Type::UINT8:      os << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(src));  break;
        case PlyProperty::Type::INT16:      os << *reinterpret_cast<int16_t*>(src);     break;
        case PlyProperty::Type::UINT16:     os << *reinterpret_cast<uint16_t*>(src);    break;
        case PlyProperty::Type::INT32:      os << *reinterpret_cast<int32_t*>(src);     break;
        case PlyProperty::Type::UINT32:     os << *reinterpret_cast<uint32_t*>(src);    break;
        case PlyProperty::Type::FLOAT32:    os << *reinterpret_cast<float*>(src);       break;
        case PlyProperty::Type::FLOAT64:    os << *reinterpret_cast<double*>(src);      break;
        case PlyProperty::Type::INVALID:    throw std::invalid_argument("invalid ply property");
    }
    os << " ";
    srcOffset += PropertyTable[t].stride;
}

void PlyFile::write_property_binary(PlyProperty::Type t, std::ostream & os, uint8_t * src, size_t & srcOffset)
{
    os.write((char *)src, PropertyTable[t].stride);
    srcOffset += PropertyTable[t].stride;
}

void PlyFile::read()
{
    // Parse but only get the data size
    read_internal(true);

    std::vector<std::shared_ptr<ParsedData>> uniqueCursors;
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

