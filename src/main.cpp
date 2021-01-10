#define GLFW_STATIC

#include <stdio.h>
#include <cstdio>
#include <iostream>
#include <functional>
#include <thread>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <GLFW/linmath.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_ja_gryph_ranges.cpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "objloader.hpp"

static const struct {
    float x, y;
    float r, g, b;
} vertices[3] = {
    { -0.6f, -0.4f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.f, 1.f, 0.f },
    {   0.f,  0.6f, 0.f, 0.f, 1.f }
};

static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";
 
static const char* fragment_shader_text =
"#version 110\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";


// フレームバッファオブジェクトのサイズ
constexpr int fboWidth(300);  // フレームバッファオブジェクトの幅
constexpr int fboHeight(300); // フレームバッファオブジェクトの高さ

int main() {
    if (!glfwInit()) {
        return -1;
    }

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    FILE * file = fopen("test.obj", "r");
    if( file == NULL ){
        printf("ファイルを開けません!n");
        return false;
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "test", NULL, NULL);
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gl3wInit();

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
 
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
 
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");
 
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) 0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) (sizeof(float) * 2));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.MouseDrawCursor = true;
    io.IniFilename = NULL;
    ImFontConfig config;
    config.MergeMode = true;
    io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF("07YasashisaGothic.ttf", 14.0f, &config, io.Fonts->GetGlyphRangesJapanese());

    //GlobalFlags
    static int GlobalFlag = ImGuiWindowFlags_HorizontalScrollbar;

    GLuint cb;
    glGenTextures(1, &cb);
    glBindTexture(GL_TEXTURE_2D, cb);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLuint rb;
    glGenRenderbuffers(1, &rb);
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fboWidth, fboHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    GLuint fb;
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cb, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        int width, height;
        float speed;
        glfwGetFramebufferSize( window, &width, &height );
        glViewport( 0, 0, width, height );

        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if(ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if(ImGui::MenuItem("Exit")) {
                exit(0);
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
        }

        // Start the Dear ImGui frame
        ImGui::Begin("FPS", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | GlobalFlag);
        ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Begin("Hello ImGui!", NULL, GlobalFlag);
        ImGui::Text("Hello World!!");
        ImGui::End();

        ImGui::Begin("Buttons", NULL, GlobalFlag);
        ImGui::Text("日本語のテスト");
        if (ImGui::Button("test")) {
            ImGui::LogToTTY();
            ImGui::LogText("Hello, world!");
            ImGui::LogFinish();
        };
        if (ImGui::Button("Dark")) {
            ImGui::StyleColorsDark();
        };
        if (ImGui::Button("Light")) {
            ImGui::StyleColorsLight();
        };
        if (ImGui::Button("Classic")) {
            ImGui::StyleColorsClassic();
        };
        ImGui::SliderFloat("speed", &speed, 0.0f, 60.0f);
        ImGui::End();

        ImGui::Begin("GL Test", NULL, GlobalFlag);

        ImGui::Image((void*)(intptr_t)cb, ImVec2(fboWidth, fboHeight), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, display_w, display_h);

        float ratio;
        ::mat4x4 m, p, mvp;
        ratio = width / (float) height;
 
        mat4x4_identity(m);
        mat4x4_rotate_X(m, m, (float) glfwGetTime() * speed);
        mat4x4_rotate_Y(m, m, (float) glfwGetTime() * speed);
        mat4x4_rotate_Z(m, m, (float) glfwGetTime() * speed);

        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_TRUE, (const GLfloat*) mvp);
        glDrawArrays(GL_TRIANGLES, 0, 12*3);

        glBindFramebuffer(GL_FRAMEBUFFER, fb);
        glViewport(0, 0, fboWidth, fboHeight);
        constexpr GLfloat color[]{ 0.0f, 0.0f, 0.0f, 0.0f }, depth(1.0f);
        glClearBufferfv(GL_COLOR, 0, color);
        glClearBufferfv(GL_DEPTH, 0, &depth);



        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;

}