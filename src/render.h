// Last updated: <2024/04/04 10:22:12 +0900>

#ifndef __RENDER_H__
#define __RENDER_H__

#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <wchar.h>

#ifdef _WIN32
// Windows
#include <tchar.h>
#include <windows.h>
#include <scrnsave.h>
#include <mmsystem.h>
#endif

// Windows and Linux
#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "resource.h"

// globals for size of screen
extern int Width;
extern int Height;

// ----------------------------------------
// prototype declaration
void Render(void);
void SetupAnimation(int Width, int Height);
void CleanupAnimation();

// Only used in glfw version
void set_use_waittime(int fg);
void set_cfg_framerate(float fps);
float get_cfg_framerate(void);
void resize_window(int w, int h);

#endif
