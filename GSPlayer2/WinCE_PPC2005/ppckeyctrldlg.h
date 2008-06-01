// PpcKeyCtrlDlg.h
#if !defined(__PPCKEYCTRLDLG_H_INCLUDED)
#define __PPCKEYCTRLDLG_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#define ID_TIMER_KEYCTRLCLOSE		1
#define INT_TIMER_KEYCTRLCLOSE		1000
#define KEYCTRLDLG_CLOSE_INT		10000

#define ID_HOTKEY_LEFT				1
#define ID_HOTKEY_RIGHT				2
#define ID_HOTKEY_UP				3
#define ID_HOTKEY_DOWN				4

class CPpcKeyCtrlDlg
{
public:
	CPpcKeyCtrlDlg();
	virtual ~CPpcKeyCtrlDlg();
	void ShowKeyCtrlDlg(HWND hwndParent, HWND hwndRect, int nColorBack, int nColorText);
	void UpdateSize();

protected:
	virtual void OnInitDialog(HWND hDlg);
	virtual void OnClose(HWND hDlg);
	virtual void OnKeyDown(HWND hDlg, UINT uKeyCode);
	virtual long OnCtlColorStatic(HDC hDC);
	virtual void OnPaint(HWND hDlg);
	virtual void OnTimer(HWND hDlg, UINT nId);
	static BOOL CALLBACK PpcKeyCtrlDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void OnActivate(HWND hDlg, int nAction);
	virtual void InitSize(HWND hDlg);
	static LRESULT CALLBACK PpcKeyCtrlStaticProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	HWND		m_hwndDlg;
	HWND		m_hwndRect;
	int			m_nColorBack;
	int			m_nColorText;
	HBRUSH		m_hBrushBack;
	DWORD		m_dwStart;
	WNDPROC		m_pOrgStaticProc;
};

#endif //__PPCKEYCTRLDLG_H_INCLUDED