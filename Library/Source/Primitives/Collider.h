/**
 CCollider
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

#include <string>
using namespace std;

class CCollider
{
public:
	// Attributes of the Shader
	glm::mat4 colliderModel; // make sure to initialize matrix to identity matrix first
	glm::mat4 colliderView;
	glm::mat4 colliderProjection;

	// Bounding Box specifications
	// Bounding Box bottom left position
	glm::vec3 vec3BoxMin;
	// Bounding Box top right position
	glm::vec3 vec3BoxMax;
	// Bounding Box colour
	glm::vec4 vec4BoxColour;

	// Boolean flag to indicate if Bounding Box is displayed
	bool bIsDisplayed;
	// Line width
	float fLineWidth;

	// Constructor(s)
	CCollider(void);
	// Destructor
	virtual ~CCollider(void);

	// Activate the CCollider for this class instance
	virtual bool InitCollider(	const std::string& _name,
								glm::vec4 vec4BoxColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
								glm::vec3 vec3BoxMin = glm::vec3(-0.5f, -0.5f, -0.5f),
								glm::vec3 vec3BoxMax = glm::vec3(0.5f, 0.5f, 0.5f));

	// Set a shader to this class instance
	virtual void SetColliderShader(const std::string& _name);

	// PreRender
	virtual void PreRender(void);
	// Render
	virtual void Render(void);
	// PostRender
	virtual void PostRender(void);

protected:
	std::string sColliderShaderName;
	unsigned int colliderVAO, colliderVBO;
};

