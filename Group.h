// Group.h: interface for the CGroup class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "IShellBrowserImpl.h"

enum EVENTS {
EVENT_ONCREATED,
EVENT_ONF1DOWN, EVENT_ONF2DOWN, EVENT_ONF3DOWN, EVENT_ONF4DOWN, EVENT_ONF5DOWN, EVENT_ONF6DOWN,
EVENT_ONF7DOWN, EVENT_ONF8DOWN, EVENT_ONF9DOWN, EVENT_ONF10DOWN, EVENT_ONF11DOWN, EVENT_ONF12DOWN,
EVENT_ONF1UP, EVENT_ONF2UP, EVENT_ONF3UP, EVENT_ONF4UP, EVENT_ONF5UP, EVENT_ONF6UP, EVENT_ONF7UP,
EVENT_ONF8UP, EVENT_ONF9UP, EVENT_ONF10UP, EVENT_ONF11UP, EVENT_ONF12UP,
EVENT_ONFOLDERCHANGE, EVENT_ONINSERTITEM, EVENT_ONREMOVEITEM,
EVENT_ONCTRLA, EVENT_ONCTRLV, EVENT_ONCTRLX, EVENT_ONCTRLC, EVENT_ONCTRLZ,
EVENT_ONDELDOWN, EVENT_ONDELUP,
EVENT_ONLEFTCLICKDOWN, EVENT_ONRIGHTCLICKDOWN, EVENT_ONMIDDLECLICKDOWN, EVENT_ONX1CLICKDOWN, EVENT_ONX2CLICKDOWN,
EVENT_ONLEFTCLICKUP, EVENT_ONRIGHTCLICKUP, EVENT_ONMIDDLECLICKUP, EVENT_ONX1CLICKUP, EVENT_ONX2CLICKUP,
EVENT_ONLEFTDBLCLICK, EVENT_ONRIGHTDBLCLICK, EVENT_ONMIDDLEDBLCLICK, EVENT_ONX1DBLCLICK, EVENT_ONX2DBLCLICK,
EVENT_ONWHEELUP, EVENT_ONWHEELDOWN, EVENT_ONWHEELLEFT, EVENT_ONWHEELRIGHT,
EVENT_ONRIGHTKEYDOWN, EVENT_ONLEFTKEYDOWN, EVENT_ONUPKEYDOWN, EVENT_ONDOWNKEYDOWN, EVENT_ONENTERDOWN,
EVENT_ONRIGHTKEYUP, EVENT_ONLEFTKEYUP, EVENT_ONUPKEYUP, EVENT_ONDOWNKEYUP, EVENT_ONENTERUP,
EVENT_COUNT };

// regular events
#define EVENT_ACTION_NONE ".none"
#define EVENT_ACTION_RENAME ".rename"
#define EVENT_ACTION_DELETE ".delete"
#define EVENT_ACTION_PASTE ".paste"
#define EVENT_ACTION_UNDO ".undo"
#define EVENT_ACTION_COPY ".copy"
#define EVENT_ACTION_CUT ".cut"
#define EVENT_ACTION_SELECTALL ".selectall"
#define EVENT_ACTION_REFRESH ".refresh"
#define EVENT_ACTION_UP ".up" // cd ..
#define EVENT_ACTION_CHANGEDIR ".changedir" // cd <folder>
#define EVENT_ACTION_DOWNKEY ".godown"
#define EVENT_ACTION_UPKEY ".goup"
#define EVENT_ACTION_LEFTKEY ".goleft"
#define EVENT_ACTION_RIGHTKEY ".goright"
#define EVENT_ACTION_EXECUTE ".execute"
#define EVENT_ACTION_BACK ".back"
#define EVENT_ACTION_FORWARD ".forward"

// mouse specific events
#define EVENT_ACTION_RECTANGLE ".rectangle"
#define EVENT_ACTION_SYSTEMMENU ".systemmenu"
#define EVENT_ACTION_FORWARD ".forward"

#define REGKEY_MODULE	"SOFTWARE\\LiteStep\\Clickonic\\"

#define HIDA_GetPIDLParent(pida) (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])
#define HIDA_GetPIDLItem(pida, i) (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])

class CDropTarget;

