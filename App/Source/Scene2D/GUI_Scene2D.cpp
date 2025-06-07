/**
 CGUI_Scene2D
 @brief A class which manages the GUI for Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
#include "GUI_Scene2D.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::CGUI_Scene2D(void)
	: pSettings(NULL)
	, fProgressBar(0.0f)
	, pInventoryManager(NULL)
	, pInventoryItem(NULL)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::~CGUI_Scene2D(void)
{
	if (pInventoryManager)
	{
		pInventoryManager->Destroy();
		pInventoryManager = NULL;
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// We won't delete this since it was created elsewhere
	pSettings = NULL;
}

/**
  @brief Initialise this instance
  */
bool CGUI_Scene2D::Init(void)
{
	// Get the handler to the CSettings instance
	pSettings = CSettings::GetInstance();

	// Store the CFPSCounter singleton instance here
	pFPSCounter = CFPSCounter::GetInstance();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(CSettings::GetInstance()->pWindow, true);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Define the window flags
	window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;

	//// Show the mouse pointer
	//glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	//// Initialise the pInventoryManager
	pInventoryManager = CInventoryManager::GetInstance();

	return true;
}

/**
 @brief Update this instance
 @param dElapsedTime A const double variable containing the elapsed time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
 */
bool CGUI_Scene2D::Update(const double dElapsedTime)
{
	// Calculate the relative scale to our default windows width
	const float relativeScale_x = CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowWidth", 800.0f) / 800.0f;
	const float relativeScale_y = CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f) / 600.0f;

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// If the OpenGL window is minimised, then don't render the IMGUI widgets
	if ((relativeScale_x == 0.0f) || (relativeScale_y == 0.0f))
	{
		return false;
	}

	// Create an invisible window which covers the entire OpenGL window
	ImGui::Begin("Invisible window", NULL, window_flags);
	ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetWindowSize(ImVec2(CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowWidth", 800.0f), 
								CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f)));
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);

	// Display the FPS
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %d", pFPSCounter->GetFrameRate());

	// Render the Health
	ImGuiWindowFlags healthWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Health", NULL, healthWindowFlags);
	ImGui::SetWindowPos(ImVec2(	CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowWidth", 800.0f) * 0.03f, 
								CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f) * 0.03f));
	ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	// set to global inventory
	pInventoryManager->BindToCharacter(nullptr);
	pInventoryItem = pInventoryManager->GetItem("Health");
	ImGui::Image((void*)(intptr_t)pInventoryItem->GetTextureID(),
		ImVec2(pInventoryItem->vec2Size.x * relativeScale_x, pInventoryItem->vec2Size.y * relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	ImGui::ProgressBar(pInventoryItem->GetCount() /
		(float)pInventoryItem->GetMaxCount(), ImVec2(100.0f * relativeScale_x, 20.0f * relativeScale_y));
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::End();

	// Render the Lives
	ImGuiWindowFlags livesWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Lives", NULL, livesWindowFlags);
	ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowWidth", 800.0f) * 0.85f, CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f) * 0.03f));
	ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
	pInventoryItem = pInventoryManager->GetItem("Lives");
	ImGui::Image((void*)(intptr_t)pInventoryItem->GetTextureID(),
		ImVec2(pInventoryItem->vec2Size.x * relativeScale_x, pInventoryItem->vec2Size.y * relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d",
		pInventoryItem->GetCount(), pInventoryItem->GetMaxCount());
	ImGui::End();

	// Render the inventory items
	pInventoryItem = pInventoryManager->GetItem("Tree");
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));  // Set a background colour
	ImGuiWindowFlags inventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Image", NULL, inventoryWindowFlags);
	ImGui::SetWindowPos(ImVec2(CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowWidth", 800.0f) * 0.03f, CSettings::GetInstance()->cSimpleIniA.GetFloatValue("Size", "iWindowHeight", 600.0f) * 0.9f));
	ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
	ImGui::Image((void*)(intptr_t)pInventoryItem->GetTextureID(),
		ImVec2(pInventoryItem->vec2Size.x * relativeScale_x, pInventoryItem->vec2Size.y * relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Tree: %d / %d",
		pInventoryItem->GetCount(), pInventoryItem->GetMaxCount());
	ImGui::End();
	ImGui::PopStyleColor();

	ImGui::End();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()!
	// You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;
		// Create a window called "Preview IMGUI" and append into it.
		ImGui::Begin("Preview IMGUI", NULL);
		ImGui::SetWindowPos(ImVec2(10.0f, 10.0f)); // Set the top-left of the window at (10,10)
		ImGui::SetWindowSize(ImVec2(300.0f, 200.0f)); // Set the window size to (300,200)
		ImGui::Text("Toggle IMGUI demo."); // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
		if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited / activated)
			counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
				1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		// Pass a pointer to our bool variable (the window will
		// have a closing button that will clear the bool when clicked)
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	//DisplayColourPicker();

	return true;
}


/**
 @brief Set up the OpenGL display environment before rendering
 */
void CGUI_Scene2D::PreRender(void)
{
}

/**
 @brief Render this instance
 */
void CGUI_Scene2D::Render(void)
{
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CGUI_Scene2D::PostRender(void)
{
}

//void CGUI_Scene2D::DisplayColourPicker(void)
//{
//	ImGui::SetWindowPos(ImVec2(10.0f, 10.0f)); // Set the top - left of the window at(10, 10)
//		ImGui::SetWindowSize(ImVec2(100.0f, 50.0f));
//	ImGui::Begin("DisplayColourPicker");
//
//	// Generate a default palette. The palette will persist and can be edited.
//	static bool saved_palette_init = true;
//	static ImVec4 saved_palette[32] = {};
//	if (saved_palette_init)
//	{
//		for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
//		{
//			ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f,
//				saved_palette[n].x, saved_palette[n].y,
//				saved_palette[n].z);
//			saved_palette[n].w = 1.0f; // Alpha
//		}
//		saved_palette_init = false;
//	}
//	static ImVec4 color;
//	static ImVec4 backup_color;
//	ImGui::OpenPopup("mypicker");
//	backup_color = color;
//	if (ImGui::BeginPopup("mypicker"))
//	{
//		static bool alpha_preview = true;
//		static bool alpha_half_preview = false;
//		static bool drag_and_drop = true;
//		static bool options_menu = true;
//		static bool hdr = false;
//		ImGuiColorEditFlags misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) |
//			(drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) | (alpha_half_preview ?
//				ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ?
//					ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 :
//						ImGuiColorEditFlags_NoOptions);
//		ImGui::Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
//		ImGui::Separator();
//		ImGui::ColorPicker4("##picker", (float*)&color, misc_flags |
//			ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
//		ImGui::SameLine();
//
//		ImGui::BeginGroup(); // Lock X position
//		ImGui::Text("Current");
//		ImGui::ColorButton("##current", color, ImGuiColorEditFlags_NoPicker |
//			ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
//		ImGui::Text("Previous");
//		if (ImGui::ColorButton("##previous", backup_color,
//			ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40)))
//			color = backup_color;
//		ImGui::Separator();
//		ImGui::Text("Palette");
//		for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
//		{
//			ImGui::PushID(n);
//			if ((n % 8) != 0)
//				ImGui::SameLine(0.0f,
//					ImGui::GetStyle().ItemSpacing.y);
//
//			ImGuiColorEditFlags palette_button_flags =
//				ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker |
//				ImGuiColorEditFlags_NoTooltip;
//			if (ImGui::ColorButton("##palette", saved_palette[n],
//				palette_button_flags, ImVec2(20, 20)))
//				color = ImVec4(saved_palette[n].x, saved_palette[n].y,
//					saved_palette[n].z, color.w); // Preserve alpha!
//			// Allow user to drop colors into each palette entry. Note that ColorButton() is already a
//				// drag source by default, unless specifying the ImGuiColorEditFlags_NoDragDrop flag.
//				if (ImGui::BeginDragDropTarget())
//				{
//					if (const ImGuiPayload* payload =
//						ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
//						memcpy((float*)&saved_palette[n],
//							payload->Data, sizeof(float) * 3);
//
//				}
//		}
//	}
//}