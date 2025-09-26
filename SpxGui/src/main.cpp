#include <iostream>
#include <glad/glad.h>
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

	while (!GLwinWindowShouldClose(window)) {

		GLwinPollEvents(); // New non-blocking event polling


		if (GLwinGetKey(window, GLWIN_ESCAPE) == GLWIN_PRESS) { // ESC key
			std::wcout << L"Escape key pressed window is closing!" << std::endl;
			break; // ESC key
		}
	}
	GLWIN_LOG_INFO("Window is closing, cleaning up.");

	return 0;
}