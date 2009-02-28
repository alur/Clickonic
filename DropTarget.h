// DropTarget.h: interface for the CDropTarget class.
//////////////////////////////////////////////////////////////////////

#pragma once

#define CLIPFORMATS_USED 2

class CGroup;

class CDropTarget : public IDropTarget {
public:
	CDropTarget();
	virtual ~CDropTarget();

	// IUnknown methods

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	// IDropTarget methods

	HRESULT STDMETHODCALLTYPE DragEnter(
		IDataObject *pDataObj,
		DWORD grfKeyState,
		POINTL ScPt,
		DWORD *pdwEffect);

	HRESULT STDMETHODCALLTYPE DragOver( 
		DWORD grfKeyState,
		POINTL ScPt,
		DWORD *pdwEffect);

	HRESULT STDMETHODCALLTYPE DragLeave(void);

	HRESULT STDMETHODCALLTYPE Drop( 
		IDataObject *pDataObj,
		DWORD grfKeyState,
		POINTL ScPt,
		DWORD *pdwEffect);

	// locally used stuff

	CGroup *m_pGroup;
	IDropTargetHelper *m_pDropHelper;

	HRESULT Register(CGroup* pGroup);
	inline static HRESULT Revoke(HWND hWnd)
	{
		return RevokeDragDrop(hWnd);
	}
	static void RegisterFormats();
	static UINT cf_types[CLIPFORMATS_USED];
};