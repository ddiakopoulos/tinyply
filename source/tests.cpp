// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.
// https://github.com/ddiakopoulos/tinyply

// ~ Work in Progress ~
// This implements a suit of file format conformance tests. Running this currently requires a very large
// folder of assets that have been sourced from a wide variety of internet sources, transcoded or exported
// from known ply-compatible software including Houdini, VTK, CGAL, Meshlab, Matlab, Blender, Draco, Assimp,
// the Stanford 3D Scanning Repository, and others. Because of the wide variety of sources, these files
// are not re-distributed. 

#include "tinyply.h"
using namespace tinyply;

#include "example-utils.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

void parse_ply_file(const std::string & filepath)
{
    manual_timer read_timer;

    std::ifstream filestream(filepath, std::ios::binary);

    if (filestream.is_open())
    {
        filestream.seekg(0, std::ios::end);
        const float size_mb = filestream.tellg() * float(1e-6);
        filestream.seekg(0, std::ios::beg);

        PlyFile file;
        file.parse_header(filestream);

        // All ply files are required to have a vertex element
        std::unordered_map<std::string, std::shared_ptr<PlyData>> vertex_element;

        std::cout << "testing: " << filepath << " - filetype:" << (file.is_binary_file() ? "binary" : "ascii") << std::endl;

        REQUIRE(file.get_elements().size() > 0);

        for (const auto & e : file.get_elements())
        {
            if (e.name == "vertex")
            {
                REQUIRE(e.properties.size() > 0);
                for (const auto & p : e.properties)
                {
                    try { vertex_element[p.name] = file.request_properties_from_element(e.name, { p.name }); }
                    catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }
                }
            }
        }

        read_timer.start();
        file.read(filestream);
        read_timer.stop();

        const float parsing_time = read_timer.get() / 1000.f;
        std::cout << "\tparsing took " << parsing_time << " seconds / " << (size_mb / parsing_time) << " MBps..." << std::endl;
        for (auto & p : vertex_element)
        {
            REQUIRE(p.second->count > 0);

            for (const auto& e : file.get_elements())
            {
                for (const auto & prop : e.properties)
                {
                    if (prop.name == p.first)
                    {
                        REQUIRE(e.size == p.second->count);
                    }
                }
            }
        }
    }
}

TEST_CASE("2d.vertex.ply")
{
    parse_ply_file("../assets/validate/valid/2d.vertex.ply");
}

TEST_CASE("airplane.ply")
{
    parse_ply_file("../assets/validate/valid/airplane.ply");
}

TEST_CASE("ant.ply")
{
    parse_ply_file("../assets/validate/valid/ant.ply");
}

TEST_CASE("armadillo.ascii.ply")
{
    parse_ply_file("../assets/validate/valid/armadillo.ascii.ply");
}

TEST_CASE("armadillo.ply")
{
    parse_ply_file("../assets/validate/valid/armadillo.ply");
}

TEST_CASE("artec.bus.ply")
{
    parse_ply_file("../assets/validate/valid/artec.bus.ply");
}

TEST_CASE("artec.crocodile-statue.ply")
{
    parse_ply_file("../assets/validate/valid/artec.crocodile-statue.ply");
}

TEST_CASE("artec.face.ply")
{
    parse_ply_file("../assets/validate/valid/artec.face.ply");
}

TEST_CASE("artec.hand.ply")
{
    parse_ply_file("../assets/validate/valid/artec.hand.ply");
}

TEST_CASE("beethoven.ply")
{
    parse_ply_file("../assets/validate/valid/beethoven.ply");
}

TEST_CASE("bird.ply")
{
    parse_ply_file("../assets/validate/valid/bird.ply");
}

TEST_CASE("blade.ply")
{
    parse_ply_file("../assets/validate/valid/blade.ply");
}

TEST_CASE("brain.ply")
{
    parse_ply_file("../assets/validate/valid/brain.ply");
}

TEST_CASE("bunny.ply")
{
    parse_ply_file("../assets/validate/valid/bunny.ply");
}

TEST_CASE("cgal.colors.ply")
{
    parse_ply_file("../assets/validate/valid/cgal.colors.ply");
}

TEST_CASE("cow.ply")
{
    parse_ply_file("../assets/validate/valid/cow.ply");
}

TEST_CASE("cube_att.ply")
{
    parse_ply_file("../assets/validate/valid/cube_att.ply");
}

TEST_CASE("dimitri-scan.ply")
{
    parse_ply_file("../assets/validate/valid/dimitri-scan.ply");
}

TEST_CASE("draco.ascii.whitespace.ply")
{
    parse_ply_file("../assets/validate/valid/draco.ascii.whitespace.ply");
}

TEST_CASE("draco.int_point_cloud.ply")
{
    parse_ply_file("../assets/validate/valid/draco.int_point_cloud.ply");
}

