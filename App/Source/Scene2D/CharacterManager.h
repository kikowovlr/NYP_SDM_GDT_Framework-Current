#pragma once

// Include Singleton template
#include "DesignPatterns\SingletonTemplate.h"

//// Include GLEW
//#ifndef GLEW_STATIC
//#include <GL/glew.h>
//#define GLEW_STATIC
//#endif
//
//// Include GLM
//#include <includes/glm.hpp>
//#include <includes/gtc/matrix_transform.hpp>
//#include <includes/gtc/type_ptr.hpp>

// Include CEntity2D
#include "Primitives/Entity2D.h"

// forward declaration
class CPlayer2D;
class CTopdee;
class CEntity2D;
#include "CharacterStates.h"

class CharacterManager : public CSingletonTemplate<CharacterManager>
{
public:
	CharacterManager();
	~CharacterManager();

	// Init
	bool Init(const std::string& shaderName);

	void Render();

	void SwitchCharacter();
	
	CEntity2D* GetActiveCharacter();

	void UpdateCurrentCharacter(float deltaTime);
	
	void ResetAllCharacters();

	void DeactivateAllCharacters();

	bool ShouldEndGame();

	CEntity2D* GetTopdee();
	CEntity2D* GetToodee();

private:
	CEntity2D* activeCharacter;
	CTopdee* topdee;
	CPlayer2D* toodee;

	// Saved states
	TopdeeState* savedTopdeeState = nullptr; // nullptr = "no state"
	ToodeeState* savedToodeeState = nullptr;
};