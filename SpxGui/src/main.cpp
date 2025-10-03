#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <iostream>

#include "GLwin.h"
#include "GLwinLOG.h"

#include "../SpxGui.h"
#include "../SpxGuiWidgets.h"



int main() {
	std::cout << "Hello, SpxGui!" << std::endl;
	GLWIN_LOG_INFO("This is an info message.");
	
	GLWIN_window* window = GLwin_CreateWindow(800, 600, L"Starting GLwin! with Modern OpenGL");

	if (!window) {
		std::cerr << "Failed to create GLwin window!" << std::endl;
		return -1;
	}

	GLwinMakeContextCurrent(window);

	int fbw, fbh;
	GLwinGetFramebufferSize(window, &fbw, &fbh);
	std::cout << "Framebuffer size: " << fbw << " x " << fbh << std::endl;

	if (!gladLoadGLLoader((GLADloadproc)GLwinGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD!" << std::endl;
		//GLwin_DestroyWindow(window);
		return -1;
	}
	else {
		GLWIN_LOG_INFO("GLAD initialized successfully.");
	}

	glViewport(0, 0, fbw, fbh);
	SpxGui::Init(fbw, fbh);
	// tell SpxGui about the real framebuffer size
	SpxGui::SetScreenSize(fbw, fbh);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	glGetString(GL_VERSION);
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

	bool showWin1 = true;
	bool showWin2 = false;
	while (!GLwinWindowShouldClose(window)) {
		SpxGui::gFrameCount++;
		GLwinPollEvents(); // New non-blocking event polling

		double mx, my;
		GLwinGetCursorPos(window, &mx, &my);
		bool down = GLwinGetMouseButton(window, GLWIN_MOUSE_BUTTON_LEFT) == GLWIN_PRESS;

		static bool prevDown = false;
		bool pressed = (down && !prevDown);
		bool released = (!down && prevDown);
		prevDown = down;

		SpxGui::NewFrame((float)mx, (float)my, down, pressed, released);

		if (GLwinGetKey(window, GLWIN_ESCAPE) == GLWIN_PRESS) { // ESC key
			std::wcout << L"Escape key pressed window is closing!" << std::endl;
			break; // ESC key
		}
		if (GLwinGetKey(window, GLWIN_SPACE) == GLWIN_PRESS) { // ESC key
			
			showWin2 = true;
		}

		glClearColor(0.17f, 0.17f, 0.18f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		// ------------GUI Rendering code ------------
		
		if (showWin1) {
			SpxGui::Begin("Lighting Editor", &showWin1,1);   // will draw rect
			if (SpxGui::Button("Add Light", 100, 30)) {
				std::cout << "Add Light clicked\n";
			}
			// testing Style changes
			if (SpxGui::Button("Add Light_01", 100, 30)) {
				std::cout << "Add Light clicked_01\n";
			}
			if (SpxGui::Button("Add Light_02", 100, 30)) {
				std::cout << "Add Light clicked_02\n";
			}
			if (SpxGui::InputText("Light Name", (char*)"This is a Text box", 32, 200, 30)) {

			}
			if (SpxGui::Button("Add Light_03", 100, 30)) {
				std::cout << "Add Light clicked_03\n";
			}
			SpxGui::SeparatorText("Light Properties", 200);
			SpxGui::ColoredLalel(1.0f, 1.0f, 0.0f, "Color Lable_1:");
			SpxGui::SeparatorLine(200);
			SpxGui::ColoredLalel(1.0f, 1.0f, 0.0f, "Color Lable_2:");
			
			float lightR = 1.0f, lightG = 1.0f, lightB = 0.0f;
			// Color box test
			if (SpxGui::ColorBox("Light Color", &lightR, &lightG, &lightB)) {
				std::cout << "Clicked color box, current color = "
					<< lightR << ", " << lightG << ", " << lightB << "\n";
				

			}
			
			SpxGui::End();
		}
		if (showWin2) {
			SpxGui::Begin("Texture Editor", &showWin2, 2);   // will draw rect
			if (SpxGui::Button("Add Texture", 100, 30)) {
				std::cout << "Add Texture clicked\n";
			}
			if (SpxGui::ImageButton("../SpxGui/Textures/Brick.jpg", 50, 50))
				std::cout << "ImageButton clicked\n";
			SpxGui::SameLine();
			if (SpxGui::ImageButton("../SpxGui/Textures/Brick.jpg", 50, 50))
				std::cout << "ImageButton clicked_01\n";		
				
			
			if (SpxGui::ImageBox("../SpxGui/Textures/container2.png", 200, 200, "Wooden Box"))
				std::cout << "Image Box clicked\n";

			

			/*float col[3] = { 0.8f, 0.2f, 0.3f };
			if (SpxGui::ColorEdit3("Clear Color", col ))
				std::cout << "Color changed: " << col[0] << ", " << col[1] << ", " << col[2] << std::endl;
				*/		
			SpxGui::End();
		}

		GLwinSwapBuffers(window);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			GLWIN_LOG_DEBUG("OpenGL Error: " << err);
		}

	}
	GLWIN_LOG_INFO("Window is closing, cleaning up.");

	return 0;
}