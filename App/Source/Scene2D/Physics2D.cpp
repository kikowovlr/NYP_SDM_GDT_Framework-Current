/**
 CPhysics2D
 By: Toh Da Jun
 Date: Sep 2021
 */
#include "Physics2D.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CPhysics2D::CPhysics2D(void)
	: vec2InitialVelocity(glm::vec2(0.0f))
	, vec2FinalVelocity(glm::vec2(0.0f))
	, vec2Acceleration(glm::vec2(0.0f))
	, vec2PrevDisplacement(glm::vec2(0.0f))
	, vec2Displacement(glm::vec2(0.0f))
	, fTotalTime(0.0f)
	, fElapsedTime(0.0f)
	, sCurrentVerticalStatus(VERTICALSTATUS::IDLE)
	, bNewJump(false)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CPhysics2D::~CPhysics2D(void)
{
}

/**
@brief Init Initialise this instance
@return true if the initialisation is successful, else false
*/
bool CPhysics2D::Init(void)
{
	// Reset these variables
	vec2InitialVelocity = glm::vec2(0.0f);
	vec2FinalVelocity = glm::vec2(0.0f);
	vec2Acceleration = glm::vec2(0.0f);		// Acceleration does not need to be reset here.
	vec2PrevDisplacement = glm::vec2(0.0f);
	vec2Displacement = glm::vec2(0.0f);
	fTotalTime = 0.0f;
	fElapsedTime = 0.0f;
	return true;
}

// Set methods
/**
 @brief Set Initial Velocity
 @param vec2InitialVelocity A const glm::vec2 variable describing the initial velocity
 */
void CPhysics2D::SetInitialVelocity(const glm::vec2 vec2InitialVelocity)
{
	this->vec2InitialVelocity = vec2InitialVelocity;	// Initial velocity
}

/**
 @brief Set Final velocity
 @param vec2FinalVelocity A const glm::vec2 variable describing the final velocity
 */
void CPhysics2D::SetFinalVelocity(const glm::vec2 vec2FinalVelocity)
{
	this->vec2FinalVelocity = vec2FinalVelocity;		// Final velocity
}

/**
 @brief Set Acceleration
 @param vec2Acceleration A const glm::vec2 variable describing the acceleration
 */
void CPhysics2D::SetAcceleration(const glm::vec2 vec2Acceleration)
{
	this->vec2Acceleration = vec2Acceleration;		// Acceleration
}

/**
 @brief Set Displacement
 @param vec2Displacement A const glm::vec2 variable describing the displacement
 */
void CPhysics2D::SetDisplacement(const glm::vec2 vec2Displacement)
{
	this->vec2Displacement = vec2Displacement;		// Displacement
}

/**
 @brief Set Total Elapsed Time
 @param fTotalTime A const float variable describing the total time since this physics calculation started
 */
void CPhysics2D::SetTotalElapsedTime(const float fTotalTime)
{
	this->fTotalTime = fTotalTime;	// Total Elapsed Time
}

/**
 @brief Set Elapsed Time
 @param fElapsedTime A const float variable containing the elapsed time since the last frame
 */
void CPhysics2D::SetElapsedTime(const float fElapsedTime)
{
	this->fElapsedTime = fElapsedTime;	// Total Elapsed Time
}

/**
 @brief Set VerticalStatus
 @param sVerticalStatus A const VERTICALSTATUS variable setting the new status of this class instance
 @param bInit A const bool variable to determine if Init() will be called. Default is true
 */
void CPhysics2D::SetVerticalStatus(const VERTICALSTATUS sVerticalStatus, const bool bInit)
{
	// If there is a change in status, then reset to default values
	if (sCurrentVerticalStatus != sVerticalStatus)
	{
		// Reset to default values
		if (bInit)
			Init();

		// Store the new status
		sCurrentVerticalStatus = sVerticalStatus;
	}
}

/**
 @brief Set HorizontalStatus
 @param sHorizontalStatus A const HORIZONTALSTATUS variable setting the new status of this class instance
 @param bInit A const bool variable to determine if Init() will be called. Default is true
 */
void CPhysics2D::SetHorizontalStatus(const HORIZONTALSTATUS sHorizontalStatus, const bool bInit)
{
	// If there is a change in status, then reset to default values
	if (sCurrentHorizontalStatus != sHorizontalStatus)
	{
		// Reset to default values
		if (bInit)
			Init();

		// Store the new status
		sCurrentHorizontalStatus = sHorizontalStatus;
	}
}

/**
 @brief Set bNewJump
 */
