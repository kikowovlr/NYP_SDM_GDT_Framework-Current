#include "StatueEnemy2D.h"

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
#include "BlockInfo.h"
#include "Enemy2DManager.h"


/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CStatueEnemy2D::CStatueEnemy2D(void)
	: CEnemy2D()
	, sCurrentFSM(FSM::INACTIVE)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CStatueEnemy2D::~CStatueEnemy2D(void)
{
}

/**
  @brief Initialise this instance
  @return true is the initialisation is successful, otherwise false
  */
bool CStatueEnemy2D::Init(void)
{
	CEnemy2D::Init();

	SetName("Statue");

	// Load this enemy's texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/statue.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/statue.png" << endl;
		return false;
	}

	//CS: Create the animated sprite and setup the animation 
	pAnimatedSprites = CMeshBuilder::GenerateSpriteAnimation(3, 3);
	pAnimatedSprites->AddAnimation("idle", 0, 2);
	pAnimatedSprites->AddAnimation("chase-right", 3, 5);
	pAnimatedSprites->AddAnimation("chase-left", 6, 8);
	//CS: Play the "idle" animation as default
	pAnimatedSprites->PlayAnimation("idle", -1, 10.f);
	
	pInventoryManager->BindToCharacter(this);
	pInventoryItem = pInventoryManager->Add("Health", "Image/Scene2D_Health.tga", 100, 100);
	pInventoryItem->vec2Size = glm::vec2(25, 25);

	// Set status to idle by default
	cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
	cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);
	cPhysics2D.SetWallJumpStatus(CPhysics2D::WALLJUMPSTATUS::NOT_ATTACHED);

	vec2WalkSpeed = glm::vec2(30.0f, 30.0f);
	vec2JumpSpeed = glm::vec2(70.f, 70.f);

	sCurrentFSM = INACTIVE;

	initialPos = vec2Position;

	return true;
}

/**
 @brief Update this instance
 @param dElapsedTime A const double variable containing the elapsed time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
 */
