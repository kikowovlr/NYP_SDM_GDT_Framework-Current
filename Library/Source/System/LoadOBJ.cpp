/**
 LoadOBJ
 @brief A class to load an OBJ file into system memory
 By: Lim Chian Song. Modified by Toh Da Jun
 Date: Sep 2021
 */
#include <iostream>
#include <fstream>
#include <map>

#include "LoadOBJ.h"

 // Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

struct PackedVertex{
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	bool operator<(const PackedVertex that) const{
		return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
	};
};

bool getSimilarVertexIndex_fast( 
	PackedVertex & packed, 
	std::map<PackedVertex, unsigned short> & VertexToOutIndex,
	unsigned short & result
){
	std::map<PackedVertex, unsigned short>::iterator it = VertexToOutIndex.find(packed);
	if(it == VertexToOutIndex.end())
	{
		return false;
	}
	else
	{
		result = it->second;
		return true;
	}
}

/**
 @brief Read an OBJ file
 @param file_path A const char* variable containing the file and path
 @param out_vertices A std::vector<glm::vec3>& variable containing the vector of vertices sent from this method
 @param out_uvs A std::vector<glm::vec2>& variable containing the vector of UVs sent from this method
 @param out_normals A std::vector<glm::vec3>& variable containing the vector of normals sent from this method
 @param bFlipTextureForZ A const bool variable indicating if the texture is flipped around the Z-axis
 @return true if the file was read in successfully, otherwise false
 */
bool CLoadOBJ::ReadOBJFile(	const char* file_path,
							std::vector<glm::vec3>& out_vertices,
							std::vector<glm::vec2>& out_uvs,
							std::vector<glm::vec3>& out_normals,
							const bool bFlipTextureForZ)
{
	// Open the file
	std::ifstream fileStream(file_path, std::ios::binary);
	if (!fileStream.is_open())
	{
		std::cout << "Impossible to open " << file_path << ". Are you in the right directory ?\n";
		return false;
	}

	std::vector<unsigned> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	while (!fileStream.eof())
	{
		char buf[256];
		fileStream.getline(buf, 256);
		if (strncmp("v ", buf, 2) == 0)
		{
			glm::vec3 vertex;
			sscanf_s((buf + 2), "%f%f%f", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strncmp("vt ", buf, 3) == 0)
		{
			glm::vec2 tc;
			sscanf_s((buf + 2), "%f%f", &tc.x, &tc.y);
			temp_uvs.push_back(tc);
		}
		else if (strncmp("vn ", buf, 3) == 0)
		{
			glm::vec3 normal;
			sscanf_s((buf + 2), "%f%f%f", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strncmp("f ", buf, 2) == 0)
		{
			unsigned int vertexIndex[4], uvIndex[4], normalIndex[4];
			int matches = sscanf_s((buf + 2), "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n",
				&vertexIndex[0], &uvIndex[0], &normalIndex[0],
				&vertexIndex[1], &uvIndex[1], &normalIndex[1],
				&vertexIndex[2], &uvIndex[2], &normalIndex[2],
				&vertexIndex[3], &uvIndex[3], &normalIndex[3]);

			if (matches == 9) //triangle
			{
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);
				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[1]);
				uvIndices.push_back(uvIndex[2]);
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);
			}
			else if (matches == 12) //quad
			{
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);
				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[1]);
				uvIndices.push_back(uvIndex[2]);
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);

				vertexIndices.push_back(vertexIndex[2]);
				vertexIndices.push_back(vertexIndex[3]);
				vertexIndices.push_back(vertexIndex[0]);
				uvIndices.push_back(uvIndex[2]);
				uvIndices.push_back(uvIndex[3]);
				uvIndices.push_back(uvIndex[0]);
				normalIndices.push_back(normalIndex[2]);
				normalIndices.push_back(normalIndex[3]);
				normalIndices.push_back(normalIndex[0]);
			}
			else
			{
				std::cout << "Error line: " << buf << std::endl;
				std::cout << "File can't be read by parser\n";
				return false;
			}
		}
	}
	// Close the file
	fileStream.close();

	// Process the data
	// For each vertex of each triangle
	for (unsigned i = 0; i < vertexIndices.size(); ++i)
	{
		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// If the texture is flipped in the Z-axis, then set bFlipTextureForZ to true
		if (bFlipTextureForZ == true)
			uv.y = 1.0f - uv.y;

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);
	}

	return true;
}

