// Last updated: <2024/04/30 03:58:41 +0900>
//
// Config dialog

#include "settings.h"

// setting value
int waitValue = 15;
int fps_display = 1;

// ----------------------------------------
// prototype declaration
BOOL getIniFilePath(TCHAR *filepath);
void initConfigValue(void);
int clamp(int, int, int);

// ========================================
void initConfigValue(void)
{
  waitValue = 15;
  fps_display = 1;
}

// get config value from ini file
BOOL getConfigFromIniFile(void)
{
  TCHAR filepath[MAX_PATH];
  TCHAR cdir[MAX_PATH];
  TCHAR buf[1024];

  if (!getIniFilePath(filepath))
  {
    // not get ini filepath
    initConfigValue();
    return FALSE;
  }

  if (!PathFileExists(filepath))
  {
    // not found ini file. create.
    initConfigValue();
    writeConfigToIniFile();
  }

  if (!PathFileExists(filepath))
    return FALSE;

  // read ini file
  waitValue = GetPrivateProfileInt(SECNAME, _T("wait"), -1, filepath);
  fps_display = GetPrivateProfileInt(SECNAME, _T("fps"), -1, filepath);

  return TRUE;
}

// set config value to ini file
void writeConfigToIniFile(void)
{
  TCHAR filepath[MAX_PATH];
  TCHAR buf[512];

  if (!getIniFilePath(filepath))
    return;

  // create and write ini file
  wsprintf(buf, _T("%d"), waitValue);
  WritePrivateProfileString(SECNAME, _T("wait"), buf, filepath);

  wsprintf(buf, _T("%d"), fps_display);
  WritePrivateProfileString(SECNAME, _T("fps"), buf, filepath);
}

// get value from config dialog
void getValueFromDialog(HWND hDlg)
{
  // get EDITTEXT
  {
    TCHAR buf[2048];
    int n;

    // get wait value
    GetDlgItemText(hDlg, IDC_WAITVALUE, buf, sizeof(buf) / sizeof(TCHAR));
    n = StrToInt(buf);
    waitValue = clamp(n, 5, 200);
  }

  // get CHECKBOX
  fps_display = (IsDlgButtonChecked(hDlg, IDC_FPSDISPLAY) == BST_CHECKED) ? 1 : 0;
}

// set value to config dialog
void setValueOnDialog(HWND hDlg, int waitValue, int fps_display)
{
  // set EDITTEXT
  {
    TCHAR buf[512];

    wsprintf(buf, _T("%d"), waitValue);
    SetDlgItemText(hDlg, IDC_WAITVALUE, buf);
  }

  // set CHECKBOX
  SendMessage(GetDlgItem(hDlg, IDC_FPSDISPLAY), BM_SETCHECK, ((fps_display) ? BST_CHECKED : BST_UNCHECKED), 0);
}

// clamp value
int clamp(int v, int minv, int maxv)
{
  if (v < minv)
    return minv;
  if (v > maxv)
    return maxv;
  return v;
}

BOOL getIniFilePath(TCHAR *filepath)
{
  TCHAR appdataFolderPath[MAX_PATH];
  TCHAR tgtFolderPath[MAX_PATH];
  BOOL createDir = FALSE;

  // get APPDATA folder path
  SHGetSpecialFolderPath(NULL, appdataFolderPath, CSIDL_APPDATA, FALSE);
  if (PathCombine(tgtFolderPath, appdataFolderPath, INIDIR) == NULL)
    return FALSE;

  if (!PathFileExists(tgtFolderPath))
  {
    createDir = TRUE;
  }
  else if (!PathIsDirectory(tgtFolderPath))
  {
    createDir = TRUE;
  }
  if (createDir)
  {
    if (!CreateDirectory(tgtFolderPath, NULL))
      return FALSE;
  }

  if (PathCombine(filepath, tgtFolderPath, INIFILENAME) == NULL)
    return FALSE;

  return TRUE;
}
