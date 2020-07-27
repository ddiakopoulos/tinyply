// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.
// https://github.com/ddiakopoulos/tinyply

// ~ Work in Progress ~
// This implements a suit of file format conformance tests. Running this currently requires a very large
// folder of assets that have been sourced from a variety of internet sources, transcoded or exported
// from known ply-compatible software including Houdini, VTK, CGAL, Meshlab, Matlab, Blender, Draco, Assimp,
// the Stanford 3D Scanning Repository, and others. Because of the wide variety of sources and copyright issues, 
// these files are not re-distributed. 

#include "tinyply.h"
using namespace tinyply;

#include "example-utils.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

inline std::string get_filename_without_extension(const std::string& path)
{
    if (path.find_last_of(".") != std::string::npos && path.find_last_of("\\") != std::string::npos)
    {
        size_t end = path.find_last_of(".");
        size_t start = path.find_last_of("\\") + 1;
        return path.substr(start, end - start);
    }
    else if (path.find_last_of(".") != std::string::npos && path.find_last_of("/") != std::string::npos)
    {
        size_t end = path.find_last_of(".");
        size_t start = path.find_last_of("/") + 1;
        return path.substr(start, end - start);
    }
    return path;
}

void transcode_ply_file(PlyFile & file, const std::string & filepath)
{
    auto filename = get_filename_without_extension(filepath);

    std::filebuf fb_binary;
    fb_binary.open(filename + "-transcode-binary.ply", std::ios::out | std::ios::binary);
    std::ostream outstream_binary(&fb_binary);
    if (outstream_binary.fail()) throw std::runtime_error("failed to open " + filename);
    file.write(outstream_binary, true);

    //std::filebuf fb_ascii;
    //fb_ascii.open(filename + "-transcode-ascii.ply", std::ios::out);
    //std::ostream outstream_ascii(&fb_ascii);
    //if (outstream_ascii.fail()) throw std::runtime_error("failed to open " + filename);
    //file.write(outstream_ascii, false);
}

bool parse_ply_file(const std::string & filepath)
{
    manual_timer timer;
    std::ifstream filestream(filepath, std::ios::binary);

    try
    {
        if (filestream.is_open())
        {
            PlyFile file;

            filestream.seekg(0, std::ios::end);
            const float size_mb = filestream.tellg() * float(1e-6);
            filestream.seekg(0, std::ios::beg);

            bool header_result = file.parse_header(filestream);

            // All ply files are required to have a vertex element
            std::unordered_map<std::string, std::shared_ptr<PlyData>> vertex_element;

            std::cout << "testing: " << filepath << " - filetype: " << (file.is_binary_file() ? "binary" : "ascii") << std::endl;

            REQUIRE(file.get_elements().size() > 0);

            std::string likely_face_property_name;
            // Extract a fat vertex structure (will likely include more than xyz)
            for (const auto & e : file.get_elements())
            {
                if (e.name == "vertex")
                {
                    REQUIRE(e.properties.size() > 0);
                    for (const auto & p : e.properties)
                    {
                        try { vertex_element[p.name] = file.request_properties_from_element(e.name, { p.name }); }
                        catch (const std::exception & e) { /**/ }
                    }
                }

                // Heuristic...
                if (e.name == "face")
                {
                    for (int i = 0; i < 1; ++i) likely_face_property_name = e.properties[i].name;
                }
            }

            std::shared_ptr<PlyData> faces, tripstrip;

            if (!likely_face_property_name.empty())
            {
                try { faces = file.request_properties_from_element("face", { likely_face_property_name }, 0); }
                catch (const std::exception& e) { /**/  }

                try { tripstrip = file.request_properties_from_element("tristrips", { likely_face_property_name }, 0); }
                catch (const std::exception& e) { /**/  }
            }

            timer.start();
            file.read(filestream);
            timer.stop();

            const float parsing_time = (float) timer.get() / 1000.f;
            std::cout << "\tparsing " << size_mb << "mb in " << parsing_time << " seconds [" << (size_mb / parsing_time) << " MBps]" << std::endl;

            for (auto & p : vertex_element)
            {
                REQUIRE(p.second->count > 0);

                for (const auto& e : file.get_elements())
                {
                    for (const auto & prop : e.properties)
                    {
                        if (e.name == "vertex" && prop.name == p.first)
                        {
                            REQUIRE(e.size == p.second->count);
                        }
                    }
                }
            }

            timer.start();
            transcode_ply_file(file, filepath);
            timer.stop();

            const float transcode_time = (float)timer.get() / 1000.f;
            std::cout << "\ttranscoded in " << transcode_time << " seconds." << std::endl;

            return header_result;
        }
    } 
    catch (const std::exception & e)
    {
        std::cout << "Caught Exception: " << e.what() << std::endl;
        REQUIRE(false);
    }

    return false;
}

