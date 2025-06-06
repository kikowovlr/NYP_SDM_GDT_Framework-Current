/**
 Mesh.h
 By: Lim Chian Song
 Date: Apr 2021
 */
#ifndef MESH_H
#define MESH_H

#include <includes/glm.hpp>

/**
	Vertex Class
	Used for 2D texture.
	Create a version with normals for 3D normal
*/
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec4 colour;
	glm::vec2 texCoord;
	Vertex()
	{
		position = glm::vec3();
		normal = glm::vec3(0.0f, 0.0f, 1.0f);
		colour = glm::vec4(1.0);
		texCoord = glm::vec2();
	}
};

/**
	Mesh 
	Used for generating mesh for rendering
	Contains the vertex buffer and index buffer
*/
class CMesh
{
public:
	enum DRAW_MODE
	{
		DRAW_TRIANGLES, //default mode
		DRAW_TRIANGLES_ARRAY,
		DRAW_TRIANGLE_STRIP,
		DRAW_LINES,
		DRAW_TRIANGLES_INSTANCED,
		DRAW_MODE_LAST,
	};

	unsigned int VAO;
	unsigned int VBO;
	unsigned int IBO;
	unsigned int indexSize;
	unsigned int NumOfInstance;

	DRAW_MODE mode;

	// Constructor
	CMesh(void);
	// Destructor
	~CMesh(void);
	// Init
	virtual void Init(void);

	// Set methods
	virtual void SetVertexAttribArrayEnabled(const int index, const bool bEnabled);
	// Get methods
	virtual int GetVertexAttribArrayEnabled(const int index);

	// Render
	virtual void Render(void);

protected:
	bool arrVertexAttribArrayEnabled[8];
};

#endif