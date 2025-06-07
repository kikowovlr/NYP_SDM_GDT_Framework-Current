/**
 InventoryManager
 @brief A class which manages all the inventory items the game
 By: Toh Da Jun
 Date: Mar 2020
 */

#include "InventoryManager.h"
#include <stdexcept>      // std::invalid_argument
#include <iostream>

/**
@brief Constructor
*/
CInventoryManager::CInventoryManager(void) 
{
}

/**
@brief Destructor
*/
CInventoryManager::~CInventoryManager(void)
{
	// Clear the memory
	Destroy();
}

/**
@brief Destroy this instance
*/
void CInventoryManager::Destroy(void)
{
	// Delete all scenes stored and empty the entire map
	std::map<std::string, CInventoryItem*>::iterator it, end;
	end = inventoryMap.end();
	for (it = inventoryMap.begin(); it != end; ++it)
	{
		delete it->second;
		it->second = nullptr;
	}
	inventoryMap.clear();
}

/**
@brief Add a Scene to this Inventory Manager
*/
//CInventoryItem* CInventoryManager::Add(	const std::string& _name,
//										const char* imagePath,
//										const int iItemMaxCount,
//										const int iItemCount)
//{
//	if (Check(_name))
//	{
//		// Item name already exist here, unable to proceed
//		throw std::exception("Duplicate item name provided");
//		return NULL;
//	}
//
//	CInventoryItem* cNewItem = new CInventoryItem(imagePath);
//	cNewItem->iItemMaxCount = iItemMaxCount;
//	cNewItem->iItemCount = iItemCount;
//
//	// Nothing wrong, add the scene to our map
//	inventoryMap[_name] = cNewItem;
//
//	return cNewItem;
//}

CInventoryItem* CInventoryManager::Add(
	const std::string& _name,
	const char* imagePath,
	const int iItemMaxCount,
	const int iItemCount)
{
	// Get current character's inventory (create if doesn't exist)
	auto& charInventory = characterInventories[activeCharacter];

	// Check for duplicates in THIS CHARACTER'S inventory only
	if (charInventory.find(_name) != charInventory.end()) {
		throw std::exception("Duplicate item name for this character");
		return nullptr;
	}

	// Create new item
	CInventoryItem* cNewItem = new CInventoryItem(imagePath);
	cNewItem->iItemMaxCount = iItemMaxCount;
	cNewItem->iItemCount = iItemCount;

	// Add to current character's inventory
	charInventory[_name] = cNewItem;

	return cNewItem;
}

/**
@brief Remove an item from this Inventory Manager
*/

//bool CInventoryManager::Remove(const std::string& _name)
//{
//	// Does nothing if it does not exist
//	if (Check(_name))
//	{
//		// Item is not available, unable to proceed
//		throw std::exception("Unknown item name provided");
//		return false;
//	}
//
//	CInventoryItem* target = inventoryMap[_name];
//
//	// Delete and remove from our map
//	delete target;
//	inventoryMap.erase(_name);
//
//	return true;
//}
bool CInventoryManager::Remove(const std::string& _name)
{
	// If no character is bound, use original inventory
	if (!activeCharacter) {
		if (!Check(_name)) {
			throw std::exception("Unknown item name provided");
			return false;
		}

		delete inventoryMap[_name];
		inventoryMap.erase(_name);
		return true;
	}

	// Handle character-specific inventory
	auto& charInventory = characterInventories[activeCharacter];
	auto it = charInventory.find(_name);

	if (it == charInventory.end()) {
		throw std::exception("Item not found in current character's inventory");
		return false;
	}

	// Delete and remove from character's inventory
	delete it->second;
	charInventory.erase(it);
	return true;
}

/**
@brief Check if a item exists in this Inventory Manager
*/
//bool CInventoryManager::Check(const std::string& _name)
//{
//	return inventoryMap.count(_name) != 0;
//}
bool CInventoryManager::Check(const std::string& _name)
{
	// Check current character's inventory first
	if (activeCharacter) {
		auto& charInventory = characterInventories[activeCharacter];
		if (charInventory.find(_name) != charInventory.end()) {
			return true;
		}
	}

	// Fallback to global inventory
	return (inventoryMap.find(_name) != inventoryMap.end());
}

/**
@brief Get an item by its name
*/ 
//CInventoryItem* CInventoryManager::GetItem(const std::string& _name)
//{
//	// Does nothing if it does not exist
//	if (!Check(_name))
//		return NULL;
//
//	// Find and return the item
//	return inventoryMap[_name];
//}
CInventoryItem* CInventoryManager::GetItem(const std::string& _name)
{
	// Check character-bound inventory first
	if (activeCharacter) {
		auto& charInventory = characterInventories[activeCharacter];
		auto it = charInventory.find(_name);
		if (it != charInventory.end()) {
			return it->second;
		}
	}

	// Fallback to global inventory
	auto it = inventoryMap.find(_name);
	return (it != inventoryMap.end()) ? it->second : nullptr;
}

/**
@brief Get the number of items
*/
//int CInventoryManager::GetNumItems(void) const
//{
//	return inventoryMap.size();
//}
int CInventoryManager::GetNumItems() const
{
	// Return count for current character's inventory
	if (activeCharacter) {
		auto it = characterInventories.find(activeCharacter);
		if (it != characterInventories.end()) {
			return static_cast<int>(it->second.size());
		}
		return 0;
	}

	// Fallback to global inventory count
	return static_cast<int>(inventoryMap.size());
}

void CInventoryManager::BindToCharacter(CEntity2D* character)
{
	activeCharacter = character;
	if (characterInventories.find(character) == characterInventories.end()) {
		characterInventories[character] = {}; // Initialize empty inventory
	}
}

void CInventoryManager::DebugPrintAllInventories(CEntity2D* topdee, CEntity2D* toodee) const
{
	std::cout << "\n=== INVENTORY DEBUG REPORT ===" << std::endl;

	// Global Inventory
	std::cout << "[GLOBAL INVENTORY]" << std::endl;
	for (const auto& entry : inventoryMap) {  // 'entry' instead of 'name, item'
		const std::string& itemName = entry.first;          // Key (string)
		CInventoryItem* pItem = entry.second;               // Value (CInventoryItem*)
		std::cout << "- " << itemName << ": "
			<< pItem->iItemCount << "/" << pItem->iItemMaxCount << std::endl;
	}

	// Character Inventories
	DebugPrintCharacterInventory(topdee, "TOPDEE");
	DebugPrintCharacterInventory(toodee, "TOODEE");
}


void CInventoryManager::DebugPrintCharacterInventory(CEntity2D* character, const std::string& charName) const
{
	auto charIt = characterInventories.find(character);
	if (charIt != characterInventories.end()) {
		std::cout << "[" << charName << " INVENTORY]" << std::endl;
		for (const auto& entry : charIt->second) {  // charIt->second = character's item map
			const std::string& itemName = entry.first;
			CInventoryItem* pItem = entry.second;
			std::cout << "- " << itemName << ": "
				<< pItem->iItemCount << "/" << pItem->iItemMaxCount << std::endl;
		}
	}
	else {
		std::cout << "[" << charName << " INVENTORY] (Empty)" << std::endl;
	}
}