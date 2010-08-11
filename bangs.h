#pragma once

void RegisterBangs();
void UnregisterBangs();

void bangShow (HWND caller, LPCSTR szArgs);
void bangHide (HWND caller, LPCSTR szArgs);
void bangToggle (HWND caller, LPCSTR szArgs);
void bangReset (HWND caller, LPCSTR szArgs);
void bangSortByName	(HWND caller, LPCSTR szArgs);
void bangSortBySize	(HWND caller, LPCSTR szArgs);
void bangSortByType	(HWND caller, LPCSTR szArgs);
void bangSortByDate	(HWND caller, LPCSTR szArgs);
void bangSetTextColor (HWND caller, LPCSTR szArgs);
void bangSetTextBkColor (HWND caller, LPCSTR szArgs);
void bangSetBackground (HWND caller, LPCSTR szArgs);
void bangSetBackgroundColor	(HWND caller, LPCSTR szArgs);
void bangHotTracking (HWND caller, LPCSTR szArgs);
void bangHotTrackingTime (HWND caller, LPCSTR szArgs);
void bangSingleClick (HWND caller, LPCSTR szArgs);
void bangConfirmFileDelete (HWND caller, LPCSTR szArgs);
void bangDontUseReycleBin (HWND caller, LPCSTR szArgs);
void bangNoConnectedFiles (HWND caller, LPCSTR szArgs);
void bangSimpleProgress (HWND caller, LPCSTR szArgs);
void bangSnapToGrid (HWND caller, LPCSTR szArgs);
void bangVirtualFileDelete (HWND caller, LPCSTR szArgs);
void bangShowMyComputer (HWND caller, LPCSTR szArgs);
void bangShowMyNetworkPlaces (HWND caller, LPCSTR szArgs);
void bangShowInternetExplorer (HWND caller, LPCSTR szArgs);
void bangShowMyDocuments (HWND caller, LPCSTR szArgs);
void bangShowRecycleBin (HWND caller, LPCSTR szArgs);
void bangHideMyComputer (HWND caller, LPCSTR szArgs);
void bangHideMyNetworkPlaces (HWND caller, LPCSTR szArgs);
void bangHideInternetExplorer (HWND caller, LPCSTR szArgs);
void bangHideMyDocuments (HWND caller, LPCSTR szArgs);
void bangHideRecycleBin (HWND caller, LPCSTR szArgs);
void bangPasteFiles (HWND caller, LPCSTR szArgs);
void bangChangeDir (HWND caller, LPCSTR szArgs);
void bangSetViewMode (HWND caller, LPCSTR szArgs);
void bangInlineBrowsing (HWND caller, LPCSTR szArgs);
void bangSelectAll (HWND caller, LPCSTR szArgs);
void bangMove (HWND caller, LPCSTR szArgs);
void bangMoveBy (HWND caller, LPCSTR szArgs);
void bangResize (HWND caller, LPCSTR szArgs);
void bangResizeBy (HWND caller, LPCSTR szArgs);
void bangReposition (HWND caller, LPCSTR szArgs);
void bangRepositionBy (HWND caller, LPCSTR szArgs);
void bangInfoTip (HWND caller, LPCSTR szArgs);
void bangAlignTop (HWND caller, LPCSTR szArgs);
void bangRefreshEvent (HWND caller, LPCSTR szArgs);
void bangUseDelete (HWND caller, LPCSTR szArgs); // DEPRECATED!
void bangUseRename (HWND caller, LPCSTR szArgs); // DEPRECATED!
void bangToggleExplorerDesktop (HWND caller, LPCSTR szArgs);
void bangExplorerDesktop (HWND caller, LPCSTR szArgs);
void bangRenameFile (HWND caller, LPCSTR szArgs);
void bangDeleteFiles (HWND caller, LPCSTR szArgs);
void bangCopyFiles (HWND caller, LPCSTR szArgs);
void bangCutFiles (HWND caller, LPCSTR szArgs);
void bangRefresh (HWND caller, LPCSTR szArgs);
void bangExecute (HWND caller, LPCSTR szArgs);
void bangMoveFocus (HWND caller, LPCSTR szArgs);
void bangClearData (HWND caller, LPCSTR szArgs);
void bangRestoreIconPositions (HWND caller, LPCSTR szArgs);
void bangCreateGroup (HWND caller, LPCSTR szArgs);
void bangDestroyGroup (HWND caller, LPCSTR szArgs);
void bangShowControlPanel (HWND caller, LPCSTR szArgs);
void bangHideControlPanel (HWND caller, LPCSTR szArgs);
void bangSetMonitor (HWND caller, LPCSTR szArgs);
void bangSetIconSize (HWND caller, LPCSTR szArgs);