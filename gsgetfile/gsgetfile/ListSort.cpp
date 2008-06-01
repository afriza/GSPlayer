#include <windows.h>
#include <commdlg.h>
#include "filedlg.h"

///////////////////////////////////////////////////////////////
int CALLBACK ListSortCompareFuncByName(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	LIST_ITEM_INFO* pList1 = (LIST_ITEM_INFO*)lParam1;
	LIST_ITEM_INFO* pList2 = (LIST_ITEM_INFO*)lParam2;
	BOOL down = (BOOL)lParamSort;

	if (pList1->type < pList2->type)
		return -1;
	else if (pList1->type == pList2->type) {
		TCHAR szName1[MAX_PATH] = _T("");
		TCHAR szName2[MAX_PATH] = _T("");
		_tcscpy(szName1, pList1->pszName);
		_tcsupr(szName1);
		_tcscpy(szName2, pList2->pszName);
		_tcsupr(szName2);
		return down ? _tcscmp(szName1, szName2) : _tcscmp(szName2, szName1);
	}
	else if (pList1->type > pList2->type)
		return 1;

	return 0;
}

///////////////////////////////////////////////////////////////
int CALLBACK ListSortCompareFuncByExt(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	LIST_ITEM_INFO* pList1 = (LIST_ITEM_INFO*)lParam1;
	LIST_ITEM_INFO* pList2 = (LIST_ITEM_INFO*)lParam2;
	BOOL down = (BOOL)lParamSort;

	if (pList1->type < pList2->type)
		return -1;
	else if (pList1->type == pList2->type) {
		TCHAR szName1[MAX_PATH] = _T("");
		TCHAR szName2[MAX_PATH] = _T("");
		_tcscpy(szName1, pList1->pszName);
		_tcsupr(szName1);
		_tcscpy(szName2, pList2->pszName);
		_tcsupr(szName2);
		if (pList1->type == ITEM_TYPE_DIR || 
			pList1->type == ITEM_TYPE_SPECIAL)
			return _tcscmp(szName1, szName2);
		else {
			LPCTSTR pszExt1 = _tcschr(szName1,_T('.'));
			LPCTSTR pszExt2 = _tcschr(szName2,_T('.'));
			if (pszExt1 && pszExt2) {
				if (down) {
					int nRet = _tcscmp(pszExt1, pszExt2);
					return nRet? nRet : _tcscmp(szName1, szName2);
				}	
				else {
					int nRet = _tcscmp(pszExt2, pszExt1);
					return nRet ? nRet : _tcscmp(szName2, szName1);
				}
			}
			else if (pszExt1==NULL)
				return down?-1:1;
			else
				return down?1:-1;
		}
	}
	else if (pList1->type > pList2->type)
		return 1;
	return 0;
}
///////////////////////////////////////////////////////////////
int CALLBACK ListSortCompareFuncBySize(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	LIST_ITEM_INFO* pList1 = (LIST_ITEM_INFO*)lParam1;
	LIST_ITEM_INFO* pList2 = (LIST_ITEM_INFO*)lParam2;
	BOOL down = (BOOL)lParamSort;

	if (pList1->type < pList2->type)
		return -1;
	else if (pList1->type == pList2->type) {
		TCHAR szName1[MAX_PATH] = _T("");
		TCHAR szName2[MAX_PATH] = _T("");
		_tcscpy(szName1, pList1->pszName);
		_tcsupr(szName1);
		_tcscpy(szName2, pList2->pszName);
		_tcsupr(szName2);
		if (pList1->type == ITEM_TYPE_DIR ||
			pList1->type == ITEM_TYPE_SPECIAL) {
			return _tcscmp(szName1, szName2);
		}
		else
		{
			if (down) {
				if (pList1->llSize < pList2->llSize)
					return -1;
				else if (pList1->llSize == pList2->llSize)
					return 0;
				else if (pList1->llSize > pList2->llSize)
					return 1;
			}
			else {
				if (pList1->llSize < pList2->llSize)
					return 1;
				else if (pList1->llSize == pList2->llSize)
					return 0;
				else if (pList1->llSize > pList2->llSize)
					return -1;
			}
		}
	}
	else if (pList1->type > pList2->type)
		return 1;

	return 0;
}
///////////////////////////////////////////////////////////////
int CALLBACK ListSortCompareFuncByTime(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	LIST_ITEM_INFO* pList1 = (LIST_ITEM_INFO*)lParam1;
	LIST_ITEM_INFO* pList2 = (LIST_ITEM_INFO*)lParam2;
	BOOL down = (BOOL)lParamSort;

	if (pList1->type < pList2->type)
		return -1;
	else if (pList1->type == pList2->type)
		return down ? CompareFileTime(&pList1->ft, &pList2->ft) : CompareFileTime(&pList2->ft, &pList1->ft);
	else if (pList1->type > pList2->type)
		return 1;

	return 0;
}