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
#include "CharacterManager.h"  // Include full class to use functions

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
	, pCharacterManager(NULL)
	, pSoundController(NULL)
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

	pCharacterManager = NULL;

	pSoundController = NULL;

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
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/toodee.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/toodee.png" << endl;
		return false;
	}

	//CS: Create the animated sprite and setup the animation 
	pAnimatedSprites = CMeshBuilder::GenerateSpriteAnimation(6, 3);// , pSettings->TILE_WIDTH, pSettings->TILE_HEIGHT);
	pAnimatedSprites->AddAnimation("idle-right", 0, 2);
	pAnimatedSprites->AddAnimation("idle-left", 3, 5);
	pAnimatedSprites->AddAnimation("walk-right", 6, 8);
	pAnimatedSprites->AddAnimation("walk-left", 9, 11);
	pAnimatedSprites->AddAnimation("jump-right", 12, 14);
	pAnimatedSprites->AddAnimation("jump-left", 15, 17);
	//CS: Play the "idle" animation as default
	pAnimatedSprites->PlayAnimation("idle-right", -1, 1.0f);
	 
	//CS: Init the colour to white
	vec4ColourTint = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Get the handler to the CInventoryManager instance
	pInventoryManager = CInventoryManager::GetInstance();

	// Bind to toodee inventory
	pInventoryManager->BindToCharacter(this);

	pInventoryItem = pInventoryManager->Add("Laser Gun", "Image/laser-gun.png",1, 0);
	pInventoryItem->vec2Size = glm::vec2(25, 25);

	pInventoryItem = pInventoryManager->Add("Energy", "Image/energy.png", 50, 0);
	pInventoryItem->vec2Size = glm::vec2(25, 25);
	
	// Set the Physics to fall status by default
	cPhysics2D.Init();
	cPhysics2D.SetHorizontalStatus(CPhysics2D::HORIZONTALSTATUS::IDLE);
	cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::FALL);
	cPhysics2D.SetWallJumpStatus(CPhysics2D::WALLJUMPSTATUS::NOT_ATTACHED);

	pCharacterManager = CharacterManager::GetInstance();

	pSoundController = CSoundController::GetInstance();

	SetName("TOODEE");

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
	cPhysics2D.SetWallJumpStatus(CPhysics2D::WALLJUMPSTATUS::NOT_ATTACHED);

	//CS: Play the "idle" animation as default
	pAnimatedSprites->PlayAnimation("idle-right", -1, 1.0f);

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
	if (isAtExit) // dont update if at exit
		return true;

	// find gun tile
	if (isKeyPicked && !isGunTileFound)
	{
		unsigned int gunRow, gunCol;
		if (pMap2D->FindValue(22, gunRow, gunCol))
		{
			gunTile = glm::ivec2(gunCol, gunRow);
		}
		isGunTileFound = true;
	}

	// Update timers
	if (m_bWallJumpCooldown) {
		m_fWallJumpCooldownTimer -= dElapsedTime;
		if (m_fWallJumpCooldownTimer <= 0.0f)
			m_bWallJumpCooldown = false;
	}

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
	}

	// Handle ALL jump types in one place (priority order: Wall Jump > Ground Jump > Double Jump)
	if (pKeyboardController->IsKeyPressed(GLFW_KEY_SPACE))
	{
		// 1. Wall Jump (highest priority)
		if (CanWallJump()) {
			float horizontalDir = (cPhysics2D.GetWallJumpStatus() == CPhysics2D::WALLJUMPSTATUS::LEFT_WALL) ? 1.0f : -1.0f;
			glm::vec2 vec2WallJumpSpeed = glm::vec2(horizontalDir * WALL_JUMP_HORIZONTAL, WALL_JUMP_VERTICAL);

			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::DOUBLEJUMP);
			cPhysics2D.SetInitialVelocity(vec2WallJumpSpeed);
			cPhysics2D.SetNewJump(true);
			m_bWallJumpCooldown = true;
			m_fWallJumpCooldownTimer = WALL_JUMP_COOLDOWN_TIME;
			pSoundController->PlaySoundByID(3);
		}
		// ground jump
		else if (cPhysics2D.GetVerticalStatus() <= CPhysics2D::VERTICALSTATUS::IDLE)
		{
			cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::JUMP);
			cPhysics2D.SetInitialVelocity(vec2JumpSpeed);
			cPhysics2D.SetNewJump(true);
			pSoundController->PlaySoundByID(3);
		}
		// double jump
		else if (cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::JUMP ||
			cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::FALL)
		{
			// Only allow if we haven't used double jump yet
			if (!m_bHasDoubleJumped) {
				cPhysics2D.SetVerticalStatus(CPhysics2D::VERTICALSTATUS::DOUBLEJUMP);
				cPhysics2D.SetInitialVelocity(vec2JumpSpeed);
				cPhysics2D.SetNewJump(true);
				m_bHasDoubleJumped = true; // Track double jump usage
				pSoundController->PlaySoundByID(3);
			}
		}
	}

	// reset double jump
	if (IsGrounded())
		m_bHasDoubleJumped = false;

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

	// wall jump mechanic
	UpdateWallDetection(dElapsedTime);

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
			pInventoryManager->BindToCharacter(this);
			pInventoryItem = pInventoryManager->GetItem("Energy");
			if (isGunPicked && pInventoryItem->GetCount() >= 10) // only shoot if player has 10 or more energy and has gun
			{
				glm::vec2 temp = glm::normalize(glm::vec2(pMouseController->GetMousePositionX() - vec2Position.x,
					CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f) -
					pMouseController->GetMousePositionY() - vec2Position.y));
				// Activate a CProjectil2D
				pProjectileManager2D->Activate(vec2Position,
					temp, 2.0, 200.0f, this);

				// remove energy from inventory every time a bullet is shot
				pInventoryItem->Remove(10);

				cShootStatus.SetToCannotShoot();
			}
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
	}

	// Interact with the Map
	InteractWithMap();

	// play sound when close to gun
	if (isKeyPicked && !isGunPicked) {
		glm::vec2 gunWorldPos = glm::vec2(gunTile) * pMap2D->GetTileSize();
		float distanceFromGun = glm::distance(vec2Position, gunWorldPos);

		if (distanceFromGun < 110.f && !hasPlayedGunProximitySFX)
		{
			pSoundController->PlaySoundByID(10);
			hasPlayedGunProximitySFX = true;
			cout << "sound played" << endl;
		}
		else if (distanceFromGun >= 110.f)
		{
			hasPlayedGunProximitySFX = false;
		}
	}

	// Update facing direction based on horizontal velocity
	if (vec2MovementVelocity.x < -0.01f)
		eFacingDirection = FacingDirection::LEFT;
	else if (vec2MovementVelocity.x > 0.01f)
		eFacingDirection = FacingDirection::RIGHT;

	// Animation
	// JUMP or DOUBLE JUMP
	if (cPhysics2D.GetVerticalStatus() >= CPhysics2D::VERTICALSTATUS::JUMP)
	{
		if (eFacingDirection == FacingDirection::LEFT)
			pAnimatedSprites->PlayAnimation("jump-left", 0, 1.0f);
		else
			pAnimatedSprites->PlayAnimation("jump-right", 0, 1.0f);
	}
	// WALKING
	else if (cPhysics2D.GetHorizontalStatus() == CPhysics2D::HORIZONTALSTATUS::WALK)
	{
		if (eFacingDirection == FacingDirection::LEFT)
			pAnimatedSprites->PlayAnimation("walk-left", -1, 1.0f);
		else
			pAnimatedSprites->PlayAnimation("walk-right", -1, 1.0f);
	}
	// IDLE
	else
	{
		if (eFacingDirection == FacingDirection::LEFT)
			pAnimatedSprites->PlayAnimation("idle-left", -1, 1.0f);
		else
			pAnimatedSprites->PlayAnimation("idle-right", -1, 1.0f);
	}

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

