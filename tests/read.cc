#include <fstream>
#include <string>
#include <iostream>
#include <vector>

#include <tinyply.h>

template <typename T>
void print_vertices(const T& vertices)
{
		for (size_t i = 0; i < vertices.size(); i += 3)
		{
			std::cout << "v" << i/3 << ": ["
								<< vertices[i] << ", " << vertices[i+1] << ", " << vertices[i+2]
								<< "]" << std::endl;
		}
}

template <typename T>
void print_faces(const T& faces, int /* n */)
{
		for (size_t i = 0; i < faces.size(); i++)
		{
			std::cout << "f" << i << ": ";
			for (size_t j = 0; j < faces[i].size(); ++j)
			{
				if (j > 0) std::cout << " ";
				std::cout << faces[i][j];
			}
			std::cout << std::endl;
		}
}

template <>
void print_faces<std::vector<uint32_t>>(const std::vector<uint32_t>& faces, int n)
{
		for (size_t i = 0; i < faces.size()/n; i++)
		{
			std::cout << "f" << i << ": ";
			for (size_t j = 0; j < n; ++j)
			{
				if (j > 0) std::cout << " ";
				std::cout << faces[i*n + j];
			}
			std::cout << std::endl;
		}
}

template <typename FaceType, int NVertices>
int read_file(const std::string& path)
{
	using namespace tinyply;
	try
	{
		std::cout << "Reading " << path << std::endl;
		std::ifstream header(path);
		PlyFile file(header);

		std::cout << "Mode: " << (file.is_binary()? "binary":"ascii") << std::endl;
		for (const auto& e : file.get_elements())
		{
			std::cout << "element - " << e.name << " (" << e.size << ")" << std::endl;
			for (const auto& p : e.properties)
			{
				std::cout << "\tproperty - " << p.name << " (" << PropertyTable[p.propertyType].str << ")" << std::endl;
			}
		}

		for (const auto& c : file.comments)
		{
			std::cout << "Comment: " << c << std::endl;
		}

		std::vector<float> vertices;
		std::vector<FaceType> faces;

		uint32_t vertexCount = file.request_properties_from_element("vertex", { "x", "y", "z" }, vertices, 1);
		uint32_t faceCount = file.request_properties_from_element("face", { "vertex_indices" }, faces, NVertices);

		// Use different ifstream with proper openmode and position
		std::ios::openmode mode = file.is_binary()? std::ios::in | std::ios::binary
                                                          : std::ios::in;
		std::ifstream ss(path, mode);
		ss.seekg(header.tellg());

		// Populate the vectors
		file.read(ss);

		print_vertices(vertices);
		print_faces(faces, NVertices);

		std::cout << std::endl;

		return 0;
	}
	catch (const std::exception & e)
	{
		std::cerr << "Caught exception: " << e.what() << std::endl;
		return 1;
	}
}

int main(int argc, char *argv[])
{
	int res = 0;
	res += read_file<std::vector<uint32_t>, 0>(ASSETS_DIR "/cube_ascii.ply");
	res += read_file<std::vector<uint32_t>, 0>(ASSETS_DIR "/icosahedron.ply");
	res += read_file<uint32_t, 4>(ASSETS_DIR "/cube_quad_ascii.ply");
	return res;
}
