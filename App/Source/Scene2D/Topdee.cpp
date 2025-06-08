/**
 Player2D
 @brief A class representing the player object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Topdee.h"

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
CTopdee::CTopdee(void)
	: pMap2D(NULL)
	, pKeyboardController(NULL)
	, pMouseController(NULL)
	, pProjectileManager2D(NULL)
	, pInventoryManager(NULL)
	, pInventoryItem(NULL)
	, pCharacterManager(NULL)
{
	// Initialise position of the player
	vec2Position = glm::vec2(0);
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CTopdee::~CTopdee(void)
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

	pCharacterManager = NULL;

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &VAO);
}

/**
  @brief Initialise this instance
  */
bool CTopdee::Init(void)
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

	// Get the handler to the CMap2D instance
	pMap2D = CMap2D::GetInstance();
	// Find the indices for the player in arrMapInfo, and assign it to pPlayer2D
	unsigned int uiRow = -1;
	unsigned int uiCol = -1;
	// start with toodee
	if (pMap2D->FindValue(201, uiRow, uiCol) == false)
		return false;	// Unable to find the start position of the player, so quit this game

	// Erase the value of the player in the arrMapInfo
	pMap2D->SetMapInfo(uiRow, uiCol, 0);

	// Set the start position of the Player to iRow and iCol
	vec2Position = glm::vec2(uiCol * pMap2D->GetTileSize().x + pMap2D->GetTileHalfSize().x,
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
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/topdee.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/topdee.png" << endl;
		return false;
	}

	//CS: Create the animated sprite and setup the animation 
	pAnimatedSprites = CMeshBuilder::GenerateSpriteAnimation(4, 3);// , pSettings->TILE_WIDTH, pSettings->TILE_HEIGHT);
	pAnimatedSprites->AddAnimation("right", 0, 2);
	pAnimatedSprites->AddAnimation("left", 3, 5);
	pAnimatedSprites->AddAnimation("front", 6, 8);
	pAnimatedSprites->AddAnimation("back", 9, 11);
	//CS: Play the "front" animation as default
	pAnimatedSprites->PlayAnimation("front", -1, 1.0f);
	 
	//CS: Init the colour to white
	vec4ColourTint = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Get the handler to the CInventoryManager instance
	pInventoryManager = CInventoryManager::GetInstance();
	// Bind to topdee invetory
	pInventoryManager->BindToCharacter(this);

	pInventoryItem = pInventoryManager->Add("Crate", "Image/crate.png", 1, 0);
	pInventoryItem->vec2Size = glm::vec2(25, 25);

	// Set the Physics to idle status by default
	cPhysics2D.Init();
	cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
	cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);

	pCharacterManager = CharacterManager::GetInstance();

	return true;
}

/**
 @brief Reset this instance
 */
bool CTopdee::Reset()
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

	//CS: Play the "front" animation as default
	pAnimatedSprites->PlayAnimation("front", -1, 1.0f);

	//CS: Init the colour to white
	vec4ColourTint = glm::vec4(1.0, 1.0, 1.0, 1.0);

	return true;
}

/**
 @brief Update this instance
 @param dElapsedTime A const double variable contains the time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
 */
