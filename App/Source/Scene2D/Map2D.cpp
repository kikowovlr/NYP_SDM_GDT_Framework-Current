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
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_GroundTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_GroundTile.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(100, iTextureID));
	}
	// Load the tree texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_TreeTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_TreeTile.tga" << endl;
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
	// Load the spike texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_Spikes.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_Spikes.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(20, iTextureID));
	}
	// Load the Spa texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_Spa.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_Spa.tga" << endl;
		return false;
	}
	else
	{
		// Store the texture ID into MapOfTextureIDs
		MapOfTextureIDs.insert(pair<int, int>(21, iTextureID));
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
	// Load metal tile texture
	//iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_MetalTile.tga", true);
	//if (iTextureID == 0)
	//{
	//	cout << "Unable to load Image/Scene2D_Exit.tga" << endl;
	//	return false;
	//}
	//else
	//{
	//	// Store the texture ID into MapOfTextureIDs
	//	MapOfTextureIDs.insert(pair<int, int>(101, iTextureID));
	//}

	return true;
}

/**
 @brief Update Update this instance
 @param dElapsedTime A const double variable containing the elapsed time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
*/
bool CMap2D::Update(const double dElapsedTime)
{
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
 					cout << "The starting index for collision check CANNOT have collision. We will skip this" << endl;
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
						cout << "The starting index for collision check CANNOT have collision. We will skip this" << endl;
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
						cout << "The starting index for collision check CANNOT have collision. We will skip this" << endl;
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
						cout << "The starting index for collision check CANNOT have collision. We will skip this" << endl;
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