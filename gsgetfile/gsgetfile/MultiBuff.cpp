#include <windows.h>
#include "multibuff.h"

LPCOMPAREFUNC g_pfnCompare = NULL;
DWORD g_dwParam = NULL;

CMultiBuff::CMultiBuff()
{
	m_Head = NULL;
}

CMultiBuff::~CMultiBuff()
{
	RemoveAll();
}

DWORD CMultiBuff::GetAt(int iPos) //zero base
{
	if (!m_Head) return -1;

	MULTIBUFFDATA* pCurrent = GetBuff(iPos);
	return pCurrent->dwData;
}

void CMultiBuff::SetAt(int iPos, DWORD dwValue)
{
	if (!m_Head) return;

	MULTIBUFFDATA* pCurrent = GetBuff(iPos);
	pCurrent->dwData = dwValue;
}

DWORD CMultiBuff::RemoveAt(int iPos)
{
	if (!m_Head) return -1;

	MULTIBUFFDATA* pCurrent = GetBuff(iPos);
	DWORD dwValue = pCurrent->dwData;
	if (pCurrent->next)
		pCurrent->next->prev = pCurrent->prev;
	if (pCurrent->prev)
		pCurrent->prev->next = pCurrent->next;
	if (!iPos)
		m_Head = pCurrent->next;
	delete pCurrent;
	return dwValue;
}

void CMultiBuff::RemoveAll()
{
	MULTIBUFFDATA* pCurrent = m_Head;
	while (pCurrent)
	{
		MULTIBUFFDATA* pNext = pCurrent->next;
		delete pCurrent;
		pCurrent = pNext;
	}
	m_Head = NULL;
}

void CMultiBuff::Insert(DWORD dwValue)
{
	MULTIBUFFDATA* pNext = m_Head;
	m_Head = new MULTIBUFFDATA;
	m_Head->dwData = dwValue;
	if (pNext)
	{
		pNext->prev = m_Head;
		m_Head->next = pNext;
	}
}

int CMultiBuff::Add(DWORD dwValue)
{
	if (!m_Head)
	{
		Insert(dwValue);
		return 0;
	}
	
	//Find Last
	int nIndex = 1;
	MULTIBUFFDATA* pCurrent = m_Head;
	while (pCurrent->next)
	{
		nIndex++;
		pCurrent = pCurrent->next;
	}
	pCurrent->next = new MULTIBUFFDATA;
	pCurrent->next->dwData = dwValue;
	pCurrent->next->prev = pCurrent;

	return nIndex;
}

int CMultiBuff::GetCount()
{
	int nCount = 0;
	MULTIBUFFDATA* pCurrent = m_Head;
	while (pCurrent)
	{
		pCurrent = pCurrent->next;
		nCount++;
	}
	return nCount;
}

BOOL CMultiBuff::IsEmpty()
{
	return m_Head?FALSE:TRUE;
}

#ifdef _WIN32_WCE_EMULATION
int __cdecl SortCompareCallback(const void *arg1, const void *arg2)
#else
int SortCompareCallback(const void *arg1, const void *arg2)
#endif
{
	DWORD* pdwValue1 = (DWORD*)arg1;
	DWORD* pdwValue2 = (DWORD*)arg2;
	return g_pfnCompare(*pdwValue1, *pdwValue2, g_dwParam);
}

BOOL CMultiBuff::Sort(LPCOMPAREFUNC pfnCompare, DWORD dwParam) // execute quick sort
{
	if (!m_Head || !pfnCompare) FALSE;

	int nCount = 0;
	DWORD* pValues = new DWORD[GetCount()];
	MULTIBUFFDATA* pCurrent = m_Head;
	while (pCurrent)
	{
		pValues[nCount++] = pCurrent->dwData;
		pCurrent = pCurrent->next;
	}
	g_pfnCompare = pfnCompare;
	g_dwParam = dwParam;
	qsort(pValues, nCount, sizeof(DWORD), SortCompareCallback);
	g_pfnCompare = NULL;
	g_dwParam = NULL;

	nCount = 0;
	pCurrent = m_Head;
	while (pCurrent)
	{
		pCurrent->dwData = pValues[nCount++];
		pCurrent = pCurrent->next;
	}
	delete[] pValues;
	return TRUE;
}

MULTIBUFFDATA* CMultiBuff::GetBuff(int iPos)
{
	MULTIBUFFDATA* pCurrent = m_Head; 
	for (int i=0; i<iPos; i++)
		pCurrent = pCurrent->next;

	return pCurrent;
}

int CMultiBuff::Find(LPFINDFUNC pfnFind, int iStart, DWORD dwParam)
{
	BOOL bFound = FALSE;
	int nCount = iStart;
	MULTIBUFFDATA* pCurrent = GetBuff(iStart);
	while (pCurrent)
	{
		if (!pfnFind(pCurrent->dwData, dwParam))
		{
			bFound = TRUE;
			break;
		}
		pCurrent = pCurrent->next;
		nCount++;
	}
	return bFound?nCount:-1;
}