ToodeeState CPlayer2D::SaveState() const
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

bool CPlayer2D::IsGunPicked() const
{
	return false;
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
	case 2: // orb
		// Erase the orb from this position
		//pSoundController->	ByID(1);
		pMap2D->SetMapInfo(iPositionY, iPositionX, 0);
		// Increase the Orb by 1
		pInventoryItem = pInventoryManager->GetItem("Orb");
		pInventoryItem->Add(1);
		pSoundController->PlaySoundByID(5);
		break;
	case 21: // health pack
		pMap2D->SetMapInfo(iPositionY, iPositionX, 0);
		// Increase the health
		pInventoryItem = pInventoryManager->GetItem("Health");
		pInventoryItem->Add(20);
		pSoundController->PlaySoundByID(1);
		break;
	case 22: // laser gun
		pInventoryManager->BindToCharacter(this);
		// Erase the gun from this position
		pMap2D->SetMapInfo(iPositionY, iPositionX, 0);
		pInventoryItem = pInventoryManager->GetItem("Laser Gun");
		pInventoryItem->Add(1);
		pInventoryItem = pInventoryManager->GetItem("Energy");
		pInventoryItem->Add(50);
		isGunPicked = true;
		pSoundController->PlaySoundByID(4);
		break;
	case 25:// energy can
	{
		pInventoryManager->BindToCharacter(this);
		// Erase the can from this position
		pMap2D->SetMapInfo(iPositionY, iPositionX, 0);
		pInventoryItem = pInventoryManager->GetItem("Energy");
		pInventoryItem->Add(25);
		// play can opening sound first
		CSoundInfo* pSoundInfo = pSoundController->GetSound(6);
		auto currSound = pSoundController->getSoundEngine()->play2D(pSoundInfo->GetSound(), pSoundInfo->GetLoopStatus(), false, false, true);
		// play slurp after can opening sound is done
		currSound->setSoundStopEventReceiver(new CDelayedSound(7));
		break;
	}
	case 26: // key
		// Erase key
		pMap2D->SetMapInfo(iPositionY, iPositionX, 0);
		isKeyPicked = true;
		// unlock chest -> replace with gun
		UnlockChest();
		pSoundController->PlaySoundByID(2);
		break;
	case 28: // spike
		// Decrease the health by 1
		pInventoryItem = pInventoryManager->GetItem("Health");
		pInventoryItem->Remove(1);
		break;
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

void CPlayer2D::UnlockChest(void)
{
	unsigned int chestRow = 0, chestCol = 0;

	if (pMap2D->FindValue(27, chestRow, chestCol))
	{
		// Convert to tile coordinates if needed
		int tileX = static_cast<int>(chestCol);
		int tileY = static_cast<int>(chestRow);

		// replace chest with gun
		pMap2D->SetMapInfo(chestRow, chestCol, 22);
	}
}

void CPlayer2D::LockChest(void)
{
	unsigned int chestRow = 0, chestCol = 0;

	if (pMap2D->FindValue(22, chestRow, chestCol))
	{
		// Convert to tile coordinates if needed
		int tileX = static_cast<int>(chestCol);
		int tileY = static_cast<int>(chestRow);

		// replace gun with chest
		pMap2D->SetMapInfo(chestRow, chestCol, 27);
		pMap2D->SetMapInfo(keyPos.y, keyPos.x, 26);
	}
}

void CPlayer2D::UpdateWallDetection(const double dElapsedTime)
{
	// reset wall jump status
	cPhysics2D.SetWallJumpStatus(CPhysics2D::WALLJUMPSTATUS::NOT_ATTACHED);
	
	// skip if grounded
	if (IsGrounded())
		return;

	// Calculate new position with wall detection offset
	glm::vec2 vec2NewPosition = vec2Position + vec2MovementVelocity * (float)dElapsedTime;
	float fCollisionCoord = 0;

	// Left wall check
	if (pMap2D->CheckHorizontalCollision(vec2Position, vec2HalfSize,
		vec2NewPosition - glm::vec2(WALL_DETECT_OFFSET, 0),fCollisionCoord) == CSettings::RESULTS::POSITIVE)
	{
		cPhysics2D.SetWallJumpStatus(CPhysics2D::WALLJUMPSTATUS::LEFT_WALL);
	}

	// Right wall check
	else if (pMap2D->CheckHorizontalCollision(vec2Position, vec2HalfSize,
		vec2NewPosition + glm::vec2(WALL_DETECT_OFFSET, 0), fCollisionCoord) == CSettings::RESULTS::POSITIVE)
	{
		cPhysics2D.SetWallJumpStatus(CPhysics2D::WALLJUMPSTATUS::RIGHT_WALL);
	}

	// Wall sliding effect
	if (IsWallAttached() && cPhysics2D.GetVerticalStatus() == CPhysics2D::VERTICALSTATUS::FALL)
	{
		vec2MovementVelocity.y *= 0.6f; // Reduce fall speed
	}
}

bool CPlayer2D::IsWallAttached() const
{
	// return true if player attached to left/right wall
	return cPhysics2D.GetWallJumpStatus() != CPhysics2D::WALLJUMPSTATUS::NOT_ATTACHED;
}

bool CPlayer2D::IsGrounded()
{
	float fCollisionCoordY = 0;
	glm::vec2 vec2FeetPosition = vec2Position - glm::vec2(0.0f, vec2HalfSize.y + 1.0f); // Small offset
	return (pMap2D->CheckVerticalCollision(vec2Position, vec2HalfSize, vec2FeetPosition, fCollisionCoordY) == CSettings::RESULTS::POSITIVE);
}

bool CPlayer2D::CanWallJump() {
	return IsWallAttached() && !m_bWallJumpCooldown;
}

void CPlayer2D::CheckDeath()
{

	Respawn();
}

bool CPlayer2D::IsAtExit() const
{
	return isAtExit;
}

void CPlayer2D::Respawn()
{
	cout << "Respawning player";
	if (isKeyPicked && !isGunPicked)
		LockChest();
	vec2Position = vec2StartPosition;
}

void CPlayer2D::InteractWithDoors()
{
	int iPositionX = 0;
	int iPositionY = 0;
	if (pMap2D->GetTileIndexAtPosition(vec2Position, iPositionX, iPositionY) == false)
		return;

	// Check if player is standing on the entrance door (not exit) and pressed 'E'
	if (pMap2D->IsEntranceDoor(iPositionX, iPositionY) && pKeyboardController->IsKeyPressed(GLFW_KEY_ENTER)) {
		pMap2D->SetAreDoorsUsed(true);
		pMap2D->SetMapInfo(iPositionY, iPositionX, 0);
		// Teleport to exit door
		glm::ivec2 exitPos = pMap2D->GetExitDoorPos();
		pMap2D->SetMapInfo(exitPos.y, exitPos.x, 0);
		vec2Position = glm::vec2((exitPos.x + 0.5f) * 25.f, (exitPos.y + 0.5f) * 25.f); // Center player
	}
}