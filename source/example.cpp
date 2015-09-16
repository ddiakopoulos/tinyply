// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.
// Authored in 2015 by Dimitri Diakopoulos (http://www.dimitridiakopoulos.com)
// https://github.com/ddiakopoulos/tinyply

#include <thread>
#include <chrono>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

#include "tinyply.h"

using namespace tinyply;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint;
std::chrono::high_resolution_clock c;

inline std::chrono::time_point<std::chrono::high_resolution_clock> now()
{
    return c.now();
}

inline double difference_micros(timepoint start, timepoint end)
{
    return (double) std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

std::vector<uint8_t> read_file_binary(std::string pathToFile)
{
    FILE * f = fopen(pathToFile.c_str(), "rb");
    
    if (!f)
        throw std::runtime_error("file not found");
    
    fseek(f, 0, SEEK_END);
    size_t lengthInBytes = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    std::vector<uint8_t> fileBuffer(lengthInBytes);
    
    size_t elementsRead = fread(fileBuffer.data(), 1, lengthInBytes, f);
    
    if (elementsRead == 0 || fileBuffer.size() < 64)
        throw std::runtime_error("error reading file or file too small");

    fclose(f);
    return fileBuffer;
}

int main(int argc, char *argv[])
{
    // icosahedron.ply
    // large/xyzrgb_dragon.ply
    // icosahedron_ascii.ply
    
    
    std::ofstream outFile("test_header.ply");
    std::ostringstream outputStream;
    
    PlyFile myFile;
    
    std::vector<float> verts;
    std::vector<uint32_t> faces;
    
    verts = { 1.0,0.0,0.0,-1.0,0.0,0.0,0.0,1.0,0.0,0.0,-1.0,0.0,0.0,0.0,1.0,0.0,0.0,-1.0 };
    faces = { 0, 2, 4, 0, 4, 3, 0, 3, 5, 0, 5, 2, 1, 2, 5, 1, 5, 3, 1, 3, 4, 1, 2, 4 };
    
    myFile.add_element({"vertices", static_cast<int>(verts.size())});
    myFile.add_element({"faces", static_cast<int>(faces.size())});
    
    myFile.add_property_to_element("vertices", {PlyProperty::Type::FLOAT32, "x"});
    myFile.add_property_to_element("vertices", {PlyProperty::Type::FLOAT32, "y"});
    myFile.add_property_to_element("vertices", {PlyProperty::Type::FLOAT32, "z"});
    
    myFile.add_property_to_element("faces", {PlyProperty::Type::UINT8, PlyProperty::Type::UINT32, "vertex_indices"});
    
    myFile.set_data_for_properties({"x", "y", "z"}, verts);
    myFile.set_data_for_properties({"vertex_indices"}, faces);
    
    myFile.write(outputStream);
    outFile << outputStream.str();
    outFile.close();
    
    /*
    auto f = read_file_binary("assets/icosahedron.ply");
    std::istringstream ss((char*)f.data(), std::ios::binary);
    
    PlyFile file(ss);
    
    std::vector<float> verts;
    std::vector<float> normals;
    std::vector<uint32_t> faces;
    
    for (auto e : file.get_elements())
    {
        std::cout << "element - " << e.get_name() << " (" << e.get_element_count() << ")" << std::endl;
        for (auto p : e.get_properties())
        {
            std::cout << "\t property - " << p.get_name() << std::endl;
        }
    }
    std::cout << std::endl;
    
    uint32_t vertexCount = file.request_properties_from_element("vertex", {"x", "y", "z"}, verts);
    uint32_t normalCount = file.request_properties_from_element("vertex", {"nx", "ny", "nz"}, normals);
    uint32_t faceCount = file.request_properties_from_element("face", {"vertex_indices"}, faces);
    
    try
    {
        timepoint before = now();
        file.parse(ss, f);
        timepoint after = now();
        
        // Good place to hang a breakpoint:
        std::cout << "Parsed " << verts.size() << " vertex elements in " << difference_micros(before, after) << "μs" << std::endl;
        std::cout << "... and " << faces.size() << " faces " << std::endl;
    }
    catch (std::exception e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }
    */
    
    return 0;
}
