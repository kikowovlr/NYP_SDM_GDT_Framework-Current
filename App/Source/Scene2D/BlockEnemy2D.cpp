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
#include "BlockInfo.h"


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

	SetName("BlockSmith");

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
	pAnimatedSprites->PlayAnimation("idle-front", -1, 1.5f);
	
	pInventoryManager->BindToCharacter(this);
	pInventoryItem = pInventoryManager->Add("Health", "Image/Scene2D_Health.tga", 100, 100);
	pInventoryItem->vec2Size = glm::vec2(25, 25);
	pInventoryItem = pInventoryManager->Add("Crate", "Image/crate.png", 1, 0);
	pInventoryItem->vec2Size = glm::vec2(25, 25);

	// Set status to idle by default
	cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
	cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);

	vec2WalkSpeed = glm::vec2(50.0f, 50.0f);

	std::vector<glm::vec2> patrolTiles = {
		glm::vec2(5, 19),
		glm::vec2(5, 21),
		glm::vec2(12, 21),
		glm::vec2(12, 13)
	};

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
bool CBlockEnemy2D::Update(const double dElapsedTime)
{
	if (!bIsActive)
		return false;

	// Reset vec2MovementVelocity
	vec2MovementVelocity = glm::vec2(0.0f);
	// Set the physics horizontal status to idle
	cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
	if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::WALK)
		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);
	
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
	}

	UpdateFSM(dElapsedTime); // update fsm states

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
	}

	// Check for collision with the Tile Maps vertically
	if ((cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::WALK) &&
		(pMap2D->CheckVerticalCollision(vec2Position, vec2HalfSize, vec2NewPosition, fCollisionCoordY) == CSettings::RESULTS::POSITIVE))
	{
		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);
		// Flip the direction since this direction is blocked
		FlipVerticalDirection();
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
void CBlockEnemy2D::PostRender(void)
{
	if (!bIsActive)
		return;

	CEnemy2D::PostRender();
}

/**
 @brief PrintSelf
 */ 
void CBlockEnemy2D::PrintSelf()
{
	cout << "BlockSmith ::PrintSelf()" << endl;
	cout << "=======================" << endl;
	CEnemy2D::PrintSelf();
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
	default:
		break;
	}
}

/**
 @brief Update the enemy's direction.
 */
void CBlockEnemy2D::UpdateDirection(void)
{
	//if (glm::distance(vec2Position, pPlayer2D->vec2Position) > glm::distance(vec2Position, pTopdee->vec2Position))
	//{
	//	// if toodee closer, set destination as toodee's pos
	//	vec2Destination = pPlayer2D->vec2Position;
	//}
	//else
	//{
	//	// else set destination as topdee
	//	vec2Destination = pTopdee->vec2Position;
	//}

	// Calculate the direction between start and destination
	vec2Direction = CalculateDirection(vec2Position, vec2Destination);
}

/**
@brief Update position.
*/
void CBlockEnemy2D::UpdatePosition(void)
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

void CBlockEnemy2D::UpdateSpriteAnimation()
{
	if (isSurprisedOrWeary)
	{
		switch (eFacingDirection)
		{
		case DIRECTION::LEFT:  pAnimatedSprites->PlayAnimation("surprised-left", -1, 1.0f); break;
		case DIRECTION::RIGHT: pAnimatedSprites->PlayAnimation("surprised-right", -1, 1.0f); break;
		case DIRECTION::UP:    pAnimatedSprites->PlayAnimation("surprised-back", -1, 1.0f); break;
		case DIRECTION::DOWN:  pAnimatedSprites->PlayAnimation("surprised-front", -1, 1.0f); break;
		}
	}
	else if (isHoldingBlock)
	{
		switch (eFacingDirection)
		{
		case DIRECTION::LEFT:  pAnimatedSprites->PlayAnimation("holdblock-left", -1, 1.0f); break;
		case DIRECTION::RIGHT: pAnimatedSprites->PlayAnimation("holdblock-right", -1, 1.0f); break;
		case DIRECTION::UP:    pAnimatedSprites->PlayAnimation("holdblock-back", -1, 1.0f); break;
		case DIRECTION::DOWN:  pAnimatedSprites->PlayAnimation("holdblock-front", -1, 1.0f); break;
		}
	}
	else
	{
		switch (eFacingDirection)
		{
		case DIRECTION::LEFT:  pAnimatedSprites->PlayAnimation("idle-left", -1, 1.0f); break;
		case DIRECTION::RIGHT: pAnimatedSprites->PlayAnimation("idle-right", -1, 1.0f); break;
		case DIRECTION::UP:    pAnimatedSprites->PlayAnimation("idle-back", -1, 1.0f); break;
		case DIRECTION::DOWN:  pAnimatedSprites->PlayAnimation("idle-front", -1, 1.0f); break;
		}
	}

}

