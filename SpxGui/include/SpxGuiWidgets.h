#pragma once
#include "SpxGui.h"
#include <unordered_map>

namespace SpxGui
{
	

    // Button(const char* label, const ImVec2& size = ImVec2(0, 0));   // button def
       // this needs to go SpxWidgets.h later
   // inline bool Button(const char* label, float offsetX, float offsetY, float w, float h) {
    inline bool Button(const char* label, float w, float h) {
        if (!gCurrent) return false;

		static int callCount = 0;
		callCount++;
		
		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

        bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
            gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);

        bool clicked = (hover && gCurrent->mousePressed);
		// Color change on hover and click
        float r = 0.3f, gcol = 0.3f, b = 0.3f;
        if (hover) { r = 0.4f; gcol = 0.4f; b = 0.6f; }
        if (clicked) { r = 0.2f; gcol = 0.6f; b = 0.2f; }	

        DrawRect(x, y, w, h, r, gcol, b);
        DrawText(x + 6, y + 4, label, 1, 1, 1);

		gCurrent->cursorY += h + gStyle.ItemSpacingY; // move cursor down for next item	

		// Record last item size
		gCurrent->lastItemW = w;
		gCurrent->lastItemH = h;

		/*std::cout << "Frame " << SpxGui::gFrameCount
			<< " | Button: " << label
			<< " y=" << y
			<< " cursorY(before)=" << gCurrent->cursorY << std::endl;*/

