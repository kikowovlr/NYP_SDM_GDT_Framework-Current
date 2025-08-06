#include "CDelayedSound.h"
#include <iostream>

void CDelayedSound::OnSoundStopped(ISound* sound, E_STOP_EVENT_CAUSE reason, void* userData)
{
	if (reason == E_STOP_EVENT_CAUSE::ESEC_SOUND_FINISHED_PLAYING) // Finished playing naturally
	{
		// play delayed sound
		auto* pSoundController = CSoundController::GetInstance();
		CSoundInfo* pSoundInfo = pSoundController->GetSound(id);

		ISound* nextSound = pSoundController->getSoundEngine()->play2D(
			pSoundInfo->GetSound(),
			false,
			false,
			true);
		
		if (id == 7)
		{
			ISoundEffectControl* fx = nextSound->getSoundEffectControl();

			if (!fx) {
				// some devices do not support sound effects
				printf("This device or sound does not support sound effects\n");
			}
			else
			{
				fx->enableGargleSoundEffect();
			}
		}
	}
}
