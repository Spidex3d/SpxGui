#pragma once
#include "glad\glad.h" // Include glad for OpenGL function loading
#include <vector>
#include <algorithm>
#include <string>
#include "../vendors/GLwin/include/GLwin.h"
#include "stb/stb_image.h" // Include stb_image.h for image loading
#include "stb\stb_truetype.h" // Include stb_truetype.h for font rendering
#include <iostream>


// Error to fix if you resize the main window befor you move the gui window you cant move it anymore

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
    };
	//struct for storage for the main gui window
    struct SpxGuiWindow {
        
		int SpxGuiWinID = 0; // later for multiple windows and so the button knows which window its in 
        std::string title;
		bool* open = nullptr; // later for multiple windows
        bool inWindow = false;

		int screenW = 800; // GLwinGetScreenSizeW(window, w); New functions to add we need the screen size from GLwin
		int screenH = 600; // GLwinGetScreenSizeH(window, h); New functions to add
        float curWinX = 50;
        float curWinY = 50;
        float curWinW = 300;
        float curWinH = 400;
        // header bar
        float headerHeight = 24.0f;
        bool dragging = false;
        float dragOffsetX = 0, dragOffsetY = 0;
        // mouse state
        float mouseX = 0, mouseY = 0;
        bool mouseDown = false;
        bool mousePressed = false;
        bool mouseReleased = false;
        // font data
        GLuint fontTex = 0;
        stbtt_bakedchar cdata[96]; // ASCII 32..126
        float fontSize = 16.0f;
    };
	// ----------------------------------------------------------- Test Area -----------------------------------------------------------
	inline std::vector<SpxGuiWindow> gWindows; // later for multiple windows
	inline SpxGuiWindow* gCurrent = nullptr; // later for multiple windows
    Style style;

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

	struct Image {
		GLuint textureID = 0;
		int width = 0 ;
		int height = 0;
		int ColIndex = 0; // later for multiple colors in one texture
		GLenum i_format; // later for different image formats
	};
   
	// ----------------------------------------------------------- End Test Area -----------------------------------------------------------
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

   // inline Context g;
    inline SpxGuiWindow g;

    inline void Init(int screenW, int screenH) {
    
        std::cout << "[SpxGui::Init] Initializing with "
            << screenW << "x" << screenH << std::endl;

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

		// --- Load font from file --- this needs to be an option setting later
        FILE* f = nullptr;
        errno_t err = fopen_s(&f, "C:/Windows/Fonts/arial.ttf", "rb"); // adjust path if needed
        //errno_t err = fopen_s(&f, "C:/Windows/Fonts/comic.ttf", "rb"); // adjust path if needed
        if (err != 0 || !f) {
            std::cerr << "Failed to open font file!" << std::endl;
            return;
        }

        // allocate on heap (safer than big stack arrays)
        std::vector<unsigned char> ttfBuffer(1 << 20);
        size_t readBytes = fread(ttfBuffer.data(), 1, ttfBuffer.size(), f);
        fclose(f);

        if (readBytes == 0) {
            std::cerr << "Failed to read font file!" << std::endl;
            return;
        }

        std::vector<unsigned char> tempBitmap(512 * 512);

        int result = stbtt_BakeFontBitmap(
            ttfBuffer.data(), 0, g.fontSize,
            tempBitmap.data(), 512, 512,
            32, 96, g.cdata
        );

        if (result <= 0) {
            std::cerr << "stbtt_BakeFontBitmap failed!" << std::endl;
            return;
        }
        glGenTextures(1, &g.fontTex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, g.fontTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0,
            GL_RED, GL_UNSIGNED_BYTE, tempBitmap.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // tell shader which texture unit the font atlas is bound to
        glUseProgram(gShader);
        glUniform1i(glGetUniformLocation(gShader, "uTex"), 0);
        
    }
	// ------------------------------------------------ Images ------------------------------------------------
    //GLuint textureID;
        //int width; int height; int ColIndex; // later for multiple colors in one texture
        //int i_formatted; // later for different image formats
    
    inline unsigned int LoadTextuer(const std::string& filePath, Image& img) {
        
		glGenTextures(1, &img.textureID);
		
		// load and generate the texture
		unsigned char* data = stbi_load(filePath.c_str(), &img.width, &img.height, &img.ColIndex, 0);
        
        if (data) {
            std::cout << "Loaded texture: " << filePath
                << " (" << img.width << " x " << img.height
                << ", channels= " << img.ColIndex << ")\n";
        
			if (img.ColIndex == 4)
				img.i_format = GL_RGBA;
			else if (img.ColIndex == 3)
				img.i_format = GL_RGB;
			else if (img.ColIndex == 1)
				img.i_format = GL_RED;
			else
				std::cerr << "Failed to load image (Unknown format): " << filePath << std::endl;

			glBindTexture(GL_TEXTURE_2D, img.textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, img.i_format, img.width, img.height, 0, img.i_format, GL_UNSIGNED_BYTE, data);
            // set the texture wrapping/filtering options (on the currently bound texture object)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
        }
        else {
			std::cerr << "Failed to load image: " << filePath << std::endl;
			stbi_image_free(data);
        }
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

            float quad[] = {
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

    inline void NewFrame(float mouseX, float mouseY, bool down, bool pressed, bool released) {
        for (auto& w : gWindows) {
            w.mouseX = mouseX;
            w.mouseY = mouseY;
            w.mouseDown = down;
            w.mousePressed = pressed;
            w.mouseReleased = released;
        }
    }
    
	// Drawing functions default window 
    inline void Begin(const char* title, bool* p_open = nullptr, int SpxGuiWinID = 0) {

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
			gCurrent->title = title; // update title
			gCurrent->open = p_open; // update open pointer
                       
        }

        if (p_open && !p_open) {
            gCurrent = nullptr;
            return;
        }       

		gCurrent->inWindow = true;  

        // background
        DrawRect(gCurrent->curWinX, gCurrent->curWinY, gCurrent->curWinW, gCurrent->curWinH,
            style.WindowBgR, style.WindowBgG, style.WindowBgB);

        // header
        DrawRect(gCurrent->curWinX, gCurrent->curWinY, gCurrent->curWinW, gCurrent->headerHeight,
            style.WindowTopBarR, style.WindowTopBarG, style.WindowTopBarB);

        if (p_open) {
            float btnSize = gCurrent->headerHeight - 6.0f;
            float btnX = gCurrent->curWinX + gCurrent->curWinW - btnSize - 4.0f;
            float btnY = gCurrent->curWinY + 3.0f;

            bool hover = (gCurrent->mouseX >= btnX && gCurrent->mouseX <= btnX + btnSize &&
                gCurrent->mouseY >= btnY && gCurrent->mouseY <= btnY + btnSize);

            float cr = style.WindowTopButR, cg = style.WindowTopButG, cb = style.WindowTopButB;
            if (hover) { cr = 0.8f; cg = 0.2f; cb = 0.2f; }

            DrawRect(btnX, btnY, btnSize, btnSize, cr, cg, cb);
            DrawText(btnX + 4, btnY - 2, "X", 1, 1, 1);

            if (hover && gCurrent->mousePressed) {
                *p_open = false;
            }
        }

        // Handle dragging
        bool overHeader =
            (gCurrent->mouseX >= gCurrent->curWinX && gCurrent->mouseX <= gCurrent->curWinX + gCurrent->curWinW &&
                gCurrent->mouseY >= gCurrent->curWinY && gCurrent->mouseY <= gCurrent->curWinY + gCurrent->headerHeight);

        if (overHeader && gCurrent->mousePressed) {
            gCurrent->dragging = true;
            gCurrent->dragOffsetX = gCurrent->mouseX - gCurrent->curWinX;
            gCurrent->dragOffsetY = gCurrent->mouseY - gCurrent->curWinY;
        }
        if (gCurrent->mouseReleased) gCurrent->dragging = false;
        if (gCurrent->dragging && gCurrent->mouseDown) {
            gCurrent->curWinX = gCurrent->mouseX - gCurrent->dragOffsetX;
            gCurrent->curWinY = gCurrent->mouseY - gCurrent->dragOffsetY;
        }

		// Draw title text
        DrawText(gCurrent->curWinX + 8, gCurrent->curWinY + 4, title, 1, 1, 1);	 
        
    }
	// End the current window
    inline void End() {
        if (!gCurrent) return;
        gCurrent->inWindow = false;
		gCurrent = nullptr;
        //std::cout << "[End Window]" << std::endl;
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
        // later: upload draw data to GPU
        std::cout << "[Render GUI]" << std::endl;
    }

} // namespace SpxGui




//g.inWindow = true;
//
//// --- Close button position ---
//float btnSize = g.headerHeight - 6.0f;
//float btnX = g.curWinX + g.curWinW - btnSize - 4.0f;
//float btnY = g.curWinY + 3.0f;
//
//// --- Draw window background ---
//DrawRect(g.curWinX, g.curWinY, g.curWinW, g.curWinH,
//    g.style.WindowBgR, g.style.WindowBgG, g.style.WindowBgB);
//
//// --- Header bar ---
//DrawRect(g.curWinX, g.curWinY, g.curWinW, g.headerHeight,
//    g.style.WindowTopBarR, g.style.WindowTopBarG, g.style.WindowTopBarB);
//
//// --- Handle dragging (only if not over close button) ---
//bool overHeader =
//(g.mouseX >= g.curWinX && g.mouseX <= g.curWinX + g.curWinW &&
//    g.mouseY >= g.curWinY && g.mouseY <= g.curWinY + g.headerHeight);
//
//bool overClose =
//(g.mouseX >= btnX && g.mouseX <= btnX + btnSize &&
//    g.mouseY >= btnY && g.mouseY <= btnY + btnSize);
//
//if (overHeader && !overClose && g.mousePressed) {
//    g.dragging = true;
//    g.dragOffsetX = g.mouseX - g.curWinX;
//    g.dragOffsetY = g.mouseY - g.curWinY;
//}
//if (g.mouseReleased) {
//    g.dragging = false;
//}
//if (g.dragging && g.mouseDown) {
//    g.curWinX = g.mouseX - g.dragOffsetX;
//    g.curWinY = g.mouseY - g.dragOffsetY;
//}
//
//// --- Draw title text ---
//DrawText(g.curWinX + 25, g.curWinY + 4, title, 1.0f, 1.0f, 1.0f);
//
//// --- Close Button ---
//if (p_open) {
//    // Change color if hovered
//    float cr = g.style.WindowTopButR;
//    float cg = g.style.WindowTopButG;
//    float cb = g.style.WindowTopButB;
//    if (overClose) { cr = 0.8f; cg = 0.2f; cb = 0.2f; }
//
//    DrawRect(btnX, btnY, btnSize, btnSize, cr, cg, cb);
//
//    // Click ? close window
//    if (overClose && g.mousePressed) {
//        *p_open = false;
//    }
//
//    // Draw "X"
//    DrawText(btnX + 5, btnY - 2, "X", 1.0f, 1.0f, 1.0f);
//}