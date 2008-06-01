#ifndef __WCEHELPER_H__
#define __WCEHELPER_H__

#include <aygshell.h>

#define MAX_LOADSTRING 256
extern HINSTANCE g_hInst;

// global helpers
void SetFormatSize(DWORD dwSize, TCHAR szBuff[64], LPTSTR pszFmtKB, LPTSTR pszFmtMB);
void SetFormatDouble(LPTSTR pszDouble, UINT nSize);
void SetFormatDateTime(SYSTEMTIME* pst, LPTSTR pszBuff, UINT nSize);
HFONT CreatePointFont(int nPointSize, LPCTSTR pszFaceName, BOOL fBold);

// class CWinceHepler
class CWinceHepler
{
public:
	CWinceHepler();
	~CWinceHepler();
	static BOOL IsPocketPC();
	static BOOL IsSmartPhone();

	BOOL DefDlgPaintProc(HWND hDlg, WPARAM wParam, LPARAM lParam);
	BOOL DefDlgCtlColorStaticProc(HWND hDlg, WPARAM wParam, LPARAM lParam);
	void SHInitDialog(HWND hwndDlg);
	HWND SHCreateMenuBar(HWND hwndParent, int nMenuID);
	BOOL IsSipPanelVisible();

protected:
	HINSTANCE m_hAygShell;
	BOOL (*m_pSHInitDialog)(PSHINITDLGINFO pshidi);
	BOOL (*m_pSHCreateMenuBar)(PSHMENUBARINFO pmb);

	HINSTANCE m_hCoreDll;
	BOOL (*m_pSipGetInfo)(SIPINFO *pSipInfo);
};

#endif // __WCEHELPER_H__