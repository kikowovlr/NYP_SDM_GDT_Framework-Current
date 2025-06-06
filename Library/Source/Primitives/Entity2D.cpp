/**
 CEntity2D
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "Entity2D.h"
#include "MeshBuilder.h"

 // Include Shader Manager
#include "..\RenderControl\ShaderManager.h"

// Include ImageLoader
#include "..\System\ImageLoader.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CEntity2D::CEntity2D(void)
	: VAO(0)
	, VBO(0)
	, EBO(0)
	, iTextureID(0)
	, pSettings(NULL)
	, p2DMesh(NULL)
	, vec4ColourTint(glm::vec4(1.0, 1.0, 1.0, 1.0))
	, vec2Position(glm::vec2(0.0f))
{
	// Initialise vec2Position
	vec2Position = glm::vec2(0.0f);
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CEntity2D::~CEntity2D(void)
{
	// optional: de-allocate all resources once they've outlived their purpose:
	//glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);
	//CS: Delete the mesh
	if (p2DMesh)
	{
		delete p2DMesh;
		p2DMesh = NULL;
	}
}

/**
  @brief Initialise this instance
  */
bool CEntity2D::Init(void)
{
	// Get the handler to the CSettings instance
	pSettings = CSettings::GetInstance();

	// Set up the projection matrix
	projection = glm::ortho(0.0f,
							static_cast<float>(CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowWidth", 800.0f)),
							0.0f,
							static_cast<float>(CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f)),
							-1.0f, 1.0f);

	//glGenVertexArrays(1, &VAO);
	//glBindVertexArray(VAO);

	// Generate the p2DMesh for a default entity2D
	p2DMesh = CMeshBuilder::GenerateQuad(glm::vec4(1, 1, 1, 1), 1.0f, 1.0f);

	// Load a texture
	iTextureID = CImageLoader::GetInstance()->LoadTextureGetID("Image/Scene2D_DefaultTile.tga", true);
	if (iTextureID == 0)
	{
		cout << "Unable to load Image/Scene2D_DefaultTile.tga" << endl;
		return false;
	}

	// Init vec4ColourTint to white
	vec4ColourTint = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Set this to be enabled initially
	bStatus = true;

	return true;
}

/**
 @brief Set projection
 @param projection A const glm::mat4 variable containing the model for this class instance
 */
void CEntity2D::SetProjection(const glm::mat4 projection)
{
	this->projection = projection;
}

/**
 @brief Get projection
 @return A const glm::mat4 variable containing the model for this class instance
 */
glm::mat4 CEntity2D::GetProjection() const
{
	return projection;
}

/**
 @brief Set the status of this class instance
 @param bStatus A bool variable containing the new status
 */
void CEntity2D::SetStatus(const bool bStatus)
{
	this->bStatus = bStatus;
}

/**
 @brief Set the status of this class instance
 @return A bool variable containing the status
 */
const bool CEntity2D::GetStatus(void) const
{
	return bStatus;
}

/**
 @brief Set the name of the shader to be used in this class instance
 @param _name The name of the Shader instance in the CShaderManager
 */
 void CEntity2D::SetShader(const std::string& _name)
 {
	 this->sShaderName = _name;
 }

/**
 @brief Update this instance
 @param dElapsedTime A const double variable containing the elapsed time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
 */
bool CEntity2D::Update(const double dElapsedTime)
{
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(vec2Position, 0.0f));

	//model = glm::translate(model, glm::vec3(0.5f * pSettings->TILE_WIDTH, 0.5f * pSettings->TILE_HEIGHT, 0.0f));
	//model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
	//model = glm::translate(model, glm::vec3(-0.5f * pSettings->TILE_WIDTH, -0.5f * pSettings->TILE_HEIGHT, 0.0f));

	model = glm::scale(model, glm::vec3(static_cast<float>(CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowWidth", 800.0f)),
		static_cast<float>(CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f)),
		1.0f));

	return true;
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CEntity2D::PreRender(void)
{
	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);

	// Activate blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate the shader
	CShaderManager::GetInstance()->Use("Shader2D");
}

/**
 @brief Render this instance
 */
void CEntity2D::Render(void)
{
	// Upload the model to OpenGL shader
	CShaderManager::GetInstance()->pActiveShader->setMat4("Model", model);
	// Upload the Projection to OpenGL shader
	CShaderManager::GetInstance()->pActiveShader->setMat4("Projection", projection);
	unsigned int colourLoc = glGetUniformLocation(CShaderManager::GetInstance()->pActiveShader->ID, "ColourTint");
	glUniform4fv(colourLoc, 1, glm::value_ptr(vec4ColourTint));

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	// Get the texture to be rendered
	glBindTexture(GL_TEXTURE_2D, iTextureID);
		//CS: Render the animated sprite
//		glBindVertexArray(VAO);
			p2DMesh->Render();
//		glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CEntity2D::PostRender(void)
{
	// Disable blending
	glDisable(GL_BLEND);
}
