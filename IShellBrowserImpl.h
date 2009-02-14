// IShellBrowserHost.h: interface for the IShellBrowserHost class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ISHELLBROWSER_HOST__
#define __ISHELLBROWSER_HOST__

#pragma once

class IShellBrowserImpl: public IShellBrowser {
	DWORD m_dwRef;

public:

	HWND m_hWnd;
	// m_pFolder is a system IShellFolder object
	LPSHELLFOLDER	m_pFolder;
	// IShellView is a graphical representation of the IShellFolder
	LPSHELLVIEW		m_pView;
	// IShellView2 interface derivated from m_pView
	IShellView2		*m_pView2;
	// IExtractIcon interface
	IExtractIcon	*m_pIcon;

public:
	IShellBrowserImpl();

	STDMETHOD(QueryInterface)(REFIID iid, void **ppvObject);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

    // IOleWindow methods
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
	STDMETHOD(SendControlMsg)(THIS_ UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT * pret);	
	STDMETHOD(GetControlWindow)(THIS_ UINT id, HWND * lphwnd);
	STDMETHOD(GetWindow)(HWND * lphwnd);

    // IShellBrowser methods (same as IOleInPlaceFrame)
    STDMETHOD(InsertMenusSB) (HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHOD(SetMenuSB) (HMENU hmenuShared, HOLEMENU holemenuReserved,HWND hwndActiveObject);
    STDMETHOD(RemoveMenusSB) (HMENU hmenuShared);
    STDMETHOD(SetStatusTextSB) (LPCOLESTR lpszStatusText);
    STDMETHOD(EnableModelessSB) (BOOL fEnable);
	STDMETHOD(GetViewStateStream)(DWORD grfMode,LPSTREAM  *ppStrm);
	STDMETHOD(SetToolbarItems)(LPTBBUTTON lpButtons, UINT nButtons,UINT uFlags);
	STDMETHOD(TranslateAcceleratorSB) (LPMSG lpmsg, WORD wID);

	STDMETHOD(QueryActiveShellView)(struct IShellView ** ppshv);
	STDMETHOD(OnViewWindowActive)(struct IShellView *ppshv);
	STDMETHOD(BrowseObject)(LPCITEMIDLIST pidl, UINT wFlags);
};

#endif // __ISHELLBROWSER_HOST__