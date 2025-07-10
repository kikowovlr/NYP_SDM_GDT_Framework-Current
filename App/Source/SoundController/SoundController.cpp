/**
 CSoundController
 @brief A class which manages the sound objects
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "SoundController.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
CSoundController::CSoundController(void)
	: pSoundEngine(NULL)
	, currentISound(NULL)
	, pSoundInfo(NULL)
	, vec3dfListenerPos(vec3df(0, 0, 0))
	, vec3dfListenerDir(vec3df(0, 0, 1))
{
}

/**
 @brief Destructor
 */
CSoundController::~CSoundController(void)
{
	// Deinitialise the instance
	DeInit();

	// Clear the sound engine
	if (pSoundEngine)
	{
		pSoundEngine->drop();
		pSoundEngine = NULL;
	}
}

/**
 @brief Initialise this class instance
 @return A bool value. true is this class instance was initialised, else false
 */
bool CSoundController::Init(void)
{
	if (pSoundEngine)
		return true;

	// Initialise the sound engine with default parameters
	pSoundEngine = createIrrKlangDevice(/*ESOD_WIN_MM, ESEO_MULTI_THREADED*/);
	if (pSoundEngine == NULL)
	{
		cout << "Unable to initialise the IrrKlang sound engine" << endl;
		return false;
	}
	return true;
}

/**
 @brief DeInitialise this instance without deleting the sound engine
 */
void CSoundController::DeInit(void)
{
	// Iterate through the soundMap
	for (std::map<int, CSoundInfo*>::iterator it = soundMap.begin(); it != soundMap.end(); ++it)
	{
		// If the value/second was not deleted elsewhere, then delete it here
		if (it->second != NULL)
		{
			delete it->second;
			it->second = NULL;
		}
	}

	// Remove all elements in soundMap
	soundMap.clear();

	// Remove all sound sources from the sound engine
	pSoundEngine->removeAllSoundSources();
}

/**
 @brief Add a sound
 @param filename A string variable storing the name of the file to read from
 @param ID A const int variable which will be the ID of the iSoundSource in the map
 @param bPreload A const bool variable which indicates if this iSoundSource will be pre-loaded into memory now.
 @param bIsLooped A const bool variable which indicates if this iSoundSource will have loop playback.
 @param eSoundType A SOUNDTYPE enum variable which states the type of sound
 @param vec3dfSoundPos A vec3df variable which contains the 3D position of the sound
 @return A bool value. True if the sound was loaded, else false.
 */
bool CSoundController::AddSound(	string filename,
									const int ID,
									const bool bPreload,
									const bool bIsLooped,
									CSoundInfo::SOUNDTYPE eSoundType,
									vec3df vec3dfSoundPos)
{
	if (filename == "")
	{
		cout << "The sound file path and name has error." << endl;
		return false;
	}

	// Load the sound from the file
	ISoundSource* pSoundSource = pSoundEngine->addSoundSourceFromFile(filename.c_str(),
																	E_STREAM_MODE::ESM_NO_STREAMING, 
																	bPreload);


	// Trivial Rejection : Invalid pointer provided
	if (pSoundSource == nullptr)
	{
		cout << "Unable to load sound " << filename.c_str() << endl;
		return false;
	}

	// Force the sound source not to have any streaming
	pSoundSource->setForcedStreamingThreshold(-1);

	// Clean up first if there is an existing Entity with the same name
	RemoveSound(ID);

	// Add the entity now
	CSoundInfo* pSoundInfo = new CSoundInfo();
	if (eSoundType == CSoundInfo::SOUNDTYPE::_2D)
		pSoundInfo->Init(ID, pSoundSource, bIsLooped);
	else
		pSoundInfo->Init(ID, pSoundSource, bIsLooped, eSoundType, vec3dfSoundPos);

	// Set to soundMap
	soundMap[ID] = pSoundInfo;

	// Store the filename in CSoundInfo
	pSoundInfo->SetFilename(filename.c_str());

	return true;
}

