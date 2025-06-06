#include "Mesh.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

/**
 @brief Constructor
 */
CMesh::CMesh(void)
	: mode(DRAW_TRIANGLES)
	, VAO(0)
	, VBO(0)
	, IBO(0)
	, indexSize(0)
	, NumOfInstance(0)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &IBO);

	// Disable the VertexAttribArray by default
	for (int i = 0; i < 8; i++)
		arrVertexAttribArrayEnabled[i] = false;
}

/**
 @brief Destructor
 */
CMesh::~CMesh(void)
{
	// Disable the VertexAttribArray
	for (int i = 0; i < 8; i++)
	{
		if (arrVertexAttribArrayEnabled[i] == true)
			glDisableVertexAttribArray(i);
	}

	glDeleteBuffers(1, &IBO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

/**
 @brief Init this mesh
 */ 
void CMesh::Init(void)
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3)));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec4)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	if (mode == DRAW_LINES)
		glDrawElements(GL_LINES, indexSize, GL_UNSIGNED_INT, 0);
	else if (mode == DRAW_TRIANGLE_STRIP)
		glDrawElements(GL_TRIANGLE_STRIP, indexSize, GL_UNSIGNED_INT, 0);
	else if (mode == DRAW_TRIANGLES_ARRAY)
		glDrawArrays(GL_TRIANGLES, 0, indexSize);
	else
		glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}

// Set methods
void CMesh::SetVertexAttribArrayEnabled(const int index, const bool bEnabled)
{
	arrVertexAttribArrayEnabled[index] = bEnabled;
}

// Get methods
int CMesh::GetVertexAttribArrayEnabled(const int index)
{
	return arrVertexAttribArrayEnabled[index];
}

/**
 @brief Render this mesh
 */
void CMesh::Render(void)
{
	//glEnableVertexAttribArray(0);
	//glEnableVertexAttribArray(1);
	//glEnableVertexAttribArray(2);
	//glEnableVertexAttribArray(3);

	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
	//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3)));
	//glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec4)));
	//
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glBindVertexArray(VAO);

	if (mode == DRAW_LINES)
		glDrawElements(GL_LINES, indexSize, GL_UNSIGNED_INT, 0);
	else if (mode == DRAW_TRIANGLE_STRIP)
		glDrawElements(GL_TRIANGLE_STRIP, indexSize, GL_UNSIGNED_INT, 0);
	else if (mode == DRAW_TRIANGLES_ARRAY)
		glDrawArrays(GL_TRIANGLES, 0, indexSize);
	else if (mode == DRAW_TRIANGLES_INSTANCED)
		glDrawElementsInstanced(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0, NumOfInstance);
	else
		glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

	//glDisableVertexAttribArray(3);
	//glDisableVertexAttribArray(2);
	//glDisableVertexAttribArray(1);
	//glDisableVertexAttribArray(0);
}