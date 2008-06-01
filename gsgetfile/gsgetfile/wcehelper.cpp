#include "resourceppc.h"
#include "windows.h"
#include "wcehelper.h"

// global helpers
void SetFormatSize(DWORD dwSize, TCHAR szBuff[64], LPTSTR pszFmtKB, LPTSTR pszFmtMB)
{
	double fSize;
	TCHAR szSize[64];
	if (dwSize < 1024000)
		fSize = (double)dwSize/1024;	
	else
		fSize = (double)dwSize/1048576;
	swprintf(szSize, _T("%f"), fSize);
	SetFormatDouble(szSize, 64);
		
	if (dwSize < 1024000)
		swprintf(szBuff, pszFmtKB, szSize);
	else
		swprintf(szBuff, pszFmtMB, szSize);
}

void SetFormatDouble(LPTSTR pszDouble, UINT nSize)
{
	LPTSTR psz = new TCHAR[nSize];
	memset(psz, 0, sizeof(TCHAR) * nSize);
	GetNumberFormat(LOCALE_USER_DEFAULT, 0, pszDouble, NULL, psz, nSize);
	_tcscpy(pszDouble, psz);
	delete[] psz;
}

void SetFormatDateTime(SYSTEMTIME* pst, LPTSTR pszBuff, UINT nSize)
{
	LPTSTR psz = new TCHAR[nSize];
	memset(psz, 0, sizeof(TCHAR) * nSize);

	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, pst, NULL, psz, nSize);
	memset(pszBuff, 0, sizeof(TCHAR) * nSize);
	_tcscpy(pszBuff, psz);
	memset(psz, 0, sizeof(TCHAR) * nSize);
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, pst, NULL, psz, nSize);
	_tcscat(pszBuff, _T(" "));
	_tcscat(pszBuff, psz);
	delete[] psz;
}

HFONT CreatePointFont(int nPointSize, LPCTSTR pszFaceName, BOOL fBold)
{
#ifdef _WIN32_WCE
	LOGFONT logFont;
	memset(&logFont, 0, sizeof(LOGFONT));
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfHeight = nPointSize;
	if (fBold)
		logFont.lfWeight = 700;
	_tcscpy(logFont.lfFaceName, pszFaceName);

	HDC hDC = ::GetDC(NULL);

	POINT pt;
	pt.y = ::GetDeviceCaps(hDC, LOGPIXELSY) * logFont.lfHeight;
	pt.y /= 720;
	POINT ptOrg = { 0, 0 };
	logFont.lfHeight = -abs(pt.y - ptOrg.y);
	ReleaseDC(NULL, hDC);

	return CreateFontIndirect(&logFont);
#else
	return NULL;
#endif
}

// class CWinceHepler
CWinceHepler::CWinceHepler() : 
m_hAygShell(NULL), m_pSHInitDialog(NULL)
{
	m_hAygShell = LoadLibrary(_T("aygshell.dll"));
	if (m_hAygShell) {
		(FARPROC&)m_pSHInitDialog = GetProcAddress(m_hAygShell, _T("SHInitDialog"));
		(FARPROC&)m_pSHCreateMenuBar = GetProcAddress(m_hAygShell, _T("SHCreateMenuBar"));		
	}
	m_hCoreDll = LoadLibrary(_T("coredll.dll"));
	if (m_hCoreDll) {
		(FARPROC&)m_pSipGetInfo = GetProcAddress(m_hCoreDll, _T("SipGetInfo"));
	}
}

CWinceHepler::~CWinceHepler()
{
	if (m_hAygShell)
		FreeLibrary(m_hAygShell);
	if (m_hCoreDll)
		FreeLibrary(m_hCoreDll);
}

BOOL CWinceHepler::DefDlgPaintProc(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	if (IsPocketPC()) {
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hDlg, &ps);
		POINT pt[2] = {{0, 24},{GetSystemMetrics(SM_CXSCREEN),24}};
		Polyline(hDC, pt, sizeof(pt)/sizeof(POINT));
		EndPaint(hDlg, &ps);
		return TRUE;
	}
	return FALSE;
}

BOOL CWinceHepler::DefDlgCtlColorStaticProc(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	HDC hDC = (HDC)wParam;
	if (GetDlgCtrlID((HWND)lParam) == IDC_TITLE ||
		GetDlgCtrlID((HWND)lParam) == IDC_STATIC_CURRENT_TEXT)
	{
		SetBkMode(hDC, TRANSPARENT);
		SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
		return (long)GetStockObject(WHITE_BRUSH);
	}
	else
		return FALSE;
}

void CWinceHepler::SHInitDialog(HWND hwndDlg)
{
	if (m_hAygShell && m_pSHInitDialog) {
		SHINITDLGINFO shidi;
		shidi.dwMask = SHIDIM_FLAGS;
		shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN;
		shidi.hDlg = hwndDlg;
		m_pSHInitDialog(&shidi);
	}
}

HWND CWinceHepler::SHCreateMenuBar(HWND hwndParent, int nMenuID)
{
	if (m_pSHCreateMenuBar) {
		SHMENUBARINFO mbi;
		memset(&mbi, 0, sizeof(SHMENUBARINFO));
		mbi.cbSize = sizeof(SHMENUBARINFO);
		mbi.hwndParent = hwndParent;
		mbi.nToolBarId = nMenuID;
		mbi.hInstRes = g_hInst;
		
		if (m_pSHCreateMenuBar(&mbi)) 
			return mbi.hwndMB;
	}
	return NULL;
}

BOOL CWinceHepler::IsSipPanelVisible()
{
	if (m_pSipGetInfo) {
		SIPINFO si;
		memset(&si, 0, sizeof(SIPINFO));
		si.cbSize = sizeof(SIPINFO);
		m_pSipGetInfo(&si);
		if ((si.fdwFlags & SIPF_ON))
			return TRUE;
	}
	return FALSE;
}

BOOL CWinceHepler::IsSmartPhone()
{
	TCHAR szPlatform[MAX_PATH] = {0};
	SystemParametersInfo(SPI_GETPLATFORMTYPE, MAX_PATH, szPlatform, 0);
	return _tcscmp(szPlatform, _T("SmartPhone")) == 0 ? TRUE : FALSE;
}

BOOL CWinceHepler::IsPocketPC()
{
	TCHAR szPlatform[MAX_PATH] = {0};
	SystemParametersInfo(SPI_GETPLATFORMTYPE, MAX_PATH, szPlatform, 0);
	return _tcscmp(szPlatform, _T("PocketPC")) == 0 ? TRUE : FALSE;
}
