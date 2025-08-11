#include "DroneEnemy2D.h"

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
CDroneEnemy2D::CDroneEnemy2D(void)
	: CEnemy2D()
	, sCurrentFSM(FSM::IDLE)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CDroneEnemy2D::~CDroneEnemy2D(void)
{
}

/**
  @brief Initialise this instance
  @return true is the initialisation is successful, otherwise false
  */
bool CDroneEnemy2D::Init(void)
{
	CEnemy2D::Init();

	SetName("Drone");

	// Load this enemy's texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/drone.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/drone.png" << endl;
		return false;
	}

	//CS: Create the animated sprite and setup the animation 
	pAnimatedSprites = CMeshBuilder::GenerateSpriteAnimation(1, 3);
	pAnimatedSprites->AddAnimation("idle", 0, 2);
	//CS: Play the "idle" animation as default
	pAnimatedSprites->PlayAnimation("idle", -1, 1.0f);
	
	pInventoryManager->BindToCharacter(this);
	pInventoryItem = pInventoryManager->Add("Health", "Image/Scene2D_Health.tga", 1, 1);
	pInventoryItem->vec2Size = glm::vec2(25, 25);

	// Set status to idle by default
	cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
	cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);

	vec2WalkSpeed = glm::vec2(30.0f, 30.0f);

	// 2 sets of patrol tiles
	std::vector<glm::vec2> patrolTiles = {
		glm::vec2(21, 11),
		glm::vec2(25, 11),
	};

	//std::vector<glm::vec2> patrolTiles = {
	//glm::vec2(20, 18),
	//glm::vec2(28, 18),
	//};

	// convert to world coord
	for (const auto& tile : patrolTiles)
	{
		glm::vec2 worldPos = pMap2D->GetWorldPositionFromTile(tile);
		patrolPoints.push_back(worldPos);
	}
	SetupPatrolPoints(patrolPoints);

	sCurrentFSM = PATROL;

	return true;
}

/**
 @brief Update this instance
 @param dElapsedTime A const double variable containing the elapsed time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
 */
bool CDroneEnemy2D::Update(const double dElapsedTime)
{
	if (!bIsActive)
		return false;

	// Reset vec2MovementVelocity
	vec2MovementVelocity = glm::vec2(0.0f);
	// Set the physics horizontal status to idle
	cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
	if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::WALK)
		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);

	// Calculate the physics for FALL movement
	if (cPhysics2D.GetVerticalStatus() <= CPhysics2D::VERTICALSTATUS::FALL)
	{
		// Update the elapsed time to the physics engine
		cPhysics2D.AddElapsedTime((float)dElapsedTime);
		// Call the physics engine update method to calculate the final velocity and displacement
		cPhysics2D.Update(dElapsedTime);
		// Get the displacement from the physics engine and update the player position
		vec2MovementVelocity += cPhysics2D.GetFinalVelocity();
	}

	// reset before updating again
	closestProjectilePos = glm::vec2(FLT_MAX, FLT_MAX);  // Init to something far away
	closestProjectileDist = FLT_MAX;
	// update bullet positions
	for (auto* projectile : pProjectileManager2D->vProjectile)
	{
		if (!projectile || !projectile->GetStatus()) continue;

		glm::vec2 projectilePos = projectile->vec2Position;

		float distFromBullet = glm::distance(vec2Position, projectilePos);

		if (distFromBullet < closestProjectileDist)
		{
			closestProjectileDist = distFromBullet;
			closestProjectilePos = projectilePos;
		}

		if (distFromBullet < 15.f)
		{
			isHit = true;
			projectile->SetStatus(false);
		}
	}

	UpdateFSM(dElapsedTime); // update fsm states

	// update sprite animations based on movement on NPC

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

		if (isHit)
		{
			if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
			{
				// Check if he is walking on air; let him fall down
				glm::vec2 vec2InAirPosition = vec2Position - glm::vec2(0.0f, vec2HalfSize.y);
				if (pMap2D->CheckVerticalCollision(vec2Position, vec2HalfSize, vec2InAirPosition, fCollisionCoordY) == CSettings::RESULTS::NEGATIVE)
				{
					cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);
					//cout << "*** Walked off a platform. Set to Fall ***" << endl;
				}
			}
		}
	}

	// only check colision with floor if hit
	if (isHit)
	{
		// Check if drone is currently colliding with floor
		bool isOnFloor = (pMap2D->CheckVerticalCollision(vec2Position, vec2HalfSize, vec2NewPosition, fCollisionCoordY) == CSettings::RESULTS::POSITIVE);

		if (!isOnFloor)
		{
			// Not on floor, so set to FALL if not already falling
			if (cPhysics2D.GetVerticalStatus() != CPhysics2D::VERTICALSTATUS::FALL)
			{
				cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);
			}
		}
		else
		{
			// Colliding with floor
			if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::FALL)
			{
				cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);
				// Optionally reset isHit here if needed
			}
		}

		//// Check for collision with the Tile Maps vertically
		//if ((cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::WALK) &&
		//	(pMap2D->CheckVerticalCollision(vec2Position, vec2HalfSize, vec2NewPosition, fCollisionCoordY) == CSettings::RESULTS::POSITIVE))
		//{
		//	cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);
		//}
		//// If not already falling, start falling
		//else if (cPhysics2D.GetVerticalStatus() != CPhysics2D::VERTICALSTATUS::FALL)
		//{
		//	cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);
		//}
		//// Check for collision with the Tile Maps vertically
		//else if ((cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::FALL) &&
		//	(pMap2D->CheckVerticalCollision(vec2Position, vec2HalfSize, vec2NewPosition, fCollisionCoordY) == CSettings::RESULTS::POSITIVE))
		//{
		//	//cout << "Vertical collision when falling!" << endl;
		//	cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);
		//	//cout << "*** Hit a platform at the bottom: Set to Idle ***" << endl;
		//}
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
		if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::WALK)
		{
			cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
		}
		if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::WALK)
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);
		}
	}

	CEnemy2D::Update(dElapsedTime); // at the end due to update model

	return true;
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CDroneEnemy2D::PreRender(void)
{
	CEnemy2D::PreRender();
}

