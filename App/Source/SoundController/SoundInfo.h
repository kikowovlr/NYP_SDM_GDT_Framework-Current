/**
 CSoundInfo
 @brief A class which stores a sound object
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

 // Include IrrKlang
#include <includes/irrKlang.h>
using namespace irrklang;
#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

class CSoundInfo
{
public:
	// The types of sound
	enum SOUNDTYPE
	{
		_2D = 0,
		_3D,
		NUM_SOUNDTYPE
	};

	// Constructor
	CSoundInfo(void);

	// Destructor
	virtual ~CSoundInfo(void);

	// Initialise this class instance
	bool Init(const int iID,
		ISoundSource* pSoundSource,
		const bool bIsLooped = false,
		SOUNDTYPE eSoundType = _2D,
		vec3df vec3dfSoundPos = vec3df(0.0f, 0.0f, 0.0f));

	// Get ID
	int GetID(void) const;

	// Set filename
	void SetFilename(const char* filename);
	// Get filename
	const char* GetFilename(void);

	// Get an sound from this map
	ISoundSource* GetSound(void) const;

	// Get loop status
	bool GetLoopStatus(void) const;

	// Get sound type
	SOUNDTYPE GetSoundType(void) const;

	// Set the volume
	void SetVolume(const float fVolume);
	// Get the volume
	float GetVolume(void);

	// For 3D sounds only
	// Set position
	void SetPosition(const float x, const float y, const float z);

	// Get position
	vec3df GetPosition(void) const;

protected:
	// ID for this sound
	int iID;

	// filename for this sound
	const char* filename;

	// The handler to the irrklang Sound Engine
	ISoundSource* pSoundSource;

	float fVolume;

	// Indicate if this sound will be looped during playback
	bool bIsLooped;

	// Sound type
	SOUNDTYPE eSoundType;

	// For 3D sound only: Sound position
	vec3df vec3dfSoundPos;
};