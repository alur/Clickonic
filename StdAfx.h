#pragma once

#define WIN32_LEAN_AND_MEAN
#define MAX_RCCOMMAND 64

#include <AggressiveOptimize.h>
#include <map>
#include <string>
#include "lsapi.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <oleidl.h>
#include "strsafe.h"

class CGroup;

// Typedefs
#ifdef UNICODE
typedef std::wstring string;
#else
typedef std::string string;
#endif

struct stringicmp
{
    bool operator()(const string& s1, const string& s2) const
    {
        return (lstrcmpi(s1.c_str(), s2.c_str()) < 0);
    }
};

typedef struct MonitorInfo {
	int Top;
	int Left;
	int ResX;
	int ResY;
} MonitorInfo;

typedef std::map<string, CGroup*, stringicmp> GroupMap;
typedef std::map<string, bool> IconMap;

extern const char g_szAppName[];
extern HINSTANCE g_hInstance;
extern MonitorInfo g_Monitors[5];
extern int g_nErrorTranslucency;

CGroup* GetGroupByName(const char *szName);

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

void CreateGroup(const char *szName);
void DestroyGroup(const char *szName);
LRESULT WINAPI ListViewProc(HWND hListView, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI GroupProc(HWND hListView, UINT msg, WPARAM wParam, LPARAM lParam);