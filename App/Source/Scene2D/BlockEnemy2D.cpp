#include "BlockEnemy2D.h"

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

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CBlockEnemy2D::CBlockEnemy2D(void)
	: CEnemy2D()
	, sCurrentFSM(FSM::IDLE)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CBlockEnemy2D::~CBlockEnemy2D(void)
{
}

/**
  @brief Initialise this instance
  @return true is the initialisation is successful, otherwise false
  */
bool CBlockEnemy2D::Init(void)
{
	CEnemy2D::Init();

	// Load this enemy's texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/blocksmith.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/blocksmith.png" << endl;
		return false;
	}

	//CS: Create the animated sprite and setup the animation 
	pAnimatedSprites = CMeshBuilder::GenerateSpriteAnimation(12, 4);
	pAnimatedSprites->AddAnimation("idle-left", 0, 3);
	pAnimatedSprites->AddAnimation("idle-right", 4, 7);
	pAnimatedSprites->AddAnimation("idle-front", 8, 11);
	pAnimatedSprites->AddAnimation("idle-back", 12, 15);
	pAnimatedSprites->AddAnimation("holdblock-left", 16, 19);
	pAnimatedSprites->AddAnimation("holdblock-right", 20, 23);
	pAnimatedSprites->AddAnimation("holdblock-front", 24, 27);
	pAnimatedSprites->AddAnimation("holdblock-back", 28, 31);
	pAnimatedSprites->AddAnimation("surprised-left", 32, 35);
	pAnimatedSprites->AddAnimation("surprised-right", 36, 39);
	pAnimatedSprites->AddAnimation("surprised-front", 40, 43);
	pAnimatedSprites->AddAnimation("surprised-back", 44, 47);
	//CS: Play the "idle" animation as default
	pAnimatedSprites->PlayAnimation("idle-left", -1, 1.5f);
	
	pInventoryManager->BindToCharacter(this);
	pInventoryItem = pInventoryManager->Add("Health", "Image/Scene2D_Health.tga", 100, 100);
	pInventoryItem->vec2Size = glm::vec2(25, 25);

	// Set status to idle by default
	cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
	cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);

	SetName("BlockSmith");

	vec2JumpSpeed = glm::vec2(0.0f, 250.0f);
	vec2WalkSpeed = glm::vec2(100.0f, 100.0f);

	return true;
}

/**
 @brief Update this instance
 @param dElapsedTime A const double variable containing the elapsed time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
 */
bool CBlockEnemy2D::Update(const double dElapsedTime)
{
	if (!bIsActive)
		return false;

	CEnemy2D::Update(dElapsedTime);

	return true;
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CBlockEnemy2D::PreRender(void)
{
	CEnemy2D::PreRender();
}

/**
 @brief Render this instance
 */
void CBlockEnemy2D::Render(void)
{
	if (!bIsActive)
		return;

	CEnemy2D::Render();
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CBlockEnemy2D::PostRender(void)
{
	if (!bIsActive)
		return;

	CEnemy2D::PostRender();
}

/**
 @brief PrintSelf
 */ 
void CBlockEnemy2D::PrintSelf(string className)
{
	CEnemy2D::PrintSelf(className);
	cout << "sCurrentFSM\t=\t" << sCurrentFSM << endl;
}

/**
 @brief Let enemy2D interact with the player.
 */
bool CBlockEnemy2D::InteractWithPlayer(void)
{
	// Check if the enemy2D is within 1 tile size of the player2D
	if (glm::distance(vec2Position, pPlayer2D->vec2Position) <= glm::length(vec2HalfSize) * 2.0f)
	{
		cout << "Gotcha!" << endl;
		pInventoryManager->BindToCharacter(this);
		pInventoryItem = pInventoryManager->GetItem("Health");
		pInventoryItem->Remove(20);
		// Since the player has been caught, then reset the FSM
		sCurrentFSM = IDLE;
		stateTimer = 0.0f;
		return true;
	}

	return false;
}

void CBlockEnemy2D::InteractWithMap() 
{
	int iPositionX = 0;
	int iPositionY = 0;
	if (pMap2D->GetTileIndexAtPosition(vec2Position, iPositionX, iPositionY) == false)
		return;

	switch (pMap2D->GetMapInfo(iPositionY, iPositionX))
	{
	case 21: // health pack
		pMap2D->SetMapInfo(iPositionY, iPositionX, 0);
		// Increase the health
		pInventoryItem = pInventoryManager->GetItem("Health");
		pInventoryItem->Add(20);
		break;
	case 28: // spike
		pInventoryItem = pInventoryManager->GetItem("Health");
		pInventoryItem->Remove(1);
		break;
	default:
		break;
	}
}

/**
 @brief Update the enemy's direction.
 */
void CBlockEnemy2D::UpdateDirection(void)
{
	if (glm::distance(vec2Position, pPlayer2D->vec2Position) > glm::distance(vec2Position, pTopdee->vec2Position))
	{
		// if toodee closer, set destination as toodee's pos
		vec2Destination = pPlayer2D->vec2Position;
	}
	else
	{
		// else set destination as topdee
		vec2Destination = pTopdee->vec2Position;
	}

	// Calculate the direction between enemy2D and player2D
	vec2Direction = CalculateDirection(vec2Position, vec2Destination);
}

/**
@brief Update position.
*/
void CBlockEnemy2D::UpdatePosition(void)
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

void CBlockEnemy2D::UpdateFSM()
{

}
