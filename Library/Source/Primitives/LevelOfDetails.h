/**
 CLevelOfDetails
 By: Toh Da Jun
 Date: Jan 2022
 */

#pragma once

// Include GLEW
#ifndef GLEW_STATIC
#include <GL/glew.h>
#define GLEW_STATIC
#endif

// Include CMesh
#include "../Primitives/Mesh.h"

class CLevelOfDetails
{
public:
	enum DETAIL_LEVEL
	{
		HIGH_DETAILS,
		MID_DETAILS,
		LOW_DETAILS,
		NUM_DETAIL_LEVEL
	};

	// Array containing the distances to switch LOD
	float arrLODDistance[3];

	// Constructor
	CLevelOfDetails(void);
	// Destructor
	virtual ~CLevelOfDetails(void);

	// Set the distances for different LODs
	virtual void SetLODDistances(const float fDistance_HighDetails, const float fDistance_MidDetails, const float fDistance_LowDetails);

	// Get the status of this LOD
	virtual bool GetLODStatus(void) const;
	// Set the status of this LOD
	virtual void SetLODStatus(const bool bLODStatus);

	// Set the current level of details
	virtual bool SetDetailLevel(const DETAIL_LEVEL eDetailLevel = HIGH_DETAILS);
	// Get the current level of details
	virtual DETAIL_LEVEL GetDetailLevel(void) const;

	// Update this class instance
	virtual void Update(const float fDistance);

protected:
	// Boolean flag to indicate if this LOD is active
	bool bLODStatus;
	// Variable storing the current level of details
	DETAIL_LEVEL eDetailLevel;

	//CS: The array of meshes that is used to draw objects
	CMesh** p3DMeshArray;

	// The texture ID in OpenGL
	GLuint arriTextureID[3];
};
