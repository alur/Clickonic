extern "C" {
    __declspec( dllexport ) int initModuleEx(HWND hwndParent, HINSTANCE hDllInstance, LPCSTR szPath);
    __declspec( dllexport ) void quitModule(HINSTANCE hDllInstance);
}

const char g_rcsRevision[]	= "1.1.5";
const char g_szAppName[]	= "Clickonic";
const char g_szMsgHandler[]	= "LSClickonicManager";
const UINT g_lsMessages[] = {LM_GETREVID, LM_REFRESH, 0};

// Variables
HWND g_hParent;
HINSTANCE g_hInstance;
char g_szPath[MAX_PATH];
GroupMap g_Groups;
MonitorInfo g_Monitors[5]; // Supports up to 4 monitors
HWND g_hwndMessageHandler;
int g_iMon = 2;
int g_nErrorTranslucency;
OSVERSIONINFO g_osVersion;

// Functions
void UpdateMonitorInfo();
bool CreateMessageHandler();
void LoadGroups();
BOOL CALLBACK SetMonitorVars(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
LRESULT WINAPI MessageHandlerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);