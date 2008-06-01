// fileimagelist.cpp

#include "resourceppc.h"
#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "multibuff.h"
#include "wcehelper.h"
#include "fileimagelist.h"



CFileImageList::CFileImageList()
{
	m_himl = NULL;
}

CFileImageList::~CFileImageList()
{
}

void CFileImageList::Init()
{
	if (m_himl)
		Destroy();

	int nSize = GetSystemMetrics(SM_CXICON) / 2;
	m_himl = ImageList_Create(nSize, nSize, ILC_COLOR | ILC_MASK, 0, 0);
	SHFILEINFO shfi;
	SHGetFileInfo(_T("\\Windows"), NULL, &shfi, sizeof(shfi), SHGFI_SMALLICON | SHGFI_ICON);
	ImageList_AddIcon(m_himl, shfi.hIcon);
}

void CFileImageList::Destroy()
{
	while (!m_listIndex.IsEmpty()) {
		IMAGELISTINDEX* pIndex = (IMAGELISTINDEX*)m_listIndex.RemoveAt(0);
		if (pIndex != NULL)
			delete pIndex;
	}
	ImageList_RemoveAll(m_himl);
	ImageList_Destroy(m_himl);
	m_himl = NULL;
}

BOOL SeekIndexCallback(LPARAM lParamItem, LPARAM lParamFind)
{
	IMAGELISTINDEX* pIndex = (IMAGELISTINDEX*)lParamItem;
	LPTSTR pszExt = (LPTSTR)lParamFind;
	if (wcscmp(pszExt, pIndex->szExt)==0)
		return FALSE;
	else
		return TRUE;
}

int CFileImageList::GetImageListIndex(LPCTSTR pszFileName, LPCTSTR pszPath)
{
	TCHAR szPath[MAX_PATH];
	wsprintf(szPath, _T("%s\\%s"), pszPath, pszFileName);
	return GetImageListIndex(szPath);
}

int CFileImageList::GetImageListIndex(LPCTSTR pszPath)
{
	TCHAR szPath[MAX_PATH];
	TCHAR szExt[MAX_PATH];
	LPTSTR psz;
	int nIndex;

	DWORD dwAttr = GetFileAttributes(pszPath);
	if (dwAttr == 0xFFFFFFFF)
		return -1; // failture

	if (dwAttr & FILE_ATTRIBUTE_DIRECTORY && 
		!(dwAttr & FILE_ATTRIBUTE_TEMPORARY))
		return 0;

	_tcscpy(szPath, pszPath);
	_tcsupr(szPath);
	psz = wcsrchr(szPath, _T('.'));
	memset(szExt, 0, sizeof(szExt));
	if (dwAttr & FILE_ATTRIBUTE_DIRECTORY && 
		dwAttr & FILE_ATTRIBUTE_TEMPORARY)
		_tcscpy(szExt, szPath);
	else if (psz == NULL)
		_tcscpy(szExt, _T("\\\\\\"));
	else if (wcscmp(psz, _T(".EXE"))==0 || wcscmp(psz, _T(".LNK"))==0)
		_tcscpy(szExt, szPath);
	else
		_tcscpy(szExt, psz);

	nIndex = m_listIndex.Find(SeekIndexCallback, 0, (DWORD)szExt);
	if (nIndex >= 0) {
		IMAGELISTINDEX* pRet = (IMAGELISTINDEX*)m_listIndex.GetAt(nIndex);
		return pRet->nIndex;
	}

	SHFILEINFO shfi;
	SHGetFileInfo(pszPath, NULL, &shfi, sizeof(shfi), SHGFI_SMALLICON | SHGFI_ICON);
	nIndex = ImageList_AddIcon(m_himl, shfi.hIcon);
	IMAGELISTINDEX* pIndex = new IMAGELISTINDEX;
	memset(pIndex, 0, sizeof(IMAGELISTINDEX));
	pIndex->nIndex = nIndex;
	_tcscpy(pIndex->szExt, szExt);
	m_listIndex.Add((DWORD)pIndex);
	return nIndex;
}
