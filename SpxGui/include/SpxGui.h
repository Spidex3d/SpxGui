#pragma once
#include "glad\glad.h" // Include glad for OpenGL function loading
#include <vector>
#include <algorithm>
#include <string>
#include "../vendors/GLwin/include/GLwin.h"
#include "../vendors/GLwin/include/GLwinDefs.h"
#include "stb/stb_image.h" // Include stb_image.h for image loading
#include "stb\stb_truetype.h" // Include stb_truetype.h for font rendering
#include <unordered_map>
#include <iostream>

// Excellent resource for learning OpenGL: https://learnopengl.com/
// Excellent resource for learning modern OpenGL: https://docs.gl/
// And lets not for get YouTub The Cherno for C++ & openGL channel https://www.youtube.com/@TheCherno
// Error to fix if you resize the main window befor you move the gui window you cant move it anymore

// moveing to Retained Mode so we can have multiple windows and widgets with state - focus

// I think we need to move this in to it's own Shader file
const char* vertexSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 TexCoord;

uniform vec2 uScreenSize;

void main() {
    vec2 ndc = (aPos / uScreenSize) * 2.0 - 1.0;
    ndc.y = -ndc.y; // flip Y
    gl_Position = vec4(ndc, 0.0, 1.0);
    TexCoord = aUV;
}
)";

const char* fragmentSrc = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTex;
uniform vec3 uColor;
uniform int useTex;   // 0 = solid, 1 = font alpha, 2 = full image

void main() {
    if (useTex == 0) {
        // solid color
        FragColor = vec4(uColor, 1.0);
    }
     else if (useTex == 1) {
        // font rendering with alpha from texture
        float alpha = texture(uTex, TexCoord).r; // assuming font atlas is in red channel         
        FragColor = vec4(uColor, alpha);
    } else if(useTex == 2) {
        // normal RGBA image    
        FragColor = texture(uTex, TexCoord);
    }
}
)";
namespace SpxGui { // I have never used namespaces before

// forward declarations
inline void DrawRect(float x, float y, float w, float h, float r, float g, float b);
inline void DrawText(float x, float y, const char* txt, float r, float gcol, float b);
inline void Init(int screenW, int screenH);
  
// Text Input related globals
inline std::string gInputChars;
inline uintptr_t activeTextID = 0; // ID of the active text box focus
inline int caretIndex = 0;   // caretIndex cursor position in the active text buffer (caret = carage return)
inline char* activeBuf = nullptr; // later for multiple text boxes


	// Struct for storing style settings
    struct Style {
        float WindowRounding = 0.0f;
        float WindowBgR = 0.15f;
        float WindowBgG = 0.15f;
        float WindowBgB = 0.17f;
        // top bar
		float WindowTopBarR = 0.25f;
		float WindowTopBarG = 0.25f;
		float WindowTopBarB = 0.28f;
        // top x button
        float WindowTopButR = 1.0f;
        float WindowTopButG = 0.25f;
        float WindowTopButB = 0.28f;
		// items like buttons, inputbox, ImageBox etc
        float ItemSpacingX = 8.0f;
        float ItemSpacingY = 6.0f;
        float WindowPaddingX = 10.0f;
        float WindowPaddingY = 28.0f; // leave room for header       
		
    };
	// struct for storing draw commands as we move to Retained Mode
    struct DrawCmd {
        enum Type { RECT, TEXT, IMAGE, CARET } type;
        float x, y, w, h;
        float r, g, b;
        unsigned int texID = 0;   // for IMAGE
        std::string text;         // for TEXT

        // Rect (generic solid rectangle)
        DrawCmd(Type t, float _x, float _y, float _w, float _h,
            float _r, float _g, float _b)
            : type(t), x(_x), y(_y), w(_w), h(_h),
            r(_r), g(_g), b(_b) {
        }

        // Text structer
        DrawCmd(Type t, float _x, float _y,
            float _r, float _g, float _b, const std::string& txt)
            : type(t), x(_x), y(_y),
            r(_r), g(_g), b(_b), text(txt) {
        }

        // Image structer
        DrawCmd(Type t, unsigned int _texID, float _x, float _y, float _w, float _h)
            : type(t), x(_x), y(_y), w(_w), h(_h),
            texID(_texID) {
        }

