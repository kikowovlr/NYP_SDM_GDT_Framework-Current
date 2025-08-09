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

// store block info
struct BlockInfo
{
	glm::vec2 defaultPos;
	glm::vec2 currentPos;
	bool hasMoved = false;
	bool isInInventory = false;
};