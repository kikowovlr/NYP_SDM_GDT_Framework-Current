/**
 CTopdee
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include Singleton template
#include "DesignPatterns\SingletonTemplate.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"

// Include Keyboard controller
#include "Inputs\KeyboardController.h"
// Include Mouse controller
#include "Inputs\MouseController.h"

// Include Physics2D
#include "Physics2D.h"

// Include ProjectileManager2D
#include "ProjectileManager2D.h"
// Include ShootStatus
#include "ShootStatus.h"

// Include Physics2D
#include "Physics2D.h"

// Include AnimatedSprites
#include "Primitives/SpriteAnimation.h"

// Include InventoryManager
#include "InventoryManager.h"

#include "CharacterManager.h"
#include "CharacterStates.h"

#include "..\SoundController\SoundController.h"

class CTopdee : public CSingletonTemplate<CTopdee>, public CEntity2D
{
	friend CSingletonTemplate<CTopdee>;
public:
	// Init
	bool Init(void);

	// Reset
	bool Reset(void);

	// Update
	bool Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	TopdeeState SaveState() const;
	void LoadState(const TopdeeState& state);

	void Respawn() override;

	glm::ivec2 GetFacingDirection();
	void PickUpOrPutDownBlock();

	bool IsAtExit() const;

protected:
	// Constructor
	CTopdee(void);

	// Destructor
	virtual ~CTopdee(void);

	// Let player interact with the map
	void InteractWithMap(void);

	// Constant variable for walk speed
	const glm::vec2 vec2WalkSpeed = glm::vec2(100.0f, 100.0f);
	glm::vec2 vec2WalkSpeedMultiplier = glm::vec2(2.f, 2.f);

	// The vec2 which stores the halfsize of an Entity2D in the Map2D; with reference to the origin which is the centre.
	const glm::vec2 vec2HalfSize = glm::vec2(12.5f, 12.5f);
	glm::vec2 vec2MovementVelocity;

	// Handler to the CMap2D instance
	CMap2D* pMap2D;

	// Keyboard Controller singleton instance
	CKeyboardController* pKeyboardController;
	// Mouse Controller singleton instance
	CMouseController* pMouseController;

	// CProjectileManager2D singleton instance
	CProjectileManager2D* pProjectileManager2D;
	// CShootStatus
	CShootStatus cShootStatus;

	// Physics
	CPhysics2D cPhysics2D;

	//CS: Animated Sprite
	CSpriteAnimation* pAnimatedSprites;

	// Current colour
	glm::vec4 vec4ColourTint;

	// InventoryManager
	CInventoryManager* pInventoryManager;
	// InventoryItem
	CInventoryItem* pInventoryItem;

	CharacterManager* pCharacterManager;

	CSoundController* pSoundController;

	bool isAtExit = false;
};

