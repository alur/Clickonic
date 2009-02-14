#ifndef __PIDL_HEADER__
#define __PIDL_HEADER__

// some unsafe macros

#define PIDLSkip(pidl, cb)       (reinterpret_cast<LPITEMIDLIST>(((BYTE*)(pidl))+cb))
#define PIDLNext(pidl)           PIDLSkip(pidl, (pidl)->mkid.cb)

namespace PIDL
{
	LPITEMIDLIST ParseDisplayName(const char *szPath);
	LPITEMIDLIST ParseDisplayNameW(const wchar_t *wszPath);
	LPSHELLFOLDER GetFolder(LPITEMIDLIST pidl);
	UINT GetItemCount (LPITEMIDLIST pidl);
	LPITEMIDLIST GetNextItem (LPITEMIDLIST pidl);
	LPITEMIDLIST GetLastItemOf (LPITEMIDLIST pidl);
	LPITEMIDLIST DuplicateItem (LPITEMIDLIST pidl);
	UINT GetSize(LPCITEMIDLIST pidl);
	LPITEMIDLIST Append(LPCITEMIDLIST pidlBase, LPCITEMIDLIST pidlAdd);
}

#endif
