// Last updated: <2024/04/30 03:59:18 +0900>
//
// pseudo 3d road screen saver by OpenGL
//
// Original open-source example screensaver by Rachel Grey, lemming@alum.mit.edu.
//
// Use Windows10 x64 22H2 + MinGW (gcc 6.3.0) + scrnsave.h + libscrnsave.a
// by mieki256
// License: CC0 / Public Domain

#define _USE_MATH_DEFINES

// use SHGetSpecialFolderPath()
#define _WIN32_IE 0x0400

#include <shlobj.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <wchar.h>
#include <tchar.h>
#include <windows.h>
#include <scrnsave.h>
#include <math.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "resource.h"
#include "settings.h"
#include "render.h"

// get rid of these warnings: truncation from const double to float conversion from double to float
// #pragma warning(disable: 4305 4244)

// Define a Windows timer
#define ID_TIMER 1
// #define ID_TIMER 32767

static HDC hDC = NULL;
static HGLRC hRC = NULL;
static RECT rect;
static int running = 0;
static UINT uTimer = 0;
static int initfg = 0;

// ----------------------------------------
// prototype declaration
bool InitGL(HWND hWnd, HDC &hDC, HGLRC &hRC);
void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC);
void SetIntervalGL(int interval);

// ========================================
// Screen Saver Procedure
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message,
                               WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_CREATE:
    LoadString(hMainInstance, idsAppName, szAppName, APPNAMEBUFFERLEN);
    LoadString(hMainInstance, idsIniFile, szIniFile, MAXFILELEN);
    GetClientRect(hWnd, &rect); // get window dimensions
    Width = rect.right - rect.left;
    Height = rect.bottom - rect.top;
    hDC = NULL;
    hRC = NULL;
    uTimer = 0;
    running = 0;

    getConfigFromIniFile(); // get config value from ini file

    if (!InitGL(hWnd, hDC, hRC))
    {
      break;
      // return -1;
    }

    SetupAnimation(Width, Height); // initialize work
    SetIntervalGL(1);

    timeBeginPeriod(1);
    uTimer = SetTimer(hWnd, ID_TIMER, waitValue, NULL); // set timer
    initfg = 1;
    break;
    // return 0;

  case WM_DESTROY:
    // KillTimer(hWnd, ID_TIMER);
    KillTimer(hWnd, uTimer);
    timeEndPeriod(1);
    CleanupAnimation(); // cleanup work
    CloseGL(hWnd, hDC, hRC);
    break;
    // return 0;

  case WM_ERASEBKGND:
    break;
    // return 1;

  case WM_SIZE:
    break;
    // return 0;

  case WM_TIMER:
    // main loop
    if (!initfg)
      break;

    if (running == 0)
    {
      running = 1;
      Render(); // animate
      SwapBuffers(hDC);
      running = 0;
    }
    break;
    // return 0;

  default:
    break;
  }

  return DefScreenSaverProc(hWnd, message, wParam, lParam);
}

// ========================================
// screen saver config dialog procedure
BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message,
                                       WPARAM wParam, LPARAM lParam)
{
  // InitCommonControls();
  // would need this for slider bars or other common controls

  switch (message)
  {
  case WM_INITDIALOG:
    // Initialize config dialog
    LoadString(hMainInstance, IDS_DESCRIPTION, szAppName, APPNAMEBUFFERLEN);
    LoadString(hMainInstance, idsIniFile, szIniFile, MAXFILELEN);
    getConfigFromIniFile();
    setValueOnDialog(hDlg, waitValue, fps_display);
    return TRUE;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
      // ok button pressed
      getValueFromDialog(hDlg);
      writeConfigToIniFile();
      EndDialog(hDlg, LOWORD(wParam) == IDOK);
      return TRUE;

    case IDCANCEL:
      // cancel button pressed
      EndDialog(hDlg, LOWORD(wParam) == IDCANCEL);
      return TRUE;

    case IDC_RESET:
      // reset button pressed
      setValueOnDialog(hDlg, 15, 1);
      return TRUE;
    }

    return FALSE;
  } // end command switch

  return FALSE;
}

// ========================================
// needed for SCRNSAVE.LIB (or libscrnsave.a)
BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
  return TRUE;
}

// Initialize OpenGL
bool InitGL(HWND hWnd, HDC &hDC, HGLRC &hRC)
{

#if 0
  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd, sizeof pfd);
  pfd.nSize = sizeof pfd;
  pfd.nVersion = 1;
  // pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL; //blaine's
  // pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW;
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;

  hDC = GetDC(hWnd);

  int i = ChoosePixelFormat(hDC, &pfd);
  SetPixelFormat(hDC, i, &pfd);

  hRC = wglCreateContext(hDC);
  wglMakeCurrent(hDC, hRC);
#else

  hDC = GetDC(hWnd);
  try
  {
    // DWORD dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    DWORD dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

    PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof(PIXELFORMATDESCRIPTOR), // nSize : size of this pfd
            1,                             // nVersion : version number
            dwFlags,                       // dwFlasg
            PFD_TYPE_RGBA,                 // iPixelType : RGBA type
            24,                            // cColorBits : color
            0, 0,                          // cRedBits, cRedShift : Red
            0, 0,                          // cGreenBits, cGreenShift : Green
            0, 0,                          // cBlueBits, cBlueShift : Blue
            0, 0,                          // cAlphaBits, cAlphaShift : Alpha
            0,                             // cAccumBits : accumulation buffer
            0,                             // cAccumRedBits : Red accum bits ignored
            0,                             // cAccumGreenBits : Green accum bits ignored
            0,                             // cAccumBlueBits : Blue accum bits ignored
            0,                             // cAccumAlphaBits : Alpha accum bits ignored
            16,                            // cDepthBits : depth
            0,                             // cStencilBits : stencil buffer
            0,                             // cAuxBuffers : auxiliary buffer
            PFD_MAIN_PLANE,                // iLayerType
            0,                             // bReserved
            0, 0, 0                        // dwLayerMask, dwVisibleMask, dwDamageMask
        };

    int format = ChoosePixelFormat(hDC, &pfd);
    if (format == 0)
    {
      MessageBox(NULL, TEXT("Err_ChoosePixelFormat"), TEXT("Error"), MB_OK);
      throw "Err_ChoosePixelFormat";
    }

    if (!SetPixelFormat(hDC, format, &pfd))
    {
      MessageBox(NULL, TEXT("Err_SetPixelFormat"), TEXT("Error"), MB_OK);
      throw "Err_SetPixelFormat";
    }

    hRC = wglCreateContext(hDC);
    if (!hRC)
    {
      MessageBox(NULL, TEXT("Err_wglCreateContext"), TEXT("Error"), MB_OK);
      throw "Err_wglCreateContext";
    }
  }
  catch (...)
  {
    if (hDC)
    {
      ReleaseDC(hWnd, hDC);
    }
    return false;
  }

  wglMakeCurrent(hDC, hRC);
#endif

  return true;
}

// Shut down OpenGL
void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(hRC);
  ReleaseDC(hWnd, hDC);
}

// OpenGL Vsync enable or disable. Windows only
// 0 : Vsync off, 1 : Vsync on
void SetIntervalGL(int interval)
{
  BOOL(WINAPI * wglSwapIntervalEXT)
  (int) = NULL;
  if (strstr((char *)glGetString(GL_EXTENSIONS), "WGL_EXT_swap_control") == 0)
  {
    // not support
    return;
  }
  else
  {
    wglSwapIntervalEXT = (BOOL(WINAPI *)(int))wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT)
    {
      wglSwapIntervalEXT(interval);
    }
  }
}
