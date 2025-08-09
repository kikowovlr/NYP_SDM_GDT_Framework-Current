#include "BlockInfo.h"
#include "Map2D.h"
#include <vector>

// Global vector holding info about all blocks in the map
std::vector<BlockInfo> crateInfos;

// The function to initialize blocks on map load
void InitializeBlockInfos(const CMap2D* pMap2D)
{
    crateInfos.clear();

    CMap2D* map = pMap2D->GetInstance();

    const glm::vec2 tileSize = map->GetTileSize();

    // Find all tile positions with value 102 (crates)
    std::vector<glm::vec2> crateTiles = map->FindAllValues(102);

    for (const glm::vec2 tilePos : crateTiles)
    {
        // convert to world pos
        glm::vec2 worldPos = tilePos * tileSize + tileSize / 2;

        BlockInfo block;
        block.defaultPos = worldPos;
        block.currentPos = worldPos;
        block.hasMoved = false;
        block.isInInventory = false;

        crateInfos.push_back(block);
    }
}