class CGroup: public IShellBrowserImpl {
public:
	// Implemented in Group.cpp
	CGroup(const char *szName);
	virtual ~CGroup();
	bool ReadSettings(bool bIsRefresh);
	bool InitFolderView();
	void ReadIconSettings();
	void ScanIcons();
	bool HandleInsItem(int iItem);
	bool IconShouldBeHidden(const char *szIconName);
	void DeleteFiles();
	void DeleteFilesVirtually();
	void CopyFiles(bool cut);
	void ClearCutMark();
	void DoPaste();
	void PasteFiles();
	void PasteText();
	void PasteBitmap();
	void PasteWave();
	void SetTextColor(COLORREF crColor);
	void SetTextBkColor(COLORREF crColor);
	void SetBackground(const char *szBackground, int nOffsetPercentX, int nOffsetPercentY, bool bTiled);
	void SetBackgroundColor(COLORREF crColor);
	void SetViewMode(const char *szNewMode);
	void HotTracking (bool bEnabled);
	void InfoTip (bool bEnabled);
	void SnapToGrid (bool bEnabled);
	void AlignTop (bool bEnabled);
	void Reposition (int DcX, int DcY, int width, int height, bool relative);
	bool GetNameFromId(int iItem, char* szFileName, UINT cchLength);
	LPITEMIDLIST GetPIDLFromId(int iItem);
	WORD GetModkeyWord(char* szOption, char* szDefault);
	void ChangeDir(const char *szNewDir);
	bool CheckModkeys(WORD wKeys);
	//void FixMyDocs();
	void HandleSettingChange();
	void relayMouseMessage(UINT message, WPARAM wParam, LPARAM lParam);
	IDropTarget* GetLastItemTarget(bool *pbExecute);
	void Drop(DWORD grfKeyState, POINTL *ScPt, DWORD *pdwEffect, CIDA *pida, POINT *pOffsets);
	void SetDropHover(POINTL *pt, DWORD *pdwEffect);
	void ClearDropHover();
	void HandleEvent(UINT uEvent);
	void LoadEvents();
	void HandleMouseEvent(UINT uEvent, UINT msg, WPARAM wParam, LPARAM lParam);
	void ToggleIcon(LPCTSTR pszFileName, bool bVisible);
	void ToggleIcon(int iD, bool bVisible);
	int GetIdFromName(LPCTSTR pszFileName);
	void CheckVirtualIcon(LPCTSTR pszFileName, bool bDeleted);
	void GetStoredFolderLocation(LPSTR szBuf);
	void SetIconSize(int size, bool bRefresh = false);

	// Implemented in IconPosition.cpp
	void Reset(const char *szFolder, bool bDontSort);
	void SaveState();
	void SaveToFile();
	void RestoreState();
	void RestoreFromFile();
	void SortByName();
	void SortBySize();
	void SortByType();
	void SortByDate();

	char	m_szName[MAX_PATH]; // Name of the group
	char	m_szInitError[MAX_LINE_LENGTH];
	bool	m_bCutMarkedFiles; // Whether or not the group currently has files with a cutmark
	char	m_szEventCommand[EVENT_COUNT][MAX_LINE_LENGTH]; // Commands to execute on events
	int		m_nBackFolderCount;
	int		m_nForwardFolderCount;
	POINTL	m_NextIconPosition;
	HWND	m_hwndView;
	HWND	m_hwndListView;
	HWND	m_hwndNextClipViewer;
	HFONT	m_hFont;
	COLORREF m_crBackgroundColor;
	COLORREF m_crTextColor;
	COLORREF m_crTextBackgroundColor;
	CDropTarget *m_pDropTarget;
	WNDPROC m_wpOrigListViewProc;
	WNDPROC m_wpOrigGroupProc;
	LPITEMIDLIST	m_pidlFolder; // PIDL address of the folder to browse
	char	m_szTextPasteFormat[MAX_PATH];
	bool	m_bPersistantFolderLocation;
	bool	m_bHidden;
	bool	m_bInitialized;
	bool	m_bNoScroll;
	bool	m_bExplorerDesktop;
	char	m_szMyDocumentsPath[MAX_PATH];
	char	m_szDesktopPath[MAX_PATH];
	char	m_szRegDeskIconsPath[MAX_PATH];
	bool	m_bDontPasteText;
	bool	m_bDontPasteImages;
	bool	m_bDontPasteAudio;
	bool	m_bDontUseRegistryIcons;
	bool	m_bAutoArrange;
	bool	m_bAlignTop;
	bool	m_bNoInfoTips;
	bool	m_bAlwaysShowSelection;
	bool	m_bDontUseRecycleBin;
	bool	m_bConfirmFileDelete;
	bool	m_bSimpleProgress;
	bool	m_bNoConnectedFiles;
	bool	m_bVirtualFileDelete;
	bool	m_bSnapToGrid;
	bool	m_bHotTracking;
	int		m_nHotTrackingTime;
	bool	m_bSingleClick;
	bool	m_bInFolderChange;
	bool	m_bStealFocus;
	
	int		m_DcX, m_DcY;
	int		m_nWidth, m_nHeight;
	
	int		m_nMonitor;
	int		m_nLastItem;
	int		m_nSpacingX, m_nSpacingY;
	int		m_nOffsetPercentX, m_nOffsetPercentY;
	UINT	m_uViewMode;
	long	m_longListViewStyle;
	char	m_szBackground[MAX_PATH];
	bool	m_bTiled;
	char	m_szFolderLocation[MAX_PATH];
	char	m_szIconPositionFile[MAX_PATH];
	bool	m_bUseIconPositionFile;
	char	m_CustomDrive;
	bool	m_bInlineBrowsing;
	bool	m_bNoVirtualSwitch;
	WORD	m_wSelectionRectangleMod;
	WORD	m_wSystemMenuMod;
	IconMap m_HiddenIcons;
	bool	m_bExplicitCalls;
	bool	m_bShortcutMode;
	int		m_iIconSize;
};