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

#include "Map2D.h"
#include <vector>

// store block info
struct BlockInfo
{
    glm::vec2 defaultPos;
    glm::vec2 currentPos;
    bool hasMoved = false;
    bool isInInventory = false;
};

extern std::vector<BlockInfo> crateInfos;

// Declare the function
void InitializeBlockInfos(const CMap2D* pMap);