/**
 Application: It manages the OpenGL environment, and the input devices and timer.
 By: Toh Da Jun
 Date: Mar 2020
 */
#pragma once

// Include SingletonTemplate
#include "DesignPatterns\SingletonTemplate.h"

// FPS Counter
#include "TimeControl\FPSCounter.h"

#include "MusicPlayer.h"

struct GLFWwindow;

class CSettings;

class Application : public CSingletonTemplate<Application>
{
	friend CSingletonTemplate<Application>;
public:
	// Initialise this class instance
	bool Init(void);
	// Run this class instance
	void Run(void);
	// Destroy this class instance
	void Destroy(void);

	// Get window height
	int GetWindowHeight(void) const;
	// Get window width
	int GetWindowWidth(void) const;
	
protected:
	// Constructor
	Application(void);

	// Destructor
	virtual ~Application(void);

	// Update input devices
	void UpdateInputDevices(void);
	void PostUpdateInputDevices(void);

	// The handler to the CSettings instance
	CSettings* pSettings;
	// The handler to the CFPSCounter instance
	CFPSCounter* pFPSCounter;
	// The handler to the CMusicPlayer instance
	CMusicPlayer* pMusicPlayer;
	// Boolean flag to indicate fullscreen
	bool bFullscreen;
};
