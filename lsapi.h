#if !defined(LSAPI_H)
#define LSAPI_H

#include <windows.h>


// Preprocessor Definitions
#define LSAPI EXTERN_C DECLSPEC_IMPORT

// General Constants
#define MAX_LINE_LENGTH 4096
#define MAGIC_DWORD     0x49474541

// LM_RECYCLE
#define LR_RECYCLE    0
#define LR_LOGOFF     1
#define LR_QUIT       2
#define LR_MSSHUTDOWN 3

// LM_RELOADMODULE/LM_UNLOADMODULE
#define LMM_HINSTANCE 0x1000

// EnumLSData
#define ELD_BANGS   0
#define ELD_MODULES 1
#define ELD_REVIDS  2

// EnumModulesProc
#define LS_MODULE_THREADED 0x0001

// is_valid_pattern
#define PATTERN_VALID 0
#define PATTERN_ESC   -1
#define PATTERN_RANGE -2
#define PATTERN_CLOSE -3
#define PATTERN_EMPTY -4

// matche
#define MATCH_VALID   1
#define MATCH_END     2
#define MATCH_ABORT   3
#define MATCH_RANGE   4
#define MATCH_LITERAL 5
#define MATCH_PATTERN 6

// LM_BANGCOMMAND
#define MAX_BANGCOMMAND 64
#define MAX_BANGARGS    256

typedef struct LMBANGCOMMAND {
    UINT cbSize;
    HWND hWnd;
    CHAR szCommand[MAX_BANGCOMMAND];
    CHAR szArgs[MAX_BANGARGS];
} LMBANGCOMMAND, *LPLMBANGCOMMAND;

// LM_SYSTRAY
#define TRAY_MAX_TIP_LENGTH       128
#define TRAY_MAX_INFO_LENGTH      256
#define TRAY_MAX_INFOTITLE_LENGTH 64

typedef struct LSNOTIFYICONDATA {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    char szTip[TRAY_MAX_TIP_LENGTH];
    DWORD dwState;
    DWORD dwStateMask;
    char szInfo[TRAY_MAX_INFO_LENGTH];
    union {
        UINT uTimeout;
        UINT uVersion;
    } DUMMYUNIONNAME;
    char szInfoTitle[TRAY_MAX_INFOTITLE_LENGTH];
    DWORD dwInfoFlags;
    GUID guidItem;
} LSNOTIFYICONDATA, *LPLSNOTIFYICONDATA;

// Messages
#define LM_SHUTDOWN              8889
#define LM_SAVEDATA              8892
#define LM_RESTOREDATA           8893
#define LM_SYSTRAY               9214
#define LM_SYSTRAYREADY          9215
#define LM_RECYCLE               9260
#define LM_REGISTERMESSAGE       9263
#define LM_UNREGISTERMESSAGE     9264
#define LM_GETREVID              9265
#define LM_UNLOADMODULE          9266
#define LM_RELOADMODULE          9267
#define LM_REGISTERHOOKMESSAGE   9268
#define LM_UNREGISTERHOOKMESSAGE 9269
#define LM_REFRESH               9305
#define LM_BANGCOMMAND           9420
#define LM_WINDOWCREATED         9501
#define LM_WINDOWDESTROYED       9502
#define LM_ACTIVATESHELLWINDOW   9503
#define LM_WINDOWACTIVATED       9504
#define LM_GETMINRECT            9505
#define LM_REDRAW                9506
#define LM_TASKMAN               9507
#define LM_LANGUAGE              9508
#define LM_ACCESSIBILITYSTATE    9511
#define LM_APPCOMMAND            9512

// Callback Functions
typedef void (*BANGCOMMANDPROC)(HWND hwndOwner, LPCSTR pszArgs);
typedef void (*BANGCOMMANDPROCEX)(HWND hwndOwner, LPCSTR pszBangCommandName, LPCSTR pszArgs);
typedef BOOL (CALLBACK *ENUMBANGCOMMANDSPROC)(LPCSTR pszBangCommandName, LPARAM lParam);
typedef BOOL (CALLBACK *ENUMMODULESPROC)(LPCSTR pszPath, DWORD fdwFlags, LPARAM lParam);
typedef BOOL (CALLBACK *ENUMREVIDSPROC)(LPCSTR pszRevID, LPARAM lParam);

// Old stuff
LSAPI int LSGetSystemMetrics(int);
LSAPI HMONITOR LSMonitorFromWindow(HWND, DWORD);
LSAPI HMONITOR LSMonitorFromRect(LPCRECT, DWORD);
LSAPI HMONITOR LSMonitorFromPoint(POINT, DWORD);
LSAPI BOOL LSGetMonitorInfo(HMONITOR, LPMONITORINFO);
LSAPI BOOL LSEnumDisplayMonitors(HDC, LPCRECT, MONITORENUMPROC, LPARAM);
LSAPI BOOL LSEnumDisplayDevices(PVOID, DWORD, PDISPLAY_DEVICE, DWORD);
LSAPI BOOL WINAPI LSLog(int nLevel, LPCSTR pszModule, LPCSTR pszMessage);
LSAPI BOOL WINAPIV LSLogPrintf(int nLevel, LPCSTR pszModule, LPCSTR pszFormat, ...);

