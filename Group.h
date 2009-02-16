// Group.h: interface for the CGroup class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "IShellBrowserImpl.h"

#define MODKEY_SHIFT	0x0001
#define MODKEY_ALT		0x0002
#define MODKEY_CTRL		0x0004

#define EVENT_ONCREATED 0
#define EVENT_ONF1DOWN 1
#define EVENT_ONF2DOWN 2
#define EVENT_ONF3DOWN 3
#define EVENT_ONF4DOWN 4
#define EVENT_ONF5DOWN 5
#define EVENT_ONF6DOWN 6
#define EVENT_ONF7DOWN 7
#define EVENT_ONF8DOWN 8
#define EVENT_ONF9DOWN 9
#define EVENT_ONF10DOWN 10
#define EVENT_ONF11DOWN 11
#define EVENT_ONF12DOWN 12
#define EVENT_ONF1UP 13
#define EVENT_ONF2UP 14
#define EVENT_ONF3UP 15
#define EVENT_ONF4UP 16
#define EVENT_ONF5UP 17
#define EVENT_ONF6UP 18
#define EVENT_ONF7UP 19
#define EVENT_ONF8UP 20
#define EVENT_ONF9UP 21
#define EVENT_ONF10UP 22
#define EVENT_ONF11UP 23
#define EVENT_ONF12UP 24
#define EVENT_ONFOLDERCHANGE 25
#define EVENT_ONINSERTITEM 26
#define EVENT_ONREMOVEITEM 27
#define EVENT_ONCTRLA 28
#define EVENT_ONCTRLV 29
#define EVENT_ONCTRLX 30
#define EVENT_ONCTRLC 31
#define EVENT_ONCTRLZ 32
#define EVENT_ONDELDOWN 33
#define EVENT_ONDELUP 34
#define EVENT_ONLEFTCLICKDOWN 35
#define EVENT_ONRIGHTCLICKDOWN 36
#define EVENT_ONMIDDLECLICKDOWN 37
#define EVENT_ONX1CLICKDOWN 38
#define EVENT_ONX2CLICKDOWN 39
#define EVENT_ONLEFTCLICKUP 40
#define EVENT_ONRIGHTCLICKUP 41
#define EVENT_ONMIDDLECLICKUP 42
#define EVENT_ONX1CLICKUP 43
#define EVENT_ONX2CLICKUP 44
#define EVENT_ONLEFTDBLCLICK 45
#define EVENT_ONRIGHTDBLCLICK 46
#define EVENT_ONMIDDLEDBLCLICK 47
#define EVENT_ONX1DBLCLICK 48
#define EVENT_ONX2DBLCLICK 49
#define EVENT_ONWHEELUP 50
#define EVENT_ONWHEELDOWN 51
#define EVENT_ONWHEELLEFT 52
#define EVENT_ONWHEELRIGHT 53
#define EVENT_ONRIGHTKEYDOWN 54
#define EVENT_ONLEFTKEYDOWN 55
#define EVENT_ONUPKEYDOWN 56
#define EVENT_ONDOWNKEYDOWN 57
#define EVENT_ONENTERDOWN 58
#define EVENT_ONRIGHTKEYUP 59
#define EVENT_ONLEFTKEYUP 60
#define EVENT_ONUPKEYUP 61
#define EVENT_ONDOWNKEYUP 62
#define EVENT_ONENTERUP 63
#define EVENT_COUNT EVENT_ONENTERUP+1

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
	CGroup(const char *szName);
	virtual ~CGroup();
	bool ReadSettings(bool bIsRefresh);
	bool InitFolderView();
	void ReadIconSettings();
	void Reset(const char *szFolder, bool bDontSort);
	void SaveState();
	void SaveToFile();
	void RestoreState();
	void RestoreFromFile();
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
	void SortByName();
	void SortBySize();
	void SortByType();
	void SortByDate();
	void SetTextColor(COLORREF crColor);
	void SetTextBkColor(COLORREF crColor);
	void SetBackground(const char *szBackground, int nOffsetPercentX, int nOffsetPercentY, bool bTiled);
	void SetBackgroundColor(COLORREF crColor);
	void SetViewMode(const char *szNewMode);
	void HotTracking (bool bEnabled);
	void InfoTip (bool bEnabled);
	void SnapToGrid (bool bEnabled);
	void AlignTop (bool bEnabled);
	void Reposition (int x, int y, int width, int height, bool relative);
	bool GetNameFromId(int iItem, char* szFileName, UINT cchLength);
	LPITEMIDLIST GetPIDLFromId(int iItem);
	WORD GetModkeyWord(char* szOption, char* szDefault);
	void ChangeDir(const char *szNewDir);
	bool CheckModkeys(WORD wKeys);
	//void FixMyDocs();
	//void HandleSettingChange();
	void relayMouseMessage(UINT message, WPARAM wParam, LPARAM lParam);
	IDropTarget* GetLastItemTarget(bool *pbExecute);
	void Drop(DWORD grfKeyState, POINTL *pt, DWORD *pdwEffect, CIDA *pida, POINT *pOffsets);
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
	bool	m_bInFolderChange;
	bool	m_bStealFocus;
	int		m_nX, m_nY;
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
};