#ifndef __PPCOPTIONS_H__
#define __PPCOPTIONS_H__

#include "options.h"

class CPpcMainWnd;
class CPpcOptions : public COptions
{
protected:
	friend CPpcMainWnd;
	BOOL		m_fReleaseKeyMap;
	CMultiBuff	m_listKeyMap;

	int			m_nDispAutoOff;
	BOOL		m_fDispAutoOn;
	BOOL		m_fDispEnableBattery;

public:
	CPpcOptions();
	virtual ~CPpcOptions();
	virtual void ShowOptionDlg(HWND hwndParent, HANDLE hMap);

protected:
	virtual void Save(HANDLE hMap);
	virtual void Load(HANDLE hMap);
	
	static BOOL CALLBACK DisplayPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void DisplayPageOnInitDialog(HWND hwndDlg);
	virtual void DisplayPageOnOK(HWND hwndDlg);
	virtual void LocationDlgOnInitDialog(HWND hwndDlg);
	virtual void LocationDlgOnOK(HWND hwndDlg);

	static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OptionsDlgOnInitDialog(HWND hwndDlg);
	virtual void OptionsDlgOnSize(HWND hwndDlg);
	virtual void OptionsDlgOnOK(HWND hwndDlg);

	static BOOL CALLBACK PlayerPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DecoderPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK StreamingPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK SkinPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK AssociatePageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK PlugInPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
#endif // __PPCOPTIONS_H__