#define STB_TRUETYPE_IMPLEMENTATION
#include <glad/glad.h>
#include <iostream>

#include "GLwin.h"
#include "GLwinLOG.h"
#include "../SpxGui.h"


int main() {
	std::cout << "Hello, SpxGui!" << std::endl;
	GLWIN_LOG_INFO("This is an info message.");
	TestSpxGui();

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

	//if (!gladLoadGL) {
	//	std::cerr << "Failed to initialize GLAD!" << std::endl;
	//	//GLwin_DestroyWindow(window);
	//	return -1;
	//}
	//else {
	//	GLWIN_LOG_INFO("GLAD initialized successfully.");
	//}
	SpxGui::Init(w, h);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGetString(GL_VERSION);
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;


	bool ShowLightEditor = true;

	while (!GLwinWindowShouldClose(window)) {

		GLwinPollEvents(); // New non-blocking event polling

		double mx, my;
		GLwinGetCursorPos(window, &mx, &my);
		bool down = GLwinGetMouseButton(window, GLWIN_MOUSE_BUTTON_LEFT) == GLWIN_PRESS;
		static bool prevDown = false;
		bool pressed = down && !prevDown;
		bool released = !down && prevDown;
		prevDown = down;

		SpxGui::NewFrame((float)mx, (float)my, down, pressed, released);

		if (GLwinGetKey(window, GLWIN_ESCAPE) == GLWIN_PRESS) { // ESC key
			std::wcout << L"Escape key pressed window is closing!" << std::endl;
			break; // ESC key
		}

		glClearColor(0.17f, 0.17f, 0.18f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, w, h);


		// ------------GUI Rendering code ------------
		
		if (ShowLightEditor) {
			SpxGui::Begin("Lighting Editor");   // will draw rect
			//SpxGui::Text("Hello GUI!");
			SpxGui::End();
		}
		//SpxGui::Render();

		GLwinSwapBuffers(window);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			GLWIN_LOG_DEBUG("OpenGL Error: " << err);
		}

	}
	GLWIN_LOG_INFO("Window is closing, cleaning up.");

	return 0;
}