#include "stdafx.h"
#include "Group.h"
#include "utils.h"
#include "lsapiex.h"

/**************************************************************************************************
	This function handles events.
**************************************************************************************************/
void CGroup::HandleEvent(UINT uEvent)
{
	if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_NONE) == 0)
	{
		return;
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_RENAME) == 0)
	{
		ListView_EditLabel(m_hwndListView, ListView_GetNextItem(m_hwndListView, -1, LVNI_FOCUSED));
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_EXECUTE) == 0)
	{
		if (ListView_GetSelectedCount(m_hwndListView) == 1)
		{
			int iItem = ListView_GetNextItem(m_hwndListView, -1, LVNI_SELECTED);
			char szFileName[MAX_PATH];
			if (GetNameFromId(iItem, szFileName, sizeof(szFileName)))
			{
				if (utils::Is_Directory(szFileName))
				{
					if (m_bInlineBrowsing)
					{
						return ChangeDir(".selected");
					}
					else if (m_bExplicitCalls)
					{
						LSExecuteEx(NULL, "open", "explorer.exe", szFileName, NULL, SW_SHOWNORMAL);
						return;
					}
				}
			}
		}
		CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_KEYDOWN, VK_RETURN, NULL);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_DELETE) == 0)
	{
		DeleteFiles();
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_PASTE) == 0)
	{
		DoPaste();
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_UNDO) == 0)
	{
		// TODO::Add support for this
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_COPY) == 0)
	{
		CopyFiles(false);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_CUT) == 0)
	{
		CopyFiles(true);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_SELECTALL) == 0)
	{
		ListView_SetItemState(m_hwndListView, -1, LVIS_SELECTED, LVIS_SELECTED);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_REFRESH) == 0)
	{
		SaveState();
		m_pView2->Refresh();
		RestoreState();
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_UP) == 0)
	{
		ChangeDir(".up");
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_CHANGEDIR) == 0)
	{
		ChangeDir(".selected");
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_DOWNKEY) == 0)
	{
		CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_KEYDOWN, VK_DOWN, NULL);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_UPKEY) == 0)
	{
		CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_KEYDOWN, VK_UP, NULL);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_LEFTKEY) == 0)
	{
		CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_KEYDOWN, VK_LEFT, NULL);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_RIGHTKEY) == 0)
	{
		CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_KEYDOWN, VK_RIGHT, NULL);
	}
	else
	{
		LSExecute(m_hwndListView, m_szEventCommand[uEvent], SW_SHOWNORMAL);
	}
}

