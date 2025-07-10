/**
 Application.cpp
 @brief This file contains the Application class for this project, which initialises and control the OpenGL environment
*/
#include "Application.h"

#include <Windows.h>
//Include the standard C++ headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#ifndef GLEW_STATIC
	#include <GL/glew.h>
	#define GLEW_STATIC
#endif

#include "GameControl\Settings.h"

// Inputs
#include "Inputs\KeyboardController.h"
#include "Inputs\MouseController.h"

#include <iostream>
using namespace std;

// Include GLFW
#include <GLFW/glfw3.h>

#include "Scene2D/Scene2D.h"
CScene2D* pScene2D = NULL;

// Include filesystem
#include "System\filesystem.h"

// Include SoundController
#include "SoundController/SoundController.h"

/**
 @brief Define an error callback
 @param error The error code
 @param description The details about the error
 */
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

/**
 @brief Define the key input callback
 @param window The window to receive the the instructions
 @param key The key code
 @param scancode The scan code
 @param action The action to take
 @param mods The modifications
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	CKeyboardController::GetInstance()->Update(key, action);
}

/**
 @brief Callback function when the OpenGL window is repositioned
 @param window The window to receive the the instructions
 @param xpos integer value of the new x position of the window
 @param ypos integer value of the new y position of the window
*/
void repos_callback(GLFWwindow* window, int xpos, int ypos)
{
	// Enable the cursor
	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
	{
		// Reset to default OpenGL window position as stored in .ini file
		glfwSetWindowPos(	window, 
							CSettings::GetInstance()->cSimpleIniA.GetIntValue("Position", "iWindowPosX", 50),
							CSettings::GetInstance()->cSimpleIniA.GetIntValue("Position", "iWindowPosY", 50));
		return;
	}

	// Set OpenGL window position
	glfwSetWindowPos(window, xpos, ypos);

	// Store the window width position
	CSettings::GetInstance()->cSimpleIniA.SetValue("Position", "iWindowPosX", (std::to_string(xpos)).c_str());
	CSettings::GetInstance()->cSimpleIniA.SetValue("Position", "iWindowPosY", (std::to_string(ypos)).c_str());
	//CSettings::GetInstance()->SaveFile();

	// Update the glViewPort
	glViewport(	0, 0,
				CSettings::GetInstance()->cSimpleIniA.GetIntValue("Size", "iWindowWidth", 800),
				CSettings::GetInstance()->cSimpleIniA.GetIntValue("Size", "iWindowHeight", 600));
}

/**
 @brief Callback function when the OpenGL window is resized
 @param window The window to receive the the instructions
 @param w integer value of the new width of the window
 @param h integer value of the new height of the window
 */
void resize_callback(GLFWwindow* window, int width, int height)
{
	// Set OpenGL window position
	glfwSetWindowSize(window, width, height);

	// Store the window width and height
	CSettings::GetInstance()->cSimpleIniA.SetValue("Size", "iWindowWidth", (std::to_string(width)).c_str());
	CSettings::GetInstance()->cSimpleIniA.SetValue("Size", "iWindowHeight", (std::to_string(height)).c_str());

	// Update the glViewPort
	glViewport(	0, 0, width, height);
}

/**
 @brief Callback function for the cursor position
 @param window The window to receive the the instructions
 @param xpos integer value of the new x position of the cursor
 @param ypos integer value of the new y position of the cursor
 */
//void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
//{
//	double newxpos, newypos;
//	glfwGetCursorPos(window, &newxpos, &newypos);
//	std::cout << "Position: (" << newxpos << ":" << newypos << ")";
//}

/**
 @brief Callback function whenever the mouse has events
 @param window The window to receive the the instructions
 @param button An integer value of the mouse button causing the event
 @param action An integer value of the action caused by the mouse button
 @param mods An integer value storing the mods of the event
 */
void MouseButtonCallbacks(GLFWwindow* window, int button, int action, int mods)
{
	// Send the callback to the mouse controller to handle
	if (action == GLFW_PRESS)
		CMouseController::GetInstance()->UpdateMouseButtonPressed(button);
	else
		CMouseController::GetInstance()->UpdateMouseButtonReleased(button);
}

/**
 @brief Callback function whenever the mouse has events
 @param window The window to receive the the instructions
 @param xoffset A double value of the mouse scroll offset in the x-axis
 @param yoffset A double value of the mouse scroll offset in the y-axis
 */