/**
 @brief Render this instance
 */
void CDroneEnemy2D::Render(void)
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
void CDroneEnemy2D::PostRender(void)
{
	if (!bIsActive)
		return;

	CEnemy2D::PostRender();
}

/**
 @brief PrintSelf
 */ 
void CDroneEnemy2D::PrintSelf()
{
	cout << "Drone ::PrintSelf()" << endl;
	cout << "=======================" << endl;
	CEnemy2D::PrintSelf();
	cout << "sCurrentFSM\t=\t" << sCurrentFSM << endl;
}

/**
 @brief Let enemy2D interact with the player.
 */
bool CDroneEnemy2D::InteractWithPlayer(void)
{

	return true;
}

void CDroneEnemy2D::InteractWithMap() 
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
void CDroneEnemy2D::UpdateDirection(void)
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
void CDroneEnemy2D::UpdatePosition(void)
{
	glm::vec2 movementSpeed = isRunning ? vec2WalkSpeed * runMultiplier : vec2WalkSpeed;
	vec2MovementVelocity = glm::vec2(0.f);

	// Calculate the vec2MovementVelocity
	if (vec2Direction.x < 0)
	{
		// Move left
		vec2MovementVelocity.x -= movementSpeed.x;
		if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::IDLE)
		{
			cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::WALK);
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);
		}
	}
	else if (vec2Direction.x > 0)
	{
		// Move right
		vec2MovementVelocity.x += movementSpeed.x;
		if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::IDLE)
		{
			cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::WALK);
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);

		}
	}
	else
	{
		vec2MovementVelocity.x = 0.0f;
	}

	if (vec2Direction.y < 0)
	{
		// Move down
		vec2MovementVelocity.y -= movementSpeed.y;
		if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::WALK);
			cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
		}
	}
	else if (vec2Direction.y > 0)
	{
		// Move up
		vec2MovementVelocity.y += movementSpeed.y;
		if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::WALK);
			cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
		}
	}
	else
	{
		vec2MovementVelocity.y = 0.0f;
	}
}

void CDroneEnemy2D::UpdateSpriteAnimation()
{
}