/**
 @brief Play a sound by its ID
 @param ID A const int variable which will be the ID of the iSoundSource in the map
 */
void CSoundController::PlaySoundByID(const int ID)
{
	pSoundInfo = GetSound(ID);
	if (!pSoundInfo)
	{
		cout << "Sound #" << ID << " is not playable." << endl;
		return;
	}
	else if (pSoundEngine->isCurrentlyPlaying(pSoundInfo->GetSound()))
	{
		cout << "Sound #" << ID << " is currently being played." << endl;
		return;
	}

	if (pSoundInfo->GetSoundType() == CSoundInfo::SOUNDTYPE::_2D)
	{
		currentISound = pSoundEngine->play2D(	pSoundInfo->GetSound(), 
												pSoundInfo->GetLoopStatus(), false, false, true);
		//ISoundEffectControl* fx = 0;
		//if (currentISound)
		//	fx = currentISound->getSoundEffectControl();
		//fx->enableWavesReverbSoundEffect();
	}
	else if (pSoundInfo->GetSoundType() == CSoundInfo::SOUNDTYPE::_3D)
	{
		pSoundEngine->setListenerPosition(vec3dfListenerPos, vec3dfListenerDir);
		currentISound = pSoundEngine->play3D(	pSoundInfo->GetSound(),
												pSoundInfo->GetPosition(), 
												pSoundInfo->GetLoopStatus(), true);
	}
}

/**
 @brief Increase Master volume
 @return true if successfully increased volume, else false
 */
bool CSoundController::MasterVolumeIncrease(void)
{
	// Get the current volume
	float fCurrentVolume = pSoundEngine->getSoundVolume() + 0.1f;
	// Check if the maximum volume has been reached
	if (fCurrentVolume > 1.0f)
		fCurrentVolume = 1.0f;

	// Update the Mastervolume
	pSoundEngine->setSoundVolume(fCurrentVolume);
	cout << "MasterVolumeIncrease: fCurrentVolume = " << fCurrentVolume << endl;

	return true;
}

/**
 @brief Decrease Master volume
 @return true if successfully decreased volume, else false
 */
bool CSoundController::MasterVolumeDecrease(void)
{
	// Get the current volume
	float fCurrentVolume = pSoundEngine->getSoundVolume() - 0.1f;
	// Check if the minimum volume has been reached
	if (fCurrentVolume < 0.0f)
		fCurrentVolume = 0.0f;

	// Update the Mastervolume
	pSoundEngine->setSoundVolume(fCurrentVolume);
	cout << "MasterVolumeDecrease: fCurrentVolume = " << fCurrentVolume << endl;

	return true;
}


/**
 @brief Increase volume of a ISoundSource
 @param ID A const int variable which contains the ID of the iSoundSource in the map
 @return true if successfully decreased volume, else false
 */
bool CSoundController::VolumeIncrease(const int ID)
{
	// Get the pSoundInfo
	pSoundInfo = GetSound(ID);
	if (pSoundInfo == nullptr)
		return false;

	// Get the volume
	float fVolume = pSoundInfo->GetVolume();

	// Increase the volume
	fVolume += 0.1f;

	// Check if the maximum volume has been reached
	if (fVolume >= 1.0f)
	{
		fVolume = 1.0f;
	}

	// Set the volume
	pSoundInfo->SetVolume(fVolume);


	//// Get the ISoundSource
	//ISoundSource* pISoundSource = GetSound(ID)->GetSound();
	//if (pISoundSource == nullptr)
	//{
	//	return false;
	//}

	//// Get the current volume
	//float fCurrentVolume = pISoundSource->getDefaultVolume();

	//// Check if the maximum volume has been reached
	//if (fCurrentVolume >= 1.0f)
	//{
	//	pISoundSource->setDefaultVolume(1.0f);
	//	return false;
	//}

	//// Increase the volume by 10%
	//pISoundSource->setDefaultVolume(fCurrentVolume + 0.1f);

	return true;
}

