/**
 CEnemy2D
 @brief A class which represents the enemy object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Enemy2D.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"
// Include Mesh Builder
#include "Primitives/MeshBuilder.h"

// Include GLEW
#include <GL/glew.h>

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
// Include math.h
#include <math.h>

// Include Player2D
#include "Player2D.h"
#include "Topdee.h"
#include "Enemy2DManager.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CEnemy2D::CEnemy2D(void)
	: bIsActive(false)
	, vec2Destination(glm::vec2(0, 0))
	, vec2Direction(glm::vec2(0, 0))
	, vec2MovementVelocity(glm::vec2(1, 1))
	, pMap2D(NULL)
	, pPlayer2D(NULL)
	, pInventoryManager(NULL)
	, pInventoryItem(NULL)
	, pTopdee(NULL)
	, pAnimatedSprites(NULL)
	, pProjectileManager2D(NULL)
{
	// Initialise position of the enemy
	vec2Position = glm::vec2(0);
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CEnemy2D::~CEnemy2D(void)
{
	// Delete the quadMesh
	if (p2DMesh)
	{
		delete p2DMesh;
		p2DMesh = NULL;
	}

	// We won't delete this since it was created elsewhere
	pPlayer2D = NULL;

	// We won't delete this since it was created elsewhere
	pMap2D = NULL;

	// We won't delete this since it was created elsewhere
	pInventoryManager = NULL;

	pInventoryItem = NULL;

	pTopdee = NULL;

	pAnimatedSprites = NULL;

	pProjectileManager2D = NULL;
}

/**
  @brief Initialise this instance
  @return true is the initialisation is successful, otherwise false
  */
bool CEnemy2D::Init(void)
{
	// Get the handler to the CSettings instance
	pSettings = CSettings::GetInstance();

	// Get the handler to the CMap2D instance
	pMap2D = CMap2D::GetInstance();

	// Get the handler to the CPlayer2D instance
	pPlayer2D = CPlayer2D::GetInstance();

	pTopdee = CTopdee::GetInstance();

	// Set up the projection matrix
	projection = glm::ortho(0.0f,
							CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowWidth", 800.0f),
							0.0f,
							CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f),
							-1.0f, 1.0f);


	//CS: Create the Quad Mesh using the mesh builder
	p2DMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), 1, 1);

	// Generate the VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//CS: Init the colour to white
	vec4ColourTint = glm::vec4(1.0, 1.0, 1.0, 1.0);

	pInventoryManager = CInventoryManager::GetInstance();

	pProjectileManager2D = CProjectileManager2D::GetInstance();

	pEnemyManager2D = CEnemy2DManager::GetInstance();

	// Set the Physics to fall status by default
	cPhysics2D.Init();

	// If this class is initialised properly, then set the bIsActive to true
	bIsActive = true;

	return true;
}

/**
 @brief Update this instance
 @param dElapsedTime A const double variable containing the elapsed time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
 */
bool CEnemy2D::Update(const double dElapsedTime)
{
	if (!bIsActive)
		return false;

	// Interact with the Player
	InteractWithPlayer();

	// Interact with the Map
	InteractWithMap();

	UpdateFacingDirection();

	// Update sprites
	UpdateSpriteAnimation();
	pAnimatedSprites->Update(dElapsedTime);

	// Update the model
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(vec2Position, 0.0f));
	model = glm::scale(model, glm::vec3(25.0f, 25.0f, 1.0f));

	return true;
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CEnemy2D::PreRender(void)
{
	if (!bIsActive)
		return;

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);

	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render this instance
 */
void CEnemy2D::Render(void)
{
	if (!bIsActive)
		return;
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(vec2Position, 0.0f));
	model = glm::scale(model, glm::vec3(25.0f, 25.0f, 1.0f));
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CEnemy2D::PostRender(void)
{
	if (!bIsActive)
		return;

	// Disable blending
	glDisable(GL_BLEND);
}

/**
 @brief Set the handle to cPlayer to this class instance
 @param pPlayer2D A CPlayer2D* variable which contains the pointer to the CPlayer2D instance
 */
void CEnemy2D::SetPlayer2D(CPlayer2D* pPlayer2D)
{
	this->pPlayer2D = pPlayer2D;

	// Update the enemy's direction
	UpdateDirection();
}

/**
 @brief PrintSelf
 */ 
void CEnemy2D::PrintSelf()
{
	cPhysics2D.PrintSelf();

	cout << "vec2Position\t=\t[" << vec2Position.x << ", " << vec2Position.y << "]" << endl;
	cout << "vec2Destination\t=\t[" << vec2Destination.x << ", " << vec2Destination.y << "]" << endl;
	cout << "vec2Direction\t=\t[" << vec2Direction.x << ", " << vec2Direction.y << "]" << endl;
	cout << "vec2MovementVelocity\t=\t[" << vec2MovementVelocity.x << ", " << vec2MovementVelocity.y << "]" << endl;
}

