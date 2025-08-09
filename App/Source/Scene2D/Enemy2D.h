/**
 CEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
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
class CTopdee;

#include "InventoryManager.h"

class CEnemy2D : public CEntity2D
{
public:
	// Constructor
	CEnemy2D(void);

	// Destructor
	virtual ~CEnemy2D(void);

	// Init
	virtual bool Init(void);

	// Update
	virtual bool Update(const double dElapsedTime);

	// PreRender
	virtual void PreRender(void);

	// Render
	virtual void Render(void);

	// PostRender
	virtual void PostRender(void);

	// Set the handle to cPlayer to this class instance
	void SetPlayer2D(CPlayer2D* pPlayer2D);

	// boolean flag to indicate if this enemy is active
	bool bIsActive;

	// PrintSelf
	virtual void PrintSelf(string className);

	// The vec2 which stores the halfsize of an Entity2D in the Map2D; with reference to the origin which is the centre.
	const glm::vec2 vec2HalfSize = glm::vec2(12.5f, 12.5f);

protected:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};
	
	glm::vec2 vec2JumpSpeed = glm::vec2(0.0f, 250.0f);
	glm::vec2 vec2WalkSpeed = glm::vec2(100.0f, 100.0f);

	glm::vec2 vec2MovementVelocity;

	// The vec2 which stores the indices of the destination for enemy2D in the Map2D
	glm::vec2 vec2Destination;
	// The vec2 which stores the direction for enemy2D movement in the Map2D
	glm::vec2 vec2Direction;

	// Handler to the CMap2D instance
	CMap2D* pMap2D;

	//CS: Animated Sprite
	CSpriteAnimation* pAnimatedSprites;

	// Physics
	CPhysics2D cPhysics2D;

	// Handle to the CPlayer2D
	CPlayer2D* pPlayer2D;

	CTopdee* pTopdee;

	CInventoryManager* pInventoryManager;
	// InventoryItem
	CInventoryItem* pInventoryItem;

	//// FSM counter - count how many frames it has been in this FSM
	//int iFSMCounter;

	//// Max count in a state
	//const int iMaxFSMCounter = 60;

	// track how long has it been in this FSM
	float stateTimer = 0.0f;

	// Let enemy2D interact with the player
	virtual bool InteractWithPlayer(void) = 0;

	// Let enemy2D interact with the map
	virtual void InteractWithMap() = 0;

	// Update direction
	virtual void UpdateDirection(void) = 0;

	// Flip horizontal direction. For patrol use only
	void FlipHorizontalDirection(void);

	// Update position
	virtual void UpdatePosition(void);

	// Calculate Direction using coordinates, not indices
	glm::vec2 CalculateDirection(const glm::vec2 vec2StartPosition, const glm::vec2 vec2EndPosition);

	virtual void UpdateFSM();

	virtual bool IsFlying(); // override this if enemy is supposed to be flying

	double dElapsedTimeSinceLastPathFind = 0.f;
};

