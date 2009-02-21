// DropTarget.cpp: implementation of the CDropTarget class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DropTarget.h"
#include "Group.h"
#include "utils.h"
#include "pidl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDropTarget::CDropTarget()
{
	m_pDropHelper = NULL;
	CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (LPVOID*)&m_pDropHelper);
}

CDropTarget::~CDropTarget()
{
	if (m_pDropHelper)
		m_pDropHelper->Release();
}

UINT CDropTarget::cf_types[CLIPFORMATS_USED];

void CDropTarget::RegisterFormats(void)
{
	cf_types[0] = RegisterClipboardFormat(CFSTR_SHELLIDLIST);
	cf_types[1] = RegisterClipboardFormat(CFSTR_SHELLIDLISTOFFSET);
}


HRESULT STDMETHODCALLTYPE CDropTarget::QueryInterface( REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject) {
	if (riid == IID_IUnknown || riid == IID_IDropTarget)
	{
		*ppvObject = this;
		AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}


ULONG STDMETHODCALLTYPE CDropTarget::AddRef()
{
	return 1;
}


ULONG STDMETHODCALLTYPE CDropTarget::Release()
{
	return 1;
}


HRESULT STDMETHODCALLTYPE CDropTarget::DragEnter(IDataObject *pDataObj, DWORD /* grfKeyState */, POINTL ptl, DWORD *pdwEffect) {
	if (m_pDropHelper) {
		POINT pt = {ptl.x, ptl.y};
		m_pDropHelper->DragEnter(m_pGroup->m_hwndView, pDataObj, &pt, *pdwEffect);
	}

	FORMATETC etc = {0};

	m_pGroup->SetDropHover(&ptl, pdwEffect);
	if (*pdwEffect == DROPEFFECT_NONE)
		return S_OK; // bad drop point =(


	// We want all of the content, in a global handle
	etc.tymed = TYMED_HGLOBAL;
	etc.dwAspect = DVASPECT_CONTENT;
	etc.lindex = -1;

	etc.cfFormat = (CLIPFORMAT)cf_types[0];
	if (pDataObj->QueryGetData(&etc) != S_OK)
		*pdwEffect = DROPEFFECT_NONE;

	return S_OK;
}


HRESULT STDMETHODCALLTYPE CDropTarget::DragOver( DWORD /*grfKeyState*/, POINTL ptl, DWORD *pdwEffect) {
	if (m_pDropHelper) {
		POINT pt = { ptl.x, ptl.y };
		m_pDropHelper->DragOver(&pt, *pdwEffect);
	}
	m_pGroup->SetDropHover(&ptl, pdwEffect);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CDropTarget::DragLeave() {
	if (m_pDropHelper)
		m_pDropHelper->DragLeave();

	m_pGroup->ClearDropHover();
	return S_OK;
}

#define HIDA_GetPIDLParent(pida) (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])
#define HIDA_GetPIDLItem(pida, i) (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])

HRESULT STDMETHODCALLTYPE CDropTarget::Drop( IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	FORMATETC etc = {0};
	STGMEDIUM stg1 = {0}, stg2 = {0}, stg3 = {0};
	CIDA *pida = NULL;
	POINT *pOffsets = NULL;
	bool bExecute;

	if (m_pDropHelper)
	{
		POINT pt = { ptl.x, ptl.y };
		m_pDropHelper->Drop(pDataObj, &pt, *pdwEffect);
	}

	m_pGroup->ClearDropHover();

	etc.tymed = TYMED_HGLOBAL;
	etc.dwAspect = DVASPECT_CONTENT;
	etc.lindex = -1;
	etc.cfFormat = (CLIPFORMAT)cf_types[0];

	IDropTarget *pObject = m_pGroup->GetLastItemTarget(&bExecute);

	if (pObject)
	{
		pObject->DragEnter(pDataObj, grfKeyState, ptl, pdwEffect);
		pObject->DragOver(grfKeyState, ptl, pdwEffect);
		pObject->Drop(pDataObj, grfKeyState, ptl, pdwEffect);
		pObject->Release();
		return S_OK;
	}

	LPSHELLFOLDER pDesktopFolder;
	SHGetDesktopFolder(&pDesktopFolder);
	bool bReorderOnly = false;
	if (pDataObj->GetData(&etc, &stg1) == S_OK)
	{
		pida = reinterpret_cast<CIDA*>(GlobalLock(stg1.hGlobal));
		bReorderOnly = (pDesktopFolder->CompareIDs(0, HIDA_GetPIDLParent(pida), m_pGroup->m_pidlFolder) == 0);
		GlobalUnlock(stg1.hGlobal);
	}

	/// Grab the stuff
	if (pDataObj->GetData(&etc, &stg1) == S_OK)
	{
		pida = reinterpret_cast<CIDA*>(GlobalLock(stg1.hGlobal));
		if (pida == NULL)
		{
			ReleaseStgMedium(&stg1);
			return E_UNEXPECTED;
		}
		etc.cfFormat = (CLIPFORMAT)cf_types[1];
		if (pDataObj->GetData(&etc, &stg2) == S_OK)
			pOffsets = reinterpret_cast<POINT*>(GlobalLock(stg2.hGlobal));
		else
			pOffsets = NULL;
	}

	if (bReorderOnly || bExecute) // Local drag/drop
	{
		m_pGroup->Drop(grfKeyState, &ptl, pdwEffect, pida, pOffsets);
	}
	else // External drag/drop
	{
		//pida = reinterpret_cast<CIDA*>(GlobalLock(stg1.hGlobal));
		//LPITEMIDLIST pidl = PIDL::ParseDisplayName(m_pGroup->m_szFolderLocation);
		//LPITEMIDLIST pidl = (LPITEMIDLIST)CoTaskMemAlloc(sizeof(USHORT));
		//ZeroMemory(pidl, sizeof(USHORT));
		POINTL ptLocal;
		ptLocal.x = ptl.x - m_pGroup->m_nX;
		ptLocal.y = ptl.y - m_pGroup->m_nY;
		m_pGroup->m_pFolder->GetUIObjectOf(NULL, 1,(LPCITEMIDLIST*)&m_pGroup->m_pidlFolder, IID_IDropTarget, 0, (void**)&pObject);
		if (pObject) // Only works for the desktop
		{
			m_pGroup->m_NextIconPosition = ptLocal; // Temporary "fix" for icon positions
			pObject->DragEnter(pDataObj, grfKeyState, ptl, pdwEffect);
			pObject->DragOver(grfKeyState, ptl, pdwEffect);
			pObject->Drop(pDataObj, grfKeyState, ptl, pdwEffect);
			pObject->Release();
		}
		else // Whatever... We'll just handle it ourselfs.
		{
			etc.cfFormat = CF_HDROP;
			if (pDataObj->GetData(&etc, &stg3) == S_OK)
			{
				char szFile[MAX_PATH];
				char szList[MAX_LINE_LENGTH*2] = {0};
				SHFILEOPSTRUCT FileOp = {0};
				DWORD dwEffect;
				HDROP hFiles = (HDROP)stg3.hGlobal;
				UINT nNumFiles = DragQueryFile(hFiles, 0xFFFFFFFF, szFile, MAX_PATH);
				memcpy(&dwEffect, pdwEffect, sizeof(DWORD));
				bool bCreateLinks = ((dwEffect & DROPEFFECT_LINK) == DROPEFFECT_LINK);

				LPSTR pszPos = szList;
				for (UINT i = 0; i < nNumFiles; i++)
				{
					DragQueryFile(hFiles, i, szFile, MAX_PATH);
					if (bCreateLinks)
					{
						pszPos = strrchr(szFile, '\\');
						StringCchPrintf(szList, sizeof(szList), "%s%s.lnk", m_pGroup->m_szFolderLocation, pszPos);
						utils::CreateLink(szFile, szList, "");
					}
					else
					{
						StringCchCopy(pszPos, szList+sizeof(szList)-pszPos, szFile);
						pszPos += strlen(szFile)+1;
					}
				}

				if (!bCreateLinks)
				{
					if ((dwEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY)
						FileOp.wFunc = FO_COPY;
					else if ((dwEffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE)
						FileOp.wFunc = FO_MOVE;

					FileOp.pFrom = szList;
					FileOp.pTo = m_pGroup->m_szFolderLocation;
					FileOp.hwnd = NULL;
					FileOp.fFlags = FOF_NOCONFIRMMKDIR;
					if (FileOp.wFunc != NULL)
					{
						SHFileOperation(&FileOp);
					}
					if (!FileOp.fAnyOperationsAborted) // Sort the icons
					{
						m_pGroup->m_NextIconPosition = ptLocal; // Temporary "fix" for icon positions
						//m_pGroup->Drop(grfKeyState, &ptl, pdwEffect, pida, pOffsets);
					}
				}
				GlobalUnlock(stg3.hGlobal);
				ReleaseStgMedium(&stg3);
			}
		}

		// unlock global memory
		GlobalUnlock(stg1.hGlobal);
		if (pOffsets)
			GlobalUnlock(stg2.hGlobal);

		// and free the mediums
		ReleaseStgMedium(&stg1);
		ReleaseStgMedium(&stg2);

		//CoTaskMemFree(pidl);
		return S_OK;
	}
	return S_OK;
}

HRESULT CDropTarget::Register(CGroup *pGroup)
{
	m_pGroup = pGroup;
	return RegisterDragDrop(m_pGroup->m_hwndListView, this);
}