void MouseScrollCallbacks(GLFWwindow* window, double xoffset, double yoffset)
{
	CMouseController::GetInstance()->UpdateMouseScroll(xoffset, yoffset);
}

/**
@brief Destructor
*/
Application::~Application(void)
{
}

/**
 @brief Initialise this class instance
 */
bool Application::Init(void)
{
	// glfw: initialize and configure
	// ------------------------------
	//Initialize GLFW
	if (!glfwInit())
	{
		// return false if the GLFW was not initialised successfully
		return false;
	}

	// Get the CSettings instance
	pSettings = CSettings::GetInstance();

	// Initialise the CSettings instance
	pSettings->Init();
	// Load the .ini file
	pSettings->LoadFile(".\\ini\\NYP_SDM_GDT_Framework.ini");

	//Set the GLFW window creation hints - these are optional
	if (pSettings->cSimpleIniA.GetBoolValue("Antialiasing", "bUse4XAntialiasing", "true") == true)
		glfwWindowHint(GLFW_SAMPLES, 4); //Request 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Request a specific OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //Request a specific OpenGL version
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

	// Display the titlebar based on the value in .ini file
	if (CSettings::GetInstance()->cSimpleIniA.GetBoolValue("TitleBar", "bDisplay", "true") == false)
		glfwWindowHint(GLFW_DECORATED, false);	// Don't display the title bar

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	//Create a window and create its OpenGL context
	pSettings->pWindow = glfwCreateWindow(pSettings->cSimpleIniA.GetIntValue("Size", "iWindowWidth", 800),
		pSettings->cSimpleIniA.GetIntValue("Size", "iWindowHeight", 600),
		"NYP Framework", NULL, NULL);
	//If the window couldn't be created, then return false
	if (pSettings->pWindow == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		return false;
	}

	// Set OpenGL window position
	glfwSetWindowPos(pSettings->pWindow,
		pSettings->cSimpleIniA.GetIntValue("Position", "iWindowPosX", 50),
		pSettings->cSimpleIniA.GetIntValue("Position", "iWindowPosY", 50));

	//This function makes the context of the specified window current on the calling thread. 
	glfwMakeContextCurrent(pSettings->pWindow);

	//Sets the callback functions for GLFW
	//Set the window position callback function
	glfwSetWindowPosCallback(pSettings->pWindow, repos_callback);
	//Set the window resize resize callback function
	glfwSetWindowSizeCallback(pSettings->pWindow, resize_callback);
	//Set the frame buffer resize callback function
	glfwSetFramebufferSizeCallback(pSettings->pWindow, resize_callback);
	//Set the error callback function
	glfwSetErrorCallback(error_callback);
	// Set the keyboard callback function
	glfwSetKeyCallback(pSettings->pWindow, key_callback);
	//// Set the cursor callback function
	//glfwSetCursorPosCallback(pSettings->pWindow, cursor_position_callback);
	//Set the mouse button callback function
	glfwSetMouseButtonCallback(pSettings->pWindow, MouseButtonCallbacks);
	//Set the mouse scroll callback function
	glfwSetScrollCallback(pSettings->pWindow, MouseScrollCallbacks);

	// Additional customisation for the GLFW environment
	// Disable the cursor
	if (pSettings->cSimpleIniA.GetBoolValue("Mouse", "bDisableMousePointer", "false") == true)
		glfwSetInputMode(pSettings->pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		// Hide the cursor
		if (pSettings->cSimpleIniA.GetBoolValue("Mouse", "bShowMousePointer", "true") == false)
			glfwSetInputMode(pSettings->pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	// glewExperimental is a variable that is already defined by GLEW. You must set it to GL_TRUE before calling glewInit(). 
	glewExperimental = GL_TRUE;
	// Initialize GLEW
	GLenum glewInitErr = glewInit();
	//If GLEW hasn't initialized, then return false
	if (glewInitErr != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glewInitErr));
		glfwTerminate();
		return false;
	}

	// Add the shaders to the ShaderManager
	CShaderManager::GetInstance()->Add("Shader2D", pSettings->cSimpleIniA.GetValue("Shader", "Shader2D.vs"), pSettings->cSimpleIniA.GetValue("Shader", "Shader2D.fs"));
	CShaderManager::GetInstance()->Add("Shader2D_Colour", pSettings->cSimpleIniA.GetValue("Shader", "Shader2D_Colour.vs"), pSettings->cSimpleIniA.GetValue("Shader", "Shader2D_Colour.fs"));

	// Initialise the CFPSCounter instance
	pFPSCounter = CFPSCounter::GetInstance();
	pFPSCounter->Init(true);

	// Initialise CScene2D
	pScene2D = CScene2D::GetInstance();
	if (pScene2D->Init() == false)
	{
		cout << "Application::Init() - Unable to initialise Cscene2D";
		return false;
	}

	// Init the CSoundController singleton
	if (CSoundController::GetInstance()->Init() == false)
	{
		// Destroy the CSoundController singleton
		cout << "Failed to initialise CSoundController class instance." << endl;

		// Destroy the cScene3D instance
		if (pScene2D)
		{
			pScene2D->Destroy();
			pScene2D = NULL;
		}
	}

	return true;
}