/**************************************************************************************************
	This function handles mouse events.
**************************************************************************************************/
void CGroup::HandleMouseEvent(UINT uEvent, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (m_bExplorerDesktop)
	{
		CallWindowProc(m_wpOrigListViewProc, m_hwndListView, msg, wParam, lParam);
		return;
	}

	if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_LBUTTONDBLCLK)
	{
		LVHITTESTINFO lvhi = {0};
		lvhi.flags = LVHT_ONITEM;
		lvhi.pt.x = GET_X_LPARAM(lParam);
		lvhi.pt.y = GET_Y_LPARAM(lParam);

		if ( ListView_HitTest(m_hwndListView, &lvhi) != -1 )
		{
			CallWindowProc(m_wpOrigListViewProc, m_hwndListView, msg, wParam, lParam);
			if (msg == WM_LBUTTONDOWN && m_bSingleClick)
				SendMessage(m_hwndListView, WM_LBUTTONDBLCLK, wParam, lParam);
			return; // An icon was clicked.
		}
	}

	if (m_bStealFocus)
		SetFocus(m_hwndListView);

	if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_CHANGEDIR) == 0)
	{
		// We can override HandleEvent actions like this ;)
		LVHITTESTINFO lvhi = {0};
		lvhi.flags = LVHT_ONITEM;
		lvhi.pt.x = GET_X_LPARAM(lParam);
		lvhi.pt.y = GET_Y_LPARAM(lParam);

		if (msg == WM_MOUSEWHEEL || msg == WM_MOUSEHWHEEL)
		{
			MapWindowPoints(HWND_DESKTOP, m_hwndListView, &lvhi.pt, 1);
		}

		int nFileid = ListView_HitTest(m_hwndListView, &lvhi);
		if ( nFileid != -1 )
		{
			char szFileName[MAX_PATH];
			if (GetNameFromId(nFileid, szFileName, sizeof(szFileName)))
			{
				if (utils::Is_Directory(szFileName))
				{
					if (!m_bNoVirtualSwitch && (_stricmp(szFileName, m_szDesktopPath) == 0))
					{
						ChangeDir(".desktop");
					}
					else
					{
						ChangeDir(szFileName);
					}
					return;
				}
			}
		}
	}

	// We just need to handle the mouse specific events and then forward the event to HandleEvent
	if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_RECTANGLE) == 0)
	{
		if (CheckModkeys(m_wSelectionRectangleMod))
		{
			CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_LBUTTONDOWN, wParam, lParam);
		}
		else
		{
			relayMouseMessage(msg, wParam, lParam);
		}
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_SYSTEMMENU) == 0)
	{
		if (CheckModkeys(m_wSystemMenuMod))
		{
			CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_RBUTTONDOWN, wParam, lParam);
		}
		else
		{
			relayMouseMessage(msg, wParam, lParam);
		}
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_FORWARD) == 0)
	{
		relayMouseMessage(msg, wParam, lParam);
	}
	else
	{
		HandleEvent(uEvent);
	}
}

