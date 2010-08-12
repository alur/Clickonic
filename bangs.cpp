#include "stdafx.h"
#include "bangs.h"
#include "group.h"
#include "utils.h"

typedef struct BangItem {
	char szName[25];
	BANGCOMMANDPROC pCommand;
} BangItem;

#define BANGS_NUM		64
BangItem g_Bangs[BANGS_NUM] =
{
	{	"Show",							bangShow	},
	{	"Hide",							bangHide	},
	{	"Toggle",						bangToggle	},
	{	"Reset",						bangReset	},
	{	"SortByName",					bangSortByName	},
	{	"SortBySize",					bangSortBySize	},
	{	"SortByType",					bangSortByType	},
	{	"SortByDate",					bangSortByDate	},
	{	"SetTextColor",					bangSetTextColor},
	{	"SetTextBkColor",				bangSetTextBkColor},
	{	"SetBackground",				bangSetBackground},
	{	"SetBackgroundColor",			bangSetBackgroundColor},
	{	"HotTracking",					bangHotTracking	},
	{	"HotTrackingTime",				bangHotTrackingTime	},
	{	"SingleClick",					bangSingleClick	},
	{	"ConfirmFileDelete",			bangConfirmFileDelete },
	{	"DontUseRecycleBin",			bangDontUseReycleBin },
	{	"NoConnectedFiles",				bangNoConnectedFiles },
	{	"SimpleProgress",				bangSimpleProgress },
	{	"SnapToGrid",					bangSnapToGrid },
	{	"VirtualFileDelete",			bangVirtualFileDelete },
	{	"ShowMyComputer",				bangShowMyComputer },
	{	"ShowMyNetworkPlaces",			bangShowMyNetworkPlaces },
	{	"ShowInternetExplorer",			bangShowInternetExplorer },
	{	"ShowMyDocuments",				bangShowMyDocuments },
	{	"ShowRecycleBin",				bangShowRecycleBin },
	{	"HideMyComputer",				bangHideMyComputer },
	{	"HideMyNetworkPlaces",			bangHideMyNetworkPlaces },
	{	"HideInternetExplorer",			bangHideInternetExplorer },
	{	"HideMyDocuments",				bangHideMyDocuments },
	{	"HideRecycleBin",				bangHideRecycleBin },
	{	"PasteFiles",					bangPasteFiles },
	{	"ChangeDir",					bangChangeDir },
	{	"SetViewMode",					bangSetViewMode },
	{	"InlineBrowsing",				bangInlineBrowsing },
	{	"SelectAll",					bangSelectAll },
	{	"Move",							bangMove },
	{	"MoveBy",						bangMoveBy },
	{	"Resize",						bangResize },
	{	"ResizeBy",						bangResizeBy },
	{	"Reposition",					bangReposition },
	{	"RepositionBy",					bangRepositionBy },
	{	"InfoTip",						bangInfoTip },
	{	"AlignTop",						bangAlignTop },
	{	"RefreshEvent",					bangRefreshEvent },
	{	"ToggleExplorerDesktop",		bangToggleExplorerDesktop	},
	{	"ExplorerDesktop",				bangExplorerDesktop	},
	{	"UseDelete",					bangUseDelete }, // DEPRECATED!
	{	"UseRename",					bangUseRename }, // DEPRECATED!
	{	"RenameFile",					bangRenameFile },
	{	"DeleteFiles",					bangDeleteFiles },
	{	"CopyFiles",					bangCopyFiles },
	{	"CutFiles",						bangCutFiles },
	{	"Refresh",						bangRefresh },
	{	"Execute",						bangExecute },
	{	"MoveFocus",					bangMoveFocus },
	{	"ClearData",					bangClearData },
	{	"RestoreIconPositions",			bangRestoreIconPositions },
	{	"CreateGroup",					bangCreateGroup },
	{	"DestroyGroup",					bangDestroyGroup },
	{	"HideControlPanel",				bangHideControlPanel },
	{	"ShowControlPanel",				bangShowControlPanel },
	{	"SetIconSize",					bangSetIconSize }
};


