#include "gsgetfile.h"
#include "filedlg.h"

HINSTANCE g_hInst = NULL;

BOOL CheckPointer(LPOPENFILENAME pofn)
{
	if (pofn->lStructSize != sizeof(OPENFILENAME))
		return FALSE;

	if (pofn->lpstrFile == NULL ||
		pofn->nMaxFile == 0)
		return FALSE;

	for (UINT i = 0; i < pofn->nMaxFile; i++) {
		if (pofn->lpstrFile[i] == NULL)
			return TRUE;
	}
	return FALSE;
}

BOOL WINAPI gsGetOpenFileName(LPOPENFILENAME pofn)
{
	if (!CheckPointer(pofn))
		return FALSE;

	CFileDialog* pDlg = new CFileDialog(pofn);
	BOOL fRet = pDlg->DoModal() == IDOK;
	delete pDlg;
	return fRet;
}

BOOL WINAPI gsGetSaveFileName(LPOPENFILENAME pofn)
{
	if (!CheckPointer(pofn))
		return FALSE;

	CFileDialog* pDlg = new CFileDialog(pofn);
	BOOL fRet = pDlg->DoModal(TRUE) == IDOK;
	delete pDlg;
	return fRet;
}

BOOL APIENTRY DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
	g_hInst = (HINSTANCE)hinstDLL;
	return TRUE;
}
