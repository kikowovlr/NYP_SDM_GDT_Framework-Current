/**
 main.cpp
 @brief This file contains the main function for this project
 */
#include "Application.h"

// Include GLM
#include <includes/glm.hpp>
#include <includes/gtc/matrix_transform.hpp>
#include <includes/gtc/type_ptr.hpp>

#include "GameControl/Scoreboard.h"

/**
 @brief This function is the main function which is called by the operating system when you run the executables
 @return This function returns the error codes
 */
int main(void)
{
	//CScoreboard* pScoreboard = CScoreboard::GetInstance();
	//CScoreboard::Entry e;
	//e.setName("ABC");
	//e.iScore = 100;
	//pScoreboard->AddEntry(e);
	//pScoreboard->SaveFile("c:\\Temp\\test.score");
	//pScoreboard->LoadFile("c:\\Temp\\test.score");

	//CScoreboard::Entry e2 = pScoreboard->GetEntry(0);
	//e2.PrintSelf();
	//pScoreboard->Destroy();

	Application* pApp = Application::GetInstance();
	// if the application is initialised properly, then run it
	if (pApp->Init() == true)
	{
		// Run the Application instance
		pApp->Run();

		// Destroy the Application instance, just to be sure
		pApp->Destroy();

		// Set the Application handler to NULL
		pApp = NULL;

		// Return 0 since the application successfully ran
		return 0;
	}

	// Return 1 if the application failed to run
	return 1;
}