/**
 @brief Let enemy2D interact with the player.
 */
//bool CEnemy2D::InteractWithPlayer(void)
//{
//	// Check if the enemy2D is within 1 tile size of the player2D
//	if (glm::distance(vec2Position, pPlayer2D->vec2Position) <= glm::length(vec2HalfSize) * 2.0f)
//	{
//		cout << "Gotcha!" << endl;
//		pInventoryManager->BindToCharacter(this);
//		pInventoryItem = pInventoryManager->GetItem("Health");
//		pInventoryItem->Remove(20);
//		// Since the player has been caught, then reset the FSM
//		sCurrentFSM = IDLE;
//		stateTimer = 0.0f;
//		return true;
//	}
//
//	return false;
//}

//void CEnemy2D::InteractWithMap() 
//{
//	int iPositionX = 0;
//	int iPositionY = 0;
//	if (pMap2D->GetTileIndexAtPosition(vec2Position, iPositionX, iPositionY) == false)
//		return;
//
//	switch (pMap2D->GetMapInfo(iPositionY, iPositionX))
//	{
//	case 21: // health pack
//		pMap2D->SetMapInfo(iPositionY, iPositionX, 0);
//		// Increase the health
//		pInventoryItem = pInventoryManager->GetItem("Health");
//		pInventoryItem->Add(20);
//		break;
//	case 28: // spike
//		pInventoryItem = pInventoryManager->GetItem("Health");
//		pInventoryItem->Remove(1);
//		break;
//	default:
//		break;
//	}
//}

/**
 @brief Update the enemy's direction.
 */
//void CEnemy2D::UpdateDirection(void)
//{
//	if (glm::distance(vec2Position, pPlayer2D->vec2Position) > glm::distance(vec2Position, pTopdee->vec2Position))
//	{
//		// if toodee closer, set destination as toodee's pos
//		vec2Destination = pPlayer2D->vec2Position;
//	}
//	else
//	{
//		// else set destination as topdee
//		vec2Destination = pTopdee->vec2Position;
//	}
//
//	// Calculate the direction between enemy2D and player2D
//	vec2Direction = CalculateDirection(vec2Position, vec2Destination);
//}

/**
 @brief Flip horizontal direction. For patrol use only
 */
void CEnemy2D::FlipHorizontalDirection(void)
{
	vec2Direction.x *= -1;
}

void CEnemy2D::FlipVerticalDirection(void)
{
	vec2Direction.y *= -1;
}

/**
@brief Update position.
*/
void CEnemy2D::UpdatePosition(void)
{
	// Calculate the vec2MovementVelocity
	if (vec2Direction.x < 0)
	{
		// Move left
		vec2MovementVelocity.x -= vec2WalkSpeed.x;
		if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::IDLE)
			cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::WALK);
	}
	else if (vec2Direction.x > 0)
	{
		// Move right
		vec2MovementVelocity.x += vec2WalkSpeed.x;
		if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::IDLE)
			cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::WALK);
	}
	else
	{
		vec2MovementVelocity.x = 0.0f;
	}

	// if the player is above the enemy2D, then jump to attack
	if (vec2Direction.y > 0)
	{
		if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::JUMP);
			cPhysics2D.SetInitialVelocity(vec2JumpSpeed);
			cPhysics2D.SetNewJump(true);
		}
	}
}

void CEnemy2D::UpdateFacingDirection()
{
	if (vec2MovementVelocity.x > 0)
		eFacingDirection = DIRECTION::RIGHT;
	else if (vec2MovementVelocity.x < 0)
		eFacingDirection = DIRECTION::LEFT;
	else if (vec2MovementVelocity.y > 0)
		eFacingDirection = DIRECTION::UP;
	else if (vec2MovementVelocity.y < 0)
		eFacingDirection = DIRECTION::DOWN;
}


/**
@brief Calculate Direction using coordinates, not indices
*/
glm::vec2 CEnemy2D::CalculateDirection(const glm::vec2 vec2StartPosition, const glm::vec2 vec2EndPosition)
{
	int iStartPositionX = 0;
	int iStartPositionY = 0;
	int iEndPositionX = 0;
	int iEndPositionY = 0;
	if ((pMap2D->GetTileIndexAtPosition(vec2StartPosition, iStartPositionX, iStartPositionY) == true) &&
		(pMap2D->GetTileIndexAtPosition(vec2EndPosition, iEndPositionX, iEndPositionY) == true))
	{
		glm::vec2 vec2Direction = glm::normalize(glm::vec2(iEndPositionX, iEndPositionY) - glm::vec2(iStartPositionX, iStartPositionY));
		return vec2Direction;
	}
	return glm::vec2(0);
}