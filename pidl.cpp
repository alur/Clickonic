#include "stdafx.h"
#include "pidl.h"
#include "utils.h"

LPITEMIDLIST PIDL::ParseDisplayName(const char *szPath)
{
	char szFullPath[MAX_PATH];
	WCHAR wszFullPath[MAX_PATH];

	LPSHELLFOLDER psfFolder;
	LPITEMIDLIST pidlMain = NULL;
	ULONG ulCount;

	bool bSuccess = false;

	// Get full file name and convert it to the Unicode form
	GetFullPathName(szPath, MAX_PATH, szFullPath, NULL);
	MultiByteToWideChar (CP_ACP, 0, szFullPath, -1, wszFullPath, MAX_PATH);

	// Get pointers to the desktop's IShellFolder interface.
	if (SUCCEEDED (SHGetDesktopFolder (&psfFolder)))
	{
		// Convert the path name into a pointer to an item ID list (pidl).
		if (SUCCEEDED(psfFolder->ParseDisplayName (NULL, NULL, wszFullPath, &ulCount, &pidlMain, NULL)))
		{
			if (pidlMain != NULL)
				bSuccess = true;
		}
		psfFolder->Release();
	}

	if (bSuccess)
		return pidlMain;
	else
		return NULL;
}

LPITEMIDLIST PIDL::ParseDisplayNameW(const wchar_t *wszPath)
{
	WCHAR wszFullPath[MAX_PATH];

	LPSHELLFOLDER psfFolder;
	LPITEMIDLIST pidlMain = NULL;
	/*ULONG ulCount;*/

	bool bSuccess = false;

	// Get full file name and convert it to the Unicode form
	//GetFullPathNameW (wszPath, MAX_PATH, wszFullPath, NULL);
	// GetFullPathNameW doesn't work on Win9x without Unicode Layer
	wcscpy_s(wszFullPath, MAX_PATH, wszPath);

	// Get pointers to the desktop's IShellFolder interface.
	if (SUCCEEDED (SHGetDesktopFolder (&psfFolder)))
	{
		// Convert the path name into a pointer to an item ID list (pidl).
		if (SUCCEEDED(psfFolder->ParseDisplayName (NULL, NULL, wszFullPath, /*&ulCount*/ NULL, &pidlMain, NULL)))
		{
			if (pidlMain != NULL && /*ulCount*/ GetItemCount(pidlMain) != 0)
				bSuccess = true;
		}
		psfFolder->Release();
	}

	if (bSuccess)
		return pidlMain;
	else
		return NULL;
}

UINT PIDL::GetItemCount (LPITEMIDLIST pidl)
{
	USHORT nLen;
	UINT nCount = 0;
	while ((nLen = pidl->mkid.cb) != 0)
	{
		pidl = GetNextItem (pidl);
		nCount++;
	}
	return nCount;
}

LPITEMIDLIST PIDL::GetNextItem (LPITEMIDLIST pidl)
{
	USHORT nLen;

	if ((nLen = pidl->mkid.cb) == 0)
		return NULL;
	else
		return (LPITEMIDLIST) (((LPBYTE) pidl) + nLen);
}

LPITEMIDLIST PIDL::DuplicateItem(LPITEMIDLIST pidl)
{
	USHORT nLen = pidl->mkid.cb;
	if (nLen == 0)
		return NULL;

	LPITEMIDLIST pidlNew = (LPITEMIDLIST) CoTaskMemAlloc (nLen + sizeof (USHORT));
	if (pidlNew == NULL)
		return NULL;

	CopyMemory (pidlNew, pidl, nLen);
	*((USHORT*) (((LPBYTE) pidlNew) + nLen)) = 0;

	return pidlNew;
}

UINT PIDL::GetSize(LPCITEMIDLIST pidl)
{
	UINT cbTotal = 0;
	if (pidl)
	{
		cbTotal += sizeof(pidl->mkid.cb);
		while (pidl->mkid.cb)
		{
			cbTotal += pidl->mkid.cb;
			pidl = PIDLNext(pidl);
		}
	}
	return cbTotal;
}

LPITEMIDLIST PIDL::GetLastItemOf (LPITEMIDLIST pidl)
{
	USHORT nLen;
	LPITEMIDLIST pidlNext, pidlLast;

	if ((nLen = pidl->mkid.cb) == 0)
		return NULL;

	pidlNext = (LPITEMIDLIST) (((LPBYTE) pidl) + nLen);
	pidlLast = GetLastItemOf(pidlNext);

	return (pidlLast) ? pidlLast : pidl;
}

LPITEMIDLIST PIDL::Append(LPCITEMIDLIST pidlBase, LPCITEMIDLIST pidlAdd)
{
	if (pidlBase == NULL)
		return NULL;

	if (pidlAdd == NULL)
		return const_cast<LPITEMIDLIST>(pidlBase);
    
	UINT cb1 = GetSize(pidlBase), cb2 = GetSize(pidlAdd);

	LPITEMIDLIST pidlNew = (LPITEMIDLIST)CoTaskMemAlloc(cb1 + cb2);

    if (pidlNew)
	{
		CopyMemory( pidlNew, pidlBase, cb1);
		CopyMemory( reinterpret_cast<PBYTE>(pidlNew) + cb1 - sizeof(pidlBase->mkid.cb), pidlAdd, cb2 );
	}
	return pidlNew;
}

LPSHELLFOLDER PIDL::GetFolder(LPITEMIDLIST pidl)
{
	LPSHELLFOLDER psfDesktop = NULL, psfFolder = NULL;
	if (SHGetDesktopFolder(&psfDesktop) != NOERROR)
		return NULL;
	
	if(pidl->mkid.cb == 0)
		psfFolder = psfDesktop;
	else
	{
		HRESULT hr = psfDesktop->BindToObject(pidl, NULL, IID_IShellFolder, (void**)&psfFolder);
		psfDesktop->Release(); // no longer required
		if(FAILED(hr))
			return NULL;
	}
	return psfFolder;
}