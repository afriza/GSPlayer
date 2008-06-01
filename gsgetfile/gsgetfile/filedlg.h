#ifndef __FILEDLG_H__
#define __FILEDLG_H__

#include <commctrl.h>
#include "wcehelper.h"
#include "multibuff.h"
#include "fileimagelist.h"

typedef enum {ITEM_TYPE_SPECIAL, ITEM_TYPE_DIR, ITEM_TYPE_FILE}ITEM_TYPE;
typedef enum {LIST_SORT_NAME, LIST_SORT_EXT, LIST_SORT_SIZE, LIST_SORT_TIME}LIST_SORT;

typedef struct _tagListItemComp
{
	ITEM_TYPE type;

	LPTSTR		pszName;
	FILETIME	ft;
	ULONGLONG	llSize;

	//•\Ž¦—p
	int nIcon;
	LPTSTR pszDispName;
	LPTSTR pszDispSize;
	LPTSTR pszDispType;
	LPTSTR pszDispTime;

	_tagListItemComp()
	{
		nIcon = -1;
		pszName = NULL;
		pszDispName = NULL;
		pszDispSize = NULL;
		pszDispType = NULL;
		pszDispTime = NULL;
	}
}LIST_ITEM_INFO;

class CFileDialog
{
public:
	CFileDialog(LPOPENFILENAME pofn);
	~CFileDialog();

	int DoModal(BOOL fSave = FALSE);

protected:
	enum {DLG_TYPE_PPC3, DLG_TYPE_SP} m_fDlgType;
	CWinceHepler	m_helper;
	LPOPENFILENAME	m_pofn;
	HWND			m_hwndDlg;
	HWND			m_hwndCB;
	HWND			m_hwndLV;
	HFONT			m_hFnt;

	BOOL			m_fSave;
	BOOL			m_fShowExt;
	LIST_SORT		m_nListSort;
	BOOL			m_fSortOrder;

	CMultiBuff		m_listExt;
	CFileImageList	m_ImageList;
	TCHAR			m_szCurrent[MAX_PATH];
	TCHAR			m_szRootName[MAX_LOADSTRING];

	LPTSTR			m_pszFilter;
	LPTSTR			m_pszDefExt;

	BOOL			m_fSelMode;
	
protected:
	BOOL OnInitDialog(HWND hwndDlg);
	void OnOK();
	void OnUp();
	void OnListClick();
	void OnListDblClk();
	void OnListKeyDown(NMLVKEYDOWN* pnmk);
	void OnListItemChanged(NM_LISTVIEW* pnmlv);
	void OnListColumnClick(NMLISTVIEW* pnmlv);
	void OnGetDispInfo(NMLVDISPINFO* pnmdi);
	void OnCBSelChange();
	void EndDialog(int nResult);
	static BOOL CALLBACK FileDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	int GetDlgResourceID();
	void CreateToolBar();
	void CheckWindowSize();
	void CheckListHeight();
	void ParseFilter();
	void ClearFilter();

	void GetShellSettings();
	void InitListView();
	void DestroyListView();
	void DeleteAllListItem();
	BOOL LoadFolderItem(LPCTSTR pszPath, BOOL fFocus = TRUE);
	void SortList();
	void AddListItem(LPCTSTR pszPath, WIN32_FIND_DATA* pwfd);
	LIST_ITEM_INFO* GetListItemInfo(int nIndex);
	int GetSelectedItemIndex(int nStart);
	void SelectAllItems();

	void CreateExtList();
	void DeleteExtList();

	LPTSTR GetDisplayName(LPTSTR pszPath);
	void ChangeListStyle(DWORD dwNewStyle);
	BOOL IsFolderShortcut(LPCTSTR pszPath, LPCTSTR pszName);

	void OnSelMode();
	void SelectToggle();
	BOOL OnListItemChanging(NM_LISTVIEW* pnmlv);
	void OnInitMenuPopup(HMENU hMenu);
	void OnSpinDeltaPos(NMUPDOWN* pnmud);

	int GetListSelectedCount();
};

int CALLBACK ListSortCompareFuncByName(LPARAM, LPARAM, LPARAM);
int CALLBACK ListSortCompareFuncByExt(LPARAM, LPARAM, LPARAM);
int CALLBACK ListSortCompareFuncBySize(LPARAM, LPARAM, LPARAM);
int CALLBACK ListSortCompareFuncByTime(LPARAM, LPARAM, LPARAM);

#endif // __FILEDLG_H__