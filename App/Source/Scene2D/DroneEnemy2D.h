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

class CDroneEnemy2D : public CEnemy2D
{
public:
	// Constructor
	CDroneEnemy2D(void);

	// Destructor
	 ~CDroneEnemy2D(void) override;

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
		IDLE = 0,
		PATROL = 1,
		CHASE = 2,
		SEARCH = 3,
		DISABLED = 4,
		REENABLE = 5,
		RETURN = 6,
		NUM_FSM
	};

	// Current FSM
	FSM sCurrentFSM;

	float chaseMultiplier = 2.f;
	
	// max amt of time in each state
	float maxIdleTime = 4.0f;
	float maxPatrolTime = 10.0f;

	// store patrol points
	std::vector<glm::vec2> patrolPoints; // patrol waypoints (world coord)
	int currentPatrolPointIndex = 0;
	float patrolThreshold = 2.f;
	bool reversePatrolPath = false;

	bool isHit = false;
	glm::vec2 closestProjectilePos = glm::vec2(FLT_MAX, FLT_MAX);  // Init to something far away
	float closestProjectileDist = FLT_MAX;
	float maxDisabledTime = 4.f;

	float flyUpAmount = 50.f;
	bool hasSetTargetHeight = false;
	float targetYHeight;

	float normalDetectionRadius = 70.f;
	float chaseDetectionRadius = 250.f;
	float bulletDetectionRadius = 30.f;

	float maxSearchTime = 5.f;

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

	void SetupPatrolPoints(std::vector<glm::vec2> points);
};