/**
 @brief Create a Mesh Object from an OBJ file and initialise it to OpenGL
 @param file_path A const char* containing the path and filename of the OBJ file
 @param bUseIndices A const bool variable indicating if the vertices are indexed and uploaded to OpenGL.
 @param bFlipTextureForZ A const bool variable indicating if the values in the Z-axis are to be flipped.
 @return CMesh* A pointer to a CMesh object
 */
CMesh* CLoadOBJ::CreateMesh(	const char* file_path,
								const bool bUseIndices,
								const bool bFlipTextureForZ)
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<ModelVertex2> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	bool success = ReadOBJFile(file_path, vertices, uvs, normals, true);
	if (!success)
	{
		//cout << "Unable to load " << filenameModel << endl;
		return NULL;
	}

	if (bUseIndices)
	{
		IndexVBO(vertices, uvs, normals, index_buffer_data, vertex_buffer_data);
	}
	else
	{
		// Load the vertices without indices
		IndexVBOWithoutIndices(vertices, uvs, normals, vertex_buffer_data);
	}

	// Create a mesh object
	CMesh* mesh = new CMesh();

	if (bUseIndices)
	{
		// Set the number of vertices into mesh->indexSize
		mesh->indexSize = index_buffer_data.size();
	}
	else
	{
		// Set the number of vertices into mesh->indexSize
		mesh->indexSize = vertex_buffer_data.size();
	}

	// Bind vertex array to the VAO now
	glBindVertexArray(mesh->VAO);

	// Upload the VBO to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(ModelVertex2), &vertex_buffer_data[0], GL_STATIC_DRAW);

	if (bUseIndices == true)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);
	}

	// Tell the GPU how to interpret the VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex2), (void*)0);
	glEnableVertexAttribArray(0);
	mesh->SetVertexAttribArrayEnabled(0, true);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex2), (void*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);
	mesh->SetVertexAttribArrayEnabled(1, true);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex2), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);
	mesh->SetVertexAttribArrayEnabled(2, true);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex2), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec4)));
	glEnableVertexAttribArray(3);
	mesh->SetVertexAttribArrayEnabled(3, true);
	// Unbind the GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind vertex array to the VAO now
	glBindVertexArray(0);

	if (bUseIndices == false)
	{
		// Set the mode to CMesh::DRAW_TRIANGLES_ARRAY
		mesh->mode = CMesh::DRAW_TRIANGLES_ARRAY;
	}

	return mesh;

}

/**
 @brief Create a Mesh Object from an OBJ file and initialise it for Instancing
 @param file_path A const char* containing the path and filename of the OBJ file
 @param bFlipTextureForZ A const bool variable indicating if the values in the Z-axis are to be flipped.
 @return CMesh* A pointer to a CMesh object
 */
CMesh* CLoadOBJ::CreateMeshForInstancing(	const char* file_path,
											const bool bFlipTextureForZ)
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<ModelVertex2> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	bool success = ReadOBJFile(file_path, vertices, uvs, normals, true);
	if (!success)
	{
		//cout << "Unable to load " << filenameModel << endl;
		return NULL;
	}

	// Load the vertices and indices
	IndexVBO(vertices, uvs, normals, index_buffer_data, vertex_buffer_data);

	// Create a mesh object
	CMesh* mesh = new CMesh();

	// Set the number of vertices into mesh->indexSize
	mesh->indexSize = vertices.size();

	// Bind vertex array to the VAO now
	glBindVertexArray(mesh->VAO);

	// Upload the VBO to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(ModelVertex2), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	// Tell the GPU how to interpret the VBO
	// position attribute
	// The location in the instancing .vs shader file is 0, so we must indicate 0 for glVertexAttribPointer(0,...
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex2), (void*)0);
	mesh->SetVertexAttribArrayEnabled(0, true);
	// normal attribute
	// The location in the instancing .vs shader file is 1, so we must indicate 1 for glVertexAttribPointer(1,...
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex2), (void*)(sizeof(glm::vec3)));
	mesh->SetVertexAttribArrayEnabled(1, true);
	// colour attribute
	// The location in the instancing .vs shader file is 2, so we must indicate 2 for glVertexAttribPointer(2,...
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex2), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3)));
	mesh->SetVertexAttribArrayEnabled(2, true);
	// texture coord attribute
	// The location in the instancing .vs shader file is 3, so we must indicate 3 for glVertexAttribPointer(3,...
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex2), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec4)));
	mesh->SetVertexAttribArrayEnabled(3, true);
	
	// Unbind the GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind vertex array to the VAO now
	glBindVertexArray(0);

	// Set the mode to CMesh::DRAW_TRIANGLES_INSTANCED
	mesh->mode = CMesh::DRAW_TRIANGLES_INSTANCED;

	return mesh;
}


