#ifndef __PLAYLISTDLG_H__
#define __PLAYLISTDLG_H__

class CMainWnd;
class CPlayListDlg
{
public:
	CPlayListDlg();
	~CPlayListDlg();
	void Show(CMainWnd* pParent);
	void Close();
	void UpdatePlayList();
	void SetCurrent(int nIndex);
	virtual BOOL IsDialogMessage(LPMSG pMsg) {return FALSE;}
};

#endif // __PLAYLISTDLG_H__