        return clicked;
    }

	//inline bool InputText(const char* label, char* buf, size_t buf_size, float offsetX, float offsetY, float w, float h) {
	inline bool InputText(const char* label, char* buf, size_t buf_size, float w, float h) {
		if (!gCurrent) return false;

		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

		bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
			gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
		bool clicked = (hover && gCurrent->mousePressed);
		// Color change on hover and click
		float r = 0.3f, gcol = 0.3f, b = 0.3f;
		if (hover) { r = 0.4f; gcol = 0.4f; b = 0.6f; }
		if (clicked) { r = 0.2f; gcol = 0.6f; b = 0.2f; }

		DrawRect(x, y, w, h, r, gcol, b);
		DrawText(x + 6, y + 4, buf, 1, 1, 1);
		// For simplicity, we won't handle actual text input here.
		// In a real implementation, you'd capture keyboard input to modify 'buf'.

		gCurrent->cursorY += h + gStyle.ItemSpacingY; // move cursor down for next item

		gCurrent->lastItemW = w;
		gCurrent->lastItemH = h;

		return clicked;
	}
	// simple text output like a label
	inline bool ColoredLalel(float r, float gcol, float b, const char* txt) {
		if (!gCurrent) return false;
		
		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;
		DrawText(x, y, txt, r, gcol, b);

		gCurrent->cursorY += g.fontSize + gStyle.ItemSpacingY; // move cursor down for next item

		// record text size
		gCurrent->lastItemW = (float)strlen(txt) * (g.fontSize * 0.6f); // crude width estimate
		gCurrent->lastItemH = g.fontSize;

		return true;
	}
	// --------------------------------------- Separator and spacing Section ---------------------------------------
	inline bool SeparatorLine(float w) {
		if (!gCurrent) return false;

		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

		DrawRect(x, y, w, 1, 0.5f, 0.5f, 0.5f); // horizontal line

		gCurrent->cursorY += 1 + gStyle.ItemSpacingY; // move cursor down for next item

		

		return true;
	}

	//inline bool SeparatorText(const char* label, float w) {
	inline void SeparatorText(const char* label, float w) {
		//if (!gCurrent) return false;
		if (!gCurrent) return;

		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

		// measure text width
		float xpos = 0, ypos = 0;
		float textWidth = 0;
		for (const char* p = label; *p; p++) {
			if (*p < 32 || *p >= 128) continue;
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(g.cdata, 512, 512, *p - 32, &xpos, &ypos, &q, 1);
			textWidth = q.x1; // last character right edge
		}

		// center text horizontally inside given width
		float textX = x + (w - textWidth) * 0.5f;
		float textY = y;

		// line Y = center aligned with text baseline
		float lineY = textY + g.fontSize * 0.5f;

		// left line
		if (textX - x > 8) {
			DrawRect(x, lineY, textX - x - 8, 1, 0.5f, 0.5f, 0.5f);
		}

		// text
		DrawText(textX, textY, label, 1, 1, 1);

		// right line
		float rightStart = textX + textWidth + 8;
		if (rightStart < x + w) {
			DrawRect(rightStart, lineY, (x + w) - rightStart, 1, 0.5f, 0.5f, 0.5f);
		}

		// advance cursor
		gCurrent->cursorY += g.fontSize + gStyle.ItemSpacingY;

		//return true;
	}
	inline void SameLine(float spacing = -1.0f) {
		if (!gCurrent) return;

		if (spacing < 0.0f) spacing = gStyle.ItemSpacingX;

		// Move cursorX to the right of the last item
		gCurrent->cursorX += gCurrent->lastItemW + spacing;

		// Reset cursorY back to last item top (instead of advancing down)
		gCurrent->cursorY -= gCurrent->lastItemH + gStyle.ItemSpacingY;
	}


	inline void Spacing() {
		if (!gCurrent) return;

		// Advance cursor by one line height (or just spacing if you prefer)
		gCurrent->cursorY += g.fontSize + gStyle.ItemSpacingY;
	}
	// --------------------------------------- Color Picker Section ---------------------------------------
	inline bool ColorBox(const char* label, float* r, float* g, float* b, float w = 40.0f, float h = 20.0f) {
		if (!gCurrent) return false;

		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

		bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
			gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
		bool clicked = (hover && gCurrent->mousePressed);

		// draw color box
		DrawRect(x, y, w, h, *r, *g, *b);

		// optional border
		DrawRect(x - 1, y - 1, w + 2, 1, 0, 0, 0);         // top
		DrawRect(x - 1, y + h, w + 2, 1, 0, 0, 0);         // bottom
		DrawRect(x - 1, y - 1, 1, h + 2, 0, 0, 0);         // left
		DrawRect(x + w, y - 1, 1, h + 2, 0, 0, 0);         // right

		// optional text label
		if (label) {
			DrawText(x + w + 6, y, label, 1, 1, 1);
		}

		// advance cursor
		gCurrent->cursorY += h + gStyle.ItemSpacingY;

		gCurrent->lastItemW = w;
		gCurrent->lastItemH = h;

		return clicked;
	}
	// Error All of the button click hover pos is up the creak 

	inline bool ColorEdit3(const char* label, float col[3]) {
		//if (!gCurrent) return false;
		//float x = gCurrent->cursorX;
		//float y = gCurrent->cursorY;
		//// Draw label
		//DrawText(x, y, label, 1, 1, 1);
		//// Color box
		//float boxSize = g.fontSize; // square box
		//float boxX = x + 100; // fixed offset for simplicity
		//float boxY = y;
		//DrawRect(boxX, boxY, boxSize, boxSize, col[0], col[1], col[2]);
		//bool hover = (gCurrent->mouseX >= boxX && gCurrent->mouseX <= boxX + boxSize &&
		//	gCurrent->mouseY >= boxY && gCurrent->mouseY <= boxY + boxSize);
		//bool clicked = (hover && gCurrent->mousePressed);
		//static bool picking = false;
		//static int pickingComponent = 0; // 0=R,1=G,2=B
		//if (clicked) {
		//	picking = true;
		//	pickingComponent = 0; // start with Red
		//}
		//if (gCurrent->mouseReleased) {
		//	picking = false;
		//}
		//if (picking) {
		//	if (gCurrent->mouseDown) {
		//		// Adjust current component based on mouse X movement
		//		float deltaX = gCurrent->mouseX - (boxX + boxSize * 0.5f);
		//		col[pickingComponent] += deltaX * 0.005f; // sensitivity
		//		if (col[pickingComponent] < 0.0f) col[pickingComponent] = 0.0f;
		//		if (col[pickingComponent] > 1.0f) col[pickingComponent] = 1.0f;
		//	}
		//	else if (gCurrent->mousePressed) {
		//		// Cycle to next component on click
		//		pickingComponent = (pickingComponent + 1) % 3;
		//	}
		//}
		//gCurrent->cursorY += boxSize + gStyle.ItemSpacingY; // move cursor down for next item
		//gCurrent->lastItemW = (boxX + boxSize) - x;
		//gCurrent->lastItemH = boxSize;

		//return clicked || picking;
	}

	// --------------------------------------- Image Section ---------------------------------------

	inline bool ImageButton(const std::string& texPath, float w, float h) {
		if (!gCurrent) return false;

		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

		bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
			gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
		bool clicked = (hover && gCurrent->mousePressed);

		// Load or reuse cached texture
		Image img;
		unsigned int texID = LoadTextuer(texPath, img);

		// Optional hover/click overlay
		if (hover || clicked) {
			float r = hover ? 0.8f : 0.6f;
			float gcol = hover ? 0.8f : 0.6f;
			float b = hover ? 0.8f : 0.6f;
			DrawRect(x - 2, y - 2, w + 4, h + 4, r, gcol, b);
		}

		// Draw the image itself
		if (texID != 0) {
			DrawImage(texID, x, y, w, h);
		}

		// Advance cursor
		gCurrent->cursorY += h + gStyle.ItemSpacingY;

		gCurrent->lastItemW = w;
		gCurrent->lastItemH = h;

		return clicked;
	}
		
	inline bool ImageBox(const std::string texPath, float w, float h, const char* label = nullptr) {
		if (!gCurrent) return false;

		float x = gCurrent->cursorX;
		float y = gCurrent->cursorY;

		bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
			gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
		bool clicked = (hover && gCurrent->mousePressed);

		Image img;
		unsigned int texID = LoadTextuer(texPath, img);
		

		float r = 0.3f, gcol = 0.3f, b = 0.3f;
		if (hover) { r = 0.4f; gcol = 0.4f; b = 0.6f; }
		if (clicked) { r = 0.2f; gcol = 0.6f; b = 0.2f; }
		DrawRect(x - 2, y - 2, w + 4, h + 4, r, gcol, b);
				
		// Image
		if (img.textureID != 0) {
			DrawImage(img.textureID, x, y, w, h);
		}

		// Caption text (optional)
		if (label) {
			float textY = y + h + 4; // 4px gap under image
			DrawText(x, textY, label, 1, 1, 1);
			gCurrent->cursorY = textY + g.fontSize + gStyle.ItemSpacingY;
			gCurrent->lastItemH = h +g.fontSize + 4; // total height including text
		}
		else {
			gCurrent->cursorY += h + gStyle.ItemSpacingY;
			gCurrent->lastItemH = h;
		}
		gCurrent->lastItemW = w;
		return clicked;
	}


} // namespace MyNamespace