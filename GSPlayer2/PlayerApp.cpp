#include "GSPlayer2.h"
#include "PlayerApp.h"
#include "MainWnd.h"

CPlayerApp::CPlayerApp()
{
	m_hInst = NULL;
	m_pszCmdLine = NULL;
	m_pWnd = NULL;
	m_hAccel = NULL;
}

CPlayerApp::~CPlayerApp()
{
	if (m_pWnd) {
		delete m_pWnd;
		m_pWnd = NULL;
	}
}

CMainWnd* CPlayerApp::GetMainWndClass()
{
	return new CMainWnd();
}

HINSTANCE CPlayerApp::GetInst()
{
	return m_hInst;
}

CMainWnd* CPlayerApp::GetMainWnd()
{
	return m_pWnd;
}

BOOL CALLBACK CPlayerApp::EnumForegroundWindowProc(HWND hWnd, LPARAM lParam)
{
	HWND hwndOwner;
	if (!::IsWindowEnabled(hWnd))
		return TRUE;

	if (!::IsWindowVisible(hWnd))
		return TRUE;

#ifdef _WIN32_WCE
	if (::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_NOACTIVATE)
		return TRUE;
#endif

	if (hWnd == (HWND)lParam) {
		::SetForegroundWindow(hWnd);
		return FALSE;
	}

	hwndOwner = hWnd;
	do {
		hwndOwner = ::GetWindow(hwndOwner, GW_OWNER);
		if (hwndOwner == (HWND)lParam) {
			::SetForegroundWindow(hWnd);
			return FALSE;
		}
	} while (hwndOwner);

	return TRUE;
}

int CPlayerApp::Run(HINSTANCE hInst, LPTSTR pszCmdLine)
{
	m_hInst = hInst;
	m_pszCmdLine = pszCmdLine;

#ifdef REGISTER_WAKE_EVENT
	if (_tcscmp(pszCmdLine, APP_RUN_AFTER_WAKEUP) == 0) {
		HWND hwndPrev = FindWindow(MAINWND_CLASS_NAME, NULL);
		if (hwndPrev) {
			PostMessage(hwndPrev, WM_WAKEUP, 0, 0);
			return FALSE;
		}
	}
#endif

	// �d���N���`�F�b�N
	HANDLE hMutex = CreateMutex(NULL, NULL, MUTEX_NAME);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		HWND hwndPrev = FindWindow(MAINWND_CLASS_NAME, NULL);
		if (hwndPrev) {
			if (!(GetWindowLong(hwndPrev, GWL_STYLE) & WS_VISIBLE))
				SendMessage(hwndPrev, WM_COMMAND, IDM_APP_SHOWHIDE, 0);
			//SetForegroundWindow(hwndPrev);
			EnumWindows(EnumForegroundWindowProc, (LPARAM)hwndPrev);
			SendCmdLine(hwndPrev, pszCmdLine);
		}
		return FALSE;
	}

	CMainWnd::CheckSystem();

	// �R�����R���g���[���̏�����
	InitCommonControls();

	// ���C���E�C���h�E�̍쐬
	m_pWnd = GetMainWndClass();
	if (!m_pWnd || !m_pWnd->Create(pszCmdLine))
		return -1;

	// ���b�Z�[�W���[�v
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (m_pWnd->CanTransAccel(&msg)) {
			if (TranslateAccelerator(m_pWnd->GetHandle(), m_pWnd->GetAccelHandle(), &msg))
				continue;
		}
		if (m_pWnd->IsDialogMessage(&msg))
			continue;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
