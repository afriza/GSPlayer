//FolderDlg.h
#if !defined(__PPCFOLDERDLG_H_INCLUDED)
#define __PPCFOLDERDLG_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "FolderDlg.h"

#define ID_BTNDOWN		(50000)

class CPpcFolderDlg : public CFolderDlg
{
public:
	CPpcFolderDlg();
	virtual ~CPpcFolderDlg();

protected:
	virtual void OnInitDialog(HWND);
	virtual void OnOK(HWND);
	virtual void OnCancel(HWND);
	virtual BOOL OnCommand(HWND, WPARAM, LPARAM);
	virtual void OnInitMenuPopup(HMENU);
	virtual void OnBtnDown(HWND hDlg, WPARAM wParam);
	static LRESULT FolderDlgTreeHookProc(HWND, UINT, WPARAM, LPARAM);

	HWND	m_hwndMB;
	WNDPROC m_pOrgProc;

};

#endif //__PPCFOLDERDLG_H_INCLUDED