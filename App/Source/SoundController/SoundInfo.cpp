/**
 CSoundInfo
 @brief A class which stores a sound object
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "SoundInfo.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
CSoundInfo::CSoundInfo(void)
	: iID(-1)
	, filename(NULL)
	, pSoundSource(NULL)
	, fVolume(1.0f)
	, bIsLooped(false)
	, eSoundType(_2D)
	, vec3dfSoundPos(vec3df(0.0, 0.0, 0.0))
{
}

/**
 @brief Destructor
 */
CSoundInfo::~CSoundInfo(void)
{
	// Clear the memory before destroying this class instance
	if (pSoundSource != NULL)
	{
		// When we drop the CSoundEngine, it will automatically drop the sound source
		// Hence we don't need to delete the pSoundSource here
		//pSoundSource->drop();
		pSoundSource = NULL;

		if (filename)
		{
			free((void*)filename);
			filename = NULL;
		}
	}
}

/**
 @brief Initialise this class instance
 @param ID A const int variable which will be the ID of the iSoundSource in the map
 @param pSoundSource A ISoundSource* variable which contains the iSoundSource
 @param bIsLooped A const bool variable which indicates if this iSoundSource will have loop playback.
 @param eSoundType A SOUNDTYPE enum variable which states the type of sound
 @param vec3dfSoundPos A vec3df variable which contains the 3D position of the sound
 @return A bool value. true is this class instance was initialised, else false
 */
bool CSoundInfo::Init(const int iID,
	ISoundSource* pSoundSource,
	const bool bIsLooped,
	SOUNDTYPE eSoundType,
	vec3df vec3dfSoundPos)
{
	this->iID = iID;
	this->pSoundSource = pSoundSource;
	this->bIsLooped = bIsLooped;
	this->eSoundType = eSoundType;
	this->vec3dfSoundPos = vec3dfSoundPos;

	return true;
}

/**
 @brief Set filename
 @return An int variable containing the ID
 */
int CSoundInfo::GetID(void) const
{
	return iID;
}

/**
 @brief Set filename
 @param filename A const char* containing the filename
 */
void CSoundInfo::SetFilename(const char* filename)
{
	this->filename = strdup(filename);
}

/**
 @brief Get filename
 @return A const char* variable containing the filename
 */
const char* CSoundInfo::GetFilename(void)
{
	return filename;
}

/**
 @brief Get an sound from this class
 @return The iSoundSource stored in this class instance
 */
ISoundSource* CSoundInfo::GetSound(void) const
{
	return pSoundSource;
}

/**
 @brief Get loop status
 @return bIsLooped value
 */
bool CSoundInfo::GetLoopStatus(void) const
{
	return bIsLooped;
}

/**
 @brief Get sound type
 @return CSoundInfo::SOUNDTYPE value
 */
CSoundInfo::SOUNDTYPE CSoundInfo::GetSoundType(void) const
{
	return eSoundType;
}

/**
 @brief Set the volume
 */
void CSoundInfo::SetVolume(const float fVolume)
{
	this->fVolume = fVolume;
}

/**
 @brief Get the volume
 @return the volume
 */
float CSoundInfo::GetVolume(void)
{
	return fVolume;
}

// For 3D sounds only
/**
 @brief Set Listener position
 @param x A const float variable containing the x-component of a position
 @param y A const float variable containing the y-component of a position
 @param z A const float variable containing the z-component of a position
 */
void CSoundInfo::SetPosition(const float x, const float y, const float z)
{
	vec3dfSoundPos.set(x, y, z);
}

/**
 @brief Get position of the 3D sound
 @return The position of the 3D sound
 */
vec3df CSoundInfo::GetPosition(void) const
{
	return vec3dfSoundPos;
}