bool CStatueEnemy2D::Update(const double dElapsedTime)
{
	if (!bIsActive)
		return false;

	// Reset vec2MovementVelocity
	vec2MovementVelocity = glm::vec2(0.0f);
	// Set the physics horizontal status to idle
	cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
	if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::WALK)
		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);

	// update bullet positions
	for (auto* projectile : pProjectileManager2D->vProjectile)
	{
		if (!projectile || !projectile->GetStatus()) continue;

		glm::vec2 projectilePos = projectile->vec2Position;

		float distFromBullet = glm::distance(vec2Position, projectilePos);

		if (distFromBullet < 15.f)
		{
			isHit = true;
			projectile->SetStatus(false);
			break;
		}
	}

	UpdateFSM(dElapsedTime); // update fsm states

	// Calculate the physics for JUMP/DOUBLE JUMP/FALL movement
	if ((cPhysics2D.GetVerticalStatus() >= CPhysics2D::VERTICALSTATUS::JUMP)
		&& (cPhysics2D.GetVerticalStatus() <= CPhysics2D::VERTICALSTATUS::FALL))
	{
		// Update the elapsed time to the physics engine
		cPhysics2D.AddElapsedTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics2D.Update(dElapsedTime);
		// Get the displacement from the physics engine and update the player position
		vec2MovementVelocity += cPhysics2D.GetFinalVelocity();
	
		// Set the physics vertical status from jump/double jump to fall if the movement direction changes to negative
		if ((cPhysics2D.GetVerticalStatus() >= CPhysics2D::VERTICALSTATUS::JUMP)
			&& (cPhysics2D.GetVerticalStatus() <= CPhysics2D::VERTICALSTATUS::DOUBLEJUMP))
		{
			if (cPhysics2D.GetFinalVelocity().y < 0.0f)
				cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL, false);
		}
	}
	
	// Update vec2Position
	glm::vec2 vec2NewPosition = vec2Position + vec2MovementVelocity * (float)dElapsedTime;
	// For calculating the collision point's x-coordinate
	float fCollisionCoordX = 0;
	// For calculating the collision point's y-coordinate
	float fCollisionCoordY = 0;

	// Check for collision with the Tile Maps vertically
	if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::WALK)
	{
		// Check if the player walks into an obstacle
		if (pMap2D->CheckHorizontalCollision(vec2Position, vec2HalfSize, vec2NewPosition, fCollisionCoordX) == CSettings::RESULTS::POSITIVE)
		{
			cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
			// Flip the direction since this direction is blocked
			FlipHorizontalDirection();
		}

		if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
		{
			// Check if he is walking on air; let him fall down
			glm::vec2 vec2InAirPosition = vec2Position - glm::vec2(0.0f, vec2HalfSize.y);
			if (pMap2D->CheckVerticalCollision(vec2Position, vec2HalfSize, vec2InAirPosition, fCollisionCoordY) == CSettings::RESULTS::NEGATIVE)
			{
				cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);
			}
		}
	}

	// Check for collision with the Tile Maps vertically
	if ((cPhysics2D.GetVerticalStatus() >= CPhysics2D::VERTICALSTATUS::JUMP) &&
		(cPhysics2D.GetVerticalStatus() <= CPhysics2D::VERTICALSTATUS::DOUBLEJUMP) &&
		(pMap2D->CheckVerticalCollision(vec2Position, vec2HalfSize, vec2NewPosition, fCollisionCoordY) == CSettings::RESULTS::POSITIVE))
	{
		if ((cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::JUMP) || (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::DOUBLEJUMP))
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);
		}
	}

	// Check for collision with the Tile Maps vertically
	if ((cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::FALL) &&
		(pMap2D->CheckVerticalCollision(vec2Position, vec2HalfSize, vec2NewPosition, fCollisionCoordY) == CSettings::RESULTS::POSITIVE))
	{
		if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::FALL)
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);
		}
	}

	// Update the vec2Position with the new position
	vec2Position = vec2NewPosition;

	// Constraint the enemy within the map
	if (pMap2D->Constraint(vec2Position) == true)
	{
		if (vec2MovementVelocity.y > 0.0f)
		{
			vec2MovementVelocity.y = 0.0f;
		}
		if ((cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::JUMP) || (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::DOUBLEJUMP))
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);
		}
		else if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::FALL)
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);
		}
	}

	// minus health when hit
	if (isHit && hitCD <= 0.f)
	{
		// Reset cooldown so can't take damage again immediately
		isHit = false;
		hitCD = 0.5f;

		// Reduce health
		pInventoryManager->BindToCharacter(this);
		pInventoryItem = pInventoryManager->GetItem("Health");
		pInventoryItem->Remove(20);

	}

	// Countdown cooldown timer
	if (hitCD > 0.f)
	{
		hitCD -= dElapsedTime;
	}
	if (attackCooldown > 0.f)
	{
		attackCooldown -= dElapsedTime;
	}

	CEnemy2D::Update(dElapsedTime); // at the end due to update model

	return true;
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CStatueEnemy2D::PreRender(void)
{
	CEnemy2D::PreRender();
}

/**
 @brief Render this instance
 */
void CStatueEnemy2D::Render(void)
{
	if (!bIsActive)
		return;

	CEnemy2D::Render();

	// note: currently we set the projection matrix each frame, but since the projection 
	// matrix rarely changes it's often best practice to set it outside the main loop only once.
	CShaderManager::GetInstance()->pActiveShader->setMat4("Model", model);
	CShaderManager::GetInstance()->pActiveShader->setMat4("Projection", projection);
	unsigned int colourLoc = glGetUniformLocation(CShaderManager::GetInstance()->pActiveShader->ID, "ColourTint");

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);
	//CS: Render the animated sprite
	glBindVertexArray(VAO);
	pAnimatedSprites->Render();
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CStatueEnemy2D::PostRender(void)
{
	if (!bIsActive)
		return;

	CEnemy2D::PostRender();
}

/**
 @brief PrintSelf
 */ 
void CStatueEnemy2D::PrintSelf()
{
	cout << "Statue ::PrintSelf()" << endl;
	cout << "=======================" << endl;
	CEnemy2D::PrintSelf();
	cout << "sCurrentFSM\t=\t" << sCurrentFSM << endl;
}

/**
 @brief Let enemy2D interact with the player.
 */
bool CStatueEnemy2D::InteractWithPlayer(void)
{
	// Check if the enemy2D is within 1 tile size of the player2D
	if (glm::distance(vec2Position, pPlayer2D->vec2Position) <= 12.5f && attackCooldown <= 0.f)
	{
		cout << "Gotcha!" << endl;
		attackCooldown = 1.5f;
		pInventoryManager->BindToCharacter(nullptr);
		pInventoryItem = pInventoryManager->GetItem("Health");
		pInventoryItem->Remove(30);
		return true;
	}

	return false;
}

void CStatueEnemy2D::InteractWithMap() 
{
	int iPositionX = 0;
	int iPositionY = 0;
	if (pMap2D->GetTileIndexAtPosition(vec2Position, iPositionX, iPositionY) == false)
		return;

	switch (pMap2D->GetMapInfo(iPositionY, iPositionX))
	{
	default:
		break;
	}
}

