#include "LevelOfDetails.h"

/**
 @brief Constructor
*/
CLevelOfDetails::CLevelOfDetails(void)
	: bLODStatus(false)
	, eDetailLevel(HIGH_DETAILS)
	, p3DMeshArray(NULL)
{
	arrLODDistance[HIGH_DETAILS]	= 0.0f;
	arrLODDistance[MID_DETAILS]		= 15.0f;
	arrLODDistance[LOW_DETAILS]		= 30.0f;

	p3DMeshArray = new CMesh*[DETAIL_LEVEL::NUM_DETAIL_LEVEL];
	for (int i = 0; i < DETAIL_LEVEL::NUM_DETAIL_LEVEL; i++)
	{
		// We won't create an instance here, as it will be created in LoadOBJ::LoadOBJ2()
		p3DMeshArray[i] = NULL;
	}
}

/** 
@brief Destructor
*/
CLevelOfDetails::~CLevelOfDetails(void)
{
	if (p3DMeshArray != NULL)
	{
		// Dynamically deallocate the 3D array used to store the p3DMeshArray
		for (int i = 0; i < DETAIL_LEVEL::NUM_DETAIL_LEVEL; i++)
		{
			delete p3DMeshArray[i];
		}
		delete[] p3DMeshArray;
		p3DMeshArray = NULL;
	}
}

/**
@brief Set the distances for different LODs
@param fDistance_HighDetails A const float variable storing the distance from the camera to this grid
@param fDistance_MidDetails A const float variable storing the distance from the camera to this grid
@param fDistance_LowDetails A const float variable storing the distance from the camera to this grid
*/
void CLevelOfDetails::SetLODDistances(	const float fDistance_HighDetails,
										const float fDistance_MidDetails, 
										const float fDistance_LowDetails)
{
	arrLODDistance[HIGH_DETAILS] = fDistance_HighDetails;
	arrLODDistance[MID_DETAILS] = fDistance_MidDetails;
	arrLODDistance[LOW_DETAILS] = fDistance_LowDetails;
}


/** 
@brief Get the LOD Status
*/
bool CLevelOfDetails::GetLODStatus(void) const
{
	return bLODStatus;
}

/** 
@brief Set the status of the LOD
*/
void CLevelOfDetails::SetLODStatus(const bool bLODStatus)
{
	this->bLODStatus = bLODStatus;

	// If we disable LOD, then let it always return HIGH_DETAILS when you call GetDetailLevel()
	if (this->bLODStatus == false)
	{
		eDetailLevel = HIGH_DETAILS;
	}
}

/** 
@brief Get the current detail level
*/
CLevelOfDetails::DETAIL_LEVEL CLevelOfDetails::GetDetailLevel(void) const
{
	return eDetailLevel;
}

/**
@brief Update this class instance
@param fDistance A const float variable storing the distance from the camera to this grid
*/
void CLevelOfDetails::Update(const float fDistance)
{
	if (fDistance > arrLODDistance[CLevelOfDetails::LOW_DETAILS])
		eDetailLevel = CLevelOfDetails::LOW_DETAILS;
	else if ((fDistance > arrLODDistance[CLevelOfDetails::MID_DETAILS]) && (fDistance <= arrLODDistance[CLevelOfDetails::LOW_DETAILS]))
		eDetailLevel = CLevelOfDetails::MID_DETAILS;
	else
		eDetailLevel = CLevelOfDetails::HIGH_DETAILS;
}

/** 
@brief Set the current detail level
*/
bool CLevelOfDetails::SetDetailLevel(const DETAIL_LEVEL eDetailLevel)
{
	if ((eDetailLevel >= HIGH_DETAILS) && (eDetailLevel < NUM_DETAIL_LEVEL))
	{
		this->eDetailLevel = eDetailLevel;
		return true;
	}
	return false;
}
