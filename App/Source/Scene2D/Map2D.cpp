/**
 Map2D
 @brief A class which manages the map in the game
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Map2D.h"

// Include Shader Manager
#include "RenderControl\ShaderManager.h"

// Include Filesystem
#include "System\filesystem.h"
// Include ImageLoader
#include "System\ImageLoader.h"
#include "Primitives/MeshBuilder.h"

// CFPSCounter
#include "TimeControl/FPSCounter.h"

#include <iostream>
#include <vector>

#include "Player2D.h"
using namespace std;

// For AStar PathFinding
using namespace std::placeholders;

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CMap2D::CMap2D(void)
	: uiCurLevel(0)
	, pQuadMesh(NULL)
	, vec2TileSize(glm::vec2(25.0f, 25.0f))
	, vec2TileHalfSize(glm::vec2(12.5f, 12.5f))
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CMap2D::~CMap2D(void)
{
	// Dynamically deallocate the 3D array used to store the map information
	for (unsigned int uiLevel = 0; uiLevel < uiNumLevels; uiLevel++)
	{
		for (unsigned int iRow = 0; iRow < (unsigned int)pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24); iRow++)
		{
			delete[] arrMapInfo[uiLevel][iRow];
		}
		delete [] arrMapInfo[uiLevel];
	}
	delete[] arrMapInfo;

	if (pQuadMesh)
	{
		delete pQuadMesh;
		pQuadMesh = NULL;
	}

	// Set this to NULL since it was created elsewhere, so we let it be deleted there.
	pSettings = NULL;

	// Delete AStar lists
	DeleteAStarLists();
}

/**
@brief Init Initialise this instance
*/ 
bool CMap2D::Init(	const unsigned int uiNumLevels,
					const unsigned int uiNumRows,
					const unsigned int uiNumCols)
{
	// Get the handler to the CSettings instance
	pSettings = CSettings::GetInstance();

	// Create the arrMapInfo and initialise to 0
	// Start by initialising the number of levels
	arrMapInfo = new Grid** [uiNumLevels];
	for (unsigned uiLevel = 0; uiLevel < uiNumLevels; uiLevel++)
	{
		arrMapInfo[uiLevel] = new Grid* [uiNumRows];
		for (unsigned uiRow = 0; uiRow < uiNumRows; uiRow++)
		{
			arrMapInfo[uiLevel][uiRow] = new Grid[uiNumCols];
			for (unsigned uiCol = 0; uiCol < uiNumCols; uiCol++)
			{
				arrMapInfo[uiLevel][uiRow][uiCol].value = 0;
			}
		}
	}

	// Store the map sizes in pSettings
	uiCurLevel = 0;
	this->uiNumLevels = uiNumLevels;
	CSettings::GetInstance()->cSimpleIniA.SetValue("NumTiles", "NUM_TILES_XAXIS", (std::to_string(uiNumCols)).c_str());
	CSettings::GetInstance()->cSimpleIniA.SetValue("NumTiles", "NUM_TILES_YAXIS", (std::to_string(uiNumRows)).c_str());

	pSettings->UpdateSpecifications();

	// Set up the projection matrix
	projection = glm::ortho(0.0f,
							static_cast<float>(CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowWidth", 800.0f)),
							0.0f,
							static_cast<float>(CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f)),
							-1.0f, 1.0f);
	model = glm::mat4(1.0f);
	//CS: Init the colour to white
	vec4ColourTint = glm::vec4(1.0, 1.0, 1.0, 1.0);

	//CS: Create the Quad Mesh using the mesh builder
	//pQuadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), pSettings->TILE_WIDTH, pSettings->TILE_HEIGHT);
	pQuadMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), 1, 1);

	// Load and create textures
	// Load the ground texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/ground.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/ground.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(100, iTextureID));
	}
	// dirt
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/dirt.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/dirt.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(101, iTextureID));
	}
	// crate
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/crate.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/crate.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(102, iTextureID));
	}
	// port off
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/port-off.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/port-off.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(103, iTextureID));
	}
	// port on
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/port-on.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/port-on.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(104, iTextureID));
	}

	// COLLECTABLES
	// Load the health pack texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/health-pack.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/health-pack.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(21, iTextureID));
	}
	// Load the gun texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/laser-gun.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/laser-gun.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(22, iTextureID));
	}
	// closed door
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/closed-door.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/closed-door.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(23, iTextureID));
	}
	// opened door
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/opened-door.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/opened-door.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(24, iTextureID));
	}
	// energy can
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/energy-can.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/energy-can.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(25, iTextureID));
	}
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/key.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/key.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(26, iTextureID));
	}
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/chest.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/chest.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(27, iTextureID));
	}
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/spike.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/spike.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(28, iTextureID));
	}

	// Load the orb texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/orb.png", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/orb.png" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(2, iTextureID));
	}
	// Load the Life texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_Lives.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_Lives.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(10, iTextureID));
	}

	// Load the Exit texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_Exit.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_Exit.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(99, iTextureID));
	}

	// Initialise the variables for AStar
	iWeight = 1;
	vec2StartPos = glm::vec2(0, 0);
	vec2TargetPos = glm::vec2(0, 0);
	//m_size = pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24)* pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_XAXIS", 32);

	iNumDirections = 4;
	vDirections = { { -1, 0 }, { 1, 0 }, { 0, 1 }, { 0, -1 },
						{ -1, -1 }, { 1, 1 }, { -1, 1 }, { 1, -1 } };

	// Resize these 2 lists
	vCameFromList.resize(pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24) * pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_XAXIS", 32));
	vClosedList.resize(pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24) * pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_XAXIS", 32), false);

	//// Clear AStar memory
	//ClearAStar();

	return true;
}

