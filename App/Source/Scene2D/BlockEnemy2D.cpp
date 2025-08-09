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
#include "BlockInfo.cpp"

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
	const float EPSILON = 0.2f;

	// Calculate the vec2MovementVelocity
	if (vec2Direction.x < -EPSILON)
	{
		// Move left
		vec2MovementVelocity.x -= vec2WalkSpeed.x;
		if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::IDLE)
		{
			cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::WALK);
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);
		}
	}
	else if (vec2Direction.x > EPSILON)
	{
		// Move right
		vec2MovementVelocity.x += vec2WalkSpeed.x;
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

	if (vec2Direction.y < -EPSILON)
	{
		// Move down
		vec2MovementVelocity.y -= vec2WalkSpeed.y;
		if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::WALK);
			cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
		}
	}
	else if (vec2Direction.y > EPSILON)
	{
		// Move up
		vec2MovementVelocity.y += vec2WalkSpeed.y;
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
	if (isSurprised)
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
		if (lastFindBlockTimer > maxLastFoundBlockTime)
		{
			sCurrentFSM = FIND_BLOCK;
			lastFindBlockTimer = 0.f;
			cout << "Switching to Find Block State" << endl;
		}
		if (stateTimer > maxIdleTime)
		{
			sCurrentFSM = PATROL;
			stateTimer = 0.f;
			cout << "Switching to Patrol State" << endl;
		}
		stateTimer += dElapsedTime;
		lastFindBlockTimer += dElapsedTime;
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

		// check if crates of a certain radius has been moved or not
		for (auto& crate : crateInfos)
		{
			// dist from default position of crate
			float dist = glm::distance(vec2Position, crate.defaultPos);

			if (dist < detectionRadius)
			{
				// if crate has deviated from og position
				if (crate.hasMoved)
				{
					// set current path to find the block
					vec2Destination = crate.currentPos;
					sCurrentFSM = FIND_BLOCK; // switch to find block state for path finding
					cout << "Switching to Find Block State" << endl;
					cout << "destination (x: " << vec2Destination.x << ", y: " << vec2Destination.y << ")" << endl;
				}
			}
		}

		if (lastFindBlockTimer > maxLastFoundBlockTime)
		{
			sCurrentFSM = FIND_BLOCK;
			lastFindBlockTimer = 0.f;
			cout << "Switching to Find Block State" << endl;
		}
		else if (stateTimer > maxPatrolTime)
		{
			sCurrentFSM = IDLE;
			stateTimer = 0.f;
			cout << "Switching to Idle State" << endl;

		}
		stateTimer += dElapsedTime;
		lastFindBlockTimer += dElapsedTime;
		break;
	case FIND_BLOCK:
		// calculate path to block
		cout << "distance: " << glm::distance(vec2Position, vec2Destination) << endl;
		// skip path finding if destination is within 1 tile size
		if (glm::distance(vec2Position, vec2Destination) > pMap2D->GetTileSize().x)
		{
			// convert to tile indices
			int iStartX = 0, iStartY = 0;
			int iTargetX = 0, iTargetY = 0;

			// Get current tile indices for enemy and destination
			if (pMap2D->GetTileIndexAtPosition(vec2Position, iStartX, iStartY) &&
				pMap2D->GetTileIndexAtPosition(vec2Destination, iTargetX, iTargetY))
			{
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

				if (dElapsedTimeSinceLastPathFind > pathFindInterval)
				{
					// compute path
					//cout << "=== Printing out the path ===" << endl;

					auto path = pMap2D->PathFind(glm::vec2(iStartX, iStartY),
						glm::vec2(iTargetX, iTargetY),
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
							vec2Destination = glm::vec2(vec2PathPoint.x * pMap2D->GetTileSize().x, vec2PathPoint.y * pMap2D->GetTileSize().y);
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
								vec2Destination = glm::vec2(vec2PathPoint.x * pMap2D->GetTileSize().x, vec2PathPoint.y * pMap2D->GetTileSize().y);
							}
							else
								break;
						}
					}
				}

				//cout << "vec2Destination : " << vec2Destination.x 
				//		<< ", " << vec2Destination.y << endl;
				//cout << "vec2Direction : " << vec2Direction.x 
				//		<< ", " << vec2Direction.y << endl;
				//system("pause");

				UpdatePosition();
			}
		}
		else {
			// reached destination -> pick up block
			
			// change state
			sCurrentFSM = MOVE_BLOCK;
			isHoldingBlock = true;
			cout << "Switching to Move Block State" << endl;
		}

		dElapsedTimeSinceLastPathFind += dElapsedTime;
		break;
	case MOVE_BLOCK:

		break;
	case PLACE_BLOCK:

		break;
	case SURPRISED:

		break;
	case FLEE:

		break;
	case WEARY:

		break;
	case RETURN:

		break;
	default:

		break;
	}
}

void CBlockEnemy2D::SetupPatrolPoints(std::vector<glm::vec2> points)
{
	patrolPoints = points;
	currentPatrolPointIndex = 0;
}
