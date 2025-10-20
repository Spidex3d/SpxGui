#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <iostream>

#include "GLwin.h"
#include "GLwinTime.h"
#include "GLwinDialog.h"
#include "GLwinLOG.h"

#include "../SpxGui.h"
#include "../SpxGuiWidgets.h"


int main() {
	std::cout << "Hello, SpxGui!" << std::endl;
	GLWIN_LOG_INFO("This is an info message.");
	
	//GLWIN_window* window = GLwin_CreateWindow(800, 600, L"GLwin! with Modern OpenGL and SpxGui");
	GLWIN_window* window = GLwin_CreateWindow(1200, 800, L"GLwin! with Modern OpenGL and SpxGui");
	//GLWIN_window* window = GLwin_CreateWindow(1760, 990, L"GLwin! with Modern OpenGL and SpxGui");

	if (!window) {
		std::cerr << "Failed to create GLwin window!" << std::endl;
		return -1;
	}

	// Toggle custom title bar on/off
	GLwinEnableCustomTitleBar(window, GLWIN_TRUE);  // Enable custom title bar - ON
	//GLwinEnableCustomTitleBar(window, GLWIN_FALSE); // Restore Windows default - OFF

	GLwinMakeContextCurrent(window);

	/*int fbw, fbh;
	GLwinGetFramebufferSize(window, &fbw, &fbh);
	std::cout << "Framebuffer size: " << fbw << " x " << fbh << std::endl;*/
	
	/*int w, h;
	SpxGui::UpdateScreenSize(fbw, fbh);*/

	// for use with Menu bar position
	//gMainWindow = window; // store pointer to main window for SpxGui
	SpxGui::gMainWindow = window;

	// Set character input callback and key callback for SpxGui
	GLwinSetCharCallback(window, SpxGui::CharCallback);
	GLwinSetKeyCallback(window, SpxGui::KeyCallback);

	
	// Load OpenGL functions using GLAD
	if (!gladLoadGLLoader((GLADloadproc)GLwinGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD!" << std::endl;
		//GLwin_DestroyWindow(window);
		return -1;
	}
	else {
		GLWIN_LOG_INFO("GLAD initialized successfully.");
	}

	//glViewport(0, 0, fbw, fbh);
	SpxGui::Init();  // sets up shaders 
	
	if (SpxGui::gMainWindow)
		std::cout << "gMainWindow set correctly\n";
	else
		std::cout << "gMainWindow is NULL!\n";

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	glGetString(GL_VERSION);
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

	// Gui state variables
	bool showWin1 = true;
	bool showWin2 = false;
	bool showPopup = false;
	bool showColWin1 = false;
	bool tabNew = false;
	static SpxGui::Image bgImg;
	static std::string currentImage = "";   // empty = none shown

	float r = 0.217, g = 0.207, b = 0.184;
	// Timing set the fps
	const double targetFPS = 60.0; // or 120.0
	const double targetFrameTime = 1.0 / targetFPS; // in seconds

	//SpxGui::SetScreenSize(fbw, fbh);

	while (!GLwinWindowShouldClose(window, 0)) {
		double frameStart = GLwinGetTime(); // new timer

		GLwinPollEvents(); // New non-blocking event polling

		// Client coords (relative to your GL window)
		double cx, cy;
		GLwinGetCursorPos(window, &cx, &cy);

		// ------------------------- Handle resizing ---------------------------
		int fbw, fbh;
		GLwinGetFramebufferSize(window, &fbw, &fbh);
		//std::cout << "Framebuffer size: " << fbw << " x " << fbh << std::endl;
		int w, h;
		SpxGui::UpdateScreenSize(fbw, fbh);

		glViewport(0, 0, fbw, fbh);
		SpxGui::SetScreenSize(fbw, fbh);
		// ------------------------- Handle resizing End -------------------------

						
		SpxGui::MenuInit(); // update mouse pos for menu bar

		bool downNow = GLwinGetMouseButton(window, GLWIN_MOUSE_BUTTON_LEFT) == GLWIN_PRESS;

		static bool prevDown = false;
		SpxGui::pressed = (downNow && !prevDown);
		SpxGui::released = (!downNow && prevDown);
		SpxGui::down = downNow;
		prevDown = downNow;

		if (SpxGui::gCurrent) {
			SpxGui::gCurrent->mouseX = (float)cx;
			SpxGui::gCurrent->mouseY = (float)cy;
		}

		// pass mouse state to SpxGui
		if (GLwinGetKey(window, GLWIN_ESCAPE) == GLWIN_PRESS) { // ESC key
			std::wcout << L"Escape key pressed window is closing!" << std::endl;
			//break; // ESC key
			GLwinWindowShouldClose(window, 1);
		}

		//glClearColor(0.17f, 0.17f, 0.18f, 1.0f);
		glClearColor(r, g, b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// ------------GUI Rendering code ------------
				
		if (showWin1) {
			SpxGui::Begin("Demo Editor", &showWin1,1);   // will draw rect

			// load background texture once
			static unsigned int bgTex = SpxGui::LoadTextuer("../SpxGui/Textures/background.jpg", bgImg);
			SpxGui::gCurrent->backgroundTex = bgTex;   //  set per-window background

			if (SpxGui::ButtonNew("Add Tabs", 100, 30)) {
				tabNew = true;
			}
			if (SpxGui::ButtonNew("Save Dialog", 100, 30)) {
				// open file dialog
				std::string filename = GLwinSaveDialog();
				if (!filename.empty()) {
					// Save to this file...
				}
			}
			if (SpxGui::ButtonNew("Open Dialog", 100, 30)) {
				// open file dialog
				std::string filename = GLwinOpenDialog();
				if (!filename.empty()) {
					// Open this file...
				}
			}
			if (SpxGui::Button("Open New Window", 200, 30)) {
				std::cout << "New Window\n";
				showWin2 = true;
			}
			
			// testing Style changes
			static char buf1[32] = "Input Text";  // make a writable buffer
			SpxGui::InputText("Text Name", (char*)buf1, sizeof(buf1), 200, 30);
						
			static char buf[1000] = "Input Text";  // make a writable buffer
			SpxGui::MultiLineText("Text Name_01", (char*)buf, sizeof(buf), 200, 200);

			
			SpxGui::SeparatorText("Light Properties", 200);
			
			
			float lightR = 1.0f, lightG = 1.0f, lightB = 0.0f;
			// Color box test
			if (SpxGui::ColorBoxLabel("Color box label", &lightR, &lightG, &lightB)) {
				std::cout << "Clicked color box, current color = "
					<< lightR << ", " << lightG << ", " << lightB << "\n";
				
			}
			
			SpxGui::End();
		}
		if (showWin2) {
			SpxGui::Begin("Demo Editor_02", &showWin2, 2);   // will draw rect
			static unsigned int bgTex = SpxGui::LoadTextuer("../SpxGui/Textures/background.jpg", bgImg);
			SpxGui::gCurrent->backgroundTex = bgTex;   //  set per-window background


			float color[4] = { 1.0f, 2.0f, 3.0f, 1.0f };
			SpxGui::Drag4Float(color, 60, 20);

			float color_t[4] = { 0.2f, 0.5f, 0.8f, 1.0f };
			SpxGui::Drag4FloatText(color_t);

			float color_r[3] = { 0.2f, 0.5f, 0.8f };
			SpxGui::Drag3FloatText(color_r);

			float pos[3] = { 1.0f, 2.0f, 3.0f };
			SpxGui::Drag3Float(pos, 60, 20);


			// inside your loop:
			if (SpxGui::ImageButton("../SpxGui/Textures/Brick.jpg", 50, 50)) {
				std::cout << "Brick clicked\n";
				currentImage = "../SpxGui/Textures/Brick.jpg";   // set active image
			}
			SpxGui::SameLine();

			if (SpxGui::ImageButton("../SpxGui/Textures/container2.png", 50, 50)) {
				std::cout << "Container clicked\n";
				currentImage = "../SpxGui/Textures/container2.png";
			}
			SpxGui::SameLine();

			if (SpxGui::ImageButton("../SpxGui/Textures/github.jpg", 50, 50)) {
				std::cout << "GitHub clicked\n";
				currentImage = "../SpxGui/Textures/github.jpg";
			}
			
			SpxGui::SeparatorLine(200);

			// now always draw the currently selected image
			if (!currentImage.empty()) {
				if (SpxGui::ImageBox(currentImage, 100, 100, "Image Preview")) {
					std::cout << "Image Box clicked\n";
				}
			}
						

			float col[3] = { 0.8f, 0.2f, 0.3f };
			if (SpxGui::ColorEdit3("Select Color", col)) {
				//showColWin1 = true;
			  // showPopup = true;
			}

			SpxGui::ColoredLabel(1.0f, 1.0f, 0.0f, "Color Lable_2:");

			SpxGui::End();
		}
		// ------------------------------------------------ Tab test ------------------------------------------------
		if (tabNew) {
			SpxGui::Begin("Tabs Editor", &tabNew, 3);

			SpxGui::BeginTabBar("MainTabs");
			if (SpxGui::BeginTabItem("Scene")) {
				static char tabbuf[1000] = "Input Text";  // make a writable buffer
				SpxGui::MultiLineText("Text Name_01", (char*)tabbuf, sizeof(tabbuf), 200, 200);
				SpxGui::EndTabItem();
			}
			if (SpxGui::BeginTabItem("Lighting")) {
				SpxGui::ColorBox("Sun Color", &r, &g, &b);
				SpxGui::EndTabItem();
			}
			if (SpxGui::BeginTabItem("Settings")) {
				static char buft[32] = "Input Text";
				SpxGui::InputText("Project Name", buft, sizeof(buft), 200, 30);
				SpxGui::EndTabItem();
			}
			SpxGui::EndTabBar();

			SpxGui::End();
		}

		

		SpxGui::NewFrame((float)cx, (float)cy, downNow, SpxGui::pressed, SpxGui::released, fbw, fbh);
		//SpxGui::NewFrame(downNow, SpxGui::pressed, SpxGui::released, fbw, fbh);
		//SpxGui::NewFrame(SpxGui::gMouseDown, SpxGui::gMousePressed,	SpxGui::gMouseReleased, fbw, fbh);

		// Draw the top menu bar
		SpxGui::RenderMenuBar(); // works with your real screen width

		SpxGui::Render();  // will render all windows

		
		// ------------ End of GUI Rendering code ------------

		GLwinSwapBuffers(window);

		// Throttle to target FPS 60 or 120
		double frameEnd = GLwinGetTime();
		double elapsed = frameEnd - frameStart;
		if (elapsed < targetFrameTime) {
			double waitTime = targetFrameTime - elapsed;
			DWORD ms = (DWORD)(waitTime * 1000.0);
			if (ms > 0) Sleep(ms); // Sleep for the remaining time
			// Spin-wait for extra precision (optional)
			while ((GLwinGetTime() - frameStart) < targetFrameTime) {}
			//std::cout << "FPS: " << targetFrameTime << std::endl;
		}

		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			GLWIN_LOG_DEBUG("OpenGL Error: " << err);
		}

	}
	GLWIN_LOG_INFO("Window is closing, cleaning up.");

	return 0;
}