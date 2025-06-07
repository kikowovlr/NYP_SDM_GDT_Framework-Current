/**
 CGameManager
 @brief This class manages the gameplay and determines if it should go to next level or end.
 By: Toh Da Jun
 Date: May 2021
 */
#include "GameManager.h"	

void CGameManager::UpdatePlayerState()
{
	// Access global inventory (no character binding needed)
	CInventoryManager* pInventory = CInventoryManager::GetInstance();
	CInventoryItem* pHealth = pInventory->GetItem("Health");
	CInventoryItem* pLives = pInventory->GetItem("Lives");

	// Check health
	if (pHealth->GetCount() <= 0)
	{
		// Reset health and deduct lives
		pHealth->iItemCount = pHealth->GetMaxCount();
		pLives->Remove(1);

		// Respawn player (assuming you have access to player position)
		if (CEntity2D* pCurrentPlayer = CharacterManager::GetInstance()->GetActiveCharacter())
		{
			pCurrentPlayer->Respawn();
		}

		// Game over check
		if (pLives->GetCount() <= 0)
		{
			bPlayerLost = true;
		}
	}
}

/**
@brief Constructor
*/
CGameManager::CGameManager(void) 
	: bPlayerWon(false)
	, bPlayerLost(false)
	, bLevelCompleted(false)
	, bLevelToReplay(false)
	, bGameToRestart(false) {
}

/**
@brief Destructor
*/
CGameManager::~CGameManager(void)
{
}

// Initialise this class instance
bool CGameManager::Init(void)
{
	bPlayerWon = false;
	bPlayerLost = false;
	bLevelCompleted = false;
	bLevelToReplay = false;
	bGameToRestart = false;
	CInventoryManager* pInventoryManager = CInventoryManager::GetInstance();
	// Add global items (no need to store pointers)
	pInventoryManager->BindToCharacter(nullptr);

	// Add a Lives icon as one of the inventory items
	CInventoryItem* pInventoryItem = pInventoryManager->Add("Lives", "Image/Scene2D_Lives.tga", 3, 3);
	pInventoryItem->vec2Size = glm::vec2(25, 25);

	// Add a Health icon as one of the inventory items
	pInventoryItem = pInventoryManager->Add("Health", "Image/Scene2D_Health.tga", 100, 100);
	pInventoryItem->vec2Size = glm::vec2(25, 25);

	pInventoryItem = pInventoryManager->Add("Tree", "Image/Scene2D_TreeTile.tga", 5, 0);
	pInventoryItem->vec2Size = glm::vec2(25, 25);

	pInventoryItem = pInventoryManager->Add("Health Pack", "Image/health-pack.png", 3, 0);
	pInventoryItem->vec2Size = glm::vec2(25, 25);

	return true;
}