void RegisterBangs()
{
	char szBangName[MAX_BANGCOMMAND];
	for (int i = 0; i < BANGS_NUM; i++)
	{
		StringCchPrintf(szBangName, MAX_BANGCOMMAND, "!%s%s", g_szAppName, g_Bangs[i].szName);
		AddBangCommand(szBangName, g_Bangs[i].pCommand);
	}
}

void UnregisterBangs()
{
	char szBangName[MAX_BANGCOMMAND];
	for (int i = 0; i < BANGS_NUM; i++)
	{
		StringCchPrintf(szBangName, MAX_BANGCOMMAND, "!%s%s", g_szAppName, g_Bangs[i].szName);
		RemoveBangCommand(szBangName);
	}
}


////////////////////////////////////////////////////////////////////////////
//
// Diagnostic message for !bangs
//
void BangFailed(const char *szBangName, const char *szGroup)
{
	utils::ErrorMessage(E_WARNING, "Icon group with name \"%s\" not found, !%s%s bang failed", szGroup, g_szAppName, szBangName);
}

void bangShow (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("Show", szArgs);
	ShowWindow(pGroup->m_hwndView, SW_SHOW);
	pGroup->m_bHidden = false;
	utils::SetEvar(pGroup->m_szName, "IsHidden", "false");
}

void bangHide (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("Hide", szArgs);
	ShowWindow(pGroup->m_hwndView, SW_HIDE);
	pGroup->m_bHidden = true;
	utils::SetEvar(pGroup->m_szName, "IsHidden", "true");
}

void bangToggle (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("Toggle", szArgs);
	ShowWindow(pGroup->m_hwndView, pGroup->m_bHidden ? SW_SHOW : SW_HIDE);
	pGroup->m_bHidden = !pGroup->m_bHidden;
	utils::SetEvar(pGroup->m_szName, "IsHidden", pGroup->m_bHidden ? "true" : "false");
}

