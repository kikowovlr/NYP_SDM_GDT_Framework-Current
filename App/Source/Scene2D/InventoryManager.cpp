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
	inventoryMap.clear(); // Explicit initialization
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
	// Delete global inventory items
	for (auto& pair : inventoryMap) {
		delete pair.second;
	}
	inventoryMap.clear();

	// Delete character inventories
	for (auto& charPair : characterInventories) {
		for (auto& itemPair : charPair.second) {
			delete itemPair.second;
		}
		charPair.second.clear();
	}
	characterInventories.clear();
}

/**
@brief Add a Scene to this Inventory Manager
*/
CInventoryItem* CInventoryManager::Add(
	const std::string& _name,
	const char* imagePath,
	const int iItemMaxCount,
	const int iItemCount)
{
	auto& currentInventory = (activeCharacterID == 0) ? inventoryMap : characterInventories[activeCharacterID];

	if (currentInventory.find(_name) != currentInventory.end()) {
		throw std::exception("Duplicate item name");
		return nullptr;
	}

	CInventoryItem* newItem = new CInventoryItem(imagePath);
	newItem->iItemMaxCount = iItemMaxCount;
	newItem->iItemCount = iItemCount;

	currentInventory[_name] = newItem;
	return newItem;
}

/**
@brief Remove an item from this Inventory Manager
*/
bool CInventoryManager::Remove(const std::string& _name)
{
	auto& currentInventory = (activeCharacterID == 0) ? inventoryMap : characterInventories[activeCharacterID];

	auto it = currentInventory.find(_name);
	if (it == currentInventory.end()) {
		throw std::exception("Item not found");
		return false;
	}

	delete it->second;
	currentInventory.erase(it);
	return true;
}

/**
@brief Check if a item exists in this Inventory Manager
*/
bool CInventoryManager::Check(const std::string& _name)
{
	auto& currentInventory = (activeCharacterID == 0) ? inventoryMap : characterInventories[activeCharacterID];

	return currentInventory.find(_name) != currentInventory.end();
}

/**
@brief Get an item by its name
*/ 
CInventoryItem* CInventoryManager::GetItem(const std::string& _name)
{
	auto& currentInventory = (activeCharacterID == 0) ? inventoryMap : characterInventories[activeCharacterID];

	auto it = currentInventory.find(_name);
	return (it != currentInventory.end()) ? it->second : nullptr;
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
	if (activeCharacterID == 0) {
		return static_cast<int>(inventoryMap.size());
	}

	auto it = characterInventories.find(activeCharacterID);
	if (it != characterInventories.end()) {
		return static_cast<int>(it->second.size());
	}

	return 0;
}

void CInventoryManager::BindToCharacter(CEntity2D* character)
{
	if (!character) {
		// bind to global inventory
		activeCharacterID = 0; // Use 0 for global inventory
		if (characterInventories.find(0) == characterInventories.end()) {
			characterInventories[0] = {};
		}
	}
	else {
		activeCharacterID = character->GetUniqueID();
		//std::cout << "Binding inventory to character ID: " << activeCharacterID << std::endl;
		//std::cout << "Name: " << character->GetName() << std::endl;
		if (characterInventories.find(activeCharacterID) == characterInventories.end()) {
			characterInventories[activeCharacterID] = {};
		}
	}
}

void CInventoryManager::DebugPrintAllInventories() const
{
	std::cout << "\n=== INVENTORY DEBUG REPORT ===" << std::endl;

	// Global Inventory
	std::cout << "[GLOBAL INVENTORY]" << std::endl;
	for (const auto& pair : inventoryMap) {
		std::cout << "- " << pair.first << ": "
			<< pair.second->iItemCount << "/" << pair.second->iItemMaxCount << std::endl;
	}

	// Character inventories
	std::cout << "\n[CHARACTER INVENTORIES]" << std::endl;
	for (const auto& charPair : characterInventories) {
		if (charPair.first == 0) continue; // skip global here

		int characterID = charPair.first;
		std::cout << "[Character ID " << characterID << " Inventory]" << std::endl;

		for (const auto& itemPair : charPair.second) {
			std::cout << "- " << itemPair.first << ": "
				<< itemPair.second->iItemCount << "/" << itemPair.second->iItemMaxCount << std::endl;
		}
	}
}

void CInventoryManager::DebugPrintCharacterInventory(int characterID, const std::string& charName) const
{
	auto charIt = characterInventories.find(characterID);
	if (charIt != characterInventories.end()) {
		std::cout << "[" << charName << " INVENTORY]" << std::endl;
		for (const auto& entry : charIt->second) {
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