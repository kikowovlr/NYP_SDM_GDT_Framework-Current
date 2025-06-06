/**
 CInventoryManager
 @brief A class which manages all the inventory items the game
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include SingletonTemplate
#include "DesignPatterns\SingletonTemplate.h"

#include <map>
#include <string>
#include "InventoryItem.h"
#include <unordered_map>

class CInventoryManager : public CSingletonTemplate<CInventoryManager>
{
	friend CSingletonTemplate<CInventoryManager>;
public:
	// Destroy this instance
	void Destroy(void);

	// Add a new item
	CInventoryItem* Add(	const std::string& _name,
							const char* imagePath, 
							const int iItemMaxCount, 
							const int iItemCount = 0);
	// Remove an item
	bool Remove(const std::string& _name);
	// Check if an item exists in inventoryMap
	bool Check(const std::string& _name);

	// Get an item by its name
	CInventoryItem* GetItem(const std::string& _name);
	// Get the number of items
	int GetNumItems(void) const;

	// Bind inventory to character
	void BindToCharacter(CEntity2D* character);



protected:
	// Constructor
	CInventoryManager(void);

	// Destructor
	virtual ~CInventoryManager(void);

	// The map containing all the items
	std::map<std::string, CInventoryItem*> inventoryMap;

	std::unordered_map<CEntity2D*, std::map<std::string, CInventoryItem*>> characterInventories;
	CEntity2D* activeCharacter = nullptr;
};