///////////////////////////
//   Conformance Tests   //
///////////////////////////

TEST_CASE("importing conformance tests")
{
    manual_timer timer;

    timer.start();
    parse_ply_file("../assets/validate/valid/bunny.ply");
    parse_ply_file("../assets/validate/valid/horse.ply");
    parse_ply_file("../assets/validate/valid/2d.vertex.ply");
    parse_ply_file("../assets/validate/valid/airplane.ply");
    parse_ply_file("../assets/validate/valid/ant.ply");
    parse_ply_file("../assets/validate/valid/armadillo.ascii.ply");
    parse_ply_file("../assets/validate/valid/armadillo.ply");
    parse_ply_file("../assets/validate/valid/artec.bus.ply");
    parse_ply_file("../assets/validate/valid/artec.crocodile-statue.ply");
    parse_ply_file("../assets/validate/valid/artec.face.ply");
    parse_ply_file("../assets/validate/valid/artec.hand.ply");
    parse_ply_file("../assets/validate/valid/beethoven.ply");
    parse_ply_file("../assets/validate/valid/bird.ply");
    parse_ply_file("../assets/validate/valid/brain.ply");
    parse_ply_file("../assets/validate/valid/cgal.colors.ply");
    parse_ply_file("../assets/validate/valid/cow.ply");
    parse_ply_file("../assets/validate/valid/cube_att.ply");
    parse_ply_file("../assets/validate/valid/dimitri-scan.ply");
    parse_ply_file("../assets/validate/valid/draco.ascii.whitespace.ply");
    parse_ply_file("../assets/validate/valid/draco.int_point_cloud.ply");
    parse_ply_file("../assets/validate/valid/dragon.ply");
    parse_ply_file("../assets/validate/valid/freedom_model.ply");
    parse_ply_file("../assets/validate/valid/golfball.ply");
    parse_ply_file("../assets/validate/valid/hand.ply");
    parse_ply_file("../assets/validate/valid/happy.ply");
    parse_ply_file("../assets/validate/valid/head1.ply");
    parse_ply_file("../assets/validate/valid/golfball.ply");
    parse_ply_file("../assets/validate/valid/helix.ply");
    parse_ply_file("../assets/validate/valid/heptoroid.ply");
    parse_ply_file("../assets/validate/valid/kcrane.csaszar.ply");
    parse_ply_file("../assets/validate/valid/kcrane.spot.ply");
    parse_ply_file("../assets/validate/valid/laserdesign.dragon.ply");
    parse_ply_file("../assets/validate/valid/lion.ply");
    parse_ply_file("../assets/validate/valid/lucy.decimated.ply");
    parse_ply_file("../assets/validate/valid/matlab.colinear.ply");
    parse_ply_file("../assets/validate/valid/matlab.ply");
    parse_ply_file("../assets/validate/valid/maxplanck.ply");
    parse_ply_file("../assets/validate/valid/nefertiti.ply");
    parse_ply_file("../assets/validate/valid/points-only.ply");
    parse_ply_file("../assets/validate/valid/random.obj-info.ply");
    parse_ply_file("../assets/validate/valid/scaninabox.dwarf.ply");
    parse_ply_file("../assets/validate/valid/shark.ply");
    parse_ply_file("../assets/validate/valid/t3.bone.big-endian.ply");
    parse_ply_file("../assets/validate/valid/teapot.ply");
    parse_ply_file("../assets/validate/valid/test_cloud.ply");
    parse_ply_file("../assets/validate/valid/tet.ascii.ply");
    parse_ply_file("../assets/validate/valid/torus.ply");
    parse_ply_file("../assets/validate/valid/tri_gouraud.ply");
    parse_ply_file("../assets/validate/valid/vtk.blob.ply");
    parse_ply_file("../assets/validate/valid/blade.ply");                      // 82mb
    parse_ply_file("../assets/validate/valid/lucy.ply");                       // 520mb
    parse_ply_file("../assets/validate/valid/redrocks.dronemapper.ply");       // 268mb
    parse_ply_file("../assets/validate/valid/navvis.HQ3rdFloor.SLAM.5mm.ply"); // 1700mb
    timer.stop();

    const float conformance_time = (float)timer.get() / 1000.f;
    std::cout << ">>> test ran in " << conformance_time << " seconds." << std::endl;
}