/**
 @brief Create the IndexVBOs using indices
 @param in_vertices A std::vector<glm::vec3>& variable containing the vector of vertices sent into this method
 @param in_uvs A std::vector<glm::vec2>& variable containing the vector of UVs sent into this method
 @param in_normals A std::vector<glm::vec3>& variable containing the vector of normals sent into this method
 @param out_indices A std::vector<glm::vec2>& variable containing the vector of UVs sent out from this method
 @param out_vertices A std::vector<glm::vec3>& variable containing the vector of vertices sent out from method
 */
void CLoadOBJ::IndexVBO(
	std::vector<glm::vec3>& in_vertices,
	std::vector<glm::vec2>& in_uvs,
	std::vector<glm::vec3>& in_normals,

	std::vector<unsigned>& out_indices,
	std::vector<ModelVertex2>& out_vertices
)
{
	std::map<PackedVertex, unsigned short> VertexToOutIndex;

	// For each input vertex
	for (unsigned int i = 0; i < in_vertices.size(); ++i)
	{

		PackedVertex packed = { in_vertices[i], in_uvs[i], in_normals[i] };

		// Try to find a similar vertex in out_XXXX
		unsigned short index;
		bool found = getSimilarVertexIndex_fast(packed, VertexToOutIndex, index);

		if (found)
		{
			// A similar vertex is already in the VBO, use it instead !
			out_indices.push_back(index);
		}
		else
		{
			// If not, it needs to be added in the output data.
			ModelVertex2 v;
			v.pos = glm::vec3(in_vertices[i].x, in_vertices[i].y, in_vertices[i].z);
			v.texCoord = glm::vec2(in_uvs[i].x, in_uvs[i].y);
			v.colour = glm::vec4(1.0f);
			v.normal = glm::vec3(in_normals[i].x, in_normals[i].y, in_normals[i].z);

			out_vertices.push_back(v);
			unsigned newindex = (unsigned)out_vertices.size() - 1;
			out_indices.push_back(newindex);
			VertexToOutIndex[packed] = newindex;
		}
	}
}

/**
 @brief Create the IndexVBOs without using indices
 @param in_vertices A std::vector<glm::vec3>& variable containing the vector of vertices sent into this method
 @param in_uvs A std::vector<glm::vec2>& variable containing the vector of UVs sent into this method
 @param in_normals A std::vector<glm::vec3>& variable containing the vector of normals sent into this method
 @param out_vertices A std::vector<glm::vec3>& variable containing the vector of vertices sent out from method
 */
void CLoadOBJ::IndexVBOWithoutIndices(	std::vector<glm::vec3>& in_vertices,
										std::vector<glm::vec2>& in_uvs,
										std::vector<glm::vec3>& in_normals,

										std::vector<ModelVertex2>& out_vertices)
{
	std::map<PackedVertex, unsigned short> VertexToOutIndex;

	// For each input vertex
	for (unsigned int i = 0; i < in_vertices.size(); ++i)
	{
		// If not, it needs to be added in the output data.
		ModelVertex2 v;
		v.pos = glm::vec3(in_vertices[i].x, in_vertices[i].y, in_vertices[i].z);
		v.normal = glm::vec3(in_normals[i].x, in_normals[i].y, in_normals[i].z);
		v.colour = glm::vec4(1.0f);
		v.texCoord = glm::vec2(in_uvs[i].x, in_uvs[i].y);

		out_vertices.push_back(v);
	}
}
