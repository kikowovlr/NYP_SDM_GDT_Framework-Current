/**
 CProjectile2D
 By: Toh Da Jun
 Date: Sep 2020
 */
#pragma once

// Include CEntity2D
#include "Primitives/Entity2D.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

// Include the Map2D as we will use it to check the player's movements and actions
#include "Map2D.h"

class CProjectile2D : public CEntity2D
{
public:
	// Default Constructor
	CProjectile2D(void);
	// Destructor
	virtual ~CProjectile2D(void);

	// Init the projectile
	bool Init(	glm::vec2 vec2Position = glm::vec2(0.0f, 0.0f),
				glm::vec2 vec2Direction = glm::vec2(0.0f, 0.0f),
				const double dLifetime = 0.0,
				const float fSpeed = 0.0f);

	// Activate the projectile. true == active, false == inactive
	void SetStatus(const bool bStatus);
	void SetDirection(glm::vec2 vec2Direction);
	// Set the lifetime of the projectile
	void SetLifetime(const double dLifetime);
	// Set the speed of the projectile
	void SetSpeed(const float fSpeed);
	// Set the source of the projectile
	void SetSource(CEntity2D* pSource);

	// Get the direction of the projectile
	glm::vec2 GetDirection(void);
	// Get the lifetime of the projectile
	double GetLifetime(void) const;
	// Get the speed of the projectile
	float GetSpeed(void) const;
	// Get the source of the projectile
	CEntity2D* GetSource(void) const;

	// Update this projectile
	virtual bool Update(const double dElapsedTime);
	// PreRender
	virtual void PreRender(void);
	// Render this projectile
	virtual void Render(void);
	// PostRender
	virtual void PostRender(void);

	// PrintSelf
	virtual void PrintSelf(void);

	const glm::vec2 vec2HalfSize = glm::vec2(2.5f, 2.5f);

protected:
	// Remaining lifetime in seconds
	double dLifetime;
	// Speed of this projectile in unit / sec
	float fSpeed;
	// The source which fired this projectile
	CEntity2D* pSource;

	// Direction of the projectile
	glm::vec2 vec2Direction;
	// Colour tint of the projectile
	glm::vec4 vec4Colour;
	// Previous position of the projectile
	glm::vec2 vec2PreviousPosition;
	// Scale of the projectile
	glm::vec2 vec2Scale;
	// Bool flag to indicate if this projectile is visible
	bool bVisible;

	// Handler to the CMap2D instance
	CMap2D* pMap2D;
};