void bangReset (HWND /* caller */, LPCSTR szArgs)
{
	char token[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[1];
	tokens[0] = token;
	LCTokenize(szArgs, tokens, 1, szExtra);
	CGroup *pGroup = GetGroupByName(tokens[0]);
	if (!pGroup)
		return BangFailed("Reset", token);
	pGroup->Reset(szExtra, false);
}

void bangClearData (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("ClearData", szArgs);
	pGroup->Reset(".all", true);
}
void bangSortByName (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("SortByName", szArgs);
	pGroup->SortByName();
}

void bangSortBySize	(HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("SortBySize", szArgs);
	pGroup->SortBySize();
}

void bangSortByType	(HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("SortByType", szArgs);
	pGroup->SortByType();
}

void bangSortByDate	(HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("SortByDate", szArgs);
	pGroup->SortByDate();
}

void bangSetTextColor (HWND /* caller */, LPCSTR szArgs)
{
	char token[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[1];
	tokens[0] = token;
	LCTokenize(szArgs, tokens, 1, szExtra);
	CGroup *pGroup = GetGroupByName(tokens[0]);
	if (!pGroup)
		return BangFailed("SetTextColor", token);
	pGroup->SetTextColor(utils::String2Color(szExtra));
}

void bangSetTextBkColor (HWND /* caller */, LPCSTR szArgs)
{
	char token[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[1];
	tokens[0] = token;
	LCTokenize(szArgs, tokens, 1, szExtra);
	CGroup *pGroup = GetGroupByName(tokens[0]);
	if (!pGroup)
		return BangFailed("SetTextBkColor", token);
	pGroup->SetTextBkColor(utils::String2Color(szExtra));
}

void bangSetBackground (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], token3[MAX_LINE_LENGTH],
		token4[MAX_LINE_LENGTH], token5[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[5] = {token1, token2, token3, token4, token5};
	LCTokenize(szArgs, tokens, 5, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("SetBackground", token1);
	pGroup->SetBackground(token2, atoi(token3), atoi(token4), utils::String2Bool(token5));
}

void bangSetBackgroundColor (HWND /* caller */, LPCSTR szArgs)
{
	char token[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[1];
	tokens[0] = token;
	LCTokenize(szArgs, tokens, 1, szExtra);
	CGroup *pGroup = GetGroupByName(tokens[0]);
	if (!pGroup)
		return BangFailed("SetBackgroundColor", token);
	pGroup->SetBackgroundColor(utils::String2Color(szExtra));
}

void bangHotTracking (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("HotTracking", token1);
	if (!_stricmp(token2, "toggle"))
		pGroup->HotTracking(!pGroup->m_bHotTracking);
	else
		pGroup->HotTracking(utils::String2Bool(token2));
}

void bangSingleClick (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("SingleClick", token1);
	if (!_stricmp(token2, "toggle"))
		pGroup->m_bSingleClick = !pGroup->m_bSingleClick;
	else
		pGroup->m_bSingleClick = utils::String2Bool(token2);
}

void bangInfoTip (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("InfoTip", token1);
	if (!_stricmp(token2, "toggle"))
		pGroup->InfoTip(pGroup->m_bNoInfoTips);
	else
		pGroup->InfoTip(utils::String2Bool(token2));
}

void bangAlignTop (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("AlignTop", token1);
	if (!_stricmp(token2, "toggle"))
		pGroup->AlignTop(!pGroup->m_bAlignTop);
	else
		pGroup->AlignTop(utils::String2Bool(token2));
}

void bangHotTrackingTime (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("HotTrackingTime", token1);
	pGroup->m_nHotTrackingTime = atoi(token2);
	ListView_SetHoverTime(pGroup->m_hwndListView, pGroup->m_nHotTrackingTime);
}

void bangConfirmFileDelete (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("ConfirmFileDelete", token1);
	if (!_stricmp(token2, "toggle"))
		pGroup->m_bConfirmFileDelete = !pGroup->m_bConfirmFileDelete;
	else
		pGroup->m_bConfirmFileDelete = utils::String2Bool(token2);
}

void bangDontUseReycleBin (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("DontUseReycleBin", token1);
	if (!_stricmp(token2, "toggle"))
		pGroup->m_bDontUseRecycleBin = !pGroup->m_bDontUseRecycleBin;
	else
		pGroup->m_bDontUseRecycleBin = utils::String2Bool(token2);
}

void bangNoConnectedFiles (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("NoConnectedFiles", token1);
	if (!_stricmp(token2, "toggle"))
		pGroup->m_bNoConnectedFiles = !pGroup->m_bNoConnectedFiles;
	else
		pGroup->m_bNoConnectedFiles = utils::String2Bool(token2);
}

void bangSimpleProgress (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("SimpleProgress", token1);
	if (!_stricmp(token2, "toggle"))
		pGroup->m_bSimpleProgress = !pGroup->m_bSimpleProgress;
	else
		pGroup->m_bSimpleProgress = utils::String2Bool(token2);
}

void bangSnapToGrid (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("SnapToGrid", token1);
	if (!_stricmp(token2, "toggle"))
		pGroup->SnapToGrid(!pGroup->m_bSnapToGrid);
	else	
		pGroup->SnapToGrid(utils::String2Bool(token2));
}

void bangVirtualFileDelete (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("VirtualFileDelete", token1);
	if (!_stricmp(token2, "toggle"))
		pGroup->m_bVirtualFileDelete = !pGroup->m_bVirtualFileDelete;
	else	
		pGroup->m_bVirtualFileDelete = utils::String2Bool(token2);
}

void bangInlineBrowsing (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("InlineBrowsing", token1);
	if (!_stricmp(token2, "toggle"))
		pGroup->m_bInlineBrowsing = !pGroup->m_bInlineBrowsing;
	else
		pGroup->m_bInlineBrowsing = utils::String2Bool(token2);
}

void bangPasteFiles	(HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("PasteFiles", szArgs);
	pGroup->DoPaste();
}

void bangSelectAll	(HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("SelectAll", szArgs);
	ListView_SetItemState(pGroup->m_hwndListView, -1, LVIS_SELECTED, LVIS_SELECTED);	
}

void bangShowMyComputer (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("ShowMyComputer", szArgs);
	pGroup->ToggleIcon("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", true);
}

void bangShowMyNetworkPlaces (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("ShowMyNetworkPlaces", szArgs);
	pGroup->ToggleIcon("::{208D2C60-3AEA-1069-A2D7-08002B30309D}", true);
}

void bangShowInternetExplorer (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("ShowInternetExplorer", szArgs);
	pGroup->ToggleIcon("::{871C5380-42A0-1069-A2EA-08002B30309D}", true);
}

void bangShowMyDocuments (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("ShowMyDocuments", szArgs);
	pGroup->ToggleIcon(pGroup->m_szMyDocumentsPath, true);
}

void bangShowRecycleBin (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("ShowRecycleBin", szArgs);
	pGroup->ToggleIcon("::{645FF040-5081-101B-9F08-00AA002F954E}", true);
}

void bangShowControlPanel (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("ShowControlPanel", szArgs);
	pGroup->ToggleIcon("::{26EE0668-A00A-44D7-9371-BEB064C98683}", true);
}

void bangHideMyComputer (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("HideMyComputer", szArgs);
	pGroup->ToggleIcon("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", false);
}

void bangHideMyNetworkPlaces (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("HideMyNetworkPlaces", szArgs);
	pGroup->ToggleIcon("::{208D2C60-3AEA-1069-A2D7-08002B30309D}", false);
}

void bangHideInternetExplorer (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("HideInternetExplorer", szArgs);
	pGroup->ToggleIcon("::{871C5380-42A0-1069-A2EA-08002B30309D}", false);
}

void bangHideMyDocuments (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("HideMyDocuments", szArgs);
	pGroup->ToggleIcon(pGroup->m_szMyDocumentsPath, false);
}

void bangHideRecycleBin (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("HideRecycleBin", szArgs);
	pGroup->ToggleIcon("::{645FF040-5081-101B-9F08-00AA002F954E}", false);
}

void bangHideControlPanel (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("HideControlPanel", szArgs);
	pGroup->ToggleIcon("::{26EE0668-A00A-44D7-9371-BEB064C98683}", false);
}

void bangChangeDir (HWND /* caller */, LPCSTR szArgs)
{
	char token[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[1];
	tokens[0] = token;
	LCTokenize(szArgs, tokens, 1, szExtra);
	CGroup *pGroup = GetGroupByName(tokens[0]);
	if (!pGroup)
		return BangFailed("ChangeDir", token);
	pGroup->ChangeDir(szExtra);	
}
void bangSetViewMode (HWND /* caller */, LPCSTR szArgs)
{
	char token[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[1];
	tokens[0] = token;
	LCTokenize(szArgs, tokens, 1, szExtra);
	CGroup *pGroup = GetGroupByName(tokens[0]);
	if (!pGroup)
		return BangFailed("SetViewMode", token);
	pGroup->SetViewMode(szExtra);
}
void bangMove (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], token3[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[3] = {token1, token2, token3};
	LCTokenize(szArgs, tokens, 3, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("Move", token1);
	pGroup->Reposition(utils::DcParseCoordinate(token2, false), utils::DcParseCoordinate(token3, true), 0, 0, false);
}
void bangMoveBy (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], token3[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[3] = {token1, token2, token3};
	LCTokenize(szArgs, tokens, 3, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("MoveBy", token1);
	pGroup->Reposition(atoi(token2), atoi(token3), 0, 0, true);
}
void bangResize (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], token3[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[3] = {token1, token2, token3};
	LCTokenize(szArgs, tokens, 3, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("Resize", token1);
	pGroup->Reposition(0, 0, atoi(token2), atoi(token3), false);	
}
void bangResizeBy (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], token3[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[3] = {token1, token2, token3};
	LCTokenize(szArgs, tokens, 3, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("ResizeBy", token1);
	pGroup->Reposition(0, 0, atoi(token2), atoi(token3), true);	
}
void bangReposition (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], token3[MAX_LINE_LENGTH], token4[MAX_LINE_LENGTH], token5[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[5] = {token1, token2, token3, token4, token5};
	LCTokenize(szArgs, tokens, 5, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("Reposition", token1);
	pGroup->Reposition(atoi(token2), atoi(token3), atoi(token4), atoi(token5), false);
}
void bangRepositionBy (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], token3[MAX_LINE_LENGTH], token4[MAX_LINE_LENGTH], token5[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[5] = {token1, token2, token3, token4, token5};
	LCTokenize(szArgs, tokens, 5, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("RepositionBy", token1);
	pGroup->Reposition(atoi(token2), atoi(token3), atoi(token4), atoi(token5), true);	
}
void bangRefreshEvent (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("RefreshEvent", token1);
	LSSetVariable(token2, szExtra);
	pGroup->LoadEvents();
}
void bangUseDelete (HWND /* caller */, LPCSTR szArgs)
{
	// This function has been deprecated.
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("UseDelete", token1);
	if (!_stricmp(token2, "toggle"))
	{
		if (_stricmp(pGroup->m_szEventCommand[EVENT_ONDELDOWN], EVENT_ACTION_DELETE) == 0)
			StringCchCopy(pGroup->m_szEventCommand[EVENT_ONDELDOWN], MAX_LINE_LENGTH, EVENT_ACTION_NONE);
		else if (_stricmp(pGroup->m_szEventCommand[EVENT_ONDELDOWN], EVENT_ACTION_NONE) == 0)
			StringCchCopy(pGroup->m_szEventCommand[EVENT_ONDELDOWN], MAX_LINE_LENGTH, EVENT_ACTION_DELETE);
	}
	else
	{
		bool bUseDelete = utils::String2Bool(token2);
		if (bUseDelete && (_stricmp(pGroup->m_szEventCommand[EVENT_ONDELDOWN], EVENT_ACTION_NONE) == 0))
			StringCchCopy(pGroup->m_szEventCommand[EVENT_ONDELDOWN], MAX_LINE_LENGTH, EVENT_ACTION_DELETE);
		else if (_stricmp(pGroup->m_szEventCommand[EVENT_ONDELDOWN], EVENT_ACTION_DELETE) == 0)
			StringCchCopy(pGroup->m_szEventCommand[EVENT_ONDELDOWN], MAX_LINE_LENGTH, EVENT_ACTION_NONE);
	}
}
void bangUseRename (HWND /* caller */, LPCSTR szArgs)
{
	// This function has been deprecated.
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("UseRename", token1);
	if (!_stricmp(token2, "toggle"))
	{
		if (_stricmp(pGroup->m_szEventCommand[EVENT_ONF2DOWN], EVENT_ACTION_RENAME) == 0)
			StringCchCopy(pGroup->m_szEventCommand[EVENT_ONF2DOWN], MAX_LINE_LENGTH, EVENT_ACTION_NONE);
		else if (_stricmp(pGroup->m_szEventCommand[EVENT_ONF2DOWN], EVENT_ACTION_NONE) == 0)
			StringCchCopy(pGroup->m_szEventCommand[EVENT_ONF2DOWN], MAX_LINE_LENGTH, EVENT_ACTION_RENAME);
	}
	else
	{
		bool bUseRename = utils::String2Bool(token2);
		if (bUseRename && (_stricmp(pGroup->m_szEventCommand[EVENT_ONF2DOWN], EVENT_ACTION_NONE) == 0))
			StringCchCopy(pGroup->m_szEventCommand[EVENT_ONF2DOWN], MAX_LINE_LENGTH, EVENT_ACTION_RENAME);
		else if (_stricmp(pGroup->m_szEventCommand[EVENT_ONF2DOWN], EVENT_ACTION_RENAME) == 0)
			StringCchCopy(pGroup->m_szEventCommand[EVENT_ONF2DOWN], MAX_LINE_LENGTH, EVENT_ACTION_NONE);
	}
}
void bangToggleExplorerDesktop (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("ToggleExplorerDesktop", szArgs);
	pGroup->m_bExplorerDesktop = !pGroup->m_bExplorerDesktop;
}
void bangExplorerDesktop (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("ExplorerDesktop", token1);
	if (!_stricmp(token2, "toggle"))
		pGroup->m_bExplorerDesktop = !pGroup->m_bExplorerDesktop;
	else
		pGroup->m_bExplorerDesktop = utils::String2Bool(token2);
}
void bangRenameFile (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("RenameFile", szArgs);
	ListView_EditLabel(pGroup->m_hwndListView, ListView_GetNextItem(pGroup->m_hwndListView, -1, LVNI_FOCUSED));
}
void bangDeleteFiles (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("DeleteFiles", szArgs);
	pGroup->DeleteFiles();
}
void bangCopyFiles (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("CopyFiles", szArgs);
	pGroup->CopyFiles(false);
}
void bangCutFiles (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("CutFiles", szArgs);
	pGroup->CopyFiles(true);
}

void bangRefresh (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("Refresh", szArgs);
	pGroup->SaveState();
	pGroup->m_pView2->Refresh();
	pGroup->RestoreState();
}

void bangExecute (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("Execute", szArgs);
	CallWindowProc(pGroup->m_wpOrigListViewProc, pGroup->m_hwndListView, WM_KEYDOWN, VK_RETURN, NULL);
}

void bangMoveFocus (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[2] = {token1, token2};
	LCTokenize(szArgs, tokens, 2, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("MoveFocus", token1);
	if (!_stricmp(token2, ".up"))
		CallWindowProc(pGroup->m_wpOrigListViewProc, pGroup->m_hwndListView, WM_KEYDOWN, VK_UP, NULL);
	else if (!_stricmp(token2, ".down"))
		CallWindowProc(pGroup->m_wpOrigListViewProc, pGroup->m_hwndListView, WM_KEYDOWN, VK_DOWN, NULL);
	else if (!_stricmp(token2, ".right"))
		CallWindowProc(pGroup->m_wpOrigListViewProc, pGroup->m_hwndListView, WM_KEYDOWN, VK_RIGHT, NULL);
	else if (!_stricmp(token2, ".left"))
		CallWindowProc(pGroup->m_wpOrigListViewProc, pGroup->m_hwndListView, WM_KEYDOWN, VK_LEFT, NULL);
}

void bangRestoreIconPositions (HWND /* caller */, LPCSTR szArgs)
{
	CGroup *pGroup = GetGroupByName(szArgs);
	if (!pGroup)
		return BangFailed("RestoreIconPositions", szArgs);
	pGroup->RestoreState();
}

void bangCreateGroup (HWND /* caller */, LPCSTR szArgs)
{
	CreateGroup(szArgs);
}

void bangDestroyGroup (HWND /* caller */, LPCSTR szArgs)
{
	DestroyGroup(szArgs);
}

void bangSetMonitor (HWND /* caller */, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[1] = {token1};
	LCTokenize(szArgs, tokens, 1, szExtra);
	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("SetMonitor", token1);

	int nNewMonitor = atoi(szExtra);
	int nOldMonitor = pGroup->m_nMonitor;

	// New monitor's origin, in the old monitor's coordinate system
	POINTL McNewMonitorOrigin = {
		g_Monitors[nNewMonitor].Left - g_Monitors[nOldMonitor].Left,
		g_Monitors[nNewMonitor].Top - g_Monitors[nOldMonitor].Top
		};
	pGroup->m_DcX += McNewMonitorOrigin.x;
	pGroup->m_DcY += McNewMonitorOrigin.y;
	pGroup->m_nMonitor = nNewMonitor;
}

void bangSetIconSize (HWND, LPCSTR szArgs)
{
	char token1[MAX_LINE_LENGTH], szExtra[MAX_LINE_LENGTH];
	char* tokens[1] = {token1};
	LCTokenize(szArgs, tokens, 1, szExtra);

	CGroup *pGroup = GetGroupByName(token1);
	if (!pGroup)
		return BangFailed("SetIconSize", token1);

	pGroup->SetIconSize(atoi(szExtra));
}