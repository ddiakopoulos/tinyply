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
        listType = get_data_type(countType);
        isList = true;
    }
    propertyType = get_data_type(type);
    is >> name;
}

PlyProperty::Type PlyProperty::get_data_type(const string & t)
{
    if      (t == "int8"    || t == "char")     return PlyProperty::Type::INT8;
    else if (t == "uint8"   || t == "uchar")    return PlyProperty::Type::UINT8;
    else if (t == "int16"   || t == "short")    return PlyProperty::Type::INT16;
    else if (t == "uint16"  || t == "ushort")   return PlyProperty::Type::UINT16;
    else if (t == "int32"   || t == "int")      return PlyProperty::Type::INT32;
    else if (t == "uint32"  || t == "uint")     return PlyProperty::Type::UINT32;
    else if (t == "float32" || t == "float")    return PlyProperty::Type::FLOAT32;
    else if (t == "float64" || t == "double")   return PlyProperty::Type::FLOAT64;
    return PlyProperty::Type::INVALID;
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

PlyFile::PlyFile(std::istream & is)
{
    if (!parse_header(is))
    {
        throw std::runtime_error("file is not ply or encounted junk in header");
    }
}

PlyFile::~PlyFile()
{
    
}

bool PlyFile::parse_header(std::istream& is)
{
    std::string line;
    bool gotMagic = false;
    while (std::getline(is, line))
    {
        std::istringstream ls(line);
        std::string token;
        ls >> token;
        if (token == "ply" || token == "PLY" || token == "")
        {
            gotMagic = true;
            continue;
        }
        else if (token == "comment")
            read_header_text(line, ls, comments, 7);
        else if (token == "format")
            read_header_format(ls);
        else if (token == "element")
            read_header_element(ls);
        else if (token == "property")
            read_header_property(ls);
        else if (token == "obj_info")
            read_header_text(line, ls, objInfo, 7);
        else if (token == "end_header")
            break;
        else
            return false;
    }
    return true;
}

void PlyFile::read_header_text(std::string line, std::istream & is, std::vector<std::string> place, int erase)
{
    place.push_back((erase > 0) ? line.erase(0, erase) : line);
}

void PlyFile::read_header_format(std::istream & is)
{
    std::string s;
    (is >> s);
    if (s == "ascii" || s == "ASCII")
        isBinary = false;
    else if (s == "binary_little_endian" || s == "binary_big_endian")
        isBinary = true;
}

void PlyFile::read_header_element(std::istream & is)
{
    PlyElement e(is);
    get_elements().push_back(e);
}

void PlyFile::read_header_property(std::istream & is)
{
    PlyProperty e(is);
    get_elements().back().properties.push_back(e);
}

void PlyFile::parse(std::istream & is, const std::vector<uint8_t> & buffer)
{
    if (isBinary) parse_data_binary(is, buffer);
    else parse_data_ascii(is, buffer);
}

void PlyFile::write(std::ostringstream & os, bool binary)
{
    isBinary = true;
    write_header(os);
    
    for (auto & e : elements)
    {
        for (int i = 0; i < e.size; ++i)
        {
            for (auto & p : e.properties)
            {
                auto & cursor = userDataTable[p.name];
                if (p.isList)
                {
                    uint8_t listSize[4] = {0, 0, 0, 0};
                    memcpy(listSize, &p.listCount, sizeof(uint32_t));
                    uint32_t dummyCount = 0;
                    write_property_binary(p.listType, os, listSize, dummyCount);
                    for (int j = 0; j < p.listCount; ++j)
                    {
                        write_property_binary(p.propertyType, os, (cursor->data + cursor->offset), cursor->offset);
                    }
                }
                else
                {
                    write_property_binary(p.propertyType, os, (cursor->data + cursor->offset), cursor->offset);
                }
            }
        }
    }
}

void PlyFile::write(std::ostringstream & os)
{
    write_header(os);
    
    for (auto & e : elements)
    {
        for (int i = 0; i < e.size; ++i)
        {
            for (auto & p : e.properties)
            {
                auto & cursor = userDataTable[p.name];
                if (p.isList)
                {
                    os << p.listCount << " ";
                    for (int j = 0; j < p.listCount; ++j)
                    {
                        write_property_ascii(p.propertyType, os, (cursor->data + cursor->offset), cursor->offset);
                    }
                }
                else
                {
                    write_property_ascii(p.propertyType, os, (cursor->data + cursor->offset), cursor->offset);
                }
            }
            os << std::endl;
        }
    }
}

void PlyFile::write_header(std::ostringstream & os)
{
    const std::locale & fixLoc = std::locale("C");
    os.imbue(fixLoc);
    
    os << "ply" << std::endl;
    if (isBinary)
        os << ((isBigEndian) ? "format binary_big_endian 1.0" : "format binary_little_endian 1.0") << std::endl;
    else
        os << "format ascii 1.0" << std::endl;
    
    for (const auto & comment : comments)
        os << "comment " << comment << std::endl;
    
    for (auto & e : elements)
    {
        os << "element " << e.name << " " << e.size << std::endl;
        for (const auto & p : e.properties)
        {
            if (p.isList)
            {
              os << "property list " << PropertyTable[p.listType].str << " "
                 << PropertyTable[p.propertyType].str << " " << p.name << std::endl;
            }
            else
            {
                os << "property " << PropertyTable[p.propertyType].str << " " << p.name << std::endl;
            }
        }
    }
    os << "end_header" << std::endl;
}

void PlyFile::parse_data_binary(std::istream & is, const std::vector<uint8_t> & buffer)
{
    uint32_t fileOffset = 0;
    const size_t headerPosition = is.tellg();
    const std::uint8_t * srcBuffer = buffer.data() + headerPosition;
    
    for (auto & element : get_elements())
    {
        if (std::find(requestedElements.begin(), requestedElements.end(), element.name) != requestedElements.end())
        {
            for (int64_t count = 0; count < element.size; ++count)
            {
                for (const auto & property : element.properties)
                {
                    if (userDataTable[property.name])
                    {
                        auto & cursor = userDataTable[property.name];
                        if (property.isList)
                        {
                            uint32_t listSize = 0;
                            uint32_t dummyCount = 0;
                            read_property(property.listType, &listSize, dummyCount, srcBuffer, fileOffset);
                            for (int i = 0; i < listSize; ++i)
                                read_property(property.propertyType, (cursor->data + cursor->offset), cursor->offset, srcBuffer, fileOffset);
                        }
                        else
                        {
                            read_property(property.propertyType, (cursor->data + cursor->offset), cursor->offset, srcBuffer, fileOffset);
                        }
                    }
                    else
                    {
                        skip_property(fileOffset, property, srcBuffer);
                    }
                }
            }
        }
        else continue;
    }
}

void PlyFile::parse_data_ascii(std::istream & is, const std::vector<uint8_t> & buffer)
{
    for (auto & element : get_elements())
    {
        if (std::find(requestedElements.begin(), requestedElements.end(), element.name) != requestedElements.end())
        {
            for (int64_t count = 0; count < element.size; ++count)
            {
                for (const auto & property : element.properties)
                {
                    if (userDataTable[property.name])
                    {
                        auto & cursor = userDataTable[property.name];
                        if (property.isList)
                        {
                            uint32_t listSize = 0;
                            uint32_t dummyCount = 0;
                            read_property(property.listType, &listSize, dummyCount, is);
                            for (int i = 0; i < listSize; ++i)
                            {
                                read_property(property.propertyType, (cursor->data + cursor->offset), cursor->offset, is);
                            }
                        }
                        else
                        {
                            read_property(property.propertyType, (cursor->data + cursor->offset), cursor->offset, is);
                        }
                    }
                    else
                    {
                        skip_property(is, property);
                    }
                }
            }
        }
        else continue;
    }
}