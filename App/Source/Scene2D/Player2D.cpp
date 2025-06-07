/**
 Player2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Player2D.h"

#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include ImageLoader
#include "System\ImageLoader.h"

// Include the MeshBuilder so we can create 2D Quad Mesh for rendering
#include "Primitives/MeshBuilder.h"

// Include Game Manager
#include "GameManager.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CPlayer2D::CPlayer2D(void)
	: pMap2D(NULL)
	, pKeyboardController(NULL)
	, pMouseController(NULL)
	, pProjectileManager2D(NULL)
	, pInventoryManager(NULL)
	, pInventoryItem(NULL)
	, isGunPicked(false)
{
	// Initialise position of the player
	vec2Position = glm::vec2(0);
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CPlayer2D::~CPlayer2D(void)
{
	// We won't delete this since it was created elsewhere
	pMouseController = NULL;

	// We won't delete this since it was created elsewhere
	pKeyboardController = NULL;

	// We won't delete this since it was created elsewhere
	pMap2D = NULL;

	// We won't delete this since it was created elsewhere
	pProjectileManager2D = NULL;

	// We won't delete this since it was created elsewhere
	pInventoryManager = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
}

/**
  @brief Initialise this instance
  */
bool CPlayer2D::Init(void)
{
	// Store the keyboard controller singleton instance here
	pKeyboardController = CKeyboardController::GetInstance();
	// Reset all keys since we are starting a new game
	pKeyboardController->Reset();
	// Store the mouse controller singleton instance here
	pMouseController = CMouseController::GetInstance();

	// Get the handler to the CSettings instance
	pSettings = CSettings::GetInstance();

	// Store the pProjectileManager2D singleton instance here
	pProjectileManager2D = CProjectileManager2D::GetInstance();
	if (pProjectileManager2D->Init() == false)
	{
		cout << "Unable to initialise pProjectileManager2D" << endl;
		return false;
	}
	// Reset the cShootStatus
	cShootStatus.Reset();
	cShootStatus.SetToCannotShoot();

	// Get the handler to the CMap2D instance
	pMap2D = CMap2D::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to pPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (pMap2D->FindValue(200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	pMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Position = glm::vec2(	uiCol * pMap2D->GetTileSize().x + pMap2D->GetTileHalfSize().x, 
								uiRow * pMap2D->GetTileSize().y + pMap2D->GetTileHalfSize().y);
	vec2StartPosition = vec2Position;
	vec2MovementVelocity = glm::vec2(1, 1);

	// Set up the projection matrix
	projection = glm::ortho(0.0f,
							CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowWidth", 800.0f),
							0.0f,
							CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f),
							-1.0f, 1.0f);

	// Generate the VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Teacher texture
	// Load the player texture 
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/scene2d_player.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/scene2d_player.png" << endl;
		return false;
	}

	////CS: Create the Quad Mesh using the mesh builder
	//p2DMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), 1, 1);

	//CS: Create the animated sprite and setup the animation 
	pAnimatedSprites = CMeshBuilder::GenerateSpriteAnimation(3, 3);// , pSettings->TILE_WIDTH, pSettings->TILE_HEIGHT);
	pAnimatedSprites->AddAnimation("idle", 0, 2);
	pAnimatedSprites->AddAnimation("right", 3, 5);
	pAnimatedSprites->AddAnimation("left", 6, 8);
	//CS: Play the "idle" animation as default
	pAnimatedSprites->PlayAnimation("idle", -1, 1.0f);

	//iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/SpriteBunny.png", true);
	//if (iTextureID == 0)
	//{
	//	cout << "Unable to load Image/SpriteBunny.png" << endl;
	//	return false;
	//}

	////CS: Create the animated sprite and setup the animation 
	//pAnimatedSprites = CMeshBuilder::GenerateSpriteAnimation(2, 3);// , pSettings->TILE_WIDTH, pSettings->TILE_HEIGHT);
	//pAnimatedSprites->AddAnimation("right", 0, 2);
	//pAnimatedSprites->AddAnimation("left", 3, 5);
	//// default anim is right
	//pAnimatedSprites->PlayAnimation("right", -1, 2.f);
	 
	//CS: Init the colour to white
	vec4ColourTint = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Get the handler to the CInventoryManager instance
	pInventoryManager = CInventoryManager::GetInstance();

	// Bind to toodee inventory
	pInventoryManager->BindToCharacter(this);

	// This set of codes should be removed once CGUI_Scene2D has been added.
	// Add a Tree as one of the inventory items
	//pInventoryItem = pInventoryManager->Add("Tree", "Image/Scene2D_TreeTile.tga", 5, 0);
	//pInventoryItem->vec2Size = glm::vec2(25, 25);
	
	//// Add a Lives icon as one of the inventory items
	//pInventoryItem = pInventoryManager->Add("Lives", "Image/Scene2D_Lives.tga", 3, 3);
	//pInventoryItem->vec2Size = glm::vec2(25, 25);

	//// Add a Health icon as one of the inventory items
	//pInventoryItem = pInventoryManager->Add("Health", "Image/Scene2D_Health.tga", 100, 100);
	//pInventoryItem->vec2Size = glm::vec2(25, 25);

	pInventoryItem = pInventoryManager->Add("Laser Gun", "Image/laser-gun.png",1, 0);
	pInventoryItem->vec2Size = glm::vec2(25, 25);

	// Set the Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
	cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);

	return true;
}

