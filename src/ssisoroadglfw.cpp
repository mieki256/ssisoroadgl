// Last updated: <2024/04/30 04:58:20 +0900>
//
// Draw isometric roads by OpenGL + glfw
//
// F key : Change framerate 60, 30, 20 FPS
// T key : Toggle FPS display
// ESC or Q key : exit
//
// Windows10 x64 22H2 + MSYS2 MinGW 64bit (g++ 13.2.0) + glfw 3.4.1
// by mieki256
// License: CC0 / Public Domain

#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include "render.h"

// #if 0
#ifdef _WIN32
// Windows
#include <windows.h>
#include <mmsystem.h>
#define WINMM_TIMER
#else
// Linux
#undef WINMM_TIMER
#endif

// window title
#define WDW_TITLE "Draw isometric roads"

// window size
#define SCRW 1280
#define SCRH 720

// setting value
int waitValue = 15;
int fps_display = 1;

// ----------------------------------------
// prototype declaration
int main(void);
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void resize(GLFWwindow *window, int w, int h);
void error_callback(int error, const char *description);
void errmsg(const char *description);
void error_exit(const char *description);

// ----------------------------------------
// Main
int main(void)
{
    GLFWwindow *window;

    Width = SCRW;
    Height = SCRH;

#ifdef WINMM_TIMER
    timeBeginPeriod(1);
#endif

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        // Initialization failed
        errmsg("Could not initialize GLFW3");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1); // set OpenGL 1.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // create window
    window = glfwCreateWindow(Width, Height, WDW_TITLE, NULL, NULL);
    if (!window)
    {
        // Window or OpenGL context creation failed
        errmsg("Could not create an window");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, resize);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    SetupAnimation(Width, Height);
    set_cfg_framerate(60.0);
    set_use_waittime(1);

    // main loop
    while (!glfwWindowShouldClose(window))
    {
        Render();
        // glFlush();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

#ifdef WINMM_TIMER
    timeEndPeriod(1);
#endif

    CleanupAnimation();

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

// ----------------------------------------
// Error callback
void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void errmsg(const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void error_exit(const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
    glfwTerminate();
    exit(EXIT_FAILURE);
}

// ----------------------------------------
// window resize callback
static void resize(GLFWwindow *window, int w, int h)
{
    if (h == 0)
        return;

    glfwSetWindowSize(window, w, h);
    glfwSwapInterval(1);
    resize_window(w, h);
}

// ----------------------------------------
// Key callback
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)
        {
            // ESC or Q key to exit
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        else if (key == GLFW_KEY_T)
        {
            fps_display = (fps_display + 1) % 2;
        }
        else if (key == GLFW_KEY_F)
        {
            float fps = get_cfg_framerate();
            if (fps == 60.0)
            {
                set_cfg_framerate(30.0);
            }
            else if (fps == 30.0)
            {
                set_cfg_framerate(20.0);
            }
            else if (fps == 20.0)
            {
                set_cfg_framerate(60.0);
            }
        }
    }
}