/**
 @brief Update the enemy's direction.
 */
void CStatueEnemy2D::UpdateDirection(void)
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
void CStatueEnemy2D::UpdatePosition(void)
{
	glm::vec2 movementSpeed = isRunning ? vec2WalkSpeed * runMultiplier : vec2WalkSpeed;

	// Calculate the vec2MovementVelocity
	if (vec2Direction.x < 0)
	{
		// Move left
		vec2MovementVelocity.x -= movementSpeed.x;
		if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::IDLE)
		{
			cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::WALK);
		}
	}
	else if (vec2Direction.x > 0)
	{
		// Move right
		vec2MovementVelocity.x += movementSpeed.x;
		if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::IDLE)
		{
			cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::WALK);
		}
	}
	else
	{
		vec2MovementVelocity.x = 0.0f;
	}

	if (vec2Direction.y < 0)
	{
		// Move down
		//vec2MovementVelocity.y -= movementSpeed.y;
		if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);
		}
	}
	else if (vec2Direction.y > 0)
	{
		vec2MovementVelocity.y += movementSpeed.y;
		if (cPhysics2D.GetVerticalStatus() <= CPhysics2D::VERTICALSTATUS::IDLE)
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::JUMP);
			cPhysics2D.SetInitialVelocity(vec2JumpSpeed);
			cPhysics2D.SetNewJump(true);
		}
	}
}

void CStatueEnemy2D::UpdateSpriteAnimation()
{
	switch (eFacingDirection)
	{
		case DIRECTION::LEFT:  pAnimatedSprites->PlayAnimation("chase-left", -1, 1.0f); break;
		case DIRECTION::RIGHT: pAnimatedSprites->PlayAnimation("chase-right", -1, 1.0f); break;
	}
}

