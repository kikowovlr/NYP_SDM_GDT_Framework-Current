/**
 Map2D
 @brief A class which manages the map in the game
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

// Include the RapidCSV
#include "System/rapidcsv.h"
// Include map storage
#include <map>

// Include Settings
#include "GameControl\Settings.h"

// Include Entity2D
#include "Primitives/Entity2D.h"

// A structure storing information about Map Sizes
struct MapSize {
	unsigned int uiRowSize;
	unsigned int uiColSize;
};

// A structure storing information about a map grid
// It includes data to be used for A* Path Finding
struct Grid {
	unsigned int value;

	Grid() 
		: value(0), pos(0, 0), parent(-1, -1), f(0), g(0), h(0) {}
	Grid(	const glm::vec2& pos, unsigned int f) 
		: value(0), pos(pos), parent(-1, 1), f(f), g(0), h(0) {}
	Grid(	const glm::vec2& pos, const glm::vec2& parent, 
			unsigned int f, unsigned int g, unsigned int h) 
		: value(0), pos(pos), parent(parent), f(f), g(g), h(h) {}

	glm::vec2 pos;
	glm::vec2 parent;
	unsigned int f;
	unsigned int g;
	unsigned int h;
};

class CMap2D : public CSingletonTemplate<CMap2D>, public CEntity2D
{
	friend CSingletonTemplate<CMap2D>;
public:
	enum DIRECTION
	{
		LEFT = 0,
		RIGHT = 1,
		UP = 2,
		DOWN = 3,
		NUM_DIRECTIONS
	};

	// Init
	bool Init(	const unsigned int uiNumLevels = 1,
				const unsigned int uiNumRows = 24,
				const unsigned int uiNumCols = 32);

	// Update
	bool Update(const double dElapsedTime);

	// Constraint a position within the boundaries of the map
	bool Constraint(glm::vec2& vec2Position);

	// PreRender
	void PreRender(void);

	// Render
	void Render(void);

	// PostRender
	void PostRender(void);

	// Set Tile Size
	void SetTileSize(const float x, const float y);
	void SetTileHalfSize(const float x, const float y);

	// Get Tile Sizes
	glm::vec2 GetTileSize(void);
	glm::vec2 GetTileHalfSize(void);
	glm::vec2 GetSizeTolerance(void);

	// Set the specifications of the map
	void SetNumTiles(const CSettings::AXIS sAxis, const unsigned int uiValue);

	// Get indices of tiles at a position
	bool GetTileIndexAtPosition(const glm::vec2 aPosition, int& iTileIndexX, int& iTileIndexY);
	// Get indices in X-axis of a tile at a position
	bool GetTileIndexXAtPosition(const glm::vec2 vec2Position, int& iTileIndexEndX);
	// Get indices in Y-axis of a tile at a position
	bool GetTileIndexYAtPosition(const glm::vec2 vec2Position, int& iTileIndexEndY);

	// Set the value at certain indices in the arrMapInfo
	void SetMapInfo(const unsigned int uiRow, const unsigned int uiCol, const int iValue, const bool bInvert = true);

	// Get the value at certain indices in the arrMapInfo
	int GetMapInfo(const unsigned int uiRow, const unsigned int uiCol, const bool bInvert = true) const;

	// Load a map
	bool LoadMap(string filename, const unsigned int uiLevel = 0);

	// Save a tilemap
	bool SaveMap(string filename, const unsigned int uiLevel = 0);

	// Find the indices of a certain value in arrMapInfo
	bool FindValue(const int iValue, unsigned int& uirRow, unsigned int& uirCol, const bool bInvert = true);

	// Set current level
	void SetCurrentLevel(unsigned int uiCurLevel);
	// Get current level
	unsigned int GetCurrentLevel(void) const;

	// Print out details about this class instance in the console window
	void PrintSelf(void) const;

	// Check for collision with the tile maps vertically when an object moves from vec2StartPosition to vec2EndPosition
	CSettings::RESULTS CheckVerticalCollision(glm::vec2 vec2StartPosition,
		glm::vec2 vec2HalfSize,
		glm::vec2& vec2EndPosition,
		float& fCollisionCoordY,
		const bool bApplyTolerance = true);
	// Check for collision with the tile maps horizontally when an object moves from vec2StartPosition to vec2EndPosition
	CSettings::RESULTS CheckHorizontalCollision(glm::vec2 vec2StartPosition,
		glm::vec2 vec2HalfSize,
		glm::vec2& vec2EndPosition,
		float& fCollisionCoordX,
		const bool bApplyTolerance = true);

protected:
	// The variable containing the rapidcsv::Document
	// We will load the CSV file's content into this Document
	rapidcsv::Document doc;

	// A 3-D array which stores the values of the tile map
	Grid*** arrMapInfo;

	// The current level
	unsigned int uiCurLevel;
	// The number of levels
	unsigned int uiNumLevels;

	// A 1-D array which stores the map sizes for each level
	MapSize* arrMapSizes;

	// Tile Sizes
	glm::vec2 vec2TileSize;
	glm::vec2 vec2TileHalfSize;
	const glm::vec2 vec2SizeTolerance = glm::vec2(1.f, 1.f); // higher value makes it easier for the player to navigate the map

	// Map containing texture IDs
	map<int, int> MapOfTextureIDs;

	//CS: The pQuadMesh for drawing the tiles
	CMesh* pQuadMesh;

	// Current vec4ColourTint
	glm::vec4 vec4ColourTint;

	// Constructor
	CMap2D(void);

	// Destructor
	virtual ~CMap2D(void);

	// Render a tile
	void RenderTile(const unsigned int uiRow, const unsigned int uiCol);
};

