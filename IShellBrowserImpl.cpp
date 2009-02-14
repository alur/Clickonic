// IShellBrowserImpl.cpp: implementation of the IShellBrowserImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IShellBrowserImpl.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class construction

IShellBrowserImpl::IShellBrowserImpl():
	m_dwRef(0),
	m_pView(NULL),
	m_pView2(NULL),
	m_pFolder(NULL)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// IUnknown methods

HRESULT STDMETHODCALLTYPE IShellBrowserImpl::QueryInterface(REFIID iid, void **ppvObject)
{
	if(ppvObject == NULL)
	{
		return E_POINTER;
	}

	*ppvObject = NULL;

	if (iid == IID_IUnknown)
		*ppvObject = (IUnknown*)(IShellBrowser*) this;
	else if (iid == IID_IOleWindow)
		*ppvObject = (IOleWindow*) this;			
	else if (iid == IID_IShellBrowser)
		*ppvObject = (IShellBrowser*) this;
	else if (iid == IID_IExtractIcon)
		*ppvObject = (IExtractIcon*) this;
	/*
	else if (iid == IID_ICommDlgBrowser)
		*ppvObject = (ICommDlgBrowser*) this;
	*/
	else
		return E_NOINTERFACE;
	((IUnknown*)(*ppvObject))->AddRef();
	return S_OK;
}
	

ULONG IShellBrowserImpl::AddRef()
{
	return ++m_dwRef;
}

ULONG IShellBrowserImpl::Release()
{
	return --m_dwRef;
} 


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// IShellBrowser methods (same as IOleInPlaceFrame)

HRESULT STDMETHODCALLTYPE IShellBrowserImpl::InsertMenusSB(HMENU /* hmenuShared */, LPOLEMENUGROUPWIDTHS /*lpMenuWidths*/)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IShellBrowserImpl::SetMenuSB(HMENU /*hmenuShared*/, HOLEMENU /*holemenuReserved*/, HWND /*hwndActiveObject*/)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IShellBrowserImpl::RemoveMenusSB(HMENU /*hmenuShared*/)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IShellBrowserImpl::SetStatusTextSB(LPCOLESTR /*lpszStatusText*/)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IShellBrowserImpl::EnableModelessSB(BOOL /*fEnable*/)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IShellBrowserImpl::GetViewStateStream(DWORD /*grfMode*/, LPSTREAM * /*ppStrm*/)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IShellBrowserImpl::SetToolbarItems(LPTBBUTTON /*lpButtons*/, UINT /*nButtons*/, UINT /*uFlags*/)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IShellBrowserImpl::TranslateAcceleratorSB(LPMSG /*lpmsg*/, WORD /*wID*/)
{
	//return S_OK;
	return S_FALSE;
}


HRESULT STDMETHODCALLTYPE IShellBrowserImpl::QueryActiveShellView(struct IShellView ** ppshv)
{
	if(!m_pView)
	{
		return E_FAIL;
	}
	m_pView->AddRef();
	*ppshv = m_pView;
	return S_OK; 
}

HRESULT STDMETHODCALLTYPE IShellBrowserImpl::OnViewWindowActive(struct IShellView * /*ppshv*/)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE IShellBrowserImpl::BrowseObject(LPCITEMIDLIST /*pidl*/, UINT /*wFlags*/)
{
	return NOERROR;
}	


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// IOleWindow methods

HRESULT STDMETHODCALLTYPE IShellBrowserImpl::ContextSensitiveHelp(BOOL /*fEnterMode*/)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IShellBrowserImpl::GetWindow(HWND *lphwnd)
{
	*lphwnd = m_hWnd;
	return S_OK; 
}


HRESULT STDMETHODCALLTYPE IShellBrowserImpl::SendControlMsg(THIS_ UINT /*id*/, UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, LRESULT * /*pret*/)
{
	return E_NOTIMPL;
}
	
HRESULT STDMETHODCALLTYPE IShellBrowserImpl::GetControlWindow(THIS_ UINT /*id*/, HWND * /*lphwnd*/)
{
	return E_NOTIMPL;
}

