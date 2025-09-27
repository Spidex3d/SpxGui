#pragma once
#include "glad\glad.h" // Include glad for OpenGL function loading
#include <vector>
#include <string>
#include "stb\stb_truetype.h" // Include stb_truetype.h for font rendering
#include <iostream>


void TestSpxGui() {
	std::cout << "Hello, SpxGui.h file!" << std::endl;
}

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
uniform int useTex;   // 0 = solid color, 1 = use texture alpha

void main() {
    if (useTex == 1) {
        float alpha = texture(uTex, TexCoord).r;
        FragColor = vec4(uColor, alpha);
    } else {
        FragColor = vec4(uColor, 1.0);
    }
}
)";

namespace SpxGui { // I have never used namespaces before


    // forward declarations
    inline void DrawRect(float x, float y, float w, float h, float r, float g, float b);
    inline void Init(int screenW, int screenH);

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

    struct Context {
        Style style;
        bool inWindow = false;
        int screenW = 800;
        int screenH = 600;
        // simple window info
        float curWinX = 50;
        float curWinY = 50;
        float curWinW = 300;
        float curWinH = 200;
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

    inline Context g;

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
        g.mouseX = mouseX;
        g.mouseY = mouseY;
        g.mouseDown = down;
        g.mousePressed = pressed;
        g.mouseReleased = released;
    }

	// Drawing functions
    inline void Begin(const char* title, bool* p_open = nullptr) {

        g.inWindow = true;
        float btnSize = g.headerHeight - 6.0f;  // padding
        float btnX = g.curWinX + g.curWinW - btnSize - 4.0f;
        float btnY = g.curWinY + 3.0f;


        // Draw background rect
        DrawRect(g.curWinX, g.curWinY, g.curWinW, g.curWinH,g.style.WindowBgR, g.style.WindowBgG, g.style.WindowBgB);
        // Header
        DrawRect(g.curWinX, g.curWinY, g.curWinW, g.headerHeight, g.style.WindowTopBarR, g.style.WindowTopBarG, g.style.WindowTopBarB);
        DrawRect(g.curWinX, g.curWinY, g.curWinW - 280, g.headerHeight, g.style.WindowTopButR, g.style.WindowTopButG, g.style.WindowTopButB);
        DrawRect(btnX, btnY, btnSize, btnSize, g.style.WindowTopButR, g.style.WindowTopButG, g.style.WindowTopButB);

        // Handle dragging
        bool hoverHeader =
            (g.mouseX >= g.curWinX && g.mouseX <= g.curWinX + g.curWinW &&
                g.mouseY >= g.curWinY && g.mouseY <= g.curWinY + g.headerHeight);

        if (hoverHeader && g.mousePressed) {
            g.dragging = true;
            g.dragOffsetX = g.mouseX - g.curWinX;
            g.dragOffsetY = g.mouseY - g.curWinY;
        }
        if (g.mouseReleased) {
            g.dragging = false;
        }
        if (g.dragging && g.mouseDown) {
            g.curWinX = g.mouseX - g.dragOffsetX;
            g.curWinY = g.mouseY - g.dragOffsetY;
        }

		// Draw title text
        DrawText(g.curWinX + 25, g.curWinY + 2, title, 1.0f, 1.0f, 1.0f);

        // -- - Close Button-- -
            if (p_open) {
                               
                // check mouse over
                bool hover = (g.mouseX >= btnX && g.mouseX <= btnX + btnSize &&
                    g.mouseY >= btnY && g.mouseY <= btnY + btnSize);

                // click
                if (hover && g.mousePressed) {
                    *p_open = false; // close the window
                }

                // draw "X" with text (simplest way)
            }
          DrawText(btnX + 5, btnY - 4, "X", 1.0f, 1.0f, 1.0f);
        
    }
	// End the current window
    inline void End() {
        if (!g.inWindow) return;
        g.inWindow = false;
        //std::cout << "[End Window]" << std::endl;
    }

    inline void Text(const char* txt) {
        if (SpxGui::g.inWindow)
            std::cout << "  Text: " << txt << std::endl;
    }

    inline void TextColored(float r, float g, float b, const char* txt) {
        if (SpxGui::g.inWindow)
            std::cout << "  TextColored(" << r << "," << g << "," << b << "): " << txt << std::endl;
    }

    inline Style& GetStyle() { return g.style; }

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