void CStatueEnemy2D::UpdateFSM(float dElapsedTime)
{
	// Get updates from AI
	switch (sCurrentFSM)
	{
	case INACTIVE:
		pAnimatedSprites->PlayAnimation("idle", -1, 10.f);
		// activate if triggered by drone
		if (pEnemyManager2D->IsPlayerChased())
		{
			sCurrentFSM = CHASE;
			cout << "Switching to CHASE State" << endl;
		}
		break;
	case LOOKAROUND:
		// search for player, if cannot find then go to return state
		if (glm::distance(vec2Position, pPlayer2D->vec2Position) <= detectionRadius || pEnemyManager2D->IsPlayerChased())
		{
			sCurrentFSM = CHASE;
			stateTimer = 0.f;
			cout << "Switching to CHASE State" << endl;
		}
		else if (stateTimer > maxSearchTime)
		{
			sCurrentFSM = RETURN;
			stateTimer = 0.f;
			cout << "Switching to RETURN State" << endl;
		}
		stateTimer += dElapsedTime;
		break;
	case CHASE:
	{
		if (!isRunning)
			isRunning = true;

		// chase player
		int iEnemyTileX = 0;
		int iEnemyTileY = 0;
		int iTargetX = 0;
		int iTargetY = 0;

		if ((pMap2D->GetTileIndexAtPosition(vec2Position, iEnemyTileX, iEnemyTileY) == true) &&
			(pMap2D->GetTileIndexAtPosition(pPlayer2D->vec2Position, iTargetX, iTargetY) == true))
		{
			// calculate a path to the player
			auto path = pMap2D->PathFind(glm::vec2(iEnemyTileX, iEnemyTileY),
				glm::vec2(iTargetX, iTargetY),
				heuristic::euclidean,
				10);

			// extract points and determine dir for the enemy to travel towards
			// Calculate new destination
			bool bFirstPosition = true;
			glm::vec2 vec2PathPoint;
			for (const auto& coord : path)
			{
				// Take a path point from path via coord
				vec2PathPoint = coord;
				if (bFirstPosition == true)
				{
					// Set a destination
					vec2Destination = glm::vec2(vec2PathPoint.x * pMap2D->GetTileSize().x + pMap2D->GetTileHalfSize().x, vec2PathPoint.y * pMap2D->GetTileSize().y + pMap2D->GetTileHalfSize().y);
					// Calculate the direction between enemy2D and this destination
					vec2Direction = glm::normalize(vec2PathPoint - glm::vec2(iEnemyTileX, iEnemyTileY));
					bFirstPosition = false;
				}
				else
				{
					// If the next path point is in the same direction, use it as the new vec2Destination
					if (glm::normalize(vec2PathPoint - glm::vec2(iEnemyTileX, iEnemyTileY)) == vec2Direction)
					{
						// Set a destination
						vec2Destination = glm::vec2(vec2PathPoint.x * pMap2D->GetTileSize().x + pMap2D->GetTileHalfSize().x, vec2PathPoint.y * pMap2D->GetTileSize().y + pMap2D->GetTileHalfSize().y);
					}
					else
						break;
				}
			}
			UpdatePosition();
		}

		// if lose sight of player
		if (glm::distance(vec2Position, pPlayer2D->vec2Position) > detectionRadius)
		{
			sCurrentFSM = LOOKAROUND;
			isRunning = false;
			cout << "Switching to LOOKAROUND State" << endl;
		}
		else
		{
			pInventoryManager->BindToCharacter(this);
			pInventoryItem = pInventoryManager->GetItem("Health");

			// no health -> switch to die state
			if (pInventoryItem->GetCount() <= 0)
			{
				sCurrentFSM = DEAD;
				isRunning = false;
				cout << "Switching to DEAD State" << endl;
			}
			// half health
			else if (pInventoryItem->GetCount() <= 50)
			{
				sCurrentFSM = STUNNED;
 				isRunning = false;
				cout << "Switching to STUNNED State" << endl;
			}
		}
		break;
	}
	case STUNNED:
		if (stateTimer > maxStunnedTime)
		{
			sCurrentFSM = LOOKAROUND;
			stateTimer = 0.f;
		}
		stateTimer += dElapsedTime;
		break;
	case DEAD:
		vec2Direction = glm::vec2(0);
		vec2MovementVelocity = glm::vec2(0);
		if (stateTimer > deadDuration)
		{
			stateTimer = 0.f;
			bStatus = false;
		}
		stateTimer += dElapsedTime;
		break;
	case RETURN:
	{
		// return to last patrol index
		glm::vec2 returnPoint = initialPos;
		float dist = glm::distance(vec2Position, returnPoint);

		// Get tile indices for enemy and patrol point
		int iEnemyTileX, iEnemyTileY;
		int iTargetX, iTargetY;

		if (pMap2D->GetTileIndexAtPosition(vec2Position, iEnemyTileX, iEnemyTileY) &&
			pMap2D->GetTileIndexAtPosition(returnPoint, iTargetX, iTargetY))
		{
			// Pathfind from enemy tile to patrol tile
			auto path = pMap2D->PathFind(
				glm::vec2(iEnemyTileX, iEnemyTileY),
				glm::vec2(iTargetX, iTargetY),
				heuristic::euclidean,
				10);

			if (!path.empty())
			{
				bool bFirstPosition = true;
				glm::vec2 vec2PathPoint;
				for (const auto& coord : path)
				{
					vec2PathPoint = coord;
					if (bFirstPosition)
					{
						vec2Destination = glm::vec2(
							vec2PathPoint.x * pMap2D->GetTileSize().x + pMap2D->GetTileHalfSize().x,
							vec2PathPoint.y * pMap2D->GetTileSize().y + pMap2D->GetTileHalfSize().y);

						vec2Direction = glm::normalize(vec2PathPoint - glm::vec2(iEnemyTileX, iEnemyTileY));
						bFirstPosition = false;
					}
					else
					{
						// Only update destination if still moving in same direction
						if (glm::normalize(vec2PathPoint - glm::vec2(iEnemyTileX, iEnemyTileY)) == vec2Direction)
						{
							vec2Destination = glm::vec2(
								vec2PathPoint.x * pMap2D->GetTileSize().x + pMap2D->GetTileHalfSize().x,
								vec2PathPoint.y * pMap2D->GetTileSize().y + pMap2D->GetTileHalfSize().y);
						}
						else
							break;
					}
				}
				UpdatePosition();
			}
		}
		cout << dist << endl;
		if (glm::distance(vec2Position, pPlayer2D->vec2Position) < detectionRadius)
		{
			sCurrentFSM = CHASE;
			stateTimer = 0.f;
			cout << "Switching to CHASE state" << endl;
		}
		// Check if reached the patrol point
		else if (dist < 10.f || stateTimer > maxReturnTime)
		{
			// Once reached, switch back to patrol and continue normally
			sCurrentFSM = INACTIVE;
			stateTimer = 0.f;
			cout << "Switching to INACTIVE state" << endl;
		}
		stateTimer += dElapsedTime;
		break;
	}
	default:

		break;
	}
}
