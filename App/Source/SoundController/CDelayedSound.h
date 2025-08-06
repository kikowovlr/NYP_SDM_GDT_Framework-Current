#pragma once
#include <includes/irrKlang.h>
using namespace irrklang;
#include "SoundController.h"

// class to play sound on completion of another sfx
class CDelayedSound : public ISoundStopEventReceiver
{
public:
	int id;

	CDelayedSound(int soundID) : id(soundID) {}
	virtual void OnSoundStopped(ISound* sound, E_STOP_EVENT_CAUSE reason, void* userData) override;
};