/**
 @brief Update Update this instance
 @param dElapsedTime A const double variable containing the elapsed time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
*/
bool CMap2D::Update(const double dElapsedTime)
{
	UpdateTimedBlocks(dElapsedTime);
	return true;
}

/**
 @brief Constraint the player's position within a boundary
 @return true if the vec2Position was constained by this method, otherwise false
 */
bool CMap2D::Constraint(glm::vec2& vec2Position)
{
	if (vec2Position.x < vec2TileHalfSize.x)
	{
		vec2Position.x = vec2TileHalfSize.x;
		return true;
	}
	else if (vec2Position.x > pSettings->cSimpleIniA.GetFloatValue("Size", "iWindowWidth", 800.0f) - vec2TileHalfSize.x)
	{
		vec2Position.x = pSettings->cSimpleIniA.GetFloatValue("Size", "iWindowWidth", 800.0f) - vec2TileHalfSize.x;
		return true;
	}

	if (vec2Position.y > pSettings->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f) - vec2TileHalfSize.y)
	{
		vec2Position.y = pSettings->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f) - vec2TileHalfSize.y;
		return true;
	}
	else if (vec2Position.y < vec2TileHalfSize.y)
	{
		vec2Position.y = max(vec2Position.y, vec2TileHalfSize.y);
		return true;
	}

	return false;
}

/**
 @brief PreRender Set up the OpenGL display environment before rendering
 */
void CMap2D::PreRender(void)
{
	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render Render this instance
 */
void CMap2D::Render(void)
{
	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);

	for (int uiRow = 0; uiRow < pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24); uiRow++)
	{
		for (int uiCol = 0; uiCol < pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_XAXIS", 32); uiCol++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(uiCol * 25.0f + this->vec2TileHalfSize.x, 
													(pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24) - 1 - uiRow) * 25.0f + this->vec2TileHalfSize.y, 
													0.0f));

			if ((arrMapInfo[uiCurLevel][uiRow][uiCol].value >= 1) &&
				(arrMapInfo[uiCurLevel][uiRow][uiCol].value <= 10))
			{
				float t = ((float)(CFPSCounter::GetInstance()->GetNumFrames() / CFPSCounter::GetInstance()->GetFrameRate())) * 360.0f - 180.0f;
				model = glm::rotate(model, glm::radians(t), glm::vec3(0.f, 0.f, 1.f));
			}

			model = glm::scale(model, glm::vec3(25.0f, 25.0f, 1.0f));

			// Upload the model to OpenGL shader
			CShaderManager::GetInstance()->pActiveShader->setMat4("Model", model);
			// Upload the Projection to OpenGL shader
			CShaderManager::GetInstance()->pActiveShader->setMat4("Projection", projection);
			unsigned int colourLoc = glGetUniformLocation(CShaderManager::GetInstance()->pActiveShader->ID, "ColourTint");
			glUniform4fv(colourLoc, 1, glm::value_ptr(vec4ColourTint));

			// Render a tile
			RenderTile(uiRow, uiCol);
		}
	}
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CMap2D::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}

/**
 @brief Set Tile Size, and it automatically sets the half tile size
 @param x The tile size in the x-axis
 @param y The tile size in the y-axis
 */
void CMap2D::SetTileSize(const float x, const float y)
{
	vec2TileSize = glm::vec2(x, y);
	SetTileHalfSize(x * 0.5f, y * 0.5f);
}
/**
 @brief Set half Tile Size
 @param x The half tile size in the x-axis
 @param y The half tile size in the y-axis
 */
void CMap2D::SetTileHalfSize(const float x, const float y)
{
	vec2TileHalfSize = glm::vec2(x, y);
}

/**
 @brief Get Tile Size
 @return The tile size, vec2TileSize
 */
glm::vec2 CMap2D::GetTileSize(void)
{
	return vec2TileSize;
}

/**
 @brief Get half Tile Size
 @return The half tile size, vec2TileHalfSize
 */
glm::vec2 CMap2D::GetTileHalfSize(void)
{
	return vec2TileHalfSize;
}

/**
 @brief Get Size Tolerance
 @return The size tolerance, vec2SizeTolerance
 */
glm::vec2 CMap2D::GetSizeTolerance(void)
{
	return vec2SizeTolerance;
}

// Set the specifications of the map
void CMap2D::SetNumTiles(const CSettings::AXIS sAxis, const unsigned int uiValue)
{
	// Check if the value is valid
	if (uiValue <= 0)
	{
		cout << "CMap2D::SetNumTiles() : value must be more than 0" << endl;
		return;
	}

	if (sAxis == CSettings::x)
	{
		CSettings::GetInstance()->cSimpleIniA.SetValue("NumTiles", "NUM_TILES_XAXIS", (std::to_string(uiValue)).c_str());
		pSettings->UpdateSpecifications();
	}
	else if (sAxis == CSettings::y)
	{
		CSettings::GetInstance()->cSimpleIniA.SetValue("NumTiles", "NUM_TILES_YAXIS", (std::to_string(uiValue)).c_str());
		pSettings->UpdateSpecifications();
	}
	else if (sAxis == CSettings::z)
	{
		// Not used in here
	}
	else
	{
		cout << "Unknown axis" << endl;
	}
}

/**
 @brief Get indices of tiles at a position
 @param vec2Position A glm::vec2 variable which is the position to get the tile indices for
 @param iTileIndexEndX A int& variable to store the tile index in the x-axis
 @param iTileIndexEndY A int& variable to store the tile index in the y-axis
 @return true if the tile indices are return, false if the position is not within the tile map
 */
