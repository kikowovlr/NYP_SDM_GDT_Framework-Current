/**
 CProjectileManager2D
 By: Toh Da Jun
 Date: Sep 2020
 */
#include "ProjectileManager2D.h"

#include <iostream>
using namespace std;

/**
 @brief Default Constructor
 */
CProjectileManager2D::CProjectileManager2D(void)
	: model(glm::mat4(1.0f))
	, view(glm::mat4(1.0f))
	, projection(glm::mat4(1.0f))
	, uiTotalElements(10)
	, uiIndexFirst(0)
	, uiIndexLast(0)
{
}

/**
 @brief Destructor
 */
CProjectileManager2D::~CProjectileManager2D(void)
{
	// Remove all CProjectile2D*
	for (unsigned int i = 0; i < vProjectile.size(); i++)
	{
		delete vProjectile[i];
		vProjectile[i] = NULL;
	}
	vProjectile.clear();
}

/**
 @brief Set a shader to this class instance
 @param _name The name of the Shader instance in the CShaderManager
 */
void CProjectileManager2D::SetShader(const std::string& _name)
{
	this->sShaderName = _name;
}

/**
 @brief Initialise this class instance
 @return A bool variable to indicate this method successfully completed its tasks
 */
bool CProjectileManager2D::Init(void)
{
	// Clear the vector
	vProjectile.clear();

	// Reserve the size of the vector
	vProjectile.reserve(uiTotalElements);

	// Create the instances of CProjectile2D* and store them in the vector
	for (unsigned int i = 0; i < uiTotalElements; i++)
	{
		CProjectile2D* temp = new CProjectile2D();
		temp->SetShader(sShaderName);
		temp->Init();
		vProjectile.push_back(temp);
	}

	return true;
}

/**
 @brief Set model
 @param model A glm::mat4 variable containing the model for this class instance
 */
void CProjectileManager2D::SetModel(glm::mat4 model)
{
	this->model = model;
}

/**
 @brief Set view
 @param view A glm::mat4 variable containing the model for this class instance
 */
void CProjectileManager2D::SetView(glm::mat4 view)
{
	this->view = view;
}

/**
 @brief Set projection
 @param projection A glm::mat4 variable containing the model for this class instance
 */
void CProjectileManager2D::SetProjection(glm::mat4 projection)
{
	this->projection = projection;
}

/**
 @brief Activate a CProjectile2D* to this class instance
 @param vec2Position A const glm::vec2 variable containing the source position of the projectile
 @param vec2Direction A const glm::vec2 variable containing the direction of the projectile
 @param dLifeTime A const double variable containing the life time of this projectile in milliseconds
 @param fSpeed A const float variable containing the speed of this projectile
 @param pSource A CEntity2D* variable containing the source of this projectile
 @return A int variable
 */
int CProjectileManager2D::Activate(	glm::vec2 vec2Position, 
									glm::vec2 vec2Direction, 
									const double dLifetime,
									const float fSpeed,
									CEntity2D* pSource)
{
	// Since a projectile has been added, we activate the next element in the vector
	vProjectile[uiIndexLast]->Init(vec2Position, vec2Direction, dLifetime, fSpeed);
	//vProjectile[uiIndexLast]->SetPreviousPosition(vec2Position);
	//vProjectile[uiIndexLast]->InitCollider(	"Shader3D_Line", 
	//										glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
	//										glm::vec2(-0.05f, -0.05f, -0.05f), 
	//										glm::vec2(0.05f, 0.05f, 0.05f));
	vProjectile[uiIndexLast]->SetStatus(true);
	vProjectile[uiIndexLast]->SetSource(pSource);
	vProjectile[uiIndexLast]->SetShader(sShaderName);

	// Increase the uiIndexLast by 1 since a projectile is going to be added
	if (uiIndexLast + 1 == vProjectile.size())
		uiIndexLast = 0;
	else
		uiIndexLast++;

	return uiIndexLast;
}

/**
 @brief Deactivate a CProjectile2D* from this class instance
 @param iIndex A const unsigned int variable containing the index of the projectile to deactivate
 @return A bool variable to indicate this method successfully completed its tasks
 */
bool CProjectileManager2D::Deactivate(const unsigned int iIndex)
{
	if (iIndex<vProjectile.size())
	{
		// Set the projectile to inactive
		vProjectile[iIndex]->SetStatus(false);
		// Return true
		return true;
	}

	// Return false if not found
	return false;
}

/**
 @brief Update this instance
 @param dElapsedTime A const double variable containing the elapsed time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
 */
void CProjectileManager2D::Update(const double dElapsedTime)
{
	uiIndexFirst = uiIndexLast + 1;
	if (uiIndexFirst >= vProjectile.size())
	{
		uiIndexFirst = 0;

		// CollisionCheck all projectiles against CEntity2D
		// Check the oldest index until the end of 
		for (unsigned int i = uiIndexFirst; i <= uiIndexLast; i++)
		{
			// If this projectile is not active, then skip it
			if (vProjectile[i]->GetStatus() == false)
				continue;

			// Update a projectile
			vProjectile[i]->Update(dElapsedTime);
		}
	}
	else
	{
		// CollisionCheck all projectiles against CEntity2D
		// Check the oldest index until the end of 
		for (unsigned int i = uiIndexFirst; i < vProjectile.size(); i++)
		{
			// If this projectile is not active, then skip it
			if (vProjectile[i]->GetStatus() == false)
				continue;

			// Update a projectile
			vProjectile[i]->Update(dElapsedTime);
		}

		// CollisionCheck all projectiles against CEntity2D
		for (unsigned int i = 0; i <= uiIndexLast; i++)
		{
			// If this projectile is not active, then skip it
			if (vProjectile[i]->GetStatus() == false)
				continue;

			// Update a projectile
			vProjectile[i]->Update(dElapsedTime);
		}
	}
}

/**
 @brief PreRender this class instance
 */
void CProjectileManager2D::PreRender(void)
{
}

/**
 @brief Render this class instance
 */
void CProjectileManager2D::Render(void)
{
	// Render all entities
	for (unsigned int i = 0; i < vProjectile.size(); i++)
	{
		// If this projectile is not active, then skip it
		if (vProjectile[i]->GetStatus() == false)
			continue;

		//vProjectile[i]->SetView(view);
		//vProjectile[i]->SetProjection(projection);
		vProjectile[i]->PreRender();
		vProjectile[i]->Render();
		vProjectile[i]->PostRender();
	}
}

/**
 @brief PostRender this class instance
 */
void CProjectileManager2D::PostRender(void)
{
}

/**
 @brief PrintSelf
 */
void CProjectileManager2D::PrintSelf(void)
{
	cout << "===============================" << endl;
	cout << "CProjectileManager2D::PrintSelf()" << endl;
	cout << "-------------------------------" << endl;
	// Render all entities
	for (unsigned int i = 0; i < vProjectile.size(); i++)
	{
		cout << i << "\t: ";
		vProjectile[i]->PrintSelf();
	}
	cout << "===============================" << endl;
}