/**
 @brief Decrease volume of a ISoundSource
 @param ID A const int variable which contains the ID of the iSoundSource in the map
 @return true if successfully decreased volume, else false
 */
bool CSoundController::VolumeDecrease(const int ID)
{
	// Get the pSoundInfo
	pSoundInfo = GetSound(ID);
	if (pSoundInfo == nullptr)
		return false;

	// Get the volume
	float fVolume = pSoundInfo->GetVolume();

	// Increase the volume
	fVolume -= 0.1f;

	// Check if the maximum volume has been reached
	if (fVolume < 0.0f)
	{
		fVolume = 0.0f;
	}

	// Set the volume
	pSoundInfo->SetVolume(fVolume);

	//// Get the ISoundSource
	//ISoundSource* pISoundSource = GetSound(ID)->GetSound();
	//if (pISoundSource == nullptr)
	//{
	//	return false;
	//}

	//// Get the current volume
	//float fCurrentVolume = pISoundSource->getDefaultVolume();

	//// Check if the minimum volume has been reached
	//if (fCurrentVolume <= 0.0f)
	//{
	//	pISoundSource->setDefaultVolume(0.0f);
	//	return false;
	//}

	//// Decrease the volume by 10%
	//pISoundSource->setDefaultVolume(fCurrentVolume - 0.1f);

	return true;
}

// For 3D sounds only
/**
 @brief Set Listener position
 @param x A const float variable containing the x-component of a position
 @param y A const float variable containing the y-component of a position
 @param z A const float variable containing the z-component of a position
 */
void CSoundController::SetListenerPosition(const float x, const float y, const float z)
{
	vec3dfListenerPos.set(x, y, z);
}

/**
 @brief Set Listener direction
 @param x A const float variable containing the x-component of a direction
 @param y A const float variable containing the y-component of a direction
 @param z A const float variable containing the z-component of a direction
 */
void CSoundController::SetListenerDirection(const float x, const float y, const float z)
{
	vec3dfListenerDir.set(x, y, z);
}

/**
 @brief PrintSelf
 */
void CSoundController::PrintSelf(void)
{
	cout << "CSoundController::PrintSelf()" << endl;
	cout << "\tFor 3D sound: Listener position\t=\t" << vec3dfListenerPos.X << ", " << vec3dfListenerPos.Y << ", " << vec3dfListenerPos.Z << endl;
	cout << "\tFor 3D sound: Listener direction\t=\t" << vec3dfListenerDir.X << ", " << vec3dfListenerDir.Y << ", " << vec3dfListenerDir.Z << endl;

	cout << "\tThe map of all the soundMap created:" << endl;
	// Iterate through the soundMap
	for (std::map<int, CSoundInfo*>::iterator it = soundMap.begin(); it != soundMap.end(); ++it)
	{
		cout << "\t" << it->first << "\t: " << ((CSoundInfo*)it->second)->GetFilename() << endl;
	}
	cout << "End of CSoundController::PrintSelf()" << endl << endl;
}

/**
 @brief Get an sound from this map
 @param ID A const int variable which will be the ID of the iSoundSource in the map
 @return A CSoundInfo* variable
 */
CSoundInfo* CSoundController::GetSound(const int iID)
{
	if (soundMap.count(iID) != 0)
		return soundMap[iID];

	return nullptr;
}

/**
 @brief Remove an sound from this map
 @param ID A const int variable which will be the ID of the iSoundSource in the map
 @return true if the sound was successfully removed, else false
 */
bool CSoundController::RemoveSound(const int iID)
{
	CSoundInfo* pSoundInfo = GetSound(iID);
	if (pSoundInfo != nullptr)
	{
		delete pSoundInfo;
		soundMap.erase(iID);
		return true;
	}
	return false;
}

/**
@brief Get the number of sounds in this map
@return The number of sounds currently stored in the Map
*/
int CSoundController::GetNumOfSounds(void) const
{
	return soundMap.size();
}