bool CMap2D::GetTileIndexAtPosition(const glm::vec2 vec2Position, int& iTileIndexEndX, int& iTileIndexEndY)
{
	if (GetTileIndexXAtPosition(vec2Position, iTileIndexEndX) == false)
		return false;

	if (GetTileIndexYAtPosition(vec2Position, iTileIndexEndY) == false)
		return false;

	return true;
}

/**
 @brief Get indices in X-axis of a tile at a position
 @param vec2Position A glm::vec2 variable which is the position to get the tile indices for
 @param iTileIndexEndX A int& variable to store the tile index in the x-axis
 @return true if the tile indices are return, false if the position is not within the tile map
 */
bool CMap2D::GetTileIndexXAtPosition(const glm::vec2 vec2Position, int& iTileIndexEndX)
{
	if ((vec2Position.x < 0) || (vec2Position.x > pSettings->cSimpleIniA.GetFloatValue("Size", "iWindowWidth", 800.0f)))
	{
		return false;
	}

	iTileIndexEndX = (int)(vec2Position.x / vec2TileSize.x);
	//float temp = (vec2Position.x / vec2TileSize.x);
	////float temp2 = temp - floor(temp / 0.5f) * 0.5f;
	//float temp2 = temp - floor(temp);
	//if (temp2 > 0.5f)
	//	iTileIndexEndX++;

	return true;
}

/**
 @brief Get indices in Y-axis of a tile at a position
 @param vec2Position A glm::vec2 variable which is the position to get the tile indices for
 @param iTileIndexEndY A int& variable to store the tile index in the y-axis
 @return true if the tile indices are return, false if the position is not within the tile map
 */
bool CMap2D::GetTileIndexYAtPosition(const glm::vec2 vec2Position, int& iTileIndexEndY)
{
	if ((vec2Position.y < 0) || (vec2Position.y > pSettings->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f)))
	{
		return false;
	}

	iTileIndexEndY = (int)(vec2Position.y / vec2TileSize.y);

	return true;
}


/**
 @brief Set the value at certain indices in the arrMapInfo
 @param iRow A const int variable containing the row index of the element to set to
 @param iCol A const int variable containing the column index of the element to set to
 @param iValue A const int variable containing the value to assign to this arrMapInfo
 */
void CMap2D::SetMapInfo(const unsigned int uiRow, const unsigned int uiCol, const int iValue, const bool bInvert)
{
	if (bInvert)
		arrMapInfo[uiCurLevel][pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24) - uiRow - 1][uiCol].value = iValue;
	else
		arrMapInfo[uiCurLevel][uiRow][uiCol].value = iValue;
}

/**
 @brief Get the value at certain indices in the arrMapInfo
 @param iRow A const int variable containing the row index of the element to get from
 @param iCol A const int variable containing the column index of the element to get from
 @param bInvert A const bool variable which indicates if the row information is inverted
 */
int CMap2D::GetMapInfo(const unsigned int uiRow, const unsigned int uiCol, const bool bInvert) const
{
	if (bInvert)
		return arrMapInfo[uiCurLevel][pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24) - uiRow - 1][uiCol].value;
	else
		return arrMapInfo[uiCurLevel][uiRow][uiCol].value;
}

/**
 @brief Load a map
 */ 
bool CMap2D::LoadMap(string filename, const unsigned int uiCurLevel)
{
	doc = rapidcsv::Document(FileSystem::getPath(filename).c_str());

	// Check if the sizes of CSV data matches the declared arrMapInfo sizes
	if ((pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_XAXIS", 32) != (unsigned int)doc.GetColumnCount()) ||
		(pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24) != (unsigned int)doc.GetRowCount()))
	{
		cout << "Sizes of CSV map does not match declared arrMapInfo sizes." << endl;
		return false;
	}

	// Read the rows and columns of CSV data into arrMapInfo
	for (int uiRow = 0; uiRow < pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24); uiRow++)
	{
		// Read a row from the CSV file
		std::vector<std::string> row = doc.GetRow<std::string>(uiRow);
		
		// Load a particular CSV value into the arrMapInfo
		for (int uiCol = 0; uiCol < pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_XAXIS", 32); ++uiCol)
		{
			try {
				arrMapInfo[uiCurLevel][uiRow][uiCol].value = (int)stoi(row[uiCol]);
			}
			catch (...) {
				arrMapInfo[uiCurLevel][uiRow][uiCol].value = 0;
			}
		}
	}

	return true;
}

/**
 @brief Save the tilemap to a text file
 @param filename A string variable containing the name of the text file to save the map to
 */
bool CMap2D::SaveMap(string filename, const unsigned int uiCurLevel)
{
	// Update the rapidcsv::Document from arrMapInfo
	for (int uiRow = 0; uiRow < pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24); uiRow++)
	{
		for (int uiCol = 0; uiCol < pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_XAXIS", 32); uiCol++)
		{
			doc.SetCell(uiCol, uiRow, arrMapInfo[uiCurLevel][uiRow][uiCol].value);
		}
		cout << endl;
	}

	// Save the rapidcsv::Document to a file
	doc.Save(FileSystem::getPath(filename).c_str());

	return true;
}

/**
@brief Find the indices of a certain value in arrMapInfo
@param iValue A const int variable containing the row index of the found element
@param iRow A const int variable containing the row index of the found element
@param iCol A const int variable containing the column index of the found element
@param bInvert A const bool variable which indicates if the row information is inverted
*/
bool CMap2D::FindValue(const int iValue, unsigned int& uirRow, unsigned int& uirCol, const bool bInvert)
{
	for (int uiRow = 0; uiRow < pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24); uiRow++)
	{
		for (int uiCol = 0; uiCol < pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_XAXIS", 32); uiCol++)
		{
			if (arrMapInfo[uiCurLevel][uiRow][uiCol].value == iValue)
			{
				if (bInvert)
					uirRow = pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24) - uiRow - 1;
				else
					uirRow = uiRow;
				uirCol = uiCol;
				return true;	// Return true immediately if the value has been found
			}
		}
	}
	return false;
}

