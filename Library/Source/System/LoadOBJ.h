/**
 LoadOBJ
 @brief A class to load an OBJ file into system memory
 By: Lim Chian Song. Modified by Toh Da Jun
 Date: Sep 2021
 */
#ifndef LOAD_OBJ_H
#define LOAD_OBJ_H

#include <includes/glm.hpp>

// Include Mesh
#include "../Primitives/Mesh.h"

// Include vector data structure
#include <vector>

class ModelVertex
{
public:
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;
	ModelVertex(void)
		: pos(glm::vec3(0.0f))
		, normal(glm::vec3(0.0f))
		, texCoord(glm::vec2(0.0f))
	{
	}
};

class ModelVertex2
{
public:
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec4 colour;
	glm::vec2 texCoord;
	ModelVertex2(void)
		: pos(glm::vec3(0.0f))
		, normal(glm::vec3(0.0f))
		, colour(glm::vec4(1.0f))
		, texCoord(glm::vec2(0.0f))
	{
	}
};

class CLoadOBJ
{
public:
	// Read an OBJ file
	bool ReadOBJFile(	const char* file_path,
					std::vector<glm::vec3>& out_vertices,
					std::vector<glm::vec2>& out_uvs,
					std::vector<glm::vec3>& out_normals,
					const bool bFlipTextureForZ = false);

	// Create a Mesh Object from an OBJ file and initialise it to OpenGL
	CMesh* CreateMesh(	const char* file_path,
						const bool bUseIndices = false,
						const bool bFlipTextureForZ = false);
	// Create a Mesh Object from an OBJ file and initialise it for Instancing
	CMesh* CreateMeshForInstancing(	const char* file_path,
									const bool bFlipTextureForZ = false);

	// Create the IndexVBOs using indices
	void IndexVBO(	std::vector<glm::vec3>& in_vertices,
					std::vector<glm::vec2>& in_uvs,
					std::vector<glm::vec3>& in_normals,

					std::vector<unsigned>& out_indices,
					std::vector<ModelVertex2>& out_vertices);
	// Create the IndexVBOs without indicies
	void IndexVBOWithoutIndices(	std::vector<glm::vec3>& in_vertices,
									std::vector<glm::vec2>& in_uvs,
									std::vector<glm::vec3>& in_normals,

									std::vector<ModelVertex2>& out_vertices);
};

#endif