void CBlockEnemy2D::UpdateFSM(float dElapsedTime)
{
	// Get updates from AI
	switch (sCurrentFSM)
	{
	case IDLE:
		if (closestProjectileDist <= surpriseRadius)
		{
			sCurrentFSM = SURPRISED;
			stateTimer = 0.f;
			cout << "Switching to Surprised State" << endl;
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

		// move towards current patrol point with axis snapping
		glm::vec2 delta = patrolPoints[currentPatrolPointIndex] - vec2Position;

		// Pick dominant axis (horizontal or vertical)
		if (fabs(delta.x) > fabs(delta.y))
		{
			vec2Direction = glm::vec2((delta.x > 0 ? 1.0f : -1.0f), 0.0f);
		}
		else
		{
			vec2Direction = glm::vec2(0.0f, (delta.y > 0 ? 1.0f : -1.0f));
		}
		UpdatePosition();

		// switch states
		if (closestProjectileDist <= surpriseRadius)
		{
			sCurrentFSM = SURPRISED;
			stateTimer = 0.f;
			cout << "Switching to Surprised State" << endl;
		}
		//else
		//{
		//	// check if crates of a certain radius has been moved or not
		//	for (auto& crate : crateInfos)
		//	{
		//		// dist from default position of crate
		//		float dist = glm::distance(vec2Position, crate.defaultPos);

		//		if (dist < detectionRadius)
		//		{
		//			// if crate has deviated from og position
		//			if (crate.currentPos != crate.defaultPos && crate.currentPos != glm::vec2(-1, -1))
		//			{
		//				// set current path to find the block
		//				vec2Destination = crate.currentPos;
		//				sCurrentFSM = FIND_BLOCK; // switch to find block state for path finding
		//				cout << "Switching to Find Block State" << endl;
		//				break;
		//			}
		//		}
		//	}
		//}
		
		// if no moved crate found, check for idle
		if (stateTimer > maxPatrolTime)
		{
			sCurrentFSM = IDLE;
			stateTimer = 0.f;
			cout << "Switching to Idle State" << endl;

		}
		stateTimer += dElapsedTime;
		break;
	case FIND_BLOCK:
		// prioritise surprised state
		if (closestProjectileDist <= surpriseRadius)
		{
			sCurrentFSM = SURPRISED;
			cout << "Switching to Surprised State" << endl;
			break;
		}
		// calculate path to block
		//cout << "distance: " << glm::distance(vec2Position, vec2Destination) << endl;
		//cout << "destination (x: " << vec2Destination.x << ", y: " << vec2Destination.y << ")" << endl;
		// skip path finding if destination is within 1 tile size
		else if (glm::distance(vec2Position, vec2Destination) > pMap2D->GetTileHalfSize().x)
		{
			// convert to tile indices
			int iStartX = 0, iStartY = 0;
			int iTargetX = 0, iTargetY = 0;

			// Get current tile indices for enemy and destination
			if (pMap2D->GetTileIndexAtPosition(vec2Position, iStartX, iStartY) &&
				pMap2D->GetTileIndexAtPosition(vec2Destination, iTargetX, iTargetY))
			{

				// Only pick adjacent tile ONCE per state entry
				if (!hasTargetTile)
				{
					// If target is blocked, try to adjust or abandon
					if (pMap2D->isBlocked(iTargetY, iTargetX))
					{
						std::vector<glm::ivec2> possibleTiles = {
							{iTargetX + 1, iTargetY}, // right
							{iTargetX - 1, iTargetY}, // left
							{iTargetX, iTargetY + 1}, // down
							{iTargetX, iTargetY - 1}  // up
						};

						cout << "Target tile is blocked! Finding next best target." << endl;
						float bestDist = FLT_MAX;
						glm::ivec2 bestTile = { iTargetX, iTargetY };

						// loop through possible tiles
						for (auto& tile : possibleTiles)
						{
							if (!pMap2D->isBlocked(tile.y, tile.x))
							{
								glm::vec2 worldPos(
									tile.x * pMap2D->GetTileSize().x + pMap2D->GetTileHalfSize().x,
									tile.y * pMap2D->GetTileSize().y + pMap2D->GetTileHalfSize().y
								);
								float dist = glm::distance(vec2Position, worldPos);
								if (dist < bestDist)
								{
									bestDist = dist;
									bestTile = tile;
								}
							}
						}
						targetTileIndex = bestTile;
					}
					else
					{
						targetTileIndex = { iTargetX, iTargetY };
					}

					vec2Destination = glm::vec2(targetTileIndex.x * pMap2D->GetTileSize().x + pMap2D->GetTileHalfSize().x, targetTileIndex.y * pMap2D->GetTileSize().y + pMap2D->GetTileHalfSize().y);
					//cout << "New target -> x:" << targetTileIndex.x << " y: " << targetTileIndex.y << endl;
					//cout << "destination (x: " << vec2Destination.x << ", y: " << vec2Destination.y << ")" << endl;
					hasTargetTile = true;
				}

	
				// Check if enemy is centered on current tile (within tolerance)
				glm::vec2 tileCenter =
					glm::vec2(iStartX * pMap2D->GetTileSize().x + vec2HalfSize.x,
						iStartY * pMap2D->GetTileSize().y + vec2HalfSize.y);
				if (glm::distance(vec2Position, tileCenter) > pMap2D->GetSizeTolerance().x)
				{
					// Move towards center of current tile before pathfinding
					vec2Direction = glm::normalize(tileCenter - vec2Position);
					UpdatePosition();
					break; // wait until centered
				}

				//if (dElapsedTimeSinceLastPathFind > pathFindInterval)
				//{
					// compute path
					auto path = pMap2D->PathFind(glm::vec2(iStartX, iStartY),
						glm::vec2(targetTileIndex.x, targetTileIndex.y),
						heuristic::euclidean,
						10);
					dElapsedTimeSinceLastPathFind = 0.f;

					// extract points and determine dir for the enemy to travel towards
					// Calculate new destination
					bool bFirstPosition = true;
					glm::vec2 vec2PathPoint;
					for (const auto& coord : path)
					{
						// Take a path point from path via coord
						vec2PathPoint = coord;
						if (bFirstPosition)
						{
							// Set a destination
							vec2Destination = glm::vec2(vec2PathPoint.x * pMap2D->GetTileSize().x + pMap2D->GetTileHalfSize().x, vec2PathPoint.y * pMap2D->GetTileSize().y + pMap2D->GetTileHalfSize().y);
							// Calculate the direction between enemy2D and this destination
							vec2Direction = glm::normalize(vec2PathPoint - glm::vec2(iStartX, iStartY));
							bFirstPosition = false;
						}
						else
						{
							// If the next path point is in the same direction, use it as the new vec2Destination
							if (glm::normalize(vec2PathPoint - glm::vec2(iStartX, iStartY)) == vec2Direction)
							{
								// Set a destination
								vec2Destination = glm::vec2(vec2PathPoint.x * pMap2D->GetTileSize().x + pMap2D->GetTileHalfSize().x, vec2PathPoint.y * pMap2D->GetTileSize().y + pMap2D->GetTileHalfSize().y);
							}
							else
								break;
						}
					//}
				}

				//cout << "vec2Destination : " << vec2Destination.x 
				//		<< ", " << vec2Destination.y << endl;
				//cout << "vec2Direction : " << vec2Direction.x 
				//		<< ", " << vec2Direction.y << endl;
				if (vec2Direction.x == -1)
				{
					system("pause");
				}

				UpdatePosition();
			}
		}
		else {
			// reached destination -> pick up block
			
			// change state
			sCurrentFSM = MOVE_BLOCK;
			isHoldingBlock = true;
			hasTargetTile = false;
			targetTileIndex = glm::ivec2(0, 0);
			cout << "Switching to Move Block State" << endl;
		}

		dElapsedTimeSinceLastPathFind += dElapsedTime;
		break;
	case MOVE_BLOCK:
		if (closestProjectileDist <= surpriseRadius)
		{
			sCurrentFSM = SURPRISED;
			cout << "Switching to Surprised State" << endl;
		}
		break;
	case PLACE_BLOCK:
		if (closestProjectileDist <= surpriseRadius)
		{
			sCurrentFSM = SURPRISED;
			cout << "Switching to Surprised State" << endl;
		}
		break;
	case SURPRISED:
	{
		// calculate direction vector to move in and save it for fleeing
		glm::vec2 dirAwayFromProjectile = glm::normalize(vec2Position - closestProjectilePos);
		// Round each component to -1, 0, or 1 based on its sign and a small threshold
		auto signum = [](float v) -> float {
			if (v > 0.1f) return 1.0f;
			else if (v < -0.1f) return -1.0f;
			else return 0.0f;
			};

		if (fabs(dirAwayFromProjectile.x) > fabs(dirAwayFromProjectile.y))
			fleeDir = glm::vec2(signum(dirAwayFromProjectile.x), 0.0f);
		else
			fleeDir = glm::vec2(0.0f, signum(dirAwayFromProjectile.y));


		// make surprised if not alrdy surprised -> // play surprised animation
		if (!isSurprisedOrWeary)
			isSurprisedOrWeary = true;
		// set direction to 0 to stop movement
		vec2Direction = glm::vec2(0.f);

		if (stateTimer > maxShockedTime)
		{
			sCurrentFSM = FLEE;
			stateTimer = 0.f;
			isSurprisedOrWeary = false;
			cout << "Switching to Flee State" << endl;
		}
		stateTimer += dElapsedTime;
		break;
	}
	case FLEE:
	{
		// Find any tile that is in opposite dir of projectile, is empty and is a min distance away
		if (!isRunning)
			isRunning = true;

		// Get enemy's current tile
		int iEnemyTileX, iEnemyTileY;
		pMap2D->GetTileIndexAtPosition(vec2Position, iEnemyTileX, iEnemyTileY);

		// if flee tile has not been calculated
		if (!isFleeTileFound)
		{
			// reset bestTile & bestDot
			fleeTargetTile = glm::ivec2(-1, -1);
			float bestDot = -1.0f; // to prefer tiles in close opposite direction (dot product)

			// Search tiles within a square around the enemy (from -maxDist to +maxDist)
			for (int dx = -maxTilesDist; dx <= maxTilesDist; ++dx)
			{
				for (int dy = -maxTilesDist; dy <= maxTilesDist; ++dy)
				{
					int tileX = iEnemyTileX + dx;
					int tileY = iEnemyTileY + dy;

					glm::vec2 candidateTile(tileX, tileY);

					// Check bounds
					if (!pMap2D->isValid(candidateTile))
						continue;

					// Distance from enemy tile
					float dist = glm::length(glm::vec2(dx, dy));
					if (dist < minTilesDist || dist > maxTilesDist)
						continue;

					// Check if tile is blocked
					if (pMap2D->isBlocked(tileY, tileX))
						continue;

					// Direction from enemy tile to candidate tile
					glm::vec2 dirToCandidate = glm::normalize(glm::vec2(dx, dy));

					// Check if candidate tile is roughly in fleeDir direction (dot product near 1)
					float dot = glm::dot(dirToCandidate, fleeDir);
					if (dot > bestDot)
					{
						bestDot = dot;
						fleeTargetTile = glm::ivec2(tileX, tileY);
					}
				}
			}

			isFleeTileFound = true;
		}

		if (fleeTargetTile.x != -1 && fleeTargetTile.y != -1)
		{
			if ((pMap2D->GetTileIndexAtPosition(vec2Position, iEnemyTileX, iEnemyTileY) == true))
			{

				// Check if the enemy is at the centre of the column
				// if not, then move towards it first.
				//// Otherwise, do pathfinding
				//vec2Destination = glm::vec2(iEnemyTileX * pMap2D->GetTileSize().x + vec2HalfSize.x, iEnemyTileY * pMap2D->GetTileSize().y + vec2HalfSize.y);
				//if (abs(vec2Destination.x - vec2Position.x) > pMap2D->GetSizeTolerance().x)
				//{
				//	// Update the Enemy2D's position for attack
				//	UpdatePosition();
				//	break;
				//}
				//else
				//	cout << "Enemy start pos => x: " << vec2Position.x << " y: " << vec2Position.y << endl;

				// found valid flee tile
				// path find to tile
				auto path = pMap2D->PathFind(glm::vec2(iEnemyTileX, iEnemyTileY),
					glm::vec2(fleeTargetTile.x, fleeTargetTile.y),
					heuristic::euclidean,
					10);

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
		}
		else {
			cout << "unable to find tile" << endl;
		}

		// fallback -> check if flee time is exceeded
		// Check if destination reached (within tolerance) -> change state
		if (stateTimer > maxFleeTime || glm::distance(vec2Position, vec2Destination) < 2.f)
		{
			isFleeTileFound = false;
			sCurrentFSM = WEARY;
			isRunning = false;
			cout << "Switching to WEARY state" << endl;
		}
		stateTimer += dElapsedTime;
	}
		break;
	case WEARY:
		if (!isSurprisedOrWeary) isSurprisedOrWeary = true;
		// during weary time
		if (stateTimer > maxWearyTime)
		{
			// check if close to player, if not return
			if (glm::distance(vec2Position, pPlayer2D->vec2Position) < detectionRadius)
			{
				// if close to player, go back to fleeing
				sCurrentFSM = FLEE;
				isSurprisedOrWeary = false;
				stateTimer = 0.f;

				// calculate direction vector to move in and save it for fleeing
				glm::vec2 dirAwayFromPlayer = glm::normalize(vec2Position - pPlayer2D->vec2Position);
				// Round each component to -1, 0, or 1 based on its sign and a small threshold
				auto signum = [](float v) -> float {
					if (v > 0.1f) return 1.0f;
					else if (v < -0.1f) return -1.0f;
					else return 0.0f;
					};

				if (fabs(dirAwayFromPlayer.x) > fabs(dirAwayFromPlayer.y))
					fleeDir = glm::vec2(signum(dirAwayFromPlayer.x), 0.0f);
				else
					fleeDir = glm::vec2(0.0f, signum(dirAwayFromPlayer.y));
				cout << "Switching to FLEE state" << endl;
			}
			else {
				sCurrentFSM = RETURN;
				isSurprisedOrWeary = false;
				stateTimer = 0.f;
				cout << "Switching to RETURN state" << endl;
			}
		}
		stateTimer += dElapsedTime;
		break;
	case RETURN:
	{
		// return to closest patrol point
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
		if (dist < patrolThreshold || stateTimer > maxReturnTime)
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

void CBlockEnemy2D::SetupPatrolPoints(std::vector<glm::vec2> points)
{
	patrolPoints = points;
	currentPatrolPointIndex = 0;
}
