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

// save character state
struct TopdeeState {
	// Physics
	glm::vec2 position;
	glm::vec2 velocity;

	glm::vec4 colourTint;  // For visual effects
	bool isActive;         // bStatus

	//// Character-specific
	//float abilityCooldown;  // Example: TopDee's dash cooldown
	//bool isFacingRight;
};

// save character state
struct ToodeeState {
	// Physics
	glm::vec2 position;
	glm::vec2 velocity;

	glm::vec4 colourTint;  // For visual effects
	bool isActive;         // bStatus

	//// Character-specific
	//float abilityCooldown;  // Example: TopDee's dash cooldown
	//bool isFacingRight;
};