TEST_CASE("dragon.ply")
{
    parse_ply_file("../assets/validate/valid/dragon.ply");
}

TEST_CASE("freedom_model.ply")
{
    parse_ply_file("../assets/validate/valid/freedom_model.ply");
}

TEST_CASE("golfball.ply")
{
    parse_ply_file("../assets/validate/valid/golfball.ply");
}

TEST_CASE("hand.ply")
{
    parse_ply_file("../assets/validate/valid/hand.ply");
}

TEST_CASE("happy.ply")
{
    parse_ply_file("../assets/validate/valid/happy.ply");
}

TEST_CASE("head1.ply")
{
    parse_ply_file("../assets/validate/valid/head1.ply");
}

TEST_CASE("golfball.ply")
{
    parse_ply_file("../assets/validate/valid/golfball.ply");
}

TEST_CASE("helix.ply")
{
    parse_ply_file("../assets/validate/valid/helix.ply");
}

TEST_CASE("heptoroid.ply")
{
    parse_ply_file("../assets/validate/valid/heptoroid.ply");
}

TEST_CASE("kcraine.csaszar.ply")
{
    parse_ply_file("../assets/validate/valid/kcraine.csaszar.ply");
}

TEST_CASE("kcrane.bob.meshconvert.com.ply")
{
    parse_ply_file("../assets/validate/valid/kcrane.bob.meshconvert.com.ply");
}

TEST_CASE("kcrane.city.ply")
{
    parse_ply_file("../assets/validate/valid/kcrane.city.ply");
}

TEST_CASE("kcrane.spot.ply")
{
    parse_ply_file("../assets/validate/valid/kcrane.spot.ply");
}

TEST_CASE("laserdesign.dragon.ply")
{
    parse_ply_file("../assets/validate/valid/laserdesign.dragon.ply");
}

TEST_CASE("lion.ply")
{
    parse_ply_file("../assets/validate/valid/lion.ply");
}

TEST_CASE("lucy.decimated.ply")
{
    parse_ply_file("../assets/validate/valid/lucy.decimated.ply");
}

TEST_CASE("lucy.ply")
{
    parse_ply_file("../assets/validate/valid/lucy.ply");
}

TEST_CASE("lucy_le.ply")
{
    parse_ply_file("../assets/validate/valid/lucy_le.ply");
}

TEST_CASE("matlab.colinear.ply")
{
    parse_ply_file("../assets/validate/valid/matlab.colinear.ply");
}

TEST_CASE("matlab.ply")
{
    parse_ply_file("../assets/validate/valid/matlab.ply");
}

TEST_CASE("maxplanck.ply")
{
    parse_ply_file("../assets/validate/valid/maxplanck.ply");
}

TEST_CASE("nefertiti.ply")
{
    parse_ply_file("../assets/validate/valid/nefertiti.ply");
}

TEST_CASE("points-only.ply")
{
    parse_ply_file("../assets/validate/valid/points-only.ply");
}

TEST_CASE("random.obj-info.ply")
{
    parse_ply_file("../assets/validate/valid/random.obj-info.ply");
}

TEST_CASE("redrocks.dronemapper.ply")
{
    parse_ply_file("../assets/validate/valid/redrocks.dronemapper.ply");
}

TEST_CASE("scaninabox.dwarf.ply")
{
    parse_ply_file("../assets/validate/valid/scaninabox.dwarf.ply");
}

TEST_CASE("shark.ply")
{
    parse_ply_file("../assets/validate/valid/shark.ply");
}

TEST_CASE("t3.bone.big-endian.ply")
{
    parse_ply_file("../assets/validate/valid/t3.bone.big-endian.ply");
}

TEST_CASE("teapot.ply")
{
    parse_ply_file("../assets/validate/valid/teapot.ply");
}

TEST_CASE("test_cloud.ply")
{
    parse_ply_file("../assets/validate/valid/test_cloud.ply");
}

TEST_CASE("tet.ascii.ply")
{
    parse_ply_file("../assets/validate/valid/tet.ascii.ply");
}

TEST_CASE("tet.ascii.variable-length.ply")
{
    parse_ply_file("../assets/validate/valid/tet.ascii.variable-length.ply");
}

TEST_CASE("torus.ply")
{
    parse_ply_file("../assets/validate/valid/torus.ply");
}

TEST_CASE("tri_gouraud.ply")
{
    parse_ply_file("../assets/validate/valid/tri_gouraud.ply");
}

TEST_CASE("vtk.blob.ply")
{
    parse_ply_file("../assets/validate/valid/vtk.blob.ply");
}

TEST_CASE("navvis.HQ3rdFloor.SLAM.5mm.ply")
{
    parse_ply_file("../assets/validate/valid/navvis.HQ3rdFloor.SLAM.5mm.ply");
}