/**
 @brief Reset this instance
 */
bool CPlayer2D::Reset()
{
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	if (pMap2D->FindValue(200, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	pMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Position = glm::vec2(uiCol * pMap2D->GetTileSize().x + pMap2D->GetTileHalfSize().x,
		uiRow * pMap2D->GetTileSize().y + pMap2D->GetTileHalfSize().y);

	//Set it to fall upon entering new level
	cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
	cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);

	//CS: Play the "idle" animation as default
	pAnimatedSprites->PlayAnimation("idle", -1, 1.0f);

	//CS: Init the colour to white
	vec4ColourTint = glm::vec4(1.0, 1.0, 1.0, 1.0);

	return true;
}

/**
 @brief Update this instance
 @param dElapsedTime A const double variable contains the time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
 */
bool CPlayer2D::Update(const double dElapsedTime)
{
	// Reset vec2MovementVelocity
	vec2MovementVelocity = glm::vec2(0.0f);
	// Set the physics horizontal status to idle
	cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
	if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::WALK)
		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);

	// Get keyboard updates
	if ((pKeyboardController->IsKeyDown(GLFW_KEY_LEFT_SHIFT)) ||
		(pKeyboardController->IsKeyDown(GLFW_KEY_RIGHT_SHIFT)))
	{
		// Left-right movement
		if ((pKeyboardController->IsKeyDown(GLFW_KEY_A)) &&
			(pKeyboardController->IsKeyDown(GLFW_KEY_D)))
		{
			vec2MovementVelocity.x = 0.0f;
		}
		else if (pKeyboardController->IsKeyDown(GLFW_KEY_A))
		{
			vec2MovementVelocity.x -= vec2WalkSpeed.x * vec2WalkSpeedMultiplier.x;
			if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::IDLE)
				cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::WALK);
		}
		else if (pKeyboardController->IsKeyDown(GLFW_KEY_D))
		{
			vec2MovementVelocity.x += vec2WalkSpeed.x * vec2WalkSpeedMultiplier.x;
			if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::IDLE)
				cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::WALK);
		}

		//// Up-down movement
		//if ((pKeyboardController->IsKeyDown(GLFW_KEY_W)) &&
		//	(pKeyboardController->IsKeyDown(GLFW_KEY_S)))
		//{
		//	vec2MovementVelocity.y = 0.0f;
		//}
		//else if (pKeyboardController->IsKeyDown(GLFW_KEY_W))
		//{
		//	vec2MovementVelocity.y += vec2WalkSpeed.y * vec2WalkSpeedMultiplier.y;
		//	if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
		//		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::WALK);
		//}
		//else if (pKeyboardController->IsKeyDown(GLFW_KEY_S))
		//{
		//	vec2MovementVelocity.y -= vec2WalkSpeed.y * vec2WalkSpeedMultiplier.y;
		//	if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
		//		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::WALK);
		//}
	}
	else {
		// Left-right movement
		if ((pKeyboardController->IsKeyDown(GLFW_KEY_A)) &&
			(pKeyboardController->IsKeyDown(GLFW_KEY_D)))
		{
			vec2MovementVelocity.x = 0.0f;
		}
		else if (pKeyboardController->IsKeyDown(GLFW_KEY_A))
		{
			vec2MovementVelocity.x -= vec2WalkSpeed.x;
			if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::IDLE)
				cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::WALK);
		}
		else if (pKeyboardController->IsKeyDown(GLFW_KEY_D))
		{
			vec2MovementVelocity.x += vec2WalkSpeed.x;
			if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::IDLE)
				cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::WALK);
		}

		//// Up-down movement
		//if ((pKeyboardController->IsKeyDown(GLFW_KEY_W)) &&
		//	(pKeyboardController->IsKeyDown(GLFW_KEY_S)))
		//{
		//	vec2MovementVelocity.y = 0.0f;
		//}
		//else if (pKeyboardController->IsKeyDown(GLFW_KEY_W))
		//{
		//	vec2MovementVelocity.y += vec2WalkSpeed.y;
		//	if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
		//		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::WALK);
		//}
		//else if (pKeyboardController->IsKeyDown(GLFW_KEY_S))
		//{
		//	vec2MovementVelocity.y -= vec2WalkSpeed.y;
		//	if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
		//		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::WALK);
		//}
	}

	// Jump movement
	if (pKeyboardController->IsKeyPressed(GLFW_KEY_SPACE))
	{
		// For jump
		if (cPhysics2D.GetVerticalStatus() <= CPhysics2D::VERTICALSTATUS::IDLE)
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::JUMP);
			cPhysics2D.SetInitialVelocity(vec2JumpSpeed);
			cPhysics2D.SetNewJump(true);
		}
		// For double jump
		else if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::JUMP)
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::DOUBLEJUMP);
			cPhysics2D.SetInitialVelocity(vec2JumpSpeed);
			cPhysics2D.SetNewJump(true);
		}
	}

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

	if ((pKeyboardController->IsKeyDown(GLFW_KEY_A)) /*&& (pKeyboardController->IsKeyDown(GLFW_KEY_SPACE))*/)
	{
		if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::WALK);
	}
	// Update vec2Position
	glm::vec2 vec2NewPosition = vec2Position + vec2MovementVelocity * (float)dElapsedTime;
	// For calculating the collision point's x-coordinate
	float fCollisionCoordX = 0;
	// For calculating the collision point's y-coordinate
	float fCollisionCoordY = 0;

	// Check for collision with the Tile Maps horizontally
	if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::WALK)
	{
		// Check if the player walks into an obstacle
		if (pMap2D->CheckHorizontalCollision(vec2Position, vec2HalfSize, vec2NewPosition, fCollisionCoordX) == CSettings::RESULTS::POSITIVE)
		{
			cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
		}
			
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

	// Check for collision with the Tile Maps vertically
	if ((cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::WALK) &&
		(pMap2D->CheckVerticalCollision(vec2Position, vec2HalfSize, vec2NewPosition, fCollisionCoordY) == CSettings::RESULTS::POSITIVE))
	{
		//cout << "Vertical collision when walking!" << endl;
		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);
		//cout << "*** Hit a platform at the top/bottom: Set to Idle ***" << endl;
	}
	// Check for collision with the Tile Maps vertically
	else if ((cPhysics2D.GetVerticalStatus() >= CPhysics2D::VERTICALSTATUS::JUMP) &&
		(cPhysics2D.GetVerticalStatus() <= CPhysics2D::VERTICALSTATUS::DOUBLEJUMP) &&
		(pMap2D->CheckVerticalCollision(vec2Position, vec2HalfSize, vec2NewPosition, fCollisionCoordY) == CSettings::RESULTS::POSITIVE))
	{
		//cout << "Vertical collision when jumping!" << endl;
		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);
		//cout << "*** Hit a platform on top: Set to Fall ***" << endl;
	}
	// Check for collision with the Tile Maps vertically
	else if ((cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::FALL) &&
		(pMap2D->CheckVerticalCollision(vec2Position, vec2HalfSize, vec2NewPosition, fCollisionCoordY) == CSettings::RESULTS::POSITIVE))
	{
		//cout << "Vertical collision when falling!" << endl;
		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);
		//cout << "*** Hit a platform at the bottom: Set to Idle ***" << endl;
	}

	// Update the vec2Position with the new position
	vec2Position = vec2NewPosition;

	// shooting mechanic
	if (pMouseController->IsButtonPressed(0) /*left mouse button*/ && cShootStatus.IsAbleToShoot())
	{
		glm::vec2 temp = glm::normalize(glm::vec2(pMouseController->GetMousePositionX() - vec2Position.x,
			CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f) -
			pMouseController->GetMousePositionY() - vec2Position.y));
		// Activate a CProjectil2D
		pProjectileManager2D->Activate(vec2Position,
			temp, 2.0, 200.0f, this);

		cShootStatus.SetToCannotShoot();
	}
	else
	{
		cShootStatus.Update(dElapsedTime);
	}

	// Constraint the player within the map
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
		if ((cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::JUMP) || (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::DOUBLEJUMP))
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);
		}
		//else if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::FALL)
		//{
		//	cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);
		//}
	}

	// Interact with the Map
	InteractWithMap();

	//CS: Update the animated sprite
	pAnimatedSprites->Update(dElapsedTime);

	return true;
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CPlayer2D::PreRender(void)
{
	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render this instance
 */
void CPlayer2D::Render(void)
{
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(vec2Position, 0.0f));

	//model = glm::translate(model, glm::vec3(0.5f * pSettings->TILE_WIDTH, 0.5f * pSettings->TILE_HEIGHT, 0.0f));
	//model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
	//model = glm::translate(model, glm::vec3(-0.5f * pSettings->TILE_WIDTH, -0.5f * pSettings->TILE_HEIGHT, 0.0f));

	model = glm::scale(model, glm::vec3(25.0f, 25.0f, 1.0f));
	
	// note: currently we set the projection matrix each frame, but since the projection 
	// matrix rarely changes it's often best practice to set it outside the main loop only once.
	CShaderManager::GetInstance()->pActiveShader->setMat4("Model", model);
	CShaderManager::GetInstance()->pActiveShader->setMat4("Projection", projection);
	unsigned int colourLoc = glGetUniformLocation(CShaderManager::GetInstance()->pActiveShader->ID, "ColourTint");
	glUniform4fv(colourLoc, 1, glm::value_ptr(vec4ColourTint));

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);
		//CS: Render the animated sprite
		glBindVertexArray(VAO);
			pAnimatedSprites->Render();
		glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	//CEntity2D::PreRender();
	//CEntity2D::Render();
	//CEntity2D::PostRender();

	return;
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CPlayer2D::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

