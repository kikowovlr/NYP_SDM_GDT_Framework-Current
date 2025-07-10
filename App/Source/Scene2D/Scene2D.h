/**
 CScene2D
 @brief A class which manages the 2D game scene
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include SingletonTemplate
#include "DesignPatterns\SingletonTemplate.h"

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"
// Include CPlayer2D
#include "Player2D.h"

// Include vector
#include <vector>

// Include Keyboard controller
#include "Inputs\KeyboardController.h"
// Include Mouse controller
#include "Inputs\MouseController.h"

#include "ProjectileManager2D.h"

#include "GUI_Scene2D.h"

// Game Manager
#include "GameManager.h"

#include "CharacterManager.h"

// Include CEnemy2DManager
#include "Enemy2DManager.h"

// Include CSoundController
#include "..\SoundController\SoundController.h"

class CScene2D : public CSingletonTemplate<CScene2D>
{
	friend CSingletonTemplate<CScene2D>;
public:
	// Init
	bool Init(void);

	// Update
	bool Update(const double dElapsedTime);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

protected:
	// Constructor
	CScene2D(void);
	// Destructor
	virtual ~CScene2D(void);

	// The handler containing the instance of the 2D Map
	CMap2D* pMap2D;
	CharacterManager* pCharacterManager;

	// Keyboard Controller singleton instance
	CKeyboardController* pKeyboardController;
	// Mouse Controller singleton instance
	CMouseController* pMouseController;

	CProjectileManager2D* pProjectileManager2D;

	CGUI_Scene2D* pGUI_Scene2D;

	CGameManager* pGameManager;

	// The handler containing the instance of CEnemy2DManager
	CEnemy2DManager* pEnemy2DManager;

	// Handle to the CSoundController
	CSoundController* pSoundController;
};

