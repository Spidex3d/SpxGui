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
	int w, h;
	GLwinGetFramebufferSize(window, &w, &h);
	std::cout << "Framebuffer size: " << w << " x " << h << std::endl;

	if (!gladLoadGLLoader((GLADloadproc)GLwinGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD!" << std::endl;
		//GLwin_DestroyWindow(window);
		return -1;
	}
	else {
		GLWIN_LOG_INFO("GLAD initialized successfully.");
	}

	SpxGui::Init(w, h);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	glGetString(GL_VERSION);
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

	bool showWin1 = true;
	bool showWin2 = false;
	while (!GLwinWindowShouldClose(window)) {

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
		glViewport(0, 0, w, h);

		// ------------GUI Rendering code ------------
		
		if (showWin1) {
			SpxGui::Begin("Lighting Editor", &showWin1,1);   // will draw rect
			if (SpxGui::Button("Add Light", 20, 40, 100, 30)) {
				std::cout << "Add Light clicked\n";
			}
			SpxGui::InputText("Light Name", (char*)"This is a Text box", 32, 20, 80, 200, 30);
			SpxGui::TextColored(1.0f, 1.0f, 0.0f, "Light Color:", 20, 140);
			SpxGui::SeparatorText("Light Properties", 20, 120, 200);
			SpxGui::SeparatorLine(20, 130, 200);
			SpxGui::End();
		}
		if (showWin2) {
			SpxGui::Begin("Texture Editor", &showWin2, 2);   // will draw rect
			if (SpxGui::Button("Add Texture", 20, 40, 100, 30)) {
				std::cout << "Add Texture clicked\n";
			}
				
			if (SpxGui::ImageBox("../SpxGui/Textures/Brick.jpg", 20, 80, 200, 200))
				std::cout << "Image Box clicked\n";
						
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