/**
 @brief Set current level
 */
void CMap2D::SetCurrentLevel(unsigned int uiCurLevel)
{
	if (uiCurLevel < uiNumLevels)
	{
		this->uiCurLevel = uiCurLevel;
	}
}
/**
 @brief Get current level
 */
unsigned int CMap2D::GetCurrentLevel(void) const
{
	return uiCurLevel;
}

/**
 @brief Render a tile at a position based on its tile index
 @param iRow A const int variable containing the row index of the tile
 @param iCol A const int variable containing the column index of the tile
 */
void CMap2D::RenderTile(const unsigned int uiRow, const unsigned int uiCol)
{
	if (arrMapInfo[uiCurLevel][uiRow][uiCol].value != 0)
	{
		//if (arrMapInfo[uiCurLevel][uiRow][uiCol].value < 3)
		if (arrMapInfo[uiCurLevel][uiRow][uiCol].value == 105)
			cout << "wait" << endl;
		glBindTexture(GL_TEXTURE_2D, MapOfTextureIDs.at(arrMapInfo[uiCurLevel][uiRow][uiCol].value));
			//CS: Render the tile
			pQuadMesh->Render();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

/**
 @brief Print out the details about this class instance in the console
 */
void CMap2D::PrintSelf(void) const
{
	cout << endl << "AStar::PrintSelf()" << endl;

	for (unsigned uiLevel = 0; uiLevel < uiNumLevels; uiLevel++)
	{
		cout << "Level: " << uiLevel << endl;
		for (int uiRow = 0; uiRow < pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24); uiRow++)
		{
			for (int uiCol = 0; uiCol < pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_XAXIS", 32); uiCol++)
			{
				cout.fill('0');
				cout.width(3);
				cout << arrMapInfo[uiLevel][uiRow][uiCol].value;
				if (uiCol != pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_XAXIS", 32) - 1)
					cout << ", ";
				else
					cout << endl;
			}
		}
	}

	cout << "pqOpenList: " << pqOpenList.size() << endl;
	cout << "vCameFromList: " << vCameFromList.size() << endl;
	cout << "vClosedList: " << vClosedList.size() << endl;

	cout << "===== AStar::PrintSelf() =====" << endl;
}

/**
 @brief Check for collision with the tile maps vertically when an object moves from vec2StartPosition to vec2EndPosition
 @param vec2Position A glm::vec2 variable which is the position to check for
 @param vec2HalfSize A glm::vec2 variable which is the half size of the bounding box to check for
 @param vec2EndPosition A glm::vec2& variable which is the end position of the movement. It updates the vec2Position values
 @param fCollisionCoordY An float& variable which returns the y-coordinate of the collision point
 @return true if the vec2EndPosition was modified due to collision, otherwise false
 */
CSettings::RESULTS CMap2D::CheckVerticalCollision(glm::vec2 vec2StartPosition,
	glm::vec2 vec2HalfSize,
	glm::vec2& vec2EndPosition,
	float& fCollisionCoordY,
	const bool bApplyTolerance)
{
	glm::vec2 vec2StartLeft, vec2StartRight, vec2EndLeft, vec2EndRight;

	// 1. Get the bounding box of the area/grids which is occupied by object
	//    The start and end positions are in the from- and to- direction of the movement
	if (vec2EndPosition.y > vec2StartPosition.y)
	{
		// Jumping up since vec2EndPosition.y > vec2StartPosition.y
		vec2StartLeft = vec2StartPosition - vec2HalfSize + vec2SizeTolerance;
		vec2StartRight = glm::vec2(vec2StartPosition.x + vec2HalfSize.x - vec2SizeTolerance.x, vec2StartPosition.y - vec2HalfSize.y + vec2SizeTolerance.y);
		vec2EndLeft = glm::vec2(vec2EndPosition.x - vec2HalfSize.x + vec2SizeTolerance.x, vec2EndPosition.y + vec2HalfSize.y - vec2SizeTolerance.y);
		vec2EndRight = vec2EndPosition + vec2HalfSize - vec2SizeTolerance;
	}
	else
	{
		// Falling down since vec2EndPosition.y <= vec2StartPosition.y
		vec2StartLeft = glm::vec2(vec2StartPosition.x - vec2HalfSize.x + vec2SizeTolerance.x, vec2StartPosition.y + vec2HalfSize.y - vec2SizeTolerance.y);
		vec2StartRight = vec2StartPosition + vec2HalfSize - vec2SizeTolerance;
		vec2EndLeft = vec2EndPosition - vec2HalfSize + vec2SizeTolerance;
		vec2EndRight = glm::vec2(vec2EndPosition.x + vec2HalfSize.x - vec2SizeTolerance.x, vec2EndPosition.y - vec2HalfSize.y + vec2SizeTolerance.y);
	}

	// 2. Check if the bounding box is within the map
	// Get the indices for vec2StartLeft
	int iStartLeftX = 0;
	int iStartLeftY = 0;
	if (GetTileIndexAtPosition(vec2StartLeft, iStartLeftX, iStartLeftY) == false)
	{
		//cout << "A. This position is outside of the map: " << vec2StartLeft.x << ", " << vec2StartLeft.y << endl;
		return CSettings::RESULTS::INVALID;
	}

	// Get the indices for vec2BottomRight
	int iStartRightX = 0;
	int iStartRightY = 0;
	if (GetTileIndexAtPosition(vec2StartRight, iStartRightX, iStartRightY) == false)
	{
		//cout << "B. This position is outside of the map: " << vec2StartRight.x << ", " << vec2StartRight.y << endl;
		return CSettings::RESULTS::INVALID;
	}

	// Get the indices for vec2TopLeft
	int iEndLeftX = 0;
	int iEndLeftY = 0;
	if (GetTileIndexAtPosition(vec2EndLeft, iEndLeftX, iEndLeftY) == false)
	{
		//cout << "C. This position is outside of the map: " << vec2EndLeft.x << ", " << vec2EndLeft.y << endl;
		return CSettings::RESULTS::INVALID;
	}

	// Get the indices for vec2TopRight
	int iEndRightX = 0;
	int iEndRightY = 0;
	if (GetTileIndexAtPosition(vec2EndRight, iEndRightX, iEndRightY) == false)
	{
		//cout << "D. This position is outside of the map: " << vec2EndRight.x << ", " << vec2EndRight.y << endl;
		return CSettings::RESULTS::INVALID;
	}

	// 3. Check the direction of the movement and check for collision in that direction

	// Check the top-bottom for collision a.k.a. check for objects falling down
	if ((vec2EndPosition - vec2StartPosition).y < 0.0f)
	{
		// Check the bottom side for collisions
		for (int iRow = iStartLeftY; iRow >= iEndLeftY; iRow--)
		{
			for (int iCol = iEndLeftX; iCol <= iEndRightX; iCol++)
			{
				int iTileInfo = GetMapInfo(iRow, iCol, true);
				if ((iTileInfo >= 100) && (iTileInfo < 200))
				{
					//Calculate new vec2EndPosition
					if (iRow == iStartLeftY)
					{
 					//cout << "The starting index for collision check CANNOT have collision. We will skip this" << endl;
					continue;
					}

					//Calculate new vec2EndPosition
					vec2EndPosition.y = (iRow + 1) * vec2TileSize.y + vec2HalfSize.y;
					// Calculate the collision point's y-coordinate
					fCollisionCoordY = (iRow + 1) * vec2TileSize.y + vec2HalfSize.y;

					return CSettings::RESULTS::POSITIVE;
				}
			}
		}
	}
	else if ((vec2EndPosition - vec2StartPosition).y > 0.0f)
	{
		// Check the top side for collisions a.k.a. check for objects jumping up
		for (int iRow = iStartLeftY; iRow <= iEndLeftY; iRow++)
		{
			for (int iCol = iEndLeftX; iCol <= iEndRightX; iCol++)
			{
				int iTileInfo = GetMapInfo(iRow, iCol, true);
				if ((iTileInfo >= 100) && (iTileInfo < 200))
				{
					if (iRow == iStartLeftY)
					{
						//cout << "The starting index for collision check CANNOT have collision. We will skip this" << endl;
						continue;
					}

					//Calculate new vec2EndPosition
					vec2EndPosition.y = (iRow - 1) * vec2TileSize.y + vec2HalfSize.y;
					// Calculate the collision point's y-coordinate
					fCollisionCoordY = iRow * vec2TileSize.y - vec2HalfSize.y;

					return CSettings::RESULTS::POSITIVE;
				}
			}
		}
	}

	return CSettings::RESULTS::NEGATIVE;
}

/**
 @brief Check for collision with the tile maps horizontically when an object moves from vec2StartPosition to vec2EndPosition
 @param vec2Position A glm::vec2 variable which is the position to check for
 @param vec2HalfSize A glm::vec2 variable which is the half size of the bounding box to check for
 @param vec2EndPosition A glm::vec2& variable which is the end position of the movement. It updates the vec2Position values
 @param fCollisionCoordX An float& variable which returns the x-coordinate of the collision point
 @return true if the vec2EndPosition was modified due to collision, otherwise false
 */
CSettings::RESULTS CMap2D::CheckHorizontalCollision(glm::vec2 vec2StartPosition,
	glm::vec2 vec2HalfSize,
	glm::vec2& vec2EndPosition,
	float& fCollisionCoordX,
	const bool bApplyTolerance)
{
	glm::vec2 vec2StartTop, vec2StartBottom, vec2EndTop, vec2EndBottom;

	// 1. Get the bounding box of the area/grids which is occupied by object
	if (vec2EndPosition.x > vec2StartPosition.x)
	{
		// Moving to the right since vec2EndPosition.x > vec2StartPosition.x
		vec2StartTop = glm::vec2(vec2StartPosition.x - vec2HalfSize.x + vec2SizeTolerance.x, vec2StartPosition.y + vec2HalfSize.y - vec2SizeTolerance.y);
		vec2StartBottom = vec2StartPosition - vec2HalfSize + vec2SizeTolerance;
		vec2EndTop = vec2EndPosition + vec2HalfSize - vec2SizeTolerance;
		vec2EndBottom = glm::vec2(vec2EndPosition.x + vec2HalfSize.x - vec2SizeTolerance.x, vec2EndPosition.y - vec2HalfSize.y + vec2SizeTolerance.y);
	}
	else
	{
		// Moving to the left since vec2EndPosition.x <= vec2StartPosition.x
		vec2StartTop = vec2StartPosition + vec2HalfSize - vec2SizeTolerance;
		vec2StartBottom = glm::vec2(vec2StartPosition.x + vec2HalfSize.x - vec2SizeTolerance.x, vec2StartPosition.y - vec2HalfSize.y + vec2SizeTolerance.y);
		vec2EndTop = glm::vec2(vec2EndPosition.x - vec2HalfSize.x + vec2SizeTolerance.x, vec2EndPosition.y + vec2HalfSize.y - vec2SizeTolerance.y);
		vec2EndBottom = vec2EndPosition - vec2HalfSize + vec2SizeTolerance;
	}

	// 2. Check if the bounding box is within the map
	// Get the indices for vec2StartTop
	int iStartTopX = 0;
	int iStartTopY = 0;
	if (GetTileIndexAtPosition(vec2StartTop, iStartTopX, iStartTopY) == false)
	{
		//cout << "A. This position is outside of the map: " << vec2StartTop.x << ", " << vec2StartTop.y << endl;
		return CSettings::RESULTS::INVALID;
	}

	// Get the indices for vec2StartBottom
	int iStartBottomX = 0;
	int iStartBottomY = 0;
	if (GetTileIndexAtPosition(vec2StartBottom, iStartBottomX, iStartBottomY) == false)
	{
		//cout << "B. This position is outside of the map: " << vec2StartBottom.x << ", " << vec2StartBottom.y << endl;
		return CSettings::RESULTS::INVALID;
	}

	// Get the indices for vec2EndTop
	int iEndTopX = 0;
	int iEndTopY = 0;
	if (GetTileIndexAtPosition(vec2EndTop, iEndTopX, iEndTopY) == false)
	{
		//cout << "C. This position is outside of the map: " << vec2EndTop.x << ", " << vec2EndTop.y << endl;
		return CSettings::RESULTS::INVALID;
	}

	// Get the indices for vec2EndBottom
	int iEndBottomX = 0;
	int iEndBottomY = 0;
	if (GetTileIndexAtPosition(vec2EndBottom, iEndBottomX, iEndBottomY) == false)
	{
		//cout << "D. This position is outside of the map: " << vec2EndBottom.x << ", " << vec2EndBottom.y << endl;
		return CSettings::RESULTS::INVALID;
	}

	// 3. Check the direction of the movement and check for collision in that direction
	//cout << "collision iStartTopX: " << iStartTopX << ", " << iStartTopY << endl;
	//cout << "collision iStartBottomX: " << iStartBottomX << ", " << iStartBottomY << endl;
	//cout << "collision iEndTopX: " << iEndTopX << ", " << iEndTopY << endl;
	//cout << "collision iEndBottomX: " << iEndBottomX << ", " << iEndBottomY << endl;

	// Check the right-left for collision a.k.a. check for objects moving to the left
	if ((vec2EndPosition - vec2StartPosition).x < 0.0f)
	{
		// Check the right-left for collision a.k.a. check for objects moving to the left
		for (int iCol = iStartBottomX; iCol >= iEndBottomX; iCol--)
		{
			for (int iRow = iStartBottomY; iRow <= iStartTopY; iRow++)
			{
				int iTileInfo = GetMapInfo(iRow, iCol, true);
				if ((iTileInfo >= 100) && (iTileInfo < 200))
				{
					if (iCol == iStartBottomX)
					{
						//cout << "The starting index for collision check CANNOT have collision. We will skip this" << endl;
						continue;
					}

					//Calculate new vec2EndPosition
					vec2EndPosition.x = (iCol + 1) * vec2TileSize.x + vec2HalfSize.x;
					// Calculate the collision point's x-coordinate
					fCollisionCoordX = (iCol + 1) * vec2TileSize.x + vec2HalfSize.x;

					return CSettings::RESULTS::POSITIVE;
				}
			}
		}
	}
	else if ((vec2EndPosition - vec2StartPosition).x > 0.0f)
	{
		// Check the left-right for collision a.k.a. check for objects moving to the right
		for (int iCol = iStartBottomX; iCol <= iEndBottomX; iCol++)
		{
			for (int iRow = iStartBottomY; iRow <= iStartTopY; iRow++)
			{
				int iTileInfo = GetMapInfo(iRow, iCol, true);
				if ((iTileInfo >= 100) && (iTileInfo < 200))
				{
					if (iCol == iStartBottomX)
					{
						//cout << "The starting index for collision check CANNOT have collision. We will skip this" << endl;
						continue;
					}

					// Calculate new vec2EndPosition
					vec2EndPosition.x = (iCol - 1) * vec2TileSize.x + vec2HalfSize.x;
					// Calculate the collision point's x-coordinate
					fCollisionCoordX = iCol * vec2TileSize.x - vec2HalfSize.x;

					return CSettings::RESULTS::POSITIVE;
				}
			}
		}
	}

	return CSettings::RESULTS::NEGATIVE;
}

/**
 @brief Find a path
 */
std::vector<glm::vec2> CMap2D::PathFind(const glm::vec2& startPos,
	const glm::vec2& targetPos,
	HeuristicFunction heuristicFunc,
	const int weight)
{
	// Check if the startPos and targetPost are blocked
	if (isBlocked((unsigned int)startPos.y, (unsigned int)startPos.x) ||
		(isBlocked((unsigned int)targetPos.y, (unsigned int)targetPos.x)))
	{
		cout << "Invalid start or target position." << endl;
		// Return an empty path
		std::vector<glm::vec2> path;
		return path;
	}

	// Set up the variables and lists
	vec2StartPos = startPos;
	vec2TargetPos = targetPos;
	iWeight = weight;
	m_heuristic = std::bind(heuristicFunc, _1, _2, _3);

	// Reset AStar lists
	ResetAStarLists();

	// Add the start pos to 2 lists
	vCameFromList[ConvertTo1D(vec2StartPos)].parent = vec2StartPos;
	pqOpenList.push(Grid(vec2StartPos, 0));

	unsigned int fNew, gNew, hNew;
	glm::vec2 currentPos;

	// Start the path finding...
	while (!pqOpenList.empty())
	{
		// Get the node with the least f value
		// a queue algorithm is attached to pqOpenList to get the lowest f value
		currentPos = pqOpenList.top().pos;
		//cout << endl << "*** New position to check: " << currentPos.x << ", " << currentPos.y << endl;
		//cout << "*** targetPos: " << vec2TargetPos.x << ", " << vec2TargetPos.y << endl;

		// If the targetPos was reached, then quit this loop -> SOLUTION FOUND! -> come out of loop
		if (currentPos == vec2TargetPos)
		{
			//cout << "=== Found the targetPos: " << vec2TargetPos.x << ", " << vec2TargetPos.y << endl;
			while (pqOpenList.size() != 0)
				pqOpenList.pop();
			break;
		}

		pqOpenList.pop();
		vClosedList[ConvertTo1D(currentPos)] = true;

		// Check the neighbors of the current node
		for (unsigned int i = 0; i < iNumDirections; ++i)
		{
			const auto neighborPos = currentPos + vDirections[i];
			const auto neighborIndex = ConvertTo1D(neighborPos);

			//cout << "\t#" << i << ": Check this: " << neighborPos.x << ", " << neighborPos.y << ":\t";
			if (!isValid(neighborPos) || 
				isBlocked((unsigned int)neighborPos.y, (unsigned int)neighborPos.x) ||
				vClosedList[neighborIndex] == true)
			{
				//cout << "This position is not valid. Going to next neighbour." << endl;
				continue;
			}

			gNew = vCameFromList[ConvertTo1D(currentPos)].g + 1;
			hNew = m_heuristic(neighborPos, vec2TargetPos, iWeight);
			fNew = gNew + hNew;

			// check if its getting us nearer to the target position
			if (vCameFromList[neighborIndex].f == 0 || fNew < vCameFromList[neighborIndex].f)
			{
				//cout << "Adding to Open List: " << neighborPos.x << ", " << neighborPos.y;
				//cout << ". [ f : " << fNew << ", g : " << gNew << ", h : " << hNew << "]" << endl;
				pqOpenList.push(Grid(neighborPos, fNew));
				vCameFromList[neighborIndex] = { neighborPos, currentPos, fNew, gNew, hNew };
			}
			// otherwise, do not add
			else
			{
				//cout << "Not adding this" << endl;
			}
		}
		//system("pause");
	}

	return BuildPath();
}

/**
 @brief Build a path
 extracts the points in the path from m_cameFromList
 */
std::vector<glm::vec2> CMap2D::BuildPath() const
{
	std::vector<glm::vec2> path;
	auto currentPos = vec2TargetPos;
	auto currentIndex = ConvertTo1D(currentPos);

	while (!(vCameFromList[currentIndex].parent == currentPos))
	{
		path.push_back(currentPos);
		currentPos = vCameFromList[currentIndex].parent;
		currentIndex = ConvertTo1D(currentPos);
	}

	// If the path has only 1 entry, then it is the the target position
	if (path.size() == 1)
	{
		// if vec2StartPos is next to vec2TargetPos, then having 1 path point is OK
		if (iNumDirections == 4)
		{
			if (abs(vec2TargetPos.y - vec2StartPos.y) + abs(vec2TargetPos.x - vec2StartPos.x) > 1)
				path.clear();
		}
		else
		{
			if (abs(vec2TargetPos.y - vec2StartPos.y) + abs(vec2TargetPos.x - vec2StartPos.x) > 2)
				path.clear();
			else if (abs(vec2TargetPos.y - vec2StartPos.y) + abs(vec2TargetPos.x - vec2StartPos.x) > 1)
				path.clear();
		}
	}
	else
		std::reverse(path.begin(), path.end());

	return path;
}

/**
 @brief Toggle the checks for diagonal movements
 decide if the algorithm will consider the diagonals as part of its calculation for the path
 */
void CMap2D::SetDiagonalMovement(const bool bEnable)
{
	iNumDirections = (bEnable) ? 8 : 4;
}

/**
 @brief Check if a position is valid (if grid position is within map)
 */
bool CMap2D::isValid(const glm::vec2& pos) const
{
	return (pos.x >= 0) && (pos.x < pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_XAXIS", 32)) &&
		(pos.y >= 0) && (pos.y < pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24));
}

/**
 @brief Check if a grid is blocked (non-accessible e.g ground tile)
 */
bool CMap2D::isBlocked(const unsigned int uiRow, const unsigned int uiCol, const bool bInvert) const
{
	if (bInvert == true)
	{
		if ((arrMapInfo[uiCurLevel][pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24) - uiRow - 1][uiCol].value >= 100) &&
			(arrMapInfo[uiCurLevel][pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_YAXIS", 24) - uiRow - 1][uiCol].value < 200))
			return true;
		else
			return false;
	}
	else
	{
		if ((arrMapInfo[uiCurLevel][uiRow][uiCol].value >= 100) &&
			(arrMapInfo[uiCurLevel][uiRow][uiCol].value < 200))
			return true;
		else
			return false;
	}
}

/**
 @brief Returns a 1D index based on a 2D coordinate using row-major layout
 @param pos A const glm::vec2& variable containing the position to convert to 1D
 @return The int value containing the 1D value
 use returned value to access the elements in two of the lists which are m_closedList and m_cameFromList
 */
int CMap2D::ConvertTo1D(const glm::vec2& pos) const
{
	//return (pos.y * m_dimensions.x) + pos.x;
	return (int)((pos.y * pSettings->cSimpleIniA.GetIntValue("NumTiles", "NUM_TILES_XAXIS", 32)) + pos.x);
}

/**
 @brief Delete AStar lists & reset their sizes to 0
 @return true is successfully delete the AStar Lists, otherwise false.
 */
bool CMap2D::DeleteAStarLists(void)
{
	// Delete pqOpenList
	while (pqOpenList.size() != 0)
		pqOpenList.pop();
	// Delete vCameFromList
	vCameFromList.clear();
	// Delete vClosedList
	vClosedList.clear();

	return true;
}

/**
 @brief Reset AStar lists but doesnt reset size
 @return true is successfully reset the AStar Lists, otherwise false.
 */
bool CMap2D::ResetAStarLists(void)
{
	// Delete pqOpenList
	while (pqOpenList.size() != 0)
		pqOpenList.pop();
	// Reset vCameFromList
	for (int i = 0; i < (int)vCameFromList.size(); i++)
	{
		vCameFromList[i].pos = glm::vec2(0, 0);
		vCameFromList[i].parent = glm::vec2(0, 0);
		vCameFromList[i].f = 0;
		vCameFromList[i].g = 0;
		vCameFromList[i].h = 0;
	}
	// Reset vClosedList
	for (int i = 0; i < (int)vClosedList.size(); i++)
	{
		vClosedList[i] = false;
	}

	return true;
}

/**
 @brief manhattan calculation method for calculation of h
 @return An unsigned int of the value of h
 */
unsigned int heuristic::manhattan(const glm::vec2& v1, const glm::vec2& v2, int weight)
{
	glm::vec2 delta = v2 - v1;
	return static_cast<unsigned int>(weight * (delta.x + delta.y));
}

/**
 @brief euclidean calculation method for calculation of h
 @return An unsigned int of the value of h
 */
unsigned int heuristic::euclidean(const glm::vec2& v1, const glm::vec2& v2, int weight)
{
	glm::vec2 delta = v2 - v1;
	return static_cast<unsigned int>(weight * sqrt((delta.x * delta.x) + (delta.y * delta.y)));
}

void CMap2D::ScheduleBlockReset(int x, int y)
{
	// Check if block is already in the vector
	for (auto& block : blocksToReset) {
		if (block.x == x && block.y == y) {
			return; // if same block, dont add but return
		}
	}
	// If new block, add it
	blocksToReset.push_back({ x, y, resetDelay });
}

void CMap2D::UpdateTimedBlocks(float deltaTime)
{
	if (areDoorsUsed)
		return; // dont reset blocks anymore if player has used the doors

	for (size_t i = 0; i < blocksToReset.size(); ) {
		blocksToReset[i].timeLeft -= deltaTime;

		if (blocksToReset[i].timeLeft <= 0) {
			// Reset the block (104 → 103)
			SetMapInfo(blocksToReset[i].y, blocksToReset[i].x, 103);
			DeactivatePort(blocksToReset[i].x, blocksToReset[i].y);

			// Remove from vector (swap with last element + pop_back)
			blocksToReset[i] = blocksToReset.back();
			blocksToReset.pop_back();
		}
		else {
			i++;
		}
	}
}

void CMap2D::ActivatePort(int x, int y)
{
	// Ignore if port is already active
	for (auto& port : activePorts) {
		if (port.first == x && port.second == y) return;
	}

	activePorts.emplace_back(x, y);

	// When enough ports are activated
	if (activePorts.size() >= requiredActivePorts) {
		UnlockDoors(); // Changes sprites to unlocked
	}
}

void CMap2D::DeactivatePort(int x, int y)
{
	if (areDoorsUsed) return; // prevent door from re-locking after player has used them

	// Find and remove the port from activePorts
	auto it = std::remove_if(
		activePorts.begin(),
		activePorts.end(),
		[x, y](const std::pair<int, int>& port) {
			return port.first == x && port.second == y;
		}
	);

	if (it != activePorts.end()) {
		activePorts.erase(it, activePorts.end());
	}

	// When ports are deactivated
	if (activePorts.size() < requiredActivePorts && AreDoorsUnlocked()) {
		LockDoors(); // Reverts sprites to locked
	}
}

void CMap2D::DeactivateAllPorts()
{
	activePorts.clear();
}

void CMap2D::LockDoors()
{
	if (!areDoorsUnlocked) return; // Already unlocked
	SetMapInfo(entranceDoorPos.y, entranceDoorPos.x, lockedDoorTileID);
	SetMapInfo(exitDoorPos.y, exitDoorPos.x, lockedDoorTileID);
	areDoorsUnlocked = false;
}

void CMap2D::UnlockDoors()
{
	if (areDoorsUnlocked) return; // Already unlocked
	SetMapInfo(entranceDoorPos.y, entranceDoorPos.x, unlockedDoorTileID);
	SetMapInfo(exitDoorPos.y, exitDoorPos.x, unlockedDoorTileID);
	areDoorsUnlocked = true;
}

bool CMap2D::AreDoorsUnlocked()
{
	return areDoorsUnlocked;
}

void CMap2D::SetAreDoorsUsed(bool _areDoorsUsed)
{
	areDoorsUsed = _areDoorsUsed;
}

void CMap2D::SetDoorPositions(glm::ivec2 entrance, glm::ivec2 exit)
{
	entranceDoorPos = entrance;
	exitDoorPos = exit;
}

bool CMap2D::IsEntranceDoor(int x, int y) const
{
	return x == entranceDoorPos.x && y == entranceDoorPos.y;
}

glm::ivec2 CMap2D::GetExitDoorPos() const
{
	return exitDoorPos;
}