/**************************************************************************************************
	This function loads commands for all events
**************************************************************************************************/
void CGroup::LoadEvents()
{
	// Backwards compability
	bool bUseDelete	= LiteStep::GetPrefixedRCBool(m_szName, "UseDelete", TRUE);
	bool bUseRename	= LiteStep::GetPrefixedRCBool(m_szName, "UseRename", TRUE);
	bool bDontUseCtrlA = LiteStep::GetPrefixedRCBool(m_szName, "DontUseCtrlA", FALSE);
	bool bDontUseCtrlC = LiteStep::GetPrefixedRCBool(m_szName, "DontUseCtrlC", FALSE);
	bool bDontUseCtrlV = LiteStep::GetPrefixedRCBool(m_szName, "DontUseCtrlV", FALSE);
	bool bDontUseCtrlX = LiteStep::GetPrefixedRCBool(m_szName, "DontUseCtrlX", FALSE);
	bool bUseSelectionRectangle = LiteStep::GetPrefixedRCBool(m_szName, "UseSelectionRectangle", FALSE);
	bool bUseSystemMenu = LiteStep::GetPrefixedRCBool(m_szName, "UseSystemMenu", FALSE);
	
	// Event Settings
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF1DOWN], m_szName, "OnF1Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF2DOWN], m_szName, "OnF2Down", bUseRename ? EVENT_ACTION_RENAME : EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF3DOWN], m_szName, "OnF3Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF4DOWN], m_szName, "OnF4Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF5DOWN], m_szName, "OnF5Down", EVENT_ACTION_REFRESH);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF6DOWN], m_szName, "OnF6Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF7DOWN], m_szName, "OnF7Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF8DOWN], m_szName, "OnF8Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF9DOWN], m_szName, "OnF9Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF10DOWN], m_szName, "OnF10Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF11DOWN], m_szName, "OnF11Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF12DOWN], m_szName, "OnF12Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF1UP], m_szName, "OnF1Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF2UP], m_szName, "OnF2Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF3UP], m_szName, "OnF3Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF4UP], m_szName, "OnF4Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF5UP], m_szName, "OnF5Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF6UP], m_szName, "OnF6Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF7UP], m_szName, "OnF7Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF8UP], m_szName, "OnF8Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF9UP], m_szName, "OnF9Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF10UP], m_szName, "OnF10Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF11UP], m_szName, "OnF11Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF12UP], m_szName, "OnF12Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF11UP], m_szName, "OnF11Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF12UP], m_szName, "OnF12Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONDELDOWN], m_szName, "OnDELDown", bUseDelete ? EVENT_ACTION_DELETE : EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONDELUP], m_szName, "OnDELUp", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONFOLDERCHANGE], m_szName, "OnFolderChange", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONINSERTITEM], m_szName, "OnRemoveItem", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONREMOVEITEM], m_szName, "OnInsertItem", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONCTRLA], m_szName, "OnCtrlA", bDontUseCtrlA ? EVENT_ACTION_NONE : EVENT_ACTION_SELECTALL);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONCTRLV], m_szName, "OnCtrlV", bDontUseCtrlV ? EVENT_ACTION_NONE : EVENT_ACTION_PASTE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONCTRLX], m_szName, "OnCtrlX", bDontUseCtrlX ? EVENT_ACTION_NONE : EVENT_ACTION_CUT);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONCTRLC], m_szName, "OnCtrlC", bDontUseCtrlC ? EVENT_ACTION_NONE : EVENT_ACTION_COPY);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONCTRLZ], m_szName, "OnCtrlZ", EVENT_ACTION_UNDO);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONLEFTCLICKDOWN], m_szName, "OnLeftClickDown", bUseSelectionRectangle ? EVENT_ACTION_RECTANGLE : EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONRIGHTCLICKDOWN], m_szName, "OnRightClickDown", bUseSystemMenu ? EVENT_ACTION_SYSTEMMENU : EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONMIDDLECLICKDOWN], m_szName, "OnMiddleClickDown", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONX1CLICKDOWN], m_szName, "OnX1ClickDown", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONX2CLICKDOWN], m_szName, "OnX2ClickDown", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONLEFTCLICKUP], m_szName, "OnLeftClickUp", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONRIGHTCLICKUP], m_szName, "OnRightClickUp", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONMIDDLECLICKUP], m_szName, "OnMiddleClickUp", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONX1CLICKUP], m_szName, "OnX1ClickUp", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONX2CLICKUP], m_szName, "OnX2ClickUp", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONLEFTDBLCLICK], m_szName, "OnLeftDoubleClick", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONRIGHTDBLCLICK], m_szName, "OnRightDoubleClick", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONMIDDLEDBLCLICK], m_szName, "OnMiddleDoubleClick", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONX1DBLCLICK], m_szName, "OnX1DoubleClick", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONX2DBLCLICK], m_szName, "OnX2DoubleClick", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONWHEELUP], m_szName, "OnWheelUp", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONWHEELDOWN], m_szName, "OnWheelDown", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONWHEELLEFT], m_szName, "OnWheelLeft", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONWHEELRIGHT], m_szName, "OnWheelRight", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONRIGHTKEYDOWN], m_szName, "OnRightKeyDown", EVENT_ACTION_RIGHTKEY);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONLEFTKEYDOWN], m_szName, "OnLeftKeyDown", EVENT_ACTION_LEFTKEY);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONUPKEYDOWN], m_szName, "OnUpKeyDown", EVENT_ACTION_UPKEY);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONDOWNKEYDOWN], m_szName, "OnDownKeyDown", EVENT_ACTION_DOWNKEY);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONENTERDOWN], m_szName, "OnEnterDown", EVENT_ACTION_EXECUTE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONRIGHTKEYUP], m_szName, "OnRightKeyUp", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONLEFTKEYUP], m_szName, "OnLeftKeyUp", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONUPKEYUP], m_szName, "OnUpKeyUp", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONDOWNKEYUP], m_szName, "OnDownKeyUp", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONENTERUP], m_szName, "OnEnterUp", EVENT_ACTION_NONE);
}
