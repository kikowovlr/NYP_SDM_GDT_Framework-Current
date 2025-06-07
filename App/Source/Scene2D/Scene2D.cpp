/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Scene2D.h"
#include <iostream>
using namespace std;

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

#include "System\filesystem.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CScene2D::CScene2D(void)
	: pMap2D(NULL)
	, pCharacterManager(NULL)
	, pKeyboardController(NULL)
	, pMouseController(NULL)
	, pProjectileManager2D(NULL)
	, pGUI_Scene2D(NULL)
	, pGameManager(NULL)
{
}

/**
 @brief Destructor
 */
CScene2D::~CScene2D(void)
{
	if (pMouseController)
	{
		// We won't delete this since it was created elsewhere
		pMouseController = NULL;
	}

	if (pKeyboardController)
	{
		// We won't delete this since it was created elsewhere
		pKeyboardController = NULL;
	}

	if (pCharacterManager)
	{
		pCharacterManager->Destroy();
		pCharacterManager = NULL;
	}

	if (pMap2D)
	{
		pMap2D->Destroy();
		pMap2D = NULL;
	}

	if (pGUI_Scene2D)
	{
		pGUI_Scene2D->Destroy();
		pGUI_Scene2D = NULL;
	}
	if (pGameManager)
	{
		pGameManager->Destroy();
		pGameManager = NULL;
	}
	// Clear out all the shaders
	//CShaderManager::GetInstance()->Destroy();
}

/**
@brief Init Initialise this instance
*/ 
bool CScene2D::Init(void)
{
	// Create and initialise the Map 2D
	pMap2D = CMap2D::GetInstance();
	// Set a shader to this class
	pMap2D->SetShader("Shader2D");
	// Load Scene2DColour into ShaderManager
	CShaderManager::GetInstance()->Use("Shader2D");

	// Initialise the instance
	if (pMap2D->Init(2, 24, 32) == false)
	{
		cout << "Failed to load CMap2D" << endl;
		return false;
	}
	// Load the map into an array
	if (pMap2D->LoadMap("Maps/DX1212_Map_Level_01.csv") == false)
	{
		// The loading of a map has failed. Return false
		return false;
	}
	// Load the map into an array
	if (pMap2D->LoadMap("Maps/DX1212_Map_Level_02.csv", 1) == false)
	{
		// The loading of a map has failed. Return false
		return false;
	}
	// Load the map into an array
	if (pMap2D->LoadMap("Maps/DX1212_Map_Level_03.csv", 1) == false)
	{
		// The loading of a map has failed. Return false
		return false;
	}

	pCharacterManager = CharacterManager::GetInstance();
	if (!pCharacterManager->Init("Shader2D")) {
		cout << "Failed to load characters" << endl;
		return false;
	}

	// Store the keyboard controller singleton instance here
	pKeyboardController = CKeyboardController::GetInstance();
	// Store the mouse controller singleton instance here
	pMouseController = CMouseController::GetInstance();

	pProjectileManager2D = CProjectileManager2D::GetInstance();

	// This set of codes should be removed once GameStateManagement has been added.
	// Enable the cursor
	if (CSettings::GetInstance()->cSimpleIniA.GetBoolValue("Mouse", "bUseMousePointerForMenu", "false") == true)
		glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Store the pGUI_Scene2D singleton instance here
	pGUI_Scene2D = CGUI_Scene2D::GetInstance();
	pGUI_Scene2D->Init();

	// Game Manager
	pGameManager = CGameManager::GetInstance();
	pGameManager->Init();

	// Debug inventories
	CInventoryManager::GetInstance()->DebugPrintAllInventories(pCharacterManager->GetTopdee(), pCharacterManager->GetToodee());

	return true;
}

/**
 @brief Update Update this instance
 @param dElapsedTime A const double variable containing the elapsed time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
 */
bool CScene2D::Update(const double dElapsedTime)
{
	// Call the pPlayer2D's update method before Map2D as we want to capture the inputs before map2D update
	pCharacterManager->UpdateCurrentCharacter(dElapsedTime);

	// Call the Map2D's update method
	pMap2D->Update(dElapsedTime);

	// Call the pProjectileManager2D's update method
	pProjectileManager2D->Update(dElapsedTime);

	// Get keyboard updates
	if (pKeyboardController->IsKeyReleased(GLFW_KEY_F6))
	{
		// Save the current game to a save file
		// Make sure the file is open
		try {
			if (pMap2D->SaveMap("Maps/DX1212_Map_Level_01_SAVEGAMEtest.csv") == false)
			{
				throw runtime_error("Unable to save the current game to a file");
			}
		}
		catch (runtime_error e)
		{
			cout << "Runtime error: " << e.what();
			return false;
		}
	}

	pGUI_Scene2D->Update(dElapsedTime);

	// Check if the game should go to the next level
	if (pGameManager->bLevelCompleted == true)
	{
		pMap2D->SetCurrentLevel(pMap2D->GetCurrentLevel() + 1);
		pCharacterManager->ResetAllCharacters();
		pGameManager->bLevelCompleted = false;
	}

	// Check if the game has been won by the player
	if (pGameManager->bPlayerWon == true)
	{
		// End the game and switch to Win screen
	}
	// Check if the game should be ended
	else if (pGameManager->bPlayerLost == true)
	{
		pCharacterManager->DeactivateAllCharacters();;
		return false; // closes the app
	}

	return true;
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void CScene2D::PreRender(void)
{
	// Reset the OpenGL rendering environment
	glLoadIdentity();

	// Clear the screen and buffer
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Enable 2D texture rendering
	glEnable(GL_TEXTURE_2D);
}

/**
 @brief Render Render this instance
 */
void CScene2D::Render(void)
{
	// Call the Map2D's PreRender()
	pMap2D->PreRender();
	// Call the Map2D's Render()
	pMap2D->Render();
	// Call the Map2D's PostRender()
	pMap2D->PostRender();

	pCharacterManager->Render();

	// Call the pProjectileManager2D's PreRender()
	pProjectileManager2D->PreRender();
	// Call the pProjectileManager2D's Render()
	pProjectileManager2D->Render();
	// Call the pProjectileManager2D's PostRender()
	pProjectileManager2D->PostRender();

	// Call the pGUI_Scene2D's PreRender()
	pGUI_Scene2D->PreRender();
	// Call the pGUI_Scene2D's Render()
	pGUI_Scene2D->Render();
	// Call the pGUI_Scene2D's PostRender()
	pGUI_Scene2D->PostRender();
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CScene2D::PostRender(void)
{
}