		// Caret (always 2px wide, height = font size) what I call carridg return - cursor
        DrawCmd(Type t, float _x, float _y, float _h,
            float _r, float _g, float _b)
            : type(t), x(_x), y(_y), w(2.0f), h(_h),
            r(_r), g(_g), b(_b) {
        }
        
    };

	//struct for storage for the main gui window
    struct SpxGuiWindow {
        int SpxGuiWinID = 0;       // ID for multiple windows
        std::string title;
        bool* open = nullptr;
        bool inWindow = false;
		bool isPopup = false; //popup

        // Window position & size
        float curWinX = 50;
        float curWinY = 50;
        float curWinW = 300;
        float curWinH = 400;

        // Widget layout state
        float cursorX = 0.0f;
        float cursorY = 0.0f;
        float lastItemW = 0.0f;
        float lastItemH = 0.0f;

        // Header bar
        float headerHeight = 24.0f;
        bool dragging = false;
        float dragOffsetX = 0, dragOffsetY = 0;

        // Mouse state (copied from global per-frame)
        float mouseX = 0, mouseY = 0;
        bool mouseDown = false;
        bool mousePressed = false;
        bool mouseReleased = false;

		std::vector<DrawCmd> drawList; // for widgets  to add their draw commands

        
		
    };
    // global-only things into a single ContextAddInputChar
    struct Context {
        int screenW = 800;
        int screenH = 600;

        GLuint fontTex = 0;
        stbtt_bakedchar cdata[96];
        float fontSize = 16.0f;

        GLuint gShader = 0;
        GLint uScreenSizeLoc = -1;
        GLint uColorLoc = -1;

        int frameCount = 0;
        int callCount = 0;
    };

	inline std::vector<SpxGuiWindow> gWindows; // later for multiple windows
	inline SpxGuiWindow* gCurrent = nullptr; // later for multiple windows
	inline int gActiveWinID = -1; // later for multiple windows focus
    Style style;

    struct Image {
        GLuint textureID = 0;
        int width = 0;
        int height = 0;
        int ColIndex = 0; // later for multiple colors in one texture
        GLenum i_format; // later for different image formats
    };

	// ------------------------------------------------- Text Input  ----------------------------------------------
   

    inline void AddInputChar(unsigned int c) {

        if (c >= 32 && c < 127) { // only printable ASCII for now
            gInputChars.push_back((char)c);
        }
        if (c == 8) { // backspace
            gInputChars.push_back((char)c);
        }
    }

    inline void AddKeyPress(int key) {
        if (!activeBuf) return; // no active input

        switch (key) {
        case GLWIN_LEFT:
            if (SpxGui::caretIndex > 0) caretIndex--;
            break;
        case GLWIN_RIGHT: {
            int len = (int)strlen(activeBuf);
            if (caretIndex < len) caretIndex++;
            break;
        }
        case GLWIN_HOME:
            caretIndex = 0;
            break;
        case GLWIN_END:
            caretIndex = (int)strlen(activeBuf);
            break;
        }
    }

    // Default callbacks for GLwin integration
    inline void CharCallback(unsigned int codepoint) {
        SpxGui::AddInputChar(codepoint);
    }

    inline void KeyCallback(int key, int action) {
        if (action == GLWIN_PRESS) {
            SpxGui::AddKeyPress(key);
        }
    }
   



	// ----------------------------------------------------------- Test Area -----------------------------------------------------------

	// Use this for X and Y coordinates
	struct SpxVec2 {
		float                                   x, y;
		constexpr SpxVec2()                     : x(0), y(0) {}
		constexpr SpxVec2(float _x, float _y)   : x(_x), y(_y) {}
		float& operator[](size_t idx) {
			if (idx == 0) return x;
			else if (idx == 1) return y;
			throw std::out_of_range("Index out of range for SpxVec2");
		}
        const float& operator[](size_t idx) const {
            if (idx == 0) return x;
            else if (idx == 1) return y;
            throw std::out_of_range("Index out of range for SpxVec2");
        }

    };
	// Use this for RGB colors
	struct SpxVec3 {
	};
	// Use this for RGBA colors and XYWH coordinates
	struct SpxVec4 {
	};
        
	
	// ----------------------------------------------------------- End Test Area -----------------------------------------------------------
    	
   
	// Global variables for SpxGui Shader
    inline GLuint gShader = 0;
    inline GLint uScreenSizeLoc, uColorLoc;

