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

class CBlockEnemy2D : public CEnemy2D
{
public:
	// Constructor
	CBlockEnemy2D(void);

	// Destructor
	 ~CBlockEnemy2D(void) override;

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

	// boolean flag to indicate if this enemy is active
	bool bIsActive;

	// PrintSelf
	void PrintSelf(string className) override;

protected:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	enum FSM
	{
		IDLE = 0,
		PATROL = 1,
		FIND_BLOCK = 2,
		MOVE_BLOCK = 3,
		PLACE_BLOCK = 4,
		SURPRISED = 5,
		FLEE = 6,
		WEARY = 7,
		RETURN = 8,
		NUM_FSM
	};

	// Current FSM
	FSM sCurrentFSM;
	
	// max amt of time in each state
	float maxIdleTime = 4.0f;
	float maxPatrolTime = 4.0f;

	// Let enemy2D interact with the player
	bool InteractWithPlayer(void) override;

	// Let enemy2D interact with the map
	void InteractWithMap() override;

	// Update direction
	void UpdateDirection(void) override;

	// Update position
	void UpdatePosition(void);

	void UpdateFSM() override;
};

