/**
 CTimeOfTheDay
 @brief A class to manage the time of the day
 By: Toh Da Jun
 Date: Jan 2024
 */
#include "TimeOfTheDay.h"

/**
@brief Constructor
*/
CTimeOfTheDay::CTimeOfTheDay(void)
	: dCurrentTime(PartsOfTheDayTransitionTime[3])
	, currentPartOfTheDay(PARTSOFTHEDAY::NOON)
{
}

/**
@brief Destructor
*/ 
CTimeOfTheDay::~CTimeOfTheDay(void)
{
}

/**
@brief Initialise this class instance
*/
void CTimeOfTheDay::Init(void)
{

}

/**
@brief Update
*/ 
bool CTimeOfTheDay::Update(const double dElapsedTime)
{
	// update dCurrentTime
	dCurrentTime += dElapsedTime / dTimeScale;

	// If dCurrentTime is greater than 24:00 hours, then reset to 00:00 hours
	if (dCurrentTime >= 24.0)
		dCurrentTime = 0.0;

	for (int i = 0; i < (int)PARTSOFTHEDAY::NUM_PARTSOFTHEDAY; i++)
	{
		if ((dCurrentTime > PartsOfTheDayTransitionTime[i]) &&
			(dCurrentTime < PartsOfTheDayTransitionTime[i + 1]))
		{
			// if dCurrentTime is greater than the start time for a PartsOfTheDayTransitionTime value,
			// then we change the currentPartOfTheDay to this new PARTSOFTHEDAY
			currentPartOfTheDay = static_cast<PARTSOFTHEDAY>(i);
			break;
		}
	}

	return true;
}

/**
 @brief Get the current PartOfTheDay
 */
CTimeOfTheDay::PARTSOFTHEDAY CTimeOfTheDay::GetCurrentPartOfTheDay(void) const
{
	return currentPartOfTheDay;
}

// Get the current time
/**
 @brief Get the current PartOfTheDay
 */
double CTimeOfTheDay::GetCurrenttime(void) const
{
	return dCurrentTime;
}