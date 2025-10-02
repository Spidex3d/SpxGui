#pragma once
#include "SpxGui.h"
#include <unordered_map>

namespace SpxGui
{

	// we need some defs like SpxTextureID later for images

	//struct SpxButtons
	//{
 //       int SpxButtonID = 0;// Add common widget properties here if needed
	//	SpxVec2 SpxButtonSize = SpxVec2(100, 30); // default size
	//	SpxVec2 SpxButtonPsition = SpxVec2(0, 0); // position relative to window
	//};
	//struct SpxTextBox
	//{
	//	int SpxTextBoxID = 0;// Add common widget properties here if needed
	//};

    // Button(const char* label, const ImVec2& size = ImVec2(0, 0));   // button def
       // this needs to go SpxWidgets.h later
    inline bool Button(const char* label, float offsetX, float offsetY, float w, float h) {
        if (!gCurrent) return false;

        float x = gCurrent->curWinX + offsetX;
        float y = gCurrent->curWinY + offsetY;

        bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
            gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);

        bool clicked = (hover && gCurrent->mousePressed);

        float r = 0.3f, gcol = 0.3f, b = 0.3f;
        if (hover) { r = 0.4f; gcol = 0.4f; b = 0.6f; }
        if (clicked) { r = 0.2f; gcol = 0.6f; b = 0.2f; }

        DrawRect(x, y, w, h, r, gcol, b);
        DrawText(x + 6, y + 4, label, 1, 1, 1);

        return clicked;
    }

	inline bool InputText(const char* label, char* buf, size_t buf_size, float offsetX, float offsetY, float w, float h) {
		if (!gCurrent) return false;
		float x = gCurrent->curWinX + offsetX;
		float y = gCurrent->curWinY + offsetY;
		bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
			gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
		bool clicked = (hover && gCurrent->mousePressed);
		float r = 0.3f, gcol = 0.3f, b = 0.3f;
		if (hover) { r = 0.4f; gcol = 0.4f; b = 0.6f; }
		if (clicked) { r = 0.2f; gcol = 0.6f; b = 0.2f; }
		DrawRect(x, y, w, h, r, gcol, b);
		DrawText(x + 6, y + 4, buf, 1, 1, 1);
		// For simplicity, we won't handle actual text input here.
		// In a real implementation, you'd capture keyboard input to modify 'buf'.
		return clicked;
	}
	inline bool TextColored(float r, float gcol, float b, const char* txt, float offsetX, float offsetY) {
		if (!gCurrent) return false;
		float x = gCurrent->curWinX + offsetX;
		float y = gCurrent->curWinY + offsetY;
		DrawText(x, y, txt, r, gcol, b);
		return true;
	}
	inline bool SeparatorLine(float offsetX, float offsetY, float w) {
		if (!gCurrent) return false;
		float x = gCurrent->curWinX + offsetX;
		float y = gCurrent->curWinY + offsetY;
		DrawRect(x, y + 50, w, 1, 0.5f, 0.5f, 0.5f); // horizontal line
		return true;
	}

    inline bool SeparatorText(const char* label, float offsetX, float offsetY, float w) {
        if (!gCurrent) return false;

        float x = gCurrent->curWinX + offsetX;
        float y = gCurrent->curWinY + offsetY;

        // measure text width
        float xpos = 0, ypos = 0;
        float textWidth = 0;
        for (const char* p = label; *p; p++) {
            if (*p < 32 || *p >= 128) continue;
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(g.cdata, 512, 512, *p - 32, &xpos, &ypos, &q, 1);
            textWidth = q.x1; // last character right edge
        }

        // center text horizontally in the given width
        float textX = x + (w - textWidth) * 0.5f;
        float textY = y;

        // line Y center aligned with text baseline
        float lineY = textY + g.fontSize * 0.5f;

        // left line
        DrawRect(x, lineY, textX - x - 8, 1, 0.5f, 0.5f, 0.5f);

        // text
        DrawText(textX, textY, label, 1, 1, 1);

        // right line
        float rightStart = textX + textWidth + 8;
        DrawRect(rightStart, lineY, (x + w) - rightStart, 1, 0.5f, 0.5f, 0.5f);

        return true;
    }

	//inline int ImageBox(const char* label, const std::string texId, float offsetX, float offsetY, float w, float h) {
	inline int ImageBox(const std::string texId, float offsetX, float offsetY, float w, float h) {
		if (!gCurrent) return false;
		float x = gCurrent->curWinX + offsetX;
		float y = gCurrent->curWinY + offsetY;
		bool hover = (gCurrent->mouseX >= x && gCurrent->mouseX <= x + w &&
			gCurrent->mouseY >= y && gCurrent->mouseY <= y + h);
		bool clicked = (hover && gCurrent->mousePressed);

		static std::unordered_map<std::string, Image> texCache;
		Image& img = texCache[texId]; // get or create image in cache
		if (img.textureID == 0) {
			LoadTextuer(texId, img); // we need to load the texture here for now
		}


		float r = 0.3f, gcol = 0.3f, b = 0.3f;
		if (hover) { r = 0.4f; gcol = 0.4f; b = 0.6f; }
		if (clicked) { r = 0.2f; gcol = 0.6f; b = 0.2f; }
		DrawRect(x - 2, y - 2, w + 4, h + 4, r, gcol, b);
				
		// Image
		if (img.textureID != 0) {
			DrawImage(img.textureID, x, y, w, h);
		}
		return clicked;
	}


} // namespace MyNamespace