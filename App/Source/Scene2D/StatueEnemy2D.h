#pragma once

// Include shader
#include "RenderControl\shader.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include the Map2D as we will use it to check the player's movements and actions
class CMap2D;

// Include Physics2D
#include "Physics2D.h"

// Include Player2D
class CPlayer2D;

#include "InventoryManager.h"

#include "Enemy2D.h"

#include "ProjectileManager2D.h"

class CStatueEnemy2D : public CEnemy2D
{
public:
	// Constructor
	CStatueEnemy2D(void);

	// Destructor
	 ~CStatueEnemy2D(void) override;

	// Init
	bool Init(void) override;

	// Update
	bool Update(const double dElapsedTime) override;

	// PreRender
	void PreRender(void) override;

	// Render
	void Render(void) override;

	// PostRender
	void PostRender(void) override;

	// PrintSelf
	void PrintSelf() override;

protected:
	enum FSM
	{
		INACTIVE = 0,
		LOOKAROUND = 1,
		CHASE = 2,
		STUNNED = 3,
		DEAD = 4,
		RETURN = 5,
		NUM_FSM
	};

	// Current FSM
	FSM sCurrentFSM;

	glm::vec2 initialPos;

	float runMultiplier = 2.f;
	
	// max amt of time in each state
	float maxIdleTime = 4.0f;
	float maxPatrolTime = 5.0f;
	
	float detectionRadius = 100.f;

	float deadDuration = 5.f;

	bool isStunned = false;
	float maxStunnedTime = 5.f;
	float maxSearchTime = 7.f;

	float positionThreshold = 2.f;

	bool isHit = false;
	float hitCD = 0.f;
	float attackCooldown = 0.f;

	float maxReturnTime = 8.f;

	// Let enemy2D interact with the player
	bool InteractWithPlayer(void) override;

	// Let enemy2D interact with the map
	void InteractWithMap() override;

	// Update direction
	void UpdateDirection(void) override;

	// Update position
	void UpdatePosition(void) override;

	void UpdateSpriteAnimation() override;

	void UpdateFSM(float dElapsedTime) override;
};

