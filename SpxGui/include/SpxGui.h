#pragma once
#include <iostream>


void 
TestSpxGui() {
	std::cout << "Hello, SpxGui.h file!" << std::endl;
}
//Button
class Button { 
public:
	Button(const std::string& text) : text(text) {}
	void click() { std::cout << "Button " << text << " clicked!" << std::endl; }

private:
	std::string text;
};
