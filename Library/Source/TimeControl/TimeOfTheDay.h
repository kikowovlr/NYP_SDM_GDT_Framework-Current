/**
 CTimeOfTheDay
 @brief A class to manage the time of the day
 By: Toh Da Jun
 Date: Jan 2024
 */
#pragma once

 // Include SingletonTemplate
#include "..\DesignPatterns\SingletonTemplate.h"

class CTimeOfTheDay : public CSingletonTemplate<CTimeOfTheDay>
{
	friend class CSingletonTemplate<CTimeOfTheDay>;

public:
	// Defines several possible options for Parts of the Day
	enum class PARTSOFTHEDAY: unsigned int
	{
		MIDNIGHT = 0,
		SUNRISE,
		MORNING,
		NOON,
		AFTERNOON,
		EVENING,
		NIGHT,
		NUM_PARTSOFTHEDAY
	};
	double PartsOfTheDayTransitionTime[8] = { 0.0, 6.0, 8.0, 12.0, 15.0, 18.0, 20.0, 23.99 };

	// Constructor
	CTimeOfTheDay(void);

	// Destructor
	~CTimeOfTheDay(void);
	
	// Initialise this class instance
	void Init(void);

	// Update
	bool Update(const double dElapsedTime);

	// Get the current PartOfTheDay
	PARTSOFTHEDAY GetCurrentPartOfTheDay(void) const;

	// Get the current time
	double GetCurrenttime(void) const;

protected:
	PARTSOFTHEDAY currentPartOfTheDay;
	double dCurrentTime;
	const double dTimeScale = 1.0;
};