///////////////////
//   Unit Tests  //
///////////////////

// See https://github.com/ddiakopoulos/tinyply/issues/25
TEST_CASE("requested property groups must all share the same type")
{
    std::ifstream filestream("../assets/validate/invalid/payload.empty.ply", std::ios::binary);
    PlyFile file;
    bool header_result = file.parse_header(filestream);
    CHECK_THROWS_AS(file.request_properties_from_element("vertex", { "x", "y", "z", "r", "g", "b", "a", "uv1", "uv2" }), std::invalid_argument);
}

// An earlier (but widespread) version of Assimp had a non-conformant PLY exporter and did not prepend comments with "comment" 
TEST_CASE("check for invalid strings in the header")
{
    std::ifstream filestream("../assets/validate/invalid/kcrane.bob.meshconvert.com.ply", std::ios::binary);
    PlyFile file;
    bool header_result = file.parse_header(filestream);
    REQUIRE_FALSE(header_result); 
}

// Reported via https://github.com/vilya/ply-parsing-perf
TEST_CASE("check that variable length lists are unsupported (without crashing)")
{
    auto variable_length_test = [](const std::string & filepath)
    {
        std::ifstream filestream(filepath, std::ios::binary);
        PlyFile file;
        bool header_result = file.parse_header(filestream);
        REQUIRE(header_result);

        std::shared_ptr<PlyData> faces;
        try { faces = file.request_properties_from_element("face", { "vertex_indices" }, 0); }
        catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        CHECK_THROWS_AS(file.read(filestream), std::runtime_error);
    };

    variable_length_test("../assets/validate/valid/tet.ascii.variable-length.ply");
    variable_length_test("../assets/validate/valid/kcrane.city.ply");
}

