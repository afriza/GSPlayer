//PpcFolderDlg.cpp
#include "GSPlayer2.h"
#include "PpcKeyCtrlDlg.h"

CPpcKeyCtrlDlg::CPpcKeyCtrlDlg()
{
	m_hwndDlg = NULL;
	m_nColorBack = 0;
	m_nColorText = 0;
	m_hBrushBack = NULL;
	m_dwStart = 0;
	m_pOrgStaticProc = NULL;
}

CPpcKeyCtrlDlg::~CPpcKeyCtrlDlg()
{
}

void CPpcKeyCtrlDlg::ShowKeyCtrlDlg(HWND hwndParent, HWND hwndRect, int nColorBack, int nColorText)
{
	m_hwndRect = hwndRect;
	m_nColorBack = nColorBack;
	m_nColorText = nColorText;
	DialogBoxParam(GetInst(), MAKEINTRESOURCE(IDD_KEYCTRL_DLG), hwndParent, PpcKeyCtrlDlgProc, (LPARAM)this);
}

void CPpcKeyCtrlDlg::OnInitDialog(HWND hDlg)
{
	m_dwStart = GetTickCount();
	SetTimer(hDlg, ID_TIMER_KEYCTRLCLOSE, INT_TIMER_KEYCTRLCLOSE, NULL);

	m_hBrushBack = CreateSolidBrush(m_nColorBack);
	InitSize(hDlg);

	SetWindowLong(GetDlgItem(hDlg, IDC_STATIC_CENTER), GWL_USERDATA, (LONG)this);
	m_pOrgStaticProc = (WNDPROC)SetWindowLong(GetDlgItem(hDlg, IDC_STATIC_CENTER), GWL_WNDPROC, (LONG)PpcKeyCtrlStaticProc);
	SetFocus(GetDlgItem(hDlg, IDC_STATIC_CENTER));
}

void CPpcKeyCtrlDlg::OnClose(HWND hDlg)
{
	SetWindowLong(GetDlgItem(hDlg, IDC_STATIC_CENTER), GWL_WNDPROC, (LONG)m_pOrgStaticProc);

	if (m_hBrushBack) {
		DeleteObject(m_hBrushBack);
		m_hBrushBack = NULL;
	}
	KillTimer(hDlg, ID_TIMER_KEYCTRLCLOSE);
	EndDialog(hDlg, IDOK);
}

void CPpcKeyCtrlDlg::OnKeyDown(HWND hDlg, UINT uKeyCode)
{
	switch (uKeyCode) {
	case VK_RETURN:
		PostMessage(hDlg, WM_COMMAND, IDOK, 0);
		break;
	case VK_UP:
		PostMessage(GetParent(hDlg), WM_COMMAND, IDM_TOOL_VOLUP, 0);
		break;
	case VK_RIGHT:
		PostMessage(GetParent(hDlg), WM_COMMAND, IDM_PLAY_NEXT, 0);
		break;
	case VK_DOWN:
		PostMessage(GetParent(hDlg), WM_COMMAND, IDM_TOOL_VOLDOWN, 0);
		break;
	case VK_LEFT:
		PostMessage(GetParent(hDlg), WM_COMMAND, IDM_PLAY_PREV, 0);
		break;
	}
	m_dwStart = GetTickCount();
}

long CPpcKeyCtrlDlg::OnCtlColorStatic(HDC hDC)
{
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, m_nColorText);
	return (long)m_hBrushBack;
}

void CPpcKeyCtrlDlg::OnPaint(HWND hDlg)
{
	PAINTSTRUCT ps = {0};
	HDC hDC = BeginPaint(hDlg, &ps);
	FillRect(ps.hdc, &ps.rcPaint, m_hBrushBack);
	EndPaint(hDlg, &ps);
}

void CPpcKeyCtrlDlg::OnTimer(HWND hDlg, UINT nId)
{
	if (nId == ID_TIMER_KEYCTRLCLOSE) {
		if (GetTickCount() > m_dwStart + KEYCTRLDLG_CLOSE_INT) {
			PostMessage(hDlg, WM_COMMAND, IDOK, 0);
		}
	}
}