bool CTopdee::Update(const double dElapsedTime)
{

	if (isAtExit) // dont update if at exit
		return true;

	// Reset vec2MovementVelocity
	vec2MovementVelocity = glm::vec2(0.0f);
	// Set the physics horizontal status to idle
	cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
	if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::WALK)
		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);

	if (pKeyboardController->IsKeyDown(GLFW_KEY_A))
		pAnimatedSprites->PlayAnimation("left", -1, 1.0f);
	else if (pKeyboardController->IsKeyDown(GLFW_KEY_D))
		pAnimatedSprites->PlayAnimation("right", -1, 1.0f);
	else if (pKeyboardController->IsKeyDown(GLFW_KEY_S))
		pAnimatedSprites->PlayAnimation("front", -1, 1.0f);
	else if (pKeyboardController->IsKeyDown(GLFW_KEY_W))
		pAnimatedSprites->PlayAnimation("back", -1, 1.0f);

	// Sprinting
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

		// Up-down movement
		if ((pKeyboardController->IsKeyDown(GLFW_KEY_W)) &&
			(pKeyboardController->IsKeyDown(GLFW_KEY_S)))
		{
			vec2MovementVelocity.y = 0.0f;
		}
		else if (pKeyboardController->IsKeyDown(GLFW_KEY_W))
		{
			vec2MovementVelocity.y += vec2WalkSpeed.y * vec2WalkSpeedMultiplier.y;
			if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
				cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::WALK);
		}
		else if (pKeyboardController->IsKeyDown(GLFW_KEY_S))
		{
			vec2MovementVelocity.y -= vec2WalkSpeed.y * vec2WalkSpeedMultiplier.y;
			if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
				cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::WALK);
		}
	}
	else { // slower movement
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

		// Up-down movement
		if ((pKeyboardController->IsKeyDown(GLFW_KEY_W)) &&
			(pKeyboardController->IsKeyDown(GLFW_KEY_S)))
		{
			vec2MovementVelocity.y = 0.0f;
		}
		else if (pKeyboardController->IsKeyDown(GLFW_KEY_W))
		{
			vec2MovementVelocity.y += vec2WalkSpeed.y;
			if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
				cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::WALK);
		}
		else if (pKeyboardController->IsKeyDown(GLFW_KEY_S))
		{
			vec2MovementVelocity.y -= vec2WalkSpeed.y;
			if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::IDLE)
				cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::WALK);
		}
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
	}

	// Check for collision with the Tile Maps vertically
	if ((cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::WALK) &&
		(pMap2D->CheckVerticalCollision(vec2Position, vec2HalfSize, vec2NewPosition, fCollisionCoordY) == CSettings::RESULTS::POSITIVE))
	{
		cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::IDLE);
	}

	// Update the vec2Position with the new position
	vec2Position = vec2NewPosition;

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
	}

	// picking up block mechanic
	if (pKeyboardController->IsKeyPressed(GLFW_KEY_E))
	{
		PickUpOrPutDownBlock();
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
void CTopdee::PreRender(void)
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
void CTopdee::Render(void)
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
	glm::vec4 darkColor = vec4ColourTint;
	// if not active character, darken sprite
	if (this != pCharacterManager->GetActiveCharacter() || isAtExit)
		// Darken the color (values between 0 and 1)
		darkColor = vec4ColourTint * glm::vec4(0.5f, 0.5f, 0.5f, 1.0f); // 50% darker
	glUniform4fv(colourLoc, 1, glm::value_ptr(darkColor));

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
void CTopdee::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

TopdeeState CTopdee::SaveState() const
{
	return {
		vec2Position,
		vec2MovementVelocity,
		vec4ColourTint,
		GetStatus()
	};
}

void CTopdee::LoadState(const TopdeeState& state)
{
	vec2Position = state.position;
	vec2MovementVelocity = state.velocity;
	vec4ColourTint = state.colourTint;
}

void CTopdee::Respawn()
{
	vec2Position = vec2StartPosition;
}

/**
 @brief Let player interact with the map. You can add collectibles such as powerups and health here.
 Decides whether smth gets destroyed after player passes it or not
 */
void CTopdee::InteractWithMap(void)
{
	int iPositionX = 0;
	int iPositionY = 0;
	if (pMap2D->GetTileIndexAtPosition(vec2Position, iPositionX, iPositionY) == false)
		return;

	switch (pMap2D->GetMapInfo(iPositionY, iPositionX))
	{
	case 99:
		// Level has been completed
		if (pKeyboardController->IsKeyPressed(GLFW_KEY_ENTER))
		{
			isAtExit = true;
			// deactive once through the door
			SetStatus(false);
		}
		break;
	default:
		break;
	}
}



glm::ivec2 CTopdee::GetFacingDirection()
{
	// get facing direction based on sprite animation
	if (pAnimatedSprites->GetCurrentAnimationName() == "right")
		return glm::ivec2(1, 0);
	else if (pAnimatedSprites->GetCurrentAnimationName() == "left")
		return glm::ivec2(-1, 0);
	else if (pAnimatedSprites->GetCurrentAnimationName() == "front")
		return glm::ivec2(0, -1);
	else if (pAnimatedSprites->GetCurrentAnimationName() == "back")
		return glm::ivec2(0, 1);

	return glm::ivec2(0, 0);
}

void CTopdee::PickUpOrPutDownBlock()
{
	int iPlayerTileX = 0;
	int iPlayerTileY = 0;
	pMap2D->GetTileIndexAtPosition(vec2Position, iPlayerTileX, iPlayerTileY);
	glm::ivec2 facingDir = GetFacingDirection();
	int iFacingTileX = iPlayerTileX + facingDir.x;
	int iFacingTileY = iPlayerTileY + facingDir.y;

	pInventoryManager->BindToCharacter(this);
	pInventoryItem = pInventoryManager->GetItem("Crate");

	// no block in inventory > pick up block
	if (pInventoryItem->GetCount() <= 0)
	{
		// Check if the CENTER of the facing tile has a crate (ID 102) within 25.0f units
		if (pMap2D->GetMapInfo(iFacingTileY, iFacingTileX) == 102 &&
			glm::distance(vec2Position, glm::vec2(iFacingTileX * 25.0f + 12.5f, iFacingTileY * 25.0f + 12.5f)) <= 26.0f) { // 26 for leeway
			// Remove from map
			pMap2D->SetMapInfo(iFacingTileY, iFacingTileX, 0);
			pInventoryItem->Add(1);
		}
	}
	// block in inventory > put down block
	else
	{
		// check if facing tile is empty
		if (pMap2D->GetMapInfo(iFacingTileY, iFacingTileX) == 0) {
			pMap2D->SetMapInfo(iFacingTileY, iFacingTileX, 102);
			pInventoryItem->Remove(1);
		}
	}
}

bool CTopdee::IsAtExit() const
{
	return isAtExit;
}