void CPhysics2D::SetNewJump(const bool bNewJump)
{
	this->bNewJump = bNewJump;
}

// Get methods
/**
 @brief Get Initial velocity
 @return a glm::vec2 variable
*/
glm::vec2 CPhysics2D::GetInitialVelocity(void) const
{
	return vec2InitialVelocity;	// Initial velocity
}

/**
 @brief Get Final velocity
 @return a glm::vec2 variable
 */
glm::vec2 CPhysics2D::GetFinalVelocity(void) const
{
	return vec2FinalVelocity;		// Final velocity
}

/**
 @brief Get Acceleration
 @return a glm::vec2 variable
 */
glm::vec2 CPhysics2D::GetAcceleration(void) const
{
	return vec2Acceleration;		// Acceleration
}

/**
 @brief Get Displacement
 @return a glm::vec2 variable
 */
glm::vec2 CPhysics2D::GetDisplacement(void) const
{
	return vec2Displacement;		// Displacement
}

/**
 @brief Get Delta Displacement
 @return a glm::vec2 variable
 */
glm::vec2 CPhysics2D::GetDeltaDisplacement(void) const
{
	return vec2Displacement - vec2PrevDisplacement;		// Delta Displacement
}

/**
 @brief Get Time
 @return a float variable
 */
float CPhysics2D::GetTotalElapsedTime(void) const
{
	return fTotalTime;	// Total Elapsed Time
}

/**
 @brief Get Vertical Status
 @return a CPhysics2D::VERTICALSTATUS variable
 */
CPhysics2D::VERTICALSTATUS CPhysics2D::GetVerticalStatus(void) const
{
	return sCurrentVerticalStatus;
}

/**
 @brief Get Horizontal Status
 @return a CPhysics2D::VERTICALSTATUS variable
 */
CPhysics2D::HORIZONTALSTATUS CPhysics2D::GetHorizontalStatus(void) const
{
	return sCurrentHorizontalStatus;
}

/**
 @brief Get bNewJump
 */
bool CPhysics2D::GetNewJump(void) const
{
	return bNewJump;
}

/**
 @brief Update
 @param dElapsedTime A const double variable containing the elapsed time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
 */
bool CPhysics2D::Update(const double dElapsedTime)
{
	// If the player is in IDLE mode, 
	// then we don't calculate further
	if (sCurrentVerticalStatus == VERTICALSTATUS::IDLE)
		return false;

	// For a new jump, we skip the assigning of vec2FinalVelocity to vec2InitialVelocity
	if (bNewJump == true)
	{
		// And we set bNewJump to true
		bNewJump = false;
	}
	else
	{
		// Store the initial velocity to the final velocity
		vec2InitialVelocity = vec2FinalVelocity;
	}

	// Calculate the final velocity
	vec2FinalVelocity = vec2InitialVelocity + float(dElapsedTime) * vec2Gravity;

	// Calculate the displacement
	vec2Displacement = float(dElapsedTime) * vec2FinalVelocity;

	return true;
}

/**
 @brief Add elapsed time
 @param fElapsedTime A const float variable containing the elapsed time since the last frame
 */
void CPhysics2D::AddElapsedTime(const float fElapsedTime)
{
	this->fElapsedTime = fElapsedTime;
	fTotalTime += fElapsedTime;
}

/**
 @brief Calculate the distance between two vec2 varables
 @param source A const glm::vec2 variable containing the source position
 @param destination A const glm::vec2 variable containing the destination position
 @return a float variable
*/
float CPhysics2D::CalculateDistance(glm::vec2 source, glm::vec2 destination)
{
	return glm::length(destination - source);
}

/**
 @brief PrintSelf
 */
void CPhysics2D::PrintSelf(void)
{
	cout << "CPhysics2D::PrintSelf()" << endl;
	cout << "=======================" << endl;
	cout << "vec2InitialVelocity\t=\t[" << vec2InitialVelocity.x << ", " << vec2InitialVelocity.y << "]" << endl;
	cout << "vec2FinalVelocity\t=\t[" << vec2FinalVelocity.x << ", " << vec2FinalVelocity.y << "]" << endl;
	cout << "vec2Acceleration\t=\t[" << vec2Acceleration.x << ", " << vec2Acceleration.y << "]" << endl;
	cout << "vec2Displacement\t=\t[" << vec2Displacement.x << ", " << vec2Displacement.y << "]" << endl;
	cout << "fTotalTime\t=\t" << fTotalTime << endl;
	cout << "fElapsedTime\t=\t" << fElapsedTime << endl;
}