// Functions
LSAPI BOOL AddBangCommand(LPCSTR pszBangCommandName, BANGCOMMANDPROC pfnCallback);
LSAPI BOOL AddBangCommandEx(LPCSTR pszBangCommandName, BANGCOMMANDPROCEX pfnCallback);
LSAPI HBITMAP BitmapFromIcon(HICON hIcon);
LSAPI HRGN BitmapToRegion(HBITMAP hbmBitmap, COLORREF crTransparent, COLORREF crTolerance, int xOffset, int yOffset);
LSAPI void CommandParse(LPCSTR pszString, LPSTR pszCommandBuffer, LPSTR pszArgsBuffer, UINT cbCommandBuffer, UINT cbArgsBuffer);
LSAPI int CommandTokenize(LPCSTR pszString, LPSTR *ppszBuffers, UINT cBuffers, LPSTR pszExtraBuffer);
LSAPI HRESULT EnumLSData(UINT uType, FARPROC pfnCallback, LPARAM lParam);
LSAPI void Frame3D(HDC hDC, RECT rc, COLORREF crTop, COLORREF crBottom, int nWidth);
LSAPI HWND GetLitestepWnd();
LSAPI void GetLSBitmapSize(HBITMAP hbmBitmap, int *pnWidth, int *pnHeight);
LSAPI BOOL GetRCBool(LPCSTR pszKeyName, BOOL fValue);
LSAPI BOOL GetRCBoolDef(LPCSTR pszKeyName, BOOL fDefault);
LSAPI COLORREF GetRCColor(LPCSTR pszKeyName, COLORREF crDefault);
LSAPI int GetRCCoordinate(LPCSTR pszKeyName, int nDefault, int nLimit);
LSAPI int GetRCInt(LPCSTR pszKeyName, int nDefault);
LSAPI BOOL GetRCLine(LPCSTR pszKeyName, LPSTR pszBuffer, UINT cbBuffer, LPCSTR pszDefault);
LSAPI BOOL GetRCString(LPCSTR pszKeyName, LPSTR pszBuffer, LPCSTR pszDefault, UINT cbBuffer);
LSAPI void GetResStr(HINSTANCE hInstance, UINT uID, LPSTR pszBuffer, UINT cbBuffer, LPCSTR pszDefault);
LSAPI void GetResStrEx(HINSTANCE hInstance, UINT uID, LPSTR pszBuffer, UINT cbBuffer, LPCSTR pszDefault, ...);
LSAPI BOOL GetToken(LPCSTR pszString, LPSTR pszBuffer, LPCSTR *ppszNext, BOOL fBrackets);
LSAPI BOOL is_valid_pattern(LPCSTR pszPattern, int *pnError);
LSAPI BOOL LCClose(LPVOID pFile);
LSAPI LPVOID LCOpen(LPCSTR pszPath);
LSAPI BOOL LCReadNextCommand(LPVOID pFile, LPSTR pszBuffer, UINT cbBuffer);
LSAPI BOOL LCReadNextConfig(LPVOID pFile, LPCSTR pszKeyName, LPSTR pszBuffer, UINT cbBuffer);
LSAPI BOOL LCReadNextLine(LPVOID pFile, LPSTR pszBuffer, UINT cbBuffer);
LSAPI int LCTokenize(LPCSTR pszString, LPSTR *ppszBuffers, UINT cBuffers, LPSTR pszExtraBuffer);
LSAPI HICON LoadLSIcon(LPCSTR pszPath, LPCSTR pszReserved);
LSAPI HBITMAP LoadLSImage(LPCSTR pszPath, LPCSTR pszReserved);
LSAPI HINSTANCE LSExecute(HWND hwndOwner, LPCSTR pszCommandLine, int nShowCmd);
LSAPI HINSTANCE LSExecuteEx(HWND hwndOwner, LPCSTR pszOperation, LPCSTR pszCommand, LPCSTR pszArgs, LPCSTR pszDirectory, int nShowCmd);
LSAPI BOOL WINAPI LSGetImagePath(LPSTR pszBuffer, UINT cbBuffer);
LSAPI BOOL WINAPI LSGetLitestepPath(LPSTR pszBuffer, UINT cbBuffer);
LSAPI BOOL LSGetVariable(LPCSTR pszKeyName, LPSTR pszBuffer);
LSAPI BOOL LSGetVariableEx(LPCSTR pszKeyName, LPSTR pszBuffer, UINT cbBuffer);
LSAPI BOOL LSSetVariable(LPCSTR pszKeyName, LPCSTR pszValue);
LSAPI BOOL match(LPCSTR pszPattern, LPCSTR pszText);
LSAPI int matche(LPCSTR pszPattern, LPCSTR pszText);
LSAPI BOOL ParseBangCommand(HWND hwndOwner, LPCSTR pszBangCommandName, LPCSTR pszArgs);
LSAPI int ParseCoordinate(LPCSTR pszString, int nDefault, int nLimit);
LSAPI BOOL RemoveBangCommand(LPCSTR pszBangCommandName);
LSAPI void SetDesktopArea(int xLeft, int yTop, int xRight, int yBottom);
LSAPI void TransparentBltLS(HDC hdcDest, int xDest, int yDest, int nWidth, int nHeight, HDC hdcSrc, int xSrc, int ySrc, COLORREF crTransparent);
LSAPI void VarExpansion(LPSTR pszBuffer, LPCSTR pszString);
LSAPI void VarExpansionEx(LPSTR pszBuffer, LPCSTR pszString, UINT cbBuffer);


#endif // LSAPI_H
