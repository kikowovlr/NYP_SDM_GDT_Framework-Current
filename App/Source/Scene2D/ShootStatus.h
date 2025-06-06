/**
 CShootStatus
 @brief A class which is used to track if a Player or NPC can shoot projectiles
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

class CShootStatus
{
public:
	// Reset
	void Reset(void)
	{
		fCountdownTimer = fDefaultCountdownTimer;
		bCanShoot = true;
	}

	// Update
	bool Update(const double dElapsedTime)
	{
		if (bCanShoot)
			return false;

		fCountdownTimer -= (float)dElapsedTime;
		if (fCountdownTimer <= 0.0f)
		{
			Reset();
		}

		return true;
	}

	// Check if player can shoot again
	bool IsAbleToShoot(void)
	{
		return bCanShoot;
	}

	// Indicate that the player cannot shoot
	void SetToCannotShoot(void)
	{
		bCanShoot = false;
	}

protected:
	// Countdown timer to prevent player from shooting again
	float fCountdownTimer;
	// Countdown timer's default value
	const float fDefaultCountdownTimer = 0.5f;
	// Bool flag to indicate if the player can shoot again
	bool bCanShoot;

};