/**
@brief Run this class instance
*/ 
void Application::Run(void)
{
	// Initialise the CFPSCounter instance
	pFPSCounter->Init(true, 0.0333333333333333);
	// Start timer to calculate how long it takes to render this frame
	pFPSCounter->StartTimer();

	double dElapsedTime = 0.0;
	
	// Render loop
	while (!glfwWindowShouldClose(pSettings->pWindow))
	{
		if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_ESCAPE))
		{
			break;
		}
		else if (CKeyboardController::GetInstance()->IsKeyReleased(GLFW_KEY_T))
		{
			bFullscreen = !bFullscreen;
			glfwSetWindowMonitor(	pSettings->pWindow, bFullscreen ? glfwGetPrimaryMonitor() : NULL, 0, 0, 
									pSettings->cSimpleIniA.GetIntValue("Size", "iWindowWidth", 800),
									pSettings->cSimpleIniA.GetIntValue("Size", "iWindowHeight", 600),
									GLFW_DONT_CARE);
		}

		// Call the active Game State's Update method
		if (pScene2D->Update(dElapsedTime) == false)
		{
			break;
		}
		

		// Call the CScene2D's Pre-Render method
		pScene2D->PreRender();

		// Call the CScene2D's Render method
		pScene2D->Render();

		// Call the CScene2D's PostRender method
		pScene2D->PostRender();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(pSettings->pWindow);

		// Perform Post Update Input Devices
		PostUpdateInputDevices();

		// Poll events
		glfwPollEvents();

		// Update Input Devices
		UpdateInputDevices();

		// Update the FPS Counter
		pFPSCounter->Update();

		// Calculate the delta time since the last frame
		dElapsedTime = pFPSCounter->GetDeltaTime();
	}

	// Stop the timer
	pFPSCounter->StopTimer();
}

/**
 @brief Destroy this class instance
 */
void Application::Destroy(void)
{
	if (pScene2D)
	{
		pScene2D->Destroy();
		pScene2D = NULL;
	}

	// Destory the ShaderManager
	CShaderManager::GetInstance()->Destroy();

	// Destroy the mouse instance
	CMouseController::GetInstance()->Destroy();
	// Destroy the keyboard instance
	CKeyboardController::GetInstance()->Destroy();

	// Destroy the CFPSCounter instance
	if (pFPSCounter)
	{
		pFPSCounter->Destroy();
		pFPSCounter = NULL;
	}

	//Close OpenGL window and terminate GLFW
	glfwDestroyWindow(pSettings->pWindow);
	//Finalize and clean up GLFW
	glfwTerminate();

	// Destroy the CSettings instance
	CSettings::GetInstance()->Destroy();
}

/**
 @brief Get window height
 */
int Application::GetWindowHeight(void) const
{
	return pSettings->cSimpleIniA.GetIntValue("Size", "iWindowHeight", 600);
}

/**
 @brief Get window width
 */
int Application::GetWindowWidth(void) const
{
	return pSettings->cSimpleIniA.GetIntValue("Size", "iWindowWidth", 800);
}

/**
 @brief Constructor
 */
Application::Application(void)
	: pFPSCounter(NULL)
	, bFullscreen(false)
{
}

/**
@brief Get updates from the input devices
*/
void Application::UpdateInputDevices(void)
{
	// Update Mouse Position
	double dMouse_X, dMouse_Y;
	glfwGetCursorPos( pSettings->pWindow, &dMouse_X, &dMouse_Y);
	CMouseController::GetInstance()->UpdateMousePosition( dMouse_X, dMouse_Y);
}

/**
 @brief End updates from the input devices. This method is not used anymore
 */
void Application::PostUpdateInputDevices(void)
{
	CKeyboardController::GetInstance()->PostUpdate();
}