void CDroneEnemy2D::UpdateFSM(float dElapsedTime)
{
	// Get updates from AI
	switch (sCurrentFSM)
	{
	case IDLE:
		// switch to disabled state if hit by bullet
		if (isHit)
		{
			sCurrentFSM = DISABLED;
			stateTimer = 0.f;
			cout << "Switching to DISABLED State" << endl;
		}
		// if bullet shot is near or player is in radius
		else if (closestProjectileDist <= bulletDetectionRadius || 
			glm::distance(pPlayer2D->vec2Position, vec2Position) <= normalDetectionRadius)
		{
			sCurrentFSM = CHASE;
			stateTimer = 0.f;
			cout << "Switching to CHASE State" << endl;
		}
		else if (stateTimer > maxIdleTime)
		{
			sCurrentFSM = PATROL;
			stateTimer = 0.f;
			cout << "Switching to Patrol State" << endl;
		}
		stateTimer += dElapsedTime;
		break;
	case PATROL:
		// if a patrol point is reached, advance to next one
		if (glm::distance(vec2Position, patrolPoints[currentPatrolPointIndex]) < patrolThreshold)
		{
			// reverse patrol path if at first or last patrol point
			if (reversePatrolPath)
			{
				if (currentPatrolPointIndex > 0)
					currentPatrolPointIndex--;
				if (currentPatrolPointIndex == 0)
					reversePatrolPath = false;
			}
			else
			{
				if (currentPatrolPointIndex < (int)patrolPoints.size() - 1)
					currentPatrolPointIndex++;
				if (currentPatrolPointIndex == (int)patrolPoints.size() - 1)
					reversePatrolPath = true;
			}
		}

		// move towards current patrol point
		vec2Direction = glm::normalize(patrolPoints[currentPatrolPointIndex] - vec2Position);
		UpdatePosition();

		if (isHit)
		{
			sCurrentFSM = DISABLED;
			stateTimer = 0.f;
			cout << "Switching to DISABLED State" << endl;
		}
		// if bullet shot is near or player is in radius
		else if (closestProjectileDist <= bulletDetectionRadius ||
			glm::distance(pPlayer2D->vec2Position, vec2Position) <= normalDetectionRadius)
		{
			sCurrentFSM = CHASE;
			stateTimer = 0.f;
			cout << "Switching to CHASE State" << endl;
		}
		else if (stateTimer > maxPatrolTime)
		{
			sCurrentFSM = IDLE;
			stateTimer = 0.f;
			cout << "Switching to Idle State" << endl;
		}
		stateTimer += dElapsedTime;
		break;
	case CHASE:
	{
		// set player as being chased to update statue
		if (!pEnemyManager2D->IsPlayerChased())
			pEnemyManager2D->SetPlayerChased(true);

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

		if (isHit)
		{
			pEnemyManager2D->SetPlayerChased(false);
			sCurrentFSM = DISABLED;
			stateTimer = 0.f;
			isRunning = false;
			cout << "Switching to DISABLED State" << endl;
		}
		// player too far away
		else if (glm::distance(pPlayer2D->vec2Position, vec2Position) > chaseDetectionRadius)
		{
			pEnemyManager2D->SetPlayerChased(false);
			sCurrentFSM = SEARCH;
			stateTimer = 0.f;
			isRunning = false;
			cout << "Switching to SEARCH State" << endl;
		}
		break;
	}
	case SEARCH:
		if (isHit)
		{
			sCurrentFSM = DISABLED;
			stateTimer = 0.f;
			cout << "Switching to DISABLED State" << endl;
		}
		// if bullet shot is near or player is in radius
		else if (closestProjectileDist <= bulletDetectionRadius ||
			glm::distance(pPlayer2D->vec2Position, vec2Position) <= normalDetectionRadius)
		{
			sCurrentFSM = CHASE;
			stateTimer = 0.f;
			cout << "Switching to CHASE State" << endl;
		}
		// switch to return state if cant find player
		else if (stateTimer > maxSearchTime)
		{
			sCurrentFSM = RETURN;
			stateTimer = 0.f;
			cout << "Switching to RETURN State" << endl;
		}
		stateTimer += dElapsedTime;
		break;
	case DISABLED:
		// reenable after a certain duration
		if (stateTimer > maxDisabledTime)
		{
			sCurrentFSM = REENABLE;
			stateTimer = 0.f;
			cout << "Switching to REENABLE State" << endl;
		}
		stateTimer += dElapsedTime;
		break;
	case REENABLE:
		if (isHit) isHit = false;

		// fly back up by a certain amount
		if (!hasSetTargetHeight)
		{
			targetYHeight = vec2Position.y + flyUpAmount;  // fly up 100 units
			hasSetTargetHeight = true;
		}

		if (vec2Position.y < targetYHeight)
		{
			vec2Direction = glm::vec2(0.0f, 1.0f);  // move up
			UpdatePosition();  // updates velocity based on direction
		}
		// after reenabling, if bullet shot is near or player is in radius
		else if (closestProjectileDist <= bulletDetectionRadius ||
			glm::distance(pPlayer2D->vec2Position, vec2Position) <= normalDetectionRadius)
		{
			sCurrentFSM = CHASE;
			stateTimer = 0.f;
			cout << "Switching to CHASE State" << endl;
		}
		else
		{
			sCurrentFSM = SEARCH;
			isHit = false;
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::WALK);
			stateTimer = 0.f;
			vec2Direction = glm::vec2(0.0f, 0.0f); // reset movement
			hasSetTargetHeight = false;
			cout << "Switching to SEARCH state." << endl;
		}
		break;
	case RETURN:
	{
		// return to last patrol index
		glm::vec2 returnPoint = patrolPoints[currentPatrolPointIndex];
		float dist = glm::distance(vec2Position, returnPoint);

		// Get tile indices for enemy and patrol point
		int iEnemyTileX, iEnemyTileY;
		int iPatrolTileX, iPatrolTileY;

		if (pMap2D->GetTileIndexAtPosition(vec2Position, iEnemyTileX, iEnemyTileY) &&
			pMap2D->GetTileIndexAtPosition(returnPoint, iPatrolTileX, iPatrolTileY))
		{
			// Pathfind from enemy tile to patrol tile
			auto path = pMap2D->PathFind(
				glm::vec2(iEnemyTileX, iEnemyTileY),
				glm::vec2(iPatrolTileX, iPatrolTileY),
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

		// Check if reached the patrol point
		if (dist < 10.f || stateTimer > maxReturnTime)
		{
			// Once reached, switch back to patrol and continue normally
			sCurrentFSM = PATROL;
			stateTimer = 0.f;
			cout << "Switching to PATROL state" << endl;
		}
		stateTimer += dElapsedTime;
		break;
	}
	default:
		break;
	}
}

void CDroneEnemy2D::SetupPatrolPoints(std::vector<glm::vec2> points)
{
	patrolPoints = points;
	currentPatrolPointIndex = 0;
}
