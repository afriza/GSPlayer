#ifndef __EFFECTDLG_H__
#define __EFFECTDLG_H__

class CEffectDlg
{
public:
	CEffectDlg();
	~CEffectDlg();
	void ShowEffectDlg(HWND hwndParent, HANDLE hMap);

protected:
	static BOOL CALLBACK EffectDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnInitDialog(HWND hwndDlg);
	void OnSelChanged(HWND hwndDlg, UINT uId);
	void OnUpDown(HWND hwndDlg, NMUPDOWN* pnmud);
	void InitEffectList(HWND hwndList, int nMin, int nMax, int nVal);
	
protected:
	HANDLE	m_hMap;
	HWND	m_hwndMB;
};

#endif // __EFFECTDLG_H__