    inline GLuint CompileShader(GLenum type, const char* src) {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, NULL);
        glCompileShader(s);

        GLint success;
        glGetShaderiv(s, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(s, 512, NULL, log);
            std::cerr << "Shader compile error: " << log << std::endl;
        }
        return s;
    }
	// ----------------------------------------------------- Font & Text Rendering -----------------------------------------------------
   // inline Context g;
    inline Context g;
    float CalcTextWidthN(const char* text, int count) {
        float xpos = 0, ypos = 0;
        for (int i = 0; i < count && text[i]; i++) {
            unsigned char c = text[i];
            if (c < 32 || c >= 128) continue;
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(g.cdata, 512, 512, c - 32, &xpos, &ypos, &q, 1);
        }
        return xpos;
    }

    inline bool LoadDefaultFont(const char* path, float size) {
        FILE* f = nullptr;
        errno_t err = fopen_s(&f, path, "rb");
        if (err != 0 || !f) {
            std::cerr << "Failed to open font file: " << path << std::endl;
            return false;
        }

        std::vector<unsigned char> ttfBuffer(1 << 20);
        size_t readBytes = fread(ttfBuffer.data(), 1, ttfBuffer.size(), f);
        fclose(f);
        if (readBytes == 0) {
            std::cerr << "Failed to read font file!\n";
            return false;
        }

        std::vector<unsigned char> tempBitmap(512 * 512);

        int result = stbtt_BakeFontBitmap(
            ttfBuffer.data(), 0, size,
            tempBitmap.data(), 512, 512,
            32, 96, g.cdata
        );

        if (result <= 0) {
            std::cerr << "stbtt_BakeFontBitmap failed!\n";
            return false;
        }

        if (g.fontTex) {
            glDeleteTextures(1, &g.fontTex);
        }

        glGenTextures(1, &g.fontTex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, g.fontTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0,
            GL_RED, GL_UNSIGNED_BYTE, tempBitmap.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        g.fontSize = size;
        return true;
    }

    inline void SetScreenSize(int fbw, int fbh) {
        g.screenW = fbw;
        g.screenH = fbh;
    }

    //inline void Init(int screenW, int screenH) {
    inline void Init() {
    
        std::cout << "[SpxGui::Init] Initializing " << std::endl;

        // --- Compile & link shader ---
        GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexSrc);
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

        gShader = glCreateProgram();
        glAttachShader(gShader, vs);
        glAttachShader(gShader, fs);
        glLinkProgram(gShader);

        GLint linked;
        glGetProgramiv(gShader, GL_LINK_STATUS, &linked);
        if (!linked) {
            char log[512];
            glGetProgramInfoLog(gShader, 512, NULL, log);
            std::cerr << "Shader link error: " << log << std::endl;
        }
        glDeleteShader(vs);
        glDeleteShader(fs);

        uScreenSizeLoc = glGetUniformLocation(gShader, "uScreenSize");
        uColorLoc = glGetUniformLocation(gShader, "uColor");
        std::cout << "uScreenSizeLoc=" << uScreenSizeLoc
            << " uColorLoc=" << uColorLoc << std::endl;

        // --- Load default font ---
        LoadDefaultFont("C:/Windows/Fonts/arial.ttf", 16.0f);

        
        // tell shader which texture unit the font atlas is bound to
        glUseProgram(gShader);
        glUniform1i(glGetUniformLocation(gShader, "uTex"), 0);

		
    }
	// ------------------------------------------------ Images ------------------------------------------------
    // global texture cache load texture only once
	inline std::unordered_map<std::string, Image> GetTextureCache; 
    //GLuint textureID;
        //int width; int height; int ColIndex; // later for multiple colors in one texture
        //int i_formatted; // later for different image formats
    
    inline unsigned int LoadTextuer(const std::string& filePath, Image& img) {
		// check if texture already loaded
		auto it = GetTextureCache.find(filePath);
		if (it != GetTextureCache.end()) {
			img = it->second;
			return img.textureID; // return existing texture ID
		}

		glGenTextures(1, &img.textureID); // Generate a texture ID
        // sets pixel storage modes that affect the operation of subsequent
        // glReadPixels as well as the unpacking of texture patterns
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Add this for safety
		// load and generate the texture
		unsigned char* data = stbi_load(filePath.c_str(), &img.width, &img.height, &img.ColIndex, 0);
        if (data) {
          
			GLWIN_LOG_INFO("Loaded texture: " << filePath
				<< " (" << img.width << " x " << img.height
				<< ", channels= " << img.ColIndex << ")");
        
            if (img.ColIndex == 4) // RGBA png
                img.i_format = GL_RGBA;
            else if (img.ColIndex == 3) // JPG RGB
                img.i_format = GL_RGB;
            else if (img.ColIndex == 1) // greyscale
                img.i_format = GL_RED;
            else
                std::cerr << "Failed to load image (Unknown format): " << filePath << std::endl;

			glBindTexture(GL_TEXTURE_2D, img.textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, img.i_format, img.width, img.height, 0, img.i_format, GL_UNSIGNED_BYTE, data);
            // set the texture wrapping/filtering options (on the currently bound texture object)
			glGenerateMipmap(GL_TEXTURE_2D); // Generate mipmaps "Esential"
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data); // Free image data after uploading to GPU
        }
        else {
			std::cerr << "Failed to load image: " << filePath << std::endl;
			stbi_image_free(data);
        }
        std::cout << "Loading textureID: " << img.textureID << std::endl;
		// save to cache
		GetTextureCache[filePath] = img;
		return img.textureID;
    }

    inline void DrawText(float x, float y, const char* title, float r, float gcol, float b) {
        if (!g.fontTex) return;

        glUseProgram(gShader);
        glUniform1i(glGetUniformLocation(gShader, "useTex"), 1); // textured
        glUniform2f(uScreenSizeLoc, (float)g.screenW, (float)g.screenH);
        glUniform3f(uColorLoc, r, gcol, b);

        glBindTexture(GL_TEXTURE_2D, g.fontTex);

        float xpos = x;
        float ypos = y + g.fontSize;

        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        std::vector<float> verts;

        for (const char* p = title; *p; p++) {
            if (*p < 32 || *p >= 128) continue;
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(g.cdata, 512, 512, *p - 32, &xpos, &ypos, &q, 1);

			float quad[] = { // 6 vertices, each with pos(2) and uv(2)
                q.x0, q.y0, q.s0, q.t0,
                q.x1, q.y0, q.s1, q.t0,
                q.x1, q.y1, q.s1, q.t1,

                q.x0, q.y0, q.s0, q.t0,
                q.x1, q.y1, q.s1, q.t1,
                q.x0, q.y1, q.s0, q.t1
            };
            verts.insert(verts.end(), quad, quad + 24);
        }

        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);

        // position(2), uv(2)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, verts.size() / 4);

        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);

    } 

	// needs to be after all widgets are drawn in main
    inline void NewFrame(float mouseX, float mouseY, bool down, bool pressed, bool released) {
       
        for (auto& w : gWindows) { 
            w.mouseX = mouseX;
            w.mouseY = mouseY;
            w.mouseDown = down;
            w.mousePressed = pressed;
            w.mouseReleased = released;
        }
		gInputChars.clear(); // clear input chars each frame
    }
	Style gStyle; // global style instance


    inline void Begin(const char* title, bool* p_open = nullptr, int SpxGuiWinID = 0) {
        // find or create window
        auto it = std::find_if(gWindows.begin(), gWindows.end(),
            [&](const SpxGuiWindow& w) { return w.SpxGuiWinID == SpxGuiWinID; });

        if (it == gWindows.end()) {
            gWindows.push_back(SpxGuiWindow());
            gWindows.back().SpxGuiWinID = SpxGuiWinID;
            gWindows.back().title = title;
            gWindows.back().open = p_open;
            gCurrent = &gWindows.back();
        }
        else {
            gCurrent = &(*it);
        }

        // if window closed, skip
        if (p_open && !*p_open) {
            gCurrent = nullptr;
            return;
        }

        // mark as used this frame
        gCurrent->inWindow = true;
        gCurrent->title = title;

        // reset layout cursor
        gCurrent->cursorX = gCurrent->curWinX + gStyle.WindowPaddingX;
        gCurrent->cursorY = gCurrent->curWinY + gStyle.WindowPaddingY;

        // check header region
        bool overHeader =
            (gCurrent->mouseX >= gCurrent->curWinX && gCurrent->mouseX <= gCurrent->curWinX + gCurrent->curWinW &&
                gCurrent->mouseY >= gCurrent->curWinY && gCurrent->mouseY <= gCurrent->curWinY + gCurrent->headerHeight);

        // focus / bring to front
        if (overHeader && gCurrent->mousePressed) {
            gActiveWinID = gCurrent->SpxGuiWinID;

            // bring to front
            auto it2 = std::find_if(gWindows.begin(), gWindows.end(),
                [&](const SpxGuiWindow& w) { return w.SpxGuiWinID == gActiveWinID; });
            if (it2 != gWindows.end()) {
                SpxGuiWindow temp = *it2;
                gWindows.erase(it2);
                gWindows.push_back(temp);
                gCurrent = &gWindows.back();
            }

            // start dragging
            gCurrent->dragging = true;
            gCurrent->dragOffsetX = gCurrent->mouseX - gCurrent->curWinX;
            gCurrent->dragOffsetY = gCurrent->mouseY - gCurrent->curWinY;
        }

        // stop dragging
        if (gCurrent->mouseReleased) {
            gCurrent->dragging = false;
        }

        // apply dragging movement
        if (gCurrent->dragging && gCurrent->mouseDown) {
            gCurrent->curWinX = gCurrent->mouseX - gCurrent->dragOffsetX;
            gCurrent->curWinY = gCurrent->mouseY - gCurrent->dragOffsetY;
        }
    }


	// End the current window
    inline void End() {
        if (!gCurrent) return;
        gCurrent->inWindow = false;
		gCurrent = nullptr;
        //std::cout << "[End Window]" << std::endl;
    }

    //// Drawing functions Popup window 
    //inline void BeginPopUp(const char* title, bool* p_open = nullptr, int SpxGuiWinID = 0) {
    //    // find or create window
    //    auto it = std::find_if(gWindows.begin(), gWindows.end(),
    //        [&](const SpxGuiWindow& w) { return w.SpxGuiWinID == SpxGuiWinID; });

    //    if (it == gWindows.end()) {
    //        gWindows.push_back(SpxGuiWindow());
    //        gWindows.back().SpxGuiWinID = SpxGuiWinID;
    //        gWindows.back().title = title;
    //        gWindows.back().open = p_open;
    //        gCurrent = &gWindows.back();
    //    }
    //    else {
    //        gCurrent = &(*it);
    //    }

    //    // if window closed, skip
    //    if (p_open && !*p_open) {
    //        gCurrent = nullptr;
    //        return;
    //    }

    //    // mark as used this frame
    //    gCurrent->inWindow = true;
    //    gCurrent->title = title;

    //    // reset layout cursor
    //    gCurrent->cursorX = gCurrent->curWinX + gStyle.WindowPaddingX;
    //    gCurrent->cursorY = gCurrent->curWinY + gStyle.WindowPaddingY;

    //    // check header region
    //    bool overHeader =
    //        (gCurrent->mouseX >= gCurrent->curWinX && gCurrent->mouseX <= gCurrent->curWinX + gCurrent->curWinW &&
    //            gCurrent->mouseY >= gCurrent->curWinY && gCurrent->mouseY <= gCurrent->curWinY + gCurrent->headerHeight);

    //    // focus / bring to front
    //    if (overHeader && gCurrent->mousePressed) {
    //        gActiveWinID = gCurrent->SpxGuiWinID;

    //        // bring to front
    //        auto it2 = std::find_if(gWindows.begin(), gWindows.end(),
    //            [&](const SpxGuiWindow& w) { return w.SpxGuiWinID == gActiveWinID; });
    //        if (it2 != gWindows.end()) {
    //            SpxGuiWindow temp = *it2;
    //            gWindows.erase(it2);
    //            gWindows.push_back(temp);
    //            gCurrent = &gWindows.back();
    //        }

    //        // start dragging
    //        gCurrent->dragging = true;
    //        gCurrent->dragOffsetX = gCurrent->mouseX - gCurrent->curWinX;
    //        gCurrent->dragOffsetY = gCurrent->mouseY - gCurrent->curWinY;
    //    }

    //    // stop dragging
    //    if (gCurrent->mouseReleased) {
    //        gCurrent->dragging = false;
    //    }

    //    // apply dragging movement
    //    if (gCurrent->dragging && gCurrent->mouseDown) {
    //        gCurrent->curWinX = gCurrent->mouseX - gCurrent->dragOffsetX;
    //        gCurrent->curWinY = gCurrent->mouseY - gCurrent->dragOffsetY;
    //    }

    //} // End draw pop up

    //inline void EndPopUp() {
    //    if (!gCurrent) return;
    //    gCurrent->inWindow = false;
    //    gCurrent = nullptr;
    //    //std::cout << "[End Window]" << std::endl;
    //}
    // Drawing functions for a popup-style window (no X button)
    inline void BeginPopUp(const char* title, bool* p_open = nullptr, int SpxGuiWinID = 0) {
        // find or create
        auto it = std::find_if(gWindows.begin(), gWindows.end(),
            [&](const SpxGuiWindow& w) { return w.SpxGuiWinID == SpxGuiWinID; });

        if (it == gWindows.end()) {
            gWindows.push_back(SpxGuiWindow());
            gWindows.back().SpxGuiWinID = SpxGuiWinID;
            gWindows.back().title = title;
            gWindows.back().open = p_open;
            gWindows.back().isPopup = true;   // mark as popup
            gCurrent = &gWindows.back();
        }
        else {
            gCurrent = &(*it);
           // gCurrent->isPopup = true; // ensure popup mode if reused
        }

        if (p_open && !*p_open) {
            gCurrent = nullptr;
            return;
        }

        gCurrent->inWindow = true;
        gCurrent->title = title;

        gCurrent->cursorX = gCurrent->curWinX + gStyle.WindowPaddingX;
        gCurrent->cursorY = gCurrent->curWinY + gStyle.WindowPaddingY;

        //  close if clicked outside popup bounds
        if (gCurrent->mousePressed) {
            bool inside =
                (gCurrent->mouseX >= gCurrent->curWinX &&
                    gCurrent->mouseX <= gCurrent->curWinX + gCurrent->curWinW &&
                    gCurrent->mouseY >= gCurrent->curWinY &&
                    gCurrent->mouseY <= gCurrent->curWinY + gCurrent->curWinH);

            if (!inside) {
                if (p_open) *p_open = false;   // close
            }
        }

        // same dragging code as before…
        bool overHeader =
            (gCurrent->mouseX >= gCurrent->curWinX && gCurrent->mouseX <= gCurrent->curWinX + gCurrent->curWinW &&
                gCurrent->mouseY >= gCurrent->curWinY && gCurrent->mouseY <= gCurrent->curWinY + gCurrent->headerHeight);

        if (overHeader && gCurrent->mousePressed) {
            gActiveWinID = gCurrent->SpxGuiWinID;
            auto it2 = std::find_if(gWindows.begin(), gWindows.end(),
                [&](const SpxGuiWindow& w) { return w.SpxGuiWinID == gActiveWinID; });
            if (it2 != gWindows.end()) {
                SpxGuiWindow temp = *it2;
                gWindows.erase(it2);
                gWindows.push_back(temp);
                gCurrent = &gWindows.back();
            }

            gCurrent->dragging = true;
            gCurrent->dragOffsetX = gCurrent->mouseX - gCurrent->curWinX;
            gCurrent->dragOffsetY = gCurrent->mouseY - gCurrent->curWinY;
        }
        if (gCurrent->mouseReleased) gCurrent->dragging = false;
        if (gCurrent->dragging && gCurrent->mouseDown) {
            gCurrent->curWinX = gCurrent->mouseX - gCurrent->dragOffsetX;
            gCurrent->curWinY = gCurrent->mouseY - gCurrent->dragOffsetY;
        }
    }

    inline void EndPopUp() {
        if (!gCurrent) return;
        gCurrent->inWindow = false;
        gCurrent = nullptr;
    }


    inline Style& GetStyle() { return style; }

    inline void DrawRect(float x, float y, float w, float h, float r, float g, float b) {
        float verts[] = {
            x,       y,
            x + w,   y,
            x + w,   y + h,

            x,       y,
            x + w,   y + h,
            x,       y + h
        };

        static GLuint vao = 0, vbo = 0;
        if (!vao) {
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
        }

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);       

        glUseProgram(gShader);
        glUniform1i(glGetUniformLocation(gShader, "useTex"), 0); // solid
        glUniform2f(uScreenSizeLoc, SpxGui::g.screenW, SpxGui::g.screenH);
        glUniform3f(uColorLoc, r, g, b);

        glDrawArrays(GL_TRIANGLES, 0, 6);
      

        glUniform1i(glGetUniformLocation(gShader, "useTex"), 0); // reset

    }
	// use for drawing images in the gui
    inline void DrawImage(unsigned int texID, float x, float y, float w, float h) {
        glUseProgram(gShader);
        glUniform2f(uScreenSizeLoc, (float)g.screenW, (float)g.screenH);
        glUniform1i(glGetUniformLocation(gShader, "useTex"), 2);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);      
        glUniform1i(glGetUniformLocation(gShader, "uTex"), 0); // sampler = unit 0

        float verts[] = {
            // pos        // uv
            x,     y,     0.0f, 0.0f,
            x + w, y,     1.0f, 0.0f,
            x + w, y + h, 1.0f, 1.0f,

            x,     y,     0.0f, 0.0f,
            x + w, y + h, 1.0f, 1.0f,
            x,     y + h, 0.0f, 1.0f,
        };

        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, 6);      

        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);

        glUniform1i(glGetUniformLocation(gShader, "useTex"), 0); // reset
    }   

    inline void Render() {
        for (auto& w : gWindows) {
            if (w.open && !*w.open) continue;

            // 1. Window background
            DrawRect(w.curWinX, w.curWinY, w.curWinW, w.curWinH,
                style.WindowBgR, style.WindowBgG, style.WindowBgB);

            // 2. Header color logic
            bool isActive = (w.SpxGuiWinID == gActiveWinID);

            float hr = style.WindowTopBarR;
            float hg = style.WindowTopBarG;
            float hb = style.WindowTopBarB;

            if (w.isPopup) {
                // Popups: header matches body color
                hr = style.WindowBgR;
                hg = style.WindowBgG;
                hb = style.WindowBgB;
            }

            // Brighten header if active
            if (isActive) {
                hr = (hr + 0.1f > 1.0f) ? 1.0f : hr + 0.1f;
                hg = (hg + 0.1f > 1.0f) ? 1.0f : hg + 0.1f;
                hb = (hb + 0.4f > 1.0f) ? 1.0f : hb + 0.4f;
            }

            // Draw header bar
            DrawRect(w.curWinX, w.curWinY, w.curWinW, w.headerHeight, hr, hg, hb);

            // 3. Close button (normal windows only)
            if (!w.isPopup && w.open) {
                float btnSize = w.headerHeight - 6.0f;
                float btnX = w.curWinX + w.curWinW - btnSize - 4.0f;
                float btnY = w.curWinY + 3.0f;

                bool hover = (w.mouseX >= btnX && w.mouseX <= btnX + btnSize &&
                    w.mouseY >= btnY && w.mouseY <= btnY + btnSize);

                float cr = style.WindowTopButR;
                float cg = style.WindowTopButG;
                float cb = style.WindowTopButB;
                if (hover) { cr = 0.8f; cg = 0.2f; cb = 0.2f; }

                DrawRect(btnX, btnY, btnSize, btnSize, cr, cg, cb);
                DrawText(btnX + 4, btnY - 2, "X", 1, 1, 1);

                if (hover && w.mousePressed) {
                    *w.open = false;
                }
            }

            // 4. Title text
            DrawText(w.curWinX + 8, w.curWinY + 4, w.title.c_str(), 1, 1, 1);

            // 5. Draw recorded widget commands
            for (auto& cmd : w.drawList) {
                switch (cmd.type) {
                case DrawCmd::RECT:
                    DrawRect(cmd.x, cmd.y, cmd.w, cmd.h, cmd.r, cmd.g, cmd.b);
                    break;
                case DrawCmd::TEXT:
                    DrawText(cmd.x, cmd.y, cmd.text.c_str(), cmd.r, cmd.g, cmd.b);
                    break;
                case DrawCmd::IMAGE:
                    DrawImage(cmd.texID, cmd.x, cmd.y, cmd.w, cmd.h);
                    break;
                case DrawCmd::CARET:
                    DrawRect(cmd.x, cmd.y, cmd.w, cmd.h, cmd.r, cmd.g, cmd.b);
                    break;
                }
            }

            // Clear drawList for next frame
            w.drawList.clear();
        }
    }


    
} // namespace SpxGui