BOOL CALLBACK CPpcKeyCtrlDlg::PpcKeyCtrlDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CPpcKeyCtrlDlg* pDlg;
	switch (uMsg) {
	case WM_INITDIALOG:
		pDlg = (CPpcKeyCtrlDlg*)lParam;
		pDlg->OnInitDialog(hDlg);
		pDlg->m_hwndDlg = hDlg;
		return TRUE;
	case WM_KEYDOWN:
		pDlg->OnKeyDown(hDlg, (UINT)wParam);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			pDlg->OnClose(hDlg);
			return TRUE;
		}
		return FALSE;
	case WM_LBUTTONUP:
		PostMessage(hDlg, WM_COMMAND, IDOK, 0);
		return TRUE;
	case WM_PAINT:
		pDlg->OnPaint(hDlg);
		return TRUE;
	case WM_CTLCOLORSTATIC:
		return pDlg->OnCtlColorStatic((HDC)wParam);
	case WM_TIMER:
		pDlg->OnTimer(hDlg, wParam);
		return TRUE;
	case WM_ACTIVATE:
		pDlg->OnActivate(hDlg, wParam);
		return TRUE;
//	case WM_WININICHANGE:
//		pDlg->InitSize(hDlg);
//		return TRUE;
	case WM_DESTROY:
		pDlg->m_hwndDlg = NULL;
		return TRUE;
	}
	return FALSE;
}

void CPpcKeyCtrlDlg::OnActivate(HWND hDlg, int nAction)
{
	switch (nAction) {
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		break;
	case WA_INACTIVE:
		PostMessage(hDlg, WM_COMMAND, IDOK, 0);
		break;
	}
}

void CPpcKeyCtrlDlg::InitSize(HWND hDlg)
{
	RECT rc, rcParent;
	int nXDiff, nYDiff;
	GetWindowRect(m_hwndRect, &rcParent);
	GetWindowRect(hDlg, &rc);
	nXDiff = (RECT_WIDTH(&rcParent) - RECT_WIDTH(&rc)) / 2;
	nYDiff = (RECT_HEIGHT(&rcParent) - RECT_HEIGHT(&rc)) / 2;
	MoveWindow(hDlg, rcParent.left, rcParent.top, RECT_WIDTH(&rcParent), RECT_HEIGHT(&rcParent), TRUE);

	GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_UP), &rc);
	ScreenToClient(hDlg, (LPPOINT)&rc);
	SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_UP), 0, rc.left + nXDiff, rc.top + nYDiff, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_DOWN), &rc);
	ScreenToClient(hDlg, (LPPOINT)&rc);
	SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_DOWN), 0, rc.left + nXDiff, rc.top + nYDiff, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_LEFT), &rc);
	ScreenToClient(hDlg, (LPPOINT)&rc);
	SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_LEFT), 0, rc.left + nXDiff, rc.top + nYDiff, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_RIGHT), &rc);
	ScreenToClient(hDlg, (LPPOINT)&rc);
	SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_RIGHT), 0, rc.left + nXDiff, rc.top + nYDiff, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_CENTER), &rc);
	ScreenToClient(hDlg, (LPPOINT)&rc);
	SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_CENTER), 0, rc.left + nXDiff, rc.top + nYDiff, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

LRESULT CALLBACK CPpcKeyCtrlDlg::PpcKeyCtrlStaticProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CPpcKeyCtrlDlg* pDlg = (CPpcKeyCtrlDlg*)GetWindowLong(hWnd, GWL_USERDATA);
	switch (uMsg) {
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
		pDlg->OnKeyDown(pDlg->m_hwndDlg, wParam);
		return 0;
	}
	return CallWindowProc(pDlg->m_pOrgStaticProc, hWnd, uMsg, wParam, lParam);
}

void CPpcKeyCtrlDlg::UpdateSize()
{
	if (m_hwndDlg) {
		InitSize(m_hwndDlg);
	}
}