CPlayer2D::ToodeeState CPlayer2D::SaveState() const
{
	return {
		vec2Position,
		vec2MovementVelocity,
		vec4ColourTint,
		GetStatus()
	};
}

void CPlayer2D::LoadState(const ToodeeState& state)
{
	vec2Position = state.position;
	vec2MovementVelocity = state.velocity;
	vec4ColourTint = state.colourTint;
}

/**
 @brief Let player interact with the map. You can add collectibles such as powerups and health here.
 Decides whether smth gets destroyed after player passes it or not
 */
void CPlayer2D::InteractWithMap(void)
{
	int iPositionX = 0;
	int iPositionY = 0;
	if (pMap2D->GetTileIndexAtPosition(vec2Position, iPositionX, iPositionY) == false)
		return;

	switch (pMap2D->GetMapInfo(iPositionY, iPositionX))
	{
	case 2:
		// Erase the tree from this position
		pMap2D->SetMapInfo(iPositionY, iPositionX, 0);
		// Increase the Tree by 1
		pInventoryItem = pInventoryManager->GetItem("Tree");
		pInventoryItem->Add(1);
		break;
	case 10:
		// Erase the life from this position
		pMap2D->SetMapInfo(iPositionY, iPositionX, 0);
		// Increase the lives by 1
		pInventoryItem = pInventoryManager->GetItem("Lives");
		pInventoryItem->Add(1);
		break;
	case 20:
		// Decrease the health by 1
		pInventoryItem = pInventoryManager->GetItem("Health");
		pInventoryItem->Remove(1);
		cout << "Health: " << pInventoryItem->GetCount() << endl;
		break;
	case 21: // health pack
		pMap2D->SetMapInfo(iPositionY, iPositionX, 0);
		// Increase the health
		pInventoryItem = pInventoryManager->GetItem("Health Pack");
		pInventoryItem->Add(20);
		break;
	case 22: // laser gun
		pInventoryManager->BindToCharacter(this);
		// Erase the gun from this position
		pMap2D->SetMapInfo(iPositionY, iPositionX, 0);
		pInventoryItem = pInventoryManager->GetItem("Laser Gun");
		pInventoryItem->Add(1);
		isGunPicked = true;
		break;
	case 99:
		// Level has been completed
		pInventoryItem = pInventoryManager->GetItem("Tree");
		if (pInventoryItem->GetCount() >= 5) {
			CGameManager::GetInstance()->bLevelCompleted = true;
		}
		break;
	default:
		break;
	}
}

void CPlayer2D::Respawn()
{
	cout << "Respawning player";
	vec2Position = vec2StartPosition;
}