//TEST_CASE("check that int128 is an unrecognized, non-conformant datatype")
//{
//    std::ifstream filestream("../assets/validate/invalid/header.invalid-face-data-type-int128.ply", std::ios::binary);
//    PlyFile file;
//    bool header_result = file.parse_header(filestream);
//    REQUIRE_FALSE(header_result); 
//}
//
//TEST_CASE("check that float16 is an unrecognized, non-conformant datatype")
//{
//    std::ifstream filestream("../assets/validate/invalid/header.invalid-face-property-type-float16.ply", std::ios::binary);
//    PlyFile file;
//    bool header_result = file.parse_header(filestream);
//    REQUIRE_FALSE(header_result); 
//}
//
//TEST_CASE("check that elements must have at least one property")
//{
//    std::ifstream filestream("../assets/validate/invalid/header.incomplete-face-def.ply", std::ios::binary);
//    PlyFile file;
//    bool header_result = file.parse_header(filestream);
//    REQUIRE(header_result);
//    for (const auto & e : file.get_elements()) REQUIRE(e.properties.size() > 0);
//}
//
// TEST_CASE("check that elements must have at least one property")
// {
//     std::ifstream filestream("../assets/validate/invalid/header.incomplete-face-def.ply", std::ios::binary);
//     PlyFile file;
//     bool header_result = file.parse_header(filestream);
//     REQUIRE(header_result);
//     //for (const auto & e : file.get_elements()) REQUIRE(e.properties.size() > 0);
// }
// 
// TEST_CASE("check that element count needs to be >= 0")
// {
//     std::ifstream filestream("../assets/validate/invalid/header.invalid-element-count.estatica.ply", std::ios::binary);
//     PlyFile file;
//     bool header_result = file.parse_header(filestream);
//     REQUIRE_FALSE(header_result);
// }
// 
// TEST_CASE("header.invalid-face-property.ply")
// {
//     parse_ply_file("../assets/validate/invalid/header.invalid-face-property.ply");
// }
// 
// TEST_CASE("header.invalid-face-size-type-int128.ply")
// {
//     parse_ply_file("../assets/validate/invalid/header.invalid-face-size-type-int128.ply");
// }
// 
// TEST_CASE("header.invalid-ply-signature.ply")
// {
//     parse_ply_file("../assets/validate/invalid/header.invalid-ply-signature.ply");
// }
// 
// TEST_CASE("header.invalid-property-type.ply")
// {
//     parse_ply_file("../assets/validate/invalid/header.invalid-property-type.ply");
// }
// 
// TEST_CASE("header.invalid-vertex-property.ply")
// {
//     parse_ply_file("../assets/validate/invalid/header.invalid-vertex-property.ply");
// }
// 
// TEST_CASE("header.malformed-extra-line.ply")
// {
//     parse_ply_file("../assets/validate/invalid/header.malformed-extra-line.ply");
// }
// 
// TEST_CASE("header.malformed-face-before-format.ply")
// {
//     parse_ply_file("../assets/validate/invalid/header.malformed-face-before-format.ply");
// }
// 
// TEST_CASE("header.malformed-format.ply")
// {
//     parse_ply_file("../assets/validate/invalid/header.malformed-format.ply");
// }
// 
// TEST_CASE("header.malformed-missing-format.ply")
// {
//     parse_ply_file("../assets/validate/invalid/header.malformed-missing-format.ply");
// }
// 
// TEST_CASE("header.malformed-unexpected-property.ply")
// {
//     parse_ply_file("../assets/validate/invalid/header.malformed-unexpected-property.ply");
// }
// 
// TEST_CASE("header.no-elements.ply")
// {
//     parse_ply_file("../assets/validate/invalid/header.no-elements.ply");
// }
// 
// TEST_CASE("header.unknown-element-edge.ply")
// {
//     parse_ply_file("../assets/validate/invalid/header.unknown-element-edge.ply");
// }
// 
// TEST_CASE("payload.corrupt-extra-props.ply")
// {
//     parse_ply_file("../assets/validate/invalid/payload.corrupt-extra-props.ply");
// }
// 
// TEST_CASE("payload.empty.ply")
// {
//     parse_ply_file("../assets/validate/invalid/payload.empty.ply");
// }
// 
// TEST_CASE("payload.fail.3.ply")
// {
//     parse_ply_file("../assets/validate/invalid/payload.fail.3.ply");
// }
// 
// TEST_CASE("payload.fail.4.ply")
// {
//     parse_ply_file("../assets/validate/invalid/payload.fail.4.ply");
// }
// 
// TEST_CASE("payload.ignored-face-components.ply")
// {
//     parse_ply_file("../assets/validate/invalid/payload.ignored-face-components.ply");
// }
// 
// TEST_CASE("payload.ignored-vertex-components.ply")
// {
//     parse_ply_file("../assets/validate/invalid/payload.ignored-vertex-components.ply");
// }
// 
// TEST_CASE("payload.unaligned-memory.ply")
// {
//     parse_ply_file("../assets/validate/invalid/payload.unaligned-memory.ply");
// }
// 
// TEST_CASE("payload.unexpected-eof.ply")
// {
//     parse_ply_file("../assets/validate/invalid/payload.unexpected-eof.ply");
// }
