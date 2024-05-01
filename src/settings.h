// Last updated: <2024/04/30 03:56:49 +0900>

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#define _USE_MATH_DEFINES

#ifdef _WIN32
// Windows

#ifdef __GNUC__ 
#if __GNUC__ < 9
// MinGW gcc 6.3.0
// use SHGetSpecialFolderPath()
#define _WIN32_IE 0x0400
#endif
#endif

#include <shlobj.h>
#include <shlwapi.h>
#include <wchar.h>
#include <tchar.h>
#include <windows.h>
#include <scrnsave.h>
#include <mmsystem.h>

#endif

// Windows and Linux
#include <stdlib.h>
#include <math.h>
#include "resource.h"

// setting value
extern int waitValue;
extern int fps_display;

#ifdef _WIN32
// Windows

// config ini file name and directory
#define INIDIR _T("ssisoroadgl")
#define INIFILENAME _T("ssisoroadgl.ini")

// ini file section name
#define SECNAME _T("ssisoroadgl_config")

// ----------------------------------------
// prototype declaration
BOOL getConfigFromIniFile(void);
void writeConfigToIniFile(void);
void getValueFromDialog(HWND hDlg);
void setValueOnDialog(HWND hDlg, int waitValue, int fps_display);

#endif

#endif
